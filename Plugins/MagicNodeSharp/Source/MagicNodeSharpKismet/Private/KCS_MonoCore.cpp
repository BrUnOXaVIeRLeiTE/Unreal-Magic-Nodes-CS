//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KCS_MonoCore.h"
#include "KCS_MonoAnalyzer.h"
#include "MCS_File.h"

#include "IMagicNodeSharp.h"
#include "IMagicNodeSharpKismet.h"
#include "MagicNodeSharpKismet_Shared.h"

#include "Runtime/CoreUObject/Public/UObject/SoftObjectPath.h"
#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"
#include "Runtime/Core/Public/Misc/CString.h"

#include "Modules/ModuleManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TMap<FName,FCompilerResults>IMonoKismet::RoslynErrorStack{};
TMap<FName,FCompilerResults>IMonoKismet::RoslynWarningStack{};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoKismet::CompileScript(UMagicNodeSharpSource* Source, UMagicNodeSharp* Node) {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	mono_thread_attach(MonoKismet.GetKismetDomain());
	MonoClass* CS_Compiler = MonoKismet.mono_find_class(MonoKismet.GetCompilerImage(),TEXT(CS_CORE_NAMESPACE),TEXT("NodeCompiler"));
	//
	FCompilerResults Results;
	if (CS_Compiler==nullptr) {
		Results.ErrorMessage = TEXT("{C#}:: Compiler service failed to initialize.");
		Results.Result = EMonoCompilerResult::Error;
		Results.ErrorInfo = FSourceInfo(0,0);
		//
		MonoKismet.CompilationFinished(Source,Results); return;
	}///
	//
	//
	LOG::CS_CHAR(ESeverity::Info,TEXT("C# Compiling Node..."));
	//
	if (MonoMethod*CSC=MonoKismet.mono_find_method(CS_Compiler,TEXT("CompileScript"))) {
		void* Args[4];
		{
			const FString MainPath = FPaths::ConvertRelativePathToFull(Source->GetScriptFullPath());
			const FString OutPath = FPaths::ConvertRelativePathToFull(CS_BINARY_DIR);
			//
			MonoString* LibOutPath = mono_string_new(MonoKismet.GetKismetDomain(),StringCast<ANSICHAR>(*OutPath).Get());
			MonoString* ScriptName = mono_string_new(MonoKismet.GetKismetDomain(),StringCast<ANSICHAR>(*Source->GetScriptName()).Get());
			//
			uintptr_t IncNum = Source->Include.Num();
			uintptr_t RefNum = Source->References.Num();
			MonoArray* References = mono_array_new(MonoKismet.GetKismetDomain(),mono_get_string_class(),RefNum);
			MonoArray* SourceFiles = mono_array_new(MonoKismet.GetKismetDomain(),mono_get_string_class(),IncNum+1);
			//
			MonoString* MainCS = mono_string_new(MonoKismet.GetKismetDomain(),StringCast<ANSICHAR>(*MainPath).Get());
			mono_array_set(SourceFiles,MonoString*,0,MainCS);
			//
			const auto &REF = Source->References;
			const auto &INC = Source->Include;
			//
			for (int32 I=0; I<IncNum; I++) {
				if (INC[I]==nullptr) {continue;}
				if (INC[I]==Source) {continue;}
				//
				const FString FilePath = FPaths::ConvertRelativePathToFull(INC[I]->GetScriptFullPath());
				MonoString* SourcePath = mono_string_new(MonoKismet.GetKismetDomain(),StringCast<ANSICHAR>(*FilePath).Get());
				//
				mono_array_set(SourceFiles,MonoString*,I+1,SourcePath);
			}///
			//
			for (int32 I=0; I<RefNum; I++) {
				FString Lib = REF[I].FilePath.TrimStartAndEnd().ToLower();
				//
				if (Lib.IsEmpty()) {continue;}
				if (!Lib.EndsWith(TEXT(".dll"))) {continue;}
				//
				MonoString* Ref = mono_string_new(MonoKismet.GetKismetDomain(),StringCast<ANSICHAR>(*Lib).Get());
				mono_array_set(References,MonoString*,I,Ref);
			}///
			//
			Args[0] = ScriptName; Args[1] = LibOutPath;
			Args[2] = SourceFiles; Args[3] = References;
		}///
		//
		MonoObject* RCall = mono_runtime_invoke(CSC,NULL,Args,NULL);
		const int32 Err = (RCall) ? *(int32*)mono_object_unbox(RCall) : 1;
		//
		if (Err!=0) {
			Results.Result = EMonoCompilerResult::Error;
			Results.ErrorMessage = FString::Printf(TEXT("Failed to compile script:  %s"),*Source->GetScriptName());
			MonoKismet.CompilationFinished(Source,Results); return;
		} else if (IMonoKismet::RoslynErrorStack.Num()>0) {
			if (IMonoKismet::RoslynErrorStack.Contains(*Source->GetScriptName())) {
				IMonoKismet::RoslynErrorStack.RemoveAndCopyValue(*Source->GetScriptName(),Results);
			} MonoKismet.CompilationFinished(Source,Results); return;
		} else if (IMonoKismet::RoslynWarningStack.Num()>0) {
			if (IMonoKismet::RoslynWarningStack.Contains(*Source->GetScriptName())) {
				IMonoKismet::RoslynWarningStack.RemoveAndCopyValue(*Source->GetScriptName(),Results);
			}///
		}///
	} else {
		Results.Result = EMonoCompilerResult::Error;
		Results.ErrorMessage = TEXT("Failed to load Compiler Service.");
		//
		MonoKismet.CompilationFinished(Source,Results); return;
	}///
	//
	MonoImageOpenStatus status; size_t length = 0;
	const FString FullPath = FPaths::Combine(*CS_BINARY_DIR,*Source->GetScriptName()+EXT);
	char* data = File::Read(StringCast<ANSICHAR>(*FullPath).Get(),&length);
	//
	MonoImage* NodeImage = mono_image_open_from_data_with_name(
		data, length,  true, &status,  false,
		StringCast<ANSICHAR>(*FullPath).Get()
	);///
	//
	if (status!=MONO_IMAGE_OK||NodeImage==nullptr) {
		Results.ErrorMessage = FString::Printf(TEXT("Error loading %s"),*(Source->GetScriptName()+EXT));
		Results.Result = EMonoCompilerResult::Error;
		//
		LOG::CS_STR(ESeverity::Error,Results.ErrorMessage);
		MonoKismet.CompilationFinished(Source,Results); return;
	}///
	//
	MonoAssembly* NodeAssembly = mono_assembly_load_from_full(NodeImage,StringCast<ANSICHAR>(*FullPath).Get(),&status,false);
	//
	if (status!=MONO_IMAGE_OK||NodeAssembly==nullptr) {
		Results.ErrorMessage = FString::Printf(TEXT("Error loading %s"),*(Source->GetScriptName()+EXT));
		Results.Result = EMonoCompilerResult::Error;
		//
		LOG::CS_STR(ESeverity::Error,Results.ErrorMessage);
		MonoKismet.CompilationFinished(Source,Results); return;
	}///
	//
	const char* ScriptName = StringCast<ANSICHAR>(*Source->GetScriptName()).Get();
	//
	if (NodeImage) {
		LOG::CS_STR(ESeverity::Info,FString::Printf(TEXT("Compiled Node Image loaded:  %s"),*Source->GetScriptName()));
		MonoClass* ManagedClass = mono_class_from_name_case(NodeImage,CS_CORE_NAMESPACE,ScriptName);
		//
		if (ManagedClass==nullptr) {
			const MonoTableInfo* TableInfo = mono_image_get_table_info(NodeImage,MONO_TABLE_TYPEDEF);
			int Rows = mono_table_info_get_rows(TableInfo);
			//
			for (int I=0; I<Rows; I++) {
				uint32_t Res[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(TableInfo,I,Res,MONO_TYPEDEF_SIZE);
				const char* ClassName = mono_metadata_string_heap(NodeImage,Res[MONO_TYPEDEF_NAME]);
				const char* NameSpace = mono_metadata_string_heap(NodeImage,Res[MONO_TYPEDEF_NAMESPACE]);
				//
				if (strcmp(ClassName,ScriptName)==0) {
					ManagedClass = mono_class_from_name(NodeImage,NameSpace,ClassName); break;
				}///
			}///
		}///
		//
		if (ManagedClass==nullptr) {
			Source->AssertScriptClass();
			Results.Result = EMonoCompilerResult::Error;
			Results.ErrorMessage = FString::Printf(TEXT("Invalid Generated Class:  %s"),*Source->GetScriptName());
			//
			MonoKismet.CompilationFinished(Source,Results); return;
		}///
		//
		if (!IMonoObject::ValidateParentClass(ManagedClass)) {
			Source->AssertScriptParentClass();
			//
			Results.Result = EMonoCompilerResult::Error;
			Results.ErrorMessage = FString::Printf(TEXT("Invalid Parent Class for Node:  %s"),*Source->GetScriptName());
			//
			MonoKismet.CompilationFinished(Source,Results); return;
		}///
		//
		MonoObject* OBJ = MonoKismet.mono_create_object(MonoKismet.GetKismetDomain(),ManagedClass);
		//
		if (OBJ==nullptr) {
			Results.Result = EMonoCompilerResult::Error;
			Results.ErrorMessage = FString::Printf(TEXT("Failed to instantiate managed class:    %s"),*Source->GetScriptName());
			//
			MonoKismet.CompilationFinished(Source,Results); return;
		}///
		//
		IMonoObject::SetupManagedHandle(Node,OBJ);
		IMonoObject::SetupManagedMethods(Node,OBJ);
		MonoKismet.SetupThreadingAttribute(Node,ManagedClass);
		//
		Node->Source = Source;
		Node->OBJHandle = mono_gchandle_new(OBJ,false);
		//
		Results.ErrorMessage = TEXT("OK");
		Results.Result = EMonoCompilerResult::Success;
	} else {
		Results.Result = EMonoCompilerResult::Error;
		Results.ErrorMessage = FString::Printf(TEXT("Unable to locate dynamic library:  %s%s"),*Source->GetScriptName(),*EXT);
		//
		MonoKismet.CompilationFinished(Source,Results); return;
	}///
	//
	if (MonoObject*MDO=Node->GetMonoObject()) {
		mono_runtime_object_init(MDO);
		//
		FMonoScriptData ClassData;
		ClassData.Asynchronous = Node->IsAsyncTask;
		ClassData.PropertyInfo = MonoKismet.CollectClassPropertyInfo(MDO);
		FMonoClassDefinition ClassDefinition = MonoKismet.CollectClassInfo(Source,MDO);
		//
		Source->Modify();
		Source->CompiledClass = ClassDefinition;
		Source->WriteIntermediateData(ClassData);
	} else {
		Results.Result = EMonoCompilerResult::Error;
		Results.ErrorMessage = FString::Printf(TEXT("Unable to allocate managed object for script:  %s"),*Source->GetScriptName());
	}///
	//
	//
	mono_image_close(NodeImage);
	MonoKismet.CompilationFinished(Source,Results);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoKismet::MonoKismet_INIT_BASE() {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	CodeEditorUtility = MonoCore.mono_find_class(MonoCore.GetEditorImage(),TEXT(CS_CORE_NAMESPACE),TEXT("NodeEditor"));
	CodeFormatMethod = MonoCore.mono_find_method(CodeEditorUtility,TEXT("FormatScript"));
	//
	FCriticalSection Mutex;
	{
		Mutex.Lock();
		//
		IKCS_MonoAnalyzer::CacheNamespaces(mono_get_corlib());
		IKCS_MonoAnalyzer::CacheNamespaces(MonoCore.GetEditorCoreImage());
		IKCS_MonoAnalyzer::CacheNamespaces(MonoCore.GetEditorUnrealImage());
		IKCS_MonoAnalyzer::CacheNamespaces(MonoCore.GetEditorSystemImage());
		//
		IKCS_MonoAnalyzer::CacheBaseClasses();
		IKCS_MonoAnalyzer::CacheClassesFromImage(mono_get_corlib());
		IKCS_MonoAnalyzer::CacheClassesFromImage(MonoCore.GetEditorCoreImage());
		IKCS_MonoAnalyzer::CacheClassesFromImage(MonoCore.GetEditorUnrealImage());
		IKCS_MonoAnalyzer::CacheClassesFromImage(MonoCore.GetEditorSystemImage());
		//
		Mutex.Unlock();
	}
}

bool IMonoKismet::MonoKismet_INIT() {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	#ifdef MCS_MARKETPLACE_BUILD
	 static FString DIRxMND = FPaths::Combine(*FPaths::EnginePluginsDir(),TEXT("Marketplace"),CS_PLUGIN_NAME,TEXT("Binaries"));
	 static FString DIRxCFG = FPaths::Combine(*FPaths::EnginePluginsDir(),TEXT("Marketplace"),CS_PLUGIN_NAME,TEXT("Binaries"),TEXT("Editor"));
	#else
	 static FString DIRxMND = FPaths::Combine(*FPaths::ProjectPluginsDir(),CS_PLUGIN_NAME,TEXT("Binaries"));
	 static FString DIRxCFG = FPaths::Combine(*FPaths::ProjectPluginsDir(),CS_PLUGIN_NAME,TEXT("Binaries"),TEXT("Editor"));
	#endif
	//
	static FString DIRxNCP = FPaths::Combine(*DIRxCFG,TEXT("NodeCompiler.dll"));
	static FString DIRxDOM = FPaths::Combine(*DIRxMND,TEXT("MagicNode.dll"));
	//
	if (!CanCompile()) {
		LOG::CS_CHAR(ESeverity::Warning,TEXT("C# cannot initialize a Compiler while the runtime is busy.")); return false;
	} if (MonoCore.IsPlaying()) {LOG::CS_CHAR(ESeverity::Error,TEXT("Error initializing Compiler Domain.")); return false;}
	//
	KismetDomain = mono_domain_create_appdomain(CS_KISMET_DOMAIN,NULL);
	mono_domain_set(KismetDomain,0);
	//
	{
		MonoImageOpenStatus status; size_t length = 0;
		char* data = File::Read(StringCast<ANSICHAR>(*DIRxDOM).Get(),&length);
		//
		CoreImage = mono_image_open_from_data_with_name(
			data, length, true, &status,  false,
			StringCast<ANSICHAR>(*DIRxDOM).Get()
		);///
		//
		if (status!=MONO_IMAGE_OK||CoreImage==nullptr) {
			LOG::CS_CHAR(ESeverity::Fatal,TEXT("Error loading MagicNode.dll"));
		}///
		//
		MonoAssembly* CoreAssembly = mono_assembly_load_from_full(CoreImage,StringCast<ANSICHAR>(*DIRxDOM).Get(),&status,false);
		//
		if (status!=MONO_IMAGE_OK||CoreAssembly==nullptr) {
			mono_image_close(CoreImage); LOG::CS_CHAR(ESeverity::Fatal,TEXT("MagicNode.dll not found."));
		}///
	}
	//
	{
		MonoImageOpenStatus status; size_t length = 0;
		char* data = File::Read(StringCast<ANSICHAR>(*DIRxNCP).Get(),&length);
		//
		CompilerImage = mono_image_open_from_data_with_name(
			data, length, true, &status,  false,
			StringCast<ANSICHAR>(*DIRxNCP).Get()
		);///
		//
		if (status!=MONO_IMAGE_OK||CompilerImage==nullptr) {
			LOG::CS_CHAR(ESeverity::Error,TEXT("Error loading NodeCompiler"));
		}///
		//
		auto CompilerAssembly = mono_assembly_load_from_full(CompilerImage,StringCast<ANSICHAR>(*DIRxNCP).Get(),&status,false);
		//
		if (status!=MONO_IMAGE_OK||CompilerAssembly==nullptr) {
			mono_image_close(CompilerImage); LOG::CS_CHAR(ESeverity::Fatal,TEXT("Error loading NodeCompiler")); return false;
		}///
	}
	//
	MonoKismetDomain_INIT();
	//
	return true;
}

bool IMonoKismet::MonoKismet_STOP() {
	MonoKismetDomain_STOP();
	//
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoKismet::MonoKismetDomain_INIT() {
	CS_API("CS_CompilerInfo",ICALL::KAPI::CS_CompilerInfo);
	CS_API("CS_CompilerError",ICALL::KAPI::CS_CompilerError);
	CS_API("CS_CompilerWarning",ICALL::KAPI::CS_CompilerWarning);
	//
	LOG::CS_CHAR(ESeverity::Info,TEXT("C# compiler initialized."));
}

void IMonoKismet::MonoKismetDomain_STOP() {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (!IsInGameThread()) {
		mono_thread_detach(mono_thread_current());
	}///
	//
	mono_image_close(CompilerImage);
	mono_image_close(CoreImage);
	//
	mono_domain_set(MonoCore.GetCoreDomain(),0);
	mono_domain_unload(KismetDomain);
	//
	KismetState = EMonoKismetState::Iddle;
	LOG::CS_CHAR(ESeverity::Info,TEXT("C# compiler shutdown."));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoKismet::CompileNode(UMagicNodeSharpSource* Source, UMagicNodeSharp* Node) {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	if (CompilerImage==nullptr) {MonoKismet.CompilationFinished(Source,FCompilerResults()); return;}
	if (mono_domain_get()!=MonoKismet.GetKismetDomain()) {MonoKismet.CompilationFinished(Source,FCompilerResults()); return;}
	//
	Async(EAsyncExecution::Thread,[&Source,&Node]() {
		IMonoKismet::CompileScript(Source,Node);
	});//
	//
	KismetState = EMonoKismetState::Compiling;
}

void IMonoKismet::CompilationFinished(UMagicNodeSharpSource* Source, const FCompilerResults &Results) {
	const UPTRINT IntPtr = reinterpret_cast<UPTRINT>(Source);
	//
	Async(EAsyncExecution::TaskGraphMainThread,[IntPtr,Results]() {
		static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
		//
		UMagicNodeSharpSource* Source = reinterpret_cast<UMagicNodeSharpSource*>(IntPtr);
		//
		if (Source->IsValidLowLevelFast()) {
			MonoKismet.CompilerResult.ExecuteIfBound(Source,Results);
		} else {MonoKismet.MonoKismetDomain_STOP();}
	});//
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FMonoClassDefinition IMonoKismet::CollectClassInfo(UMagicNodeSharpSource* const &Script, MonoObject* ManagedObject) {
	MonoClass* ManagedClass = mono_object_get_class(ManagedObject);
	return IKCS_MonoAnalyzer::GetClassInfo(ManagedClass);
}

TArray<FMonoPropertyInfo>IMonoKismet::CollectClassPropertyInfo(MonoObject* ManagedObject) {
	MonoClass* ManagedClass = mono_object_get_class(ManagedObject);
	MonoProperty* Property = nullptr; void* Itr=0;
	//
	FString Description;
	if (MonoType*ClassType=mono_class_get_type(ManagedClass)) {
		const char* Info = mono_type_get_name_full(ClassType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
		Description = FString(StringCast<TCHAR>(Info).Get());
	}///
	//
	TArray<FMonoPropertyInfo>Props;
	while((Property=mono_class_get_properties(ManagedClass,&Itr)) != NULL) {
		MonoMethod* MethodGet = mono_property_get_get_method(Property);
		MonoMethod* MethodSet = mono_property_get_set_method(Property);
		const char* FieldName = mono_property_get_name(Property);
		//
		bool IsInput = false;
		bool IsOutput = false;
		//
		FMonoPropertyInfo Info;
		Info.Description = Description;
		Info.Name = StringCast<TCHAR>(FieldName).Get();
		//
		if (MethodGet) {
			auto Flags = mono_method_get_flags(MethodGet,nullptr);
			auto MethodSign = mono_method_get_signature(MethodGet,mono_class_get_image(ManagedClass),mono_method_get_token(MethodGet));
			auto ReturnType = mono_signature_get_return_type(MethodSign);
			//
			GetDataTypeFromManagedType(ReturnType,Info);
			//
			if (((Flags)&MONO_METHOD_ATTR_STATIC)!=NULL) {
				Info.ScopeType = EMonoScopeType::Static;
			} if (((Flags)&MONO_METHOD_ATTR_VIRTUAL)!=NULL) {
				Info.ScopeType = EMonoScopeType::Virtual;
			} if (((Flags)&MONO_METHOD_ATTR_FAMILY)!=NULL) {
				Info.ScopeType = EMonoScopeType::Member;
			} if (((Flags)&MONO_METHOD_ATTR_ASSEM)!=NULL) {
				Info.ScopeType = EMonoScopeType::Internal;
			} if (((Flags)&MONO_METHOD_ATTR_FAM_AND_ASSEM)!=NULL) {
				Info.ScopeType = EMonoScopeType::Internal;
			}///
			//
			if (((Flags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_PUBLIC) {
				Info.AccessType = EMonoAccessType::Public;
				IsOutput = true;
				//
				if (Info.ListType==EMonoListType::None) {
					switch(Info.DataType) {
						case EMonoDataType::Void: break;
						//
						case EMonoDataType::Bool:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								bool Value = *(bool*)mono_object_unbox(iCall);
								Info.Value = (Value) ? TEXT("true") : TEXT("false");
							}///
						} break;
						//
						case EMonoDataType::Byte:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								uint8 Value = *(uint8*)mono_object_unbox(iCall);
								Info.Value = FString::Printf(TEXT("%i"),Value);
							}///
						} break;
						//
						case EMonoDataType::Int:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								int32 Value = *(int32*)mono_object_unbox(iCall);
								Info.Value = FString::Printf(TEXT("%i"),Value);
							}///
						} break;
						//
						case EMonoDataType::Int64:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								int64 Value = *(int64*)mono_object_unbox(iCall);
								Info.Value = FString::Printf(TEXT("%i"),Value);
							}///
						} break;
						//
						case EMonoDataType::Float:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								float Value = *(float*)mono_object_unbox(iCall);
								Info.Value = FString::Printf(TEXT("%f"),Value);
							}///
						} break;
						//
						case EMonoDataType::String:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoString Value = *(FMonoString*)mono_object_unbox(iCall);
								Info.Value = Value.ToString();
							}///
						} break;
						//
						case EMonoDataType::Name:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoName Value = *(FMonoName*)mono_object_unbox(iCall);
								Info.Value = Value.ToName().ToString();
							}///
						} break;
						//
						case EMonoDataType::Text:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoText Value = *(FMonoText*)mono_object_unbox(iCall);
								Info.Value = Value.ToText().ToString();
							}///
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoVector2D Value = *(FMonoVector2D*)mono_object_unbox(iCall);
								FVector2D V2D = Value.ToVector2D();
								//
								FJsonObjectConverter::UStructToJsonObjectString(TBaseStructure<FVector2D>::Get(),&V2D,Info.Value,0,0);
							}///
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoVector3D Value = *(FMonoVector3D*)mono_object_unbox(iCall);
								FVector V3D = Value.ToVector3D();
								//
								FJsonObjectConverter::UStructToJsonObjectString(TBaseStructure<FVector>::Get(),&V3D,Info.Value,0,0);
							}///
						} break;
						//
						case EMonoDataType::Rotator:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoRotator Value = *(FMonoRotator*)mono_object_unbox(iCall);
								FRotator ROT = Value.ToRotator();
								//
								FJsonObjectConverter::UStructToJsonObjectString(TBaseStructure<FRotator>::Get(),&ROT,Info.Value,0,0);
							}///
						} break;
						//
						case EMonoDataType::Color:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoColor Value = *(FMonoColor*)mono_object_unbox(iCall);
								FColor RGB = Value.ToColor();
								//
								FJsonObjectConverter::UStructToJsonObjectString(TBaseStructure<FColor>::Get(),&RGB,Info.Value,0,0);
							}///
						} break;
						//
						case EMonoDataType::Transform:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoTransform Value = *(FMonoTransform*)mono_object_unbox(iCall);
								FTransform TRF = Value.ToTransform();
								//
								FJsonObjectConverter::UStructToJsonObjectString(TBaseStructure<FTransform>::Get(),&TRF,Info.Value,0,0);
							}///
						} break;
						//
						case EMonoDataType::Class:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoClass Value = *(FMonoClass*)mono_object_unbox(iCall);
								UClass* CLC = Value.ToClass();
								//
								Info.Value = FSoftClassPath(CLC).ToString();
							}///
						} break;
						//
						case EMonoDataType::Object:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoObject Value = *(FMonoObject*)mono_object_unbox(iCall);
								UObject* OBJ = Value.ToObject();
								//
								Info.Value = FSoftObjectPath(OBJ).ToString();
							}///
						} break;
						//
						case EMonoDataType::Actor:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoActor Value = *(FMonoActor*)mono_object_unbox(iCall);
								AActor* ACT = Value.ToActor();
								//
								Info.Value = FSoftObjectPath(ACT).ToString();
							}///
						} break;
						//
						case EMonoDataType::Component:
						{
							if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
								FMonoComponent Value = *(FMonoComponent*)mono_object_unbox(iCall);
								UActorComponent* CMP = Value.ToComponent();
								//
								Info.Value = FSoftObjectPath(CMP).ToString();
							}///
						} break;
					default: break;}
				}///
				//
				if (Info.ListType==EMonoListType::Array) {
					if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
						FMonoArray Value = *(FMonoArray*)mono_object_unbox(iCall);
						Info.DataType = Value.InnerType;
						Info.Value = TEXT("");
					}///
				}///
				//
				if (Info.ListType==EMonoListType::Set) {
					if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
						FMonoSet Value = *(FMonoSet*)mono_object_unbox(iCall);
						Info.DataType = Value.InnerType;
						Info.Value = TEXT("");
					}///
				}///
				//
				if (Info.ListType==EMonoListType::Map) {
					if (MonoObject*iCall=mono_runtime_invoke(MethodGet,ManagedObject,NULL,NULL)) {
						FMonoMap Value = *(FMonoMap*)mono_object_unbox(iCall);
						Info.DataType = Value.InnerType;
						Info.SubType = Value.SubType;
						Info.Value = TEXT("");
					}///
				}///
			}///
		}///
		//
		if (MethodSet) {
			auto Flags = mono_method_get_flags(MethodSet,nullptr);
			//
			if (((Flags)&MONO_METHOD_ATTR_STATIC)!=NULL) {
				Info.ScopeType = EMonoScopeType::Static;
			} if (((Flags)&MONO_METHOD_ATTR_VIRTUAL)!=NULL) {
				Info.ScopeType = EMonoScopeType::Virtual;
			} if (((Flags)&MONO_METHOD_ATTR_FAMILY)!=NULL) {
				Info.ScopeType = EMonoScopeType::Member;
			} if (((Flags)&MONO_METHOD_ATTR_ASSEM)!=NULL) {
				Info.ScopeType = EMonoScopeType::Internal;
			} if (((Flags)&MONO_METHOD_ATTR_FAM_AND_ASSEM)!=NULL) {
				Info.ScopeType = EMonoScopeType::Internal;
			}///
			//
			if (((Flags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_PUBLIC) {
				Info.AccessType = EMonoAccessType::Public;
				IsInput = true;
			}///
		}///
		//
		if (IsInput && IsOutput) {Info.ParamType=EMonoParamType::InOut;}
		else if (IsOutput) {Info.ParamType=EMonoParamType::Output;}
		else if (IsInput) {Info.ParamType=EMonoParamType::Input;}
		//
		if ((Info.DataType!=EMonoDataType::Unknown)&&(Info.SubType!=EMonoDataType::Unknown)) {Props.Add(Info);}
	}///
	//
	return Props;
}

void IMonoKismet::GetDataTypeFromManagedType(MonoType* ManagedType, FMonoFieldDefinition &OutDefinition) {
	IKCS_MonoAnalyzer::GetDataTypeFromMonoType(ManagedType,OutDefinition);
}

void IMonoKismet::GetDataTypeFromManagedType(MonoType* ManagedType, FMonoPropertyInfo &OutDefinition) {
	FMonoFieldDefinition Info{};
	//
	IKCS_MonoAnalyzer::GetDataTypeFromMonoType(ManagedType,Info);
	//
	OutDefinition.DataType = Info.DataType;
	OutDefinition.ListType = Info.ListType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoKismet::SetupThreadingAttribute(UMagicNodeSharp* Node, MonoClass* NodeClass) {
	if (MonoCustomAttrInfo*AttrInfo=mono_custom_attrs_from_class(NodeClass)) {
		for (int32 I=0; I<AttrInfo->num_attrs; I++) {
			if (MonoClass*AttrClass=mono_method_get_class(AttrInfo->attrs[I].ctor)) {
				const char* AttrName = mono_class_get_name(AttrClass);
				if (strcmp(AttrName,"Async")==0) {
					Node->IsAsyncTask=true; break;
				}///
			}///
		}///
		//
		mono_custom_attrs_free(AttrInfo);
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MonoDomain* IMonoKismet::GetKismetDomain() const {
	return KismetDomain;
}

MonoImage* IMonoKismet::GetCoreImage() const {
	return CoreImage;
}

MonoImage* IMonoKismet::GetCompilerImage() const {
	return CompilerImage;
}

MonoMethod* IMonoKismet::GetCodeFormatMethod() const {
	return CodeFormatMethod;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IMonoKismet::IsPlaying() const {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	return MonoCore.IsPlaying();
}

bool IMonoKismet::IsCompiling() const {
	return (KismetState==EMonoKismetState::Compiling);
}

bool IMonoKismet::CanCompile() const {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	return ((KismetState==EMonoKismetState::Iddle)&&(!MonoCore.IsPlaying())&&(mono_domain_get()==MonoCore.GetCoreDomain()));
}

bool IMonoKismet::CanFormatCode() const {
	return ((CodeEditorUtility!=nullptr)&&(CodeFormatMethod!=nullptr));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MonoClass* IMonoKismet::mono_find_class(MonoImage* FromImage, const FString &Namespace, const FString &ClassName) {
	if (FromImage==nullptr) {return NULL;}
	//
	return mono_class_from_name(FromImage,StringCast<ANSICHAR>(*Namespace).Get(),StringCast<ANSICHAR>(*ClassName).Get());
}

MonoMethod* IMonoKismet::mono_find_method(MonoClass* ManagedClass, const FString &MethodName) {
	if (ManagedClass==nullptr)	{return NULL;}
	//
	return mono_class_get_method_from_name(ManagedClass,StringCast<ANSICHAR>(*MethodName).Get(),-1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MonoObject* IMonoKismet::mono_create_object(MonoDomain *InDomain, MonoClass* ManagedClass) {
	if(!CS_ENSURE(ManagedClass)) {return NULL;}
	if(!CS_ENSURE(InDomain)) {return NULL;}
	//
	return mono_object_new(InDomain,ManagedClass);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////