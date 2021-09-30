//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MCS_API.h"
#include "MCS_MonoObject.h"

#include "mono/jit/jit.h"
#include "mono/metadata/image.h"
#include "mono/metadata/class.h"
#include "mono/metadata/object.h"
#include "mono/metadata/threads.h"
#include "mono/metadata/mono-gc.h"
#include "mono/metadata/attrdefs.h"
#include "mono/metadata/profiler.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/reflection.h"
#include "mono/metadata/environment.h"
#include "mono/metadata/mono-config.h"

#if !UE_BUILD_SHIPPING
 #include "mono/utils/mono-logger.h"
 #include "mono/metadata/mono-debug.h"
 #include "mono/metadata/debug-helpers.h"
#endif

#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/CoreUObject/Public/UObject/GCObject.h"

#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformProcess.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IMonoCore;
class UMonoGameInstanceSubsystem;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (UE_BUILD_DEBUG||UE_BUILD_DEVELOPMENT)
	static char* MonoJIT_Options[] = {"--debugger-agent=transport=dt_socket,address=127.0.0.1:1980,embedding=1 --soft-breakpoints"};
	static char* MonoLogLevel = "debug";
#elif UE_BUILD_SHIPPING
	static char* MonoLogLevel = "warning";
#endif

#define CS_CORE_DOMAIN "CsCoreDomain"
#define CS_PLAY_DOMAIN "CsPlayDomain"

#define CS_CORE_NAMESPACE "Unreal"
#define CS_NODE_CLASS "Node"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MAGICNODESHARP_API IMonoCore : public FGCObject {
	friend class UMonoGameInstanceSubsystem;
private:
	EMonoPlayState PlayState;
private:
	void* CLR = nullptr;
	void* CLX = nullptr;
private:
	MonoImage* CoreImage = nullptr;
	MonoImage* UnrealImage = nullptr;
private:
	#if WITH_EDITOR
	MonoImage* CodeEditorImage = nullptr;
	MonoImage* EditorCoreImage = nullptr;
	MonoImage* EditorUnrealImage = nullptr;
	MonoImage* EditorSystemImage = nullptr;
	#endif
private:
	MonoDomain* CoreDomain = nullptr;
	MonoDomain* PlayDomain = nullptr;
private:
	TArray<MonoImage*>PlayImages;
	TArray<UMagicNodeSharp*>PlayObjects;
private:
#if !UE_BUILD_SHIPPING
	static void MonoLog_Callback(const char* log_domain, const char* log_level, const char* message, mono_bool fatal, void* user_data);
	static void MonoPrint_Callback(const char* string, mono_bool is_stdout);
#endif
private:
	void MonoCoreDomain_INIT();
	void MonoCoreDomain_STOP();
private:
	void MonoPlayDomain_INIT();
	void MonoPlayDomain_STOP();
protected:
	void MonoCore_INIT();
	void MonoCore_STOP();
protected:
	void BeginPlay();
	void EndPlay();
public:
	bool IsPlaying() const;
	bool CanPlay() const;
public:
	MonoImage* GetCoreImage() const;
	MonoImage* GetUnrealImage() const;
public:
	#if WITH_EDITOR
	MonoImage* GetEditorImage() const;
	MonoImage* GetEditorCoreImage() const;
	MonoImage* GetEditorUnrealImage() const;
	MonoImage* GetEditorSystemImage() const;
	#endif
public:
	MonoDomain* GetCoreDomain() const;
	MonoDomain* GetPlayDomain() const;
public:
	MonoImage* mono_find_image(const FString &ScriptName);
public:
	MonoClass* mono_find_class(MonoImage* FromImage, const FString &Namespace, const FString &ClassName);
	MonoMethod* mono_find_method(MonoClass* ManagedClass, const FString &MethodName);
public:
	MonoObject* mono_create_object(MonoDomain* InDomain, MonoClass* ManagedClass);
	MonoObject* mono_find_object(const FGuid &NodeID);
public:
	void mono_register_image(MonoImage* Instance);
	void mono_register_object(UMagicNodeSharp* Instance);
	void mono_unregister_object(UMagicNodeSharp* Instance);
public:
	virtual void AddReferencedObjects(FReferenceCollector &Collector) override;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////