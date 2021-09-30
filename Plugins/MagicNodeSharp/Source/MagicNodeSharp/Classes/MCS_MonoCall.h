//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MCS_Types.h"
#include "MCS_MonoThread.h"
#include "MCS_MonoUtility.h"
#include "MCS_ManagedTypes.h"

#include "mono/metadata/image.h"
#include "mono/metadata/class.h"
#include "mono/metadata/object.h"
#include "mono/metadata/loader.h"
#include "mono/metadata/reflection.h"

#include "Runtime/Core/Public/Math/UnrealMathUtility.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetArrayLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CS_API(Address,Method) mono_add_internal_call(StringCast<ANSICHAR>(*FString::Printf(TEXT("Unreal.API::%s"),TEXT(Address))).Get(),&Method)
#define CS_ICALL(Namespace,Address,Method) mono_add_internal_call(StringCast<ANSICHAR>(*FString::Printf(TEXT("%s::%s"),TEXT(Namespace),TEXT(Address))).Get(),&Method)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UMagicNodeSharp;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace ICALL {
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class API {
public:
	/// :: UnrealEngine.cs ::
	//
	static void LogError(MonoString* Message);
	static void LogWarning(MonoString* Message);
	static void LogMessage(MonoString* Message);
	static void PrintMessage(MonoString* Message);
	//
	static void DestroyNativeObject(UObject* OBJ);
	static bool IsNativeObjectValid(UObject* OBJ);
	static bool IsNativeClassValid(UClass* Class);
public:
	/// :: EngineFramework.cs ::
	//
	static UClass* LoadObjectClass(MonoString* ClassPath);
	static UObject* CreateNewObject(FMonoObject Outer, FMonoClass Class);
public:
	/// :: MagicNode.cs ::
	//
	static bool IsEditor();
	static bool IsShipping();
	static bool IsDevelopment();
	static bool IsStandalone(UMagicNodeSharp* Node);
	//
	static bool IsServer(UMagicNodeSharp* Node);
	static bool IsClient(UMagicNodeSharp* Node);
	static bool IsDedicatedServer(UMagicNodeSharp* Node);
	//
	static bool IsGameThread();
	static bool HasAuthority(UMagicNodeSharp* Node);
	//
	static void QuitGame();
	//
	static void DestroyNativeNode(UMagicNodeSharp* Node);
	//
	static UObject* GetNativeContext(UMagicNodeSharp* Node);
	//
	static void ConsoleCommand(UMagicNodeSharp* Node, MonoString* Command);
	static void InvokeFunction(UMagicNodeSharp* Node, MonoString* MethodName, MonoString* Params);
public:
	/// :: FString.cs ::
	//
	static FMonoString FString_Empty(FMonoString Self);
	static FMonoString FString_ToLower(FMonoString Self);
	static FMonoString FString_ToUpper(FMonoString Self);
	static FMonoString FString_TrimEnd(FMonoString Self);
	static FMonoString FString_TrimStart(FMonoString Self);
	static FMonoString FString_TrimStartAndEnd(FMonoString Self);
	static FMonoString FString_FromInt(FMonoString Self, int32 Number);
	static FMonoString FString_Append(FMonoString Self, FMonoString Other);
	static FMonoString FString_Mid(FMonoString Self, int32 Start, int32 Count);
	static FMonoString FString_RemoveAt(FMonoString Self, int32 Start, int32 Count);
	static FMonoString FString_InsertAt(FMonoString Self, int32 Index, FMonoString Characters);
	static FMonoString FString_RemoveFromEnd(FMonoString Self, FMonoString Suffix, EMonoSearchCase Case);
	static FMonoString FString_RemoveFromStart(FMonoString Self, FMonoString Prefix, EMonoSearchCase Case);
	static FMonoString FString_Replace(FMonoString Self, FMonoString From, FMonoString To, EMonoSearchCase Case);
	//
	static int32 FString_Compare(FMonoString A, FMonoString B, EMonoSearchCase Case);
	static int32 FString_Find(FMonoString Self, int32 SearchFrom, FMonoString SubString, EMonoSearchCase Case, EMonoSearchDir Direction);
	//
	static bool FString_IsEmpty(FMonoString Self);
	static bool FString_IsNumeric(FMonoString Self);
	static bool FString_IsValidIndex(FMonoString Self, int32 Index);
	static bool FString_FindChar(FMonoString Self, int32 Search, int32 FromIndex);
	static bool FString_Equals(FMonoString Self, FMonoString Other, EMonoSearchCase Case);
	static bool FString_Contains(FMonoString Self, FMonoString Other, EMonoSearchCase Case);
	static bool FString_EndsWith(FMonoString Self, FMonoString Suffix, EMonoSearchCase Case);
	static bool FString_StartsWith(FMonoString Self, FMonoString Prefix, EMonoSearchCase Case);
public:
	/// :: FName.cs ::
	//
	static FMonoName FName_AppendString(FMonoName Self, FMonoString Other);
	//
	static bool FName_IsEqual(FMonoName Self, FMonoName Other, EMonoSearchCase Case);
	static bool FName_IsValid(FMonoName Self);
	static bool FName_IsNone(FMonoName Self);
public:
	/// :: FText.cs ::
	//
	static FMonoText FText_AsDate(int64 Ticks, EMonoDateStyle DateStyle, FMonoString TimeZone);
	static FMonoText FText_AsCurrency(int32 Value, FMonoString CurrencyCode);
	static FMonoText FText_AsCultureInvariant(FMonoString Text);
	static FMonoText FText_ToLower(FMonoText Self);
	static FMonoText FText_ToUpper(FMonoText Self);
	static FMonoText FText_AsPercent(float Value);
	static FMonoText FText_AsNumber(int32 Value);
	//
	static bool FText_EqualTo(FMonoText Self, FMonoText Other);
	static bool FText_IsNumeric(FMonoText Self);
	static bool FText_IsEmpty(FMonoText Self);
public:
	/// :: FColor.cs ::
	//
	static FMonoColor FColor_FromHex(FMonoColor Self, FMonoString HexString);
	static FMonoString FColor_ToHex(FMonoColor Self);
	//
	static bool FColor_IsEqual(FMonoColor Self, FMonoColor Other);
public:
	/// :: FVector2D.cs ::
	//
	static FMonoVector2D FVector2D_ClampAxes(FMonoVector2D Self, float MinAxisVal, float MaxAxisVal);
	static FMonoVector2D FVector2D_GetSafeNormal(FMonoVector2D Self, float Tolerance);
	static FMonoVector2D FVector2D_Normalize(FMonoVector2D Self, float Tolerance);
	static FMonoVector2D FVector2D_SphericalToUnitCartesian(FMonoVector2D Self);
	static FMonoVector2D FVector2D_Max(FMonoVector2D Self, FMonoVector2D Other);
	static FMonoVector2D FVector2D_Min(FMonoVector2D Self, FMonoVector2D Other);
	static FMonoVector2D FVector2D_GetRotated(FMonoVector2D Self, float Angle);
	static FMonoVector2D FVector2D_GetSignVector(FMonoVector2D Self);
	static FMonoVector2D FVector2D_RoundToVector(FMonoVector2D Self);
	static FMonoVector2D FVector2D_ToDirection(FMonoVector2D Self);
	static FMonoVector2D FVector2D_GetAbs(FMonoVector2D Self);
	//
	static float FVector2D_CrossProduct(FMonoVector2D Self, FMonoVector2D Other);
	static float FVector2D_DistSquared(FMonoVector2D Self, FMonoVector2D Other);
	static float FVector2D_DotProduct(FMonoVector2D Self, FMonoVector2D Other);
	static float FVector2D_Distance(FMonoVector2D Self, FMonoVector2D Other);
	static float FVector2D_SizeSquared(FMonoVector2D Self);
	static float FVector2D_GetAbsMax(FMonoVector2D Self);
	static float FVector2D_GetMax(FMonoVector2D Self);
	static float FVector2D_GetMin(FMonoVector2D Self);
	static float FVector2D_Size(FMonoVector2D Self);
	//
	static bool FVector2D_Equals(FMonoVector2D Self, FMonoVector2D Other);
	static bool FVector2D_IsNearlyZero(FMonoVector2D Self);
	static bool FVector2D_ContainsNaN(FMonoVector2D Self);
	static bool FVector2D_IsZero(FMonoVector2D Self);
public:
	/// :: FVector3D.cs ::
	//
	static FMonoVector3D FVector3D_CreateOrthonormalBasis(FMonoVector3D XAxis, FMonoVector3D YAxis, FMonoVector3D ZAxis);
	static FMonoVector3D FVector3D_BoundToBox(FMonoVector3D Self, FMonoVector3D Min, FMonoVector3D Max);
	static FMonoVector3D FVector3D_RotateAngleAxis(FMonoVector3D Self, float Angle, FMonoVector3D Axis);
	static FMonoVector3D FVector3D_AddBounded(FMonoVector3D Self, FMonoVector3D Other, float Radius);
	static FMonoVector3D FVector3D_VectorPlaneProject(FMonoVector3D Self, FMonoVector3D PlaneNormal);
	static FMonoVector3D FVector3D_MirrorByVector(FMonoVector3D Self, FMonoVector3D MirrorNormal);
	static FMonoVector3D FVector3D_ProjectOnToNormal(FMonoVector3D Self, FMonoVector3D Normal);
	static FMonoVector3D FVector3D_GetClampedToSize(FMonoVector3D Self, float Min, float Max);
	static FMonoVector3D FVector3D_GetClampedToMaxSize(FMonoVector3D Self, float MaxSize);
	static FMonoVector3D FVector3D_CrossProduct(FMonoVector3D Self, FMonoVector3D Other);
	static FMonoVector3D FVector3D_ProjectOnTo(FMonoVector3D Self, FMonoVector3D Other);
	static FMonoVector3D FVector3D_GetSafeNormal(FMonoVector3D Self, float Tolerance);
	static FMonoVector3D FVector3D_Normalize(FMonoVector3D Self, float Tolerance);
	static FMonoVector3D FVector3D_BoundToCube(FMonoVector3D Self, float Radius);
	static FMonoVector3D FVector3D_GridSnap(FMonoVector3D Self, float GridSize);
	static FMonoVector3D FVector3D_RadiansToDegrees(FMonoVector3D Radians);
	static FMonoVector3D FVector3D_DegreesToRadians(FMonoVector3D Degrees);
	static FMonoVector3D FVector3D_GetUnsafeNormal(FMonoVector3D Self);
	static FMonoVector3D FVector3D_GetSignVector(FMonoVector3D Self);
	static FMonoVector3D FVector3D_Projection(FMonoVector3D Self);
	static FMonoVector3D FVector3D_Reciprocal(FMonoVector3D Self);
	static FMonoVector3D FVector3D_GetAbs(FMonoVector3D Self);
	//
	static FMonoRotator FVector3D_Rotation(FMonoVector3D Self);
	//
	static float FVector3D_PointPlaneDist(FMonoVector3D Self, FMonoVector3D PlaneBase, FMonoVector3D PlaneNormal);
	static float FVector3D_Triple(FMonoVector3D X, FMonoVector3D Y, FMonoVector3D Z);
	static float FVector3D_CosineAngle2D(FMonoVector3D Self, FMonoVector3D Other);
	static float FVector3D_BoxPushOut(FMonoVector3D Normal, FMonoVector3D Size);
	static float FVector3D_DistSquared(FMonoVector3D Self, FMonoVector3D Other);
	static float FVector3D_DotProduct(FMonoVector3D Self, FMonoVector3D Other);
	static float FVector3D_Distance(FMonoVector3D Self, FMonoVector3D Other);
	static float FVector3D_GetAbsMax(FMonoVector3D Self);
	static float FVector3D_GetAbsMin(FMonoVector3D Self);
	static float FVector3D_GetMax(FMonoVector3D Self);
	static float FVector3D_GetMin(FMonoVector3D Self);
	static float FVector3D_HeadingAngle(FMonoVector3D Self);
	//
	static bool FVector3D_Coplanar(FMonoVector3D Base1, FMonoVector3D Normal1, FMonoVector3D Base2, FMonoVector3D Normal2, float ParallelCosineThreshold);
	static bool FVector3D_Orthogonal(FMonoVector3D Normal1, FMonoVector3D Normal2, float OrthogonalCosineThreshold);
	static bool FVector3D_Coincident(FMonoVector3D Normal1, FMonoVector3D Normal2, float ParallelCosineThreshold);
	static bool FVector3D_Parallel(FMonoVector3D Normal1, FMonoVector3D Normal2, float ParallelCosineThreshold);
	static bool FVector3D_PointsAreNear(FMonoVector3D Point1, FMonoVector3D Point2, float Dist);
	static bool FVector3D_IsUnit(FMonoVector3D Self, float LengthSquaredTolerance);
	static bool FVector3D_PointsAreSame(FMonoVector3D Self, FMonoVector3D Other);
	static bool FVector3D_Equals(FMonoVector3D Self, FMonoVector3D Other);
	static bool FVector3D_IsUniform(FMonoVector3D Self, float Tolerance);
	static bool FVector3D_IsNearlyZero(FMonoVector3D Self);
	static bool FVector3D_IsNormalized(FMonoVector3D Self);
	static bool FVector3D_ContainsNaN(FMonoVector3D Self);
	static bool FVector3D_IsZero(FMonoVector3D Self);
public:
	/// :: FRotator.cs ::
	//
	static FMonoRotator FRotator_Add(FMonoRotator Self, float DeltaPitch, float DeltaYaw, float DeltaRoll);
	static FMonoRotator FRotator_GridSnap(FMonoRotator Self, FMonoRotator Snap);
	static FMonoRotator FRotator_GetEquivalentRotator(FMonoRotator Self);
	static FMonoRotator FRotator_GetDenormalized(FMonoRotator Self);
	static FMonoRotator FRotator_MakeFromEuler(FMonoVector3D Euler);
	static FMonoRotator FRotator_GetNormalized(FMonoRotator Self);
	static FMonoRotator FRotator_GetInverse(FMonoRotator Self);
	static FMonoRotator FRotator_Normalize(FMonoRotator Self);
	static FMonoRotator FRotator_Clamp(FMonoRotator Self);
	//
	static FMonoVector3D FRotator_Euler(FMonoRotator Self);
	static FMonoVector3D FRotator_RotateVector(FMonoRotator Self, FMonoVector3D Vector);
	static FMonoVector3D FRotator_UnrotateVector(FMonoRotator Self, FMonoVector3D Vector);
	//
	static float FRotator_GetManhattanDistance(FMonoRotator Self, FMonoRotator Rotator);
	static float FRotator_ClampAxis(FMonoRotator Self, float Angle);
	//
	static bool FRotator_IsZero(FMonoRotator Self);
	static bool FRotator_ContainsNaN(FMonoRotator Self);
	static bool FRotator_IsNearlyZero(FMonoRotator Self, float Tolerance);
	static bool FRotator_Equals(FMonoRotator Self, FMonoRotator Other, float Tolerance);
public:
	/// :: FTransform.cs ::
	//
	static FMonoTransform FTransform_Multiply(FMonoTransform A, FMonoTransform B);
	//
	static FMonoTransform FTransform_Blend(FMonoTransform Self, FMonoTransform Atom1, FMonoTransform Atom2, float Alpha);
	static FMonoTransform FTransform_GetRelativeTransformReverse(FMonoTransform Self, FMonoTransform Other);
	static FMonoTransform FTransform_BlendWith(FMonoTransform Self, FMonoTransform OtherAtom, float Alpha);
	static FMonoTransform FTransform_CopyTranslationAndScale3D(FMonoTransform Self, FMonoTransform Other);
	static FMonoTransform FTransform_GetRelativeTransform(FMonoTransform Self, FMonoTransform Other);
	static FMonoTransform FTransform_CopyTranslation(FMonoTransform Self, FMonoTransform Other);
	static FMonoTransform FTransform_CopyRotation(FMonoTransform Self, FMonoTransform Other);
	static FMonoTransform FTransform_CopyScale3D(FMonoTransform Self, FMonoTransform Other);
	static FMonoTransform FTransform_GetScaled(FMonoTransform Self, FMonoVector3D Scale);
	static FMonoTransform FTransform_NormalizeRotation(FMonoTransform Self);
	static FMonoTransform FTransform_Inverse(FMonoTransform Self);
	//
	static FMonoVector3D FTransform_AddTranslations(FMonoTransform A, FMonoTransform B);
	static FMonoVector3D FTransform_GetTranslation(FMonoTransform Self);
	static FMonoVector3D FTransform_GetRotation(FMonoTransform Self);
	static FMonoVector3D FTransform_GetScale3D(FMonoTransform Self);
	//
	static float FTransform_GetDeterminant(FMonoTransform Self);
	static float FTransform_GetMaximumAxisScale(FMonoTransform Self);
	static float FTransform_GetMinimumAxisScale(FMonoTransform Self);
	//
	static bool FTransform_AnyHasNegativeScale(FMonoVector3D InScale3D, FMonoVector3D InOtherScale3D);
	static bool FTransform_AreTranslationsEqual(FMonoTransform A, FMonoTransform B, float Tolerance);
	static bool FTransform_AreRotationsEqual(FMonoTransform A, FMonoTransform B, float Tolerance);
	static bool FTransform_AreScale3DsEqual(FMonoTransform A, FMonoTransform B, float Tolerance);
	//
	static bool FTransform_TranslationEquals(FMonoTransform Self, FMonoTransform Other, float Tolerance);
	static bool FTransform_RotationEquals(FMonoTransform Self, FMonoTransform Other, float Tolerance);
	static bool FTransform_EqualsNoScale(FMonoTransform Self, FMonoTransform Other, float Tolerance);
	static bool FTransform_Scale3DEquals(FMonoTransform Self, FMonoTransform Other, float Tolerance);
	static bool FTransform_Equals(FMonoTransform Self, FMonoTransform Other, float Tolerance);
	static bool FTransform_IsRotationNormalized(FMonoTransform Self);
	static bool FTransform_ContainsNaN(FMonoTransform Self);
	static bool FTransform_IsValid(FMonoTransform Self);
public:
	/// :: UClass.cs ::
	//
	static bool FClassPtr_IsStale(FMonoClass Self);
	static bool FClassPtr_IsValid(FMonoClass Self);
	static bool FClassPtr_IsChildOf(FMonoClass Self, FMonoClass Class);
	//
	static FMonoString FClassPtr_GetName(FMonoClass Self);
	static FMonoString FClassPtr_GetFullPath(FMonoClass Self);
public:
	/// :: UObject.cs ::
	//
	static bool FObjectPtr_IsStale(FMonoObject Self);
	static bool FObjectPtr_IsValid(FMonoObject Self);
	static bool FObjectPtr_IsA(FMonoObject Self, FMonoClass Class);
	//
	static FMonoClass FObjectPtr_GetClass(FMonoObject Self);
	static FMonoString FObjectPtr_GetName(FMonoObject Self);
	static FMonoString FObjectPtr_GetFullPath(FMonoObject Self);
	//
	static void FObjectPtr_InvokeFunction(FMonoObject Self, MonoString* MethodName, MonoString* Params);
public:
	/// :: AActor.cs ::
	//
	static bool FActorPtr_IsStale(FMonoActor Self);
	static bool FActorPtr_IsValid(FMonoActor Self);
	static bool FActorPtr_IsA(FMonoActor Self, FMonoClass Class);
	//
	static FMonoClass FActorPtr_GetClass(FMonoActor Self);
	static FMonoString FActorPtr_GetName(FMonoActor Self);
	static FMonoString FActorPtr_GetFullPath(FMonoActor Self);
	//
	static void FActorPtr_InvokeFunction(FMonoActor Self, MonoString* MethodName, MonoString* Params);
public:
	/// :: UComponent.cs ::
	//
	static bool FComponentPtr_IsStale(FMonoComponent Self);
	static bool FComponentPtr_IsValid(FMonoComponent Self);
	static bool FComponentPtr_IsA(FMonoComponent Self, FMonoClass Class);
	//
	static FMonoClass FComponentPtr_GetClass(FMonoComponent Self);
	static FMonoString FComponentPtr_GetName(FMonoComponent Self);
	static FMonoString FComponentPtr_GetFullPath(FMonoComponent Self);
	//
	static void FComponentPtr_InvokeFunction(FMonoComponent Self, MonoString* MethodName, MonoString* Params);
public:
	/// :: TArray.cs ::
	//
	static void TArray_Append(void* PropertyPtr, void* ArrayAddr, FMonoArray SourceArray);
	static void TArray_Swap(void* PropertyPtr, void* ArrayAddr, int32 A, int32 B);
	static void TArray_RemoveAt(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static void TArray_Reverse(void* PropertyPtr, void* ArrayAddr);
	static void TArray_Shuffle(void* PropertyPtr, void* ArrayAddr);
	static void TArray_Clear(void* PropertyPtr, void* ArrayAddr);
	//
	static int32 TArray_Length(void* PropertyPtr, void* ArrayAddr);
	static int32 TArray_LastIndex(void* PropertyPtr, void* ArrayAddr);
	static bool TArray_IsValidIndex(void* PropertyPtr, void* ArrayAddr, int32 Index);
	//
	static bool TArray_Get_Bool(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static uint8 TArray_Get_Byte(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static int32 TArray_Get_Int(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static int64 TArray_Get_Int64(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static float TArray_Get_Float(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoName TArray_Get_Name(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoText TArray_Get_Text(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoColor TArray_Get_Color(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoClass TArray_Get_Class(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoActor TArray_Get_Actor(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoObject TArray_Get_Object(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoString TArray_Get_String(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoRotator TArray_Get_Rotator(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoVector2D TArray_Get_Vector2D(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoVector3D TArray_Get_Vector3D(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoTransform TArray_Get_Transform(void* PropertyPtr, void* ArrayAddr, int32 Index);
	static FMonoComponent TArray_Get_Component(void* PropertyPtr, void* ArrayAddr, int32 Index);
	//
	static void TArray_Set_Bool(void* PropertyPtr, void* ArrayAddr, int32 Index, bool NewValue);
	static void TArray_Set_Byte(void* PropertyPtr, void* ArrayAddr, int32 Index, uint8 NewValue);
	static void TArray_Set_Int(void* PropertyPtr, void* ArrayAddr, int32 Index, int32 NewValue);
	static void TArray_Set_Int64(void* PropertyPtr, void* ArrayAddr, int32 Index, int64 NewValue);
	static void TArray_Set_Float(void* PropertyPtr, void* ArrayAddr, int32 Index, float NewValue);
	static void TArray_Set_Name(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoName NewValue);
	static void TArray_Set_Text(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoText NewValue);
	static void TArray_Set_Color(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoColor NewValue);
	static void TArray_Set_Class(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoClass NewValue);
	static void TArray_Set_Actor(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoActor NewValue);
	static void TArray_Set_Object(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoObject NewValue);
	static void TArray_Set_String(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoString NewValue);
	static void TArray_Set_Rotator(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoRotator NewValue);
	static void TArray_Set_Vector2D(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoVector2D NewValue);
	static void TArray_Set_Vector3D(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoVector3D NewValue);
	static void TArray_Set_Transform(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoTransform NewValue);
	static void TArray_Set_Component(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoComponent NewValue);
	//
	static void TArray_Add_Bool(void* PropertyPtr, void* ArrayAddr, bool NewValue);
	static void TArray_Add_Byte(void* PropertyPtr, void* ArrayAddr, uint8 NewValue);
	static void TArray_Add_Int(void* PropertyPtr, void* ArrayAddr, int32 NewValue);
	static void TArray_Add_Int64(void* PropertyPtr, void* ArrayAddr, int64 NewValue);
	static void TArray_Add_Float(void* PropertyPtr, void* ArrayAddr, float NewValue);
	static void TArray_Add_Name(void* PropertyPtr, void* ArrayAddr, FMonoName NewValue);
	static void TArray_Add_Text(void* PropertyPtr, void* ArrayAddr, FMonoText NewValue);
	static void TArray_Add_Color(void* PropertyPtr, void* ArrayAddr, FMonoColor NewValue);
	static void TArray_Add_Class(void* PropertyPtr, void* ArrayAddr, FMonoClass NewValue);
	static void TArray_Add_Actor(void* PropertyPtr, void* ArrayAddr, FMonoActor NewValue);
	static void TArray_Add_Object(void* PropertyPtr, void* ArrayAddr, FMonoObject NewValue);
	static void TArray_Add_String(void* PropertyPtr, void* ArrayAddr, FMonoString NewValue);
	static void TArray_Add_Rotator(void* PropertyPtr, void* ArrayAddr, FMonoRotator NewValue);
	static void TArray_Add_Vector2D(void* PropertyPtr, void* ArrayAddr, FMonoVector2D NewValue);
	static void TArray_Add_Vector3D(void* PropertyPtr, void* ArrayAddr, FMonoVector3D NewValue);
	static void TArray_Add_Transform(void* PropertyPtr, void* ArrayAddr, FMonoTransform NewValue);
	static void TArray_Add_Component(void* PropertyPtr, void* ArrayAddr, FMonoComponent NewValue);
	//
	static bool TArray_Contains_Bool(void* PropertyPtr, void* ArrayAddr, bool TypedValue);
	static bool TArray_Contains_Byte(void* PropertyPtr, void* ArrayAddr, uint8 TypedValue);
	static bool TArray_Contains_Int(void* PropertyPtr, void* ArrayAddr, int32 TypedValue);
	static bool TArray_Contains_Int64(void* PropertyPtr, void* ArrayAddr, int64 TypedValue);
	static bool TArray_Contains_Float(void* PropertyPtr, void* ArrayAddr, float TypedValue);
	static bool TArray_Contains_Name(void* PropertyPtr, void* ArrayAddr, FMonoName TypedValue);
	static bool TArray_Contains_Text(void* PropertyPtr, void* ArrayAddr, FMonoText TypedValue);
	static bool TArray_Contains_Color(void* PropertyPtr, void* ArrayAddr, FMonoColor TypedValue);
	static bool TArray_Contains_Class(void* PropertyPtr, void* ArrayAddr, FMonoClass TypedValue);
	static bool TArray_Contains_Actor(void* PropertyPtr, void* ArrayAddr, FMonoActor TypedValue);
	static bool TArray_Contains_Object(void* PropertyPtr, void* ArrayAddr, FMonoObject TypedValue);
	static bool TArray_Contains_String(void* PropertyPtr, void* ArrayAddr, FMonoString TypedValue);
	static bool TArray_Contains_Rotator(void* PropertyPtr, void* ArrayAddr, FMonoRotator TypedValue);
	static bool TArray_Contains_Vector2D(void* PropertyPtr, void* ArrayAddr, FMonoVector2D TypedValue);
	static bool TArray_Contains_Vector3D(void* PropertyPtr, void* ArrayAddr, FMonoVector3D TypedValue);
	static bool TArray_Contains_Transform(void* PropertyPtr, void* ArrayAddr, FMonoTransform TypedValue);
	static bool TArray_Contains_Component(void* PropertyPtr, void* ArrayAddr, FMonoComponent TypedValue);
	//
	static int32 TArray_FindItem_Bool(void* PropertyPtr, void* ArrayAddr, bool TypedValue);
	static int32 TArray_FindItem_Byte(void* PropertyPtr, void* ArrayAddr, uint8 TypedValue);
	static int32 TArray_FindItem_Int(void* PropertyPtr, void* ArrayAddr, int32 TypedValue);
	static int32 TArray_FindItem_Int64(void* PropertyPtr, void* ArrayAddr, int64 TypedValue);
	static int32 TArray_FindItem_Float(void* PropertyPtr, void* ArrayAddr, float TypedValue);
	static int32 TArray_FindItem_Name(void* PropertyPtr, void* ArrayAddr, FMonoName TypedValue);
	static int32 TArray_FindItem_Text(void* PropertyPtr, void* ArrayAddr, FMonoText TypedValue);
	static int32 TArray_FindItem_Color(void* PropertyPtr, void* ArrayAddr, FMonoColor TypedValue);
	static int32 TArray_FindItem_Class(void* PropertyPtr, void* ArrayAddr, FMonoClass TypedValue);
	static int32 TArray_FindItem_Actor(void* PropertyPtr, void* ArrayAddr, FMonoActor TypedValue);
	static int32 TArray_FindItem_Object(void* PropertyPtr, void* ArrayAddr, FMonoObject TypedValue);
	static int32 TArray_FindItem_String(void* PropertyPtr, void* ArrayAddr, FMonoString TypedValue);
	static int32 TArray_FindItem_Rotator(void* PropertyPtr, void* ArrayAddr, FMonoRotator TypedValue);
	static int32 TArray_FindItem_Vector2D(void* PropertyPtr, void* ArrayAddr, FMonoVector2D TypedValue);
	static int32 TArray_FindItem_Vector3D(void* PropertyPtr, void* ArrayAddr, FMonoVector3D TypedValue);
	static int32 TArray_FindItem_Transform(void* PropertyPtr, void* ArrayAddr, FMonoTransform TypedValue);
	static int32 TArray_FindItem_Component(void* PropertyPtr, void* ArrayAddr, FMonoComponent TypedValue);
	//
	static bool TArray_RemoveItem_Bool(void* PropertyPtr, void* ArrayAddr, bool TypedValue);
	static bool TArray_RemoveItem_Byte(void* PropertyPtr, void* ArrayAddr, uint8 TypedValue);
	static bool TArray_RemoveItem_Int(void* PropertyPtr, void* ArrayAddr, int32 TypedValue);
	static bool TArray_RemoveItem_Int64(void* PropertyPtr, void* ArrayAddr, int64 TypedValue);
	static bool TArray_RemoveItem_Float(void* PropertyPtr, void* ArrayAddr, float TypedValue);
	static bool TArray_RemoveItem_Name(void* PropertyPtr, void* ArrayAddr, FMonoName TypedValue);
	static bool TArray_RemoveItem_Text(void* PropertyPtr, void* ArrayAddr, FMonoText TypedValue);
	static bool TArray_RemoveItem_Color(void* PropertyPtr, void* ArrayAddr, FMonoColor TypedValue);
	static bool TArray_RemoveItem_Class(void* PropertyPtr, void* ArrayAddr, FMonoClass TypedValue);
	static bool TArray_RemoveItem_Actor(void* PropertyPtr, void* ArrayAddr, FMonoActor TypedValue);
	static bool TArray_RemoveItem_Object(void* PropertyPtr, void* ArrayAddr, FMonoObject TypedValue);
	static bool TArray_RemoveItem_String(void* PropertyPtr, void* ArrayAddr, FMonoString TypedValue);
	static bool TArray_RemoveItem_Rotator(void* PropertyPtr, void* ArrayAddr, FMonoRotator TypedValue);
	static bool TArray_RemoveItem_Vector2D(void* PropertyPtr, void* ArrayAddr, FMonoVector2D TypedValue);
	static bool TArray_RemoveItem_Vector3D(void* PropertyPtr, void* ArrayAddr, FMonoVector3D TypedValue);
	static bool TArray_RemoveItem_Transform(void* PropertyPtr, void* ArrayAddr, FMonoTransform TypedValue);
	static bool TArray_RemoveItem_Component(void* PropertyPtr, void* ArrayAddr, FMonoComponent TypedValue);
public:
	/// :: TSet.cs ::
	//
	static void TSet_Append(void* PropertyPtr, void* SetAddr, FMonoArray SourceArray);
	static void TSet_Clear(void* PropertyPtr, void* SetAddr);
	//
	static int32 TSet_Length(void* PropertyPtr, void* SetAddr);
	static FMonoArray TSet_ToArray(void* PropertyPtr, void* SetAddr, FMonoArray TargetArray);
	//
	static void TSet_Add_Byte(void* PropertyPtr, void* SetAddr, uint8 NewElement);
	static void TSet_Add_Int(void* PropertyPtr, void* SetAddr, int32 NewElement);
	static void TSet_Add_Int64(void* PropertyPtr, void* SetAddr, int64 NewElement);
	static void TSet_Add_Float(void* PropertyPtr, void* SetAddr, float NewElement);
	static void TSet_Add_Name(void* PropertyPtr, void* SetAddr, FMonoName NewElement);
	static void TSet_Add_Color(void* PropertyPtr, void* SetAddr, FMonoColor NewElement);
	static void TSet_Add_Class(void* PropertyPtr, void* SetAddr, FMonoClass NewElement);
	static void TSet_Add_Actor(void* PropertyPtr, void* SetAddr, FMonoActor NewElement);
	static void TSet_Add_Object(void* PropertyPtr, void* SetAddr, FMonoObject NewElement);
	static void TSet_Add_String(void* PropertyPtr, void* SetAddr, FMonoString NewElement);
	static void TSet_Add_Vector2D(void* PropertyPtr, void* SetAddr, FMonoVector2D NewElement);
	static void TSet_Add_Vector3D(void* PropertyPtr, void* SetAddr, FMonoVector3D NewElement);
	static void TSet_Add_Component(void* PropertyPtr, void* SetAddr, FMonoComponent NewElement);
	//
	static bool TSet_Contains_Byte(void* PropertyPtr, void* SetAddr, uint8 TypedElement);
	static bool TSet_Contains_Int(void* PropertyPtr, void* SetAddr, int32 TypedElement);
	static bool TSet_Contains_Int64(void* PropertyPtr, void* SetAddr, int64 TypedElement);
	static bool TSet_Contains_Float(void* PropertyPtr, void* SetAddr, float TypedElement);
	static bool TSet_Contains_Name(void* PropertyPtr, void* SetAddr, FMonoName TypedElement);
	static bool TSet_Contains_Color(void* PropertyPtr, void* SetAddr, FMonoColor TypedElement);
	static bool TSet_Contains_Class(void* PropertyPtr, void* SetAddr, FMonoClass TypedElement);
	static bool TSet_Contains_Actor(void* PropertyPtr, void* SetAddr, FMonoActor TypedElement);
	static bool TSet_Contains_Object(void* PropertyPtr, void* SetAddr, FMonoObject TypedElement);
	static bool TSet_Contains_String(void* PropertyPtr, void* SetAddr, FMonoString TypedElement);
	static bool TSet_Contains_Vector2D(void* PropertyPtr, void* SetAddr, FMonoVector2D TypedElement);
	static bool TSet_Contains_Vector3D(void* PropertyPtr, void* SetAddr, FMonoVector3D TypedElement);
	static bool TSet_Contains_Component(void* PropertyPtr, void* SetAddr, FMonoComponent TypedElement);
	//
	static bool TSet_RemoveItem_Byte(void* PropertyPtr, void* SetAddr, uint8 TypedElement);
	static bool TSet_RemoveItem_Int(void* PropertyPtr, void* SetAddr, int32 TypedElement);
	static bool TSet_RemoveItem_Int64(void* PropertyPtr, void* SetAddr, int64 TypedElement);
	static bool TSet_RemoveItem_Float(void* PropertyPtr, void* SetAddr, float TypedElement);
	static bool TSet_RemoveItem_Name(void* PropertyPtr, void* SetAddr, FMonoName TypedElement);
	static bool TSet_RemoveItem_Color(void* PropertyPtr, void* SetAddr, FMonoColor TypedElement);
	static bool TSet_RemoveItem_Class(void* PropertyPtr, void* SetAddr, FMonoClass TypedElement);
	static bool TSet_RemoveItem_Actor(void* PropertyPtr, void* SetAddr, FMonoActor TypedElement);
	static bool TSet_RemoveItem_Object(void* PropertyPtr, void* SetAddr, FMonoObject TypedElement);
	static bool TSet_RemoveItem_String(void* PropertyPtr, void* SetAddr, FMonoString TypedElement);
	static bool TSet_RemoveItem_Vector2D(void* PropertyPtr, void* SetAddr, FMonoVector2D TypedElement);
	static bool TSet_RemoveItem_Vector3D(void* PropertyPtr, void* SetAddr, FMonoVector3D TypedElement);
	static bool TSet_RemoveItem_Component(void* PropertyPtr, void* SetAddr, FMonoComponent TypedElement);
public:
	/// :: TMap.cs ::
	//
	static void TMap_Clear(void* PropertyPtr, void* MapAddr);
	//
	static int32 TMap_Length(void* PropertyPtr, void* MapAddr);
	static FMonoArray TMap_Keys(void* PropertyPtr, void* MapAddr, FMonoArray TargetArray);
	static FMonoArray TMap_Values(void* PropertyPtr, void* MapAddr, FMonoArray TargetArray);
	//
	static bool TMap_FindItem_Byte(void* PropertyPtr, void* MapAddr, uint8 TypedKey);
	static bool TMap_FindItem_Int(void* PropertyPtr, void* MapAddr, int32 TypedKey);
	static bool TMap_FindItem_Int64(void* PropertyPtr, void* MapAddr, int64 TypedKey);
	static bool TMap_FindItem_Float(void* PropertyPtr, void* MapAddr, float TypedKey);
	static bool TMap_FindItem_Name(void* PropertyPtr, void* MapAddr, FMonoName TypedKey);
	static bool TMap_FindItem_Color(void* PropertyPtr, void* MapAddr, FMonoColor TypedKey);
	static bool TMap_FindItem_Class(void* PropertyPtr, void* MapAddr, FMonoClass TypedKey);
	static bool TMap_FindItem_Actor(void* PropertyPtr, void* MapAddr, FMonoActor TypedKey);
	static bool TMap_FindItem_Object(void* PropertyPtr, void* MapAddr, FMonoObject TypedKey);
	static bool TMap_FindItem_String(void* PropertyPtr, void* MapAddr, FMonoString TypedKey);
	static bool TMap_FindItem_Vector2D(void* PropertyPtr, void* MapAddr, FMonoVector2D TypedKey);
	static bool TMap_FindItem_Vector3D(void* PropertyPtr, void* MapAddr, FMonoVector3D TypedKey);
	static bool TMap_FindItem_Component(void* PropertyPtr, void* MapAddr, FMonoComponent TypedKey);
	//
	static bool TMap_RemoveItem_Byte(void* PropertyPtr, void* MapAddr, uint8 TypedKey);
	static bool TMap_RemoveItem_Int(void* PropertyPtr, void* MapAddr, int32 TypedKey);
	static bool TMap_RemoveItem_Int64(void* PropertyPtr, void* MapAddr, int64 TypedKey);
	static bool TMap_RemoveItem_Float(void* PropertyPtr, void* MapAddr, float TypedKey);
	static bool TMap_RemoveItem_Name(void* PropertyPtr, void* MapAddr, FMonoName TypedKey);
	static bool TMap_RemoveItem_Color(void* PropertyPtr, void* MapAddr, FMonoColor TypedKey);
	static bool TMap_RemoveItem_Class(void* PropertyPtr, void* MapAddr, FMonoClass TypedKey);
	static bool TMap_RemoveItem_Actor(void* PropertyPtr, void* MapAddr, FMonoActor TypedKey);
	static bool TMap_RemoveItem_Object(void* PropertyPtr, void* MapAddr, FMonoObject TypedKey);
	static bool TMap_RemoveItem_String(void* PropertyPtr, void* MapAddr, FMonoString TypedKey);
	static bool TMap_RemoveItem_Vector2D(void* PropertyPtr, void* MapAddr, FMonoVector2D TypedKey);
	static bool TMap_RemoveItem_Vector3D(void* PropertyPtr, void* MapAddr, FMonoVector3D TypedKey);
	static bool TMap_RemoveItem_Component(void* PropertyPtr, void* MapAddr, FMonoComponent TypedKey);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////