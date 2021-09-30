//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MCS_MonoCall.h"
#include "MCS_MonoUtility.h"

#include "MagicNodeSharp_Shared.h"

#include "Interfaces/IPluginManager.h"

#include "Runtime/Core/Public/Misc/App.h"
#include "Runtime/Core/Public/CoreGlobals.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/CoreUObject/Public/UObject/Stack.h"
#include "Runtime/Core/Public/Misc/OutputDeviceNull.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetArrayLibrary.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintSetLibrary.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintMapLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: DEBUG ::

void ICALL::API::LogMessage(MonoString* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	WIDECHAR* CH = mono_string_to_utf16(Message);
	FString Str = StringCast<TCHAR>(CH).Get();
	//
	LOG::CS_STR(ESeverity::Info,Str);
	//
	mono_free(CH);
}

void ICALL::API::LogWarning(MonoString* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	WIDECHAR* CH = mono_string_to_utf16(Message);
	FString Str = StringCast<TCHAR>(CH).Get();
	//
	LOG::CS_STR(ESeverity::Warning,Str);
	//
	mono_free(CH);
}

void ICALL::API::LogError(MonoString* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	WIDECHAR* CH = mono_string_to_utf16(Message);
	FString Str = StringCast<TCHAR>(CH).Get();
	//
	LOG::CS_STR(ESeverity::Error,Str);
	//
	mono_free(CH);
}

void ICALL::API::PrintMessage(MonoString* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
#if WITH_EDITOR
	WIDECHAR* CH = mono_string_to_utf16(Message);
	FString Str = StringCast<TCHAR>(CH).Get();
	//
	LOG::CSX_STR(ESeverity::Info,Str);
	//
	mono_free(CH);
#else
	ICALL::API::LogMessage(Message);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: INTEROP ::

void ICALL::API::ConsoleCommand(UMagicNodeSharp* Node, MonoString* Command) {
	if (Node==nullptr) {return;}
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<UMagicNodeSharp*,MonoString*>(&ICALL::API::ConsoleCommand,Node,Command),
			GET_STATID(STAT_FMonoMethod_SafeCall), nullptr, ENamedThreads::GameThread
		);//
	return;}
	//
	UObject* const &Context = Node->GetOuter();
	if (Context==nullptr) {return;}
	//
	UWorld* const &World = Context->GetWorld();
	if (World==nullptr) {return;}
	//
	WIDECHAR* _Command = mono_string_to_utf16(Command);
	FString CCmd = StringCast<TCHAR>(_Command).Get();
	CCmd.TrimStartAndEndInline();
	//
	APlayerController* PC = Node->GetTypedOuter<APlayerController>();
	if (PC==nullptr) {PC=World->GetFirstPlayerController();}
	//
	if ((PC!=nullptr)&&(!CCmd.IsEmpty())) {
		PC->ConsoleCommand(CCmd);
	}///
	//
	mono_free(_Command);
}

void ICALL::API::InvokeFunction(UMagicNodeSharp* Node, MonoString* MethodName, MonoString* Params) {
	if (Node==nullptr) {return;}
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<UMagicNodeSharp*,MonoString*,MonoString*>(&ICALL::API::InvokeFunction,Node,MethodName,Params),
			GET_STATID(STAT_FMonoMethod_SafeCall), nullptr, ENamedThreads::GameThread
		);//
	return;}
	//
	UObject* const &Context = Node->GetOuter();
	if (Context==nullptr) {return;}
	//
	WIDECHAR* _Method = mono_string_to_utf16(MethodName);
	WIDECHAR* _Params = mono_string_to_utf16(Params);
	//
	FString Method = StringCast<TCHAR>(_Method).Get();
	FString Args = StringCast<TCHAR>(_Params).Get();
	//
	FString Call = (Method+TEXT(" ")+Args).TrimStartAndEnd();
	//
	FOutputDeviceNull AR;
	Context->CallFunctionByNameWithArguments(*Call,AR,NULL,true);
	//
	mono_free(_Method);
	mono_free(_Params);
}

void ICALL::API::FObjectPtr_InvokeFunction(FMonoObject Self, MonoString* MethodName, MonoString* Params) {
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<FMonoObject,MonoString*,MonoString*>(&ICALL::API::FObjectPtr_InvokeFunction,Self,MethodName,Params),
			GET_STATID(STAT_FMonoMethod_SafeCall), nullptr, ENamedThreads::GameThread
		);//
	return;}
	//
	UObject* Target = Self.ToObject();
	if (Target==nullptr||Target->IsPendingKill()) {return;}
	//
	WIDECHAR* _Method = mono_string_to_utf16(MethodName);
	WIDECHAR* _Params = mono_string_to_utf16(Params);
	//
	FString Method = StringCast<TCHAR>(_Method).Get();
	FString Args = StringCast<TCHAR>(_Params).Get();
	//
	FString Call = (Method+TEXT(" ")+Args).TrimStartAndEnd();
	//
	FOutputDeviceNull AR;
	Target->CallFunctionByNameWithArguments(*Call,AR,NULL,true);
	//
	mono_free(_Method);
	mono_free(_Params);
}

void ICALL::API::FActorPtr_InvokeFunction(FMonoActor Self, MonoString* MethodName, MonoString* Params) {
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<FMonoActor,MonoString*,MonoString*>(&ICALL::API::FActorPtr_InvokeFunction,Self,MethodName,Params),
			GET_STATID(STAT_FMonoMethod_SafeCall), nullptr, ENamedThreads::GameThread
		);//
	return;}
	//
	AActor* Target = Self.ToActor();
	if (Target==nullptr||Target->IsPendingKill()) {return;}
	//
	WIDECHAR* _Method = mono_string_to_utf16(MethodName);
	WIDECHAR* _Params = mono_string_to_utf16(Params);
	//
	FString Method = StringCast<TCHAR>(_Method).Get();
	FString Args = StringCast<TCHAR>(_Params).Get();
	//
	FString Call = (Method+TEXT(" ")+Args).TrimStartAndEnd();
	//
	FOutputDeviceNull AR;
	Target->CallFunctionByNameWithArguments(*Call,AR,NULL,true);
	//
	mono_free(_Method);
	mono_free(_Params);
}

void ICALL::API::FComponentPtr_InvokeFunction(FMonoComponent Self, MonoString* MethodName, MonoString* Params) {
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<FMonoComponent,MonoString*,MonoString*>(&ICALL::API::FComponentPtr_InvokeFunction,Self,MethodName,Params),
			GET_STATID(STAT_FMonoMethod_SafeCall), nullptr, ENamedThreads::GameThread
		);//
	return;}
	//
	UActorComponent* Target = Self.ToComponent();
	if (Target==nullptr||Target->IsPendingKill()) {return;}
	//
	WIDECHAR* _Method = mono_string_to_utf16(MethodName);
	WIDECHAR* _Params = mono_string_to_utf16(Params);
	//
	FString Method = StringCast<TCHAR>(_Method).Get();
	FString Args = StringCast<TCHAR>(_Params).Get();
	//
	FString Call = (Method+TEXT(" ")+Args).TrimStartAndEnd();
	//
	FOutputDeviceNull AR;
	Target->CallFunctionByNameWithArguments(*Call,AR,NULL,true);
	//
	mono_free(_Method);
	mono_free(_Params);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: NODE API ::

void ICALL::API::QuitGame() {
	FGenericPlatformMisc::RequestExit(false);
}

bool ICALL::API::IsEditor() {
	if (GEngine) {
		return GEngine->IsEditor();
	}///
	//
	return false;
}

bool ICALL::API::IsShipping() {
#if UE_BUILD_SHIPPING
	return true;
#endif
	//
	return UKismetSystemLibrary::IsPackagedForDistribution();
}

bool ICALL::API::IsDevelopment() {
#if UE_BUILD_DEVELOPMENT
	return true;
#endif
	//
	return !UKismetSystemLibrary::IsPackagedForDistribution();
}

bool ICALL::API::IsStandalone(UMagicNodeSharp* Node) {
	if (Node==nullptr) {return false;}
	//
	return UKismetSystemLibrary::IsStandalone(Node);
}

bool ICALL::API::IsServer(UMagicNodeSharp* Node) {
	if (Node==nullptr) {return false;}
	//
#if UE_GAME || UE_SERVER
	return true;
#endif
	//
	return UKismetSystemLibrary::IsServer(Node);
}

bool ICALL::API::IsClient(UMagicNodeSharp* Node) {
	if (Node==nullptr) {return false;}
	//
#if UE_GAME || (UE_GAME && (!UE_SERVER))
	return true;
#endif
	//
	return !(UKismetSystemLibrary::IsServer(Node)||UKismetSystemLibrary::IsDedicatedServer(Node));
}

bool ICALL::API::IsDedicatedServer(UMagicNodeSharp* Node) {
	if (Node==nullptr) {return false;}
	//
	return UKismetSystemLibrary::IsDedicatedServer(Node);
}

bool ICALL::API::HasAuthority(UMagicNodeSharp* Node) {
	if (Node==nullptr) {return false;}
	//
	if (AActor*Outer=Node->GetTypedOuter<AActor>()) {
		return Outer->HasAuthority();
	}///
	//
	return false;
}

bool ICALL::API::IsGameThread() {
	return IsInGameThread();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ICALL::API::DestroyNativeNode(UMagicNodeSharp* Node) {
	if (Node && Node->IsAsyncTask) {
		Node->ExitMonoPlay();
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UObject* ICALL::API::GetNativeContext(UMagicNodeSharp* Node) {
	if (Node) {
		return Node->GetOuter();
	}///
	//
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ICALL::API::IsNativeObjectValid(UObject* OBJ) {
	if (OBJ==nullptr||OBJ->IsPendingKill()) {return false;}
	//
	return OBJ->IsValidLowLevel();
}

bool ICALL::API::IsNativeClassValid(UClass* Class) {
	if (Class==nullptr||Class->IsPendingKill()) {return false;}
	//
	return Class->IsValidLowLevel();
}

void ICALL::API::DestroyNativeObject(UObject* OBJ) {
	if (OBJ==nullptr||(!OBJ->IsValidLowLevel())) {return;}
	if (OBJ==nullptr||OBJ->IsPendingKill()) {return;}
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<UObject*>(&ICALL::API::DestroyNativeObject,OBJ),
			GET_STATID(STAT_FMonoMethod_SafeCall), nullptr, ENamedThreads::GameThread
		);//
	return;}
	//
	OBJ->ConditionalBeginDestroy();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UClass* ICALL::API::LoadObjectClass(MonoString* ClassPath) {
	WIDECHAR* CH = mono_string_to_utf16(ClassPath);
	FString Path = StringCast<TCHAR>(CH).Get();
	//
	if (Path.IsEmpty()) {
		LOG::CSX_CHAR(ESeverity::Error,TEXT(":: LoadObjectClass() :: Path is invalid.")); return nullptr;
	}///
	//
	ConstructorHelpers::StripObjectClass(Path,true);
	UClass* Class = ConstructorHelpersInternal::FindOrLoadClass(Path,UObject::StaticClass());
	//
	if (Class==nullptr) {
		LOG::CSX_CHAR(ESeverity::Error,TEXT(":: LoadObjectClass() :: Path is invalid.")); return nullptr;
	} else {LOG::CS_STR(ESeverity::Info,FString::Printf(TEXT(":: LoadObjectClass() :: class loaded successfully:  %s"),*Class->GetName()));}
	//
	mono_free(CH);
	return Class;
}

UObject* ICALL::API::CreateNewObject(FMonoObject Outer, FMonoClass Class) {
	if (Outer.ToObject()->IsValidLowLevelFast()) {
		const UClass* BaseClass = Class.ToClass();
		//
		if (!BaseClass->IsValidLowLevelFast()) {
			LOG::CSX_CHAR(ESeverity::Error,TEXT(":: CreateNewObject() :: Target Class is invalid.")); return nullptr;
		}///
		//
		const FName ID = MakeUniqueObjectName(Outer.ToObject(),BaseClass,BaseClass->GetFName());
		UObject* OBJ = NewObject<UMagicNodeSharpSource>(Outer.ToObject(),UMagicNodeSharpSource::StaticClass(),ID,RF_NoFlags);
		//
		if (OBJ==nullptr) {
			LOG::CS_STR(ESeverity::Error,FString::Printf(TEXT(":: CreateNewObject() :: failed to instantiate object of class:  %s"),*BaseClass->GetName()));
		} else {LOG::CS_STR(ESeverity::Info,FString::Printf(TEXT(":: CreateNewObject() :: object instantiated successfully:  %s"),*OBJ->GetName()));}
		//
		return OBJ;
	} else {
		LOG::CSX_CHAR(ESeverity::Error,TEXT(":: CreateNewObject() :: Outer context object is invalid.")); return nullptr;
	}///
	//
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: STRING API ::

FMonoString ICALL::API::FString_Append(FMonoString Self, FMonoString Other) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Other.Data);
	//
	FString SA = StringCast<TCHAR>(WA).Get();
	FString SB = StringCast<TCHAR>(WB).Get();
	//
	SA.Append(SB);
	mono_free(WA);
	mono_free(WB);
	//
	return FMonoString(SA,MonoCore.GetCoreDomain());
}

int32 ICALL::API::FString_Compare(FMonoString A, FMonoString B, EMonoSearchCase Case) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(A.Data);
	WIDECHAR* WB = mono_string_to_utf16(B.Data);
	//
	const FString SA = StringCast<TCHAR>(WA).Get();
	const FString SB = StringCast<TCHAR>(WB).Get();
	//
	mono_free(WA);
	mono_free(WB);
	//
	return SA.Compare(SB,(ESearchCase::Type)Case);
}

FMonoString ICALL::API::FString_Empty(FMonoString Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	return FMonoString(FString{},MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_ToLower(FMonoString Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoString(SA.ToLower(),MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_ToUpper(FMonoString Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoString(SA.ToUpper(),MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_TrimEnd(FMonoString Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoString(SA.TrimEnd(),MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_TrimStart(FMonoString Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoString(SA.TrimStart(),MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_TrimStartAndEnd(FMonoString Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoString(SA.TrimStartAndEnd(),MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_FromInt(FMonoString Self, int32 Number) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoString(SA.FromInt(Number),MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_Mid(FMonoString Self, int32 Start, int32 Count) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoString(SA.Mid(Start,Count),MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_RemoveAt(FMonoString Self, int32 Start, int32 Count) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	FString SA = StringCast<TCHAR>(WA).Get();
	SA.RemoveAt(Start,Count);
	//
	mono_free(WA);
	//
	return FMonoString(SA,MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_InsertAt(FMonoString Self, int32 Index, FMonoString Characters) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Characters.Data);
	//
	FString SA = StringCast<TCHAR>(WA).Get();
	FString SB = StringCast<TCHAR>(WB).Get();
	SA.InsertAt(Index,SB);
	//
	mono_free(WA);
	mono_free(WB);
	//
	return FMonoString(SA,MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_RemoveFromEnd(FMonoString Self, FMonoString Suffix, EMonoSearchCase Case) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Suffix.Data);
	//
	FString SA = StringCast<TCHAR>(WA).Get();
	FString SB = StringCast<TCHAR>(WB).Get();
	SA.RemoveFromEnd(SB,(ESearchCase::Type)Case);
	//
	mono_free(WA);
	mono_free(WB);
	//
	return FMonoString(SA,MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_RemoveFromStart(FMonoString Self, FMonoString Prefix, EMonoSearchCase Case) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Prefix.Data);
	//
	FString SA = StringCast<TCHAR>(WA).Get();
	FString SB = StringCast<TCHAR>(WB).Get();
	SA.RemoveFromStart(SB,(ESearchCase::Type)Case);
	//
	mono_free(WA);
	mono_free(WB);
	//
	return FMonoString(SA,MonoCore.GetCoreDomain());
}

FMonoString ICALL::API::FString_Replace(FMonoString Self, FMonoString From, FMonoString To, EMonoSearchCase Case) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(From.Data);
	WIDECHAR* WC = mono_string_to_utf16(To.Data);
	//
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	mono_free(WB);
	//
	return FMonoString(SA.Replace(WB,WC,(ESearchCase::Type)Case),MonoCore.GetCoreDomain());
}

int32 ICALL::API::FString_Find(FMonoString Self, int32 SearchFrom, FMonoString SubString, EMonoSearchCase Case, EMonoSearchDir Direction) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(SubString.Data);
	//
	const FString SA = StringCast<TCHAR>(WA).Get();
	const FString SB = StringCast<TCHAR>(WB).Get();
	//
	mono_free(WA);
	mono_free(WB);
	//
	return SA.Find(SB,(ESearchCase::Type)Case,(ESearchDir::Type)Direction);
}

bool ICALL::API::FString_IsEmpty(FMonoString Self) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return SA.IsEmpty();
}

bool ICALL::API::FString_IsNumeric(FMonoString Self) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return SA.IsNumeric();
}

bool ICALL::API::FString_IsValidIndex(FMonoString Self, int32 Index) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return SA.IsValidIndex(Index);
}

bool ICALL::API::FString_FindChar(FMonoString Self, int32 Search, int32 FromIndex) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return SA.FindChar(Search,FromIndex);
}

bool ICALL::API::FString_Equals(FMonoString Self, FMonoString Other, EMonoSearchCase Case) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Other.Data);
	//
	const FString SA = StringCast<TCHAR>(WA).Get();
	const FString SB = StringCast<TCHAR>(WB).Get();
	//
	mono_free(WA);
	mono_free(WB);
	//
	return SA.Equals(SB,(ESearchCase::Type)Case);
}

bool ICALL::API::FString_Contains(FMonoString Self, FMonoString Other, EMonoSearchCase Case) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Other.Data);
	//
	const FString SA = StringCast<TCHAR>(WA).Get();
	const FString SB = StringCast<TCHAR>(WB).Get();
	//
	mono_free(WA);
	mono_free(WB);
	//
	return SA.Contains(SB,(ESearchCase::Type)Case);
}

bool ICALL::API::FString_EndsWith(FMonoString Self, FMonoString Suffix, EMonoSearchCase Case) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Suffix.Data);
	//
	const FString SA = StringCast<TCHAR>(WA).Get();
	const FString SB = StringCast<TCHAR>(WB).Get();
	//
	mono_free(WA);
	mono_free(WB);
	//
	return SA.EndsWith(SB,(ESearchCase::Type)Case);
}

bool ICALL::API::FString_StartsWith(FMonoString Self, FMonoString Prefix, EMonoSearchCase Case) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Prefix.Data);
	//
	const FString SA = StringCast<TCHAR>(WA).Get();
	const FString SB = StringCast<TCHAR>(WB).Get();
	//
	mono_free(WA);
	mono_free(WB);
	//
	return SA.StartsWith(SB,(ESearchCase::Type)Case);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: NAME API ::

FMonoName ICALL::API::FName_AppendString(FMonoName Self, FMonoString Other) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Other.Data);
	//
	FString SA = StringCast<TCHAR>(WA).Get();
	FString SB = StringCast<TCHAR>(WB).Get();
	SA.Append(SB);
	//
	mono_free(WA);
	mono_free(WB);
	//
	return FMonoName(*SA,MonoCore.GetCoreDomain());
}

bool ICALL::API::FName_IsEqual(FMonoName Self, FMonoName Other, EMonoSearchCase Case) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Other.Data);
	//
	const FName NA = StringCast<TCHAR>(WA).Get();
	const FName NB = StringCast<TCHAR>(WB).Get();
	//
	mono_free(WA);
	mono_free(WB);
	//
	return NA.IsEqual(NB,(ENameCase)Case);
}

bool ICALL::API::FName_IsValid(FMonoName Self) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FName NA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return NA.IsValid();
}

bool ICALL::API::FName_IsNone(FMonoName Self) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FName NA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return NA.IsNone();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: TEXT API ::

FMonoText ICALL::API::FText_AsCultureInvariant(FMonoString Text) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Text.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoText(FText::AsCultureInvariant(SA),MonoCore.GetCoreDomain());
}

FMonoText ICALL::API::FText_AsCurrency(int32 Value, FMonoString CurrencyCode) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(CurrencyCode.Data);
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoText(FText::AsCurrency(Value,SA),MonoCore.GetCoreDomain());
}

FMonoText ICALL::API::FText_AsDate(int64 Ticks, EMonoDateStyle DateStyle, FMonoString TimeZone) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(TimeZone.Data);
	//
	const FString SA = StringCast<TCHAR>(WA).Get();
	const FDateTime DateTime = FDateTime(Ticks);
	//
	mono_free(WA);
	//
	return FMonoText(FText::AsDate(DateTime,(EDateTimeStyle::Type)DateStyle,SA),MonoCore.GetCoreDomain());
}

FMonoText ICALL::API::FText_AsNumber(int32 Value) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	return FMonoText(FText::AsNumber(Value),MonoCore.GetCoreDomain());
}

FMonoText ICALL::API::FText_AsPercent(float Value) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	return FMonoText(FText::AsPercent(Value),MonoCore.GetCoreDomain());
}

FMonoText ICALL::API::FText_ToLower(FMonoText Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FText TA = FText::FromString(StringCast<TCHAR>(WA).Get());
	//
	mono_free(WA);
	//
	return FMonoText(TA.ToLower(),MonoCore.GetCoreDomain());
}

FMonoText ICALL::API::FText_ToUpper(FMonoText Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	const FText TA = FText::FromString(StringCast<TCHAR>(WA).Get());
	//
	mono_free(WA);
	//
	return FMonoText(TA.ToUpper(),MonoCore.GetCoreDomain());
}

bool ICALL::API::FText_EqualTo(FMonoText Self, FMonoText Other) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	WIDECHAR* WB = mono_string_to_utf16(Other.Data);
	//
	const FText TA = FText::FromString(StringCast<TCHAR>(WA).Get());
	const FText TB = FText::FromString(StringCast<TCHAR>(WB).Get());
	//
	mono_free(WA);
	mono_free(WB);
	//
	return TA.EqualTo(TB);
}

bool ICALL::API::FText_IsEmpty(FMonoText Self) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	//
	const FText TA = FText::FromString(StringCast<TCHAR>(WA).Get());
	//
	mono_free(WA);
	//
	return TA.IsEmpty();
}

bool ICALL::API::FText_IsNumeric(FMonoText Self) {
	WIDECHAR* WA = mono_string_to_utf16(Self.Data);
	//
	const FText TA = FText::FromString(StringCast<TCHAR>(WA).Get());
	//
	mono_free(WA);
	//
	return TA.IsNumeric();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: COLOR API ::

FMonoColor ICALL::API::FColor_FromHex(FMonoColor Self, FMonoString HexString) {
	WIDECHAR* WA = mono_string_to_utf16(HexString.Data);
	//
	const FColor CA = Self.ToColor();
	const FString SA = StringCast<TCHAR>(WA).Get();
	//
	mono_free(WA);
	//
	return FMonoColor(CA.FromHex(SA));
}

FMonoString ICALL::API::FColor_ToHex(FMonoColor Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	const FColor CA = Self.ToColor();
	//
	return FMonoString(CA.ToHex(),MonoCore.GetCoreDomain());
}

bool ICALL::API::FColor_IsEqual(FMonoColor Self, FMonoColor Other) {
	const FColor CA = Self.ToColor();
	const FColor CB = Other.ToColor();
	//
	return FLinearColor(CA).Equals(FLinearColor(CB));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: VECTOR 2D API ::

FMonoVector2D ICALL::API::FVector2D_ClampAxes(FMonoVector2D Self, float MinAxisVal, float MaxAxisVal) {
	const FVector2D V2A = Self.ToVector2D();
	//
	return FMonoVector2D(V2A.ClampAxes(MinAxisVal,MaxAxisVal));
}

FMonoVector2D ICALL::API::FVector2D_GetAbs(FMonoVector2D Self) {
	const FVector2D V2A = Self.ToVector2D();
	//
	return FMonoVector2D(V2A.GetAbs());
}

FMonoVector2D ICALL::API::FVector2D_GetRotated(FMonoVector2D Self, float Angle) {
	const FVector2D V2A = Self.ToVector2D();
	//
	return FMonoVector2D(V2A.GetRotated(Angle));
}

FMonoVector2D ICALL::API::FVector2D_GetSafeNormal(FMonoVector2D Self, float Tolerance) {
	const FVector2D V2A = Self.ToVector2D();
	//
	return FMonoVector2D(V2A.GetSafeNormal(Tolerance));
}

FMonoVector2D ICALL::API::FVector2D_GetSignVector(FMonoVector2D Self) {
	const FVector2D V2A = Self.ToVector2D();
	//
	return FMonoVector2D(V2A.GetSignVector());
}

FMonoVector2D ICALL::API::FVector2D_Max(FMonoVector2D Self, FMonoVector2D Other) {
	const FVector2D V2A = Self.ToVector2D();
	const FVector2D V2B = Other.ToVector2D();
	//
	return FMonoVector2D(FVector2D::Max(V2A,V2B));
}

FMonoVector2D ICALL::API::FVector2D_Min(FMonoVector2D Self, FMonoVector2D Other) {
	const FVector2D V2A = Self.ToVector2D();
	const FVector2D V2B = Other.ToVector2D();
	//
	return FMonoVector2D(FVector2D::Min(V2A,V2B));
}

FMonoVector2D ICALL::API::FVector2D_RoundToVector(FMonoVector2D Self) {
	const FVector2D V2A = Self.ToVector2D();
	//
	return FMonoVector2D(V2A.RoundToVector());
}

FMonoVector2D ICALL::API::FVector2D_SphericalToUnitCartesian(FMonoVector2D Self) {
	const FVector2D V2A = Self.ToVector2D();
	//
	const FVector V3A = V2A.SphericalToUnitCartesian();
	//
	return FMonoVector2D(V3A.X,V3A.Y);
}

FMonoVector2D ICALL::API::FVector2D_ToDirection(FMonoVector2D Self) {
	const FVector2D V2A = Self.ToVector2D();
	//
	FVector2D V2{};
	float Len = 0.f;
	V2A.ToDirectionAndLength(V2,Len);
	//
	return FMonoVector2D(V2);
}

FMonoVector2D ICALL::API::FVector2D_Normalize(FMonoVector2D Self, float Tolerance) {
	FVector2D V2A = Self.ToVector2D();
	V2A.Normalize(Tolerance);
	//
	return V2A;
}

float ICALL::API::FVector2D_CrossProduct(FMonoVector2D Self, FMonoVector2D Other) {
	return FVector2D::CrossProduct(Self.ToVector2D(),Other.ToVector2D());
}

float ICALL::API::FVector2D_Distance(FMonoVector2D Self, FMonoVector2D Other) {
	return FVector2D::Distance(Self.ToVector2D(),Other.ToVector2D());
}

float ICALL::API::FVector2D_DistSquared(FMonoVector2D Self, FMonoVector2D Other) {
	return FVector2D::DistSquared(Self.ToVector2D(),Other.ToVector2D());
}

float ICALL::API::FVector2D_DotProduct(FMonoVector2D Self, FMonoVector2D Other) {
	return FVector2D::DotProduct(Self.ToVector2D(),Other.ToVector2D());
}

float ICALL::API::FVector2D_GetAbsMax(FMonoVector2D Self) {
	return Self.ToVector2D().GetAbsMax();
}

float ICALL::API::FVector2D_GetMax(FMonoVector2D Self) {
	return Self.ToVector2D().GetMax();
}

float ICALL::API::FVector2D_GetMin(FMonoVector2D Self) {
	return Self.ToVector2D().GetMin();
}

float ICALL::API::FVector2D_Size(FMonoVector2D Self) {
	return Self.ToVector2D().Size();
}

float ICALL::API::FVector2D_SizeSquared(FMonoVector2D Self) {
	return Self.ToVector2D().SizeSquared();
}

bool ICALL::API::FVector2D_ContainsNaN(FMonoVector2D Self) {
	return Self.ToVector2D().ContainsNaN();
}

bool ICALL::API::FVector2D_IsNearlyZero(FMonoVector2D Self) {
	return Self.ToVector2D().IsNearlyZero();
}

bool ICALL::API::FVector2D_IsZero(FMonoVector2D Self) {
	return Self.ToVector2D().IsZero();
}

bool ICALL::API::FVector2D_Equals(FMonoVector2D Self, FMonoVector2D Other) {
	return Self.ToVector2D().Equals(Other.ToVector2D());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: VECTOR 3D API ::

FMonoVector3D ICALL::API::FVector3D_AddBounded(FMonoVector3D Self, FMonoVector3D Other, float Radius) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Other.ToVector3D();
	//
	V3A.AddBounded(V3B,Radius);
	//
	return FMonoVector3D(V3A);
}

FMonoVector3D ICALL::API::FVector3D_BoundToBox(FMonoVector3D Self, FMonoVector3D Min, FMonoVector3D Max) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Min.ToVector3D();
	FVector V3C = Max.ToVector3D();
	//
	V3A.BoundToBox(V3B,V3C);
	//
	return FMonoVector3D(V3A);
}

FMonoVector3D ICALL::API::FVector3D_BoundToCube(FMonoVector3D Self, float Radius) {
	FVector V3A = Self.ToVector3D();
	//
	V3A.BoundToCube(Radius);
	//
	return FMonoVector3D(V3A);
}

FMonoVector3D ICALL::API::FVector3D_CreateOrthonormalBasis(FMonoVector3D XAxis, FMonoVector3D YAxis, FMonoVector3D ZAxis) {
	FVector V3A = XAxis.ToVector3D();
	FVector V3B = YAxis.ToVector3D();
	FVector V3C = ZAxis.ToVector3D();
	//
	FVector::CreateOrthonormalBasis(V3A,V3B,V3C);
	//
	return FMonoVector3D(V3A);
}

FMonoVector3D ICALL::API::FVector3D_CrossProduct(FMonoVector3D Self, FMonoVector3D Other) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Other.ToVector3D();
	//
	return FMonoVector3D(FVector::CrossProduct(V3A,V3B));
}

FMonoVector3D ICALL::API::FVector3D_DegreesToRadians(FMonoVector3D Degrees) {
	return FMonoVector3D(FVector::DegreesToRadians(Degrees.ToVector3D()));
}

FMonoVector3D ICALL::API::FVector3D_GetAbs(FMonoVector3D Self) {
	return FMonoVector3D(Self.ToVector3D().GetAbs());
}

FMonoVector3D ICALL::API::FVector3D_GetClampedToMaxSize(FMonoVector3D Self, float MaxSize) {
	return FMonoVector3D(Self.ToVector3D().GetClampedToMaxSize(MaxSize));
}

FMonoVector3D ICALL::API::FVector3D_GetClampedToSize(FMonoVector3D Self, float Min, float Max) {
	return FMonoVector3D(Self.ToVector3D().GetClampedToSize(Min,Max));
}

FMonoVector3D ICALL::API::FVector3D_GetSafeNormal(FMonoVector3D Self, float Tolerance) {
	return FMonoVector3D(Self.ToVector3D().GetSafeNormal(Tolerance));
}

FMonoVector3D ICALL::API::FVector3D_GetUnsafeNormal(FMonoVector3D Self) {
	return FMonoVector3D(Self.ToVector3D().GetUnsafeNormal());
}

FMonoVector3D ICALL::API::FVector3D_GetSignVector(FMonoVector3D Self) {
	return FMonoVector3D(Self.ToVector3D().GetSignVector());
}

FMonoVector3D ICALL::API::FVector3D_GridSnap(FMonoVector3D Self, float GridSize) {
	return FMonoVector3D(Self.ToVector3D().GridSnap(GridSize));
}

FMonoVector3D ICALL::API::FVector3D_MirrorByVector(FMonoVector3D Self, FMonoVector3D MirrorNormal) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = MirrorNormal.ToVector3D();
	//
	return FMonoVector3D(V3A.MirrorByVector(V3B));
}

FMonoVector3D ICALL::API::FVector3D_Projection(FMonoVector3D Self) {
	return FMonoVector3D(Self.ToVector3D().Projection());
}

FMonoVector3D ICALL::API::FVector3D_ProjectOnTo(FMonoVector3D Self, FMonoVector3D Other) {
	return FMonoVector3D(Self.ToVector3D().ProjectOnTo(Other.ToVector3D()));
}

FMonoVector3D ICALL::API::FVector3D_ProjectOnToNormal(FMonoVector3D Self, FMonoVector3D Normal) {
	return FMonoVector3D(Self.ToVector3D().ProjectOnToNormal(Normal.ToVector3D()));
}

FMonoVector3D ICALL::API::FVector3D_RadiansToDegrees(FMonoVector3D Radians) {
	return FMonoVector3D(FVector::RadiansToDegrees(Radians.ToVector3D()));
}

FMonoVector3D ICALL::API::FVector3D_Reciprocal(FMonoVector3D Self) {
	return FMonoVector3D(Self.ToVector3D().Reciprocal());
}

FMonoVector3D ICALL::API::FVector3D_RotateAngleAxis(FMonoVector3D Self, float Angle, FMonoVector3D Axis) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Axis.ToVector3D();
	//
	return FMonoVector3D(V3A.RotateAngleAxis(Angle,V3B));
}

FMonoVector3D ICALL::API::FVector3D_Normalize(FMonoVector3D Self, float Tolerance) {
	FVector V3A = Self.ToVector3D();
	V3A.Normalize(Tolerance);
	//
	return FMonoVector3D(V3A);
}

FMonoVector3D ICALL::API::FVector3D_VectorPlaneProject(FMonoVector3D Self, FMonoVector3D PlaneNormal) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = PlaneNormal.ToVector3D();
	//
	return FMonoVector3D(FVector::VectorPlaneProject(V3A,V3B));
}

FMonoRotator ICALL::API::FVector3D_Rotation(FMonoVector3D Self) {
	return FMonoRotator(Self.ToVector3D().Rotation());
}

float ICALL::API::FVector3D_CosineAngle2D(FMonoVector3D Self, FMonoVector3D Other) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Other.ToVector3D();
	//
	return V3A.CosineAngle2D(V3B);
}

float ICALL::API::FVector3D_BoxPushOut(FMonoVector3D Normal, FMonoVector3D Size) {
	FVector V3A = Normal.ToVector3D();
	FVector V3B = Size.ToVector3D();
	//
	return FVector::BoxPushOut(V3A,V3B);
}

float ICALL::API::FVector3D_Distance(FMonoVector3D Self, FMonoVector3D Other) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Other.ToVector3D();
	//
	return FVector::Distance(V3A,V3B);
}

float ICALL::API::FVector3D_DistSquared(FMonoVector3D Self, FMonoVector3D Other) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Other.ToVector3D();
	//
	return FVector::DistSquared(V3A,V3B);
}

float ICALL::API::FVector3D_DotProduct(FMonoVector3D Self, FMonoVector3D Other) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Other.ToVector3D();
	//
	return FVector::DotProduct(V3A,V3B);
}

float ICALL::API::FVector3D_GetAbsMax(FMonoVector3D Self) {
	return Self.ToVector3D().GetAbsMax();
}

float ICALL::API::FVector3D_GetAbsMin(FMonoVector3D Self) {
	return Self.ToVector3D().GetAbsMin();
}

float ICALL::API::FVector3D_GetMax(FMonoVector3D Self) {
	return Self.ToVector3D().GetMax();
}

float ICALL::API::FVector3D_GetMin(FMonoVector3D Self) {
	return Self.ToVector3D().GetMin();
}

float ICALL::API::FVector3D_HeadingAngle(FMonoVector3D Self) {
	return Self.ToVector3D().HeadingAngle();
}

float ICALL::API::FVector3D_PointPlaneDist(FMonoVector3D Self, FMonoVector3D PlaneBase, FMonoVector3D PlaneNormal) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = PlaneBase.ToVector3D();
	FVector V3C = PlaneNormal.ToVector3D();
	//
	return FVector::PointPlaneDist(V3A,V3B,V3C);
}

float ICALL::API::FVector3D_Triple(FMonoVector3D X, FMonoVector3D Y, FMonoVector3D Z) {
	FVector V3A = X.ToVector3D();
	FVector V3B = Y.ToVector3D();
	FVector V3C = Z.ToVector3D();
	//
	return FVector::Triple(V3A,V3B,V3C);
}

bool ICALL::API::FVector3D_Coincident(FMonoVector3D Normal1, FMonoVector3D Normal2, float ParallelCosineThreshold) {
	FVector V3A = Normal1.ToVector3D();
	FVector V3B = Normal2.ToVector3D();
	//
	return FVector::Coincident(V3A,V3B,ParallelCosineThreshold);
}

bool ICALL::API::FVector3D_ContainsNaN(FMonoVector3D Self) {
	return Self.ToVector3D().ContainsNaN();
}

bool ICALL::API::FVector3D_Coplanar(FMonoVector3D Base1, FMonoVector3D Normal1, FMonoVector3D Base2, FMonoVector3D Normal2, float ParallelCosineThreshold) {
	FVector V3A = Base1.ToVector3D();
	FVector V3B = Normal1.ToVector3D();
	FVector V3C = Base2.ToVector3D();
	FVector V3D = Normal2.ToVector3D();
	//
	return FVector::Coplanar(V3A,V3B,V3C,V3D);
}

bool ICALL::API::FVector3D_IsNormalized(FMonoVector3D Self) {
	return Self.ToVector3D().IsNormalized();
}

bool ICALL::API::FVector3D_IsUniform(FMonoVector3D Self, float Tolerance) {
	return Self.ToVector3D().IsUniform(Tolerance);
}

bool ICALL::API::FVector3D_IsUnit(FMonoVector3D Self, float LengthSquaredTolerance) {
	return Self.ToVector3D().IsUnit(LengthSquaredTolerance);
}

bool ICALL::API::FVector3D_Orthogonal(FMonoVector3D Normal1, FMonoVector3D Normal2, float OrthogonalCosineThreshold) {
	FVector V3A = Normal1.ToVector3D();
	FVector V3B = Normal2.ToVector3D();
	//
	return FVector::Orthogonal(V3A,V3B,OrthogonalCosineThreshold);
}

bool ICALL::API::FVector3D_Parallel(FMonoVector3D Normal1, FMonoVector3D Normal2, float ParallelCosineThreshold) {
	FVector V3A = Normal1.ToVector3D();
	FVector V3B = Normal2.ToVector3D();
	//
	return FVector::Parallel(V3A,V3B,ParallelCosineThreshold);
}

bool ICALL::API::FVector3D_PointsAreNear(FMonoVector3D Point1, FMonoVector3D Point2, float Dist) {
	FVector V3A = Point1.ToVector3D();
	FVector V3B = Point2.ToVector3D();
	//
	return FVector::PointsAreNear(V3A,V3B,Dist);
}

bool ICALL::API::FVector3D_PointsAreSame(FMonoVector3D Self, FMonoVector3D Other) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Other.ToVector3D();
	//
	return FVector::PointsAreSame(V3A,V3B);
}

bool ICALL::API::FVector3D_IsNearlyZero(FMonoVector3D Self) {
	return Self.ToVector3D().IsNearlyZero();
}

bool ICALL::API::FVector3D_IsZero(FMonoVector3D Self) {
	return Self.ToVector3D().IsZero();
}

bool ICALL::API::FVector3D_Equals(FMonoVector3D Self, FMonoVector3D Other) {
	FVector V3A = Self.ToVector3D();
	FVector V3B = Other.ToVector3D();
	//
	return V3A.Equals(V3B);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: ROTATOR API ::

FMonoRotator ICALL::API::FRotator_Add(FMonoRotator Self, float DeltaPitch, float DeltaYaw, float DeltaRoll) {
	return FMonoRotator(Self.ToRotator().Add(DeltaPitch,DeltaYaw,DeltaRoll));
}

FMonoRotator ICALL::API::FRotator_Clamp(FMonoRotator Self) {
	return FMonoRotator(Self.ToRotator().Clamp());
}

FMonoRotator ICALL::API::FRotator_GetDenormalized(FMonoRotator Self) {
	return FMonoRotator(Self.ToRotator().GetDenormalized());
}

FMonoRotator ICALL::API::FRotator_GetEquivalentRotator(FMonoRotator Self) {
	return FMonoRotator(Self.ToRotator().GetEquivalentRotator());
}

FMonoRotator ICALL::API::FRotator_GetInverse(FMonoRotator Self) {
	return FMonoRotator(Self.ToRotator().GetInverse());
}

FMonoRotator ICALL::API::FRotator_GetNormalized(FMonoRotator Self) {
	return FMonoRotator(Self.ToRotator().GetNormalized());
}

FMonoRotator ICALL::API::FRotator_GridSnap(FMonoRotator Self, FMonoRotator Snap) {
	FRotator RA = Self.ToRotator();
	FRotator RB = Snap.ToRotator();
	//
	return FMonoRotator(RA.GridSnap(RB));
}

FMonoRotator ICALL::API::FRotator_MakeFromEuler(FMonoVector3D Euler) {
	return FMonoRotator(FRotator::MakeFromEuler(Euler.ToVector3D()));
}

FMonoRotator ICALL::API::FRotator_Normalize(FMonoRotator Self) {
	FRotator RA = Self.ToRotator();
	RA.Normalize();
	//
	return FMonoRotator(RA);
}

FMonoVector3D ICALL::API::FRotator_Euler(FMonoRotator Self) {
	return FMonoVector3D(Self.ToRotator().Euler());
}

FMonoVector3D ICALL::API::FRotator_RotateVector(FMonoRotator Self, FMonoVector3D Vector) {
	return FMonoVector3D(Self.ToRotator().RotateVector(Vector.ToVector3D()));
}

FMonoVector3D ICALL::API::FRotator_UnrotateVector(FMonoRotator Self, FMonoVector3D Vector) {
	return FMonoVector3D(Self.ToRotator().UnrotateVector(Vector.ToVector3D()));
}

float ICALL::API::FRotator_GetManhattanDistance(FMonoRotator Self, FMonoRotator Rotator) {
	FRotator RA = Self.ToRotator();
	FRotator RB = Rotator.ToRotator();
	//
	return RA.GetManhattanDistance(RB);
}

float ICALL::API::FRotator_ClampAxis(FMonoRotator Self, float Angle) {
	return Self.ToRotator().ClampAxis(Angle);
}

bool ICALL::API::FRotator_ContainsNaN(FMonoRotator Self) {
	return Self.ToRotator().ContainsNaN();
}

bool ICALL::API::FRotator_IsNearlyZero(FMonoRotator Self, float Tolerance) {
	return Self.ToRotator().IsNearlyZero(Tolerance);
}

bool ICALL::API::FRotator_IsZero(FMonoRotator Self) {
	return Self.ToRotator().IsZero();
}

bool ICALL::API::FRotator_Equals(FMonoRotator Self, FMonoRotator Other, float Tolerance) {
	return Self.ToRotator().Equals(Other.ToRotator(),Tolerance);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: TRANSFORM API ::

FMonoTransform ICALL::API::FTransform_Multiply(FMonoTransform A, FMonoTransform B) {
	const FTransform TA = A.ToTransform();
	const FTransform TB = B.ToTransform();
	//
	FTransform T{};
	FTransform::Multiply(&T,&TA,&TB);
	//
	return FMonoTransform(T);
}

FMonoTransform ICALL::API::FTransform_Blend(FMonoTransform Self, FMonoTransform Atom1, FMonoTransform Atom2, float Alpha) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Atom1.ToTransform();
	FTransform TC = Atom2.ToTransform();
	//
	TA.Blend(TB,TC,Alpha);
	//
	return FMonoTransform(TA);
}

FMonoTransform ICALL::API::FTransform_BlendWith(FMonoTransform Self, FMonoTransform OtherAtom, float Alpha) {
	FTransform TA = Self.ToTransform();
	FTransform TB = OtherAtom.ToTransform();
	//
	TA.BlendWith(TB,Alpha);
	//
	return FMonoTransform(TA);
}

FMonoTransform ICALL::API::FTransform_CopyRotation(FMonoTransform Self, FMonoTransform Other) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	TA.CopyRotation(TB);
	//
	return FMonoTransform(TA);
}

FMonoTransform ICALL::API::FTransform_CopyScale3D(FMonoTransform Self, FMonoTransform Other) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	TA.CopyScale3D(TB);
	//
	return FMonoTransform(TA);
}

FMonoTransform ICALL::API::FTransform_CopyTranslation(FMonoTransform Self, FMonoTransform Other) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	TA.CopyTranslation(TB);
	//
	return FMonoTransform(TA);
}

FMonoTransform ICALL::API::FTransform_CopyTranslationAndScale3D(FMonoTransform Self, FMonoTransform Other) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	TA.CopyTranslationAndScale3D(TB);
	//
	return FMonoTransform(TA);
}

FMonoTransform ICALL::API::FTransform_GetRelativeTransform(FMonoTransform Self, FMonoTransform Other) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	return FMonoTransform(TA.GetRelativeTransform(TB));
}

FMonoTransform ICALL::API::FTransform_GetRelativeTransformReverse(FMonoTransform Self, FMonoTransform Other) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	return FMonoTransform(TA.GetRelativeTransformReverse(TB));
}

FMonoTransform ICALL::API::FTransform_GetScaled(FMonoTransform Self, FMonoVector3D Scale) {
	FTransform TA = Self.ToTransform();
	FVector VA = Scale.ToVector3D();
	//
	return FMonoTransform(TA.GetScaled(VA));
}

FMonoTransform ICALL::API::FTransform_Inverse(FMonoTransform Self) {
	FTransform TA = Self.ToTransform();
	//
	return FMonoTransform(TA.Inverse());
}

FMonoTransform ICALL::API::FTransform_NormalizeRotation(FMonoTransform Self) {
	FTransform TA = Self.ToTransform();
	TA.NormalizeRotation();
	//
	return FMonoTransform(TA);
}

FMonoVector3D ICALL::API::FTransform_AddTranslations(FMonoTransform A, FMonoTransform B) {
	FTransform TA = A.ToTransform();
	FTransform TB = B.ToTransform();
	//
	return FMonoVector3D(FTransform::AddTranslations(TA,TB));
}

FMonoVector3D ICALL::API::FTransform_GetTranslation(FMonoTransform Self) {
	FTransform TA = Self.ToTransform();
	//
	return FMonoVector3D(TA.GetTranslation());
}

FMonoVector3D ICALL::API::FTransform_GetRotation(FMonoTransform Self) {
	FTransform TA = Self.ToTransform();
	//
	return FMonoVector3D(TA.GetRotation().Euler());
}

FMonoVector3D ICALL::API::FTransform_GetScale3D(FMonoTransform Self) {
	FTransform TA = Self.ToTransform();
	//
	return FMonoVector3D(TA.GetScale3D());
}

float ICALL::API::FTransform_GetDeterminant(FMonoTransform Self) {
	return Self.ToTransform().GetDeterminant();
}

float ICALL::API::FTransform_GetMaximumAxisScale(FMonoTransform Self) {
	return Self.ToTransform().GetMaximumAxisScale();
}

float ICALL::API::FTransform_GetMinimumAxisScale(FMonoTransform Self) {
	return Self.ToTransform().GetMinimumAxisScale();
}

bool ICALL::API::FTransform_AnyHasNegativeScale(FMonoVector3D InScale3D, FMonoVector3D InOtherScale3D) {
	FVector VA = InScale3D.ToVector3D();
	FVector VB = InOtherScale3D.ToVector3D();
	//
	return FTransform::AnyHasNegativeScale(VA,VB);
}

bool ICALL::API::FTransform_AreRotationsEqual(FMonoTransform A, FMonoTransform B, float Tolerance) {
	FTransform TA = A.ToTransform();
	FTransform TB = B.ToTransform();
	//
	return FTransform::AreRotationsEqual(TA,TB,Tolerance);
}

bool ICALL::API::FTransform_AreScale3DsEqual(FMonoTransform A, FMonoTransform B, float Tolerance) {
	FTransform TA = A.ToTransform();
	FTransform TB = B.ToTransform();
	//
	return FTransform::AreScale3DsEqual(TA,TB,Tolerance);
}

bool ICALL::API::FTransform_AreTranslationsEqual(FMonoTransform A, FMonoTransform B, float Tolerance) {
	FTransform TA = A.ToTransform();
	FTransform TB = B.ToTransform();
	//
	return FTransform::AreTranslationsEqual(TA,TB,Tolerance);
}

bool ICALL::API::FTransform_ContainsNaN(FMonoTransform Self) {
	return Self.ToTransform().ContainsNaN();
}

bool ICALL::API::FTransform_EqualsNoScale(FMonoTransform Self, FMonoTransform Other, float Tolerance) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	return TA.EqualsNoScale(TB,Tolerance);
}

bool ICALL::API::FTransform_IsRotationNormalized(FMonoTransform Self) {
	return Self.ToTransform().IsRotationNormalized();
}

bool ICALL::API::FTransform_IsValid(FMonoTransform Self) {
	return Self.ToTransform().IsValid();
}

bool ICALL::API::FTransform_RotationEquals(FMonoTransform Self, FMonoTransform Other, float Tolerance) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	return TA.RotationEquals(TB,Tolerance);
}

bool ICALL::API::FTransform_Scale3DEquals(FMonoTransform Self, FMonoTransform Other, float Tolerance) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	return TA.Scale3DEquals(TB,Tolerance);
}

bool ICALL::API::FTransform_TranslationEquals(FMonoTransform Self, FMonoTransform Other, float Tolerance) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	return TA.TranslationEquals(TB,Tolerance);
}

bool ICALL::API::FTransform_Equals(FMonoTransform Self, FMonoTransform Other, float Tolerance) {
	FTransform TA = Self.ToTransform();
	FTransform TB = Other.ToTransform();
	//
	return TA.Equals(TB,Tolerance);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: WEAK POINTER API ::

bool ICALL::API::FClassPtr_IsStale(FMonoClass Self) {
	return (Self.ToClass()==nullptr||Self.ToClass()->IsPendingKill());
}

bool ICALL::API::FClassPtr_IsValid(FMonoClass Self) {
	return ((Self.ToClass()!=nullptr)&&(Self.ToClass()->IsValidLowLevel()));
}

bool ICALL::API::FClassPtr_IsChildOf(FMonoClass Self, FMonoClass Class) {
	if (Class.ToClass()==nullptr) {return false;}
	if (Self.ToClass()==nullptr) {return false;}
	//
	return (Self.ToClass()->IsChildOf(Class.ToClass()));
}

bool ICALL::API::FObjectPtr_IsA(FMonoObject Self, FMonoClass Class) {
	if (Self.ToObject()==nullptr) {return false;}
	if (Class.ToClass()==nullptr) {return false;}
	//
	return (Self.ToObject()->IsA(Class.ToClass()));
}

bool ICALL::API::FObjectPtr_IsStale(FMonoObject Self) {
	return (Self.ToObject()==nullptr||Self.ToObject()->IsPendingKill());
}

bool ICALL::API::FObjectPtr_IsValid(FMonoObject Self) {
	return ((Self.ToObject()!=nullptr)&&(Self.ToObject()->IsValidLowLevel()));
}

bool ICALL::API::FActorPtr_IsStale(FMonoActor Self) {
	return (Self.ToActor()==nullptr||Self.ToActor()->IsPendingKill());
}

bool ICALL::API::FActorPtr_IsValid(FMonoActor Self) {
	return ((Self.ToActor()!=nullptr)&&(Self.ToActor()->IsValidLowLevel()));
}

bool ICALL::API::FActorPtr_IsA(FMonoActor Self, FMonoClass Class) {
	if (Class.ToClass()==nullptr) {return false;}
	if (Self.ToActor()==nullptr) {return false;}
	//
	return (Self.ToActor()->IsA(Class.ToClass()));
}

bool ICALL::API::FComponentPtr_IsStale(FMonoComponent Self) {
	return (Self.ToComponent()==nullptr||Self.ToComponent()->IsPendingKill());
}

bool ICALL::API::FComponentPtr_IsValid(FMonoComponent Self) {
	return ((Self.ToComponent()!=nullptr)&&(Self.ToComponent()->IsValidLowLevel()));
}

bool ICALL::API::FComponentPtr_IsA(FMonoComponent Self, FMonoClass Class) {
	if (Self.ToComponent()==nullptr) {return false;}
	if (Class.ToClass()==nullptr) {return false;}
	//
	return (Self.ToComponent()->IsA(Class.ToClass()));
}

FMonoString ICALL::API::FClassPtr_GetName(FMonoClass Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (UClass*CLS=Self.ToClass()) {
		return FMonoString(CLS->GetName(),MonoCore.GetCoreDomain());
	} return FMonoString{};
}

FMonoString ICALL::API::FClassPtr_GetFullPath(FMonoClass Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (UClass*CLS=Self.ToClass()) {
		return FMonoString(CLS->GetFullName(),MonoCore.GetCoreDomain());
	} return FMonoString{};
}

FMonoClass ICALL::API::FObjectPtr_GetClass(FMonoObject Self) {
	if (Self.ToObject()==nullptr) {return FMonoClass{};}
	//
	return FMonoClass(Self.ToObject()->GetClass());
}

FMonoString ICALL::API::FObjectPtr_GetName(FMonoObject Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (UObject*OBJ=Self.ToObject()) {
		return FMonoString(OBJ->GetName(),MonoCore.GetCoreDomain());
	} return FMonoString{};
}

FMonoString ICALL::API::FObjectPtr_GetFullPath(FMonoObject Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (UObject*OBJ=Self.ToObject()) {
		return FMonoString(OBJ->GetFullName(),MonoCore.GetCoreDomain());
	} return FMonoString{};
}

FMonoClass ICALL::API::FActorPtr_GetClass(FMonoActor Self) {
	if (Self.ToActor()==nullptr) {return FMonoClass{};}
	//
	return FMonoClass(Self.ToActor()->GetClass());
}

FMonoString ICALL::API::FActorPtr_GetName(FMonoActor Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (AActor*ACT=Self.ToActor()) {
		return FMonoString(ACT->GetName(),MonoCore.GetCoreDomain());
	} return FMonoString{};
}

FMonoString ICALL::API::FActorPtr_GetFullPath(FMonoActor Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (AActor*ACT=Self.ToActor()) {
		return FMonoString(ACT->GetFullName(),MonoCore.GetCoreDomain());
	} return FMonoString{};
}

FMonoClass ICALL::API::FComponentPtr_GetClass(FMonoComponent Self) {
	if (Self.ToComponent()==nullptr) {return FMonoClass{};}
	//
	return FMonoClass(Self.ToComponent()->GetClass());
}

FMonoString ICALL::API::FComponentPtr_GetName(FMonoComponent Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (UActorComponent*CMP=Self.ToComponent()) {
		return FMonoString(CMP->GetName(),MonoCore.GetCoreDomain());
	} return FMonoString{};
}

FMonoString ICALL::API::FComponentPtr_GetFullPath(FMonoComponent Self) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (UActorComponent*CMP=Self.ToComponent()) {
		return FMonoString(CMP->GetFullName(),MonoCore.GetCoreDomain());
	} return FMonoString{};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: ARRAY API ::

void ICALL::API::TArray_Append(void* PropertyPtr, void* ArrayAddr, FMonoArray SourceArray) {
	FArrayProperty* SourceProperty = reinterpret_cast<FArrayProperty*>(SourceArray.Property);
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && SourceProperty) {
		if (Property->Inner->SameType(SourceProperty->Inner)) {
			UKismetArrayLibrary::GenericArray_Append(ArrayAddr,Property,SourceArray.ValueAddr,SourceProperty);
		}///
	}///
}

void ICALL::API::TArray_Clear(void* PropertyPtr, void* ArrayAddr) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		UKismetArrayLibrary::GenericArray_Clear(ArrayAddr,Property);
	}///
}

void ICALL::API::TArray_RemoveAt(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		UKismetArrayLibrary::GenericArray_Remove(ArrayAddr,Property,Index);
	}///
}

void ICALL::API::TArray_Reverse(void* PropertyPtr, void* ArrayAddr) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		UKismetArrayLibrary::GenericArray_Reverse(ArrayAddr,Property);
	}///
}

void ICALL::API::TArray_Shuffle(void* PropertyPtr, void* ArrayAddr) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		UKismetArrayLibrary::GenericArray_Shuffle(ArrayAddr,Property);
	}///
}

void ICALL::API::TArray_Swap(void* PropertyPtr, void* ArrayAddr, int32 A, int32 B) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		UKismetArrayLibrary::GenericArray_Swap(ArrayAddr,Property,A,B);
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int32 ICALL::API::TArray_Length(void* PropertyPtr, void* ArrayAddr) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		return UKismetArrayLibrary::GenericArray_Length(ArrayAddr,Property);
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_LastIndex(void* PropertyPtr, void* ArrayAddr) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		return UKismetArrayLibrary::GenericArray_LastIndex(ArrayAddr,Property);
	}///
	//
	return INDEX_NONE;
}

bool ICALL::API::TArray_IsValidIndex(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		return UKismetArrayLibrary::GenericArray_IsValidIndex(ArrayAddr,Property,Index);
	}///
	//
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ICALL::API::TArray_Get_Bool(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FBoolProperty*Inner=CastField<FBoolProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return Inner->GetPropertyValue(Array.GetRawPtr(Index));
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return false;
}

uint8 ICALL::API::TArray_Get_Byte(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return Inner->GetPropertyValue(Array.GetRawPtr(Index));
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return 0;
}

int32 ICALL::API::TArray_Get_Int(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FIntProperty*Inner=CastField<FIntProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return Inner->GetPropertyValue(Array.GetRawPtr(Index));
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return 0;
}

int64 ICALL::API::TArray_Get_Int64(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FInt64Property*Inner=CastField<FInt64Property>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return Inner->GetPropertyValue(Array.GetRawPtr(Index));
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return 0;
}

float ICALL::API::TArray_Get_Float(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FFloatProperty*Inner=CastField<FFloatProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return Inner->GetPropertyValue(Array.GetRawPtr(Index));
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return 0.f;
}

FMonoString ICALL::API::TArray_Get_String(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return FMonoString(Inner->GetPropertyValue(Array.GetRawPtr(Index)),MonoCore.GetCoreDomain());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return FMonoString(TEXT(""),MonoCore.GetCoreDomain());
}

FMonoName ICALL::API::TArray_Get_Name(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return FMonoName(Inner->GetPropertyValue(Array.GetRawPtr(Index)),MonoCore.GetCoreDomain());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return FMonoName(TEXT(""),MonoCore.GetCoreDomain());
}

FMonoText ICALL::API::TArray_Get_Text(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FTextProperty*Inner=CastField<FTextProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return FMonoText(Inner->GetPropertyValue(Array.GetRawPtr(Index)),MonoCore.GetCoreDomain());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return FMonoText(FText::FromString(TEXT("")),MonoCore.GetCoreDomain());
}

FMonoColor ICALL::API::TArray_Get_Color(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					const uint8* ValuePtr = Array.GetRawPtr(Index);
					//
					FColor Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					return FMonoColor(Value);
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
						Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
						ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
	//
	return FMonoColor{};
}

FMonoRotator ICALL::API::TArray_Get_Rotator(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FRotator>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					const uint8* ValuePtr = Array.GetRawPtr(Index);
					//
					FRotator Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					return FMonoRotator(Value);
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
						Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
						ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
	//
	return FMonoRotator{};
}

FMonoVector2D ICALL::API::TArray_Get_Vector2D(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					const uint8* ValuePtr = Array.GetRawPtr(Index);
					//
					FVector2D Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					return FMonoVector2D(Value);
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
						Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
						ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
	//
	return FMonoVector2D{};
}

FMonoVector3D ICALL::API::TArray_Get_Vector3D(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					const uint8* ValuePtr = Array.GetRawPtr(Index);
					//
					FVector Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					return FMonoVector3D(Value);
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
						Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
						ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
	//
	return FMonoVector3D{};
}

FMonoTransform ICALL::API::TArray_Get_Transform(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FTransform>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					const uint8* ValuePtr = Array.GetRawPtr(Index);
					//
					FTransform Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					return FMonoTransform(Value);
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
						Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
						ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
	//
	return FMonoTransform{};
}

FMonoClass ICALL::API::TArray_Get_Class(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FClassProperty*Inner=CastField<FClassProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return FMonoClass(Cast<UClass>(Inner->GetPropertyValue(Array.GetRawPtr(Index))));
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return FMonoClass{};
}

FMonoObject ICALL::API::TArray_Get_Object(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return FMonoObject(Inner->GetPropertyValue(Array.GetRawPtr(Index)));
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return FMonoObject{};
}

FMonoActor ICALL::API::TArray_Get_Actor(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return FMonoActor(Cast<AActor>(Inner->GetPropertyValue(Array.GetRawPtr(Index))));
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return FMonoActor{};
}

FMonoComponent ICALL::API::TArray_Get_Component(void* PropertyPtr, void* ArrayAddr, int32 Index) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				return FMonoComponent(Cast<UActorComponent>(Inner->GetPropertyValue(Array.GetRawPtr(Index))));
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"),
					Index, *Property->GetName(), Array.Num(), *Property->GetOwnerVariant().GetPathName()),
					ELogVerbosity::Warning, TEXT("GetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
	//
	return FMonoComponent{};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ICALL::API::TArray_Set_Bool(void* PropertyPtr, void* ArrayAddr, int32 Index, bool NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FBoolProperty*Inner=CastField<FBoolProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue);
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Byte(void* PropertyPtr, void* ArrayAddr, int32 Index, uint8 NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue);
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Int(void* PropertyPtr, void* ArrayAddr, int32 Index, int32 NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FIntProperty*Inner=CastField<FIntProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue);
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Int64(void* PropertyPtr, void* ArrayAddr, int32 Index, int64 NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FInt64Property*Inner=CastField<FInt64Property>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue);
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Float(void* PropertyPtr, void* ArrayAddr, int32 Index, float NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FFloatProperty*Inner=CastField<FFloatProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue);
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_String(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoString NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue.ToString());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Name(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoName NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue.ToName());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Text(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoText NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FTextProperty*Inner=CastField<FTextProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue.ToText());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Color(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoColor NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					auto ValuePtr = Inner->ContainerPtrToValuePtr<FColor>(Array.GetRawPtr(Index));
					if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToColor();}
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
						*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Rotator(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoRotator NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FRotator>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					auto ValuePtr = Inner->ContainerPtrToValuePtr<FRotator>(Array.GetRawPtr(Index));
					if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToRotator();}
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
						*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Vector2D(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoVector2D NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					auto ValuePtr = Inner->ContainerPtrToValuePtr<FVector2D>(Array.GetRawPtr(Index));
					if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToVector2D();}
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
						*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Vector3D(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoVector3D NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					auto ValuePtr = Inner->ContainerPtrToValuePtr<FVector>(Array.GetRawPtr(Index));
					if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToVector3D();}
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
						*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Transform(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoTransform NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FTransform>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				if (Array.IsValidIndex(Index)) {
					auto ValuePtr = Inner->ContainerPtrToValuePtr<FTransform>(Array.GetRawPtr(Index));
					if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToTransform();}
				} else {
					FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
						*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
					);//
				}///
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Class(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoClass NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FClassProperty*Inner=CastField<FClassProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue.ToClass());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Object(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoObject NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue.ToObject());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Actor(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoActor NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue.ToActor());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

void ICALL::API::TArray_Set_Component(void* PropertyPtr, void* ArrayAddr, int32 Index, FMonoComponent NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			if (Array.IsValidIndex(Index)) {
				Inner->SetPropertyValue(Array.GetRawPtr(Index),NewValue.ToComponent());
			} else {
				FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to set an invalid index on array: %s [%d/%d]!"),
					*Property->GetName(), Index, UKismetArrayLibrary::GetLastIndex(Array)), ELogVerbosity::Warning, TEXT("SetOutOfBoundsWarning")
				);//
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ICALL::API::TArray_Add_Bool(void* PropertyPtr, void* ArrayAddr, bool NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FBoolProperty*Inner=CastField<FBoolProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue);
		}///
	}///
}

void ICALL::API::TArray_Add_Byte(void* PropertyPtr, void* ArrayAddr, uint8 NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue);
		}///
	}///
}

void ICALL::API::TArray_Add_Int(void* PropertyPtr, void* ArrayAddr, int32 NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FIntProperty*Inner=CastField<FIntProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue);
		}///
	}///
}

void ICALL::API::TArray_Add_Int64(void* PropertyPtr, void* ArrayAddr, int64 NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FInt64Property*Inner=CastField<FInt64Property>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue);
		}///
	}///
}

void ICALL::API::TArray_Add_Float(void* PropertyPtr, void* ArrayAddr, float NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FFloatProperty*Inner=CastField<FFloatProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue);
		}///
	}///
}

void ICALL::API::TArray_Add_String(void* PropertyPtr, void* ArrayAddr, FMonoString NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue.ToString());
		}///
	}///
}

void ICALL::API::TArray_Add_Name(void* PropertyPtr, void* ArrayAddr, FMonoName NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue.ToName());
		}///
	}///
}

void ICALL::API::TArray_Add_Text(void* PropertyPtr, void* ArrayAddr, FMonoText NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FTextProperty*Inner=CastField<FTextProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue.ToText());
		}///
	}///
}

void ICALL::API::TArray_Add_Color(void* PropertyPtr, void* ArrayAddr, FMonoColor NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				int32 I = Array.AddValue();
				//
				auto ValuePtr = Inner->ContainerPtrToValuePtr<FColor>(Array.GetRawPtr(I));
				if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToColor();}
			}///
		}///
	}///
}

void ICALL::API::TArray_Add_Vector2D(void* PropertyPtr, void* ArrayAddr, FMonoVector2D NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				int32 I = Array.AddValue();
				//
				auto ValuePtr = Inner->ContainerPtrToValuePtr<FVector2D>(Array.GetRawPtr(I));
				if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToVector2D();}
			}///
		}///
	}///
}

void ICALL::API::TArray_Add_Vector3D(void* PropertyPtr, void* ArrayAddr, FMonoVector3D NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				int32 I = Array.AddValue();
				//
				auto ValuePtr = Inner->ContainerPtrToValuePtr<FVector>(Array.GetRawPtr(I));
				if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToVector3D();}
			}///
		}///
	}///
}

void ICALL::API::TArray_Add_Rotator(void* PropertyPtr, void* ArrayAddr, FMonoRotator NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FRotator>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				int32 I = Array.AddValue();
				//
				auto ValuePtr = Inner->ContainerPtrToValuePtr<FRotator>(Array.GetRawPtr(I));
				if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToRotator();}
			}///
		}///
	}///
}

void ICALL::API::TArray_Add_Transform(void* PropertyPtr, void* ArrayAddr, FMonoTransform NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FTransform>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				int32 I = Array.AddValue();
				//
				auto ValuePtr = Inner->ContainerPtrToValuePtr<FTransform>(Array.GetRawPtr(I));
				if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToTransform();}
			}///
		}///
	}///
}

void ICALL::API::TArray_Add_Class(void* PropertyPtr, void* ArrayAddr, FMonoClass NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FClassProperty*Inner=CastField<FClassProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue.ToClass());
		}///
	}///
}

void ICALL::API::TArray_Add_Object(void* PropertyPtr, void* ArrayAddr, FMonoObject NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue.ToObject());
		}///
	}///
}

void ICALL::API::TArray_Add_Actor(void* PropertyPtr, void* ArrayAddr, FMonoActor NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue.ToActor());
		}///
	}///
}

void ICALL::API::TArray_Add_Component(void* PropertyPtr, void* ArrayAddr, FMonoComponent NewValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			int32 I = Array.AddValue();
			//
			Inner->SetPropertyValue(Array.GetRawPtr(I),NewValue.ToComponent());
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ICALL::API::TArray_Contains_Bool(void* PropertyPtr, void* ArrayAddr, bool TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FBoolProperty*Inner=CastField<FBoolProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Byte(void* PropertyPtr, void* ArrayAddr, uint8 TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Int(void* PropertyPtr, void* ArrayAddr, int32 TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FIntProperty*Inner=CastField<FIntProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Int64(void* PropertyPtr, void* ArrayAddr, int64 TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FInt64Property*Inner=CastField<FInt64Property>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Float(void* PropertyPtr, void* ArrayAddr, float TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FFloatProperty*Inner=CastField<FFloatProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_String(void* PropertyPtr, void* ArrayAddr, FMonoString TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue.ToString()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Name(void* PropertyPtr, void* ArrayAddr, FMonoName TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue.ToName()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Text(void* PropertyPtr, void* ArrayAddr, FMonoText TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FTextProperty*Inner=CastField<FTextProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I)).EqualTo(TypedValue.ToText())) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Color(void* PropertyPtr, void* ArrayAddr, FMonoColor TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FColor Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedValue.ToColor()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Rotator(void* PropertyPtr, void* ArrayAddr, FMonoRotator TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FRotator>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FRotator Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedValue.ToRotator()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Vector2D(void* PropertyPtr, void* ArrayAddr, FMonoVector2D TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FVector2D Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedValue.ToVector2D()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Vector3D(void* PropertyPtr, void* ArrayAddr, FMonoVector3D TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FVector Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedValue.ToVector3D()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Transform(void* PropertyPtr, void* ArrayAddr, FMonoTransform TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FTransform>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FTransform Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value.Equals(TypedValue.ToTransform())) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Class(void* PropertyPtr, void* ArrayAddr, FMonoClass TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FClassProperty*Inner=CastField<FClassProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				UClass* Field = Cast<UClass>(Inner->GetObjectPropertyValue(Array.GetRawPtr(I)));
				if (Field==TypedValue.ToClass()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Object(void* PropertyPtr, void* ArrayAddr, FMonoObject TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				UObject* Field = Inner->GetObjectPropertyValue(Array.GetRawPtr(I));
				if (Field==TypedValue.ToObject()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Actor(void* PropertyPtr, void* ArrayAddr, FMonoActor TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				AActor* Field = Cast<AActor>(Inner->GetObjectPropertyValue(Array.GetRawPtr(I)));
				if (Field==TypedValue.ToActor()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TArray_Contains_Component(void* PropertyPtr, void* ArrayAddr, FMonoComponent TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				UActorComponent* Field = Cast<UActorComponent>(Inner->GetObjectPropertyValue(Array.GetRawPtr(I)));
				if (Field==TypedValue.ToComponent()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int32 ICALL::API::TArray_FindItem_Bool(void* PropertyPtr, void* ArrayAddr, bool TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FBoolProperty*Inner=CastField<FBoolProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Byte(void* PropertyPtr, void* ArrayAddr, uint8 TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Int(void* PropertyPtr, void* ArrayAddr, int32 TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FIntProperty*Inner=CastField<FIntProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Int64(void* PropertyPtr, void* ArrayAddr, int64 TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FInt64Property*Inner=CastField<FInt64Property>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Float(void* PropertyPtr, void* ArrayAddr, float TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FFloatProperty*Inner=CastField<FFloatProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_String(void* PropertyPtr, void* ArrayAddr, FMonoString TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue.ToString()) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Name(void* PropertyPtr, void* ArrayAddr, FMonoName TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I))==TypedValue.ToName()) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Text(void* PropertyPtr, void* ArrayAddr, FMonoText TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FTextProperty*Inner=CastField<FTextProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				if (Inner->GetPropertyValue(Array.GetRawPtr(I)).EqualTo(TypedValue.ToText())) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Color(void* PropertyPtr, void* ArrayAddr, FMonoColor TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FColor Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedValue.ToColor()) {return I;}
				}///
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Rotator(void* PropertyPtr, void* ArrayAddr, FMonoRotator TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FRotator>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FRotator Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedValue.ToRotator()) {return I;}
				}///
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Vector2D(void* PropertyPtr, void* ArrayAddr, FMonoVector2D TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FVector2D Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedValue.ToVector2D()) {return I;}
				}///
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Vector3D(void* PropertyPtr, void* ArrayAddr, FMonoVector3D TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FVector Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedValue.ToVector3D()) {return I;}
				}///
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Transform(void* PropertyPtr, void* ArrayAddr, FMonoTransform TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->Inner)) {
			if (Inner->Struct==TBaseStructure<FTransform>::Get()) {
				FScriptArrayHelper Array(Property,ArrayAddr);
				//
				for (int32 I=0; I<Array.Num(); ++I) {
					const uint8* ValuePtr = Array.GetRawPtr(I);
					//
					FTransform Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value.Equals(TypedValue.ToTransform())) {return I;}
				}///
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Class(void* PropertyPtr, void* ArrayAddr, FMonoClass TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FClassProperty*Inner=CastField<FClassProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				UClass* Field = Cast<UClass>(Inner->GetObjectPropertyValue(Array.GetRawPtr(I)));
				if (Field==TypedValue.ToClass()) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Object(void* PropertyPtr, void* ArrayAddr, FMonoObject TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				UObject* Field = Inner->GetObjectPropertyValue(Array.GetRawPtr(I));
				if (Field==TypedValue.ToObject()) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Actor(void* PropertyPtr, void* ArrayAddr, FMonoActor TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				AActor* Field = Cast<AActor>(Inner->GetObjectPropertyValue(Array.GetRawPtr(I)));
				if (Field==TypedValue.ToActor()) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

int32 ICALL::API::TArray_FindItem_Component(void* PropertyPtr, void* ArrayAddr, FMonoComponent TypedValue) {
	FArrayProperty* Property = reinterpret_cast<FArrayProperty*>(PropertyPtr);
	//
	if (Property && ArrayAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
			FScriptArrayHelper Array(Property,ArrayAddr);
			//
			for (int32 I=0; I<Array.Num(); ++I) {
				UActorComponent* Field = Cast<UActorComponent>(Inner->GetObjectPropertyValue(Array.GetRawPtr(I)));
				if (Field==TypedValue.ToComponent()) {return I;}
			}///
		}///
	}///
	//
	return INDEX_NONE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ICALL::API::TArray_RemoveItem_Bool(void* PropertyPtr, void* ArrayAddr, bool TypedValue) {
	int32 RemoveNext = TArray_FindItem_Bool(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Bool(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Byte(void* PropertyPtr, void* ArrayAddr, uint8 TypedValue) {
	int32 RemoveNext = TArray_FindItem_Byte(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Byte(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Int(void* PropertyPtr, void* ArrayAddr, int32 TypedValue) {
	int32 RemoveNext = TArray_FindItem_Int(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Int(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Int64(void* PropertyPtr, void* ArrayAddr, int64 TypedValue) {
	int32 RemoveNext = TArray_FindItem_Int64(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Int64(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Float(void* PropertyPtr, void* ArrayAddr, float TypedValue) {
	int32 RemoveNext = TArray_FindItem_Float(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Float(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_String(void* PropertyPtr, void* ArrayAddr, FMonoString TypedValue) {
	int32 RemoveNext = TArray_FindItem_String(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_String(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Name(void* PropertyPtr, void* ArrayAddr, FMonoName TypedValue) {
	int32 RemoveNext = TArray_FindItem_Name(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Name(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Text(void* PropertyPtr, void* ArrayAddr, FMonoText TypedValue) {
	int32 RemoveNext = TArray_FindItem_Text(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Text(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Color(void* PropertyPtr, void* ArrayAddr, FMonoColor TypedValue) {
	int32 RemoveNext = TArray_FindItem_Color(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Color(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Rotator(void* PropertyPtr, void* ArrayAddr, FMonoRotator TypedValue) {
	int32 RemoveNext = TArray_FindItem_Rotator(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Rotator(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Vector2D(void* PropertyPtr, void* ArrayAddr, FMonoVector2D TypedValue) {
	int32 RemoveNext = TArray_FindItem_Vector2D(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Vector2D(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Vector3D(void* PropertyPtr, void* ArrayAddr, FMonoVector3D TypedValue) {
	int32 RemoveNext = TArray_FindItem_Vector3D(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Vector3D(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Transform(void* PropertyPtr, void* ArrayAddr, FMonoTransform TypedValue) {
	int32 RemoveNext = TArray_FindItem_Transform(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Transform(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Class(void* PropertyPtr, void* ArrayAddr, FMonoClass TypedValue) {
	int32 RemoveNext = TArray_FindItem_Class(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Class(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Object(void* PropertyPtr, void* ArrayAddr, FMonoObject TypedValue) {
	int32 RemoveNext = TArray_FindItem_Object(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Object(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Actor(void* PropertyPtr, void* ArrayAddr, FMonoActor TypedValue) {
	int32 RemoveNext = TArray_FindItem_Actor(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Actor(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

bool ICALL::API::TArray_RemoveItem_Component(void* PropertyPtr, void* ArrayAddr, FMonoComponent TypedValue) {
	int32 RemoveNext = TArray_FindItem_Component(PropertyPtr,ArrayAddr,TypedValue);
	//
	bool Removed = false;
	while(RemoveNext != INDEX_NONE) {
		TArray_RemoveAt(PropertyPtr,ArrayAddr,RemoveNext);
		RemoveNext = TArray_FindItem_Component(PropertyPtr,ArrayAddr,TypedValue);
		//
		Removed = true;
	}///
	//
	return Removed;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: SET API ::

void ICALL::API::TSet_Append(void* PropertyPtr, void* SetAddr, FMonoArray SourceArray) {
	FArrayProperty* SourceProperty = reinterpret_cast<FArrayProperty*>(SourceArray.Property);
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SourceProperty) {
		UBlueprintSetLibrary::GenericSet_AddItems(SetAddr,Property,SourceArray.ValueAddr,SourceProperty);
	}///
}

void ICALL::API::TSet_Clear(void* PropertyPtr, void* SetAddr) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		UBlueprintSetLibrary::GenericSet_Clear(SetAddr,Property);
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int32 ICALL::API::TSet_Length(void* PropertyPtr, void* SetAddr) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		return UBlueprintSetLibrary::GenericSet_Length(SetAddr,Property);
	}///
	//
	return INDEX_NONE;
}

FMonoArray ICALL::API::TSet_ToArray(void* PropertyPtr, void* SetAddr, FMonoArray TargetArray) {
	FArrayProperty* TargetArr = reinterpret_cast<FArrayProperty*>(TargetArray.Property);
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && TargetArr) {
		UKismetArrayLibrary::GenericArray_Clear(TargetArray.ValueAddr,TargetArr);
		UBlueprintSetLibrary::GenericSet_ToArray(SetAddr,Property,TargetArray.ValueAddr,TargetArr);
	}///
	//
	return TargetArray;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ICALL::API::TSet_Add_Byte(void* PropertyPtr, void* SetAddr, uint8 NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue);
			//
			Set.Rehash();
		}///
	}///
}

void ICALL::API::TSet_Add_Int(void* PropertyPtr, void* SetAddr, int32 NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FIntProperty*Inner=CastField<FIntProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue);
			//
			Set.Rehash();
		}///
	}///
}

void ICALL::API::TSet_Add_Int64(void* PropertyPtr, void* SetAddr, int64 NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FInt64Property*Inner=CastField<FInt64Property>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue);
			//
			Set.Rehash();
		}///
	}///
}

void ICALL::API::TSet_Add_Float(void* PropertyPtr, void* SetAddr, float NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FFloatProperty*Inner=CastField<FFloatProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue);
			//
			Set.Rehash();
		}///
	}///
}

void ICALL::API::TSet_Add_String(void* PropertyPtr, void* SetAddr, FMonoString NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue.ToString());
			//
			Set.Rehash();
		}///
	}///
}

void ICALL::API::TSet_Add_Name(void* PropertyPtr, void* SetAddr, FMonoName NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue.ToName());
			//
			Set.Rehash();
		}///
	}///
}

void ICALL::API::TSet_Add_Color(void* PropertyPtr, void* SetAddr, FMonoColor NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->ElementProp)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptSetHelper Set(Property,SetAddr);
				//
				int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
				auto ValuePtr = Inner->ContainerPtrToValuePtr<FColor>(Set.GetElementPtr(I));
				if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToColor();}
				//
				Set.Rehash();
			}///
		}///
	}///
}

void ICALL::API::TSet_Add_Vector2D(void* PropertyPtr, void* SetAddr, FMonoVector2D NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->ElementProp)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptSetHelper Set(Property,SetAddr);
				//
				int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
				auto ValuePtr = Inner->ContainerPtrToValuePtr<FVector2D>(Set.GetElementPtr(I));
				if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToVector2D();}
				//
				Set.Rehash();
			}///
		}///
	}///
}

void ICALL::API::TSet_Add_Vector3D(void* PropertyPtr, void* SetAddr, FMonoVector3D NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->ElementProp)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptSetHelper Set(Property,SetAddr);
				//
				int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
				auto ValuePtr = Inner->ContainerPtrToValuePtr<FVector>(Set.GetElementPtr(I));
				if (ValuePtr!=nullptr) {(*ValuePtr)=NewValue.ToVector3D();}
				//
				Set.Rehash();
			}///
		}///
	}///
}

void ICALL::API::TSet_Add_Class(void* PropertyPtr, void* SetAddr, FMonoClass NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FClassProperty*Inner=CastField<FClassProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue.ToClass());
			//
			Set.Rehash();
		}///
	}///
}

void ICALL::API::TSet_Add_Object(void* PropertyPtr, void* SetAddr, FMonoObject NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue.ToObject());
			//
			Set.Rehash();
		}///
	}///
}

void ICALL::API::TSet_Add_Actor(void* PropertyPtr, void* SetAddr, FMonoActor NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue.ToActor());
			//
			Set.Rehash();
		}///
	}///
}

void ICALL::API::TSet_Add_Component(void* PropertyPtr, void* SetAddr, FMonoComponent NewValue) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			int32 I = Set.AddDefaultValue_Invalid_NeedsRehash();
			Inner->SetPropertyValue(Set.GetElementPtr(I),NewValue.ToComponent());
			//
			Set.Rehash();
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ICALL::API::TSet_Contains_Byte(void* PropertyPtr, void* SetAddr, uint8 TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Int(void* PropertyPtr, void* SetAddr, int32 TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FIntProperty*Inner=CastField<FIntProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Int64(void* PropertyPtr, void* SetAddr, int64 TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FInt64Property*Inner=CastField<FInt64Property>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Float(void* PropertyPtr, void* SetAddr, float TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FFloatProperty*Inner=CastField<FFloatProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_String(void* PropertyPtr, void* SetAddr, FMonoString TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement.ToString()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Name(void* PropertyPtr, void* SetAddr, FMonoName TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement.ToName()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Color(void* PropertyPtr, void* SetAddr, FMonoColor TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->ElementProp)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptSetHelper Set(Property,SetAddr);
				//
				for (int32 I=0; I<Set.Num(); ++I) {
					const uint8* ValuePtr = Set.GetElementPtr(I);
					//
					FColor Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedElement.ToColor()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Vector2D(void* PropertyPtr, void* SetAddr, FMonoVector2D TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->ElementProp)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptSetHelper Set(Property,SetAddr);
				//
				for (int32 I=0; I<Set.Num(); ++I) {
					const uint8* ValuePtr = Set.GetElementPtr(I);
					//
					FVector2D Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedElement.ToVector2D()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Vector3D(void* PropertyPtr, void* SetAddr, FMonoVector3D TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->ElementProp)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptSetHelper Set(Property,SetAddr);
				//
				for (int32 I=0; I<Set.Num(); ++I) {
					const uint8* ValuePtr = Set.GetElementPtr(I);
					//
					FVector Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedElement.ToVector3D()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Class(void* PropertyPtr, void* SetAddr, FMonoClass TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				UClass* Class = Cast<UClass>(Inner->GetPropertyValue(Set.GetElementPtr(I)));
				if (Class==TypedElement.ToClass()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Object(void* PropertyPtr, void* SetAddr, FMonoObject TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement.ToObject()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Actor(void* PropertyPtr, void* SetAddr, FMonoActor TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				AActor* Actor = Cast<AActor>(Inner->GetPropertyValue(Set.GetElementPtr(I)));
				if (Actor==TypedElement.ToActor()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_Contains_Component(void* PropertyPtr, void* SetAddr, FMonoComponent TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=0; I<Set.Num(); ++I) {
				UActorComponent* Component = Cast<UActorComponent>(Inner->GetPropertyValue(Set.GetElementPtr(I)));
				if (Component==TypedElement.ToComponent()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ICALL::API::TSet_RemoveItem_Byte(void* PropertyPtr, void* SetAddr, uint8 TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Int(void* PropertyPtr, void* SetAddr, int32 TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FIntProperty*Inner=CastField<FIntProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Int64(void* PropertyPtr, void* SetAddr, int64 TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FInt64Property*Inner=CastField<FInt64Property>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Float(void* PropertyPtr, void* SetAddr, float TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FFloatProperty*Inner=CastField<FFloatProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_String(void* PropertyPtr, void* SetAddr, FMonoString TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement.ToString()) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Name(void* PropertyPtr, void* SetAddr, FMonoName TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement.ToName()) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Color(void* PropertyPtr, void* SetAddr, FMonoColor TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->ElementProp)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptSetHelper Set(Property,SetAddr);
				//
				for (int32 I=Set.Num(); I>=0; --I) {
					const uint8* ValuePtr = Set.GetElementPtr(I);
					//
					FColor Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedElement.ToColor()) {
						Set.RemoveAt(I); Set.Rehash();
						return true;
					}///
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Vector2D(void* PropertyPtr, void* SetAddr, FMonoVector2D TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->ElementProp)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptSetHelper Set(Property,SetAddr);
				//
				for (int32 I=Set.Num(); I>=0; --I) {
					const uint8* ValuePtr = Set.GetElementPtr(I);
					//
					FVector2D Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedElement.ToVector2D()) {
						Set.RemoveAt(I); Set.Rehash();
						return true;
					}///
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Vector3D(void* PropertyPtr, void* SetAddr, FMonoVector3D TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->ElementProp)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptSetHelper Set(Property,SetAddr);
				//
				for (int32 I=Set.Num(); I>=0; --I) {
					const uint8* ValuePtr = Set.GetElementPtr(I);
					//
					FVector Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedElement.ToVector3D()) {
						Set.RemoveAt(I); Set.Rehash();
						return true;
					}///
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Class(void* PropertyPtr, void* SetAddr, FMonoClass TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FClassProperty*Inner=CastField<FClassProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Cast<UClass>(Inner->GetPropertyValue(Set.GetElementPtr(I)))==TypedElement.ToClass()) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Object(void* PropertyPtr, void* SetAddr, FMonoObject TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Set.GetElementPtr(I))==TypedElement.ToObject()) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Actor(void* PropertyPtr, void* SetAddr, FMonoActor TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Cast<AActor>(Inner->GetPropertyValue(Set.GetElementPtr(I)))==TypedElement.ToActor()) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TSet_RemoveItem_Component(void* PropertyPtr, void* SetAddr, FMonoComponent TypedElement) {
	FSetProperty* Property = reinterpret_cast<FSetProperty*>(PropertyPtr);
	//
	if (Property && SetAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
			FScriptSetHelper Set(Property,SetAddr);
			//
			for (int32 I=Set.Num()-1; I>=0; --I) {
				if (Cast<UActorComponent>(Inner->GetPropertyValue(Set.GetElementPtr(I)))==TypedElement.ToComponent()) {
					Set.RemoveAt(I); Set.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// :: MAP API ::

void ICALL::API::TMap_Clear(void* PropertyPtr, void* MapAddr) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		UBlueprintMapLibrary::GenericMap_Clear(MapAddr,Property);
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int32 ICALL::API::TMap_Length(void* PropertyPtr, void* MapAddr) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		return UBlueprintMapLibrary::GenericMap_Length(MapAddr,Property);
	}///
	//
	return INDEX_NONE;
}

FMonoArray ICALL::API::TMap_Keys(void* PropertyPtr, void* MapAddr, FMonoArray TargetArray) {
	FArrayProperty* TargetArr = reinterpret_cast<FArrayProperty*>(TargetArray.Property);
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && TargetArr) {
		UBlueprintMapLibrary::GenericMap_Keys(MapAddr,Property,TargetArray.ValueAddr,TargetArr);
	}///
	//
	return TargetArray;
}

FMonoArray ICALL::API::TMap_Values(void* PropertyPtr, void* MapAddr, FMonoArray TargetArray) {
	FArrayProperty* TargetArr = reinterpret_cast<FArrayProperty*>(TargetArray.Property);
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && TargetArr) {
		UBlueprintMapLibrary::GenericMap_Values(MapAddr,Property,TargetArray.ValueAddr,TargetArr);
	}///
	//
	return TargetArray;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ICALL::API::TMap_FindItem_Byte(void* PropertyPtr, void* MapAddr, uint8 TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Int(void* PropertyPtr, void* MapAddr, int32 TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FIntProperty*Inner=CastField<FIntProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Int64(void* PropertyPtr, void* MapAddr, int64 TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FInt64Property*Inner=CastField<FInt64Property>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Float(void* PropertyPtr, void* MapAddr, float TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FFloatProperty*Inner=CastField<FFloatProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_String(void* PropertyPtr, void* MapAddr, FMonoString TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey.ToString()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Name(void* PropertyPtr, void* MapAddr, FMonoName TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey.ToName()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Color(void* PropertyPtr, void* MapAddr, FMonoColor TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->KeyProp)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptMapHelper Map(Property,MapAddr);
				//
				for (int32 I=0; I<Map.Num(); ++I) {
					const uint8* ValuePtr = Map.GetKeyPtr(I);
					//
					FColor Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedKey.ToColor()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Vector2D(void* PropertyPtr, void* MapAddr, FMonoVector2D TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->KeyProp)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptMapHelper Map(Property,MapAddr);
				//
				for (int32 I=0; I<Map.Num(); ++I) {
					const uint8* ValuePtr = Map.GetKeyPtr(I);
					//
					FVector2D Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedKey.ToVector2D()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Vector3D(void* PropertyPtr, void* MapAddr, FMonoVector3D TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->KeyProp)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptMapHelper Map(Property,MapAddr);
				//
				for (int32 I=0; I<Map.Num(); ++I) {
					const uint8* ValuePtr = Map.GetKeyPtr(I);
					//
					FVector Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedKey.ToVector3D()) {return true;}
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Class(void* PropertyPtr, void* MapAddr, FMonoClass TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FClassProperty*Inner=CastField<FClassProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				UClass* Class = Cast<UClass>(Inner->GetPropertyValue(Map.GetKeyPtr(I)));
				if (Class==TypedKey.ToClass()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Object(void* PropertyPtr, void* MapAddr, FMonoObject TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey.ToObject()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Actor(void* PropertyPtr, void* MapAddr, FMonoActor TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				AActor* Actor = Cast<AActor>(Inner->GetPropertyValue(Map.GetKeyPtr(I)));
				if (Actor==TypedKey.ToActor()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_FindItem_Component(void* PropertyPtr, void* MapAddr, FMonoComponent TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=0; I<Map.Num(); ++I) {
				UActorComponent* Component = Cast<UActorComponent>(Inner->GetPropertyValue(Map.GetKeyPtr(I)));
				if (Component==TypedKey.ToComponent()) {return true;}
			}///
		}///
	}///
	//
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ICALL::API::TMap_RemoveItem_Byte(void* PropertyPtr, void* MapAddr, uint8 TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey) {
					Map.RemoveAt(I); Map.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Int(void* PropertyPtr, void* MapAddr, int32 TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey) {
					Map.RemoveAt(I); Map.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Int64(void* PropertyPtr, void* MapAddr, int64 TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey) {
					Map.RemoveAt(I); Map.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Float(void* PropertyPtr, void* MapAddr, float TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FByteProperty*Inner=CastField<FByteProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey) {
					Map.RemoveAt(I); Map.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_String(void* PropertyPtr, void* MapAddr, FMonoString TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FStrProperty*Inner=CastField<FStrProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey.ToString()) {
					Map.RemoveAt(I); Map.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Name(void* PropertyPtr, void* MapAddr, FMonoName TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FNameProperty*Inner=CastField<FNameProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey.ToName()) {
					Map.RemoveAt(I); Map.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Color(void* PropertyPtr, void* MapAddr, FMonoColor TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->KeyProp)) {
			if (Inner->Struct==TBaseStructure<FColor>::Get()) {
				FScriptMapHelper Map(Property,MapAddr);
				//
				for (int32 I=Map.Num(); I>=0; --I) {
					const uint8* ValuePtr = Map.GetKeyPtr(I);
					//
					FColor Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedKey.ToColor()) {
						Map.RemoveAt(I); Map.Rehash();
						return true;
					}///
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Vector2D(void* PropertyPtr, void* MapAddr, FMonoVector2D TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->KeyProp)) {
			if (Inner->Struct==TBaseStructure<FVector2D>::Get()) {
				FScriptMapHelper Map(Property,MapAddr);
				//
				for (int32 I=Map.Num(); I>=0; --I) {
					const uint8* ValuePtr = Map.GetKeyPtr(I);
					//
					FVector2D Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedKey.ToVector2D()) {
						Map.RemoveAt(I); Map.Rehash();
						return true;
					}///
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Vector3D(void* PropertyPtr, void* MapAddr, FMonoVector3D TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FStructProperty*Inner=CastField<FStructProperty>(Property->KeyProp)) {
			if (Inner->Struct==TBaseStructure<FVector>::Get()) {
				FScriptMapHelper Map(Property,MapAddr);
				//
				for (int32 I=Map.Num(); I>=0; --I) {
					const uint8* ValuePtr = Map.GetKeyPtr(I);
					//
					FVector Value;
					Inner->CopySingleValue(&Value,ValuePtr);
					//
					if (Value==TypedKey.ToVector3D()) {
						Map.RemoveAt(I); Map.Rehash();
						return true;
					}///
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Class(void* PropertyPtr, void* MapAddr, FMonoClass TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FClassProperty*Inner=CastField<FClassProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				UClass* Class = Cast<UClass>(Inner->GetPropertyValue(Map.GetKeyPtr(I)));
				if (Class==TypedKey.ToClass()) {Map.RemoveAt(I); Map.Rehash(); return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Object(void* PropertyPtr, void* MapAddr, FMonoObject TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				if (Inner->GetPropertyValue(Map.GetKeyPtr(I))==TypedKey.ToObject()) {
					Map.RemoveAt(I); Map.Rehash(); return true;
				}///
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Actor(void* PropertyPtr, void* MapAddr, FMonoActor TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				AActor* Actor = Cast<AActor>(Inner->GetPropertyValue(Map.GetKeyPtr(I)));
				if (Actor==TypedKey.ToActor()) {Map.RemoveAt(I); Map.Rehash(); return true;}
			}///
		}///
	}///
	//
	return false;
}

bool ICALL::API::TMap_RemoveItem_Component(void* PropertyPtr, void* MapAddr, FMonoComponent TypedKey) {
	FMapProperty* Property = reinterpret_cast<FMapProperty*>(PropertyPtr);
	//
	if (Property && MapAddr) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->KeyProp)) {
			FScriptMapHelper Map(Property,MapAddr);
			//
			for (int32 I=Map.Num()-1; I>=0; --I) {
				UActorComponent* Component = Cast<UActorComponent>(Inner->GetPropertyValue(Map.GetKeyPtr(I)));
				if (Component==TypedKey.ToComponent()) {Map.RemoveAt(I); Map.Rehash(); return true;}
			}///
		}///
	}///
	//
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////