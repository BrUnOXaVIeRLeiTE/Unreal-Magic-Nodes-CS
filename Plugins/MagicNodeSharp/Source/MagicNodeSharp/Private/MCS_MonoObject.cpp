//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MCS_MonoObject.h"
#include "MagicNodeSharp.h"

#include "UObject/UnrealType.h"
#include "Interfaces/IPluginManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::ThreadStart(const UPTRINT IntPtr) {
	CS_THREAD_ATTACH();
	//
	if (UMagicNodeSharp*Node=reinterpret_cast<UMagicNodeSharp*>(IntPtr)) {
		Node->OnStart();
	}///
}

void IMonoObject::ThreadUpdate(const UPTRINT IntPtr) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	if (!MonoCore.IsPlaying()) {return;}
	//
	if (UMagicNodeSharp*Node=reinterpret_cast<UMagicNodeSharp*>(IntPtr)) {
		while(Node->IsAsyncTask && Node->GetWorld() && (!Node->IsPendingKill())) {
			const float DeltaTime = Node->GetWorld()->GetDeltaSeconds();
			//
			const auto Result = Node->OnUpdate(DeltaTime);
			FPlatformProcess::Sleep(DeltaTime+SMALL_NUMBER);
			//
			if (Result!=0) {
				Node->Throw_MonoError(TEXT("C# Runtime exception ::    OnUpdate()"));
				Node->IsAsyncTask = false;
			break;}
		}///
		//
		IMonoObject::ThreadExit(IntPtr);
	}///
}

void IMonoObject::ThreadExit(const UPTRINT IntPtr) {
	if (UMagicNodeSharp*Node=reinterpret_cast<UMagicNodeSharp*>(IntPtr)) {
		Node->IsAsyncTask = false;
		Node->OnExit();
	}///
	//
	CS_THREAD_DETACH();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::SetupManagedHandle(UMagicNodeSharp* Node, MonoObject* ManagedNode) {
	if (ManagedNode==nullptr) {return;}
	if (Node==nullptr) {return;}
	//
	MonoClass* ManagedClass = mono_object_get_class(ManagedNode);
	MonoClassField* ClassField = nullptr; void* Itr=0;
	//
	while(ManagedClass!=nullptr) {
		ClassField = mono_class_get_fields(ManagedClass,&Itr);
		//
		if (ClassField==nullptr) {
			ManagedClass = mono_class_get_parent(ManagedClass);
			if (ManagedClass==mono_get_object_class()) {break;}
			Itr=0; continue;
		}///
		//
		const char* FieldName = mono_field_get_name(ClassField);
		void* IntPtr = reinterpret_cast<void*>(Node);
		//
		if (strcmp(FieldName,"NodePtr")==0) {
			mono_field_set_value(ManagedNode,ClassField,&IntPtr); break;
		}///
	}///
}

void IMonoObject::SetupManagedMethods(UMagicNodeSharp* Node, MonoObject* ManagedNode) {
	check(Node); check(ManagedNode);
	//
	MonoClass* ManagedClass = mono_object_get_class(ManagedNode);
	if (ManagedClass==mono_get_object_class()) {return;}
	//
	MonoMethod* Method = nullptr; void* Itr=0;
	while((Method=mono_class_get_methods(ManagedClass,&Itr)) != NULL) {
		auto Flags = mono_method_get_flags(Method,nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;
		if (Flags!=MONO_METHOD_ATTR_PUBLIC) {continue;}
		//
		const char* MethodName = mono_method_get_name(Method);
		if (strcmp(MethodName,"OnLoad")==0) {Node->MonoOnLoad=Method; continue;}
		if (strcmp(MethodName,"OnExit")==0) {Node->MonoOnExit=Method; continue;}
		if (strcmp(MethodName,"OnStart")==0) {Node->MonoOnStart=Method; continue;}
		if (strcmp(MethodName,"OnUpdate")==0) {Node->MonoOnUpdate=Method; continue;}
		if (strcmp(MethodName,"OnExecute")==0) {Node->MonoOnExecute=Method; continue;}
	}///
}

void IMonoObject::SetupThreadingAttribute(UMagicNodeSharp* Node, MonoClass* NodeClass) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	check(Node); check(NodeClass);
	//
	MonoClass* Attribute = nullptr;
	if (MonoCustomAttrInfo*AttrInfo=mono_custom_attrs_from_class(NodeClass)) {
		for (int32 I=0; I<AttrInfo->num_attrs; I++) {
			if (MonoClass*AttrClass=mono_method_get_class(AttrInfo->attrs[I].ctor)) {
				const char* AttrName = mono_class_get_name(AttrClass);
				if (strcmp(AttrName,"Async")==0){Attribute=AttrClass; break;}
			}///
		}///
		//
		if (Attribute) {
			MonoProperty*TaskProp = mono_class_get_property_from_name(Attribute,"Task");
			MonoObject* Attr = mono_custom_attrs_get_attr(AttrInfo,Attribute);
			MonoMethod* AttrGet = mono_property_get_get_method(TaskProp);
			//
			if (MonoObject*iCall=mono_runtime_invoke(AttrGet,Attr,NULL,NULL)) {
				uint8 Val = *(uint8*)mono_object_unbox(iCall);
				Node->MonoThread = (EMonoThread)Val;
			}///
			//
			Node->IsAsyncTask = true;
		}///
		//
		mono_custom_attrs_free(AttrInfo);
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IMonoObject::ValidateParentClass(MonoClass* ManagedClass) {
	MonoClass* Parent = mono_class_get_parent(ManagedClass);
	check(ManagedClass);
	//
	while(Parent) {
		const char* ParentName = mono_class_get_name(Parent);
		//
		if (strcmp(ParentName,CS_NODE_CLASS)==0) {return true;}
		Parent = mono_class_get_parent(Parent);
		//
		if (Parent==mono_get_object_class()) {break;}
	}///
	//
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FGuid IMonoObject::PtrToGuid(UMagicNodeSharp* Ptr) {
	FGuid Guid = FGuid::NewGuid();
	//
	if (Ptr==nullptr) {Guid.Invalidate();} else {
		UPTRINT IntPtr = reinterpret_cast<UPTRINT>(Ptr);
		//
		if (sizeof(UPTRINT)>4) {
			Guid[0] ^= (static_cast<uint64>(IntPtr)>>32);
		} Guid[1] ^= IntPtr & 0xFFFFFFFF;
	} return Guid;
}

UMagicNodeSharp* IMonoObject::GuidToPtr(const FGuid &Guid) {
	UPTRINT IntPtr = 0;
	//
	if (sizeof(UPTRINT)>4) {
		IntPtr = static_cast<UPTRINT>(static_cast<uint64>(Guid[0]) << 32);
	} IntPtr |= Guid[1] & 0xFFFFFFFF;
	//
	return reinterpret_cast<UMagicNodeSharp*>(IntPtr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::SET_MonoPropertyValue_Bool(UMagicNodeSharp* Node, const FName &Field, bool Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Bool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Bool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Bool::  Invalid Managed Object.")); return;}
	//
	void* Args[1]; Args[0] = &Input;
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Bool(UMagicNodeSharp* Node, const FName &Field, bool &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Bool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Bool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Bool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			Output = *(bool*)mono_object_unbox(iCall);
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Byte(UMagicNodeSharp* Node, const FName &Field, uint8 Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Byte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Byte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Byte::  Invalid Managed Object.")); return;}
	//
	void* Args[1]; Args[0] = &Input;
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Byte(UMagicNodeSharp* Node, const FName &Field, uint8 &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Byte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Byte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Byte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			Output = *(uint8*)mono_object_unbox(iCall);
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Int(UMagicNodeSharp* Node, const FName &Field, int32 Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Int::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Int::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Int::  Invalid Managed Object.")); return;}
	//
	void* Args[1]; Args[0] = &Input;
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Int(UMagicNodeSharp* Node, const FName &Field, int32 &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Int::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Int::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Int::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			Output = *(int32*)mono_object_unbox(iCall);
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Int64(UMagicNodeSharp* Node, const FName &Field, int64 Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Int64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Int64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Int64::  Invalid Managed Object.")); return;}
	//
	void* Args[1]; Args[0] = &Input;
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Int64(UMagicNodeSharp* Node, const FName &Field, int64 &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Int64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Int64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Int64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			Output = *(int64*)mono_object_unbox(iCall);
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Float(UMagicNodeSharp* Node, const FName &Field, float Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Float::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Float::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Float::  Invalid Managed Object.")); return;}
	//
	void* Args[1]; Args[0] = &Input;
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Float(UMagicNodeSharp* Node, const FName &Field, float &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Float::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Float::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Float::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			Output = *(float*)mono_object_unbox(iCall);
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_String(UMagicNodeSharp* Node, const FName &Field, const FString &Input) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_String::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_String::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_String::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoString String = FMonoString(Input,MonoCore.GetCoreDomain());
		void* Args[1]; Args[0] = &String;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_String(UMagicNodeSharp* Node, const FName &Field, FString &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_String::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_String::  No Context.")); return;}
	//
	MonoObject* Except = nullptr;
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_String::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoString Out = *(FMonoString*)mono_object_unbox(iCall);
			Output = Out.ToString();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Name(UMagicNodeSharp* Node, const FName &Field, const FName &Input) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Name::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Name::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Name::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoName Name = FMonoName(Input,MonoCore.GetCoreDomain());
		void* Args[1]; Args[0] = &Name;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Name(UMagicNodeSharp* Node, const FName &Field, FName &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Name::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Name::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Name::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoName Out = *(FMonoName*)mono_object_unbox(iCall);
			Output = Out.ToName();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Text(UMagicNodeSharp* Node, const FName &Field, const FText &Input) {
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Text::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Text::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Text::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoText Text = FMonoText(Input,MonoCore.GetCoreDomain());
		void* Args[1]; Args[0] = &Text;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Text(UMagicNodeSharp* Node, const FName &Field, FText &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Text::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Text::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Text::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoText Out = *(FMonoText*)mono_object_unbox(iCall);
			Output = Out.ToText();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Color(UMagicNodeSharp* Node, const FName &Field, const FColor &Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Color::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Color::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Color::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoColor Color = FMonoColor(Input);
		void* Args[1]; Args[0] = &Color;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Color(UMagicNodeSharp* Node, const FName &Field, FColor &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Color::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Color::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Color::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoColor Out = *(FMonoColor*)mono_object_unbox(iCall);
			Output = Out.ToColor();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Vector2D(UMagicNodeSharp* Node, const FName &Field, const FVector2D &Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Vector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Vector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Vector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoVector2D Vector = FMonoVector2D(Input);
		void* Args[1]; Args[0] = &Vector;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Vector2D(UMagicNodeSharp* Node, const FName &Field, FVector2D &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Vector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Vector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Vector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoVector2D Out = *(FMonoVector2D*)mono_object_unbox(iCall);
			Output = Out.ToVector2D();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Vector3D(UMagicNodeSharp* Node, const FName &Field, const FVector &Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Vector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Vector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Vector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoVector3D Vector = FMonoVector3D(Input);
		void* Args[1]; Args[0] = &Vector;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Vector3D(UMagicNodeSharp* Node, const FName &Field, FVector &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Vector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Vector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Vector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoVector3D Out = *(FMonoVector3D*)mono_object_unbox(iCall);
			Output = Out.ToVector3D();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Rotator(UMagicNodeSharp* Node, const FName &Field, const FRotator &Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Rotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Rotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Rotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoRotator Rotator = FMonoRotator(Input);
		void* Args[1]; Args[0] = &Rotator;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Rotator(UMagicNodeSharp* Node, const FName &Field, FRotator &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Rotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Rotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Rotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoRotator Out = *(FMonoRotator*)mono_object_unbox(iCall);
			Output = Out.ToRotator();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Transform(UMagicNodeSharp* Node, const FName &Field, const FTransform &Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Transform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Transform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Transform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoTransform Transform = FMonoTransform(Input);
		void* Args[1]; Args[0] = &Transform;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Transform(UMagicNodeSharp* Node, const FName &Field, FTransform &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Transform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Transform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Transform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoTransform Out = *(FMonoTransform*)mono_object_unbox(iCall);
			Output = Out.ToTransform();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Class(UMagicNodeSharp* Node, const FName &Field, UClass* Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Class::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Class::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Class::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoClass Class = FMonoClass(Input);
		void* Args[1]; Args[0] = &Class;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Class(UMagicNodeSharp* Node, const FName &Field, UClass* &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Class::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Class::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Class::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoClass Out = *(FMonoClass*)mono_object_unbox(iCall);
			Output = Out.ToClass();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Object(UMagicNodeSharp* Node, const FName &Field, UObject* Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Object::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Object::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Object::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoObject Object = FMonoObject(Input);
		void* Args[1]; Args[0] = &Object;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Object(UMagicNodeSharp* Node, const FName &Field, UObject* &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Object::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Object::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Object::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoObject Out = *(FMonoObject*)mono_object_unbox(iCall);
			Output = Out.ToObject();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Actor(UMagicNodeSharp* Node, const FName &Field, AActor* Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Actor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Actor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Actor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoActor Actor = FMonoActor(Input);
		void* Args[1]; Args[0] = &Actor;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Actor(UMagicNodeSharp* Node, const FName &Field, AActor* &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Actor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Actor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Actor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoActor Out = *(FMonoActor*)mono_object_unbox(iCall);
			Output = Out.ToActor();
		}///
	}///
}

void IMonoObject::SET_MonoPropertyValue_Component(UMagicNodeSharp* Node, const FName &Field, UActorComponent* Input) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Component::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Component::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoPropertyValue_Component::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		FMonoComponent Component = FMonoComponent(Input);
		void* Args[1]; Args[0] = &Component;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(Property)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoPropertyValue_Component(UMagicNodeSharp* Node, const FName &Field, UActorComponent* &Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Component::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Component::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoPropertyValue_Component::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*Property=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(Property);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoComponent Out = *(FMonoComponent*)mono_object_unbox(iCall);
			Output = Out.ToComponent();
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::SET_MonoArrayValue(UMagicNodeSharp* Node, const FName &Field, FArrayProperty* Property, void* Address) {
	if (!Property->IsValidLowLevel()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoArrayValue::  Invalid Property.")); return;}
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoArrayValue::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoArrayValue::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoArrayValue::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		FMonoArray Array = FMonoArray(Property,Address);
		void* Args[1]; Args[0] = &Array;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(MonoProp)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Bool(UMagicNodeSharp* Node, const FName &Field, TArray<bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Bool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Bool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Bool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (Property->Inner->IsA(FBoolProperty::StaticClass())) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = CastFieldChecked<FBoolProperty>(Property->Inner)->GetPropertyValue(Array.GetRawPtr(I));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Byte(UMagicNodeSharp* Node, const FName &Field, TArray<uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Byte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Byte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Byte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (Property->Inner->IsA(FByteProperty::StaticClass())) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = CastFieldChecked<FByteProperty>(Property->Inner)->GetPropertyValue(Array.GetRawPtr(I));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Int(UMagicNodeSharp* Node, const FName &Field, TArray<int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Int::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Int::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Int::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (Property->Inner->IsA(FIntProperty::StaticClass())) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = CastFieldChecked<FIntProperty>(Property->Inner)->GetPropertyValue(Array.GetRawPtr(I));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Int64(UMagicNodeSharp* Node, const FName &Field, TArray<int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Int64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Int64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Int64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (Property->Inner->IsA(FInt64Property::StaticClass())) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = CastFieldChecked<FInt64Property>(Property->Inner)->GetPropertyValue(Array.GetRawPtr(I));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Float(UMagicNodeSharp* Node, const FName &Field, TArray<float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Float::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Float::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Float::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (Property->Inner->IsA(FFloatProperty::StaticClass())) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = CastFieldChecked<FFloatProperty>(Property->Inner)->GetPropertyValue(Array.GetRawPtr(I));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_String(UMagicNodeSharp* Node, const FName &Field, TArray<FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_String::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_String::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_String::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (Property->Inner->IsA(FStrProperty::StaticClass())) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = CastFieldChecked<FStrProperty>(Property->Inner)->GetPropertyValue(Array.GetRawPtr(I));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Name(UMagicNodeSharp* Node, const FName &Field, TArray<FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Name::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Name::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Name::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (Property->Inner->IsA(FNameProperty::StaticClass())) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = CastFieldChecked<FNameProperty>(Property->Inner)->GetPropertyValue(Array.GetRawPtr(I));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Text(UMagicNodeSharp* Node, const FName &Field, TArray<FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Text::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Text::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Text::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (Property->Inner->IsA(FTextProperty::StaticClass())) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = CastFieldChecked<FTextProperty>(Property->Inner)->GetPropertyValue(Array.GetRawPtr(I));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Color(UMagicNodeSharp* Node, const FName &Field, TArray<FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Color::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Color::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Color::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->Inner)) {
					if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
						FScriptArrayHelper Array(Property,Out.ValueAddr);
						Output.SetNum(Array.Num());
						//
						for (int32 I=0; I<Array.Num(); ++I) {
							const uint8* ValuePtr = Array.GetRawPtr(I);
							FColor ValueCopy;
							//
							InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
							Output[I] = ValueCopy;
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Vector2D(UMagicNodeSharp* Node, const FName &Field, TArray<FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Vector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Vector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Vector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->Inner)) {
					if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptArrayHelper Array(Property,Out.ValueAddr);
						Output.SetNum(Array.Num());
						//
						for (int32 I=0; I<Array.Num(); ++I) {
							const uint8* ValuePtr = Array.GetRawPtr(I);
							FVector2D ValueCopy;
							//
							InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
							Output[I] = ValueCopy;
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Vector3D(UMagicNodeSharp* Node, const FName &Field, TArray<FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Vector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Vector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Vector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->Inner)) {
					if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
						FScriptArrayHelper Array(Property,Out.ValueAddr);
						Output.SetNum(Array.Num());
						//
						for (int32 I=0; I<Array.Num(); ++I) {
							const uint8* ValuePtr = Array.GetRawPtr(I);
							FVector ValueCopy;
							//
							InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
							Output[I] = ValueCopy;
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Rotator(UMagicNodeSharp* Node, const FName &Field, TArray<FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Rotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Rotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Rotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->Inner)) {
					if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
						FScriptArrayHelper Array(Property,Out.ValueAddr);
						Output.SetNum(Array.Num());
						//
						for (int32 I=0; I<Array.Num(); ++I) {
							const uint8* ValuePtr = Array.GetRawPtr(I);
							FRotator ValueCopy;
							//
							InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
							Output[I] = ValueCopy;
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Transform(UMagicNodeSharp* Node, const FName &Field, TArray<FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Transform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Transform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Transform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->Inner)) {
					if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
						FScriptArrayHelper Array(Property,Out.ValueAddr);
						Output.SetNum(Array.Num());
						//
						for (int32 I=0; I<Array.Num(); ++I) {
							const uint8* ValuePtr = Array.GetRawPtr(I);
							FTransform ValueCopy;
							//
							InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
							Output[I] = ValueCopy;
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Class(UMagicNodeSharp* Node, const FName &Field, TArray<UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Class::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Class::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Class::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (FClassProperty*Inner=CastField<FClassProperty>(Property->Inner)) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = Cast<UClass>(Inner->GetObjectPropertyValue(Array.GetRawPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Object(UMagicNodeSharp* Node, const FName &Field, TArray<UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Object::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Object::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Object::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = Inner->GetObjectPropertyValue(Array.GetRawPtr(I));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Actor(UMagicNodeSharp* Node, const FName &Field, TArray<AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Actor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Actor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Actor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = Cast<AActor>(Inner->GetObjectPropertyValue(Array.GetRawPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoArrayValue_Component(UMagicNodeSharp* Node, const FName &Field, TArray<UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Component::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Component::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoArrayValue_Component::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoArray Out = *(FMonoArray*)mono_object_unbox(iCall);
			//
			if (FArrayProperty*Property=reinterpret_cast<FArrayProperty*>(Out.Property)) {
				if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->Inner)) {
					FScriptArrayHelper Array(Property,Out.ValueAddr);
					Output.SetNum(Array.Num());
					//
					for (int32 I=0; I<Array.Num(); ++I) {
						Output[I] = Cast<UActorComponent>(Inner->GetObjectPropertyValue(Array.GetRawPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::SET_MonoSetValue(UMagicNodeSharp* Node, const FName &Field, FSetProperty* Property, void* Address) {
	if (!Property->IsValidLowLevel()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoSetValue::  Invalid Property.")); return;}
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoSetValue::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoSetValue::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoSetValue::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		FMonoSet Set = FMonoSet(Property,Address);
		void* Args[1]; Args[0] = &Set;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(MonoProp)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Byte(UMagicNodeSharp* Node, const FName &Field, TSet<uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Byte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Byte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Byte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (Property->ElementProp->IsA(FByteProperty::StaticClass())) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(CastFieldChecked<FByteProperty>(Property->ElementProp)->GetPropertyValue(Set.GetElementPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Int(UMagicNodeSharp* Node, const FName &Field, TSet<int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Int::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Int::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Int::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (Property->ElementProp->IsA(FIntProperty::StaticClass())) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(CastFieldChecked<FIntProperty>(Property->ElementProp)->GetPropertyValue(Set.GetElementPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Int64(UMagicNodeSharp* Node, const FName &Field, TSet<int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Int64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Int64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Int64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (Property->ElementProp->IsA(FInt64Property::StaticClass())) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(CastFieldChecked<FInt64Property>(Property->ElementProp)->GetPropertyValue(Set.GetElementPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Float(UMagicNodeSharp* Node, const FName &Field, TSet<float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Float::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Float::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Float::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (Property->ElementProp->IsA(FFloatProperty::StaticClass())) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(CastFieldChecked<FFloatProperty>(Property->ElementProp)->GetPropertyValue(Set.GetElementPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_String(UMagicNodeSharp* Node, const FName &Field, TSet<FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_String::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_String::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_String::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (Property->ElementProp->IsA(FStrProperty::StaticClass())) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(CastFieldChecked<FStrProperty>(Property->ElementProp)->GetPropertyValue(Set.GetElementPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Name(UMagicNodeSharp* Node, const FName &Field, TSet<FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Name::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Name::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Name::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (Property->ElementProp->IsA(FNameProperty::StaticClass())) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(CastFieldChecked<FNameProperty>(Property->ElementProp)->GetPropertyValue(Set.GetElementPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Color(UMagicNodeSharp* Node, const FName &Field, TSet<FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Color::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Color::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Color::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ElementProp)) {
					if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
						FScriptSetHelper Set(Property,Out.ValueAddr);
						Output.Reserve(Set.Num());
						//
						for (int32 I=0; I<Set.Num(); ++I) {
							const uint8* ValuePtr = Set.GetElementPtr(I);
							FColor ValueCopy;
							//
							InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
							Output.Add(ValueCopy);
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Vector2D(UMagicNodeSharp* Node, const FName &Field, TSet<FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Vector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Vector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Vector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ElementProp)) {
					if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptSetHelper Set(Property,Out.ValueAddr);
						Output.Reserve(Set.Num());
						//
						for (int32 I=0; I<Set.Num(); ++I) {
							const uint8* ValuePtr = Set.GetElementPtr(I);
							FVector2D ValueCopy;
							//
							InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
							Output.Add(ValueCopy);
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Vector3D(UMagicNodeSharp* Node, const FName &Field, TSet<FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Vector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Vector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Vector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ElementProp)) {
					if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
						FScriptSetHelper Set(Property,Out.ValueAddr);
						Output.Reserve(Set.Num());
						//
						for (int32 I=0; I<Set.Num(); ++I) {
							const uint8* ValuePtr = Set.GetElementPtr(I);
							FVector ValueCopy;
							//
							InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
							Output.Add(ValueCopy);
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Class(UMagicNodeSharp* Node, const FName &Field, TSet<UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Class::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Class::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Class::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (FClassProperty*Inner=CastField<FClassProperty>(Property->ElementProp)) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(Cast<UClass>(Inner->GetObjectPropertyValue(Set.GetElementPtr(I))));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Object(UMagicNodeSharp* Node, const FName &Field, TSet<UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Object::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Object::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Object::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(Inner->GetObjectPropertyValue(Set.GetElementPtr(I)));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Actor(UMagicNodeSharp* Node, const FName &Field, TSet<AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Actor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Actor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Actor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(Cast<AActor>(Inner->GetObjectPropertyValue(Set.GetElementPtr(I))));
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoSetValue_Component(UMagicNodeSharp* Node, const FName &Field, TSet<UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Component::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Component::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoSetValue_Component::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoSet Out = *(FMonoSet*)mono_object_unbox(iCall);
			//
			if (FSetProperty*Property=reinterpret_cast<FSetProperty*>(Out.Property)) {
				if (FObjectProperty*Inner=CastField<FObjectProperty>(Property->ElementProp)) {
					FScriptSetHelper Set(Property,Out.ValueAddr);
					Output.Reserve(Set.Num());
					//
					for (int32 I=0; I<Set.Num(); ++I) {
						Output.Add(Cast<UActorComponent>(Inner->GetObjectPropertyValue(Set.GetElementPtr(I))));
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::SET_MonoMapValue(UMagicNodeSharp* Node, const FName &Field, FMapProperty* Property, void* Address) {
	if (!Property->IsValidLowLevel()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoMapValue::  Invalid Property.")); return;}
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoMapValue::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoMapValue::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__SET_MonoMapValue::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		FMonoMap Map = FMonoMap(Property,Address);
		void* Args[1]; Args[0] = &Map;
		//
		if (MonoMethod*SetMethod=mono_property_get_set_method(MonoProp)) {
			mono_runtime_invoke(SetMethod,ManagedObject,Args,NULL);
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_ByteBool(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FByteProperty::StaticClass()))&&(Property->ValueProp->IsA(FBoolProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteByte(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FByteProperty::StaticClass()))&&(Property->ValueProp->IsA(FByteProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteInt(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FByteProperty::StaticClass()))&&(Property->ValueProp->IsA(FIntProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteInt64(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FByteProperty::StaticClass()))&&(Property->ValueProp->IsA(FInt64Property::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteFloat(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FByteProperty::StaticClass()))&&(Property->ValueProp->IsA(FFloatProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteString(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FByteProperty::StaticClass()))&&(Property->ValueProp->IsA(FStrProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteName(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FByteProperty::StaticClass()))&&(Property->ValueProp->IsA(FNameProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteText(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FByteProperty::StaticClass()))&&(Property->ValueProp->IsA(FTextProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteColor(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FByteProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FByteProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FByteProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteRotator(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FByteProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteTransform(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FByteProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteClass(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FByteProperty::StaticClass())) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteObject(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FByteProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteActor(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FByteProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ByteComponent(UMagicNodeSharp* Node, const FName &Field, TMap<uint8,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ByteComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FByteProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FByteProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_IntBool(UMagicNodeSharp* Node, const FName &Field, TMap<int32,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FIntProperty::StaticClass()))&&(Property->ValueProp->IsA(FBoolProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntByte(UMagicNodeSharp* Node, const FName &Field, TMap<int32,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FIntProperty::StaticClass()))&&(Property->ValueProp->IsA(FByteProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntInt(UMagicNodeSharp* Node, const FName &Field, TMap<int32,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FIntProperty::StaticClass()))&&(Property->ValueProp->IsA(FIntProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntInt64(UMagicNodeSharp* Node, const FName &Field, TMap<int32,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FIntProperty::StaticClass()))&&(Property->ValueProp->IsA(FInt64Property::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntFloat(UMagicNodeSharp* Node, const FName &Field, TMap<int32,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FIntProperty::StaticClass()))&&(Property->ValueProp->IsA(FFloatProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntString(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FIntProperty::StaticClass()))&&(Property->ValueProp->IsA(FStrProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntName(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FIntProperty::StaticClass()))&&(Property->ValueProp->IsA(FNameProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntText(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FIntProperty::StaticClass()))&&(Property->ValueProp->IsA(FTextProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntColor(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FIntProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FIntProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FIntProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntRotator(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FIntProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntTransform(UMagicNodeSharp* Node, const FName &Field, TMap<int32,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FIntProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntClass(UMagicNodeSharp* Node, const FName &Field, TMap<int32,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FIntProperty::StaticClass())) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntObject(UMagicNodeSharp* Node, const FName &Field, TMap<int32,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FIntProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntActor(UMagicNodeSharp* Node, const FName &Field, TMap<int32,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FIntProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_IntComponent(UMagicNodeSharp* Node, const FName &Field, TMap<int32,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_IntComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FIntProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FIntProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_Int64Bool(UMagicNodeSharp* Node, const FName &Field, TMap<int64,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Bool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Bool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Bool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FInt64Property::StaticClass()))&&(Property->ValueProp->IsA(FBoolProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Byte(UMagicNodeSharp* Node, const FName &Field, TMap<int64,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Byte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Byte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Byte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FInt64Property::StaticClass()))&&(Property->ValueProp->IsA(FByteProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Int(UMagicNodeSharp* Node, const FName &Field, TMap<int64,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Int::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Int::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Int::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FInt64Property::StaticClass()))&&(Property->ValueProp->IsA(FIntProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Int64(UMagicNodeSharp* Node, const FName &Field, TMap<int64,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Int64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Int64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Int64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FInt64Property::StaticClass()))&&(Property->ValueProp->IsA(FInt64Property::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Float(UMagicNodeSharp* Node, const FName &Field, TMap<int64,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Float::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Float::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Float::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FInt64Property::StaticClass()))&&(Property->ValueProp->IsA(FFloatProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64String(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64String::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64String::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64String::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FInt64Property::StaticClass()))&&(Property->ValueProp->IsA(FStrProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Name(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Name::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Name::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Name::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FInt64Property::StaticClass()))&&(Property->ValueProp->IsA(FNameProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Text(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Text::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Text::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Text::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FInt64Property::StaticClass()))&&(Property->ValueProp->IsA(FTextProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Color(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Color::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Color::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Color::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FInt64Property::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Vector2D(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Vector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Vector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Vector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FInt64Property::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Vector3D(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Vector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Vector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Vector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FInt64Property::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Rotator(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Rotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Rotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Rotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FInt64Property::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Transform(UMagicNodeSharp* Node, const FName &Field, TMap<int64,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Transform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Transform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Transform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FInt64Property::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Class(UMagicNodeSharp* Node, const FName &Field, TMap<int64,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Class::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Class::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Class::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FInt64Property::StaticClass())) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Object(UMagicNodeSharp* Node, const FName &Field, TMap<int64,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Object::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Object::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Object::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FInt64Property::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Actor(UMagicNodeSharp* Node, const FName &Field, TMap<int64,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Actor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Actor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Actor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FInt64Property::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Int64Component(UMagicNodeSharp* Node, const FName &Field, TMap<int64,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Component::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Component::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Int64Component::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FInt64Property::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FInt64Property>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_FloatBool(UMagicNodeSharp* Node, const FName &Field, TMap<float,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FFloatProperty::StaticClass()))&&(Property->ValueProp->IsA(FBoolProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatByte(UMagicNodeSharp* Node, const FName &Field, TMap<float,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FFloatProperty::StaticClass()))&&(Property->ValueProp->IsA(FByteProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatInt(UMagicNodeSharp* Node, const FName &Field, TMap<float,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FFloatProperty::StaticClass()))&&(Property->ValueProp->IsA(FIntProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatInt64(UMagicNodeSharp* Node, const FName &Field, TMap<float,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FFloatProperty::StaticClass()))&&(Property->ValueProp->IsA(FInt64Property::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatFloat(UMagicNodeSharp* Node, const FName &Field, TMap<float,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FFloatProperty::StaticClass()))&&(Property->ValueProp->IsA(FFloatProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatString(UMagicNodeSharp* Node, const FName &Field, TMap<float,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FFloatProperty::StaticClass()))&&(Property->ValueProp->IsA(FStrProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatName(UMagicNodeSharp* Node, const FName &Field, TMap<float,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FFloatProperty::StaticClass()))&&(Property->ValueProp->IsA(FNameProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatText(UMagicNodeSharp* Node, const FName &Field, TMap<float,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FFloatProperty::StaticClass()))&&(Property->ValueProp->IsA(FTextProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatColor(UMagicNodeSharp* Node, const FName &Field, TMap<float,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FFloatProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<float,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FFloatProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<float,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FFloatProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatRotator(UMagicNodeSharp* Node, const FName &Field, TMap<float,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FFloatProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatTransform(UMagicNodeSharp* Node, const FName &Field, TMap<float,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FFloatProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatClass(UMagicNodeSharp* Node, const FName &Field, TMap<float,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FFloatProperty::StaticClass())) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatObject(UMagicNodeSharp* Node, const FName &Field, TMap<float,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FFloatProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatActor(UMagicNodeSharp* Node, const FName &Field, TMap<float,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FFloatProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_FloatComponent(UMagicNodeSharp* Node, const FName &Field, TMap<float,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_FloatComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FFloatProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FFloatProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_StringBool(UMagicNodeSharp* Node, const FName &Field, TMap<FString,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FStrProperty::StaticClass()))&&(Property->ValueProp->IsA(FBoolProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringByte(UMagicNodeSharp* Node, const FName &Field, TMap<FString,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FStrProperty::StaticClass()))&&(Property->ValueProp->IsA(FByteProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringInt(UMagicNodeSharp* Node, const FName &Field, TMap<FString,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FStrProperty::StaticClass()))&&(Property->ValueProp->IsA(FIntProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FString,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FStrProperty::StaticClass()))&&(Property->ValueProp->IsA(FInt64Property::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FString,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FStrProperty::StaticClass()))&&(Property->ValueProp->IsA(FFloatProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringString(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FStrProperty::StaticClass()))&&(Property->ValueProp->IsA(FStrProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringName(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FStrProperty::StaticClass()))&&(Property->ValueProp->IsA(FNameProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringText(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FStrProperty::StaticClass()))&&(Property->ValueProp->IsA(FTextProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringColor(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FStrProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FStrProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FStrProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FStrProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FString,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FStrProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringClass(UMagicNodeSharp* Node, const FName &Field, TMap<FString,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FStrProperty::StaticClass())) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringObject(UMagicNodeSharp* Node, const FName &Field, TMap<FString,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FStrProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringActor(UMagicNodeSharp* Node, const FName &Field, TMap<FString,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FStrProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_StringComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FString,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_StringComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FStrProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FStrProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_NameBool(UMagicNodeSharp* Node, const FName &Field, TMap<FName,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FNameProperty::StaticClass()))&&(Property->ValueProp->IsA(FBoolProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameByte(UMagicNodeSharp* Node, const FName &Field, TMap<FName,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FNameProperty::StaticClass()))&&(Property->ValueProp->IsA(FByteProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameInt(UMagicNodeSharp* Node, const FName &Field, TMap<FName,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FNameProperty::StaticClass()))&&(Property->ValueProp->IsA(FIntProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FName,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FNameProperty::StaticClass()))&&(Property->ValueProp->IsA(FInt64Property::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FName,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FNameProperty::StaticClass()))&&(Property->ValueProp->IsA(FFloatProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameString(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FNameProperty::StaticClass()))&&(Property->ValueProp->IsA(FStrProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameName(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FNameProperty::StaticClass()))&&(Property->ValueProp->IsA(FNameProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameText(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if ((Property->KeyProp->IsA(FNameProperty::StaticClass()))&&(Property->ValueProp->IsA(FTextProperty::StaticClass()))) {
					FScriptMapHelper Map(Property,Out.ValueAddr);
					Output.Reserve(Map.Num());
					//
					for (int32 I=0; I<Map.Num(); ++I) {
						Output.Add(
							CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
							CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
						);///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameColor(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FNameProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FNameProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FNameProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FNameProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FName,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FNameProperty::StaticClass())) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameClass(UMagicNodeSharp* Node, const FName &Field, TMap<FName,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FNameProperty::StaticClass())) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameObject(UMagicNodeSharp* Node, const FName &Field, TMap<FName,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FNameProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameActor(UMagicNodeSharp* Node, const FName &Field, TMap<FName,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FNameProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_NameComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FName,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_NameComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (Property->KeyProp->IsA(FNameProperty::StaticClass())) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								CastFieldChecked<FNameProperty>(Property->KeyProp)->GetPropertyValue(Map.GetKeyPtr(I)),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_Vector2DBool(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector2D KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DByte(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector2D KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DInt(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector2D KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector2D KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector2D KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DString(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector2D KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DName(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector2D KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DText(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector2D KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DColor(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector2D KeyCopy; FColor ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector2D KeyCopy; FVector2D ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector2D KeyCopy; FVector ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector2D KeyCopy; FRotator ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector2D KeyCopy; FTransform ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DClass(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FVector2D KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I))));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DObject(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FVector2D KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Value->GetObjectPropertyValue(Map.GetValuePtr(I)));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DActor(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FVector2D KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I))));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector2DComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FVector2D,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector2DComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FVector2D KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I))));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_Vector3DBool(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DByte(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DInt(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DString(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DName(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DText(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FVector KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DColor(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector KeyCopy; FColor ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector KeyCopy; FVector2D ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector KeyCopy; FVector ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector KeyCopy; FRotator ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FVector KeyCopy; FTransform ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DClass(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FVector KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I))));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DObject(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FVector KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Value->GetObjectPropertyValue(Map.GetValuePtr(I)));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DActor(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FVector KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I))));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_Vector3DComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FVector,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_Vector3DComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FVector KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I))));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_ColorBool(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FColor KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorByte(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FColor KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorInt(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FColor KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorInt64(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FColor KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorFloat(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FColor KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorString(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FColor KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorName(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FColor KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorText(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							const uint8* KeyPtr = Map.GetKeyPtr(I);
							FColor KeyCopy;
							//
							InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
							Output.Add(KeyCopy,CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I)));
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorColor(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FColor KeyCopy; FColor ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FColor KeyCopy; FVector2D ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FColor KeyCopy; FVector ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorRotator(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FColor KeyCopy; FRotator ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorTransform(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
							if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
								FScriptMapHelper Map(Property,Out.ValueAddr);
								Output.Reserve(Map.Num());
								//
								for (int32 I=0; I<Map.Num(); ++I) {
									const uint8* ValuePtr = Map.GetValuePtr(I);
									const uint8* KeyPtr = Map.GetKeyPtr(I);
									FColor KeyCopy; FTransform ValueCopy;
									//
									InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
									InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
									Output.Add(KeyCopy,ValueCopy);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorClass(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FColor KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I))));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorObject(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FColor KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Value->GetObjectPropertyValue(Map.GetValuePtr(I)));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorActor(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FColor KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I))));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ColorComponent(UMagicNodeSharp* Node, const FName &Field, TMap<FColor,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ColorComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FStructProperty*InnerKey=CastField<FStructProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						if (InnerKey->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* KeyPtr = Map.GetKeyPtr(I);
								FColor KeyCopy;
								//
								InnerKey->CopySingleValue(&KeyCopy,KeyPtr);
								Output.Add(KeyCopy,Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I))));
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_ClassBool(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FBoolProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassByte(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FByteProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassInt(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FIntProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassInt64(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FInt64Property::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassFloat(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FFloatProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassString(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FStrProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassName(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FNameProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassText(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FTextProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassColor(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassRotator(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassTransform(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassClass(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassObject(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassActor(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ClassComponent(UMagicNodeSharp* Node, const FName &Field, TMap<UClass*,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ClassComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FClassProperty*Key=CastField<FClassProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UClass>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_ObjectBool(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FBoolProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectByte(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FByteProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectInt(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FIntProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectInt64(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FInt64Property::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectFloat(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FFloatProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectString(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FStrProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectName(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FNameProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectText(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FTextProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectColor(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectRotator(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectTransform(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectClass(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectObject(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectActor(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ObjectComponent(UMagicNodeSharp* Node, const FName &Field, TMap<UObject*,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ObjectComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Key->GetObjectPropertyValue(Map.GetKeyPtr(I)),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_ActorBool(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FBoolProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorByte(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FByteProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorInt(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FIntProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorInt64(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FInt64Property::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorFloat(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FFloatProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorString(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FStrProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorName(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FNameProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorText(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FTextProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorColor(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorRotator(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorTransform(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorClass(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorObject(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorActor(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ActorComponent(UMagicNodeSharp* Node, const FName &Field, TMap<AActor*,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ActorComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<AActor>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMonoObject::GET_MonoMapValue_ComponentBool(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,bool>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentBool::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentBool::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentBool::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FBoolProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FBoolProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentByte(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,uint8>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentByte::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentByte::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentByte::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FByteProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FByteProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentInt(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,int32>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentInt::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentInt::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentInt::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FIntProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FIntProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentInt64(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,int64>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentInt64::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentInt64::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentInt64::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FInt64Property::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FInt64Property>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentFloat(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,float>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentFloat::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentFloat::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentFloat::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FFloatProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FFloatProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentString(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FString>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentString::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentString::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentString::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FStrProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FStrProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentName(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FName>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentName::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentName::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentName::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FNameProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FNameProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentText(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FText>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentText::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentText::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentText::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (Property->ValueProp->IsA(FTextProperty::StaticClass())) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								CastFieldChecked<FTextProperty>(Property->ValueProp)->GetPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentColor(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FColor>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentColor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentColor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentColor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FColor>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FColor ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentVector2D(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FVector2D>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentVector2D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentVector2D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentVector2D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector2D>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector2D ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentVector3D(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FVector>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentVector3D::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentVector3D::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentVector3D::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FVector>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FVector ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentRotator(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FRotator>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentRotator::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentRotator::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentRotator::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FRotator>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FRotator ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentTransform(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,FTransform>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentTransform::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentTransform::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentTransform::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FStructProperty*InnerValue=CastField<FStructProperty>(Property->ValueProp)) {
						if (InnerValue->Struct==TBaseStructure<FTransform>::Get()) {
							FScriptMapHelper Map(Property,Out.ValueAddr);
							Output.Reserve(Map.Num());
							//
							for (int32 I=0; I<Map.Num(); ++I) {
								const uint8* ValuePtr = Map.GetValuePtr(I);
								FTransform ValueCopy;
								//
								InnerValue->CopySingleValue(&ValueCopy,ValuePtr);
								Output.Add(Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),ValueCopy);
							}///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentClass(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,UClass*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentClass::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentClass::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentClass::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FClassProperty*Value=CastField<FClassProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Cast<UClass>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentObject(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,UObject*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentObject::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentObject::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentObject::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Value->GetObjectPropertyValue(Map.GetValuePtr(I))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentActor(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,AActor*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentActor::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentActor::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentActor::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Cast<AActor>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

void IMonoObject::GET_MonoMapValue_ComponentComponent(UMagicNodeSharp* Node, const FName &Field, TMap<UActorComponent*,UActorComponent*>&Output) {
	if (Field.IsNone()) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentComponent::  Field is None.")); return;}
	if (Node==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentComponent::  No Context.")); return;}
	//
	MonoObject* ManagedObject = Node->GetMonoObject();
	if (ManagedObject==nullptr) {LOG::CS_CHAR(ESeverity::Error,TEXT("__GET_MonoMapValue_ComponentComponent::  Invalid Managed Object.")); return;}
	//
	if (MonoProperty*MonoProp=Node->GetMonoProperty(Field)) {
		MonoMethod* GET = mono_property_get_get_method(MonoProp);
		//
		if (MonoObject*iCall=mono_runtime_invoke(GET,ManagedObject,NULL,NULL)) {
			FMonoMap Out = *(FMonoMap*)mono_object_unbox(iCall);
			//
			if (FMapProperty*Property=reinterpret_cast<FMapProperty*>(Out.Property)) {
				if (FObjectProperty*Key=CastField<FObjectProperty>(Property->KeyProp)) {
					if (FObjectProperty*Value=CastField<FObjectProperty>(Property->ValueProp)) {
						FScriptMapHelper Map(Property,Out.ValueAddr);
						Output.Reserve(Map.Num());
						//
						for (int32 I=0; I<Map.Num(); ++I) {
							Output.Add(
								Cast<UActorComponent>(Key->GetObjectPropertyValue(Map.GetKeyPtr(I))),
								Cast<UActorComponent>(Value->GetObjectPropertyValue(Map.GetValuePtr(I)))
							);///
						}///
					}///
				}///
			}///
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////