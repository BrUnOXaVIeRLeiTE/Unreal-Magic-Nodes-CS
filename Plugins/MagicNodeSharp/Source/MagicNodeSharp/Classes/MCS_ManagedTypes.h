//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MCS_Types.h"

#include "mono/metadata/image.h"
#include "mono/metadata/class.h"
#include "mono/metadata/object.h"

#include "MagicNodeSharp_Shared.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// :: TYPE HELPERS ::

typedef uint32_t CSHANDLE;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class EMonoSearchCase : uint8 {
	CaseSensitive		= 0,
	IgnoreCase			= 1
};

enum class EMonoSearchDir : uint8 {
	FromStart		= 0,
	FromEnd			= 1
};

enum class EMonoDateStyle : uint8 {
	Default		= 0,
	Short		= 1,
	Medium		= 2,
	Long		= 3,
	Full		= 4
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static EMonoDataType GetPropertyDataType(FProperty* Property) {
	if (Property==nullptr) {return EMonoDataType::Unknown;}
	//
	const bool IsArray = Property->IsA(FArrayProperty::StaticClass());
	const bool IsSet = Property->IsA(FSetProperty::StaticClass());
	const bool IsMap = Property->IsA(FMapProperty::StaticClass());
	//
	const bool IsBool = Property->IsA(FBoolProperty::StaticClass());
	const bool IsByte = Property->IsA(FByteProperty::StaticClass());
	const bool IsInt = Property->IsA(FIntProperty::StaticClass());
	const bool IsInt64 = Property->IsA(FInt64Property::StaticClass());
	const bool IsFloat = Property->IsA(FFloatProperty::StaticClass());
	const bool IsString = Property->IsA(FStrProperty::StaticClass());
	const bool IsName = Property->IsA(FNameProperty::StaticClass());
	const bool IsText = Property->IsA(FTextProperty::StaticClass());
	//
	const bool IsClass = Property->IsA(FClassProperty::StaticClass());
	const bool IsObject = Property->IsA(FObjectProperty::StaticClass());
	const bool IsStruct = Property->IsA(FStructProperty::StaticClass());
	//
	//
	if (IsBool) {return EMonoDataType::Bool;}
	if (IsByte) {return EMonoDataType::Byte;}
	if (IsInt) {return EMonoDataType::Int;}
	if (IsInt64) {return EMonoDataType::Int64;}
	if (IsFloat) {return EMonoDataType::Float;}
	if (IsString) {return EMonoDataType::String;}
	if (IsName) {return EMonoDataType::Name;}
	if (IsText) {return EMonoDataType::Text;}
	if (IsClass) {return EMonoDataType::Class;}
	if (IsObject) {return EMonoDataType::Object;}
	//
	//
	if (IsStruct) {
		FStructProperty* Prop = CastFieldChecked<FStructProperty>(Property);
		//
		if (Prop->Struct==TBaseStructure<FColor>::Get()) {return EMonoDataType::Color;}
		if (Prop->Struct==TBaseStructure<FRotator>::Get()) {return EMonoDataType::Rotator;}
		if (Prop->Struct==TBaseStructure<FVector>::Get()) {return EMonoDataType::Vector3D;}
		if (Prop->Struct==TBaseStructure<FVector2D>::Get()) {return EMonoDataType::Vector2D;}
		if (Prop->Struct==TBaseStructure<FTransform>::Get()) {return EMonoDataType::Transform;}
	}///
	//
	//
	if (IsArray) {
		FArrayProperty* Array = CastFieldChecked<FArrayProperty>(Property);
		//
		if (Array->Inner->IsA(FBoolProperty::StaticClass())) {return EMonoDataType::Bool;}
		if (Array->Inner->IsA(FByteProperty::StaticClass())) {return EMonoDataType::Byte;}
		if (Array->Inner->IsA(FIntProperty::StaticClass())) {return EMonoDataType::Int;}
		if (Array->Inner->IsA(FInt64Property::StaticClass())) {return EMonoDataType::Int64;}
		if (Array->Inner->IsA(FFloatProperty::StaticClass())) {return EMonoDataType::Float;}
		if (Array->Inner->IsA(FStrProperty::StaticClass())) {return EMonoDataType::String;}
		if (Array->Inner->IsA(FNameProperty::StaticClass())) {return EMonoDataType::Name;}
		if (Array->Inner->IsA(FTextProperty::StaticClass())) {return EMonoDataType::Text;}
		if (Array->Inner->IsA(FClassProperty::StaticClass())) {return EMonoDataType::Class;}
		if (Array->Inner->IsA(FObjectProperty::StaticClass())) {return EMonoDataType::Object;}
		//
		if (Array->Inner->IsA(FStructProperty::StaticClass())) {
			FStructProperty* Prop = CastFieldChecked<FStructProperty>(Array->Inner);
			//
			if (Prop->Struct==TBaseStructure<FColor>::Get()) {return EMonoDataType::Color;}
			if (Prop->Struct==TBaseStructure<FRotator>::Get()) {return EMonoDataType::Rotator;}
			if (Prop->Struct==TBaseStructure<FVector>::Get()) {return EMonoDataType::Vector3D;}
			if (Prop->Struct==TBaseStructure<FVector2D>::Get()) {return EMonoDataType::Vector2D;}
			if (Prop->Struct==TBaseStructure<FTransform>::Get()) {return EMonoDataType::Transform;}
		}///
	}///
	//
	//
	if (IsSet) {
		FSetProperty* Set = CastFieldChecked<FSetProperty>(Property);
		//
		if (Set->ElementProp->IsA(FBoolProperty::StaticClass())) {return EMonoDataType::Bool;}
		if (Set->ElementProp->IsA(FByteProperty::StaticClass())) {return EMonoDataType::Byte;}
		if (Set->ElementProp->IsA(FIntProperty::StaticClass())) {return EMonoDataType::Int;}
		if (Set->ElementProp->IsA(FInt64Property::StaticClass())) {return EMonoDataType::Int64;}
		if (Set->ElementProp->IsA(FFloatProperty::StaticClass())) {return EMonoDataType::Float;}
		if (Set->ElementProp->IsA(FStrProperty::StaticClass())) {return EMonoDataType::String;}
		if (Set->ElementProp->IsA(FNameProperty::StaticClass())) {return EMonoDataType::Name;}
		if (Set->ElementProp->IsA(FClassProperty::StaticClass())) {return EMonoDataType::Class;}
		if (Set->ElementProp->IsA(FObjectProperty::StaticClass())) {return EMonoDataType::Object;}
		//
		if (Set->ElementProp->IsA(FStructProperty::StaticClass())) {
			FStructProperty* Prop = CastFieldChecked<FStructProperty>(Set->ElementProp);
			//
			if (Prop->Struct==TBaseStructure<FColor>::Get()) {return EMonoDataType::Color;}
			if (Prop->Struct==TBaseStructure<FVector>::Get()) {return EMonoDataType::Vector3D;}
			if (Prop->Struct==TBaseStructure<FVector2D>::Get()) {return EMonoDataType::Vector2D;}
		}///
	}///
	//
	//
	if (IsMap) {
		FMapProperty* Map = CastFieldChecked<FMapProperty>(Property);
		//
		if (Map->KeyProp->IsA(FBoolProperty::StaticClass())) {return EMonoDataType::Bool;}
		if (Map->KeyProp->IsA(FByteProperty::StaticClass())) {return EMonoDataType::Byte;}
		if (Map->KeyProp->IsA(FIntProperty::StaticClass())) {return EMonoDataType::Int;}
		if (Map->KeyProp->IsA(FInt64Property::StaticClass())) {return EMonoDataType::Int64;}
		if (Map->KeyProp->IsA(FFloatProperty::StaticClass())) {return EMonoDataType::Float;}
		if (Map->KeyProp->IsA(FStrProperty::StaticClass())) {return EMonoDataType::String;}
		if (Map->KeyProp->IsA(FNameProperty::StaticClass())) {return EMonoDataType::Name;}
		if (Map->KeyProp->IsA(FClassProperty::StaticClass())) {return EMonoDataType::Class;}
		if (Map->KeyProp->IsA(FObjectProperty::StaticClass())) {return EMonoDataType::Object;}
		//
		if (Map->KeyProp->IsA(FStructProperty::StaticClass())) {
			FStructProperty* Prop = CastFieldChecked<FStructProperty>(Map->KeyProp);
			//
			if (Prop->Struct==TBaseStructure<FColor>::Get()) {return EMonoDataType::Color;}
			if (Prop->Struct==TBaseStructure<FVector>::Get()) {return EMonoDataType::Vector3D;}
			if (Prop->Struct==TBaseStructure<FVector2D>::Get()) {return EMonoDataType::Vector2D;}
		}///
	}///
	//
	//
	return EMonoDataType::Unknown;
}

static EMonoDataType GetPropertySubType(FProperty* Property) {
	const bool IsMap = Property->IsA(FMapProperty::StaticClass());
	//
	if (!IsMap) {return EMonoDataType::Void;} else {
		FMapProperty* Map = CastFieldChecked<FMapProperty>(Property);
		//
		if (Map->ValueProp->IsA(FBoolProperty::StaticClass())) {return EMonoDataType::Bool;}
		if (Map->ValueProp->IsA(FByteProperty::StaticClass())) {return EMonoDataType::Byte;}
		if (Map->ValueProp->IsA(FIntProperty::StaticClass())) {return EMonoDataType::Int;}
		if (Map->ValueProp->IsA(FInt64Property::StaticClass())) {return EMonoDataType::Int64;}
		if (Map->ValueProp->IsA(FFloatProperty::StaticClass())) {return EMonoDataType::Float;}
		if (Map->ValueProp->IsA(FStrProperty::StaticClass())) {return EMonoDataType::String;}
		if (Map->ValueProp->IsA(FNameProperty::StaticClass())) {return EMonoDataType::Float;}
		if (Map->ValueProp->IsA(FTextProperty::StaticClass())) {return EMonoDataType::Text;}
		if (Map->ValueProp->IsA(FClassProperty::StaticClass())) {return EMonoDataType::Class;}
		if (Map->ValueProp->IsA(FObjectProperty::StaticClass())) {return EMonoDataType::Object;}
		//
		if (Map->ValueProp->IsA(FStructProperty::StaticClass())) {
			FStructProperty* Prop = CastFieldChecked<FStructProperty>(Map->ValueProp);
			//
			if (Prop->Struct==TBaseStructure<FColor>::Get()) {return EMonoDataType::Color;}
			if (Prop->Struct==TBaseStructure<FRotator>::Get()) {return EMonoDataType::Rotator;}
			if (Prop->Struct==TBaseStructure<FVector>::Get()) {return EMonoDataType::Vector3D;}
			if (Prop->Struct==TBaseStructure<FVector2D>::Get()) {return EMonoDataType::Vector2D;}
			if (Prop->Struct==TBaseStructure<FTransform>::Get()) {return EMonoDataType::Transform;}
		}///
	}///
	//
	return EMonoDataType::Unknown;
}

static EMonoListType GetPropertyStackType(FProperty* Property) {
	const bool IsArray = Property->IsA(FArrayProperty::StaticClass());
	const bool IsSet = Property->IsA(FSetProperty::StaticClass());
	const bool IsMap = Property->IsA(FMapProperty::StaticClass());
	//
	if (IsArray) {return EMonoListType::Array;}
	if (IsSet) {return EMonoListType::Set;}
	if (IsMap) {return EMonoListType::Map;}
	//
	return EMonoListType::None;
}

static EMonoDataType GetValueDataType(FProperty* Property, void* ValuePtr) {
	if (Property==nullptr) {return EMonoDataType::Unknown;}
	//
	if (FObjectProperty*OBJProp=CastField<FObjectProperty>(Property)) {
		UObject* OBJ = OBJProp->GetPropertyValue(ValuePtr);
		//
		if (!OBJ->IsValidLowLevel()) {return EMonoDataType::Object;}
		if (OBJ->IsA(AActor::StaticClass())) {return EMonoDataType::Actor;}
		if (OBJ->IsA(UActorComponent::StaticClass())) {return EMonoDataType::Component;}
		//
		return EMonoDataType::Object;
	} else if (FArrayProperty*Array=CastField<FArrayProperty>(Property)) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Array->Inner)) {
			UObject* OBJ = Inner->GetPropertyValue(ValuePtr);
			//
			if (!OBJ->IsValidLowLevel()) {return EMonoDataType::Object;}
			if (OBJ->IsA(AActor::StaticClass())) {return EMonoDataType::Actor;}
			if (OBJ->IsA(UActorComponent::StaticClass())) {return EMonoDataType::Component;}
			//
			return EMonoDataType::Object;
		}///
	} else if (FSetProperty*Set=CastField<FSetProperty>(Property)) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Set->ElementProp)) {
			UObject* OBJ = Inner->GetPropertyValue(ValuePtr);
			//
			if (!OBJ->IsValidLowLevel()) {return EMonoDataType::Object;}
			if (OBJ->IsA(AActor::StaticClass())) {return EMonoDataType::Actor;}
			if (OBJ->IsA(UActorComponent::StaticClass())) {return EMonoDataType::Component;}
			//
			return EMonoDataType::Object;
		}///
	} else if (FMapProperty*Map=CastField<FMapProperty>(Property)) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Map->KeyProp)) {
			UObject* OBJ = Inner->GetPropertyValue(ValuePtr);
			//
			if (!OBJ->IsValidLowLevel()) {return EMonoDataType::Object;}
			if (OBJ->IsA(AActor::StaticClass())) {return EMonoDataType::Actor;}
			if (OBJ->IsA(UActorComponent::StaticClass())) {return EMonoDataType::Component;}
			//
			return EMonoDataType::Object;
		}///
	}///
	//
	return GetPropertyDataType(Property);
}

static EMonoDataType GetValueSubType(FProperty* Property, void* ValuePtr) {
	if (Property==nullptr) {return EMonoDataType::Unknown;}
	//
	if (FMapProperty*Map=CastField<FMapProperty>(Property)) {
		if (FObjectProperty*Inner=CastField<FObjectProperty>(Map->ValueProp)) {
			UObject* OBJ = Inner->GetPropertyValue(ValuePtr);
			//
			if (!OBJ->IsValidLowLevel()) {return EMonoDataType::Object;}
			if (OBJ->IsA(AActor::StaticClass())) {return EMonoDataType::Actor;}
			if (OBJ->IsA(UActorComponent::StaticClass())) {return EMonoDataType::Component;}
			//
			return EMonoDataType::Object;
		}///
	}///
	//
	return GetPropertySubType(Property);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// :: BASE VALUE TYPES ::

struct FMonoString {
	MonoString* Data;
	const int32 Len;
public:
	FMonoString() : Data(nullptr), Len(0) {}
public:
	FMonoString(const FString &InString, MonoDomain* InDomain)
	 : Data(mono_string_new(InDomain,StringCast<ANSICHAR>(*InString).Get()))
	 , Len(StringCast<WIDECHAR>(*InString).Length())
	{}
public:
	FString ToString() const {
		if (Data==nullptr) {return FString{};}
		//
		WIDECHAR* Value = mono_string_to_utf16(Data);
		return FString(StringCast<TCHAR>(Value).Get());
	}///
};

struct FMonoName {
	MonoString* Data;
	const int32 Len;
public:
	FMonoName() : Data(nullptr), Len(0) {}
public:
	FMonoName(const FName &InName, MonoDomain* InDomain)
	 : Data(mono_string_new(InDomain,StringCast<ANSICHAR>(*InName.ToString()).Get()))
	 , Len(StringCast<WIDECHAR>(*InName.ToString()).Length())
	{}
public:
	FName ToName() const {
		if (Data==nullptr) {return FName{};}
		//
		WIDECHAR* Value = mono_string_to_utf16(Data);
		return FName(StringCast<TCHAR>(Value).Get());
	}///
};

struct FMonoText {
	MonoString* Data;
	const int32 Len;
public:
	FMonoText() : Data(nullptr), Len(0) {}
public:
	FMonoText(const FText &InText, MonoDomain* InDomain)
	 : Data(mono_string_new(InDomain,StringCast<ANSICHAR>(*InText.ToString()).Get()))
	 , Len(StringCast<WIDECHAR>(*InText.ToString()).Length())
	{}
public:
	FText ToText() const {
		if (Data==nullptr) {return FText{};}
		//
		WIDECHAR* Value = mono_string_to_utf16(Data);
		return FText::FromString(StringCast<TCHAR>(Value).Get());
	}///
};

struct FMonoColor {
	const uint8 R;
	const uint8 G;
	const uint8 B;
	const uint8 A;
public:
	FMonoColor() : R(0) , G(0) , B(0) , A(0) {}
	FMonoColor(uint8 r, uint8 g, uint8 b, uint8 a) : R(r) , G(g), B(b), A(a) {}
	FMonoColor(const FColor &Color) : R(Color.R) , G(Color.G), B(Color.B), A(Color.A) {}
public:
	FMonoColor(const FLinearColor &Color, const bool sRGB)
		: R(Color.ToFColor(sRGB).R)
		, G(Color.ToFColor(sRGB).G)
		, B(Color.ToFColor(sRGB).B)
		, A(Color.ToFColor(sRGB).A)
	{}//
public:
	FColor ToColor() const {
		return FColor(R,G,B,A);
	}///
};

struct FMonoVector2D {
	const float X;
	const float Y;
public:
	FMonoVector2D() : X(0) , Y(0) {}
	FMonoVector2D(float x, float y) : X(x) , Y(y) {}
	FMonoVector2D(const FVector2D &Vector) : X(Vector.X) , Y(Vector.Y) {}
public:
	FVector2D ToVector2D() const {
		return FVector2D(X,Y);
	}///
};

struct FMonoVector3D {
	const float X;
	const float Y;
	const float Z;
public:
	FMonoVector3D() : X(0) , Y(0), Z(0) {}
	FMonoVector3D(float x, float y, float z) : X(x) , Y(y), Z(z) {}
	FMonoVector3D(const FVector &Vector) : X(Vector.X) , Y(Vector.Y) , Z(Vector.Z) {}
public:
	FVector ToVector3D() const {
		return FVector(X,Y,Z);
	}///
};

struct FMonoRotator {
	const float Pitch;
	const float Yaw;
	const float Roll;
public:
	FMonoRotator() : Pitch(0) , Yaw(0), Roll(0) {}
	FMonoRotator(float pitch, float yaw, float roll) : Pitch(pitch) , Yaw(yaw), Roll(roll) {}
	FMonoRotator(const FRotator &Rotator) : Pitch(Rotator.Pitch) , Yaw(Rotator.Yaw) , Roll(Rotator.Roll) {}
public:
	FRotator ToRotator() const {
		return FRotator(Pitch,Yaw,Roll);
	}///
};

struct FMonoTransform {
	const FMonoRotator Rotation;
	const FMonoVector3D Translation;
	const FMonoVector3D Scale;
public:
	FMonoTransform() : Rotation() , Translation() , Scale() {}
public:
	FMonoTransform(const FTransform &Transform)
		: Rotation(Transform.Rotator())
		, Translation(Transform.GetTranslation())
		, Scale(Transform.GetScale3D())
	{}//
public:
	FTransform ToTransform() const {
		return FTransform(Rotation.ToRotator(),Translation.ToVector3D(),Scale.ToVector3D());
	}///
};

struct FMonoClass {
	void* IntPtr;	/// : Instance Value Address in Kismet VM
public:
	FMonoClass() : IntPtr(0) {}
	FMonoClass(void* Addr) : IntPtr(Addr) {}
	FMonoClass(UClass* CLSS) : IntPtr(reinterpret_cast<void*>(CLSS)) {}
public:
	UClass* ToClass() const {
		return reinterpret_cast<UClass*>(IntPtr);
	}///
};

struct FMonoObject {
	void* IntPtr;	/// : Instance Value Address in Kismet VM
public:
	FMonoObject() : IntPtr(0) {}
	FMonoObject(void* Addr) : IntPtr(Addr) {}
	FMonoObject(UObject* OBJ) : IntPtr(reinterpret_cast<void*>(OBJ)) {}
public:
	UObject* ToObject() const {
		return reinterpret_cast<UObject*>(IntPtr);
	}///
};

struct FMonoActor {
	void* IntPtr;	/// : Instance Value Address in Kismet VM
public:
	FMonoActor() : IntPtr(0) {}
	FMonoActor(void* Addr) : IntPtr(Addr) {}
	FMonoActor(AActor* ACT) : IntPtr(reinterpret_cast<void*>(ACT)) {}
public:
	AActor* ToActor() const {
		return reinterpret_cast<AActor*>(IntPtr);
	}///
};

struct FMonoComponent {
	void* IntPtr;	/// : Instance Value Address in Kismet VM
public:
	FMonoComponent() : IntPtr(0) {}
	FMonoComponent(void* Addr) : IntPtr(Addr) {}
	FMonoComponent(UActorComponent* CMP) : IntPtr(reinterpret_cast<void*>(CMP)) {}
public:
	UActorComponent* ToComponent() const {
		return reinterpret_cast<UActorComponent*>(IntPtr);
	}///
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// :: BASE CONTAINER TYPES ::

struct FMonoArray {
	EMonoDataType InnerType;
	void* Property;		/// : Prop  address in Kismet VM
	void* ValueAddr;	/// : Value address in Kismet VM
public:
	FMonoArray() : InnerType(EMonoDataType::Unknown), Property(0), ValueAddr(0) {}
public:
	FMonoArray(FArrayProperty* DataType, void* Addr)
	 : InnerType(GetValueDataType(CastFieldChecked<FProperty>(DataType),Addr))
	 , Property(DataType), ValueAddr(Addr)
	{}
};

struct FMonoSet {
	EMonoDataType InnerType;
	void* Property;		/// : Prop  address in Kismet VM
	void* ValueAddr;	/// : Value address in Kismet VM
public:
	FMonoSet() : InnerType(EMonoDataType::Unknown), Property(0), ValueAddr(0) {}
public:
	FMonoSet(FSetProperty* DataType, void* Addr)
	 : InnerType(GetValueDataType(CastFieldChecked<FProperty>(DataType),Addr))
	 , Property(DataType), ValueAddr(Addr)
	{}
};

struct FMonoMap {
	EMonoDataType InnerType;
	EMonoDataType SubType;
	void* Property;		/// : Prop  address in Kismet VM
	void* ValueAddr;	/// : Value address in Kismet VM
public:
	FMonoMap() : InnerType(EMonoDataType::Unknown), SubType(EMonoDataType::Unknown), Property(0), ValueAddr(0) {}
public:
	FMonoMap(FMapProperty* DataType, void* Addr)
	 : InnerType(GetValueDataType(CastFieldChecked<FProperty>(DataType),Addr))
	 , SubType(GetValueSubType(CastFieldChecked<FProperty>(DataType),Addr))
	 , Property(DataType), ValueAddr(Addr)
	{}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////