//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MCS_MonoCore.h"
#include "MCS_Subsystem.h"
#include "MCS_File.h"

#include "Interfaces/IPluginManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoCore::MonoCore_INIT() {
	static IPlatformFile &PlatformManager = FPlatformFileManager::Get().GetPlatformFile();
	//
	#ifdef MCS_MARKETPLACE_BUILD
	 static FString DIRxCED = FPaths::Combine(*FPaths::EnginePluginsDir(),TEXT("Marketplace"),CS_PLUGIN_NAME,TEXT("Binaries"),TEXT("Editor"));
	 static FString DIRxCFG = FPaths::Combine(*FPaths::EnginePluginsDir(),TEXT("Marketplace"),CS_PLUGIN_NAME,TEXT("Binaries"));
	#else
	 static FString DIRxCED = FPaths::Combine(*FPaths::ProjectPluginsDir(),CS_PLUGIN_NAME,TEXT("Binaries"),TEXT("Editor"));
	 static FString DIRxCFG = FPaths::Combine(*FPaths::ProjectPluginsDir(),CS_PLUGIN_NAME,TEXT("Binaries"));
	#endif
	//
	static FString DIRxCLX = FPaths::Combine(*DIRxCFG,FString::Printf(TEXT("MonoPosixHelper%s"),*EXT));
	static FString DIRxCLR = FPaths::Combine(*DIRxCFG,FString::Printf(TEXT("mono-2.0-sgen%s"),*EXT));
	static FString DIRxLIB = FPaths::Combine(*DIRxCFG,TEXT("Mono/lib"));
	static FString DIRxETC = FPaths::Combine(*DIRxCFG,TEXT("Mono/etc"));
	//
	#if WITH_EDITOR
	 static FString DIRxSYS = FPaths::Combine(*DIRxLIB,TEXT("mono/4.8-api/mscorlib.dll"));
	 static FString DIRxUEN = FPaths::Combine(*DIRxCFG,TEXT("UnrealEngine.dll"));
	 static FString DIRxNED = FPaths::Combine(*DIRxCED,TEXT("NodeEditor.dll"));
	 static FString DIRxDOM = FPaths::Combine(*DIRxCFG,TEXT("MagicNode.dll"));
	#endif
	//
	{
		if (!PlatformManager.FileExists(*DIRxCLR)) {
			LOG::CS_CHAR(ESeverity::Error,TEXT("C# Core Assembly not found!"));
			LOG::CS_STR(ESeverity::Warning,DIRxCLR); return;
		} CLR = FPlatformProcess::GetDllHandle(*DIRxCLR);
		//
		#if WITH_EDITOR
		if (!PlatformManager.FileExists(*DIRxCLX)) {
			LOG::CS_CHAR(ESeverity::Error,TEXT("C# Core Assembly not found!"));
			LOG::CS_STR(ESeverity::Warning,DIRxCLX); return;
		} CLX = FPlatformProcess::GetDllHandle(*DIRxCLX);
		#endif
	}
	//
	#if (UE_BUILD_DEBUG||UE_BUILD_DEVELOPMENT)
	{
		mono_set_crash_chaining(1);
		mono_set_signal_chaining(1);
		//
		mono_trace_set_level_string(MonoLogLevel);
		mono_trace_set_print_handler(IMonoCore::MonoPrint_Callback);
		mono_trace_set_log_handler(IMonoCore::MonoLog_Callback,this);
		mono_trace_set_printerr_handler(IMonoCore::MonoPrint_Callback);
	}
	#endif
	//
	mono_set_dirs(StringCast<ANSICHAR>(*DIRxLIB).Get(),StringCast<ANSICHAR>(*DIRxETC).Get());
	CoreDomain = mono_jit_init_version(CS_CORE_DOMAIN,"v4.0.30319");
	//
	#if (UE_BUILD_DEBUG||UE_BUILD_DEVELOPMENT)
	mono_jit_parse_options(1,MonoJIT_Options);
	#endif
	//
	#if WITH_EDITOR
	{
		MonoImageOpenStatus status; size_t length = 0;
		char* data = File::Read(StringCast<ANSICHAR>(*DIRxSYS).Get(),&length);
		//
		EditorSystemImage = mono_image_open_from_data_with_name(
			data, length, true, &status,  false,
			StringCast<ANSICHAR>(*DIRxSYS).Get()
		);///
		//
		if (status!=MONO_IMAGE_OK||EditorSystemImage==nullptr) {
			LOG::CS_CHAR(ESeverity::Fatal,TEXT("Error loading mscorlib.dll"));
		}///
		//
		MonoAssembly* SystemAssembly = mono_assembly_load_from_full(EditorSystemImage,StringCast<ANSICHAR>(*DIRxSYS).Get(),&status,false);
		//
		if (status!=MONO_IMAGE_OK||SystemAssembly==nullptr) {
			mono_image_close(EditorSystemImage); LOG::CS_CHAR(ESeverity::Fatal,TEXT("mscorlib.dll not found."));
		}///
	}
	//
	{
		MonoImageOpenStatus status; size_t length = 0;
		char* data = File::Read(StringCast<ANSICHAR>(*DIRxUEN).Get(),&length);
		//
		EditorUnrealImage = mono_image_open_from_data_with_name(
			data, length, true, &status,  false,
			StringCast<ANSICHAR>(*DIRxUEN).Get()
		);///
		//
		if (status!=MONO_IMAGE_OK||EditorUnrealImage==nullptr) {
			LOG::CS_CHAR(ESeverity::Fatal,TEXT("Error loading UnrealEngine.dll"));
		}///
		//
		MonoAssembly* UnrealAssembly = mono_assembly_load_from_full(EditorUnrealImage,StringCast<ANSICHAR>(*DIRxUEN).Get(),&status,false);
		//
		if (status!=MONO_IMAGE_OK||UnrealAssembly==nullptr) {
			mono_image_close(EditorUnrealImage); LOG::CS_CHAR(ESeverity::Fatal,TEXT("UnrealEngine.dll not found."));
		}///
	}
	//
	{
		MonoImageOpenStatus status; size_t length = 0;
		char* data = File::Read(StringCast<ANSICHAR>(*DIRxDOM).Get(),&length);
		//
		EditorCoreImage = mono_image_open_from_data_with_name(
			data, length, true, &status,  false,
			StringCast<ANSICHAR>(*DIRxDOM).Get()
		);///
		//
		if (status!=MONO_IMAGE_OK||EditorCoreImage==nullptr) {
			LOG::CS_CHAR(ESeverity::Fatal,TEXT("Error loading MagicNode.dll"));
		}///
		//
		MonoAssembly* CoreAssembly = mono_assembly_load_from_full(EditorCoreImage,StringCast<ANSICHAR>(*DIRxDOM).Get(),&status,false);
		//
		if (status!=MONO_IMAGE_OK||CoreAssembly==nullptr) {
			mono_image_close(EditorCoreImage); LOG::CS_CHAR(ESeverity::Fatal,TEXT("MagicNode.dll not found."));
		}///
	}
	//
	{
		MonoImageOpenStatus status; size_t length = 0;
		char* data = File::Read(StringCast<ANSICHAR>(*DIRxNED).Get(),&length);
		//
		CodeEditorImage = mono_image_open_from_data_with_name(
			data, length, true, &status,  false,
			StringCast<ANSICHAR>(*DIRxNED).Get()
		);///
		//
		if (status!=MONO_IMAGE_OK||CodeEditorImage==nullptr) {
			LOG::CS_CHAR(ESeverity::Error,TEXT("Error loading NodeEditor"));
		}///
		//
		auto EditorAssembly = mono_assembly_load_from_full(CodeEditorImage,StringCast<ANSICHAR>(*DIRxNED).Get(),&status,false);
		//
		if (status!=MONO_IMAGE_OK||CodeEditorImage==nullptr) {
			mono_image_close(CodeEditorImage); LOG::CS_CHAR(ESeverity::Fatal,TEXT("Error loading NodeEditor")); return;
		}///
	}
	#endif
	//
	MonoCoreDomain_INIT();
}

void IMonoCore::MonoCore_STOP() {
	MonoCoreDomain_STOP();
	//
	FPlatformProcess::FreeDllHandle(CLR);
	//
	#if WITH_EDITOR
	FPlatformProcess::FreeDllHandle(CLX);
	#endif
	//
	CLR = nullptr;
	CLX = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoCore::MonoCoreDomain_INIT() {
	mono_thread_set_main(mono_thread_current());
	//
	// :: CORE API ::
	{
		ICORE::Register();
	}
	// :: ENGINE API ::
	{
		IENGINE::Register();
	}
	// :: KISMET MATH API ::
	{
		//IMATH::Register();
	}
	// :: KISMET SYSTEM API ::
	{
		//ISYSTEM::Register();
	}
	// :: KISMET GAMEPLAY API ::
	{
		//IGAMEPLAY::Register();
	}
	// :: KISMET FRAMEWORK API ::
	{
		//IFRAMEWORK::Register();
	}
	//
	LOG::CS_CHAR(ESeverity::Info,TEXT("C# runtime initialized."));
}

void IMonoCore::MonoCoreDomain_STOP() {
	if (CoreDomain) {mono_runtime_cleanup(CoreDomain);}
	//
	LOG::CS_CHAR(ESeverity::Info,TEXT("C# runtime shutdown."));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoCore::MonoPlayDomain_INIT() {
	static IPlatformFile &PlatformManager = FPlatformFileManager::Get().GetPlatformFile();
	//
	#ifdef MCS_MARKETPLACE_BUILD
	 static FString DIRxCFG = FPaths::Combine(*FPaths::EnginePluginsDir(),TEXT("Marketplace"),CS_PLUGIN_NAME,TEXT("Binaries"));
	#else
	 static FString DIRxCFG = FPaths::Combine(*FPaths::ProjectPluginsDir(),CS_PLUGIN_NAME,TEXT("Binaries"));
	#endif
	//
	static FString DIRxUEN = FPaths::Combine(*DIRxCFG,TEXT("UnrealEngine.dll"));
	static FString DIRxDOM = FPaths::Combine(*DIRxCFG,TEXT("MagicNode.dll"));
	//
	if (!CanPlay()) {
		LOG::CS_CHAR(ESeverity::Warning,TEXT("C# cannot enter play mode while compiler is busy.")); return;
	}///
	//
	PlayDomain = mono_domain_create_appdomain(CS_PLAY_DOMAIN,NULL);
	mono_domain_set(PlayDomain,0);
	//
	{
		MonoImageOpenStatus status; size_t length = 0;
		char* data = File::Read(StringCast<ANSICHAR>(*DIRxUEN).Get(),&length);
		//
		UnrealImage = mono_image_open_from_data_with_name(
			data, length, true, &status,  false,
			StringCast<ANSICHAR>(*DIRxUEN).Get()
		);///
		//
		if (status!=MONO_IMAGE_OK||UnrealImage==nullptr) {
			LOG::CS_CHAR(ESeverity::Fatal,TEXT("Error loading UnrealEngine.dll"));
		}///
		//
		MonoAssembly* UnrealAssembly = mono_assembly_load_from_full(UnrealImage,StringCast<ANSICHAR>(*DIRxUEN).Get(),&status,false);
		//
		if (status!=MONO_IMAGE_OK||UnrealAssembly==nullptr) {
			mono_image_close(UnrealImage); LOG::CS_CHAR(ESeverity::Fatal,TEXT("UnrealEngine.dll not found."));
		}///
	}
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
	static TArray<FString>AssemblyPaths;
	IFileManager &FileManager = IFileManager::Get();
	FileManager.FindFilesRecursive(AssemblyPaths,*CS_BINARY_DIR,*(FString(TEXT("/*"))+EXT),true,false);
	if (AssemblyPaths.Num()==0) {return;}
	//
	for (const FString &AS : AssemblyPaths) {
		MonoImageOpenStatus status; size_t length = 0;
		const FString FullPath = FPaths::ConvertRelativePathToFull(AS);
		char* data = File::Read(StringCast<ANSICHAR>(*FullPath).Get(),&length);
		//
		FString FileName;
		FullPath.Split(TEXT("/"),nullptr,&FileName,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
		if (FileName==TEXT("UnrealEngine.dll")||FileName==TEXT("MagicNode.dll")) {continue;}
		//
		MonoImage* NodeImage = mono_image_open_from_data_with_name(
			data, length, true, &status, false,
			StringCast<ANSICHAR>(*FullPath).Get()
		);///
		//
		if (status!=MONO_IMAGE_OK||NodeImage==nullptr) {
			LOG::CS_STR(ESeverity::Error,FString::Printf(TEXT("Error loading %s"),*FileName)); continue;
		}///
		//
		MonoAssembly* NodeAssembly = mono_assembly_load_from_full(NodeImage,StringCast<ANSICHAR>(*FullPath).Get(),&status,false);
		//
		if (status!=MONO_IMAGE_OK||NodeAssembly==nullptr) {
			mono_image_close(NodeImage); LOG::CS_STR(ESeverity::Error,FString::Printf(TEXT("Error loading %s"),*FileName)); continue;
		}///
		//
		mono_register_image(NodeImage);
		LOG::CS_STR(ESeverity::Info,FString::Printf(TEXT("Registered Node Assembly: %s"),*FileName));
	}///
	//
	LOG::CS_CHAR(ESeverity::Info,TEXT("C# runtime begin play."));
}

void IMonoCore::MonoPlayDomain_STOP() {
	for (const auto &OBJ : PlayObjects) {
		if (OBJ==nullptr||!OBJ->IsValidLowLevelFast()||OBJ->IsPendingKill()) {continue;}
		//
		OBJ->CleanupOnQuit();
	}///
	for (const auto &IMG : PlayImages) {
		mono_image_close(IMG);
	} PlayImages.Empty();
	//
	mono_image_close(CoreImage);
	mono_image_close(UnrealImage);
	//
#if WITH_EDITOR
	if (PlayDomain && (mono_domain_get()==PlayDomain)) {
		mono_domain_set(CoreDomain,0);
	} mono_domain_unload(PlayDomain);
	//
	mono_gc_collect(mono_gc_max_generation());
#endif
	//
	LOG::CS_CHAR(ESeverity::Info,TEXT("C# runtime end play."));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoCore::BeginPlay() {
	switch(PlayState) {
		case EMonoPlayState::Shutdown:
		{
			MonoPlayDomain_INIT();
			PlayState = EMonoPlayState::Playing;
		} break;
	default: break;}
}

void IMonoCore::EndPlay() {
	switch(PlayState) {
		case EMonoPlayState::Playing:
		{
			MonoPlayDomain_STOP();
			PlayState = EMonoPlayState::Shutdown;
		} break;
	default: break;}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MonoImage* IMonoCore::GetCoreImage() const {
	return CoreImage;
}

MonoImage* IMonoCore::GetUnrealImage() const {
	return UnrealImage;
}

MonoDomain* IMonoCore::GetCoreDomain() const {
	return CoreDomain;
}

MonoDomain* IMonoCore::GetPlayDomain() const {
	return PlayDomain;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if WITH_EDITOR

MonoImage* IMonoCore::GetEditorImage() const {
	return CodeEditorImage;
}

MonoImage* IMonoCore::GetEditorCoreImage() const {
	return EditorCoreImage;
}

MonoImage* IMonoCore::GetEditorUnrealImage() const {
	return EditorUnrealImage;
}

MonoImage* IMonoCore::GetEditorSystemImage() const {
	return EditorSystemImage;
}

#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IMonoCore::IsPlaying() const {
	return ((PlayState==EMonoPlayState::Playing)&&(PlayDomain));
}

bool IMonoCore::CanPlay() const {
	return ((PlayState==EMonoPlayState::Shutdown)&&(mono_domain_get()==CoreDomain));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MonoImage* IMonoCore::mono_find_image(const FString &ScriptName) {
	if (PlayImages.Num()==0) {return NULL;}
	//
	for (MonoImage* IMG : PlayImages) {
		FString FileName = StringCast<TCHAR>(mono_image_get_filename(IMG)).Get();
		FileName.Split(TEXT("/"),nullptr,&FileName,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
		FileName.RemoveFromEnd(EXT);
		//
		if (FileName==ScriptName) {return IMG;}
	} return NULL;
}

MonoClass* IMonoCore::mono_find_class(MonoImage* FromImage, const FString &Namespace, const FString &ClassName) {
	if (FromImage==nullptr) {return NULL;}
	//
	return mono_class_from_name(FromImage,StringCast<ANSICHAR>(*Namespace).Get(),StringCast<ANSICHAR>(*ClassName).Get());
}

MonoMethod* IMonoCore::mono_find_method(MonoClass* ManagedClass, const FString &MethodName) {
	if (ManagedClass==nullptr)	{return NULL;}
	//
	return mono_class_get_method_from_name(ManagedClass,StringCast<ANSICHAR>(*MethodName).Get(),-1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MonoObject* IMonoCore::mono_create_object(MonoDomain *InDomain, MonoClass* ManagedClass) {
	if(!CS_ENSURE(ManagedClass)) {return NULL;}
	if(!CS_ENSURE(InDomain)) {return NULL;}
	//
	return mono_object_new(InDomain,ManagedClass);
}

MonoObject* IMonoCore::mono_find_object(const FGuid &NodeID) {
	if (PlayObjects.Num()==0) {return NULL;}
	//
	for (const auto &OBJ : PlayObjects) {
		if (OBJ==nullptr||!OBJ->IsValidLowLevelFast()||OBJ->IsPendingKill()) {continue;}
		if (OBJ->GetNodeID()==NodeID) {return OBJ->GetMonoObject();}
	} return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoCore::mono_register_image(MonoImage* Instance) {
	if(!CS_ENSURE(Instance)) {return;}
	//
	PlayImages.Add(Instance);
}

void IMonoCore::mono_register_object(UMagicNodeSharp* Instance) {
	if(!CS_ENSURE(Instance)) {return;}
	//
	PlayObjects.Add(Instance);
}

void IMonoCore::mono_unregister_object(UMagicNodeSharp* Instance) {
	if(!CS_ENSURE(Instance)) {return;}
	//
	if (PlayObjects.Contains(Instance)) {
		PlayObjects.Remove(Instance);
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoCore::AddReferencedObjects(FReferenceCollector &Collector) {
	for (auto &OBJ : PlayObjects) {
		Collector.AddReferencedObject(OBJ);
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !UE_BUILD_SHIPPING

void IMonoCore::MonoLog_Callback(const char* log_domain, const char* log_level, const char* message, mono_bool fatal, void* user_data) {
	if (strlen(message)<3) {return;}
	//
	if ((bool)fatal) {
		Async(EAsyncExecution::TaskGraphMainThread,[log_domain,message] {
			const WIDECHAR* Message = StringCast<WIDECHAR>(message).Get();
			const WIDECHAR* Domain = StringCast<WIDECHAR>(log_domain).Get();
			LOG::CS_STR(ESeverity::Fatal,FString::Printf(TEXT("%s :: %s"),Domain,Message).Replace(TEXT("Mono ::"),TEXT("C# ::")));
		});//
	}///
	//
	int32 InLogLevel = -1;
	if (strcmp(log_level,"error")		==0)	{InLogLevel=0;}
	if (strcmp(log_level,"critical")	==0)	{InLogLevel=1;}
	if (strcmp(log_level,"warning")		==0)	{InLogLevel=2;}
	if (strcmp(log_level,"message")		==0)	{InLogLevel=3;}
	if (strcmp(log_level,"info")		==0)	{InLogLevel=4;}
	if (strcmp(log_level,"debug")		==0)	{InLogLevel=5;}
	//
	const WIDECHAR* Domain = StringCast<WIDECHAR>(log_domain).Get();
	const WIDECHAR* Message = StringCast<WIDECHAR>(message).Get();
	//
	switch(InLogLevel) {
		case 0: LOG::CS_STR(ESeverity::Error,FString::Printf(TEXT("%s :: %s"),Domain,Message).Replace(TEXT("Mono ::"),TEXT("C# ::")));		break;
		case 1: LOG::CS_STR(ESeverity::Error,FString::Printf(TEXT("%s :: %s"),Domain,Message).Replace(TEXT("Mono ::"),TEXT("C# ::")));		break;
		case 2: LOG::CS_STR(ESeverity::Warning,FString::Printf(TEXT("%s :: %s"),Domain,Message).Replace(TEXT("Mono ::"),TEXT("C# ::")));	break;
		case 3: LOG::CS_STR(ESeverity::Info,FString::Printf(TEXT("%s :: %s"),Domain,Message).Replace(TEXT("Mono ::"),TEXT("C# ::")));		break;
		//case 4: LOG::CS_STR(ESeverity::Info,FString::Printf(TEXT("%s :: %s"),Domain,Message).Replace(TEXT("Mono ::"),TEXT("C# ::")));		break;
		//case 5: LOG::CS_STR(ESeverity::Info,FString::Printf(TEXT("%s :: %s"),Domain,Message).Replace(TEXT("Mono ::"),TEXT("C# ::")));		break;
	default: break;}
}

void IMonoCore::MonoPrint_Callback(const char* string, mono_bool is_stdout) {
	if (strlen(string)<3) {return;}
	//
	const bool stout = (bool)is_stdout;
	const WIDECHAR* Message = StringCast<WIDECHAR>(string).Get();
	//
	if (stout) {
		LOG::CSX_STR(ESeverity::Info,FString::Printf(TEXT("%s"),Message));
	} else {LOG::CSX_STR(ESeverity::Warning,FString::Printf(TEXT("%s"),Message));}
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////