//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MCS_Types.h"
#include "MCS_ManagedTypes.h"

#include "MCS_MonoCall.h"
#include "MCS_MonoThread.h"
#include "MCS_MonoUtility.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UMagicNodeSharp;
class IMonoKismet;
class IMonoCore;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IMonoObject {
	friend class IMonoCore;
	friend class IMonoKismet;
protected:
	bool IsAsyncTask;
	EMonoThread MonoThread;
protected:
	CSHANDLE OBJHandle;
protected:
	MonoObject* Except;
	MonoObject* Update;
protected:
	MonoMethod* MonoOnLoad;
	MonoMethod* MonoOnExit;
	MonoMethod* MonoOnStart;
	MonoMethod* MonoOnUpdate;
	MonoMethod* MonoOnExecute;
protected:
	inline static void ThreadExit(const UPTRINT IntPtr);
	inline static void ThreadStart(const UPTRINT IntPtr);
	inline static void ThreadUpdate(const UPTRINT IntPtr);

protected:
	MAGICNODESHARP_API static void SetupThreadingAttribute(UMagicNodeSharp* Node, MonoClass* NodeClass);
	MAGICNODESHARP_API static void SetupManagedMethods(UMagicNodeSharp* Node, MonoObject* ManagedNode);
	MAGICNODESHARP_API static void SetupManagedHandle(UMagicNodeSharp* Node, MonoObject* ManagedNode);
public:
	MAGICNODESHARP_API static FGuid PtrToGuid(UMagicNodeSharp* Ptr);
	MAGICNODESHARP_API static UMagicNodeSharp* GuidToPtr(const FGuid &Guid);
protected:
	MAGICNODESHARP_API static bool ValidateParentClass(MonoClass* ManagedClass);

protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Bool(UMagicNodeSharp* Node, const FName &Field, bool Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Bool(UMagicNodeSharp* Node, const FName &Field, bool &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Byte(UMagicNodeSharp* Node, const FName &Field, uint8 Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Byte(UMagicNodeSharp* Node, const FName &Field, uint8 &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Int(UMagicNodeSharp* Node, const FName &Field, int32 Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Int(UMagicNodeSharp* Node, const FName &Field, int32 &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Int64(UMagicNodeSharp* Node, const FName &Field, int64 Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Int64(UMagicNodeSharp* Node, const FName &Field, int64 &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Float(UMagicNodeSharp* Node, const FName &Field, float Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Float(UMagicNodeSharp* Node, const FName &Field, float &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_String(UMagicNodeSharp* Node, const FName &Field, const FString &Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_String(UMagicNodeSharp* Node, const FName &Field, FString &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Name(UMagicNodeSharp* Node, const FName &Field, const FName &Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Name(UMagicNodeSharp* Node, const FName &Field, FName &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Text(UMagicNodeSharp* Node, const FName &Field, const FText &Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Text(UMagicNodeSharp* Node, const FName &Field, FText &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Color(UMagicNodeSharp* Node, const FName &Field, const FColor &Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Color(UMagicNodeSharp* Node, const FName &Field, FColor &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Vector2D(UMagicNodeSharp* Node, const FName &Field, const FVector2D &Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Vector2D(UMagicNodeSharp* Node, const FName &Field, FVector2D &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Vector3D(UMagicNodeSharp* Node, const FName &Field, const FVector &Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Vector3D(UMagicNodeSharp* Node, const FName &Field, FVector &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Rotator(UMagicNodeSharp* Node, const FName &Field, const FRotator &Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Rotator(UMagicNodeSharp* Node, const FName &Field, FRotator &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Transform(UMagicNodeSharp* Node, const FName &Field, const FTransform &Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Transform(UMagicNodeSharp* Node, const FName &Field, FTransform &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Class(UMagicNodeSharp* Node, const FName &Field, UClass* Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Class(UMagicNodeSharp* Node, const FName &Field, UClass* &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Object(UMagicNodeSharp* Node, const FName &Field, UObject* Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Object(UMagicNodeSharp* Node, const FName &Field, UObject* &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Actor(UMagicNodeSharp* Node, const FName &Field, AActor* Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Actor(UMagicNodeSharp* Node, const FName &Field, AActor* &Output);
protected:
	MAGICNODESHARP_API static void SET_MonoPropertyValue_Component(UMagicNodeSharp* Node, const FName &Field, UActorComponent* Input);
	MAGICNODESHARP_API static void GET_MonoPropertyValue_Component(UMagicNodeSharp* Node, const FName &Field, UActorComponent* &Output);


protected:
	MAGICNODESHARP_API static void SET_MonoArrayValue(UMagicNodeSharp* Node, const FName &Field, FArrayProperty* Property, void* Address);
	//
	MAGICNODESHARP_API static void GET_MonoArrayValue_Bool(UMagicNodeSharp* Node, const FName &Field, TArray<bool>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Byte(UMagicNodeSharp* Node, const FName &Field, TArray<uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Int(UMagicNodeSharp* Node, const FName &Field, TArray<int32>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Int64(UMagicNodeSharp* Node, const FName &Field, TArray<int64>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Float(UMagicNodeSharp* Node, const FName &Field, TArray<float>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_String(UMagicNodeSharp* Node, const FName &Field, TArray<FString>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Name(UMagicNodeSharp* Node, const FName &Field, TArray<FName>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Text(UMagicNodeSharp* Node, const FName &Field, TArray<FText>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Color(UMagicNodeSharp* Node, const FName &Field, TArray<FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Vector2D(UMagicNodeSharp* Node, const FName &Field, TArray<FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Vector3D(UMagicNodeSharp* Node, const FName &Field, TArray<FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Rotator(UMagicNodeSharp* Node, const FName &Field, TArray<FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Transform(UMagicNodeSharp* Node, const FName &Field, TArray<FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Class(UMagicNodeSharp* Node, const FName &Field, TArray<UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Object(UMagicNodeSharp* Node, const FName &Field, TArray<UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Actor(UMagicNodeSharp* Node, const FName &Field, TArray<AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoArrayValue_Component(UMagicNodeSharp* Node, const FName &Field, TArray<UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void SET_MonoSetValue(UMagicNodeSharp* Node, const FName &Field, FSetProperty* Property, void* Address);
	//
	MAGICNODESHARP_API static void GET_MonoSetValue_Byte(UMagicNodeSharp* Node, const FName &Field, TSet<uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Int(UMagicNodeSharp* Node, const FName &Field, TSet<int32>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Int64(UMagicNodeSharp* Node, const FName &Field, TSet<int64>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Float(UMagicNodeSharp* Node, const FName &Field, TSet<float>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_String(UMagicNodeSharp* Node, const FName &Field, TSet<FString>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Name(UMagicNodeSharp* Node, const FName &Field, TSet<FName>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Color(UMagicNodeSharp* Node, const FName &Field, TSet<FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Vector2D(UMagicNodeSharp* Node, const FName &Field, TSet<FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Vector3D(UMagicNodeSharp* Node, const FName &Field, TSet<FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Class(UMagicNodeSharp* Node, const FName &Field, TSet<UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Object(UMagicNodeSharp* Node, const FName &Field, TSet<UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Actor(UMagicNodeSharp* Node, const FName &Field, TSet<AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoSetValue_Component(UMagicNodeSharp* Node, const FName &Field, TSet<UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void SET_MonoMapValue(UMagicNodeSharp* Node, const FName &Field, FMapProperty* Property, void* Address);
	//
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteBool(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteByte(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteInt(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteInt64(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteFloat(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteString(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteName(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteText(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteColor(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteRotator(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteTransform(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteClass(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteObject(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteActor(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ByteComponent(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_IntBool(UMagicNodeSharp* Node, const FName &Field, TMap<int32,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntByte(UMagicNodeSharp* Node, const FName &Field, TMap<int32,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntInt(UMagicNodeSharp* Node, const FName &Field, TMap<int32,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntInt64(UMagicNodeSharp* Node, const FName &Field, TMap<int32,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntFloat(UMagicNodeSharp* Node, const FName &Field, TMap<int32,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntString(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntName(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntText(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntColor(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntRotator(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntTransform(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntClass(UMagicNodeSharp* Node, const FName &Field, TMap<int32,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntObject(UMagicNodeSharp* Node, const FName &Field, TMap<int32,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntActor(UMagicNodeSharp* Node, const FName &Field, TMap<int32,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_IntComponent(UMagicNodeSharp* Node, const FName &Field, TMap<int32,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Bool(UMagicNodeSharp* Node, const FName &Field, TMap<int64,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Byte(UMagicNodeSharp* Node, const FName &Field, TMap<int64,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Int(UMagicNodeSharp* Node, const FName &Field, TMap<int64,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Int64(UMagicNodeSharp* Node, const FName &Field, TMap<int64,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Float(UMagicNodeSharp* Node, const FName &Field, TMap<int64,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64String(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Name(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Text(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Color(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Vector2D(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Vector3D(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Rotator(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Transform(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Class(UMagicNodeSharp* Node, const FName &Field, TMap<int64,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Object(UMagicNodeSharp* Node, const FName &Field, TMap<int64,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Actor(UMagicNodeSharp* Node, const FName &Field, TMap<int64,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Int64Component(UMagicNodeSharp* Node, const FName &Field, TMap<int64,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatBool(UMagicNodeSharp* Node, const FName &Field, TMap<float,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatByte(UMagicNodeSharp* Node, const FName &Field, TMap<float,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatInt(UMagicNodeSharp* Node, const FName &Field, TMap<float,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatInt64(UMagicNodeSharp* Node, const FName &Field, TMap<float,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatFloat(UMagicNodeSharp* Node, const FName &Field, TMap<float,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatString(UMagicNodeSharp* Node, const FName &Field, TMap<float,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatName(UMagicNodeSharp* Node, const FName &Field, TMap<float,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatText(UMagicNodeSharp* Node, const FName &Field, TMap<float,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatColor(UMagicNodeSharp* Node, const FName &Field, TMap<float,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<float,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<float,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatRotator(UMagicNodeSharp* Node, const FName &Field, TMap<float,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatTransform(UMagicNodeSharp* Node, const FName &Field, TMap<float,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatClass(UMagicNodeSharp* Node, const FName &Field, TMap<float,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatObject(UMagicNodeSharp* Node, const FName &Field, TMap<float,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatActor(UMagicNodeSharp* Node, const FName &Field, TMap<float,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_FloatComponent(UMagicNodeSharp* Node, const FName &Field, TMap<float,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_StringBool(UMagicNodeSharp* Node, const FName &Field, TMap<FString,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringByte(UMagicNodeSharp* Node, const FName &Field, TMap<FString,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringInt(UMagicNodeSharp* Node, const FName &Field, TMap<FString,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FString,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FString,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringString(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringName(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringText(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringColor(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringClass(UMagicNodeSharp* Node, const FName &Field, TMap<FString,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringObject(UMagicNodeSharp* Node, const FName &Field, TMap<FString,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringActor(UMagicNodeSharp* Node, const FName &Field, TMap<FString,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_StringComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FString,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_NameBool(UMagicNodeSharp* Node, const FName &Field, TMap<FName,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameByte(UMagicNodeSharp* Node, const FName &Field, TMap<FName,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameInt(UMagicNodeSharp* Node, const FName &Field, TMap<FName,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FName,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FName,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameString(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameName(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameText(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameColor(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameClass(UMagicNodeSharp* Node, const FName &Field, TMap<FName,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameObject(UMagicNodeSharp* Node, const FName &Field, TMap<FName,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameActor(UMagicNodeSharp* Node, const FName &Field, TMap<FName,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_NameComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FName,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DBool(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DByte(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DInt(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DString(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DName(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DText(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DColor(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DClass(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DObject(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DActor(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector2DComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DBool(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DByte(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DInt(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DString(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DName(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DText(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DColor(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DClass(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DObject(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DActor(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_Vector3DComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorBool(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorByte(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorInt(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorString(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorName(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorText(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorColor(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorClass(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorObject(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorActor(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ColorComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassBool(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassByte(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassInt(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassInt64(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassFloat(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassString(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassName(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassText(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassColor(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassRotator(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassTransform(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassClass(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassObject(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassActor(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ClassComponent(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectBool(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectByte(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectInt(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectInt64(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectFloat(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectString(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectName(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectText(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectColor(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectRotator(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectTransform(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectClass(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectObject(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectActor(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ObjectComponent(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorBool(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorByte(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorInt(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorInt64(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorFloat(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorString(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorName(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorText(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorColor(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorRotator(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorTransform(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorClass(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorObject(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorActor(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ActorComponent(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,UActorComponent*>&Output);


protected:
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentBool(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,bool>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentByte(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,uint8>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentInt(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,int32>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentInt64(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,int64>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentFloat(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,float>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentString(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FString>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentName(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FName>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentText(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FText>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentColor(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FColor>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FVector2D>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FVector>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentRotator(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FRotator>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentTransform(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FTransform>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentClass(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,UClass*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentObject(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,UObject*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentActor(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,AActor*>&Output);
	MAGICNODESHARP_API static void GET_MonoMapValue_ComponentComponent(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,UActorComponent*>&Output);

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////