//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MagicNodeSharpKismet_Shared.h"
#include "MCS_Types.h"
#include "MCS_API.h"

#include "Runtime/Engine/Public/EngineUtils.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UMagicNodeSharpSource;

#define MAX_SUGGESTION_ROWS 25
#define MAX_SUGGESTIONS 255
#define MIN_BOX_SIZE 100

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CS {
	const TCHAR* Operators[] = {
		TEXT("."),
		TEXT(","),
		TEXT(":"),
		TEXT(";"),
		//
		TEXT("+"),
		TEXT("-"),
		TEXT("%"),
		TEXT("^"),
		//
		TEXT("&"),
		TEXT("?"),
		TEXT("|"),
		TEXT("!"),
		TEXT("?!"),
		TEXT("&&"),
		TEXT("??"),
		TEXT("||"),
		//
		TEXT("++"),
		TEXT("--"),
		//
		TEXT("=="),
		TEXT("!="),
		TEXT("<"),
		TEXT(">"),
		TEXT("<="),
		TEXT(">="),
		//
		TEXT("="),
		TEXT("+="),
		TEXT("-="),
		TEXT("*="),
		TEXT("/="),
		TEXT("%="),
		TEXT("&="),
		TEXT("|="),
		TEXT("^="),
		TEXT(">>"),
		TEXT("<<"),
		TEXT("=>"),
		TEXT("<<="),
		TEXT(">>="),
		TEXT("??="),
		//
		TEXT("("),
		TEXT(")"),
		TEXT("["),
		TEXT("]"),
		TEXT("{"),
		TEXT("}"),
		TEXT("\'"),
		TEXT("\""),
		TEXT("\\"),
		//
		TEXT("/*"),
		TEXT("*/"),
		TEXT("//")
	};///
	//
	const TCHAR* Keywords[] = {
		TEXT("void"),
		TEXT("null"),
		TEXT("notnull"),
		//
		TEXT("true"),
		TEXT("false"),
		//
		TEXT("namespace"),
		TEXT("struct"),
		TEXT("class"),
		TEXT("event"),
		//
		TEXT("internal"),
		TEXT("protected"),
		TEXT("readonly"),
		TEXT("private"),
		TEXT("public"),
		TEXT("static"),
		TEXT("const"),
		//
		TEXT("default"),
		TEXT("delegate"),
		TEXT("unchecked"),
		TEXT("stackalloc"),
		TEXT("checked"),
		TEXT("nameof"),
		TEXT("sizeof"),
		TEXT("typeof"),
		TEXT("new"),
		TEXT("as"),
		//
		TEXT("abstract"),
		TEXT("const"),
		TEXT("extern"),
		//
		TEXT("base"),
		TEXT("break"),
		TEXT("case"),
		TEXT("catch"),
		TEXT("checked"),
		TEXT("continue"),
		TEXT("do"),
		TEXT("else"),
		TEXT("enum"),
		TEXT("event"),
		TEXT("explicit"),
		TEXT("finally"),
		TEXT("fixed"),
		TEXT("for"),
		TEXT("foreach"),
		TEXT("goto"),
		TEXT("if"),
		TEXT("implicit"),
		TEXT("in"),
		TEXT("interface"),
		TEXT("internal"),
		TEXT("is"),
		TEXT("lock"),
		TEXT("operator"),
		TEXT("out"),
		TEXT("override"),
		TEXT("params"),
		TEXT("ref"),
		TEXT("sealed"),
		TEXT("switch"),
		TEXT("this"),
		TEXT("throw"),
		TEXT("try"),
		TEXT("unchecked"),
		TEXT("unsafe"),
		TEXT("virtual"),
		TEXT("volatile"),
		TEXT("while"),
		//
		TEXT("add"),
		TEXT("alias"),
		TEXT("ascending"),
		TEXT("async"),
		TEXT("await"),
		TEXT("by"),
		TEXT("descending"),
		TEXT("dynamic"),
		TEXT("equals"),
		TEXT("from"),
		TEXT("get"),
		TEXT("global"),
		TEXT("group"),
		TEXT("into"),
		TEXT("join"),
		TEXT("let"),
		TEXT("object"),
		TEXT("on"),
		TEXT("orderby"),
		TEXT("partial"),
		TEXT("remove"),
		TEXT("select"),
		TEXT("set"),
		TEXT("unmanaged"),
		TEXT("using"),
		TEXT("value"),
		TEXT("var"),
		TEXT("when"),
		TEXT("where"),
		TEXT("with"),
		TEXT("yield"),
		//
		TEXT("if"),
		TEXT("else"),
		//
		TEXT("return")
	};///
	//
	const TCHAR* Types[] = {
		TEXT("char"),
		TEXT("int"),
		TEXT("uint"),
		TEXT("bool"),
		TEXT("byte"),
		TEXT("sbyte"),
		TEXT("long"),
		TEXT("ulong"),
		TEXT("short"),
		TEXT("ushort"),
		TEXT("float"),
		TEXT("double"),
		TEXT("decimal"),
		TEXT("string"),
		//
		TEXT("String"),
		TEXT("Strand"),
		TEXT("Name"),
		TEXT("Text"),
		TEXT("Vector2D"),
		TEXT("Vector3D"),
		TEXT("Rotator"),
		TEXT("Color"),
		TEXT("Transform"),
		//
		TEXT("ClassPtr"),
		TEXT("ObjectPtr"),
		TEXT("ActorPtr"),
		TEXT("ComponentPtr"),
		//
		TEXT("IntPtr")
	};///
	//
	const TCHAR* Containers[] = {
		TEXT("IArray"),
		TEXT("ISet"),
		TEXT("IMap")
	};///
	//
	const TCHAR* Macros[] = {
		TEXT("#")
	};///
	//
	const TCHAR GUIDs[] = {
		TEXT('1'),
		TEXT('2'),
		TEXT('3'),
		TEXT('4'),
		TEXT('5'),
		TEXT('6'),
		TEXT('7'),
		TEXT('8'),
		TEXT('9'),
		TEXT('A'),
		TEXT('B'),
		TEXT('C'),
		TEXT('D'),
		TEXT('E'),
		TEXT('F')
	};///
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static class MAGICNODESHARPKISMET_API IKCS_MonoAnalyzer {
private:
	static const FMonoNamespaceDefinition VOID_Namespace;
	static const FMonoMethodDefinition VOID_Method;
	static const FMonoClassDefinition VOID_Class;
	static const FMonoFieldDefinition VOID_Field;
	static const FMonoFieldDefinition VOID_Prop;
private:
	static  FMonoClassDefinition ContextClass;
	static  FMonoClassDefinition ScopedClass;
public:
	static TArray<FMonoNamespaceDefinition>SpaceInfoCache;
	static TMap<FString,FMonoClassDefinition>TypeInfoCache;
	static TMap<FString,FMonoClassDefinition>SuggestedClassCache;
	static TMap<FString,FMonoFieldDefinition>SuggestedFieldCache;
	static TMap<FString,FMonoMethodDefinition>SuggestedMethodCache;
	static TMap<FString,FMonoFieldDefinition>SuggestedPropertyCache;
public:
	static const FMonoClassDefinition &GetVoidClass();
	static const FMonoFieldDefinition &GetVoidField();
	static const FMonoMethodDefinition &GetVoidMethod();
	static const FMonoFieldDefinition &GetVoidProperty();
	static const FMonoNamespaceDefinition &GetVoidNamespace();
public:
	static void CacheBaseClasses();
	static void CacheNamespaces(MonoImage* Image);
	static void CacheClassesFromImage(MonoImage* Image);
public:
	static bool IsSystemClass(MonoClass* ManagedClass);
public:
	static MonoClass* GetClassFromType(MonoType* Type);
	static MonoClass* GetClassFromAlias(const FString &AliasName);
public:
	static MonoClass* FindClassByName(MonoImage* Image, const FString &ClassName);
	static MonoClass* FindClassByName(MonoImage* Image, const FString &Space, const FString &ClassName);
public:
	static MonoType* GetFieldType(MonoClassField* Field);
	static MonoType* GetPropertyType(MonoProperty* Property);
	static MonoType* GetMethodReturnType(MonoClass* ParentClass, MonoMethod* Method);
public:
	static const FMonoClassDefinition GetClassInfo(MonoClass* ManagedClass);
	static const FMonoFieldDefinition GetPropertyInfo(MonoProperty* Property);
	static const FMonoFieldDefinition GetFieldInfo(MonoClassField* ClassField);
	static const FMonoClassDefinition GetClassMethodsInfo(MonoClass* ManagedClass);
	static const FMonoMethodDefinition GetMethodInfo(MonoClass* ParentClass, MonoMethod* Method);
	static const TArray<FMonoParameterType>GetMethodParameterInfo(MonoClass* ParentClass, MonoMethod* Method);
public:
	static const FMonoFieldDefinition &GetFieldInfo(UMagicNodeSharpSource* const &Script, const FString &TypeName);
	static const FMonoClassDefinition &GetClassInfo(UMagicNodeSharpSource* const &Script, const FString &ClassName);
	static const FMonoFieldDefinition &GetPropertyInfo(UMagicNodeSharpSource* const &Script, const FString &TypeName);
	static const FMonoMethodDefinition &GetMethodInfo(UMagicNodeSharpSource* const &Script, const FString &MethodName);
	static const FMonoNamespaceDefinition &GetNamespaceInfo(UMagicNodeSharpSource* const &Script, const FString &SpaceName);
public:
	static const FMonoClassDefinition &GetClassInfo(UMagicNodeSharpSource* const &Script, const FString &Context, const FString &Member);
	static const FMonoFieldDefinition &GetFieldInfo(UMagicNodeSharpSource* const &Script, const FString &Context, const FString &TypeName);
	static const FMonoFieldDefinition &GetPropertyInfo(UMagicNodeSharpSource* const &Script, const FString &Context, const FString &TypeName);
	static const FMonoMethodDefinition &GetMethodInfo(UMagicNodeSharpSource* const &Script, const FString &Context, const FString &MethodName);
public:
	static const FMonoClassDefinition &GetClassInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member);
	static const FMonoFieldDefinition &GetFieldInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member);
	static const FMonoMethodDefinition &GetMethodInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member);
	static const FMonoFieldDefinition &GetPropertyInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member);
	static const FMonoNamespaceDefinition &GetNamespaceInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member);
public:
	static void SearchClassInfo(const FMonoClassDefinition &ClassInfo, const FString &TypeName, TArray<FString>&Results);
	static void SearchFieldInfo(const FMonoFieldDefinition &FieldInfo, const FString &TypeName, TArray<FString>&Results);
	static void SearchMethodInfo(const FMonoMethodDefinition &MethodInfo, const FString &TypeName, TArray<FString>&Results);
	static void SearchPropertyInfo(const FMonoFieldDefinition &PropertyInfo, const FString &TypeName, TArray<FString>&Results);
	static void SearchNamespaceInfo(const FMonoNamespaceDefinition &NamespaceInfo, const FString &ClassName, TArray<FString>&Results);
public:
	static void SearchClassInfo(const FMonoClassDefinition &ClassInfo, const TArray<FString>&Tree, TArray<FString>&Results);
	static void SearchNamespaceInfo(const FMonoNamespaceDefinition &NamespaceInfo, const TArray<FString>&Tree, TArray<FString>&Results);
public:
	static void SearchForClassType(MonoImage* Image, const FString &TypeName, TArray<FString>&Results);
public:
	static void SearchForClassType(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results);
	static void SearchForFieldType(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results);
	static void SearchForMethodType(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results);
	static void SearchForPropertyType(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results);
public:
	static void SearchForFieldMembers(MonoImage* Image, const FString &TypeName, TArray<FString>&Results);
	static void SearchForMethodMembers(MonoImage* Image, const FString &TypeName, TArray<FString>&Results);
	static void SearchForPropertyMembers(MonoImage* Image, const FString &TypeName, TArray<FString>&Results);
public:
	static void SearchForFieldMembers(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results);
	static void SearchForMethodMembers(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results);
	static void SearchForPropertyMembers(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results);
public:
	static bool SearchForSpaceMembers(MonoImage* Image, const FString &SpaceName, TArray<FString>&Results);
	static bool SearchForClassMembers(MonoImage* Image, const FString &ClassName, TArray<FString>&Results);
	static bool SearchForAliasMembers(const FString &AliasName, TArray<FString>&Results);
public:
	static void CollectAllClassMembers(MonoClass* ManagedClass, TArray<FString>&Results);
	static void CollectClassMethods(MonoClass* ManagedClass, TArray<FString>&Results);
public:
	static void GetDataTypeFromMonoType(MonoType* ManagedType, FMonoFieldDefinition &OutDefinition);
public:
	static void AutoSuggest(UMagicNodeSharpSource* const &Script, const FString &Keyword, TArray<FString>&Results);
	static void AutoSuggest(const FString &Keyword, UMagicNodeSharpSource* const &Script, TArray<FString>&Results);
	static void AutoSuggest(const TArray<FString>&Tree, UMagicNodeSharpSource* const &Script, TArray<FString>&Results);
	static void AutoSuggestMethod(const FString &Keyword, UMagicNodeSharpSource* const &Script, TArray<FString>&Results);
	static void AutoSuggestMethod(const TArray<FString>&Tree, UMagicNodeSharpSource* const &Script, TArray<FString>&Results);
public:
	static FString GenerateGUID(const FString &Base);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////