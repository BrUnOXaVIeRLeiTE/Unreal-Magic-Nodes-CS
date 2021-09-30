//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MCS_API.h"
#include "mono/metadata/debug-helpers.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CS_KISMET_DOMAIN "CsKismetDomain"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DELEGATE_TwoParams(FOnScriptSourceCompiled,const UMagicNodeSharpSource*,const FCompilerResults);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FCS_Toolkit;
class UKCS_MagicNode;
class IMagicNodeSharpKismet;
class FMagicNodeSharpKismet;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MAGICNODESHARPKISMET_API IMonoKismet {
	friend class UKCS_MagicNode;
	friend class FCS_Toolkit;
private:
	FOnScriptSourceCompiled CompilerResult;
	EMonoKismetState KismetState;
private:
	MonoImage* CoreImage = nullptr;
	MonoImage* CompilerImage = nullptr;
private:
	MonoDomain* KismetDomain = nullptr;
private:
	MonoClass* CodeEditorUtility = nullptr;
private:
	MonoMethod* CodeFormatMethod = nullptr;
private:
	void MonoKismetDomain_INIT();
	void MonoKismetDomain_STOP();
protected:
	void MonoKismet_INIT_BASE();
protected:
	bool MonoKismet_INIT();
	bool MonoKismet_STOP();
protected:
	void SetupThreadingAttribute(UMagicNodeSharp* Node, MonoClass* NodeClass);
protected:
	void CompileNode(UMagicNodeSharpSource* Source, UMagicNodeSharp* Node);
	void CompilationFinished(UMagicNodeSharpSource* Source, const FCompilerResults &Results);
protected:
	void GetDataTypeFromManagedType(MonoType* ManagedType, FMonoFieldDefinition &OutDefinition);
	void GetDataTypeFromManagedType(MonoType* ManagedType, FMonoPropertyInfo &OutDefinition);
protected:
	FMonoClassDefinition CollectClassInfo(UMagicNodeSharpSource* const &Script, MonoObject* ManagedObject);
	TArray<FMonoPropertyInfo>CollectClassPropertyInfo(MonoObject* ManagedObject);
public:
	MonoDomain* GetKismetDomain() const;
public:
	MonoImage* GetCoreImage() const;
	MonoImage* GetCompilerImage() const;
public:
	MonoMethod* GetCodeFormatMethod() const;
public:
	MonoClass* mono_find_class(MonoImage* FromImage, const FString &Namespace, const FString &ClassName);
	MonoMethod* mono_find_method(MonoClass* ManagedClass, const FString &MethodName);
public:
	MonoObject* mono_create_object(MonoDomain* InDomain, MonoClass* ManagedClass);
public:
	bool IsPlaying() const;
	bool CanCompile() const;
	bool IsCompiling() const;
	bool CanFormatCode() const;
public:
	static TMap<FName,FCompilerResults>RoslynErrorStack;
	static TMap<FName,FCompilerResults>RoslynWarningStack;
public:
	static void CompileScript(UMagicNodeSharpSource* Source, UMagicNodeSharp* Node);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////