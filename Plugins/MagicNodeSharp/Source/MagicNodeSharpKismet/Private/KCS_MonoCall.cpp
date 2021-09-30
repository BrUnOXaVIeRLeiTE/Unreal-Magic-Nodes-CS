//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KCS_MonoCall.h"
#include "KCS_MonoCore.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ICALL::KAPI::CS_CompilerInfo(MonoString* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	WIDECHAR* CH = mono_string_to_utf16(Message);
	FString Str = StringCast<TCHAR>(CH).Get();
	//
	LOG::CSX_STR(ESeverity::Info,Str);
	//
	mono_free(CH);
}

void ICALL::KAPI::CS_CompilerWarning(MonoString* Node, int32 Line, int32 Column, MonoString* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	WIDECHAR* ND = mono_string_to_utf16(Node);
	WIDECHAR* CH = mono_string_to_utf16(Message);
	//
	FString Str = StringCast<TCHAR>(CH).Get();
	FString Name = StringCast<TCHAR>(ND).Get();
	FString Final = FString::Printf(TEXT("[%i , %i]   %s  ::  %s"),Line+1,Column+1,*Name,*Str);
	//
	FCompilerResults Results;
	Results.ErrorMessage = Final;
	Results.ErrorInfo = FSourceInfo(Line+1,Column+1);
	Results.Result = EMonoCompilerResult::Warning;
	//
	IMonoKismet::RoslynWarningStack.Add(*Name,Results);
	LOG::CSX_STR(ESeverity::Warning,FString::Printf(TEXT("%s  ::  %s"),*Name,*Str));
	//
	mono_free(ND);
	mono_free(CH);
}

void ICALL::KAPI::CS_CompilerError(MonoString* Node, int32 Line, int32 Column, MonoString* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	WIDECHAR* ND = mono_string_to_utf16(Node);
	WIDECHAR* CH = mono_string_to_utf16(Message);
	//
	FString Str = StringCast<TCHAR>(CH).Get();
	FString Name = StringCast<TCHAR>(ND).Get();
	FString Final = FString::Printf(TEXT("[%i , %i]   %s  ::  %s"),Line+1,Column+1,*Name,*Str);
	//
	FCompilerResults Results;
	Results.ErrorMessage = Final;
	Results.Result = EMonoCompilerResult::Error;
	Results.ErrorInfo = FSourceInfo(Line+1,Column+1);
	//
	IMonoKismet::RoslynErrorStack.Add(*Name,Results);
	LOG::CSX_STR(ESeverity::Error,FString::Printf(TEXT("%s  ::  %s"),*Name,*Str));
	//
	mono_free(ND);
	mono_free(CH);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////