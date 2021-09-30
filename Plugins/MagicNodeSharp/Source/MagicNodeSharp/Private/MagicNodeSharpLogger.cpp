//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MagicNodeSharpLogger.h"

#include "Runtime/Core/Public/Async/TaskGraphInterfaces.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(NodeCS);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LOG::CS(const TCHAR* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<const TCHAR*>(&LOG::CS,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	UE_LOG(NodeCS,Log,TEXT("%s"),Message);
}

void LOG::CS(const FName Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<const FName>(&LOG::CS,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	UE_LOG(NodeCS,Log,TEXT("%s"),*Message.ToString());
}

void LOG::CS(const FString Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<const FString>(&LOG::CS,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	UE_LOG(NodeCS,Log,TEXT("%s"),*Message);
}

void LOG::CS_CHAR(ESeverity Severity, const TCHAR* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<ESeverity,const TCHAR*>(&LOG::CS_CHAR,Severity,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	switch (Severity) {
		case ESeverity::Info:
			UE_LOG(NodeCS,Log,TEXT("%s"),Message); break;
		case ESeverity::Warning:
			UE_LOG(NodeCS,Warning,TEXT("%s"),Message); break;
		case ESeverity::Error:
			UE_LOG(NodeCS,Error,TEXT("%s"),Message); break;
		case ESeverity::Fatal:
			UE_LOG(NodeCS,Fatal,TEXT("%s"),Message); break;
	default: break;}
}

void LOG::CS_NAME(ESeverity Severity, const FName Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<ESeverity,const FName>(&LOG::CS_NAME,Severity,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	switch (Severity) {
		case ESeverity::Info:
			UE_LOG(NodeCS,Log,TEXT("%s"),*Message.ToString()); break;
		case ESeverity::Warning:
			UE_LOG(NodeCS,Warning,TEXT("%s"),*Message.ToString()); break;
		case ESeverity::Error:
			UE_LOG(NodeCS,Error,TEXT("%s"),*Message.ToString()); break;
		case ESeverity::Fatal:
			UE_LOG(NodeCS,Fatal,TEXT("%s"),*Message.ToString()); break;
	default: break;}
}

void LOG::CS_STR(ESeverity Severity, const FString Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<ESeverity,const FString>(&LOG::CS_STR,Severity,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	switch (Severity) {
		case ESeverity::Info:
			UE_LOG(NodeCS,Log,TEXT("%s"),*Message); break;
		case ESeverity::Warning:
			UE_LOG(NodeCS,Warning,TEXT("%s"),*Message); break;
		case ESeverity::Error:
			UE_LOG(NodeCS,Error,TEXT("%s"),*Message); break;
		case ESeverity::Fatal:
			UE_LOG(NodeCS,Fatal,TEXT("%s"),*Message); break;
	default: break;}
}

void LOG::CS_PRINT_CHAR(const float Duration, const FColor Color, const TCHAR* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<float,const FColor,const TCHAR*>(&LOG::CS_PRINT_CHAR,Duration,Color,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	if (GEngine) {GEngine->AddOnScreenDebugMessage(-1,Duration,Color,FString::Printf(TEXT("{C#}:: %s"),Message));}
}

void LOG::CS_PRINT_NAME(const float Duration, const FColor Color, const FName Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<float,const FColor,const FName>(&LOG::CS_PRINT_NAME,Duration,Color,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	if (GEngine) {GEngine->AddOnScreenDebugMessage(-1,Duration,Color,FString::Printf(TEXT("{C#}:: %s"),*Message.ToString()));}
}

void LOG::CS_PRINT_STRING(const float Duration, const FColor Color, const FString Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<float,const FColor,const FString>(&LOG::CS_PRINT_STRING,Duration,Color,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	if (GEngine) {GEngine->AddOnScreenDebugMessage(-1,Duration,Color,FString::Printf(TEXT("{C#}:: %s"),*Message));}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LOG::CSX_CHAR(ESeverity Severity, const TCHAR* Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<ESeverity,const TCHAR*>(&LOG::CSX_CHAR,Severity,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
#if WITH_EDITOR
	if (GEngine==nullptr) {return;}
	//
	FFormatNamedArguments ARG;
	FFormatArgumentValue AMessage = FText::FromString(Message);
	//
	ARG.Add(TEXT("Message"),AMessage);
	const auto Token = FTextToken::Create(FText::Format(LOCTEXT("LOG::CSX_CHAR","{Message}"),ARG));
	//
	switch (Severity) {
		case ESeverity::Info:
			GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Cyan,FString::Printf(TEXT("{C#}: %s"),Message)); break;
		case ESeverity::Warning:
			GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Orange,FString::Printf(TEXT("{C#}: %s"),Message)); break;
		case ESeverity::Error:
			GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Red,FString::Printf(TEXT("{C#}: %s"),Message)); break;
	default: break;}
	//
	switch (Severity) {
		case ESeverity::Info:
			FMessageLog("PIE").Message(EMessageSeverity::Info,LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Warning:
			FMessageLog("PIE").Warning(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Error:
			FMessageLog("PIE").Error(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Fatal:
			FMessageLog("PIE").CriticalError(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
	default: break;}
#else
	LOG::CS_CHAR(Severity,Message);
#endif
}

void LOG::CSX_NAME(ESeverity Severity, const FName Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<ESeverity,const FName>(&LOG::CSX_NAME,Severity,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
#if WITH_EDITOR
	if (GEngine==nullptr) {return;}
	//
	FFormatNamedArguments ARG;
	FFormatArgumentValue AMessage = FText::FromString(Message.ToString());
	//
	ARG.Add(TEXT("Message"),AMessage);
	const auto Token = FTextToken::Create(FText::Format(LOCTEXT("LOG::CSX_NAME","{Message}"),ARG));
	//
	switch (Severity) {
		case ESeverity::Info:
			GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Cyan,FString::Printf(TEXT("{C#}: %s"),*Message.ToString())); break;
		case ESeverity::Warning:
			GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Orange,FString::Printf(TEXT("{C#}: %s"),*Message.ToString())); break;
		case ESeverity::Error:
			GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Red,FString::Printf(TEXT("{C#}: %s"),*Message.ToString())); break;
	default: break;}
	//
	switch (Severity) {
		case ESeverity::Info:
			FMessageLog("PIE").Message(EMessageSeverity::Info,LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Warning:
			FMessageLog("PIE").Warning(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Error:
			FMessageLog("PIE").Error(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Fatal:
			FMessageLog("PIE").CriticalError(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
	default: break;}
#else
	LOG::CS_NAME(Severity,Message);
#endif
}

void LOG::CSX_STR(ESeverity Severity, const FString Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<ESeverity,const FString>(&LOG::CSX_STR,Severity,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
#if WITH_EDITOR
	if (GEngine==nullptr) {return;}
	//
	FFormatNamedArguments ARG;
	FFormatArgumentValue AMessage = FText::FromString(Message);
	//
	ARG.Add(TEXT("Message"),AMessage);
	const auto Token = FTextToken::Create(FText::Format(LOCTEXT("LOG::CSX_STR","{Message}"),ARG));
	//
	switch (Severity) {
		case ESeverity::Info:
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Cyan,FString::Printf(TEXT("{C#}: %s"),*Message)); break;
		case ESeverity::Warning:
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Orange,FString::Printf(TEXT("{C#}: %s"),*Message)); break;
		case ESeverity::Error:
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Red,FString::Printf(TEXT("{C#}: %s"),*Message)); break;
	default: break;}
	//
	switch (Severity) {
		case ESeverity::Info:
			FMessageLog("PIE").Message(EMessageSeverity::Info,LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Warning:
			FMessageLog("PIE").Warning(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Error:
			FMessageLog("PIE").Error(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Fatal:
			FMessageLog("PIE").CriticalError(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
	default: break;}
#else
	LOG::CS_STR(Severity,Message);
#endif
}

void LOG::CSX_OBJ(ESeverity Severity, const UObject* Owner, const FString Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<ESeverity,const UObject*,const FString>(&LOG::CSX_OBJ,Severity,Owner,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	if (!Owner) {return;}
	//
#if WITH_EDITOR
	if (GEngine==nullptr) {return;}
	//
	FFormatNamedArguments ARG;
	FFormatArgumentValue AMessage = FText::FromString(Message);
	FFormatArgumentValue APackage = FText::FromString(Owner->GetName());
	//
	ARG.Add(TEXT("Message"),AMessage);
	ARG.Add(TEXT("Package"),APackage);
	const auto Token = FTextToken::Create(FText::Format(LOCTEXT("LOG::CSX_OBJ","[{Package}]:\n\t{Message}"),ARG));
	//
	switch (Severity) {
		case ESeverity::Info:
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Cyan,FString::Printf(TEXT("{C#}:  %s  ::  %s"),*Owner->GetName(),*Message)); break;
		case ESeverity::Warning:
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Orange,FString::Printf(TEXT("{C#}:  %s  ::  %s"),*Owner->GetName(),*Message)); break;
		case ESeverity::Error:
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Red,FString::Printf(TEXT("{C#}:  %s  ::  %s"),*Owner->GetName(),*Message)); break;
	default: break;}
	//
	switch (Severity) {
		case ESeverity::Info:
			FMessageLog("PIE").Message(EMessageSeverity::Info,LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Warning:
			FMessageLog("PIE").Warning(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Error:
			FMessageLog("PIE").Error(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Fatal:
			FMessageLog("PIE").CriticalError(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
	default: break;}
#endif
}

void LOG::CSX_FUN(ESeverity Severity, const UObject* Owner, const UFunction* Function, const FString Message) {
#if UE_BUILD_SHIPPING
	return;
#endif
	//
	if (!IsInGameThread()) {
		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateStatic<ESeverity,const UObject*, const UFunction*,const FString>(&LOG::CSX_FUN,Severity,Owner,Function,Message),
			GET_STATID(STAT_FMonoMethod_Log), nullptr,
			ENamedThreads::GameThread
		);//
	return;}
	//
	if (!Function) {return;}
	if (!Owner) {return;}
	//
#if WITH_EDITOR
	if (GEngine==nullptr) {return;}
	//
	FFormatNamedArguments ARG;
	FFormatArgumentValue AMessage = FText::FromString(Message);
	FFormatArgumentValue AFunction = Function->GetDisplayNameText();
	FFormatArgumentValue AInfo = FText::FromString(Function->GetName());
	FFormatArgumentValue APackage = FText::FromString(Owner->GetName());
	//
	ARG.Add(TEXT("Details"),AInfo);
	ARG.Add(TEXT("Message"),AMessage);
	ARG.Add(TEXT("Package"),APackage);
	ARG.Add(TEXT("Function"),AFunction);
	const auto Token = FTextToken::Create(FText::Format(LOCTEXT("LOG::CSX_FUN","{Function}: [{Details} at ({Package})]:\n\t{Message}"),ARG));
	//
	switch (Severity) {
		case ESeverity::Info:
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Cyan,FString::Printf(TEXT("{C#}:  %s ==> %s  ::  %s"),*Owner->GetName(),*Function->GetName(),*Message)); break;
		case ESeverity::Warning:
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Orange,FString::Printf(TEXT("{C#}:  %s ==> %s  ::  %s"),*Owner->GetName(),*Function->GetName(),*Message)); break;
		case ESeverity::Error:
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Red,FString::Printf(TEXT("{C#}:  %s ==> %s  ::  %s"),*Owner->GetName(),*Function->GetName(),*Message)); break;
	default: break;}
	//
	switch (Severity) {
		case ESeverity::Info:
			FMessageLog("PIE").Message(EMessageSeverity::Info,LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Warning:
			FMessageLog("PIE").Warning(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Error:
			FMessageLog("PIE").Error(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
		case ESeverity::Fatal:
			FMessageLog("PIE").CriticalError(LOCTEXT("LOG::CS","{C#}: "))->AddToken(Token); break;
	default: break;}
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////