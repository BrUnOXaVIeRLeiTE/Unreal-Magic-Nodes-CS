//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MCS_API.h"

#include "Runtime/Engine/Public/Tickable.h"

#if WITH_EDITOR
 #include "Developer/DirectoryWatcher/Public/DirectoryWatcherModule.h"
 #include "Developer/DirectoryWatcher/Public/IDirectoryWatcher.h"
#endif

#include "MagicNodeSharp.generated.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static FString CS_BINARY_DIR = FPaths::Combine(*FPaths::ProjectDir(),TEXT("Binaries/Managed"));

#if WITH_EDITOR
 static FString CS_DATA_EXT = FString(TEXT(".so"));
 static FString CS_SCRIPT_EXT = FString(TEXT(".cs"));
 static FString CS_SCRIPT_DIR = FPaths::Combine(*FPaths::ConvertRelativePathToFull(FPaths::GameSourceDir()),TEXT("Scripts"));
 static FString CS_DATA_DIR = FPaths::Combine(*FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()),TEXT("Intermediate/Managed"));
#endif

#if PLATFORM_WINDOWS
 static FString EXT = TEXT(".dll");
 static FString FIX = TEXT("");
#elif PLATFORM_LINUX
 static FString EXT = TEXT(".so");
 static FString FIX = TEXT("lib");
#elif PLATFORM_MAC
 static FString EXT = TEXT(".dylib");
 static FString FIX = TEXT("lib");
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UMonoGameInstanceSubsystem;
class UMagicNodeSharpSource;
class SDIFFMainWidget;
class IMonoKismet;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_MULTICAST_DELEGATE(FOnScriptSourceDeleted);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnScriptSourceExported, UMagicNodeSharpSource*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnScriptRuntimeException, UMagicNodeSharpSource*, const TCHAR*);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Node Marshal Helper:

UCLASS(classGroup="Synaptech", Category="MagicNode", Abstract, hidedropdown)
class MAGICNODESHARP_API UMagicNodeSharpMarshal final : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY() bool _Bool;
	UPROPERTY() uint8 _Byte;
	UPROPERTY() int32 _Int;
	UPROPERTY() int64 _Int64;
	UPROPERTY() float _Float;
	//
	UPROPERTY() FString _String;
	UPROPERTY() FName _Name;
	UPROPERTY() FText _Text;
	//
	UPROPERTY() FVector2D _Vector2D;
	UPROPERTY() FVector _Vector3D;
	UPROPERTY() FRotator _Rotator;
	UPROPERTY() FColor _Color;
	UPROPERTY() FTransform _Transform;
	//
	UPROPERTY() UClass* _Class;
	UPROPERTY() UObject* _Object;
	UPROPERTY() AActor* _Actor;
	UPROPERTY() UActorComponent* _Component;
public:
	UPROPERTY() TArray<bool>_BoolArray;
	UPROPERTY() TArray<uint8>_ByteArray;
	UPROPERTY() TArray<int32>_IntArray;
	UPROPERTY() TArray<int64>_Int64Array;
	UPROPERTY() TArray<float>_FloatArray;
	//
	UPROPERTY() TArray<FString>_StringArray;
	UPROPERTY() TArray<FName>_NameArray;
	UPROPERTY() TArray<FText>_TextArray;
	//
	UPROPERTY() TArray<FVector2D>_Vector2DArray;
	UPROPERTY() TArray<FVector>_Vector3DArray;
	UPROPERTY() TArray<FRotator>_RotatorArray;
	UPROPERTY() TArray<FColor>_ColorArray;
	UPROPERTY() TArray<FTransform>_TransformArray;
	//
	UPROPERTY() TArray<UClass*>_ClassArray;
	UPROPERTY() TArray<UObject*>_ObjectArray;
	UPROPERTY() TArray<AActor*>_ActorArray;
	UPROPERTY() TArray<UActorComponent*>_ComponentArray;
public:
	UPROPERTY() TSet<uint8>_ByteSet;
	UPROPERTY() TSet<int32>_IntSet;
	UPROPERTY() TSet<int64>_Int64Set;
	UPROPERTY() TSet<float>_FloatSet;
	//
	UPROPERTY() TSet<FString>_StringSet;
	UPROPERTY() TSet<FName>_NameSet;
	//
	UPROPERTY() TSet<FVector2D>_Vector2DSet;
	UPROPERTY() TSet<FVector>_Vector3DSet;
	UPROPERTY() TSet<FColor>_ColorSet;
	//
	UPROPERTY() TSet<UClass*>_ClassSet;
	UPROPERTY() TSet<UObject*>_ObjectSet;
	UPROPERTY() TSet<AActor*>_ActorSet;
	UPROPERTY() TSet<UActorComponent*>_ComponentSet;
public:
	UPROPERTY() TMap<uint8,bool>_ByteBoolMap;
	UPROPERTY() TMap<uint8,uint8>_ByteByteMap;
	UPROPERTY() TMap<uint8,int32>_ByteIntMap;
	UPROPERTY() TMap<uint8,int64>_ByteInt64Map;
	UPROPERTY() TMap<uint8,float>_ByteFloatMap;
	//
	UPROPERTY() TMap<uint8,FString>_ByteStringMap;
	UPROPERTY() TMap<uint8,FName>_ByteNameMap;
	UPROPERTY() TMap<uint8,FText>_ByteTextMap;
	//
	UPROPERTY() TMap<uint8,FVector2D>_ByteVector2DMap;
	UPROPERTY() TMap<uint8,FVector>_ByteVector3DMap;
	UPROPERTY() TMap<uint8,FRotator>_ByteRotatorMap;
	UPROPERTY() TMap<uint8,FColor>_ByteColorMap;
	UPROPERTY() TMap<uint8,FTransform>_ByteTransformMap;
	//
	UPROPERTY() TMap<uint8,UClass*>_ByteClassMap;
	UPROPERTY() TMap<uint8,UObject*>_ByteObjectMap;
	UPROPERTY() TMap<uint8,AActor*>_ByteActorMap;
	UPROPERTY() TMap<uint8,UActorComponent*>_ByteComponentMap;
public:
	UPROPERTY() TMap<int32,bool>_IntBoolMap;
	UPROPERTY() TMap<int32,uint8>_IntByteMap;
	UPROPERTY() TMap<int32,int32>_IntIntMap;
	UPROPERTY() TMap<int32,int64>_IntInt64Map;
	UPROPERTY() TMap<int32,float>_IntFloatMap;
	//
	UPROPERTY() TMap<int32,FString>_IntStringMap;
	UPROPERTY() TMap<int32,FName>_IntNameMap;
	UPROPERTY() TMap<int32,FText>_IntTextMap;
	//
	UPROPERTY() TMap<int32,FVector2D>_IntVector2DMap;
	UPROPERTY() TMap<int32,FVector>_IntVector3DMap;
	UPROPERTY() TMap<int32,FRotator>_IntRotatorMap;
	UPROPERTY() TMap<int32,FColor>_IntColorMap;
	UPROPERTY() TMap<int32,FTransform>_IntTransformMap;
	//
	UPROPERTY() TMap<int32,UClass*>_IntClassMap;
	UPROPERTY() TMap<int32,UObject*>_IntObjectMap;
	UPROPERTY() TMap<int32,AActor*>_IntActorMap;
	UPROPERTY() TMap<int32,UActorComponent*>_IntComponentMap;
public:
	UPROPERTY() TMap<int64,bool>_Int64BoolMap;
	UPROPERTY() TMap<int64,uint8>_Int64ByteMap;
	UPROPERTY() TMap<int64,int32>_Int64IntMap;
	UPROPERTY() TMap<int64,int64>_Int64Int64Map;
	UPROPERTY() TMap<int64,float>_Int64FloatMap;
	//
	UPROPERTY() TMap<int64,FString>_Int64StringMap;
	UPROPERTY() TMap<int64,FName>_Int64NameMap;
	UPROPERTY() TMap<int64,FText>_Int64TextMap;
	//
	UPROPERTY() TMap<int64,FVector2D>_Int64Vector2DMap;
	UPROPERTY() TMap<int64,FVector>_Int64Vector3DMap;
	UPROPERTY() TMap<int64,FRotator>_Int64RotatorMap;
	UPROPERTY() TMap<int64,FColor>_Int64ColorMap;
	UPROPERTY() TMap<int64,FTransform>_Int64TransformMap;
	//
	UPROPERTY() TMap<int64,UClass*>_Int64ClassMap;
	UPROPERTY() TMap<int64,UObject*>_Int64ObjectMap;
	UPROPERTY() TMap<int64,AActor*>_Int64ActorMap;
	UPROPERTY() TMap<int64,UActorComponent*>_Int64ComponentMap;
public:
	UPROPERTY() TMap<float,bool>_FloatBoolMap;
	UPROPERTY() TMap<float,uint8>_FloatByteMap;
	UPROPERTY() TMap<float,int32>_FloatIntMap;
	UPROPERTY() TMap<float,int64>_FloatInt64Map;
	UPROPERTY() TMap<float,float>_FloatFloatMap;
	//
	UPROPERTY() TMap<float,FString>_FloatStringMap;
	UPROPERTY() TMap<float,FName>_FloatNameMap;
	UPROPERTY() TMap<float,FText>_FloatTextMap;
	//
	UPROPERTY() TMap<float,FVector2D>_FloatVector2DMap;
	UPROPERTY() TMap<float,FVector>_FloatVector3DMap;
	UPROPERTY() TMap<float,FRotator>_FloatRotatorMap;
	UPROPERTY() TMap<float,FColor>_FloatColorMap;
	UPROPERTY() TMap<float,FTransform>_FloatTransformMap;
	//
	UPROPERTY() TMap<float,UClass*>_FloatClassMap;
	UPROPERTY() TMap<float,UObject*>_FloatObjectMap;
	UPROPERTY() TMap<float,AActor*>_FloatActorMap;
	UPROPERTY() TMap<float,UActorComponent*>_FloatComponentMap;
public:
	UPROPERTY() TMap<FString,bool>_StringBoolMap;
	UPROPERTY() TMap<FString,uint8>_StringByteMap;
	UPROPERTY() TMap<FString,int32>_StringIntMap;
	UPROPERTY() TMap<FString,int64>_StringInt64Map;
	UPROPERTY() TMap<FString,float>_StringFloatMap;
	//
	UPROPERTY() TMap<FString,FString>_StringStringMap;
	UPROPERTY() TMap<FString,FName>_StringNameMap;
	UPROPERTY() TMap<FString,FText>_StringTextMap;
	//
	UPROPERTY() TMap<FString,FVector2D>_StringVector2DMap;
	UPROPERTY() TMap<FString,FVector>_StringVector3DMap;
	UPROPERTY() TMap<FString,FRotator>_StringRotatorMap;
	UPROPERTY() TMap<FString,FColor>_StringColorMap;
	UPROPERTY() TMap<FString,FTransform>_StringTransformMap;
	//
	UPROPERTY() TMap<FString,UClass*>_StringClassMap;
	UPROPERTY() TMap<FString,UObject*>_StringObjectMap;
	UPROPERTY() TMap<FString,AActor*>_StringActorMap;
	UPROPERTY() TMap<FString,UActorComponent*>_StringComponentMap;
public:
	UPROPERTY() TMap<FName,bool>_NameBoolMap;
	UPROPERTY() TMap<FName,uint8>_NameByteMap;
	UPROPERTY() TMap<FName,int32>_NameIntMap;
	UPROPERTY() TMap<FName,int64>_NameInt64Map;
	UPROPERTY() TMap<FName,float>_NameFloatMap;
	//
	UPROPERTY() TMap<FName,FString>_NameStringMap;
	UPROPERTY() TMap<FName,FName>_NameNameMap;
	UPROPERTY() TMap<FName,FText>_NameTextMap;
	//
	UPROPERTY() TMap<FName,FVector2D>_NameVector2DMap;
	UPROPERTY() TMap<FName,FVector>_NameVector3DMap;
	UPROPERTY() TMap<FName,FRotator>_NameRotatorMap;
	UPROPERTY() TMap<FName,FColor>_NameColorMap;
	UPROPERTY() TMap<FName,FTransform>_NameTransformMap;
	//
	UPROPERTY() TMap<FName,UClass*>_NameClassMap;
	UPROPERTY() TMap<FName,AActor*>_NameActorMap;
	UPROPERTY() TMap<FName,UActorComponent*>_NameComponentMap;
public:
	UPROPERTY() TMap<FColor,bool>_ColorBoolMap;
	UPROPERTY() TMap<FColor,uint8>_ColorByteMap;
	UPROPERTY() TMap<FColor,int32>_ColorIntMap;
	UPROPERTY() TMap<FColor,int64>_ColorInt64Map;
	UPROPERTY() TMap<FColor,float>_ColorFloatMap;
	//
	UPROPERTY() TMap<FColor,FString>_ColorStringMap;
	UPROPERTY() TMap<FColor,FName>_ColorNameMap;
	UPROPERTY() TMap<FColor,FText>_ColorTextMap;
	//
	UPROPERTY() TMap<FColor,FVector2D>_ColorVector2DMap;
	UPROPERTY() TMap<FColor,FVector>_ColorVector3DMap;
	UPROPERTY() TMap<FColor,FRotator>_ColorRotatorMap;
	UPROPERTY() TMap<FColor,FColor>_ColorColorMap;
	UPROPERTY() TMap<FColor,FTransform>_ColorTransformMap;
	//
	UPROPERTY() TMap<FColor,UClass*>_ColorClassMap;
	UPROPERTY() TMap<FColor,UObject*>_ColorObjectMap;
	UPROPERTY() TMap<FColor,AActor*>_ColorActorMap;
	UPROPERTY() TMap<FColor,UActorComponent*>_ColorComponentMap;
public:
	UPROPERTY() TMap<FVector2D,bool>_Vector2DBoolMap;
	UPROPERTY() TMap<FVector2D,uint8>_Vector2DByteMap;
	UPROPERTY() TMap<FVector2D,int32>_Vector2DIntMap;
	UPROPERTY() TMap<FVector2D,int64>_Vector2DInt64Map;
	UPROPERTY() TMap<FVector2D,float>_Vector2DFloatMap;
	//
	UPROPERTY() TMap<FVector2D,FString>_Vector2DStringMap;
	UPROPERTY() TMap<FVector2D,FName>_Vector2DNameMap;
	UPROPERTY() TMap<FVector2D,FText>_Vector2DTextMap;
	//
	UPROPERTY() TMap<FVector2D,FVector2D>_Vector2DVector2DMap;
	UPROPERTY() TMap<FVector2D,FVector>_Vector2DVector3DMap;
	UPROPERTY() TMap<FVector2D,FRotator>_Vector2DRotatorMap;
	UPROPERTY() TMap<FVector2D,FColor>_Vector2DColorMap;
	UPROPERTY() TMap<FVector2D,FTransform>_Vector2DTransformMap;
	//
	UPROPERTY() TMap<FVector2D,UClass*>_Vector2DClassMap;
	UPROPERTY() TMap<FVector2D,UObject*>_Vector2DObjectMap;
	UPROPERTY() TMap<FVector2D,AActor*>_Vector2DActorMap;
	UPROPERTY() TMap<FVector2D,UActorComponent*>_Vector2DComponentMap;
public:
	UPROPERTY() TMap<FVector,bool>_Vector3DBoolMap;
	UPROPERTY() TMap<FVector,uint8>_Vector3DByteMap;
	UPROPERTY() TMap<FVector,int32>_Vector3DIntMap;
	UPROPERTY() TMap<FVector,int64>_Vector3DInt64Map;
	UPROPERTY() TMap<FVector,float>_Vector3DFloatMap;
	//
	UPROPERTY() TMap<FVector,FString>_Vector3DStringMap;
	UPROPERTY() TMap<FVector,FName>_Vector3DNameMap;
	UPROPERTY() TMap<FVector,FText>_Vector3DTextMap;
	//
	UPROPERTY() TMap<FVector,FVector2D>_Vector3DVector2DMap;
	UPROPERTY() TMap<FVector,FVector>_Vector3DVector3DMap;
	UPROPERTY() TMap<FVector,FRotator>_Vector3DRotatorMap;
	UPROPERTY() TMap<FVector,FColor>_Vector3DColorMap;
	UPROPERTY() TMap<FVector,FTransform>_Vector3DTransformMap;
	//
	UPROPERTY() TMap<FVector,UClass*>_Vector3DClassMap;
	UPROPERTY() TMap<FVector,UObject*>_Vector3DObjectMap;
	UPROPERTY() TMap<FVector,AActor*>_Vector3DActorMap;
	UPROPERTY() TMap<FVector,UActorComponent*>_Vector3DComponentMap;
public:
	UPROPERTY() TMap<UClass*,bool>_ClassBoolMap;
	UPROPERTY() TMap<UClass*,uint8>_ClassByteMap;
	UPROPERTY() TMap<UClass*,int32>_ClassIntMap;
	UPROPERTY() TMap<UClass*,int64>_ClassInt64Map;
	UPROPERTY() TMap<UClass*,float>_ClassFloatMap;
	//
	UPROPERTY() TMap<UClass*,FString>_ClassStringMap;
	UPROPERTY() TMap<UClass*,FName>_ClassNameMap;
	UPROPERTY() TMap<UClass*,FText>_ClassTextMap;
	//
	UPROPERTY() TMap<UClass*,FVector2D>_ClassVector2DMap;
	UPROPERTY() TMap<UClass*,FVector>_ClassVector3DMap;
	UPROPERTY() TMap<UClass*,FRotator>_ClassRotatorMap;
	UPROPERTY() TMap<UClass*,FColor>_ClassColorMap;
	UPROPERTY() TMap<UClass*,FTransform>_ClassTransformMap;
	//
	UPROPERTY() TMap<UClass*,UClass*>_ClassClassMap;
	UPROPERTY() TMap<UClass*,UObject*>_ClassObjectMap;
	UPROPERTY() TMap<UClass*,AActor*>_ClassActorMap;
	UPROPERTY() TMap<UClass*,UActorComponent*>_ClassComponentMap;
public:
	UPROPERTY() TMap<UObject*,bool>_ObjectBoolMap;
	UPROPERTY() TMap<UObject*,uint8>_ObjectByteMap;
	UPROPERTY() TMap<UObject*,int32>_ObjectIntMap;
	UPROPERTY() TMap<UObject*,int64>_ObjectInt64Map;
	UPROPERTY() TMap<UObject*,float>_ObjectFloatMap;
	//
	UPROPERTY() TMap<UObject*,FString>_ObjectStringMap;
	UPROPERTY() TMap<UObject*,FName>_ObjectNameMap;
	UPROPERTY() TMap<UObject*,FText>_ObjectTextMap;
	//
	UPROPERTY() TMap<UObject*,FVector2D>_ObjectVector2DMap;
	UPROPERTY() TMap<UObject*,FVector>_ObjectVector3DMap;
	UPROPERTY() TMap<UObject*,FRotator>_ObjectRotatorMap;
	UPROPERTY() TMap<UObject*,FColor>_ObjectColorMap;
	UPROPERTY() TMap<UObject*,FTransform>_ObjectTransformMap;
	//
	UPROPERTY() TMap<UObject*,UClass*>_ObjectClassMap;
	UPROPERTY() TMap<UObject*,UObject*>_ObjectObjectMap;
	UPROPERTY() TMap<UObject*,AActor*>_ObjectActorMap;
	UPROPERTY() TMap<UObject*,UActorComponent*>_ObjectComponentMap;
public:
	UPROPERTY() TMap<AActor*,bool>_ActorBoolMap;
	UPROPERTY() TMap<AActor*,uint8>_ActorByteMap;
	UPROPERTY() TMap<AActor*,int32>_ActorIntMap;
	UPROPERTY() TMap<AActor*,int64>_ActorInt64Map;
	UPROPERTY() TMap<AActor*,float>_ActorFloatMap;
	//
	UPROPERTY() TMap<AActor*,FString>_ActorStringMap;
	UPROPERTY() TMap<AActor*,FName>_ActorNameMap;
	UPROPERTY() TMap<AActor*,FText>_ActorTextMap;
	//
	UPROPERTY() TMap<AActor*,FVector2D>_ActorVector2DMap;
	UPROPERTY() TMap<AActor*,FVector>_ActorVector3DMap;
	UPROPERTY() TMap<AActor*,FRotator>_ActorRotatorMap;
	UPROPERTY() TMap<AActor*,FColor>_ActorColorMap;
	UPROPERTY() TMap<AActor*,FTransform>_ActorTransformMap;
	//
	UPROPERTY() TMap<AActor*,UClass*>_ActorClassMap;
	UPROPERTY() TMap<AActor*,UObject*>_ActorObjectMap;
	UPROPERTY() TMap<AActor*,AActor*>_ActorActorMap;
	UPROPERTY() TMap<AActor*,UActorComponent*>_ActorComponentMap;
public:
	UPROPERTY() TMap<UActorComponent*,bool>_ComponentBoolMap;
	UPROPERTY() TMap<UActorComponent*,uint8>_ComponentByteMap;
	UPROPERTY() TMap<UActorComponent*,int32>_ComponentIntMap;
	UPROPERTY() TMap<UActorComponent*,int64>_ComponentInt64Map;
	UPROPERTY() TMap<UActorComponent*,float>_ComponentFloatMap;
	//
	UPROPERTY() TMap<UActorComponent*,FString>_ComponentStringMap;
	UPROPERTY() TMap<UActorComponent*,FName>_ComponentNameMap;
	UPROPERTY() TMap<UActorComponent*,FText>_ComponentTextMap;
	//
	UPROPERTY() TMap<UActorComponent*,FVector2D>_ComponentVector2DMap;
	UPROPERTY() TMap<UActorComponent*,FVector>_ComponentVector3DMap;
	UPROPERTY() TMap<UActorComponent*,FRotator>_ComponentRotatorMap;
	UPROPERTY() TMap<UActorComponent*,FColor>_ComponentColorMap;
	UPROPERTY() TMap<UActorComponent*,FTransform>_ComponentTransformMap;
	//
	UPROPERTY() TMap<UActorComponent*,UClass*>_ComponentClassMap;
	UPROPERTY() TMap<UActorComponent*,UObject*>_ComponentObjectMap;
	UPROPERTY() TMap<UActorComponent*,AActor*>_ComponentActorMap;
	UPROPERTY() TMap<UActorComponent*,UActorComponent*>_ComponentComponentMap;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Node Runtime Wrapper:

/* Scriptable Node for Blueprint Graphs (C# Scripts) */
UCLASS(classGroup="Synaptech", Category="MagicNode", hidedropdown, hideCategories=("Activation","Variable"), meta=(DisplayName="(C#) Magic Node"))
class MAGICNODESHARP_API UMagicNodeSharp : public UObject, public FTickableGameObject, public IMonoObject {
	GENERATED_BODY()
	//
	UMagicNodeSharp();
	//
	friend class IMonoCore;
	friend class ICALL::API;
	friend class IMonoKismet;
	friend class IMonoObject;
	//
	#if WITH_EDITORONLY_DATA
	 friend class UKCS_MagicNode;
	#endif
private:
	UWorld* World;
private:
	#if WITH_EDITORONLY_DATA
	TWeakObjectPtr<UMagicNodeSharpSource>Source;
	#endif
protected:
	void OnExit();
	void OnLoad();
	void OnStart();
	void OnExecute();
protected:
	int32 OnUpdate(float DeltaTime);
protected:
	void ExitMonoPlay();
public:
	void CleanupOnQuit();
public:
	void Register(UWorld* InWorld);
public:
	FGuid GetNodeID() const;
	MonoClass* GetMonoClass() const;
	MonoObject* GetMonoObject() const;
	MonoProperty* GetMonoProperty(const FName &Name);
public:
	virtual bool IsTickable() const override;
	virtual UWorld* GetWorld() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
public:
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaTime) override;
protected:
	UFUNCTION() void Throw_MonoException();
	UFUNCTION() void Throw_MonoError(const FString &Error);


public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static UMagicNodeSharp* LoadScript(UObject* Context, UMagicNodeSharpSource* Script);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void PostLoadScript(UObject* Context, UMagicNodeSharp* Node);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void Execute(UObject* Context, UMagicNodeSharp* Node);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void Exit(UObject* Context, UMagicNodeSharp* Node);


public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, bool Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, bool &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, uint8 Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, uint8 &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, int32 Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, int32 &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, int64 Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, int64 &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, float Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, float &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FString Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, FString &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FName Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, FName &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FText Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, FText &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FVector2D Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, FVector2D &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FVector Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, FVector &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FRotator Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, FRotator &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FColor Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, FColor &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const FTransform Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field, FTransform &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, UClass* Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, UClass* &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, UObject* Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, UObject* &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, AActor* Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, AActor* &Output);
public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoProperty_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, UActorComponent* Input);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoProperty_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, UActorComponent* &Output);


public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoArray_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TArray<UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoArray_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, TArray<UActorComponent*>&Output);


public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_String(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoSet_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TSet<UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoSet_Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, TSet<UActorComponent*>&Output);


public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ByteComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<uint8,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ByteComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<uint8,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_IntComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int32,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_IntComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int32,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Bool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Byte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Int(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Int64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Float(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64String(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64String(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Name(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Text(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Color(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Vector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Vector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Rotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Transform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Class(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Object(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Actor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Int64Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<int64,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Int64Component(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<int64,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_FloatComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<float,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_FloatComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<float,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_StringComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FString,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_StringComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FString,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_NameComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FName,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_NameComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FName,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector2DComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector2D,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector2DComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector2D,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_Vector3DComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FVector,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_Vector3DComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FVector,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ColorComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<FColor,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ColorComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<FColor,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ClassComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UClass*,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ClassComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UClass*,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ObjectComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UObject*,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ObjectComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UObject*,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ActorComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<AActor*,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ActorComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<AActor*,UActorComponent*>&Output);

public:
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,bool>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentBool(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,bool>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,uint8>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentByte(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,uint8>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,int32>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentInt(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,int32>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,int64>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentInt64(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,int64>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,float>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentFloat(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,float>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentString(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,FString>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentString(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FString>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentName(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,FName>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentName(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FName>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentText(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,FText>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentText(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FText>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,FColor>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentColor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FColor>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,FVector2D>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentVector2D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FVector2D>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,FVector>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentVector3D(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FVector>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,FRotator>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentRotator(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FRotator>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,FTransform>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentTransform(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,FTransform>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,UClass*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentClass(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,UClass*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,UObject*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentObject(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,UObject*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,AActor*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentActor(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,AActor*>&Output);
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,CustomThunk,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void SetMonoMap_ComponentComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, const TMap<UActorComponent*,UActorComponent*>&Input){}
	//
	UFUNCTION(Category="MagicNode",BlueprintCallable,meta=(WorldContext="Context",BlueprintInternalUseOnly=true))
	static void GetMonoMap_ComponentComponent(UObject* Context, UMagicNodeSharp* Node, const FName Field, TMap<UActorComponent*,UActorComponent*>&Output);


public:
	DECLARE_FUNCTION(execSetMonoArray_Bool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Byte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Int) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Int64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Float) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_String) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Name) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Text) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Vector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Vector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Rotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Color) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Transform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Class) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Object) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Actor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoArray_Component) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FArrayProperty*_Array = CastField<FArrayProperty>(Stack.MostRecentProperty);
		//
		if (!_Array) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoArrayValue(_Node,_Field,_Array,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoSet_Byte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Int) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Int64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Float) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_String) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Name) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Vector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Vector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Color) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Class) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Object) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Actor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoSet_Component) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FSetProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FSetProperty*_Set = CastField<FSetProperty>(Stack.MostRecentProperty);
		//
		if (!_Set) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoSetValue(_Node,_Field,_Set,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_ByteBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ByteComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_IntBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_IntComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_Int64Bool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Byte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Int) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Int64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Float) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64String) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Name) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Text) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Color) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Vector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Vector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Rotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Transform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Class) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Object) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Actor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Int64Component) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_FloatBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_FloatComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_StringBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_StringComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_NameBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_NameComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_Vector2DBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector2DComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_Vector3DBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_Vector3DComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_ColorBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ColorComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_ClassBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ClassComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_ObjectBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ObjectComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_ActorBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ActorComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


public:
	DECLARE_FUNCTION(execSetMonoMap_ComponentBool) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentByte) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentInt) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentInt64) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentFloat) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentString) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentName) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentText) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentColor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentVector2D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentVector3D) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentRotator) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentTransform) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentClass) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentObject) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentActor) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///
	//
	DECLARE_FUNCTION(execSetMonoMap_ComponentComponent) {
		P_GET_OBJECT(UObject,_Context);
		P_GET_OBJECT(UMagicNodeSharp,_Node);
		P_GET_PROPERTY(FNameProperty,_Field);
		//
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void*_Address = Stack.MostRecentPropertyAddress;
		FMapProperty*_Map = CastField<FMapProperty>(Stack.MostRecentProperty);
		//
		if (!_Map) {Stack.bArrayContextFailed=true; return;}
		//
		P_FINISH;
		//
		P_NATIVE_BEGIN;
		  SET_MonoMapValue(_Node,_Field,_Map,_Address);
		P_NATIVE_END;
	}///


#if WITH_EDITOR
public:
	static FOnScriptRuntimeException OnScriptRuntimeException;
#endif
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Node Script Wrapper:

/* Scriptable Node Source File (C# Scripts) */
UCLASS(classGroup="Synaptech", Category="MagicNode", hidedropdown, hideCategories=("Activation","Variable"), meta=(DisplayName="(C#) Magic Node Script"))
class MAGICNODESHARP_API UMagicNodeSharpSource : public UObject {
	GENERATED_BODY()
	//
	UMagicNodeSharpSource();
	virtual ~UMagicNodeSharpSource();
	//
	friend class IMonoKismet;
	friend class UMagicNodeSharp;
	friend class SDIFFMainWidget;
private:
	void AssertScriptClass();
	void AssertScriptParentClass();
public:
	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& AR) override;
public:
	FString GetScriptName() const;
////
#if WITH_EDITORONLY_DATA
private:
	bool IsSaving;
	bool DataInitialized;
private:
	FDelegateHandle WatcherHandle;
protected:
	UPROPERTY() FString Source;
	UPROPERTY() FMonoScriptData ScriptData;
	UPROPERTY() FMonoClassDefinition CompiledClass;
public:
	UPROPERTY(Category="MagicNode",EditDefaultsOnly)
	FLinearColor NodeColor;
	//
	UPROPERTY(Category="MagicNode",EditDefaultsOnly)
	bool LockSourceCode;
public:
	UPROPERTY(Category="Solution",EditDefaultsOnly)
	TArray<UMagicNodeSharpSource*>Include;
	//
	UPROPERTY(Category="Library",EditDefaultsOnly)
	TArray<FFilePath>References;
#endif
////
#if WITH_EDITOR
protected:
	void OnAssetPreDeleted(const TArray<UObject*>&Objects);
	void OnProjectDirectoryChanged(const TArray<FFileChangeData>&Data);
public:
	bool ExportFile();
	bool LoadScript();
	bool DestroyScript();
public:
	void SetSource(const FString &NewText);
public:
	void LoadIntermediateData();
	bool ExportIntermediateData();
	bool WriteIntermediateData(const FMonoScriptData &CompilerData);
public:
	FString GetSource() const;
	FString GetDataFullPath() const;
	FString GetScriptFullPath() const;
public:
	const FMonoScriptData &GetScriptData() const;
	const FMonoClassDefinition &GetClassDefinition() const;
public:
	static FString LoadFileToFString(const TCHAR* FullPath);
public:
	static FOnScriptSourceDeleted OnScriptSourceDeleted;
	static FOnScriptSourceExported OnScriptSourceExported;
#endif
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////