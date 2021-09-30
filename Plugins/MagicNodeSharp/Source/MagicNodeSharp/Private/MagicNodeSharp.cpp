//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MagicNodeSharp.h"

#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"

#include "UObject/UObjectGlobals.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Runtime/Core/Public/Async/TaskGraphInterfaces.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformProcess.h"

#include "Runtime/Core/Public/HAL/FileManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
 #include "Editor/UnrealEd/Public/Editor.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
 FOnScriptRuntimeException UMagicNodeSharp::OnScriptRuntimeException;
 FOnScriptSourceDeleted UMagicNodeSharpSource::OnScriptSourceDeleted;
 FOnScriptSourceExported UMagicNodeSharpSource::OnScriptSourceExported;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UMagicNodeSharp::UMagicNodeSharp() {
	MonoThread = EMonoThread::MainThread;
	IsAsyncTask = false;
	OBJHandle = 0;
	//
	World = nullptr;
	//
	Except = nullptr;
	Update = nullptr;
	MonoOnLoad = nullptr;
	MonoOnExit = nullptr;
	MonoOnStart = nullptr;
	MonoOnUpdate = nullptr;
	MonoOnExecute = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::Throw_MonoException() {
	if (Except==nullptr) {return;}
	//
	const char* Message = mono_string_to_utf8(mono_object_to_string(Except,NULL));
	LOG::CS_CHAR(ESeverity::Error,StringCast<WIDECHAR>(Message).Get());
	//
	FBlueprintExceptionInfo ExceptionInfo(
		EBlueprintExceptionType::NonFatalError,
		FText::Format(NSLOCTEXT(
			"Throw_MonoException","MonoException","   ===>   {0}   on   {1}"),
			FText::FromString(this->GetName()),FText::FromString(this->GetOuter()->GetName())
		)///
	);///
	//
	FMessageLog("PIE").Error(FText::FromString(StringCast<WIDECHAR>(Message).Get()))->AddToken(FTextToken::Create(ExceptionInfo.GetDescription()));
	Except = nullptr;
	//
	#if WITH_EDITOR
	 OnScriptRuntimeException.Broadcast(Source.Get(),StringCast<WIDECHAR>(Message).Get());
	#endif
}

void UMagicNodeSharp::Throw_MonoError(const FString &Error) {
	LOG::CS_STR(ESeverity::Error,Error);
	//
	FBlueprintExceptionInfo ExceptionInfo(
		EBlueprintExceptionType::NonFatalError,
		FText::Format(NSLOCTEXT(
			"Throw_MonoError", "MonoError","   ===>   {0}   on   {1}"),
			FText::FromString(this->GetName()),FText::FromString(this->GetOuter()->GetName())
		)///
	);///
	//
	FMessageLog("PIE").Error(FText::FromString(Error))->AddToken(FTextToken::Create(ExceptionInfo.GetDescription()));
	//
	#if WITH_EDITOR
	 OnScriptRuntimeException.Broadcast(Source.Get(),StringCast<WIDECHAR>(*Error).Get());
	#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UMagicNodeSharp* UMagicNodeSharp::LoadScript(UObject* Context, UMagicNodeSharpSource* Script) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	if (!MonoCore.IsPlaying()) {return NULL;}
	//
	if (Context->HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) {return NULL;}
	if (Context->IsPendingKill()) {return NULL;}
	//
	if (Script==nullptr) {LOG::CSX_CHAR(ESeverity::Error,TEXT("Invalid Magic Node Script.")); return NULL;}
	if (Context==nullptr) {LOG::CSX_STR(ESeverity::Error,FString::Printf(TEXT("Invalid World Context for Node:  %s"),*Script->GetScriptName())); return NULL;}
	//
	const char* ScriptName = StringCast<ANSICHAR>(*Script->GetScriptName()).Get();
	const FName ID = MakeUniqueObjectName(Context,UMagicNodeSharp::StaticClass(),*Script->GetScriptName());
	//
	UMagicNodeSharp* Node = NewObject<UMagicNodeSharp>(Context,UMagicNodeSharp::StaticClass(),*ID.ToString(),RF_NoFlags);
	Node->Register(Context->GetWorld());
	//
	#if WITH_EDITOR
	Node->Source = Script;
	#endif
	//
	if (MonoImage*PlayImage=MonoCore.mono_find_image(Script->GetScriptName())) {
		LOG::CS_STR(ESeverity::Info,FString::Printf(TEXT("Node Image loaded:  %s"),*Script->GetScriptName()));
		//
		LOG::CS_STR(ESeverity::Info,FString::Printf(TEXT("Creating Managed Node instance for:  %s"),*ID.ToString()));
		MonoClass* ManagedClass = mono_class_from_name_case(PlayImage,CS_CORE_NAMESPACE,ScriptName);
		//
		if (ManagedClass==nullptr) {Script->AssertScriptClass(); return NULL;}
		if (!ValidateParentClass(ManagedClass)) {Script->AssertScriptParentClass(); return NULL;}
		//
		MonoObject* OBJ = MonoCore.mono_create_object(MonoCore.GetPlayDomain(),ManagedClass);
		if (OBJ==nullptr) {LOG::CS_STR(ESeverity::Error,FString::Printf(TEXT("Failed to load instance of managed class:    %s"),*Script->GetScriptName())); return NULL;}
		//
		SetupManagedHandle(Node,OBJ);
		SetupManagedMethods(Node,OBJ);
		SetupThreadingAttribute(Node,ManagedClass);
		//
		Node->OBJHandle = mono_gchandle_new(OBJ,false);
		MonoCore.mono_register_object(Node);
		//
		mono_runtime_object_init(OBJ);
	} else {
		LOG::CS_STR(ESeverity::Error,FString::Printf(TEXT("Unable to locate dynamic library:  %s%s"),*Script->GetScriptName(),*EXT)); return NULL;
	}///
	//
	return Node;
}

void UMagicNodeSharp::PostLoadScript(UObject* Context, UMagicNodeSharp* Node) {
	if (Node==nullptr) {LOG::CSX_CHAR(ESeverity::Error,TEXT("Invalid Magic Node Object.")); return;}
	if (Context==nullptr) {LOG::CSX_STR(ESeverity::Error,FString::Printf(TEXT("Invalid World Context for Node:  %s"),*Node->GetName())); return;}
	//
	if (Node->HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) {return;}
	if (Node->IsPendingKill()) {return;}
	//
	Node->OnLoad();
}

void UMagicNodeSharp::Execute(UObject* Context, UMagicNodeSharp* Node) {
	if (Node==nullptr) {LOG::CSX_CHAR(ESeverity::Error,TEXT("Invalid Magic Node Object."));  return;}
	if (Context==nullptr) {LOG::CSX_STR(ESeverity::Error,FString::Printf(TEXT("Invalid World Context for Node:  %s"),*Node->GetName())); return;}
	//
	if (Node->HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) { return;}
	if (Node->IsPendingKill()) { return;}
	//
	if (Node->IsAsyncTask) {
		switch(Node->MonoThread) {
			case EMonoThread::MainThread:
			{
				Node->OnStart();
			} break;
			//
			case EMonoThread::OwnThread:
			{
				const UPTRINT IntPtr = reinterpret_cast<UPTRINT>(Node);
				Async(EAsyncExecution::Thread,[&IntPtr](){IMonoObject::ThreadStart(IntPtr);});
			} break;
		default: break;}
	} else {Node->OnExecute();}
}

void UMagicNodeSharp::Exit(UObject* Context, UMagicNodeSharp* Node) {
	if (Node==nullptr) {LOG::CSX_CHAR(ESeverity::Error,TEXT("Invalid Magic Node Object.")); return;}
	if (Context==nullptr) {LOG::CSX_STR(ESeverity::Error,FString::Printf(TEXT("Invalid World Context for Node:  %s"),*Node->GetName())); return;}
	//
	if (Node->HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) {return;}
	if (Node->IsPendingKill()) {return;}
	//
	if (!Node->IsAsyncTask) {Node->ExitMonoPlay();}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::OnLoad() {
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) {return;}
	//
	MonoObject* Self = GetMonoObject();
	if (MonoOnLoad==nullptr||Self==nullptr) {return;}
	//
	mono_runtime_invoke(MonoOnLoad,Self,NULL,&Except);
	//
	if (Except) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateUObject(this,&UMagicNodeSharp::Throw_MonoException),
			GetStatId(), nullptr, ENamedThreads::GameThread
		);//
	}///
}

void UMagicNodeSharp::OnExecute() {
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) {return;}
	//
	MonoObject* Self = GetMonoObject();
	if (MonoOnExecute==nullptr||Self==nullptr) {return;}
	//
	mono_runtime_invoke(MonoOnExecute,Self,NULL,&Except);
	//
	if (Except) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateUObject(this,&UMagicNodeSharp::Throw_MonoException),
			GetStatId(), nullptr, ENamedThreads::GameThread
		);//
	}///
}

void UMagicNodeSharp::OnStart() {
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) {return;}
	//
	MonoObject* This = GetMonoObject();
	if (MonoOnStart==nullptr||This==nullptr) {return;}
	//
	mono_runtime_invoke(MonoOnStart,This,NULL,&Except);
	//
	if (Except) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateUObject(this,&UMagicNodeSharp::Throw_MonoException),
			GetStatId(), nullptr, ENamedThreads::GameThread
		); IsAsyncTask = false;
	}///
	//
	switch(MonoThread) {
		case EMonoThread::OwnThread:
		{
			auto* Self = const_cast<UMagicNodeSharp*>(this);
			const UPTRINT IntPtr = reinterpret_cast<UPTRINT>(Self);
			//
			IMonoObject::ThreadUpdate(IntPtr);
		} break;
	default: break;}
}

int32 UMagicNodeSharp::OnUpdate(float DeltaTime) {
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) {return 0;}
	//
	MonoObject* Self = GetMonoObject();
	if (MonoOnUpdate==nullptr||Self==nullptr||!IsAsyncTask) {return 0;}
	//
	void* Args[1]; Args[0] = &DeltaTime;
	Update = mono_runtime_invoke(MonoOnUpdate,Self,Args,&Except);
	const int32 Result = (Update) ? *(int32*)mono_object_unbox(Update) : 1;
	//
	if (Except) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateUObject(this,&UMagicNodeSharp::Throw_MonoException),
			GetStatId(), nullptr, ENamedThreads::GameThread
		); IsAsyncTask = false;
	}///
	//
	return Result;
}

void UMagicNodeSharp::OnExit() {
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) {return;}
	//
	MonoObject* Self = GetMonoObject();
	if (MonoOnExit==nullptr||Self==nullptr) {return;}
	//
	mono_runtime_invoke(MonoOnExit,Self,NULL,&Except);
	//
	if (Except) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateUObject(this,&UMagicNodeSharp::Throw_MonoException),
			GetStatId(), nullptr, ENamedThreads::GameThread
		); IsAsyncTask = false;
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::ExitMonoPlay() {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject|RF_ClassDefaultObject|RF_BeginDestroyed)) {return;}
	if (IsPendingKill()) {return;} //MonoCore.mono_unregister_object(this);
	//
	if (IsAsyncTask) {
		switch(MonoThread) {
			case EMonoThread::MainThread:
			{
				OnExit();
			} break;
			//
			case EMonoThread::OwnThread:
			{
				auto* Self = const_cast<UMagicNodeSharp*>(this);
				const UPTRINT IntPtr = reinterpret_cast<UPTRINT>(Self);
				//
				IMonoObject::ThreadExit(IntPtr);
			} break;
		default: break;}
	} else {OnExit();}
	//
	IsAsyncTask = false;
	///ConditionalBeginDestroy();
}

void UMagicNodeSharp::CleanupOnQuit() {
	IsAsyncTask = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UWorld* UMagicNodeSharp::GetWorld() const {
	return World;
}

UWorld* UMagicNodeSharp::GetTickableGameObjectWorld() const {
	return (IsTickable()) ? World : nullptr;
}

TStatId UMagicNodeSharp::GetStatId() const {
	return GetStatID(false);
}

FGuid UMagicNodeSharp::GetNodeID() const {
	return PtrToGuid(const_cast<UMagicNodeSharp*>(this));
}

bool UMagicNodeSharp::IsTickable() const {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (World==nullptr) {return false;}
	if (!World->IsGameWorld()) {return false;}
	//
	return (IsAsyncTask && MonoCore.IsPlaying());
}

void UMagicNodeSharp::Register(UWorld* InWorld) {
	World = InWorld;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MonoObject* UMagicNodeSharp::GetMonoObject() const {
	if (OBJHandle==0) {return nullptr;}
	//
	if (MonoObject*OBJ=mono_gchandle_get_target(OBJHandle)) {
		return OBJ;
	} return NULL;
}

MonoClass* UMagicNodeSharp::GetMonoClass() const {
	if (MonoObject*OBJ=GetMonoObject()) {
		return mono_object_get_class(OBJ);
	} return NULL;
}

MonoProperty* UMagicNodeSharp::GetMonoProperty(const FName &Name) {
	if (MonoClass*ManagedClass=GetMonoClass()) {
		while(ManagedClass!=nullptr) {
			void* PItr=0; MonoProperty* Property = nullptr;
			while((Property=mono_class_get_properties(ManagedClass,&PItr)) != NULL) {
				const char* PropertyName = mono_property_get_name(Property);
				const FString PropName(StringCast<TCHAR>(PropertyName).Get());
				//
				if (Name.ToString().Equals(PropName,ESearchCase::CaseSensitive)) {
					return Property;
				}///
			}///
			//
			ManagedClass = mono_class_get_parent(ManagedClass);
			if (ManagedClass==mono_get_object_class()) {break;}
			PItr=0;
		}///
	}///
	//
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::Tick(float DeltaTime) {
	if (!IsTickable()) {return;}
	//
	SCOPE_CYCLE_COUNTER(STAT_FMonoMethod_AsyncTask);
	//
	switch(MonoThread) {
		case EMonoThread::MainThread:
		{
			int32 Result = OnUpdate(DeltaTime);
			if (Result!=0) {Throw_MonoError(TEXT("C# Runtime exception ::    OnUpdate()")); IsAsyncTask=false;}
		} break;
	default: break;}
}

void UMagicNodeSharp::BeginDestroy() {
	if (OBJHandle !=0 ) {
		mono_gchandle_free(OBJHandle);
	}///
	//
	Super::BeginDestroy();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UMagicNodeSharpSource::UMagicNodeSharpSource() {
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject)) {return;}
	//
#if WITH_EDITOR
	static FDirectoryWatcherModule &DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	WatcherHandle = FDelegateHandle();
	IsSaving = false;
	//
	DirectoryWatcherModule.Get()->RegisterDirectoryChangedCallback_Handle(
		CS_SCRIPT_DIR,IDirectoryWatcher::FDirectoryChanged::CreateUObject(
			this, &UMagicNodeSharpSource::OnProjectDirectoryChanged
		), WatcherHandle, IDirectoryWatcher::WatchOptions::IncludeDirectoryChanges
	);//
	//
	FEditorDelegates::OnAssetsPreDelete.AddUObject(this,&UMagicNodeSharpSource::OnAssetPreDeleted);
	//
	if (Source.IsEmpty()) {
	Source = FString::Printf(TEXT("\n\
using System;\n\
using Unreal.Core;\n\
\n\
namespace Unreal\n\
{\n\
	public class %s : Node\n\
	{\n\
		private string _Hello = \"Hello World!\";\n\
\n\
		public Strand Hello { get{return _Hello;} set{_Hello=value;} }\n\
\n\
		public void OnExecute()\n\
		{\n\
			Debug.Log(\"OnExecute()::   C# member function called!\");\n\
			Debug.Log(Hello);\n\
		}\n\
	}\n\
}\n"),*GetScriptName());}
#endif
}

UMagicNodeSharpSource::~UMagicNodeSharpSource() {
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject)) {return;}
	//
#if WITH_EDITOR
	static FDirectoryWatcherModule &DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	DirectoryWatcherModule.Get()->UnregisterDirectoryChangedCallback_Handle(CS_SCRIPT_DIR,WatcherHandle);
	//
	FEditorDelegates::OnAssetsPreDelete.RemoveAll(this);
#endif
}

void UMagicNodeSharpSource::PostLoad() {
	Super::PostLoad();
	//
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject)){return;}
#if WITH_EDITOR
	LoadScript();
#endif
}

void UMagicNodeSharpSource::PostInitProperties() {
	Super::PostInitProperties();
	//
	if (HasAnyFlags(RF_Transient|RF_ArchetypeObject)) {return;}
#if WITH_EDITOR
	if (Source.IsEmpty()) {
		Source = TEXT("");
	}///
	//
	LoadScript();
#endif
	//
#if !WITH_EDITOR
	static IPlatformFile &PlatformManager = FPlatformFileManager::Get().GetPlatformFile();
	//
	FString LIB = GetScriptName()+EXT;
	FString DLL = FPaths::Combine(*CS_BINARY_DIR,*LIB);
	//
	checkCode(
		if (!PlatformManager.FileExists(*DLL)) {
			LOG::CS_STR(ESeverity::Fatal,FString(TEXT("(C#) Script Assembly missing:    "))+LIB);
		}///
	);///
#endif
}

void UMagicNodeSharpSource::Serialize(FArchive &AR) {
	Super::Serialize(AR);
	//
#if WITH_EDITOR
	if (AR.IsSaving()) {
		ExportIntermediateData();
		DataInitialized=false;
		ExportFile();
	} else if (AR.IsLoading()) {
		LoadIntermediateData();
	}///
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FString UMagicNodeSharpSource::GetScriptName() const {
	FString File = GetName();
	//
	File.ReplaceInline(TEXT("Default__"),TEXT(""),ESearchCase::CaseSensitive);
	File.Split(TEXT("_C"),&File,nullptr,ESearchCase::CaseSensitive,ESearchDir::FromEnd);
	File.Split(TEXT("_GEN_"),&File,nullptr,ESearchCase::CaseSensitive,ESearchDir::FromEnd);
	//
	return File;
}///

void UMagicNodeSharpSource::AssertScriptClass() {
	LOG::CSX_STR(ESeverity::Error,FString::Printf(TEXT("Failed to load Class for script:  %s"),*GetScriptName()));
	LOG::CSX_STR(ESeverity::Warning,FString::Printf(TEXT("Class name should be '%s'!"),*GetScriptName()));
}

void UMagicNodeSharpSource::AssertScriptParentClass() {
	LOG::CSX_STR(ESeverity::Error,FString::Printf(TEXT("Failed to load Parent Class for script:  %s"),*GetScriptName()));
	LOG::CSX_STR(ESeverity::Warning,FString::Printf(TEXT("Script classes must be sub type of the '%s' class!"),TEXT(CS_NODE_CLASS)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if WITH_EDITOR

bool UMagicNodeSharpSource::LoadScript() {
	FString Chunk = UMagicNodeSharpSource::LoadFileToFString(*GetScriptFullPath());
	//
	if (!Chunk.IsEmpty()) {Source=Chunk;}
	//
	return !Source.IsEmpty();
}

bool UMagicNodeSharpSource::ExportFile() {
	if (HasAnyFlags(RF_ArchetypeObject)) {return false;}
	//
	IsSaving = true;
	bool Exported = FFileHelper::SaveStringToFile(Source,*GetScriptFullPath(),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	//
	if (Exported) {OnScriptSourceExported.Broadcast(this);} return Exported;
}

bool UMagicNodeSharpSource::DestroyScript() {
	FString File = GetScriptFullPath();
	//
	bool Deleted = IFileManager::Get().Delete(*File);
	//
	if (Deleted) {
		OnScriptSourceDeleted.Broadcast();
	} return Deleted;
}

FString UMagicNodeSharpSource::GetSource() const {
	return Source;
}

FString UMagicNodeSharpSource::GetScriptFullPath() const {
	return FPaths::Combine(CS_SCRIPT_DIR,GetScriptName()+CS_SCRIPT_EXT);
}

FString UMagicNodeSharpSource::LoadFileToFString(const TCHAR* FullPath) {
	FString Chunk;
	//
	FArchive* Reader = IFileManager::Get().CreateFileReader(FullPath,FILEREAD_AllowWrite);
	if (Reader) {
		int32 Size = Reader->TotalSize();
		//
		uint8* Ch = (uint8*)FMemory::Malloc(Size);
		Reader->Serialize(Ch,Size);
		//
		FFileHelper::BufferToString(Chunk,Ch,Size);
		Reader->Close(); FMemory::Free(Ch);
	} Reader = nullptr;
	//
	return Chunk;
}

FString UMagicNodeSharpSource::GetDataFullPath() const {
	return FPaths::Combine(CS_DATA_DIR,GetScriptName()+CS_DATA_EXT);
}

const FMonoScriptData &UMagicNodeSharpSource::GetScriptData() const {
	return ScriptData;
}

const FMonoClassDefinition &UMagicNodeSharpSource::GetClassDefinition() const {
	return CompiledClass;
}

bool UMagicNodeSharpSource::ExportIntermediateData() {
	if (HasAnyFlags(RF_ArchetypeObject)) {return false;}
	//
	FString Data;
	FJsonObjectConverter::UStructToJsonObjectString<FMonoScriptData>(ScriptData,Data,0,0);
	//
	return FFileHelper::SaveStringToFile(Data,*GetDataFullPath(),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

bool UMagicNodeSharpSource::WriteIntermediateData(const FMonoScriptData &CompilerData) {
	ScriptData = FMonoScriptData(CompilerData);
	//
	return ExportIntermediateData();
}

#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if WITH_EDITOR

void UMagicNodeSharpSource::OnProjectDirectoryChanged(const TArray<FFileChangeData>&Data) {
	if (!IsSaving) {
		LoadScript();
	} else {IsSaving=false;}
}

void UMagicNodeSharpSource::OnAssetPreDeleted(const TArray<UObject*>&Objects) {
	for (UObject* OBJ : Objects) {
		if (Cast<UMagicNodeSharpSource>(OBJ)==this) {DestroyScript();}
	}///
}

void UMagicNodeSharpSource::LoadIntermediateData() {
	if (DataInitialized) {return;}
	//
	FString File = GetDataFullPath();
	const FString Chunk = UMagicNodeSharpSource::LoadFileToFString(*File);
	//
	if (!Chunk.IsEmpty()) {
		DataInitialized = FJsonObjectConverter::JsonObjectStringToUStruct<FMonoScriptData>(Chunk,&ScriptData,0,0);
	}///
}

void UMagicNodeSharpSource::SetSource(const FString &NewText) {
	Source = NewText;
}

#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::SetMonoProperty_Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, bool Input) {
	SET_MonoPropertyValue_Bool(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, bool &Output) {
	GET_MonoPropertyValue_Bool(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, uint8 Input) {
	SET_MonoPropertyValue_Byte(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, uint8 &Output) {
	GET_MonoPropertyValue_Byte(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, int32 Input) {
	SET_MonoPropertyValue_Int(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, int32 &Output) {
	GET_MonoPropertyValue_Int(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, int64 Input) {
	SET_MonoPropertyValue_Int64(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, int64 &Output) {
	GET_MonoPropertyValue_Int64(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, float Input) {
	SET_MonoPropertyValue_Float(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, float &Output) {
	GET_MonoPropertyValue_Float(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FString Input) {
	SET_MonoPropertyValue_String(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, FString &Output) {
	GET_MonoPropertyValue_String(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FName Input) {
	SET_MonoPropertyValue_Name(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, FName &Output) {
	GET_MonoPropertyValue_Name(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FText Input) {
	SET_MonoPropertyValue_Text(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, FText &Output) {
	GET_MonoPropertyValue_Text(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FVector2D Input) {
	SET_MonoPropertyValue_Vector2D(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, FVector2D &Output) {
	GET_MonoPropertyValue_Vector2D(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FVector Input) {
	SET_MonoPropertyValue_Vector3D(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, FVector &Output) {
	GET_MonoPropertyValue_Vector3D(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FRotator Input) {
	SET_MonoPropertyValue_Rotator(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, FRotator &Output) {
	GET_MonoPropertyValue_Rotator(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FColor Input) {
	SET_MonoPropertyValue_Color(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, FColor &Output) {
	GET_MonoPropertyValue_Color(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FTransform Input) {
	SET_MonoPropertyValue_Transform(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field, FTransform &Output) {
	GET_MonoPropertyValue_Transform(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, UClass* Input) {
	SET_MonoPropertyValue_Class(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, UClass* &Output) {
	GET_MonoPropertyValue_Class(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, UObject* Input) {
	SET_MonoPropertyValue_Object(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, UObject* &Output) {
	GET_MonoPropertyValue_Object(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, AActor* Input) {
	SET_MonoPropertyValue_Actor(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, AActor* &Output) {
	GET_MonoPropertyValue_Actor(Node,Field,Output);
}

void UMagicNodeSharp::SetMonoProperty_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, UActorComponent* Input) {
	SET_MonoPropertyValue_Component(Node,Field,Input);
}

void UMagicNodeSharp::GetMonoProperty_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, UActorComponent* &Output) {
	GET_MonoPropertyValue_Component(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoArray_Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<bool>&Output) {
	GET_MonoArrayValue_Bool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<uint8>&Output) {
	GET_MonoArrayValue_Byte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<int32>&Output) {
	GET_MonoArrayValue_Int(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<int64>&Output) {
	GET_MonoArrayValue_Int64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<float>&Output) {
	GET_MonoArrayValue_Float(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FString>&Output) {
	GET_MonoArrayValue_String(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FName>&Output) {
	GET_MonoArrayValue_Name(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FText>&Output) {
	GET_MonoArrayValue_Text(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FVector2D>&Output) {
	GET_MonoArrayValue_Vector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FVector>&Output) {
	GET_MonoArrayValue_Vector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FRotator>&Output) {
	GET_MonoArrayValue_Rotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FColor>&Output) {
	GET_MonoArrayValue_Color(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FTransform>&Output) {
	GET_MonoArrayValue_Transform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<UClass*>&Output) {
	GET_MonoArrayValue_Class(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<UObject*>&Output) {
	GET_MonoArrayValue_Object(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<AActor*>&Output) {
	GET_MonoArrayValue_Actor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoArray_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<UActorComponent*>&Output) {
	GET_MonoArrayValue_Component(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void UMagicNodeSharp::GetMonoSet_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<uint8>&Output) {
	GET_MonoSetValue_Byte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<int32>&Output) {
	GET_MonoSetValue_Int(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<int64>&Output) {
	GET_MonoSetValue_Int64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<float>&Output) {
	GET_MonoSetValue_Float(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FString>&Output) {
	GET_MonoSetValue_String(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FName>&Output) {
	GET_MonoSetValue_Name(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FVector2D>&Output) {
	GET_MonoSetValue_Vector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FVector>&Output) {
	GET_MonoSetValue_Vector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FColor>&Output) {
	GET_MonoSetValue_Color(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<UClass*>&Output) {
	GET_MonoSetValue_Class(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<UObject*>&Output) {
	GET_MonoSetValue_Object(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<AActor*>&Output) {
	GET_MonoSetValue_Actor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoSet_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<UActorComponent*>&Output) {
	GET_MonoSetValue_Component(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_ByteBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,bool>&Output) {
	GET_MonoMapValue_ByteBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,uint8>&Output) {
	GET_MonoMapValue_ByteByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,int32>&Output) {
	GET_MonoMapValue_ByteInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,int64>&Output) {
	GET_MonoMapValue_ByteInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,float>&Output) {
	GET_MonoMapValue_ByteFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FString>&Output) {
	GET_MonoMapValue_ByteString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FName>&Output) {
	GET_MonoMapValue_ByteName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FText>&Output) {
	GET_MonoMapValue_ByteText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FColor>&Output) {
	GET_MonoMapValue_ByteColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FVector2D>&Output) {
	GET_MonoMapValue_ByteVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FVector>&Output) {
	GET_MonoMapValue_ByteVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FRotator>&Output) {
	GET_MonoMapValue_ByteRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FTransform>&Output) {
	GET_MonoMapValue_ByteTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,UClass*>&Output) {
	GET_MonoMapValue_ByteClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,UObject*>&Output) {
	GET_MonoMapValue_ByteObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,AActor*>&Output) {
	GET_MonoMapValue_ByteActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ByteComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,UActorComponent*>&Output) {
	GET_MonoMapValue_ByteComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_IntBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,bool>&Output) {
	GET_MonoMapValue_IntBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,uint8>&Output) {
	GET_MonoMapValue_IntByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,int32>&Output) {
	GET_MonoMapValue_IntInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,int64>&Output) {
	GET_MonoMapValue_IntInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,float>&Output) {
	GET_MonoMapValue_IntFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FString>&Output) {
	GET_MonoMapValue_IntString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FName>&Output) {
	GET_MonoMapValue_IntName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FText>&Output) {
	GET_MonoMapValue_IntText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FColor>&Output) {
	GET_MonoMapValue_IntColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FVector2D>&Output) {
	GET_MonoMapValue_IntVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FVector>&Output) {
	GET_MonoMapValue_IntVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FRotator>&Output) {
	GET_MonoMapValue_IntRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FTransform>&Output) {
	GET_MonoMapValue_IntTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,UClass*>&Output) {
	GET_MonoMapValue_IntClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,UObject*>&Output) {
	GET_MonoMapValue_IntObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,AActor*>&Output) {
	GET_MonoMapValue_IntActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_IntComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,UActorComponent*>&Output) {
	GET_MonoMapValue_IntComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_Int64Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,bool>&Output) {
	GET_MonoMapValue_Int64Bool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,uint8>&Output) {
	GET_MonoMapValue_Int64Byte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,int32>&Output) {
	GET_MonoMapValue_Int64Int(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,int64>&Output) {
	GET_MonoMapValue_Int64Int64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,float>&Output) {
	GET_MonoMapValue_Int64Float(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64String(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FString>&Output) {
	GET_MonoMapValue_Int64String(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FName>&Output) {
	GET_MonoMapValue_Int64Name(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FText>&Output) {
	GET_MonoMapValue_Int64Text(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FColor>&Output) {
	GET_MonoMapValue_Int64Color(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FVector2D>&Output) {
	GET_MonoMapValue_Int64Vector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FVector>&Output) {
	GET_MonoMapValue_Int64Vector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FRotator>&Output) {
	GET_MonoMapValue_Int64Rotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<int64,FTransform>&Output) {
	GET_MonoMapValue_Int64Transform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,UClass*>&Output) {
	GET_MonoMapValue_Int64Class(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,UObject*>&Output) {
	GET_MonoMapValue_Int64Object(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,AActor*>&Output) {
	GET_MonoMapValue_Int64Actor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Int64Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,UActorComponent*>&Output) {
	GET_MonoMapValue_Int64Component(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_FloatBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,bool>&Output) {
	GET_MonoMapValue_FloatBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,uint8>&Output) {
	GET_MonoMapValue_FloatByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,int32>&Output) {
	GET_MonoMapValue_FloatInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,int64>&Output) {
	GET_MonoMapValue_FloatInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,float>&Output) {
	GET_MonoMapValue_FloatFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FString>&Output) {
	GET_MonoMapValue_FloatString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FName>&Output) {
	GET_MonoMapValue_FloatName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FText>&Output) {
	GET_MonoMapValue_FloatText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FColor>&Output) {
	GET_MonoMapValue_FloatColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FVector2D>&Output) {
	GET_MonoMapValue_FloatVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FVector>&Output) {
	GET_MonoMapValue_FloatVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FRotator>&Output) {
	GET_MonoMapValue_FloatRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<float,FTransform>&Output) {
	GET_MonoMapValue_FloatTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,UClass*>&Output) {
	GET_MonoMapValue_FloatClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,UObject*>&Output) {
	GET_MonoMapValue_FloatObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,AActor*>&Output) {
	GET_MonoMapValue_FloatActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_FloatComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,UActorComponent*>&Output) {
	GET_MonoMapValue_FloatComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_StringBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,bool>&Output) {
	GET_MonoMapValue_StringBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,uint8>&Output) {
	GET_MonoMapValue_StringByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,int32>&Output) {
	GET_MonoMapValue_StringInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,int64>&Output) {
	GET_MonoMapValue_StringInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,float>&Output) {
	GET_MonoMapValue_StringFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FString>&Output) {
	GET_MonoMapValue_StringString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FName>&Output) {
	GET_MonoMapValue_StringName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FText>&Output) {
	GET_MonoMapValue_StringText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FColor>&Output) {
	GET_MonoMapValue_StringColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FVector2D>&Output) {
	GET_MonoMapValue_StringVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FVector>&Output) {
	GET_MonoMapValue_StringVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FRotator>&Output) {
	GET_MonoMapValue_StringRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<FString,FTransform>&Output) {
	GET_MonoMapValue_StringTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,UClass*>&Output) {
	GET_MonoMapValue_StringClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,UObject*>&Output) {
	GET_MonoMapValue_StringObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,AActor*>&Output) {
	GET_MonoMapValue_StringActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_StringComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,UActorComponent*>&Output) {
	GET_MonoMapValue_StringComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_NameBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,bool>&Output) {
	GET_MonoMapValue_NameBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,uint8>&Output) {
	GET_MonoMapValue_NameByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,int32>&Output) {
	GET_MonoMapValue_NameInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,int64>&Output) {
	GET_MonoMapValue_NameInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,float>&Output) {
	GET_MonoMapValue_NameFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FString>&Output) {
	GET_MonoMapValue_NameString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FName>&Output) {
	GET_MonoMapValue_NameName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FText>&Output) {
	GET_MonoMapValue_NameText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FColor>&Output) {
	GET_MonoMapValue_NameColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FVector2D>&Output) {
	GET_MonoMapValue_NameVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FVector>&Output) {
	GET_MonoMapValue_NameVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FRotator>&Output) {
	GET_MonoMapValue_NameRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<FName,FTransform>&Output) {
	GET_MonoMapValue_NameTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,UClass*>&Output) {
	GET_MonoMapValue_NameClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,UObject*>&Output) {
	GET_MonoMapValue_NameObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,AActor*>&Output) {
	GET_MonoMapValue_NameActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_NameComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,UActorComponent*>&Output) {
	GET_MonoMapValue_NameComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_Vector2DBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,bool>&Output) {
	GET_MonoMapValue_Vector2DBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,uint8>&Output) {
	GET_MonoMapValue_Vector2DByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,int32>&Output) {
	GET_MonoMapValue_Vector2DInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,int64>&Output) {
	GET_MonoMapValue_Vector2DInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,float>&Output) {
	GET_MonoMapValue_Vector2DFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FString>&Output) {
	GET_MonoMapValue_Vector2DString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FName>&Output) {
	GET_MonoMapValue_Vector2DName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FText>&Output) {
	GET_MonoMapValue_Vector2DText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FColor>&Output) {
	GET_MonoMapValue_Vector2DColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FVector2D>&Output) {
	GET_MonoMapValue_Vector2DVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FVector>&Output) {
	GET_MonoMapValue_Vector2DVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FRotator>&Output) {
	GET_MonoMapValue_Vector2DRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<FVector2D,FTransform>&Output) {
	GET_MonoMapValue_Vector2DTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,UClass*>&Output) {
	GET_MonoMapValue_Vector2DClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,UObject*>&Output) {
	GET_MonoMapValue_Vector2DObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,AActor*>&Output) {
	GET_MonoMapValue_Vector2DActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector2DComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,UActorComponent*>&Output) {
	GET_MonoMapValue_Vector2DComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_Vector3DBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,bool>&Output) {
	GET_MonoMapValue_Vector3DBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,uint8>&Output) {
	GET_MonoMapValue_Vector3DByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,int32>&Output) {
	GET_MonoMapValue_Vector3DInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,int64>&Output) {
	GET_MonoMapValue_Vector3DInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,float>&Output) {
	GET_MonoMapValue_Vector3DFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FString>&Output) {
	GET_MonoMapValue_Vector3DString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FName>&Output) {
	GET_MonoMapValue_Vector3DName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FText>&Output) {
	GET_MonoMapValue_Vector3DText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FColor>&Output) {
	GET_MonoMapValue_Vector3DColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FVector2D>&Output) {
	GET_MonoMapValue_Vector3DVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FVector>&Output) {
	GET_MonoMapValue_Vector3DVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FRotator>&Output) {
	GET_MonoMapValue_Vector3DRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<FVector,FTransform>&Output) {
	GET_MonoMapValue_Vector3DTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,UClass*>&Output) {
	GET_MonoMapValue_Vector3DClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,UObject*>&Output) {
	GET_MonoMapValue_Vector3DObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,AActor*>&Output) {
	GET_MonoMapValue_Vector3DActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_Vector3DComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,UActorComponent*>&Output) {
	GET_MonoMapValue_Vector3DComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_ColorBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,bool>&Output) {
	GET_MonoMapValue_ColorBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,uint8>&Output) {
	GET_MonoMapValue_ColorByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,int32>&Output) {
	GET_MonoMapValue_ColorInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,int64>&Output) {
	GET_MonoMapValue_ColorInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,float>&Output) {
	GET_MonoMapValue_ColorFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FString>&Output) {
	GET_MonoMapValue_ColorString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FName>&Output) {
	GET_MonoMapValue_ColorName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FText>&Output) {
	GET_MonoMapValue_ColorText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FColor>&Output) {
	GET_MonoMapValue_ColorColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FVector2D>&Output) {
	GET_MonoMapValue_ColorVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FVector>&Output) {
	GET_MonoMapValue_ColorVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FRotator>&Output) {
	GET_MonoMapValue_ColorRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<FColor,FTransform>&Output) {
	GET_MonoMapValue_ColorTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,UClass*>&Output) {
	GET_MonoMapValue_ColorClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,UObject*>&Output) {
	GET_MonoMapValue_ColorObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,AActor*>&Output) {
	GET_MonoMapValue_ColorActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ColorComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,UActorComponent*>&Output) {
	GET_MonoMapValue_ColorComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_ClassBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,bool>&Output) {
	GET_MonoMapValue_ClassBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,uint8>&Output) {
	GET_MonoMapValue_ClassByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,int32>&Output) {
	GET_MonoMapValue_ClassInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,int64>&Output) {
	GET_MonoMapValue_ClassInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,float>&Output) {
	GET_MonoMapValue_ClassFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FString>&Output) {
	GET_MonoMapValue_ClassString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FName>&Output) {
	GET_MonoMapValue_ClassName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FText>&Output) {
	GET_MonoMapValue_ClassText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FColor>&Output) {
	GET_MonoMapValue_ClassColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FVector2D>&Output) {
	GET_MonoMapValue_ClassVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FVector>&Output) {
	GET_MonoMapValue_ClassVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FRotator>&Output) {
	GET_MonoMapValue_ClassRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<UClass*,FTransform>&Output) {
	GET_MonoMapValue_ClassTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,UClass*>&Output) {
	GET_MonoMapValue_ClassClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,UObject*>&Output) {
	GET_MonoMapValue_ClassObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,AActor*>&Output) {
	GET_MonoMapValue_ClassActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ClassComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,UActorComponent*>&Output) {
	GET_MonoMapValue_ClassComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_ObjectBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,bool>&Output) {
	GET_MonoMapValue_ObjectBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,uint8>&Output) {
	GET_MonoMapValue_ObjectByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,int32>&Output) {
	GET_MonoMapValue_ObjectInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,int64>&Output) {
	GET_MonoMapValue_ObjectInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,float>&Output) {
	GET_MonoMapValue_ObjectFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FString>&Output) {
	GET_MonoMapValue_ObjectString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FName>&Output) {
	GET_MonoMapValue_ObjectName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FText>&Output) {
	GET_MonoMapValue_ObjectText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FColor>&Output) {
	GET_MonoMapValue_ObjectColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FVector2D>&Output) {
	GET_MonoMapValue_ObjectVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FVector>&Output) {
	GET_MonoMapValue_ObjectVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FRotator>&Output) {
	GET_MonoMapValue_ObjectRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<UObject*,FTransform>&Output) {
	GET_MonoMapValue_ObjectTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,UClass*>&Output) {
	GET_MonoMapValue_ObjectClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,UObject*>&Output) {
	GET_MonoMapValue_ObjectObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,AActor*>&Output) {
	GET_MonoMapValue_ObjectActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ObjectComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,UActorComponent*>&Output) {
	GET_MonoMapValue_ObjectComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_ActorBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,bool>&Output) {
	GET_MonoMapValue_ActorBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,uint8>&Output) {
	GET_MonoMapValue_ActorByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,int32>&Output) {
	GET_MonoMapValue_ActorInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,int64>&Output) {
	GET_MonoMapValue_ActorInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,float>&Output) {
	GET_MonoMapValue_ActorFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FString>&Output) {
	GET_MonoMapValue_ActorString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FName>&Output) {
	GET_MonoMapValue_ActorName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FText>&Output) {
	GET_MonoMapValue_ActorText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FColor>&Output) {
	GET_MonoMapValue_ActorColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FVector2D>&Output) {
	GET_MonoMapValue_ActorVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FVector>&Output) {
	GET_MonoMapValue_ActorVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FRotator>&Output) {
	GET_MonoMapValue_ActorRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<AActor*,FTransform>&Output) {
	GET_MonoMapValue_ActorTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,UClass*>&Output) {
	GET_MonoMapValue_ActorClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,UObject*>&Output) {
	GET_MonoMapValue_ActorObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,AActor*>&Output) {
	GET_MonoMapValue_ActorActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ActorComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,UActorComponent*>&Output) {
	GET_MonoMapValue_ActorComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UMagicNodeSharp::GetMonoMap_ComponentBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,bool>&Output) {
	GET_MonoMapValue_ComponentBool(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,uint8>&Output) {
	GET_MonoMapValue_ComponentByte(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,int32>&Output) {
	GET_MonoMapValue_ComponentInt(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,int64>&Output) {
	GET_MonoMapValue_ComponentInt64(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,float>&Output) {
	GET_MonoMapValue_ComponentFloat(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FString>&Output) {
	GET_MonoMapValue_ComponentString(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FName>&Output) {
	GET_MonoMapValue_ComponentName(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FText>&Output) {
	GET_MonoMapValue_ComponentText(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FColor>&Output) {
	GET_MonoMapValue_ComponentColor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FVector2D>&Output) {
	GET_MonoMapValue_ComponentVector2D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FVector>&Output) {
	GET_MonoMapValue_ComponentVector3D(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FRotator>&Output) {
	GET_MonoMapValue_ComponentRotator(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field,TMap<UActorComponent*,FTransform>&Output) {
	GET_MonoMapValue_ComponentTransform(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,UClass*>&Output) {
	GET_MonoMapValue_ComponentClass(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,UObject*>&Output) {
	GET_MonoMapValue_ComponentObject(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,AActor*>&Output) {
	GET_MonoMapValue_ComponentActor(Node,Field,Output);
}

void UMagicNodeSharp::GetMonoMap_ComponentComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,UActorComponent*>&Output) {
	GET_MonoMapValue_ComponentComponent(Node,Field,Output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////