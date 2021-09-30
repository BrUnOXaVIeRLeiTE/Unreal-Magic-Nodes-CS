//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KCS_MonoAnalyzer.h"
#include "KCS_MonoCore.h"
#include "MCS_File.h"

#include "MagicNodeSharp.h"
#include "IMagicNodeSharp.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FMonoNamespaceDefinition IKCS_MonoAnalyzer::VOID_Namespace{};
const FMonoMethodDefinition IKCS_MonoAnalyzer::VOID_Method{};
const FMonoClassDefinition IKCS_MonoAnalyzer::VOID_Class{};
const FMonoFieldDefinition IKCS_MonoAnalyzer::VOID_Field{};
const FMonoFieldDefinition IKCS_MonoAnalyzer::VOID_Prop{};

TArray<FMonoNamespaceDefinition>IKCS_MonoAnalyzer::SpaceInfoCache{};
TMap<FString,FMonoClassDefinition>IKCS_MonoAnalyzer::TypeInfoCache{};
TMap<FString,FMonoClassDefinition>IKCS_MonoAnalyzer::SuggestedClassCache{};
TMap<FString,FMonoFieldDefinition>IKCS_MonoAnalyzer::SuggestedFieldCache{};
TMap<FString,FMonoMethodDefinition>IKCS_MonoAnalyzer::SuggestedMethodCache{};
TMap<FString,FMonoFieldDefinition>IKCS_MonoAnalyzer::SuggestedPropertyCache{};

FMonoClassDefinition IKCS_MonoAnalyzer::ContextClass{};
FMonoClassDefinition IKCS_MonoAnalyzer::ScopedClass{};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::AutoSuggest(UMagicNodeSharpSource* const &Script, const FString &Keyword, TArray<FString>&Results) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	FCriticalSection Mutex;
	{
		Mutex.Lock();
		//
		if (Results.Num()<MAX_SUGGESTIONS) {
			static const TCHAR* Whitespace[] = {
				TEXT("\t "),TEXT("\t"),TEXT(" "),
				TEXT("("),TEXT(")"),TEXT("&"),
				TEXT("["),TEXT("]"),TEXT("*"),
				TEXT("{"),TEXT("}"),TEXT("?"),
				TEXT("<"),TEXT(">"),TEXT(":"),
				TEXT("."),TEXT(","),TEXT(";"),
				TEXT("-"),TEXT("+"),TEXT("="),
				TEXT("!"),TEXT("^"),TEXT("#"),
				TEXT("%"),TEXT("/"),TEXT("|"),
				TEXT("\'"),TEXT("\""),TEXT("\\")
			};//
			//
			TArray<FString>Content{};
			int32 Nums = UE_ARRAY_COUNT(Whitespace);	
			Script->GetSource().ParseIntoArray(Content,Whitespace,Nums,true);
			//
			for (int32 I=0; I<Content.Num(); ++I) {
				if (Content[I].Contains(Keyword,ESearchCase::CaseSensitive)) {
					Results.AddUnique(Content[I]);
				}///
			}///
		}///
		//
		if (Results.Num()<MAX_SUGGESTIONS) {
			for (int32 I=0; I<101; ++I) {
				const FString &Item = CS::Keywords[I];
				if (Item.Contains(Keyword,ESearchCase::CaseSensitive)) {Results.AddUnique(Item);}
			}///
			//
			for (int32 I=0; I<28; ++I) {
				const FString &Item = CS::Types[I];
				if (Item.Contains(Keyword,ESearchCase::CaseSensitive)) {Results.AddUnique(Item);}
			}///
			//
			for (int32 I=0; I<3; ++I) {
				const FString &Item = CS::Containers[I];
				if (Item.Contains(Keyword,ESearchCase::CaseSensitive)) {Results.AddUnique(Item);}
			}///
		}///
		//
		if (Results.Num()<MAX_SUGGESTIONS) {
			SearchForClassType(Script,Keyword,Results);
			SearchForFieldType(Script,Keyword,Results);
			SearchForPropertyType(Script,Keyword,Results);
			SearchForMethodType(Script,Keyword,Results);
		}///
		//
		if (Results.Num()<MAX_SUGGESTIONS) {
			SearchForClassType(MonoCore.GetEditorCoreImage(),Keyword,Results);
			SearchForClassType(MonoCore.GetEditorUnrealImage(),Keyword,Results);
			SearchForClassType(MonoCore.GetEditorSystemImage(),Keyword,Results);
		}///
		//
		if (Results.Num()<MAX_SUGGESTIONS) {
			if (SearchForSpaceMembers(MonoCore.GetEditorCoreImage(),Keyword,Results)) {goto END;}
			if (SearchForSpaceMembers(MonoCore.GetEditorUnrealImage(),Keyword,Results)) {goto END;}
			if (SearchForSpaceMembers(MonoCore.GetEditorSystemImage(),Keyword,Results)) {goto END;}
			//
			if (SearchForClassMembers(MonoCore.GetEditorCoreImage(),Keyword,Results)) {goto END;}
			if (SearchForClassMembers(MonoCore.GetEditorUnrealImage(),Keyword,Results)) {goto END;}
			if (SearchForClassMembers(MonoCore.GetEditorSystemImage(),Keyword,Results)) {goto END;}
		}///
		//
		END:{
			Results.Sort();
		}///
		//
		Mutex.Unlock();
	}
}

void IKCS_MonoAnalyzer::AutoSuggest(const FString &Keyword, UMagicNodeSharpSource* const &Script, TArray<FString>&Results) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	FCriticalSection Mutex;
	{
		Mutex.Lock();
		//
		if (Keyword==TEXT("global")||Keyword==TEXT("::")) {
			const FMonoNamespaceDefinition &SpaceInfo = GetNamespaceInfo(Script,TEXT("global"));
			SearchNamespaceInfo(SpaceInfo,TEXT("#"),Results); return;
		}///
		//
		if ((Keyword==TEXT("this"))||(Keyword==Script->GetScriptName())) {
			const FMonoClassDefinition &ClassInfo = Script->GetClassDefinition();
			SearchClassInfo(ClassInfo,TEXT("#"),Results); return;
		}///
		//
		if (Results.Num()<MAX_SUGGESTIONS) {
			SearchForFieldMembers(Script,Keyword,Results);
			SearchForPropertyMembers(Script,Keyword,Results);
			SearchForMethodMembers(Script,Keyword,Results);
		}///
		//
		if (Results.Num()<MAX_SUGGESTIONS) {
			if (SearchForSpaceMembers(MonoCore.GetEditorCoreImage(),Keyword,Results)) {goto END;}
			if (SearchForSpaceMembers(MonoCore.GetEditorUnrealImage(),Keyword,Results)) {goto END;}
			if (SearchForSpaceMembers(MonoCore.GetEditorSystemImage(),Keyword,Results)) {goto END;}
			//
			if (SearchForClassMembers(MonoCore.GetEditorCoreImage(),Keyword,Results)) {goto END;}
			if (SearchForClassMembers(MonoCore.GetEditorUnrealImage(),Keyword,Results)) {goto END;}
			if (SearchForClassMembers(MonoCore.GetEditorSystemImage(),Keyword,Results)) {goto END;}
		}///
		//
		END:{
			Results.Sort();
		}///
		//
		Mutex.Unlock();
	}
}

void IKCS_MonoAnalyzer::AutoSuggest(const TArray<FString>&Tree, UMagicNodeSharpSource* const &Script, TArray<FString>&Results) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (Tree.Num()==0) {return;}
	//
	FCriticalSection Mutex;
	{
		Mutex.Lock();
		//
		if (Results.Num()<MAX_SUGGESTIONS) {
			//
			FString Namespace{};
			FString Parent{};
			FString Scope{};
			//
			for (int32 I=0; I<Tree.Num(); ++I) {
				Namespace.Append(Tree[I]);
				if (I<(Tree.Num()-1)) {
					Namespace.AppendChar(TEXT('.'));
				}///
			}///
			//
			if (Tree[0].Equals(TEXT("this"),ESearchCase::CaseSensitive) ||
			Tree[Tree.Num()-1].Equals(Script->GetScriptName(),ESearchCase::CaseSensitive))
			{///
				const FMonoClassDefinition &ClassInfo = Script->GetClassDefinition();
				SearchClassInfo(ClassInfo,Tree,Results); goto END;
			}///
			//
			if (Tree[0].Equals(TEXT("global"),ESearchCase::CaseSensitive)||Tree[Tree.Num()-1].Equals(TEXT("::"))) {
				const FMonoNamespaceDefinition &SpaceInfo = GetNamespaceInfo(Script,TEXT("global"));
				SearchNamespaceInfo(SpaceInfo,Tree,Results); goto END;
			} else {
				const FMonoNamespaceDefinition &SpaceInfo = GetNamespaceInfo(Script,Namespace);
				if (SpaceInfo.IsValid()) {SearchNamespaceInfo(SpaceInfo,Tree,Results); goto END;}
			}///
			//
			if (Namespace.Split(TEXT("."),&Parent,&Scope,ESearchCase::IgnoreCase,ESearchDir::FromEnd)) {
				if (MonoClass*TypeClass=mono_class_from_name_case(MonoCore.GetEditorCoreImage(),StringCast<ANSICHAR>(*Parent).Get(),StringCast<ANSICHAR>(*Scope).Get())) {CollectAllClassMembers(TypeClass,Results); goto END;}
				if (MonoClass*TypeClass=mono_class_from_name_case(MonoCore.GetEditorUnrealImage(),StringCast<ANSICHAR>(*Parent).Get(),StringCast<ANSICHAR>(*Scope).Get())) {CollectAllClassMembers(TypeClass,Results); goto END;}
				if (MonoClass*TypeClass=mono_class_from_name_case(MonoCore.GetEditorSystemImage(),StringCast<ANSICHAR>(*Parent).Get(),StringCast<ANSICHAR>(*Scope).Get())) {CollectClassMethods(TypeClass,Results); goto END;}
			}///
		}///
		//
		END:{
			Results.Sort();
		}///
		//
		Mutex.Unlock();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::AutoSuggestMethod(const FString &Keyword, UMagicNodeSharpSource* const &Script, TArray<FString>&Results) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	FCriticalSection Mutex;
	{
		Mutex.Lock();
		//
		TArray<FString>Props{};
		TArray<FString>Fields{};
		TArray<FString>Methods{};
		//
		FString Scope = Keyword;
		Scope.RemoveFromEnd(TEXT("@"));
		//
		for (const auto &Method : ScriptInfo.Methods) {
			if (Method.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				if (TypeInfoCache.Contains(Method.TypeToString())) {
					const auto &TypeInfo = TypeInfoCache.FindChecked(Method.TypeToString());
					//
					for (const auto &Prop : TypeInfo.Props) {Props.AddUnique(Prop.Name.ToString());}
					for (const auto &Field : TypeInfo.Fields) {Fields.AddUnique(Field.Name.ToString());}
					for (const auto &Member : TypeInfo.Methods) {Methods.AddUnique(Member.Name.ToString());}
					//
					goto END;
				}///
			}///
		}///
		//
		END:{
			Props.Sort();
			Fields.Sort();
			Methods.Sort();
			//
			Results.Append(Props);
			Results.Append(Fields);
			Results.Append(Methods);
		}///
		//
		Mutex.Unlock();
	}
}

void IKCS_MonoAnalyzer::AutoSuggestMethod(const TArray<FString>&Tree, UMagicNodeSharpSource* const &Script, TArray<FString>&Results) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	FCriticalSection Mutex;
	{
		Mutex.Lock();
		//
		TArray<FString>Root = Tree;
		FString Scope{};
		//
		TArray<FString>Props{};
		TArray<FString>Fields{};
		TArray<FString>Methods{};
		//
		if (Tree.Num()==0) {goto END;}
		//
		Scope = Root.Pop();
		const auto &Method = GetMethodInfo(Script,Root,Scope);
		//
		if (Method.IsValid()) {
			if (TypeInfoCache.Contains(Method.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Method.TypeToString());
				//
				for (const auto &Prop : TypeInfo.Props) {Props.AddUnique(Prop.Name.ToString());}
				for (const auto &Field : TypeInfo.Fields) {Fields.AddUnique(Field.Name.ToString());}
				for (const auto &Member : TypeInfo.Methods) {Methods.AddUnique(Member.Name.ToString());}
				//
				goto END;
			}///
		} else {return;}
		//
		END:{
			Props.Sort();
			Fields.Sort();
			Methods.Sort();
			//
			Results.Append(Props);
			Results.Append(Fields);
			Results.Append(Methods);
		}///
		//
		Mutex.Unlock();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::CollectAllClassMembers(MonoClass* ManagedClass, TArray<FString>&Results) {
	if (IsSystemClass(ManagedClass)) {CollectClassMethods(ManagedClass,Results); return;}
	//
	FCriticalSection Mutex;
	{
		Mutex.Lock();
		//
		MonoImage* Image = mono_class_get_image(ManagedClass);
		const FString ClassName = FString(StringCast<TCHAR>(mono_class_get_name(ManagedClass)).Get());
		//
		while(ManagedClass!=nullptr) {
			void* FItr=0; MonoClassField* ClassField = nullptr;
			while((ClassField=mono_class_get_fields(ManagedClass,&FItr)) != NULL) {
				const char* FieldName = mono_field_get_name(ClassField);
				Results.AddUnique(StringCast<TCHAR>(FieldName).Get());
			}///
			//
			void* PItr=0; MonoProperty* ClassProperty = nullptr;
			while((ClassProperty=mono_class_get_properties(ManagedClass,&PItr)) != NULL) {
				const char* PropertyName = mono_property_get_name(ClassProperty);
				Results.AddUnique(StringCast<TCHAR>(PropertyName).Get());
			}///
			//
			void* MItr=0; MonoMethod* ClassMethod = nullptr;
			while((ClassMethod=mono_class_get_methods(ManagedClass,&MItr)) != NULL) {
				const char* MethodName = mono_method_get_name(ClassMethod);
				Results.AddUnique(StringCast<TCHAR>(MethodName).Get());
			}///
			//
			ManagedClass = mono_class_get_parent(ManagedClass);
			if (ManagedClass==mono_get_object_class()) {break;}
			FItr=0; PItr=0; MItr=0;
		}///
		//
		{
			const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
			int Rows = mono_table_info_get_rows(TbInfo);
			//
			const FString XName = ClassName+TEXT("Extensions");
			//
			for (int X=0; X<Rows; ++X) {
				uint32_t XNMS[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(TbInfo,X,XNMS,MONO_TYPEDEF_SIZE);
				//
				const char* _ExtSpace = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAMESPACE]);
				const char* _ExtName = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAME]);
				//
				const FString ExtSpace = StringCast<TCHAR>(_ExtSpace).Get();
				const FString ExtName = StringCast<TCHAR>(_ExtName).Get();
				//
				if (!ExtName.Equals(XName,ESearchCase::CaseSensitive)) {continue;}
				if (MonoClass*ExtClass=mono_class_from_name_case(Image,_ExtSpace,_ExtName)) {
					if (MonoType*ExtType=mono_class_get_type(ExtClass)) {
						const char* _ExtTypeName = mono_type_get_name_full(ExtType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
						FString ExtTypeName = StringCast<TCHAR>(_ExtTypeName).Get();
						//
						const FMonoClassDefinition ExtDefinition = GetClassMethodsInfo(ExtClass);
						for (const auto &Method : ExtDefinition.Methods) {
							if (Method.Name.ToString().StartsWith(TEXT("Internal"),ESearchCase::CaseSensitive)) {continue;}
							Results.AddUnique(Method.Name.ToString());
						}///
					}///
				}///
				//
			}///
		}
		//
		Mutex.Unlock();
	}
}

void IKCS_MonoAnalyzer::CollectClassMethods(MonoClass* ManagedClass, TArray<FString>&Results) {
	MonoImage* Image = mono_class_get_image(ManagedClass);
	//
	FCriticalSection Mutex;
	{
		Mutex.Lock();
		//
		while(ManagedClass!=nullptr) {
			void* Itr=0; MonoMethod* ClassMethod = nullptr;
			while((ClassMethod=mono_class_get_methods(ManagedClass,&Itr)) != NULL) {
				const char* MethodName = mono_method_get_name(ClassMethod);
				Results.AddUnique(StringCast<TCHAR>(MethodName).Get());
			}///
			//
			ManagedClass = mono_class_get_parent(ManagedClass);
			if (ManagedClass==mono_get_object_class()) {break;} Itr=0;
		}///
		//
		{
			const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
			int Rows = mono_table_info_get_rows(TbInfo);
			//
			const FString ClassName = FString(StringCast<TCHAR>(mono_class_get_name(ManagedClass)).Get());
			const FString XName = ClassName+TEXT("Extensions");
			//
			//for (int X=0; X<Rows; ++X) {
			ParallelFor((int32)Rows,[&](int32 X) {
				uint32_t XNMS[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(TbInfo,X,XNMS,MONO_TYPEDEF_SIZE);
				//
				const char* _ExtSpace = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAMESPACE]);
				const char* _ExtName = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAME]);
				//
				const FString ExtSpace = StringCast<TCHAR>(_ExtSpace).Get();
				const FString ExtName = StringCast<TCHAR>(_ExtName).Get();
				//
				if (ExtName.Equals(XName,ESearchCase::CaseSensitive)) {
					if (MonoClass*ExtClass=mono_class_from_name_case(Image,_ExtSpace,_ExtName)) {
						if (MonoType*ExtType=mono_class_get_type(ExtClass)) {
							const char* _ExtTypeName = mono_type_get_name_full(ExtType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
							FString ExtTypeName = StringCast<TCHAR>(_ExtTypeName).Get();
							//
							const FMonoClassDefinition ExtDefinition = GetClassMethodsInfo(ExtClass);
							for (const auto &Method : ExtDefinition.Methods) {
								if (Method.Name.ToString().StartsWith(TEXT("Internal"),ESearchCase::CaseSensitive)) {continue;}
								Results.AddUnique(Method.Name.ToString());
							}///
						}///
					}///
				}///
			});///
		}
		//
		Mutex.Unlock();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::SearchForClassType(MonoImage* Image, const FString &TypeName, TArray<FString>&Results) {
	const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
	int Rows = mono_table_info_get_rows(TbInfo);
	//
	//for (int I=0; I<Rows; ++I) {
	ParallelFor((int32)Rows,[&](int32 I) {
		uint32_t NMS[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
		//
		const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
		const char* _Name = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAME]);
		//
		if (strcmp(_Name,"<Module>")!=0) {
			const FString Space = StringCast<TCHAR>(_Space).Get();
			const FString Name = StringCast<TCHAR>(_Name).Get();
			//
			if (Space.Contains(TypeName,ESearchCase::CaseSensitive)) {
				Results.AddUnique(Space);
			} else if (Name.Contains(TypeName,ESearchCase::CaseSensitive)) {
				Results.AddUnique(Name);
			}///
		}///
	});///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::SearchForClassType(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results) {
	const FMonoClassDefinition &ClassInfo = Script->GetClassDefinition();
	//
	if (ClassInfo.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
		Results.AddUnique(ClassInfo.Name.ToString());
		SuggestedClassCache.Add(TypeName,ClassInfo);
	}///
	//
	if (ClassInfo.ParentClass.Contains(TypeName,ESearchCase::CaseSensitive)) {
		SuggestedClassCache.Add(ClassInfo.ParentClass,ClassInfo);
		Results.AddUnique(ClassInfo.ParentClass);
	}///
}

void IKCS_MonoAnalyzer::SearchForFieldType(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results) {
	const FMonoClassDefinition &ClassInfo = Script->GetClassDefinition();
	//
	for (const auto &Field : ClassInfo.Fields) {
		if (Field.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
			SuggestedFieldCache.Add(Field.Name.ToString(),Field);
			Results.AddUnique(Field.Name.ToString());
		}///
	}///
}

void IKCS_MonoAnalyzer::SearchForPropertyType(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results) {
	const FMonoClassDefinition &ClassInfo = Script->GetClassDefinition();
	//
	for (const auto &Prop : ClassInfo.Props) {
		if (Prop.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
			SuggestedPropertyCache.Add(Prop.Name.ToString(),Prop);
			Results.AddUnique(Prop.Name.ToString());
		}///
	}///
}

void IKCS_MonoAnalyzer::SearchForMethodType(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results) {
	const FMonoClassDefinition &ClassInfo = Script->GetClassDefinition();
	//
	for (const auto &Method : ClassInfo.Methods) {
		if (Method.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
			SuggestedMethodCache.Add(Method.Name.ToString(),Method);
			Results.AddUnique(Method.Name.ToString());
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IKCS_MonoAnalyzer::IsSystemClass(MonoClass* ManagedClass) {
	FString Space = StringCast<TCHAR>(mono_class_get_namespace(ManagedClass)).Get();
	Space.Split(TEXT("`"),&Space,nullptr,ESearchCase::IgnoreCase);
	//
	return (Space.StartsWith(TEXT("System"),ESearchCase::CaseSensitive)||Space.Equals(TEXT("System"),ESearchCase::CaseSensitive));
}

bool IKCS_MonoAnalyzer::SearchForAliasMembers(const FString &AliasName, TArray<FString>&Results) {
	if (AliasName.Equals("byte",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_byte_class(),Results); return true;}
	if (AliasName.Equals("char",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_char_class(),Results); return true;}
	if (AliasName.Equals("int",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_int32_class(),Results); return true;}
	if (AliasName.Equals("long",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_int64_class(),Results); return true;}
	if (AliasName.Equals("short",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_int16_class(),Results); return true;}
	if (AliasName.Equals("uint",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_uint32_class(),Results); return true;}
	if (AliasName.Equals("sbyte",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_sbyte_class(),Results); return true;}
	if (AliasName.Equals("ulong",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_uint64_class(),Results); return true;}
	if (AliasName.Equals("float",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_single_class(),Results); return true;}
	if (AliasName.Equals("bool",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_boolean_class(),Results); return true;}
	if (AliasName.Equals("double",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_double_class(),Results); return true;}
	if (AliasName.Equals("ushort",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_uint16_class(),Results); return true;}
	if (AliasName.Equals("string",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_string_class(),Results); return true;}
	if (AliasName.Equals("object",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_object_class(),Results); return true;}
	if (AliasName.Equals("IntPtr",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_intptr_class(),Results); return true;}
	if (AliasName.Equals("dynamic",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_get_object_class(),Results); return true;}
	if (AliasName.Equals("decimal",ESearchCase::CaseSensitive)) {CollectClassMethods(mono_class_from_name_case(mono_get_corlib(),"System","Decimal"),Results); return true;}
	//
	return false;
}

bool IKCS_MonoAnalyzer::SearchForSpaceMembers(MonoImage* Image, const FString &SpaceName, TArray<FString>&Results) {
	const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
	int Rows = mono_table_info_get_rows(TbInfo);
	bool GotSpace = false;
	//
	//for (int I=0; I<Rows; ++I) {
	ParallelFor((int32)Rows,[&](int32 I) {
		uint32_t NMS[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
		//
		const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
		const FString Space = StringCast<TCHAR>(_Space).Get();
		//
		if (strcmp(_Space,"<Module>")!=0) {
			FString Root{}; FString Scope{};
			if (Space.Split(TEXT("."),&Root,&Scope,ESearchCase::IgnoreCase)) {
				if (Root.Equals(SpaceName,ESearchCase::CaseSensitive)) {
					Results.AddUnique(Scope); GotSpace=true;
				}///
			}///
		}///
	});///
	//
	return GotSpace;
}

bool IKCS_MonoAnalyzer::SearchForClassMembers(MonoImage* Image, const FString &ClassName, TArray<FString>&Results) {
	if (MonoClass*UnrealClass=mono_class_from_name_case(Image,CS_CORE_NAMESPACE,StringCast<ANSICHAR>(*ClassName).Get())) {
		CollectAllClassMembers(UnrealClass,Results); return true;
	}///
	//
	if (SearchForAliasMembers(ClassName,Results)) {return true;}
	//
	const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
	int Rows = mono_table_info_get_rows(TbInfo);
	//
	//for (int I=0; I<Rows; ++I) {
	ParallelFor((int32)Rows,[&](int32 I) {
		uint32_t NMS[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
		//
		const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
		const char* _Name = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAME]);
		//
		if (strcmp(_Name,"<Module>")!=0) {
			const FString Space = StringCast<TCHAR>(_Space).Get();
			const FString Name = StringCast<TCHAR>(_Name).Get();
			//
			if (Space.Equals(ClassName,ESearchCase::CaseSensitive)) {
				Results.AddUnique(Name);
			} else if (Name.Equals(ClassName,ESearchCase::CaseSensitive)) {
				MonoClass* ManagedClass = mono_class_from_name_case(Image,_Space,_Name);
				if (strcmp(_Space,"System")==0) {CollectClassMethods(ManagedClass,Results);}
				else {CollectAllClassMembers(ManagedClass,Results);} return true;
			} else if (Name.Equals((ClassName+TEXT("Extensions")),ESearchCase::CaseSensitive)) {
				MonoClass* ManagedClass = mono_class_from_name_case(Image,_Space,_Name);
				if (strcmp(_Space,"System")==0) {CollectClassMethods(ManagedClass,Results);}
				else {CollectClassMethods(ManagedClass,Results);} return true;
			}///
		} return false;
	});///
	//
	return false;
}

void IKCS_MonoAnalyzer::SearchForFieldMembers(MonoImage* Image, const FString &TypeName, TArray<FString>&Results) {
	if (SearchForAliasMembers(TypeName,Results)) {return;}
	//
	const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
	int Rows = mono_table_info_get_rows(TbInfo);
	//
	//for (int I=0; I<Rows; ++I) {
	ParallelFor((int32)Rows,[&](int32 I) {
		uint32_t NMS[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
		//
		const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
		const char* _Name = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAME]);
		//
		if (strcmp(_Name,"<Module>")!=0) {
			if (MonoClass*ParentClass=mono_class_from_name_case(Image,_Space,_Name)) {
				MonoClassField* ClassField = nullptr; void* Itr=0;
				//
				while(ParentClass!=nullptr) {
					ClassField = mono_class_get_fields(ParentClass,&Itr);
					//
					if (ClassField==nullptr) {
						ParentClass = mono_class_get_parent(ParentClass);
						if (ParentClass==mono_get_object_class()){break;}
						Itr=0; continue;
					}///
					//
					MonoType* FieldType = mono_field_get_type(ClassField);
					MonoClass* TypeClass = mono_class_from_mono_type(FieldType);
					//
					const char* _FieldName = mono_field_get_name(ClassField);
					FString FieldName = FString(StringCast<TCHAR>(_FieldName).Get());
					//
					if (FieldName.Equals(TypeName,ESearchCase::CaseSensitive)) {
						CollectAllClassMembers(TypeClass,Results); return;
					}///
				}///
			}///
		}///
	});///
}

void IKCS_MonoAnalyzer::SearchForPropertyMembers(MonoImage* Image, const FString &TypeName, TArray<FString>&Results) {
	if (SearchForAliasMembers(TypeName,Results)) {return;}
	//
	const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
	int Rows = mono_table_info_get_rows(TbInfo);
	//
	//for (int I=0; I<Rows; ++I) {
	ParallelFor((int32)Rows,[&](int32 I) {
		uint32_t NMS[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
		//
		const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
		const char* _Name = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAME]);
		//
		if (strcmp(_Name,"<Module>")!=0) {
			if (MonoClass*ParentClass=mono_class_from_name_case(Image,_Space,_Name)) {
				MonoProperty* ClassProperty = nullptr; void* Itr=0;
				//
				while(ParentClass!=nullptr) {
					ClassProperty=mono_class_get_properties(ParentClass,&Itr);
					//
					if (ClassProperty==nullptr) {
						ParentClass = mono_class_get_parent(ParentClass);
						if (ParentClass==mono_get_object_class()){break;}
						Itr=0; continue;
					}///
					//
					MonoType* PropType = GetPropertyType(ClassProperty);
					MonoClass* TypeClass = mono_class_from_mono_type(PropType);
					//
					const char* _PropName = mono_property_get_name(ClassProperty);
					FString PropName = FString(StringCast<TCHAR>(_PropName).Get());
					//
					if (PropName.Equals(TypeName,ESearchCase::CaseSensitive)) {
						CollectAllClassMembers(TypeClass,Results); return;
					}///
				}///
			}///
		}///
	});///
}

void IKCS_MonoAnalyzer::SearchForMethodMembers(MonoImage* Image, const FString &TypeName, TArray<FString>&Results) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	if (SearchForAliasMembers(TypeName,Results)) {return;}
	//
	FString Parent{}; FString Scope{};
	if (TypeName.Split(TEXT("."),&Parent,&Scope,ESearchCase::IgnoreCase,ESearchDir::FromEnd)) {
		const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
		int Rows = mono_table_info_get_rows(TbInfo);
		//
		//for (int I=0; I<Rows; ++I) {
		ParallelFor((int32)Rows,[&](int32 I) {
			uint32_t NMS[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
			//
			const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
			const char* _Name = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAME]);
			//
			if (strcmp(_Name,"<Module>")!=0) {
				//if (!Parent.Equals(StringCast<TCHAR>(_Space).Get(),ESearchCase::CaseSensitive)) {continue;}
				//if (!Scope.Equals(StringCast<TCHAR>(_Name).Get(),ESearchCase::CaseSensitive)) {continue;}
				//
				if (MonoClass*ParentClass=mono_class_from_name_case(Image,_Space,_Name)) {
					MonoMethod* ClassMethod = nullptr; void* Itr=0;
					//
					while(ParentClass!=nullptr) {
						ClassMethod=mono_class_get_methods(ParentClass,&Itr);
						//
						if (ClassMethod==nullptr) {
							ParentClass = mono_class_get_parent(ParentClass);
							if (ParentClass==mono_get_object_class()){break;}
							Itr=0; continue;
						}///
						//
						MonoType* MethodType = GetMethodReturnType(ParentClass,ClassMethod);
						MonoClass* TypeClass = mono_class_from_mono_type(MethodType);
						//
						const char* _MethodName = mono_class_get_name(TypeClass);
						FString MethodName = FString(StringCast<TCHAR>(_MethodName).Get());
						MethodName.Split(TEXT("`"),&MethodName,nullptr,ESearchCase::IgnoreCase);
						//
						if (MethodName.Equals(Scope,ESearchCase::CaseSensitive)) {
							CollectAllClassMembers(TypeClass,Results); return;
						}///
					}///
				}///
			}///
		});///
	} else {
		const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
		int Rows = mono_table_info_get_rows(TbInfo);
		//
		//for (int I=0; I<Rows; ++I) {
		ParallelFor((int32)Rows,[&](int32 I) {
			uint32_t NMS[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
			//
			const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
			const char* _Name = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAME]);
			//
			if (strcmp(_Name,"<Module>")!=0) {
				if (MonoClass*ParentClass=mono_class_from_name_case(Image,_Space,_Name)) {
					MonoMethod* ClassMethod = nullptr; void* Itr=0;
					//
					while(ParentClass!=nullptr) {
						ClassMethod=mono_class_get_methods(ParentClass,&Itr);
						//
						if (ClassMethod==nullptr) {
							ParentClass = mono_class_get_parent(ParentClass);
							if (ParentClass==mono_get_object_class()){break;}
							Itr=0; continue;
						}///
						//
						MonoType* MethodType = GetMethodReturnType(ParentClass,ClassMethod);
						MonoClass* TypeClass = mono_class_from_mono_type(MethodType);
						//
						const char* _MethodName = mono_class_get_name(TypeClass);
						FString MethodName = FString(StringCast<TCHAR>(_MethodName).Get());
						MethodName.Split(TEXT("`"),&MethodName,nullptr,ESearchCase::IgnoreCase);
						//
						if (MethodName.Equals(TypeName,ESearchCase::CaseSensitive)) {
							CollectAllClassMembers(TypeClass,Results); return;
						}///
					}///
				}///
			}///
		});///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::SearchForFieldMembers(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	const FMonoClassDefinition &ClassInfo = Script->GetClassDefinition();
	//
	for (const auto &Field : ClassInfo.Fields) {
		if (Field.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
			if (SearchForAliasMembers(Field.TypeToString(),Results)) {goto END;}
			//
			if (Field.TypeToString().Contains(TEXT("."))) {
				FString Space{}; FString Class{};
				Field.TypeToString().Split(TEXT("."),&Space,&Class,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
				//
				if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorCoreImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
				if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorUnrealImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
				if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorSystemImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectClassMethods(FullClass,Results); goto END;}
			}///
			//
			if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorCoreImage(),Field.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
			if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorUnrealImage(),Field.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
			if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorSystemImage(),Field.TypeToString())) {CollectClassMethods(TypeClass,Results); goto END;}
		}///
	}///
	//
	END:{
		Results.Sort();
	}///
}

void IKCS_MonoAnalyzer::SearchForPropertyMembers(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	const FMonoClassDefinition &ClassInfo = Script->GetClassDefinition();
	//
	for (const auto &Prop : ClassInfo.Props) {
		if (Prop.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
			if (SearchForAliasMembers(Prop.TypeToString(),Results)) {goto END;}
			//
			if (Prop.TypeToString().Contains(TEXT("."))) {
				FString Space{}; FString Class{};
				//
				Prop.TypeToString().Split(TEXT("."),&Space,&Class,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
				if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorCoreImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
				if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorUnrealImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
				if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorSystemImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectClassMethods(FullClass,Results); goto END;}
			}///
			//
			if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorCoreImage(),Prop.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
			if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorUnrealImage(),Prop.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
			if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorSystemImage(),Prop.TypeToString())) {CollectClassMethods(TypeClass,Results); goto END;}
		}///
	}///
	//
	END:{
		Results.Sort();
	}///
}

void IKCS_MonoAnalyzer::SearchForMethodMembers(UMagicNodeSharpSource* const &Script, const FString &TypeName, TArray<FString>&Results) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	const FMonoClassDefinition &ClassInfo = Script->GetClassDefinition();
	//
	for (const auto &Method : ClassInfo.Methods) {
		if (Method.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
			if (SearchForAliasMembers(Method.TypeToString(),Results)) {goto END;}
			//
			FString Space{}; FString Class{};
			if (Method.TypeToString().Contains(TEXT("."))) {
				Method.TypeToString().Split(TEXT("."),&Space,&Class,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
				if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorCoreImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
				if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorUnrealImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
				if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorSystemImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectClassMethods(FullClass,Results); goto END;}
			}///
			//
			if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorCoreImage(),Method.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
			if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorUnrealImage(),Method.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
			if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorSystemImage(),Method.TypeToString())) {CollectClassMethods(TypeClass,Results); goto END;}
		}///
	}///
	//
	END:{
		Results.Sort();
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::SearchNamespaceInfo(const FMonoNamespaceDefinition &NamespaceInfo, const FString &ClassName, TArray<FString>&Results) {
	for (const auto &Space : NamespaceInfo.Namespaces) {
		if (Space.Contains(ClassName,ESearchCase::CaseSensitive)||ClassName==TEXT("#")) {
			Results.AddUnique(Space);
		}///
	}///
	//
	for (const auto &IT : TypeInfoCache) {
		if (IT.Key.Equals(ClassName,ESearchCase::CaseSensitive)) {
			if (IT.Value.Namespace.Equals(NamespaceInfo.Name.ToString(),ESearchCase::CaseSensitive)) {
				Results.AddUnique(IT.Value.Name.ToString());
			}///
		}///
	}///
}

void IKCS_MonoAnalyzer::SearchClassInfo(const FMonoClassDefinition &ClassInfo, const FString &TypeName, TArray<FString>&Results) {
	for (const auto &Field : ClassInfo.Fields) {
		if (Results.Num()>=MAX_SUGGESTIONS) {break;}
		if (Field.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)||TypeName==TEXT("#")) {
			SuggestedFieldCache.Add(Field.Name.ToString(),Field);
			Results.AddUnique(Field.Name.ToString());
		}///
	}///
	//
	for (const auto &Prop : ClassInfo.Props) {
		if (Results.Num()>=MAX_SUGGESTIONS) {break;}
		if (Prop.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)||TypeName==TEXT("#")) {
			SuggestedPropertyCache.Add(Prop.Name.ToString(),Prop);
			Results.AddUnique(Prop.Name.ToString());
		}///
	}///
	//
	for (const auto &Method : ClassInfo.Methods) {
		if (Results.Num()>=MAX_SUGGESTIONS) {break;}
		if (Method.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)||TypeName==TEXT("#")) {
			SuggestedMethodCache.Add(Method.Name.ToString(),Method);
			Results.AddUnique(Method.Name.ToString());
		}///
	}///
}

void IKCS_MonoAnalyzer::SearchFieldInfo(const FMonoFieldDefinition &FieldInfo, const FString &TypeName, TArray<FString>&Results) {
	if (FieldInfo.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
		Results.AddUnique(FieldInfo.Name.ToString());
	}///
}

void IKCS_MonoAnalyzer::SearchPropertyInfo(const FMonoFieldDefinition &PropertyInfo, const FString &TypeName, TArray<FString>&Results) {
	if (PropertyInfo.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
		Results.AddUnique(PropertyInfo.Name.ToString());
	}///
}

void IKCS_MonoAnalyzer::SearchMethodInfo(const FMonoMethodDefinition &MethodInfo, const FString &TypeName, TArray<FString>&Results) {
	if (MethodInfo.Name.ToString().Contains(TypeName,ESearchCase::CaseSensitive)) {
		Results.AddUnique(MethodInfo.Name.ToString());
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::SearchNamespaceInfo(const FMonoNamespaceDefinition &NamespaceInfo, const TArray<FString>&Tree, TArray<FString>&Results) {
	if (Tree.Num()==0) {return;}
	//
	FString FullSpace{};
	for (int32 I=0; I<Tree.Num(); ++I) {
		FullSpace.Append(Tree[I]);
		//
		if (I<(Tree.Num()-1)) {
			FullSpace.AppendChar(TEXT('.'));
		}///
	}///
	//
	for (const auto &Space : NamespaceInfo.Namespaces) {
		if (Space.Equals(FullSpace,ESearchCase::CaseSensitive)) {Results.AddUnique(Space);}
		if (Space.Equals(Tree[Tree.Num()-1],ESearchCase::CaseSensitive)) {Results.AddUnique(Space);}
	}///
	//
	for (const auto &IT : TypeInfoCache) {
		if (IT.Value.Namespace.Equals(FullSpace,ESearchCase::CaseSensitive)) {
			Results.AddUnique(IT.Value.Name.ToString());
		} else if (IT.Value.Namespace.Equals(Tree[Tree.Num()-1],ESearchCase::CaseSensitive)) {
			Results.AddUnique(IT.Value.Name.ToString());
		}///
	}///
}

void IKCS_MonoAnalyzer::SearchClassInfo(const FMonoClassDefinition &ClassInfo, const TArray<FString>&Tree, TArray<FString>&Results) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	FString Parent; FString Scope;
	for (int32 I=0; I<Tree.Num(); ++I) {
		Parent = Scope; Scope = Tree[I];
		//
		for (const auto &Field : ClassInfo.Fields) {
			if (Field.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				if (I==(Tree.Num()-1)) {
					if (SearchForAliasMembers(Field.TypeToString(),Results)) {goto END;}
					//
					if (Field.TypeToString().Contains(TEXT("."))) {
						FString Space{}; FString Class{};
						Field.TypeToString().Split(TEXT("."),&Space,&Class,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
						//
						if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorCoreImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
						if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorUnrealImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
						if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorSystemImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectClassMethods(FullClass,Results); goto END;}
					}///
					//
					if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorCoreImage(),Field.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
					if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorUnrealImage(),Field.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
					if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorSystemImage(),Field.TypeToString())) {CollectClassMethods(TypeClass,Results); goto END;}
				} else {goto NEXT;}
			}///
		}///
		//
		for (const auto &Prop : ClassInfo.Props) {
			if (Prop.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				if (I==(Tree.Num()-1)) {
					if (SearchForAliasMembers(Prop.TypeToString(),Results)) {goto END;}
					//
					if (Prop.TypeToString().Contains(TEXT("."))) {
						FString Space{}; FString Class{};
						Prop.TypeToString().Split(TEXT("."),&Space,&Class,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
						//
						if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorCoreImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
						if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorUnrealImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
						if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorSystemImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectClassMethods(FullClass,Results); goto END;}
					}///
					//
					if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorCoreImage(),Prop.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
					if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorUnrealImage(),Prop.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
					if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorSystemImage(),Prop.TypeToString())) {CollectClassMethods(TypeClass,Results); goto END;}
				} else {goto NEXT;}
			}///
		}///
		//
		for (const auto &Method : ClassInfo.Methods) {
			if (Method.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				if (I==(Tree.Num()-1)) {
					if (SearchForAliasMembers(Method.TypeToString(),Results)) {goto END;}
					//
					if (Method.TypeToString().Contains(TEXT("."))) {
						FString Space{}; FString Class{};
						Method.TypeToString().Split(TEXT("."),&Space,&Class,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
						//
						if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorCoreImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
						if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorUnrealImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectAllClassMembers(FullClass,Results); goto END;}
						if (MonoClass*FullClass=mono_class_from_name_case(MonoCore.GetEditorSystemImage(),StringCast<ANSICHAR>(*Space).Get(),StringCast<ANSICHAR>(*Class).Get())) {CollectClassMethods(FullClass,Results); goto END;}
					}///
					//
					if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorCoreImage(),Method.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
					if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorUnrealImage(),Method.TypeToString())) {CollectAllClassMembers(TypeClass,Results); goto END;}
					if (MonoClass*TypeClass=FindClassByName(MonoCore.GetEditorSystemImage(),Method.TypeToString())) {CollectClassMethods(TypeClass,Results); goto END;}
				} else {goto NEXT;}
			}///
		}///
		//
		NEXT:continue;
	}///
	//
	END:{
		Results.Sort();
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FMonoClassDefinition IKCS_MonoAnalyzer::GetClassInfo(MonoClass* ManagedClass) {
	FMonoClassDefinition ClassInfo{};
	//
	if (ManagedClass==nullptr) {return ClassInfo;}
	//
	const char* _Space = mono_class_get_namespace(ManagedClass);
	const char* _Name = mono_class_get_name(ManagedClass);
	//
	FString CleanName = StringCast<TCHAR>(_Name).Get();
	CleanName.Split(TEXT("`"),&CleanName,nullptr,ESearchCase::IgnoreCase);
	//
	ClassInfo.Name = FName(*CleanName);
	ClassInfo.Namespace = StringCast<TCHAR>(_Space).Get();
	//
	MonoImage* Image = mono_class_get_image(ManagedClass);
	if (MonoType*ClassType=mono_class_get_type(ManagedClass)) {
		const char* Info = mono_type_get_name_full(ClassType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
		ClassInfo.Description = StringCast<TCHAR>(Info).Get();
	}///
	//
	{
		while(ManagedClass!=nullptr) {
			void* FItr=0; MonoClassField* ClassField = nullptr;
			while((ClassField=mono_class_get_fields(ManagedClass,&FItr)) != NULL) {
				const FMonoFieldDefinition Field = GetFieldInfo(ClassField);
				if (Field.IsValid()&&(!Field.Name.ToString().StartsWith(TEXT("Internal")))) {ClassInfo.Fields.Add(Field);}
			}///
			//
			void* PItr=0; MonoProperty* ClassProperty = nullptr;
			while((ClassProperty=mono_class_get_properties(ManagedClass,&PItr))!=NULL) {
				const FMonoFieldDefinition Property = GetPropertyInfo(ClassProperty);
				if (Property.IsValid()&&(!Property.Name.ToString().StartsWith(TEXT("Internal")))) {ClassInfo.Props.Add(Property);}
			}///
			//
			void* MItr=0; MonoMethod* ClassMethod = nullptr;
			while((ClassMethod=mono_class_get_methods(ManagedClass,&MItr)) != NULL) {
				const FMonoMethodDefinition Method = GetMethodInfo(ManagedClass,ClassMethod);
				if (Method.IsValid()&&(!Method.Name.ToString().StartsWith(TEXT("Internal")))) {ClassInfo.Methods.Add(Method);}
			}///
			//
			FItr=0; PItr=0; MItr=0;
			ManagedClass = mono_class_get_parent(ManagedClass);
			if (ManagedClass==mono_get_object_class()) {break;}
		}///
	}
	//
	{
		const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
		int Rows = mono_table_info_get_rows(TbInfo);
		//
		const FString XName = CleanName+TEXT("Extensions");
		//
		//for (int X=0; X<Rows; ++X) {
		ParallelFor((int32)Rows,[&](int32 X) {
			uint32_t XNMS[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(TbInfo,X,XNMS,MONO_TYPEDEF_SIZE);
			//
			const char* _ExtSpace = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAMESPACE]);
			const char* _ExtName = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAME]);
			//
			const FString ExtSpace = StringCast<TCHAR>(_ExtSpace).Get();
			const FString ExtName = StringCast<TCHAR>(_ExtName).Get();
			//
			if (ExtName.Equals(XName,ESearchCase::CaseSensitive)) {
				if (MonoClass*ExtClass=mono_class_from_name_case(Image,_ExtSpace,_ExtName)) {
					if (MonoType*ExtType=mono_class_get_type(ExtClass)) {
						const char* _ExtTypeName = mono_type_get_name_full(ExtType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
						FString ExtTypeName = StringCast<TCHAR>(_ExtTypeName).Get();
						//
						const FMonoClassDefinition ExtDefinition = GetClassMethodsInfo(ExtClass);
						//
						for (const auto &Method : ExtDefinition.Methods) {
							if (Method.Name.ToString().StartsWith(TEXT("Internal"),ESearchCase::CaseSensitive)) {continue;}
							ClassInfo.Methods.Add(Method);
						}///
					}///
				}///
			}///
		});///
	}
	//
	{
		auto Flags = mono_class_get_flags(ManagedClass);
		uint32_t MF_STATIC = MONO_TYPE_ATTR_ABSTRACT | MONO_TYPE_ATTR_SEALED;
		//
		if (((Flags)&MF_STATIC)==MF_STATIC) {ClassInfo.ScopeType=EMonoScopeType::Static;}
		if (((Flags)&MONO_TYPE_ATTR_CLASS_SEMANTIC_MASK)==MONO_TYPE_ATTR_INTERFACE) {ClassInfo.ScopeType=EMonoScopeType::Virtual;}
		if (((Flags)&MONO_TYPE_ATTR_VISIBILITY_MASK)==MONO_TYPE_ATTR_NESTED_ASSEMBLY) {ClassInfo.ScopeType=EMonoScopeType::Internal;}
		//
		if (((Flags)&MONO_TYPE_ATTR_VISIBILITY_MASK)==MONO_TYPE_ATTR_NOT_PUBLIC) {ClassInfo.AccessType=EMonoAccessType::Private;}
		if (((Flags)&MONO_TYPE_ATTR_VISIBILITY_MASK)==MONO_TYPE_ATTR_NESTED_PRIVATE) {ClassInfo.AccessType=EMonoAccessType::Private;}
		//
		if (((Flags)&MONO_TYPE_ATTR_VISIBILITY_MASK)==MONO_TYPE_ATTR_NESTED_ASSEMBLY) {ClassInfo.AccessType=EMonoAccessType::Protected;}
		if (((Flags)&MONO_TYPE_ATTR_VISIBILITY_MASK)==MONO_TYPE_ATTR_NESTED_FAM_OR_ASSEM) {ClassInfo.AccessType=EMonoAccessType::Protected;}
		if (((Flags)&MONO_TYPE_ATTR_VISIBILITY_MASK)==MONO_TYPE_ATTR_NESTED_FAM_AND_ASSEM) {ClassInfo.AccessType=EMonoAccessType::Protected;}
		//
		if (((Flags)&MONO_TYPE_ATTR_VISIBILITY_MASK)==MONO_TYPE_ATTR_PUBLIC) {ClassInfo.AccessType=EMonoAccessType::Public;}
		if (((Flags)&MONO_TYPE_ATTR_VISIBILITY_MASK)==MONO_TYPE_ATTR_NESTED_PUBLIC) {ClassInfo.AccessType=EMonoAccessType::Public;}
	}
	//
	return ClassInfo;
}

const FMonoClassDefinition IKCS_MonoAnalyzer::GetClassMethodsInfo(MonoClass* ManagedClass) {
	FMonoClassDefinition ClassInfo{};
	//
	if (ManagedClass==nullptr) {return ClassInfo;}
	//
	const char* _Space = mono_class_get_namespace(ManagedClass);
	const char* _Name = mono_class_get_name(ManagedClass);
	//
	FString ClassName = FString(StringCast<TCHAR>(_Name).Get());
	ClassName.Split(TEXT("`"),&ClassName,nullptr,ESearchCase::IgnoreCase);
	//
	ClassInfo.Namespace = StringCast<TCHAR>(_Space).Get();
	ClassInfo.Name = FName(*ClassName);
	//
	MonoImage* Image = mono_class_get_image(ManagedClass);
	if (MonoType*ClassType=mono_class_get_type(ManagedClass)) {
		const char* Info = mono_type_get_name_full(ClassType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
		ClassInfo.Description = StringCast<TCHAR>(Info).Get();
	}///
	//
	while(ManagedClass!=nullptr) {
		MonoMethod* ClassMethod = nullptr;
		void* MItr=0;
		//
		while((ClassMethod=mono_class_get_methods(ManagedClass,&MItr)) != NULL) {
			const FMonoMethodDefinition Method = GetMethodInfo(ManagedClass,ClassMethod);
			if (Method.IsValid()&&(!Method.Name.ToString().StartsWith(TEXT("Internal")))) {
				ClassInfo.Methods.Add(Method);
			}///
		} MItr=0;
		//
		ManagedClass = mono_class_get_parent(ManagedClass);
		if (ManagedClass==mono_get_object_class()) {break;}
	}///
	//
	{
		const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
		int Rows = mono_table_info_get_rows(TbInfo);
		//
		const FString XName = ClassName+TEXT("Extensions");
		//
		//for (int X=0; X<Rows; ++X) {
		ParallelFor((int32)Rows,[&](int32 X) {
			uint32_t XNMS[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(TbInfo,X,XNMS,MONO_TYPEDEF_SIZE);
			//
			const char* _ExtSpace = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAMESPACE]);
			const char* _ExtName = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAME]);
			//
			const FString ExtSpace = StringCast<TCHAR>(_ExtSpace).Get();
			const FString ExtName = StringCast<TCHAR>(_ExtName).Get();
			//
			if (ExtName.Equals(XName,ESearchCase::CaseSensitive)) {
				if (MonoClass*ExtClass=mono_class_from_name_case(Image,_ExtSpace,_ExtName)) {
					if (MonoType*ExtType=mono_class_get_type(ExtClass)) {
						const char* _ExtTypeName = mono_type_get_name_full(ExtType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
						FString ExtTypeName = StringCast<TCHAR>(_ExtTypeName).Get();
						//
						const FMonoClassDefinition ExtDefinition = GetClassMethodsInfo(ExtClass);
						//
						for (const auto &Method : ExtDefinition.Methods) {
							if (Method.Name.ToString().StartsWith(TEXT("Internal"),ESearchCase::CaseSensitive)) {continue;}
							ClassInfo.Methods.Add(Method);
						}///
					}///
				}///
			}///
		});///
	}
	//
	ClassInfo.ScopeType = EMonoScopeType::Member;
	ClassInfo.AccessType = EMonoAccessType::Public;
	//
	return ClassInfo;
}

const FMonoFieldDefinition IKCS_MonoAnalyzer::GetFieldInfo(MonoClassField* ClassField) {
	FMonoFieldDefinition FieldInfo{};
	//
	if (ClassField==nullptr) {return FieldInfo;}
	//
	const char* Name = mono_field_get_name(ClassField);
	FString FieldName = FString(StringCast<TCHAR>(Name).Get());
	//
	if (FieldName.StartsWith(TEXT("Internal"),ESearchCase::CaseSensitive)) {return FieldInfo;}
	if (FieldName.EndsWith(TEXT("k__BackingField"),ESearchCase::CaseSensitive)) {return FieldInfo;}
	{
		auto Flags = mono_field_get_flags(ClassField);
		FieldInfo.Name = FName(StringCast<TCHAR>(Name).Get());
		//
		GetDataTypeFromMonoType(GetFieldType(ClassField),FieldInfo);
		//
		if (((Flags)&MONO_FIELD_ATTR_FIELD_ACCESS_MASK)==MONO_FIELD_ATTR_ASSEMBLY) {FieldInfo.ScopeType=EMonoScopeType::Internal;}
		if (((Flags)&MONO_FIELD_ATTR_FIELD_ACCESS_MASK)==MONO_FIELD_ATTR_STATIC) {FieldInfo.ScopeType=EMonoScopeType::Static;}
		if (((Flags)&MONO_FIELD_ATTR_FIELD_ACCESS_MASK)==MONO_FIELD_ATTR_FAMILY) {FieldInfo.ScopeType=EMonoScopeType::Member;}
		//
		if (((Flags)&MONO_FIELD_ATTR_FIELD_ACCESS_MASK)==MONO_FIELD_ATTR_PUBLIC) {FieldInfo.AccessType=EMonoAccessType::Public;}
		if (((Flags)&MONO_FIELD_ATTR_FIELD_ACCESS_MASK)==MONO_FIELD_ATTR_PRIVATE) {FieldInfo.AccessType=EMonoAccessType::Private;}
		if (((Flags)&MONO_FIELD_ATTR_FIELD_ACCESS_MASK)==MONO_FIELD_ATTR_FAMILY) {FieldInfo.AccessType=EMonoAccessType::Protected;}
		if (((Flags)&MONO_FIELD_ATTR_FIELD_ACCESS_MASK)==MONO_FIELD_ATTR_FAM_OR_ASSEM) {FieldInfo.AccessType=EMonoAccessType::Protected;}
		if (((Flags)&MONO_FIELD_ATTR_FIELD_ACCESS_MASK)==MONO_FIELD_ATTR_FAM_AND_ASSEM) {FieldInfo.AccessType=EMonoAccessType::Protected;}
	}
	//
	if (MonoType*FieldType=mono_field_get_type(ClassField)) {
		const char* Info = mono_type_get_name_full(FieldType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
		FieldInfo.Description = FString::Printf(TEXT("%s   (%s)"),*FieldName,StringCast<TCHAR>(Info).Get());
		FieldInfo.TypeName = StringCast<TCHAR>(mono_type_get_name(FieldType)).Get();
	}///
	//
	return FieldInfo;
}

const FMonoFieldDefinition IKCS_MonoAnalyzer::GetPropertyInfo(MonoProperty* Property) {
	FMonoFieldDefinition PropInfo{};
	//
	if (Property==nullptr) {return PropInfo;}
	//
	const char* Name = mono_property_get_name(Property);
	FString PropName = FString(StringCast<TCHAR>(Name).Get());
	//
	if (PropName.StartsWith(TEXT("Internal"),ESearchCase::CaseSensitive)) {return PropInfo;}
	{
		PropInfo.Name = FName(StringCast<TCHAR>(Name).Get());
		//
		auto Flags = mono_property_get_flags(Property);
		MonoMethod* MethodGet = mono_property_get_get_method(Property);
		MonoMethod* MethodSet = mono_property_get_set_method(Property);
		//
		if (MethodSet) {
			auto MethodSign = mono_method_get_signature(MethodSet,mono_class_get_image(mono_property_get_parent(Property)),mono_method_get_token(MethodSet));
			auto ReturnType = mono_signature_get_return_type(MethodSign);
			auto MethodFlags = mono_method_get_flags(MethodSet,nullptr);
			//
			GetDataTypeFromMonoType(ReturnType,PropInfo);
			//
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAM_AND_ASSEM) {PropInfo.ScopeType=EMonoScopeType::Internal;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_ASSEM) {PropInfo.ScopeType=EMonoScopeType::Internal;}
			//
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_VIRTUAL) {PropInfo.ScopeType=EMonoScopeType::Virtual;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAMILY) {PropInfo.ScopeType=EMonoScopeType::Member;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_STATIC) {PropInfo.ScopeType=EMonoScopeType::Static;}
			//
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_PUBLIC) {PropInfo.AccessType=EMonoAccessType::Public;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_PRIVATE) {PropInfo.AccessType=EMonoAccessType::Private;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAMILY) {PropInfo.AccessType=EMonoAccessType::Protected;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAM_OR_ASSEM) {PropInfo.AccessType=EMonoAccessType::Protected;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAM_AND_ASSEM) {PropInfo.AccessType=EMonoAccessType::Protected;}
		}///
		//
		if (MethodGet) {
			auto MethodSign = mono_method_get_signature(MethodGet,mono_class_get_image(mono_property_get_parent(Property)),mono_method_get_token(MethodGet));
			auto ReturnType = mono_signature_get_return_type(MethodSign);
			auto MethodFlags = mono_method_get_flags(MethodGet,nullptr);
			//
			GetDataTypeFromMonoType(ReturnType,PropInfo);
			//
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAM_AND_ASSEM) {PropInfo.ScopeType=EMonoScopeType::Internal;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_ASSEM) {PropInfo.ScopeType=EMonoScopeType::Internal;}
			//
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_VIRTUAL) {PropInfo.ScopeType=EMonoScopeType::Virtual;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAMILY) {PropInfo.ScopeType=EMonoScopeType::Member;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_STATIC) {PropInfo.ScopeType=EMonoScopeType::Static;}
			//
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_PUBLIC) {PropInfo.AccessType=EMonoAccessType::Public;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_PRIVATE) {PropInfo.AccessType=EMonoAccessType::Private;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAMILY) {PropInfo.AccessType=EMonoAccessType::Protected;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAM_OR_ASSEM) {PropInfo.AccessType=EMonoAccessType::Protected;}
			if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAM_AND_ASSEM) {PropInfo.AccessType=EMonoAccessType::Protected;}
			//
			const char* Info = mono_type_get_name_full(ReturnType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
			PropInfo.Description = FString::Printf(TEXT("%s   (%s)"),*PropName,StringCast<TCHAR>(Info).Get());
			PropInfo.TypeName = StringCast<TCHAR>(mono_type_get_name(ReturnType)).Get();
			//
			if (((Flags)&MONO_PROPERTY_ATTR_UNUSED)!=NULL) {
				LOG::CSX_STR(ESeverity::Warning,FString::Printf(TEXT("C# Property declared, but never used:	%s"),*PropInfo.Description));
			}///
		}///
	}
	//
	return PropInfo;
}

const FMonoMethodDefinition IKCS_MonoAnalyzer::GetMethodInfo(MonoClass* ParentClass, MonoMethod* Method) {
	FMonoMethodDefinition MethodInfo{};
	//
	if (Method==nullptr) {return MethodInfo;}
	if (ParentClass==nullptr) {return MethodInfo;}
	//
	const char* Name = mono_method_get_name(Method);
	const char* ParentName = mono_class_get_name(ParentClass);
	//
	FString MethodName = FString(StringCast<TCHAR>(Name).Get());
	MethodName.Split(TEXT("`"),&MethodName,nullptr,ESearchCase::IgnoreCase);
	//
	{
		MethodInfo.Name = FName(StringCast<TCHAR>(Name).Get());
		//
		const char* Info = mono_method_full_name(Method,true);
		auto MethodFlags = mono_method_get_flags(Method,nullptr);
		//
		auto MethodSign = mono_method_get_signature(Method,mono_class_get_image(ParentClass),mono_method_get_token(Method));
		auto MethodParams = GetMethodParameterInfo(ParentClass,Method);
		auto ReturnType = mono_signature_get_return_type(MethodSign);
		//
		for (const auto &Param : MethodParams) {
			if (!Param.IsValid()) {continue;}
			//
			if (Param.ParamType==EMonoParamType::Input) {
				MethodInfo.Inputs.Add(Param);
			} else {MethodInfo.Outputs.Add(Param);}
		}///
		//
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAM_AND_ASSEM) {MethodInfo.ScopeType=EMonoScopeType::Internal;}
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_ASSEM) {MethodInfo.ScopeType=EMonoScopeType::Internal;}
		//
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_VIRTUAL) {MethodInfo.ScopeType=EMonoScopeType::Virtual;}
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_STATIC) {MethodInfo.ScopeType=EMonoScopeType::Static;}
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAMILY) {MethodInfo.ScopeType=EMonoScopeType::Member;}
		//
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_PUBLIC) {MethodInfo.AccessType=EMonoAccessType::Public;}
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_PRIVATE) {MethodInfo.AccessType=EMonoAccessType::Private;}
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAMILY) {MethodInfo.AccessType=EMonoAccessType::Protected;}
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAM_OR_ASSEM) {MethodInfo.AccessType=EMonoAccessType::Protected;}
		if (((MethodFlags)&MONO_METHOD_ATTR_ACCESS_MASK)==MONO_METHOD_ATTR_FAM_AND_ASSEM) {MethodInfo.AccessType=EMonoAccessType::Protected;}
		//
		MethodInfo.ReturnTypeName = StringCast<TCHAR>(mono_type_get_name(ReturnType)).Get();
		MethodInfo.Description = FString::Printf(TEXT("<%s> %s :\n%s :: %s"),*MethodInfo.ScopeToString(),*MethodInfo.AccessToString(),*MethodInfo.ReturnTypeName,StringCast<TCHAR>(Info).Get());
		MethodInfo.Description.RemoveFromStart(TEXT("<> "));
	}
	//
	return MethodInfo;
}

const TArray<FMonoParameterType>IKCS_MonoAnalyzer::GetMethodParameterInfo(MonoClass* ParentClass, MonoMethod* Method) {
	TArray<FMonoParameterType>MethodParams{};
	std::vector<const char*>ParamNames{};
	//
	if (Method==nullptr) {return MethodParams;}
	//
	MonoMethodSignature* MethodSign = mono_method_get_signature(Method,mono_class_get_image(ParentClass),mono_method_get_token(Method));
	uint32_t ParamCount = mono_signature_get_param_count(MethodSign);
	//
	if (ParamCount>0) {
		ParamNames.resize(ParamCount);
		MethodParams.Reserve(ParamCount);
		mono_method_get_param_names(Method,ParamNames.data());
		//
		void* Itr = nullptr; uint32_t SigIdx=0; MonoType* SigParam = nullptr;
		while((SigParam=mono_signature_get_params(MethodSign,&Itr)) != NULL) {
			FMonoParameterType TypeInfo{};
			//
			const char* ClassName = mono_type_get_name(SigParam);
			const char* TypeName = ParamNames[SigIdx];
			//
			TypeInfo.Name = StringCast<TCHAR>(TypeName).Get();
			TypeInfo.TypeName = StringCast<TCHAR>(ClassName).Get();
			//
			MethodParams.Insert(TypeInfo,SigIdx); SigIdx++;
		}///
		//
		for (uint32_t I=0; I<ParamCount; ++I) {
			bool IsOutput = (bool)mono_signature_param_is_out(MethodSign,I);
			if (MethodParams.IsValidIndex(I)) {
				if (IsOutput) {
					MethodParams[I].ParamType = EMonoParamType::Output;
				} else {
					MethodParams[I].ParamType = EMonoParamType::Input;
				}///
			}///
		}///
	}///
	//
	return MethodParams;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FMonoNamespaceDefinition & IKCS_MonoAnalyzer::GetNamespaceInfo(UMagicNodeSharpSource* const &Script, const FString &SpaceName) {
	for (const auto &Space : SpaceInfoCache) {
		if (Space.Name.ToString().Equals(SpaceName,ESearchCase::CaseSensitive)) {return Space;}
	}///
	//
	return VOID_Namespace;
}

const FMonoClassDefinition & IKCS_MonoAnalyzer::GetClassInfo(UMagicNodeSharpSource* const &Script, const FString &ClassName) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	if (Script->GetScriptName().Equals(ClassName,ESearchCase::CaseSensitive)) {SuggestedClassCache.Add(ClassName,ScriptInfo); return ScriptInfo;}
	if (ClassName.Equals(TEXT("this"),ESearchCase::CaseSensitive)) {SuggestedClassCache.Add(ClassName,ScriptInfo); return ScriptInfo;}
	//
	for (const auto &IT : TypeInfoCache) {
		if (IT.Key.Equals(ClassName,ESearchCase::CaseSensitive)) {SuggestedClassCache.Add(ClassName,IT.Value); return IT.Value;} else
		if (IT.Value.Name.ToString().Equals(ClassName,ESearchCase::CaseSensitive)) {SuggestedClassCache.Add(ClassName,IT.Value); return IT.Value;}
	}///
	//
	return VOID_Class;
}

const FMonoFieldDefinition & IKCS_MonoAnalyzer::GetFieldInfo(UMagicNodeSharpSource* const &Script, const FString &TypeName) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	for (const auto &IT : ScriptInfo.Fields) {
		if (IT.Name.ToString().Equals(TypeName,ESearchCase::CaseSensitive)) {return IT;}
	}///
	//
	return VOID_Field;
}

const FMonoFieldDefinition & IKCS_MonoAnalyzer::GetPropertyInfo(UMagicNodeSharpSource* const &Script, const FString &TypeName) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	for (const auto &IT : ScriptInfo.Props) {
		if (IT.Name.ToString().Equals(TypeName,ESearchCase::CaseSensitive)) {return IT;}
	}///
	//
	return VOID_Prop;
}

const FMonoMethodDefinition & IKCS_MonoAnalyzer::GetMethodInfo(UMagicNodeSharpSource* const &Script, const FString &MethodName) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	for (const auto &IT : ScriptInfo.Methods) {
		if (IT.Name.ToString().Equals(MethodName,ESearchCase::CaseSensitive)) {return IT;}
	}///
	//
	return VOID_Method;
}

const FMonoClassDefinition & IKCS_MonoAnalyzer::GetClassInfo(UMagicNodeSharpSource* const &Script, const FString &Context, const FString &Member) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	if (Context.IsEmpty()) {
		if (Script->GetScriptName().Equals(Member,ESearchCase::CaseSensitive)) {
			SuggestedClassCache.Add(Member,ScriptInfo); return ScriptInfo;
		}///
	} else if (Member.Equals(TEXT("this"),ESearchCase::CaseSensitive)) {
		SuggestedClassCache.Add(Member,ScriptInfo);
		return ScriptInfo;
	}///
	//
	if (ScriptInfo.Namespace.Equals(Context,ESearchCase::CaseSensitive)) {
		if (Script->GetScriptName().Equals(Member,ESearchCase::CaseSensitive)) {
			SuggestedClassCache.Add(Member,ScriptInfo);
			return ScriptInfo;
		}///
	}///
	//
	for (const auto &IT : TypeInfoCache) {
		if (IT.Value.Namespace.Equals(Context,ESearchCase::CaseSensitive)) {
			if (IT.Value.Name.ToString().Equals(Member,ESearchCase::CaseSensitive)) {
				SuggestedClassCache.Add(Member,IT.Value); return IT.Value;
			}///
		} else if (IT.Key.Equals(Member,ESearchCase::CaseSensitive)) {
			SuggestedClassCache.Add(Member,IT.Value); return IT.Value;
		} else if (IT.Key.EndsWith(Member,ESearchCase::CaseSensitive)) {
			if (IT.Value.Name.ToString().Equals(Member,ESearchCase::CaseSensitive)) {
				SuggestedClassCache.Add(Member,IT.Value); return IT.Value;
			}///
		}///
	}///
	//
	return VOID_Class;
}

const FMonoFieldDefinition & IKCS_MonoAnalyzer::GetFieldInfo(UMagicNodeSharpSource* const &Script, const FString &Context, const FString &TypeName) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	FString Super = Context;
	FString Scope = TypeName;
	//
	Super.RemoveFromEnd(TEXT("@"));
	Scope.RemoveFromEnd(TEXT("."));
	//
	if (Super.IsEmpty()||Script->GetScriptName().Equals(Super,ESearchCase::CaseSensitive)||Super.Equals(TEXT("this"),ESearchCase::CaseSensitive)) {
		for (const auto &Field : ScriptInfo.Fields) {
			if (Field.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				SuggestedFieldCache.Add(Scope,Field); return Field;
			}///
		}///
	}///
	//
	for (const auto &Field : ScriptInfo.Fields) {
		if (Field.Name.ToString().Equals(Super,ESearchCase::CaseSensitive)) {
			if (TypeInfoCache.Contains(Field.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Field.TypeToString());
				//
				for (const auto &TypeField : TypeInfo.Fields) {
					if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
					}///
				}///
			}///
			//
			const FString Type = FString(TEXT("."))+Field.TypeToString();
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(Type)) {
					for (const auto &TypeField : IT.Value.Fields) {
						if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
						}///
					}///
				}///
			}///
			//
			const FString ExtType = FString(TEXT("."))+Field.TypeToString()+FString(TEXT("Extensions"));
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(ExtType)) {
					for (const auto &TypeField : IT.Value.Fields) {
						if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
						}///
					}///
				}///
			}///
		} else if (Field.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
			SuggestedFieldCache.Add(Scope,Field); return Field;
		}///
	}///
	//
	for (const auto &Prop : ScriptInfo.Props) {
		if (Prop.Name.ToString().Equals(Super,ESearchCase::CaseSensitive)) {
			if (TypeInfoCache.Contains(Prop.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Prop.TypeToString());
				//
				for (const auto &TypeField : TypeInfo.Fields) {
					if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
					}///
				}///
			}///
			//
			const FString Type = FString(TEXT("."))+Prop.TypeToString();
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(Type)) {
					for (const auto &TypeField : IT.Value.Fields) {
						if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
						}///
					}///
				}///
			}///
			//
			const FString ExtType = FString(TEXT("."))+Prop.TypeToString()+FString(TEXT("Extensions"));
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(ExtType)) {
					for (const auto &TypeField : IT.Value.Fields) {
						if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
						}///
					}///
				}///
			}///
		}///
	}///
	//
	for (const auto &Method : ScriptInfo.Methods) {
		if (Method.Name.ToString().Equals(Super,ESearchCase::CaseSensitive)) {
			if (TypeInfoCache.Contains(Method.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Method.TypeToString());
				//
				for (const auto &TypeField : TypeInfo.Fields) {
					if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
					}///
				}///
			}///
			//
			const FString Type = FString(TEXT("."))+Method.TypeToString();
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(Type)) {
					for (const auto &TypeField : IT.Value.Fields) {
						if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
						}///
					}///
				}///
			}///
			//
			const FString ExtType = FString(TEXT("."))+Method.TypeToString()+FString(TEXT("Extensions"));
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(ExtType)) {
					for (const auto &TypeField : IT.Value.Fields) {
						if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
						}///
					}///
				}///
			}///
		}///
	}///
	//
	const FString Type = FString(TEXT("."))+Super;
	for (const auto &IT : TypeInfoCache) {
		if (IT.Key.EndsWith(Type)) {
			for (const auto &TypeField : IT.Value.Fields) {
				if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
					SuggestedFieldCache.Add(Super+TEXT(".")+Scope,TypeField); return TypeField;
				}///
			}///
		}///
	}///
	//
	for (const auto &IT : TypeInfoCache) {
		if (IT.Key.Equals(Super,ESearchCase::CaseSensitive)) {
			for (const auto &Field : IT.Value.Fields) {
				if (Field.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
					SuggestedFieldCache.Add(Super+TEXT(".")+Scope,Field); return Field;
				}///
			}///
		}///
	}///
	//
	return VOID_Field;
}

const FMonoFieldDefinition & IKCS_MonoAnalyzer::GetPropertyInfo(UMagicNodeSharpSource* const &Script, const FString &Context, const FString &TypeName) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	FString Super = Context;
	FString Scope = TypeName;
	//
	Super.RemoveFromEnd(TEXT("@"));
	Scope.RemoveFromEnd(TEXT("."));
	//
	if (Super.IsEmpty()||Script->GetScriptName().Equals(Super,ESearchCase::CaseSensitive)||Super.Equals(TEXT("this"),ESearchCase::CaseSensitive)) {
		for (const auto &Prop : ScriptInfo.Props) {
			if (Prop.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				SuggestedPropertyCache.Add(Scope,Prop); return Prop;
			}///
		}///
	}///
	//
	for (const auto &Field : ScriptInfo.Fields) {
		if (Field.Name.ToString().Equals(Super,ESearchCase::CaseSensitive)) {
			if (TypeInfoCache.Contains(Field.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Field.TypeToString());
				//
				for (const auto &TypeProp : TypeInfo.Props) {
					if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
					}///
				}///
			}///
			//
			const FString Type = FString(TEXT("."))+Field.TypeToString();
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(Type)) {
					for (const auto &TypeProp : IT.Value.Props) {
						if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
						}///
					}///
				}///
			}///
			//
			const FString ExtType = FString(TEXT("."))+Field.TypeToString()+FString(TEXT("Extensions"));
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(ExtType)) {
					for (const auto &TypeProp : IT.Value.Props) {
						if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
						}///
					}///
				}///
			}///
		}///
	}///
	//
	for (const auto &Prop : ScriptInfo.Props) {
		if (Prop.Name.ToString().Equals(Super,ESearchCase::CaseSensitive)) {
			if (TypeInfoCache.Contains(Prop.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Prop.TypeToString());
				//
				for (const auto &TypeProp : TypeInfo.Props) {
					if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
					}///
				}///
			}///
			//
			const FString Type = FString(TEXT("."))+Prop.TypeToString();
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(Type)) {
					for (const auto &TypeProp : IT.Value.Props) {
						if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
						}///
					}///
				}///
			}///
			//
			const FString ExtType = FString(TEXT("."))+Prop.TypeToString()+FString(TEXT("Extensions"));
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(ExtType)) {
					for (const auto &TypeProp : IT.Value.Props) {
						if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
						}///
					}///
				}///
			}///
		} else if (Prop.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
			SuggestedPropertyCache.Add(Scope,Prop); return Prop;
		}///
	}///
	//
	for (const auto &Method : ScriptInfo.Methods) {
		if (Method.Name.ToString().Equals(Super,ESearchCase::CaseSensitive)) {
			if (TypeInfoCache.Contains(Method.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Method.TypeToString());
				//
				for (const auto &TypeProp : TypeInfo.Props) {
					if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
					}///
				}///
			}///
			//
			const FString Type = FString(TEXT("."))+Method.TypeToString();
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(Type)) {
					for (const auto &TypeProp : IT.Value.Props) {
						if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
						}///
					}///
				}///
			}///
			//
			const FString ExtType = FString(TEXT("."))+Method.TypeToString()+FString(TEXT("Extensions"));
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(ExtType)) {
					for (const auto &TypeProp : IT.Value.Props) {
						if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
						}///
					}///
				}///
			}///
		}///
	}///
	//
	const FString Type = FString(TEXT("."))+Super;
	for (const auto &IT : TypeInfoCache) {
		if (IT.Key.EndsWith(Type)) {
			for (const auto &TypeProp : IT.Value.Props) {
				if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
					SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,TypeProp); return TypeProp;
				}///
			}///
		}///
	}///
	//
	for (const auto &IT : TypeInfoCache) {
		if (IT.Key.Equals(Super,ESearchCase::CaseSensitive)) {
			for (const auto &Prop : IT.Value.Props) {
				if (Prop.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
					SuggestedPropertyCache.Add(Super+TEXT(".")+Scope,Prop); return Prop;
				}///
			}///
		}///
	}///
	//
	return VOID_Prop;
}

const FMonoMethodDefinition & IKCS_MonoAnalyzer::GetMethodInfo(UMagicNodeSharpSource* const &Script, const FString &Context, const FString &MethodName) {
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	//
	FString Super = Context;
	FString Scope = MethodName;
	//
	Super.RemoveFromEnd(TEXT("@"));
	Scope.RemoveFromEnd(TEXT("@"));
	//
	if (Super.IsEmpty()||Script->GetScriptName().Equals(Super,ESearchCase::CaseSensitive)||Super.Equals(TEXT("this"),ESearchCase::CaseSensitive)) {
		for (const auto &Method : ScriptInfo.Methods) {
			if (Method.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				SuggestedMethodCache.Add(Scope,Method); return Method;
			}///
		}///
	}///
	//
	for (const auto &Field : ScriptInfo.Fields) {
		if (Field.Name.ToString().Equals(Super,ESearchCase::CaseSensitive)) {
			if (TypeInfoCache.Contains(Field.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Field.TypeToString());
				//
				for (const auto &TypeMethod : TypeInfo.Methods) {
					if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
					}///
				}///
			}///
			//
			const FString Type = FString(TEXT("."))+Field.TypeToString();
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(Type)) {
					for (const auto &TypeMethod : IT.Value.Methods) {
						if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
						}///
					}///
				}///
			}///
			//
			const FString ExtType = FString(TEXT("."))+Field.TypeToString()+FString(TEXT("Extensions"));
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(ExtType)) {
					for (const auto &TypeMethod : IT.Value.Methods) {
						if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
						}///
					}///
				}///
			}///
		}///
	}///
	//
	for (const auto &Prop : ScriptInfo.Props) {
		if (Prop.Name.ToString().Equals(Super,ESearchCase::CaseSensitive)) {
			if (TypeInfoCache.Contains(Prop.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Prop.TypeToString());
				//
				for (const auto &TypeMethod : TypeInfo.Methods) {
					if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
					}///
				}///
			}///
			//
			const FString Type = FString(TEXT("."))+Prop.TypeToString();
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(Type)) {
					for (const auto &TypeMethod : IT.Value.Methods) {
						if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
						}///
					}///
				}///
			}///
			//
			const FString ExtType = FString(TEXT("."))+Prop.TypeToString()+FString(TEXT("Extensions"));
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(ExtType)) {
					for (const auto &TypeMethod : IT.Value.Methods) {
						if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
						}///
					}///
				}///
			}///
		}///
	}///
	//
	for (const auto &Method : ScriptInfo.Methods) {
		if (Method.Name.ToString().Equals(Super,ESearchCase::CaseSensitive)) {
			if (TypeInfoCache.Contains(Method.TypeToString())) {
				const auto &TypeInfo = TypeInfoCache.FindChecked(Method.TypeToString());
				//
				for (const auto &TypeMethod : TypeInfo.Methods) {
					if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
					}///
				}///
			}///
			//
			const FString Type = FString(TEXT("."))+Method.TypeToString();
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(Type)) {
					for (const auto &TypeMethod : IT.Value.Methods) {
						if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
						}///
					}///
				}///
			}///
			//
			const FString ExtType = FString(TEXT("."))+Method.TypeToString()+FString(TEXT("Extensions"));
			for (const auto &IT : TypeInfoCache) {
				if (IT.Key.EndsWith(ExtType)) {
					for (const auto &TypeMethod : IT.Value.Methods) {
						if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
							SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
						}///
					}///
				}///
			}///
		} else if (Method.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
			SuggestedMethodCache.Add(Super+TEXT(".")+Scope,Method); return Method;
		}///
	}///
	//
	const FString Type = FString(TEXT("."))+Super;
	for (const auto &IT : TypeInfoCache) {
		if (IT.Key.EndsWith(Type)) {
			for (const auto &TypeMethod : IT.Value.Methods) {
				if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
					SuggestedMethodCache.Add(Super+TEXT(".")+Scope,TypeMethod); return TypeMethod;
				}///
			}///
		}///
	}///
	//
	for (const auto &IT : TypeInfoCache) {
		if (IT.Key.Equals(Super,ESearchCase::CaseSensitive)) {
			for (const auto &Method : IT.Value.Methods) {
				if (Method.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
					SuggestedMethodCache.Add(Super+TEXT(".")+Scope,Method); return Method;
				}///
			}///
		}///
	}///
	//
	return VOID_Method;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FMonoNamespaceDefinition & IKCS_MonoAnalyzer::GetNamespaceInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member) {
	FString FullSpace{};
	FString Parent{};
	//
	for (int32 I=0; I<Tree.Num(); ++I) {
		Parent.Append(Tree[I]);
		//
		if (I<(Tree.Num()-1)) {
			Parent.AppendChar(TEXT('.'));
		}///
	}///
	//
	if (!Member.IsEmpty()) {
		FullSpace = (Parent+TEXT(".")+Member);
	} else {FullSpace=Parent;}
	//
	FullSpace.TrimStartAndEndInline();
	FullSpace.RemoveFromEnd(TEXT("."));
	//
	return GetNamespaceInfo(Script,FullSpace);
}

const FMonoClassDefinition & IKCS_MonoAnalyzer::GetClassInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member) {
	FString Parent{};
	//
	for (int32 I=0; I<Tree.Num(); ++I) {
		Parent.Append(Tree[I]);
		//
		if (I<(Tree.Num()-1)) {
			Parent.AppendChar(TEXT('.'));
		}///
	}///
	//
	for (const auto &IT : TypeInfoCache) {
		if (IT.Key.Equals(Parent+TEXT(".")+Member,ESearchCase::CaseSensitive)) {SuggestedClassCache.Add(Parent+TEXT(".")+Member,IT.Value); return IT.Value;} else
		if (IT.Key.EndsWith(Parent+TEXT(".")+Member,ESearchCase::CaseSensitive)) {SuggestedClassCache.Add(Parent+TEXT(".")+Member,IT.Value); return IT.Value;}
	}///
	//
	return GetClassInfo(Script,Parent,Member);
}

const FMonoFieldDefinition & IKCS_MonoAnalyzer::GetFieldInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member) {
	if (Member.Equals(TEXT("."))) {return VOID_Field;}
	if (Tree.Num()==0) {return VOID_Field;}
	//
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	const FString ScriptName = ScriptInfo.Name.ToString();
	//
	TArray<FString>Root;
	//
	FString Context{};
	FString Scope = Member;
	Scope.RemoveFromEnd(TEXT("."));
	{
		for (int32 I=0; I<Tree.Num(); ++I) {
			Context.Append(Tree[I]);
			if (I<(Tree.Num()-1)) {
				Context.AppendChar(TEXT('.'));
			}///
		}///
		//
		for (const auto &IT : TypeInfoCache) {
			if (IT.Key.Equals(Context)) {
				for (const auto &TypeField : IT.Value.Fields) {
					if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedFieldCache.Add(Scope,TypeField); return TypeField;
					}///
				}///
			} else if (IT.Key.Equals(FString(TEXT(CS_CORE_NAMESPACE))+TEXT(".")+Context)) {
				for (const auto &TypeField : IT.Value.Fields) {
					if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedFieldCache.Add(Scope,TypeField); return TypeField;
					}///
				}///
			} else if (IT.Key.Equals(FString(TEXT(CS_CORE_NAMESPACE))+TEXT(".Core")+TEXT(".")+Context)) {
				for (const auto &TypeField : IT.Value.Fields) {
					if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedFieldCache.Add(Scope,TypeField); return TypeField;
					}///
				}///
			} else if (IT.Key.EndsWith(TEXT(".")+Context)) {
				for (const auto &TypeField : IT.Value.Fields) {
					if (TypeField.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedFieldCache.Add(Scope,TypeField); return TypeField;
					}///
				}///
			}///
		}///
	}
	//
	for (int32 I=Tree.Num()-1; I>INDEX_NONE; --I) {
		Root.Add(Tree[I]);
	}///
	//
	Context.Empty();
	ScopedClass.Reset();
	ContextClass.Reset();
	//
	while (Root.Num()>(0)) {
		ContextClass = ScopedClass;
		//
		Context = Root.Pop();
		Context.RemoveFromEnd(TEXT("@"));
		//
		if (Context.Equals(ScriptName,ESearchCase::CaseSensitive)||Context.Equals(TEXT("this"),ESearchCase::CaseSensitive)) {
			ScopedClass = ScriptInfo; continue;
		}///
		//
		if (ContextClass.IsValid()) {
			for (const auto &Field : ContextClass.Fields) {
				if (Field.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Field.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Field.TypeToString();
					//
					if (TypeInfoCache.Contains(Field.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Field.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Prop : ContextClass.Props) {
				if (Prop.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Prop.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Prop.TypeToString();
					//
					if (TypeInfoCache.Contains(Prop.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Prop.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Method : ContextClass.Methods) {
				if (Method.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Method.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Method.TypeToString();
					//
					if (TypeInfoCache.Contains(Method.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Method.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
		} else {
			for (const auto &Field : ScriptInfo.Fields) {
				if (Field.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Field.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Field.TypeToString();
					//
					if (TypeInfoCache.Contains(Field.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Field.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Prop : ScriptInfo.Props) {
				if (Prop.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Prop.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Prop.TypeToString();
					//
					if (TypeInfoCache.Contains(Prop.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Prop.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Method : ScriptInfo.Methods) {
				if (Method.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Method.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Method.TypeToString();
					//
					if (TypeInfoCache.Contains(Method.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Method.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
		}///
		//
		LOOP:{continue;}
	}///
	//
	if (ScopedClass.IsValid()) {
		for (const auto &Field : ScopedClass.Fields) {
			if (Field.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				SuggestedFieldCache.Add(Scope,Field); return Field;
			}///
		}///
	}///
	//
	return VOID_Field;
}

const FMonoFieldDefinition & IKCS_MonoAnalyzer::GetPropertyInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member) {
	if (Member.Equals(TEXT("."))) {return VOID_Prop;}
	if (Tree.Num()==0) {return VOID_Prop;}
	//
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	const FString ScriptName = ScriptInfo.Name.ToString();
	//
	TArray<FString>Root;
	FString Context{};
	//
	FString Scope = Member;
	Scope.RemoveFromEnd(TEXT("."));
	{
		for (int32 I=0; I<Tree.Num(); ++I) {
			Context.Append(Tree[I]);
			if (I<(Tree.Num()-1)) {
				Context.AppendChar(TEXT('.'));
			}///
		}///
		//
		for (const auto &IT : TypeInfoCache) {
			if (IT.Key.Equals(Context)) {
				for (const auto &TypeProp : IT.Value.Props) {
					if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedPropertyCache.Add(Scope,TypeProp); return TypeProp;
					}///
				}///
			} else if (IT.Key.Equals(FString(TEXT(CS_CORE_NAMESPACE))+TEXT(".")+Context)) {
				for (const auto &TypeProp : IT.Value.Props) {
					if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedPropertyCache.Add(Scope,TypeProp); return TypeProp;
					}///
				}///
			} else if (IT.Key.Equals(FString(TEXT(CS_CORE_NAMESPACE))+TEXT(".Core")+TEXT(".")+Context)) {
				for (const auto &TypeProp : IT.Value.Props) {
					if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedPropertyCache.Add(Scope,TypeProp); return TypeProp;
					}///
				}///
			} else if (IT.Key.EndsWith(TEXT(".")+Context)) {
				for (const auto &TypeProp : IT.Value.Props) {
					if (TypeProp.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedPropertyCache.Add(Scope,TypeProp); return TypeProp;
					}///
				}///
			}///
		}///
	}
	//
	for (int32 I=Tree.Num()-1; I>INDEX_NONE; --I) {
		Root.Add(Tree[I]);
	}///
	//
	Context.Empty();
	ScopedClass.Reset();
	ContextClass.Reset();
	//
	while (Root.Num()>(0)) {
		ContextClass = ScopedClass;
		//
		Context = Root.Pop();
		Context.RemoveFromEnd(TEXT("@"));
		//
		if (Context.Equals(ScriptName,ESearchCase::CaseSensitive)||Context.Equals(TEXT("this"),ESearchCase::CaseSensitive)) {
			ScopedClass = ScriptInfo; continue;
		}///
		//
		if (ContextClass.IsValid()) {
			for (const auto &Field : ContextClass.Fields) {
				if (Field.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Field.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Field.TypeToString();
					//
					if (TypeInfoCache.Contains(Field.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Field.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Prop : ContextClass.Props) {
				if (Prop.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Prop.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Prop.TypeToString();
					//
					if (TypeInfoCache.Contains(Prop.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Prop.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Method : ContextClass.Methods) {
				if (Method.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Method.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Method.TypeToString();
					//
					if (TypeInfoCache.Contains(Method.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Method.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
		} else {
			for (const auto &Field : ScriptInfo.Fields) {
				if (Field.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Field.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Field.TypeToString();
					//
					if (TypeInfoCache.Contains(Field.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Field.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Prop : ScriptInfo.Props) {
				if (Prop.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Prop.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Prop.TypeToString();
					//
					if (TypeInfoCache.Contains(Prop.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Prop.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Method : ScriptInfo.Methods) {
				if (Method.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Method.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Method.TypeToString();
					//
					if (TypeInfoCache.Contains(Method.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Method.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
		}///
		//
		LOOP:{continue;}
	}///
	//
	if (ScopedClass.IsValid()) {
		for (const auto &Prop : ScopedClass.Props) {
			if (Prop.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				SuggestedPropertyCache.Add(Scope,Prop); return Prop;
			}///
		}///
	}///
	//
	return VOID_Prop;
}

const FMonoMethodDefinition & IKCS_MonoAnalyzer::GetMethodInfo(UMagicNodeSharpSource* const &Script, const TArray<FString>&Tree, const FString &Member) {
	if (Tree.Num()==0) {FString None=Member; None.RemoveFromEnd(TEXT("@")); return GetMethodInfo(Script,None);}
	//
	const FMonoClassDefinition &ScriptInfo = Script->GetClassDefinition();
	const FString ScriptName = ScriptInfo.Name.ToString();
	//
	TArray<FString>Root;
	FString Context{};
	//
	FString Scope = Member;
	Scope.RemoveFromEnd(TEXT("@"));
	{
		for (int32 I=0; I<Tree.Num(); ++I) {
			Context.Append(Tree[I]);
			if (I<(Tree.Num()-1)) {
				Context.AppendChar(TEXT('.'));
			}///
		}///
		//
		for (const auto &IT : TypeInfoCache) {
			if (IT.Key.Equals(Context)) {
				for (const auto &TypeMethod : IT.Value.Methods) {
					if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedMethodCache.Add(Scope,TypeMethod); return TypeMethod;
					}///
				}///
			} else if (IT.Key.Equals(FString(TEXT(CS_CORE_NAMESPACE))+TEXT(".")+Context)) {
				for (const auto &TypeMethod : IT.Value.Methods) {
					if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedMethodCache.Add(Scope,TypeMethod); return TypeMethod;
					}///
				}///
			} else if (IT.Key.Equals(FString(TEXT(CS_CORE_NAMESPACE))+TEXT(".Core")+TEXT(".")+Context)) {
				for (const auto &TypeMethod : IT.Value.Methods) {
					if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedMethodCache.Add(Scope,TypeMethod); return TypeMethod;
					}///
				}///
			} else if (IT.Key.EndsWith(TEXT(".")+Context)) {
				for (const auto &TypeMethod : IT.Value.Methods) {
					if (TypeMethod.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
						SuggestedMethodCache.Add(Scope,TypeMethod); return TypeMethod;
					}///
				}///
			}///
		}///
	}
	//
	for (int32 I=Tree.Num()-1; I>INDEX_NONE; --I) {
		Root.Add(Tree[I]);
	}///
	//
	Context.Empty();
	ScopedClass.Reset();
	ContextClass.Reset();
	//
	while (Root.Num()>(0)) {
		ContextClass = ScopedClass;
		//
		Context = Root.Pop();
		Context.RemoveFromEnd(TEXT("@"));
		//
		if (Context.Equals(ScriptName,ESearchCase::CaseSensitive)||Context.Equals(TEXT("this"),ESearchCase::CaseSensitive)) {
			ScopedClass = ScriptInfo; continue;
		}///
		//
		if (ContextClass.IsValid()) {
			for (const auto &Field : ContextClass.Fields) {
				if (Field.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Field.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Field.TypeToString();
					//
					if (TypeInfoCache.Contains(Field.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Field.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Prop : ContextClass.Props) {
				if (Prop.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Prop.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Prop.TypeToString();
					//
					if (TypeInfoCache.Contains(Prop.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Prop.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Method : ContextClass.Methods) {
				if (Method.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Method.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Method.TypeToString();
					//
					if (TypeInfoCache.Contains(Method.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Method.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
		} else {
			for (const auto &Field : ScriptInfo.Fields) {
				if (Field.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Field.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Field.TypeToString();
					//
					if (TypeInfoCache.Contains(Field.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Field.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Prop : ScriptInfo.Props) {
				if (Prop.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Prop.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Prop.TypeToString();
					//
					if (TypeInfoCache.Contains(Prop.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Prop.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
			//
			for (const auto &Method : ScriptInfo.Methods) {
				if (Method.Name.ToString().Equals(Context,ESearchCase::CaseSensitive)) {
					const FString ExtType = FString(TEXT("."))+Method.TypeToString()+FString(TEXT("Extensions"));
					const FString Type = FString(TEXT("."))+Method.TypeToString();
					//
					if (TypeInfoCache.Contains(Method.TypeToString())) {
						ScopedClass = TypeInfoCache.FindChecked(Method.TypeToString()); {goto LOOP;}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(Type)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
					//
					for (const auto &IT : TypeInfoCache) {
						if (IT.Key.EndsWith(ExtType)) {ScopedClass=IT.Value; {goto LOOP;}}
					}///
				}///
			}///
		}///
		//
		LOOP:{continue;}
	}///
	//
	if (ScopedClass.IsValid()) {
		for (const auto &Method : ScopedClass.Methods) {
			if (Method.Name.ToString().Equals(Scope,ESearchCase::CaseSensitive)) {
				SuggestedMethodCache.Add(Scope,Method); return Method;
			}///
		}///
	}///
	//
	return VOID_Method;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::GetDataTypeFromMonoType(MonoType* ManagedType, FMonoFieldDefinition &OutDefinition) {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	OutDefinition.SubType = EMonoDataType::Unknown;
	OutDefinition.ListType = EMonoListType::None;
	//
	switch (mono_type_get_type(ManagedType)) {
		case MONO_TYPE_BOOLEAN:
		{
			OutDefinition.DataType = EMonoDataType::Bool;
		} break;
		//
		case MONO_TYPE_U1:
		{
			OutDefinition.DataType = EMonoDataType::Byte;
		} break;
		//
		case MONO_TYPE_I4:
		{
			OutDefinition.DataType = EMonoDataType::Int;
		} break;
		//
		case MONO_TYPE_I8:
		{
			OutDefinition.DataType = EMonoDataType::Int64;
		} break;
		//
		case MONO_TYPE_R4:
		{
			OutDefinition.DataType = EMonoDataType::Float;
		} break;
		//
		case MONO_TYPE_STRING:
		{
			//OutDefinition.DataType = EMonoDataType::String;
			OutDefinition.DataType = EMonoDataType::Unknown;
		} break;
		//
		case MONO_TYPE_VALUETYPE:
		{
			char* TypeName = mono_type_get_name(ManagedType);
			//
			if (strcmp(TypeName,"Unreal.Strand")==0)		{OutDefinition.DataType=EMonoDataType::String;}
			if (strcmp(TypeName,"Unreal.Name")==0)			{OutDefinition.DataType=EMonoDataType::Name;}
			if (strcmp(TypeName,"Unreal.Text")==0)			{OutDefinition.DataType=EMonoDataType::Text;}
			if (strcmp(TypeName,"Unreal.Vector2D")==0)		{OutDefinition.DataType=EMonoDataType::Vector2D;}
			if (strcmp(TypeName,"Unreal.Vector3D")==0)		{OutDefinition.DataType=EMonoDataType::Vector3D;}
			if (strcmp(TypeName,"Unreal.Rotator")==0)		{OutDefinition.DataType=EMonoDataType::Rotator;}
			if (strcmp(TypeName,"Unreal.Color")==0)			{OutDefinition.DataType=EMonoDataType::Color;}
			if (strcmp(TypeName,"Unreal.Transform")==0)		{OutDefinition.DataType=EMonoDataType::Transform;}
			if (strcmp(TypeName,"Unreal.ClassPtr")==0)		{OutDefinition.DataType=EMonoDataType::Class;}
			if (strcmp(TypeName,"Unreal.ObjectPtr")==0)		{OutDefinition.DataType=EMonoDataType::Object;}
			if (strcmp(TypeName,"Unreal.ActorPtr")==0)		{OutDefinition.DataType=EMonoDataType::Actor;}
			if (strcmp(TypeName,"Unreal.ComponentPtr")==0)	{OutDefinition.DataType=EMonoDataType::Component;}
			//
			if (strcmp(TypeName,"Unreal.IArray")==0) {
				OutDefinition.DataType = EMonoDataType::Unknown;
				OutDefinition.ListType = EMonoListType::Array;
				OutDefinition.SubType = EMonoDataType::Void;
			}///
			//
			if (strcmp(TypeName,"Unreal.ISet")==0) {
				OutDefinition.ListType = EMonoListType::Set;
				OutDefinition.SubType = EMonoDataType::Void;
				OutDefinition.DataType = EMonoDataType::Unknown;
			}///
			//
			if (strcmp(TypeName,"Unreal.IMap")==0) {
				OutDefinition.ListType = EMonoListType::Map;
				OutDefinition.SubType = EMonoDataType::Unknown;
				OutDefinition.DataType = EMonoDataType::Unknown;
			}///
		} break;
		//
		case MONO_TYPE_ARRAY:
		case MONO_TYPE_SZARRAY:
		{
			if (MonoArrayType*ArrayType=mono_type_get_array_type(ManagedType)) {
				MonoType* InnerType = mono_class_get_type(ArrayType->eklass);
				OutDefinition.ListType = EMonoListType::Array;
				OutDefinition.SubType = EMonoDataType::Void;
				//
				switch (mono_type_get_type(InnerType)) {
					case MONO_TYPE_BOOLEAN:
					{
						OutDefinition.DataType = EMonoDataType::Bool;
					} break;
					//
					case MONO_TYPE_U1:
					{
						OutDefinition.DataType = EMonoDataType::Byte;
					} break;
					//
					case MONO_TYPE_I4:
					{
						OutDefinition.DataType = EMonoDataType::Int;
					} break;
					//
					case MONO_TYPE_I8:
					{
						OutDefinition.DataType = EMonoDataType::Int64;
					} break;
					//
					case MONO_TYPE_R4:
					{
						OutDefinition.DataType = EMonoDataType::Float;
					} break;
					//
					case MONO_TYPE_STRING:
					{
						//OutDefinition.DataType = EMonoDataType::String;
						OutDefinition.DataType = EMonoDataType::Unknown;
					} break;
					//
					case MONO_TYPE_VALUETYPE:
					{
						char* TypeName = mono_type_get_name(InnerType);
						//
						if (strcmp(TypeName,"Unreal.Strand")==0)		{OutDefinition.DataType=EMonoDataType::String;}
						if (strcmp(TypeName,"Unreal.Name")==0)			{OutDefinition.DataType=EMonoDataType::Name;}
						if (strcmp(TypeName,"Unreal.Text")==0)			{OutDefinition.DataType=EMonoDataType::Text;}
						if (strcmp(TypeName,"Unreal.Vector2D")==0)		{OutDefinition.DataType=EMonoDataType::Vector2D;}
						if (strcmp(TypeName,"Unreal.Vector3D")==0)		{OutDefinition.DataType=EMonoDataType::Vector3D;}
						if (strcmp(TypeName,"Unreal.Rotator")==0)		{OutDefinition.DataType=EMonoDataType::Rotator;}
						if (strcmp(TypeName,"Unreal.Color")==0)			{OutDefinition.DataType=EMonoDataType::Color;}
						if (strcmp(TypeName,"Unreal.Transform")==0)		{OutDefinition.DataType=EMonoDataType::Transform;}
						if (strcmp(TypeName,"Unreal.ClassPtr")==0)		{OutDefinition.DataType=EMonoDataType::Class;}
						if (strcmp(TypeName,"Unreal.ObjectPtr")==0)		{OutDefinition.DataType=EMonoDataType::Object;}
						if (strcmp(TypeName,"Unreal.ActorPtr")==0)		{OutDefinition.DataType=EMonoDataType::Actor;}
						if (strcmp(TypeName,"Unreal.ComponentPtr")==0)	{OutDefinition.DataType=EMonoDataType::Component;}
					} break;
					//
					default:
					{
						OutDefinition.DataType = EMonoDataType::Unknown;
					} break;
				}///
			}///
		} break;
		//
		///case MONO_TYPE_CLASS:
		///case MONO_TYPE_OBJECT:
		default:
		{
			OutDefinition.DataType = EMonoDataType::Unknown;
			OutDefinition.ListType = EMonoListType::None;
		} break;
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FMonoNamespaceDefinition & IKCS_MonoAnalyzer::GetVoidNamespace() {
	return VOID_Namespace;
}

const FMonoClassDefinition & IKCS_MonoAnalyzer::GetVoidClass() {
	return VOID_Class;
}

const FMonoFieldDefinition & IKCS_MonoAnalyzer::GetVoidField() {
	return VOID_Field;
}

const FMonoFieldDefinition & IKCS_MonoAnalyzer::GetVoidProperty() {
	return VOID_Prop;
}

const FMonoMethodDefinition & IKCS_MonoAnalyzer::GetVoidMethod() {
	return VOID_Method;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MonoClass* IKCS_MonoAnalyzer::GetClassFromAlias(const FString &AliasName) {
	if (AliasName.Equals("void",ESearchCase::CaseSensitive)) {return mono_get_void_class();}
	if (AliasName.Equals("byte",ESearchCase::CaseSensitive)) {return mono_get_byte_class();}
	if (AliasName.Equals("char",ESearchCase::CaseSensitive)) {return mono_get_char_class();}
	if (AliasName.Equals("int",ESearchCase::CaseSensitive)) {return mono_get_int32_class();}
	if (AliasName.Equals("long",ESearchCase::CaseSensitive)) {return mono_get_int64_class();}
	if (AliasName.Equals("short",ESearchCase::CaseSensitive)) {return mono_get_int16_class();}
	if (AliasName.Equals("uint",ESearchCase::CaseSensitive)) {return mono_get_uint32_class();}
	if (AliasName.Equals("sbyte",ESearchCase::CaseSensitive)) {return mono_get_sbyte_class();}
	if (AliasName.Equals("ulong",ESearchCase::CaseSensitive)) {return mono_get_uint64_class();}
	if (AliasName.Equals("float",ESearchCase::CaseSensitive)) {return mono_get_single_class();}
	if (AliasName.Equals("bool",ESearchCase::CaseSensitive)) {return mono_get_boolean_class();}
	if (AliasName.Equals("double",ESearchCase::CaseSensitive)) {return mono_get_double_class();}
	if (AliasName.Equals("ushort",ESearchCase::CaseSensitive)) {return mono_get_uint16_class();}
	if (AliasName.Equals("string",ESearchCase::CaseSensitive)) {return mono_get_string_class();}
	if (AliasName.Equals("object",ESearchCase::CaseSensitive)) {return mono_get_object_class();}
	if (AliasName.Equals("IntPtr",ESearchCase::CaseSensitive)) {return mono_get_intptr_class();}
	if (AliasName.Equals("dynamic",ESearchCase::CaseSensitive)) {return mono_get_object_class();}
	if (AliasName.Equals("decimal",ESearchCase::CaseSensitive)) {return mono_class_from_name_case(mono_get_corlib(),"System","Decimal");}
	//
	return nullptr;
}

MonoClass* IKCS_MonoAnalyzer::GetClassFromType(MonoType* Type) {
	return mono_type_get_class(Type);
}

MonoType* IKCS_MonoAnalyzer::GetFieldType(MonoClassField* Field) {
	return mono_field_get_type(Field);
}

MonoType* IKCS_MonoAnalyzer::GetPropertyType(MonoProperty* Property) {
	MonoMethod* MethodGet = mono_property_get_get_method(Property);
	//
	if (MethodGet) {
		auto Image = mono_class_get_image(mono_property_get_parent(Property));
		auto MethodSign = mono_method_get_signature(MethodGet,Image,mono_method_get_token(MethodGet));
		//
		return mono_signature_get_return_type(MethodSign);
	}///
	//
	return nullptr;
}

MonoType* IKCS_MonoAnalyzer::GetMethodReturnType(MonoClass* ParentClass, MonoMethod* Method) {
	auto Image = mono_class_get_image(ParentClass);
	//
	auto MethodSign = mono_method_get_signature(Method,Image,mono_method_get_token(Method));
	//
	return mono_signature_get_return_type(MethodSign);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MonoClass* IKCS_MonoAnalyzer::FindClassByName(MonoImage* Image, const FString &ClassName) {
	if (MonoClass*AliasClass=GetClassFromAlias(ClassName)) {return AliasClass;}
	//
	const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
	int Rows = mono_table_info_get_rows(TbInfo);
	//
	//for (int I=0; I<Rows; ++I) {
	ParallelFor((int32)Rows,[&](int32 I) {
		uint32_t NMS[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
		//
		const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
		const char* _Name = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAME]);
		//
		if (strcmp(_Name,"<Module>")!=0) {
			const FString Name = StringCast<TCHAR>(_Name).Get();
			if (Name.Equals(ClassName,ESearchCase::CaseSensitive)) {
				return mono_class_from_name_case(Image,_Space,_Name);
			}///
		} return (MonoClass*)nullptr;
	});///
	//
	return nullptr;
}

MonoClass* IKCS_MonoAnalyzer::FindClassByName(MonoImage* Image, const FString &Space, const FString &ClassName) {
	if (MonoClass*AliasClass=GetClassFromAlias(ClassName)) {
		FString ClassSpace = StringCast<TCHAR>(mono_class_get_namespace(AliasClass)).Get();
		ClassSpace.Split(TEXT("`"),&ClassSpace,nullptr,ESearchCase::IgnoreCase);
		//
		if (Space.Equals(ClassSpace,ESearchCase::CaseSensitive)) {return AliasClass;}
	}///
	//
	const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
	int Rows = mono_table_info_get_rows(TbInfo);
	//
	//for (int I=0; I<Rows; ++I) {
	ParallelFor((int32)Rows,[&](int32 I) {
		uint32_t NMS[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
		//
		const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
		const char* _Name = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAME]);
		//
		if (strcmp(_Name,"<Module>")!=0) {
			const FString Rowspace = StringCast<TCHAR>(_Space).Get();
			const FString Name = StringCast<TCHAR>(_Name).Get();
			//
			if (Rowspace.Equals(Space,ESearchCase::CaseSensitive)) {
				if (Name.Equals(ClassName,ESearchCase::CaseSensitive)) {
					return mono_class_from_name_case(Image,_Space,_Name);
				}///
			}///
		} return (MonoClass*)nullptr;
	});///
	//
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKCS_MonoAnalyzer::CacheBaseClasses() {
	TypeInfoCache.Add(TEXT("void"),GetClassInfo(mono_get_void_class()));
	TypeInfoCache.Add(TEXT("byte"),GetClassInfo(mono_get_byte_class()));
	TypeInfoCache.Add(TEXT("char"),GetClassInfo(mono_get_char_class()));
	TypeInfoCache.Add(TEXT("int"),GetClassInfo(mono_get_int32_class()));
	TypeInfoCache.Add(TEXT("long"),GetClassInfo(mono_get_int64_class()));
	TypeInfoCache.Add(TEXT("short"),GetClassInfo(mono_get_int16_class()));
	TypeInfoCache.Add(TEXT("sbyte"),GetClassInfo(mono_get_sbyte_class()));
	TypeInfoCache.Add(TEXT("uint"),GetClassInfo(mono_get_uint32_class()));
	TypeInfoCache.Add(TEXT("ulong"),GetClassInfo(mono_get_uint64_class()));
	TypeInfoCache.Add(TEXT("float"),GetClassInfo(mono_get_single_class()));
	TypeInfoCache.Add(TEXT("bool"),GetClassInfo(mono_get_boolean_class()));
	TypeInfoCache.Add(TEXT("double"),GetClassInfo(mono_get_double_class()));
	TypeInfoCache.Add(TEXT("ushort"),GetClassInfo(mono_get_uint16_class()));
	TypeInfoCache.Add(TEXT("string"),GetClassInfo(mono_get_string_class()));
	TypeInfoCache.Add(TEXT("IntPtr"),GetClassInfo(mono_get_intptr_class()));
	TypeInfoCache.Add(TEXT("object"),GetClassMethodsInfo(mono_get_object_class()));
	TypeInfoCache.Add(TEXT("dynamic"),GetClassMethodsInfo(mono_get_object_class()));
	TypeInfoCache.Add(TEXT("decimal"),GetClassInfo(mono_class_from_name_case(mono_get_corlib(),"System","Decimal")));
	//
	TypeInfoCache.Add(TEXT("System.Void"),GetClassInfo(mono_get_void_class()));
	TypeInfoCache.Add(TEXT("System.Byte"),GetClassInfo(mono_get_byte_class()));
	TypeInfoCache.Add(TEXT("System.Char"),GetClassInfo(mono_get_char_class()));
	TypeInfoCache.Add(TEXT("System.Int32"),GetClassInfo(mono_get_int32_class()));
	TypeInfoCache.Add(TEXT("System.Int64"),GetClassInfo(mono_get_int64_class()));
	TypeInfoCache.Add(TEXT("System.Int16"),GetClassInfo(mono_get_int16_class()));
	TypeInfoCache.Add(TEXT("System.SByte"),GetClassInfo(mono_get_sbyte_class()));
	TypeInfoCache.Add(TEXT("System.UInt32"),GetClassInfo(mono_get_uint32_class()));
	TypeInfoCache.Add(TEXT("System.UInt64"),GetClassInfo(mono_get_uint64_class()));
	TypeInfoCache.Add(TEXT("System.Single"),GetClassInfo(mono_get_single_class()));
	TypeInfoCache.Add(TEXT("System.Double"),GetClassInfo(mono_get_double_class()));
	TypeInfoCache.Add(TEXT("System.UInt16"),GetClassInfo(mono_get_uint16_class()));
	TypeInfoCache.Add(TEXT("System.String"),GetClassInfo(mono_get_string_class()));
	TypeInfoCache.Add(TEXT("System.IntPtr"),GetClassInfo(mono_get_intptr_class()));
	TypeInfoCache.Add(TEXT("System.Boolean"),GetClassInfo(mono_get_boolean_class()));
	TypeInfoCache.Add(TEXT("System.Object"),GetClassMethodsInfo(mono_get_object_class()));
	TypeInfoCache.Add(TEXT("System.Decimal"),GetClassInfo(mono_class_from_name_case(mono_get_corlib(),"System","Decimal")));
}

void IKCS_MonoAnalyzer::CacheNamespaces(MonoImage* Image) {
	const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
	int Rows = mono_table_info_get_rows(TbInfo);
	//
	for (int I=0; I<Rows; ++I) {
		uint32_t NMS[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
		//
		const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
		const FString Space = StringCast<TCHAR>(_Space).Get();
		//
		if (strcmp(_Space,"<Module>")!=0) {
			FMonoNamespaceDefinition FullSpace{};
			FullSpace.Name = FName(*Space);
			SpaceInfoCache.AddUnique(FullSpace);
			//
			FString Root{}; FString Scope{};
			if (Space.Split(TEXT("."),&Root,&Scope,ESearchCase::IgnoreCase)) {
				FMonoNamespaceDefinition Parent{};
				FMonoNamespaceDefinition Child{};
				//
				Parent.Name = FName(*Root);
				Child.Name = FName(*Scope);
				Parent.Namespaces.Add(Scope);
				//
				SpaceInfoCache.AddUnique(Parent);
				SpaceInfoCache.AddUnique(Child);
			}///
		}///
	}///
}

void IKCS_MonoAnalyzer::CacheClassesFromImage(MonoImage* Image) {
	const MonoTableInfo* TbInfo = mono_image_get_table_info(Image,MONO_TABLE_TYPEDEF);
	int Rows = mono_table_info_get_rows(TbInfo);
	//
	for (int I=0; I<Rows; ++I) {
		uint32_t NMS[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(TbInfo,I,NMS,MONO_TYPEDEF_SIZE);
		//
		const char* _Space = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAMESPACE]);
		const char* _Name = mono_metadata_string_heap(Image,NMS[MONO_TYPEDEF_NAME]);
		//
		if (strcmp(_Name,"<Module>")!=0) {
			const FString Space = StringCast<TCHAR>(_Space).Get();
			const FString Name = StringCast<TCHAR>(_Name).Get();
			//
			FString Root{}; FString Scope{}; FString TypeName{};
			Space.Split(TEXT("."),&Root,&Scope,ESearchCase::IgnoreCase);
			//
			if (MonoClass*ManagedClass=mono_class_from_name_case(Image,_Space,_Name)) {
				if (MonoType*ClassType=mono_class_get_type(ManagedClass)) {
					const char* _TypeName = mono_type_get_name_full(ClassType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
					TypeName = StringCast<TCHAR>(_TypeName).Get();
					//
					TypeName.Split(TEXT("`"),&TypeName,nullptr,ESearchCase::IgnoreCase);
					if (Space.Equals(TEXT("System"),ESearchCase::CaseSensitive)||Root.Equals(TEXT("System"),ESearchCase::CaseSensitive)) {
						TypeInfoCache.Add(TypeName,GetClassMethodsInfo(ManagedClass));
					} else if (Space.Equals(TEXT("Microsoft"),ESearchCase::CaseSensitive)||Root.Equals(TEXT("Microsoft"),ESearchCase::CaseSensitive)) {
						TypeInfoCache.Add(TypeName,GetClassMethodsInfo(ManagedClass));
					} else if (Space.Equals(TEXT("Mono"),ESearchCase::CaseSensitive)||Root.Equals(TEXT("Mono"),ESearchCase::CaseSensitive)) {
						TypeInfoCache.Add(TypeName,GetClassMethodsInfo(ManagedClass));
					} else {TypeInfoCache.Add(TypeName,GetClassInfo(ManagedClass));}
				}///
			}///
			//
			if (TypeInfoCache.Contains(TypeName)) {
				const FString XName = Name+TEXT("Extensions");
				//
				for (int X=0; X<Rows; ++X) {
					uint32_t XNMS[MONO_TYPEDEF_SIZE];
					mono_metadata_decode_row(TbInfo,X,XNMS,MONO_TYPEDEF_SIZE);
					//
					const char* _ExtSpace = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAMESPACE]);
					const char* _ExtName = mono_metadata_string_heap(Image,XNMS[MONO_TYPEDEF_NAME]);
					//
					const FString ExtSpace = StringCast<TCHAR>(_ExtSpace).Get();
					const FString ExtName = StringCast<TCHAR>(_ExtName).Get();
					//
					if (!ExtName.Equals(XName,ESearchCase::CaseSensitive)) {continue;}
					if (MonoClass*ExtClass=mono_class_from_name_case(Image,_ExtSpace,_ExtName)) {
						if (MonoType*ExtType=mono_class_get_type(ExtClass)) {
							const char* _ExtTypeName = mono_type_get_name_full(ExtType,MonoTypeNameFormat::MONO_TYPE_NAME_FORMAT_FULL_NAME);
							FString ExtTypeName = StringCast<TCHAR>(_ExtTypeName).Get();
							//
							const FMonoClassDefinition ExtDefinition = GetClassMethodsInfo(ExtClass);
							FMonoClassDefinition &Registry = TypeInfoCache.FindChecked(TypeName);
							Registry.Methods.Append(ExtDefinition.Methods);
						}///
					}///
					//
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FString IKCS_MonoAnalyzer::GenerateGUID(const FString &Base) {
	if (Base.IsEmpty()){return TEXT("00000000-0000-0000-0000-000000000000");}
	//
	FString NewGUID{};
	for (const TCHAR &CH : Base) {
		if (CH!=TEXT('0')) {
			NewGUID.AppendChar(CH);
		continue;}
		//
		NewGUID.AppendChar(CS::GUIDs[FMath::RandRange(0,14)]);
	}///
	//
	return NewGUID;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////