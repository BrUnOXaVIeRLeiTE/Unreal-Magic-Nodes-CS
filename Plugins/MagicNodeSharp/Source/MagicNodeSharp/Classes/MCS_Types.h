//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MagicNodeSharp_Shared.h"

#include "MCS_Types.generated.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class EMonoThread : uint8 {
	MainThread		= 0,
	OwnThread		= 1
};

enum class EMonoPlayState : uint8 {
	Shutdown		= 0,
	Playing			= 1
};

enum class EMonoKismetState : uint8 {
	Iddle			= 0,
	Compiling		= 1
};

enum class EMonoCompilerResult : uint8 {
	Success			= 0,
	Warning			= 1,
	Error			= 2
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FSourceInfo final {
	int32 Column;
	int32 Line;
	//
	FSourceInfo()
	  : Column(0)
	  , Line(0)
	{}
	//
	FSourceInfo(int32 L, int32 C)
	  : Column(C)
	  , Line(L)
	{}
public:
	bool operator == (const FSourceInfo &Other) const {
		return (
			(Column==Other.Column) &&
			(Line==Other.Line)
		);//
	}///
};

struct FCompilerResults {
	EMonoCompilerResult Result;
	FSourceInfo ErrorInfo;
	FString ErrorMessage;
	//
	FCompilerResults()
	  : Result(EMonoCompilerResult::Success)
	  , ErrorInfo(-1,-1)
	  , ErrorMessage()
	{}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(meta=(BlueprintInternalUseOnly=true))
enum class EMonoScopeType : uint8 {
	Member,
	Static,
	Virtual,
	Internal
};

UENUM(meta=(BlueprintInternalUseOnly=true))
enum class EMonoParamType : uint8 {
	Input,
	Output,
	InOut
};

UENUM(meta=(BlueprintInternalUseOnly=true))
enum class EMonoAccessType : uint8 {
	Private,
	Protected,
	Public
};

UENUM(meta=(BlueprintInternalUseOnly=true))
enum class EMonoDataType : uint8 {
	Unknown						= 0,
	Void						= 1,
	Bool						= 2,
	Byte						= 3,
	Int							= 4,
	Int64						= 5,
	Float						= 6,
	String						= 7,
	Name						= 8,
	Text						= 9,
	Color						= 10,
	Vector2D					= 11,
	Vector3D					= 12,
	Rotator						= 13,
	Transform					= 14,
	Class						= 15,
	Object						= 16,
	Actor						= 17,
	Component					= 18
};

UENUM(meta=(BlueprintInternalUseOnly=true))
enum class EMonoListType : uint8 {
	None,
	Array,
	Set,
	Map
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(meta=(BlueprintInternalUseOnly=true))
struct MAGICNODESHARP_API FMonoPropertyInfo {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY() FName Name;							// Node pins need to know the name of the property.
	UPROPERTY() FString Value;						// The actual value of the property  .ToString()
	UPROPERTY() EMonoDataType DataType;				// Managed versions of supported Array/Set Types.
	UPROPERTY() EMonoDataType SubType;				// Managed versions of supported Map Types.
	UPROPERTY() EMonoListType ListType;				// Managed versions of supported List Containers.
	UPROPERTY() EMonoParamType ParamType;			// Property is exposed as input, output pin, or both.
	UPROPERTY() EMonoAccessType AccessType;			// Property access level declared on script.
	UPROPERTY() EMonoScopeType ScopeType;			// Property scope level declared on script.
	UPROPERTY() FString Description;				// Reflection description of Get/Set accessors.
public:
	FMonoPropertyInfo()
		: Name(NAME_None)
		, Value(TEXT(""))
		, DataType(EMonoDataType::Void)
		, SubType(EMonoDataType::Void)
		, ListType(EMonoListType::None)
		, ParamType(EMonoParamType::Input)
		, AccessType(EMonoAccessType::Private)
		, ScopeType(EMonoScopeType::Member)
		, Description(TEXT(""))
	{}//
public:
	inline FMonoPropertyInfo &operator = (const FMonoPropertyInfo &Other) {
		Name = Other.Name;
		Value = Other.Value;
		SubType = Other.SubType;
		DataType = Other.DataType;
		ListType = Other.ListType;
		ParamType = Other.ParamType;
		ScopeType = Other.ScopeType;
		AccessType = Other.AccessType;
		Description = Other.Description;
		//
		return *this;
	}///
};

USTRUCT(meta=(BlueprintInternalUseOnly=true))
struct MAGICNODESHARP_API FMonoScriptData {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY() TArray<FMonoPropertyInfo>PropertyInfo;
	UPROPERTY() bool Asynchronous;
public:
	FMonoScriptData() : PropertyInfo(), Asynchronous(false) {}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(meta=(BlueprintInternalUseOnly=true))
struct FMonoFieldDefinition {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY() FName Name;
	UPROPERTY() FString TypeName;
	UPROPERTY() FString Description;
	UPROPERTY() EMonoDataType DataType;
	UPROPERTY() EMonoDataType SubType;
	UPROPERTY() EMonoListType ListType;
	UPROPERTY() EMonoAccessType AccessType;
	UPROPERTY() EMonoScopeType ScopeType;
public:
	FMonoFieldDefinition()
		: Name(NAME_None)
		, TypeName(TEXT(""))
		, Description(TEXT(""))
		, DataType(EMonoDataType::Void)
		, SubType(EMonoDataType::Void)
		, ListType(EMonoListType::None)
		, AccessType(EMonoAccessType::Private)
		, ScopeType(EMonoScopeType::Member)
	{}///
public:
	friend inline uint32 GetTypeHash(const FMonoFieldDefinition &DEF) {
		return FCrc::MemCrc32(&DEF,sizeof(FMonoFieldDefinition));
	}///
	//
	inline FMonoFieldDefinition &operator = (const FMonoFieldDefinition &Other) {
		Name = Other.Name;
		SubType = Other.SubType;
		DataType = Other.DataType;
		ListType = Other.ListType;
		TypeName = Other.TypeName;
		ScopeType = Other.ScopeType;
		AccessType = Other.AccessType;
		Description = Other.Description;
		//
		return *this;
	}///
	//
	bool operator == (const FMonoFieldDefinition &Other) const {
		return (
			Name == Other.Name &&
			SubType == Other.SubType &&
			DataType == Other.DataType &&
			ListType == Other.ListType &&
			TypeName == Other.TypeName &&
			ScopeType == Other.ScopeType &&
			AccessType == Other.AccessType &&
			Description == Other.Description
		);//
	}///
	//
	bool operator != (const FMonoFieldDefinition &Other) const {
		return !(*this==Other);
	}///
public:
	bool IsValid() const {
		return !(
			Name.IsNone() &&
			TypeName.IsEmpty() &&
			Description.IsEmpty()
		);//
	}///
public:
	void Reset() {
		Name = NAME_None;
		//
		TypeName.Empty();
		Description.Empty();
		//
		ListType = EMonoListType::None;
		SubType = EMonoDataType::Unknown;
		DataType = EMonoDataType::Unknown;
		ScopeType = EMonoScopeType::Member;
		AccessType = EMonoAccessType::Private;
	}///
public:
	FString TypeToString() const {
		switch (DataType) {
			case EMonoDataType::Void:			return TEXT("void"); break;
			case EMonoDataType::Bool:			return TEXT("bool"); break;
			case EMonoDataType::Byte:			return TEXT("byte"); break;
			case EMonoDataType::Int:			return TEXT("int"); break;
			case EMonoDataType::Int64:			return TEXT("long"); break;
			case EMonoDataType::Float:			return TEXT("float"); break;
			case EMonoDataType::String:			return TEXT("Strand"); break;
			case EMonoDataType::Name:			return TEXT("Name"); break;
			case EMonoDataType::Text:			return TEXT("Text"); break;
			case EMonoDataType::Vector2D:		return TEXT("Vector2D"); break;
			case EMonoDataType::Vector3D:		return TEXT("Vector3D"); break;
			case EMonoDataType::Rotator:		return TEXT("Rotator"); break;
			case EMonoDataType::Color:			return TEXT("Color"); break;
			case EMonoDataType::Transform:		return TEXT("Transform"); break;
			case EMonoDataType::Class:			return TEXT("ClassPtr"); break;
			case EMonoDataType::Object:			return TEXT("ObjectPtr"); break;
			case EMonoDataType::Actor:			return TEXT("ActorPtr"); break;
			case EMonoDataType::Component:		return TEXT("ComponentPtr"); break;
		default: break;}
		//
		return TypeName;
	}///
	//
	FString SubTypeToString() const {
		switch (SubType) {
			case EMonoDataType::Void:			return TEXT("void"); break;
			case EMonoDataType::Bool:			return TEXT("bool"); break;
			case EMonoDataType::Byte:			return TEXT("byte"); break;
			case EMonoDataType::Int:			return TEXT("int"); break;
			case EMonoDataType::Int64:			return TEXT("long"); break;
			case EMonoDataType::Float:			return TEXT("float"); break;
			case EMonoDataType::String:			return TEXT("Strand"); break;
			case EMonoDataType::Name:			return TEXT("Name"); break;
			case EMonoDataType::Text:			return TEXT("Text"); break;
			case EMonoDataType::Vector2D:		return TEXT("Vector2D"); break;
			case EMonoDataType::Vector3D:		return TEXT("Vector3D"); break;
			case EMonoDataType::Rotator:		return TEXT("Rotator"); break;
			case EMonoDataType::Color:			return TEXT("Color"); break;
			case EMonoDataType::Transform:		return TEXT("Transform"); break;
			case EMonoDataType::Class:			return TEXT("ClassPtr"); break;
			case EMonoDataType::Object:			return TEXT("ObjectPtr"); break;
			case EMonoDataType::Actor:			return TEXT("ActorPtr"); break;
			case EMonoDataType::Component:		return TEXT("ComponentPtr"); break;
		default: break;}
		//
		return TypeName;
	}///
	//
	FString StackToString() const {
		switch (ListType) {
			case EMonoListType::Map: return TEXT("IMap"); break;
			case EMonoListType::Set: return TEXT("ISet"); break;
			case EMonoListType::Array: return TEXT("IArray"); break;
		default: break;}
		//
		return TEXT("");
	}///
	//
	FString AccessToString() const {
		switch (AccessType) {
			case EMonoAccessType::Private:			return TEXT("Private"); break;
			case EMonoAccessType::Protected:		return TEXT("Protected"); break;
			case EMonoAccessType::Public:			return TEXT("Public"); break;
		default: break;}
		//
		return TEXT("");
	}///
	//
	FString ScopeToString() const {
		switch (ScopeType) {
			case EMonoScopeType::Member:			return TEXT("Field"); break;
			case EMonoScopeType::Static:			return TEXT("Static"); break;
			case EMonoScopeType::Virtual:			return TEXT("Virtual"); break;
			case EMonoScopeType::Internal:			return TEXT("Internal"); break;
		default: break;}
		//
		return TEXT("");
	}///
};

USTRUCT(meta=(BlueprintInternalUseOnly=true))
struct MAGICNODESHARP_API FMonoParameterType {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY() FName Name;
	UPROPERTY() FString TypeName;
	UPROPERTY() EMonoParamType ParamType;
public:
	friend inline uint32 GetTypeHash(const FMonoParameterType &DEF) {
		return FCrc::MemCrc32(&DEF,sizeof(FMonoParameterType));
	}///
	//
	inline FMonoParameterType &operator = (const FMonoParameterType &Other) {
		Name = Other.Name;
		TypeName = Other.TypeName;
		ParamType = Other.ParamType;
		//
		return *this;
	}///
	//
	bool operator == (const FMonoParameterType &Other) const {
		return (
			Name == Other.Name &&
			TypeName == Other.TypeName &&
			ParamType == Other.ParamType
		);//
	}///
	//
	bool operator != (const FMonoParameterType &Other) const {
		return !(*this==Other);
	}///
public:
	bool IsValid() const {
		return !(
			Name.IsNone() &&
			TypeName.IsEmpty()
		);//
	}///
public:
	void Reset() {
		Name = NAME_None;
		//
		TypeName.Empty();
		//
		ParamType = EMonoParamType::Input;
	}///
};

USTRUCT(meta=(BlueprintInternalUseOnly=true))
struct FMonoMethodDefinition {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY() FName Name;
	UPROPERTY() FString ReturnTypeName;
	UPROPERTY() EMonoDataType ReturnType;
	UPROPERTY() EMonoAccessType AccessType;
	UPROPERTY() EMonoScopeType ScopeType;
	//
	UPROPERTY() TSet<FMonoParameterType>Inputs;
	UPROPERTY() TSet<FMonoParameterType>Outputs;
	//
	UPROPERTY() FString Description;
public:
	FMonoMethodDefinition()
		: Name(NAME_None)
		, ReturnTypeName(TEXT(""))
		, ReturnType(EMonoDataType::Unknown)
		, AccessType(EMonoAccessType::Private)
		, ScopeType(EMonoScopeType::Member)
		, Inputs()
		, Outputs()
		, Description(TEXT(""))
	{}///
public:
	friend inline uint32 GetTypeHash(const FMonoMethodDefinition &DEF) {
		return FCrc::MemCrc32(&DEF,sizeof(FMonoMethodDefinition));
	}///
	//
	inline FMonoMethodDefinition &operator = (const FMonoMethodDefinition &Other) {
		Name = Other.Name;
		Inputs = Other.Inputs;
		Outputs = Other.Outputs;
		ScopeType = Other.ScopeType;
		AccessType = Other.AccessType;
		ReturnType = Other.ReturnType;
		Description = Other.Description;
		ReturnTypeName = Other.ReturnTypeName;
		//
		return *this;
	}///
	//
	bool operator == (const FMonoMethodDefinition &Other) const {
		return (
			Outputs.GetAllocatedSize() == Other.Outputs.GetAllocatedSize() &&
			Inputs.GetAllocatedSize() == Other.Inputs.GetAllocatedSize() &&
			//
			ReturnTypeName == Other.ReturnTypeName &&
			ReturnType == Other.ReturnType &&
			AccessType == Other.AccessType &&
			ScopeType == Other.ScopeType &&
			Name == Other.Name &&
			//
			Description == Other.Description
		);//
	}///
	//
	bool operator != (const FMonoMethodDefinition &Other) const {
		return !(*this==Other);
	}///
public:
	bool IsValid() const {
		return !(
			Name.IsNone() &&
			Description.IsEmpty() &&
			ReturnTypeName.IsEmpty() &&
			ReturnType==EMonoDataType::Unknown
		);//
	}///
public:
	void Reset() {
		Name = NAME_None;
		//
		Inputs.Empty();
		Outputs.Empty();
		Description.Empty();
		ReturnTypeName.Empty();
		//
		ScopeType = EMonoScopeType::Member;
		ReturnType = EMonoDataType::Unknown;
		AccessType = EMonoAccessType::Private;
	}///
public:
	FString TypeToString() const {
		switch (ReturnType) {
			case EMonoDataType::Void:			return TEXT("void"); break;
			case EMonoDataType::Bool:			return TEXT("bool"); break;
			case EMonoDataType::Byte:			return TEXT("byte"); break;
			case EMonoDataType::Int:			return TEXT("int"); break;
			case EMonoDataType::Int64:			return TEXT("long"); break;
			case EMonoDataType::Float:			return TEXT("float"); break;
			case EMonoDataType::String:			return TEXT("Strand"); break;
			case EMonoDataType::Name:			return TEXT("Name"); break;
			case EMonoDataType::Text:			return TEXT("Text"); break;
			case EMonoDataType::Vector2D:		return TEXT("Vector2D"); break;
			case EMonoDataType::Vector3D:		return TEXT("Vector3D"); break;
			case EMonoDataType::Rotator:		return TEXT("Rotator"); break;
			case EMonoDataType::Color:			return TEXT("Color"); break;
			case EMonoDataType::Transform:		return TEXT("Transform"); break;
			case EMonoDataType::Class:			return TEXT("ClassPtr"); break;
			case EMonoDataType::Object:			return TEXT("ObjectPtr"); break;
			case EMonoDataType::Actor:			return TEXT("ActorPtr"); break;
			case EMonoDataType::Component:		return TEXT("ComponentPtr"); break;
		default: break;}
		//
		return ReturnTypeName;
	}///
	//
	FString AccessToString() const {
		switch (AccessType) {
			case EMonoAccessType::Private:			return TEXT("Private"); break;
			case EMonoAccessType::Protected:		return TEXT("Protected"); break;
			case EMonoAccessType::Public:			return TEXT("Public"); break;
		default: break;}
		//
		return TEXT("");
	}///
	//
	FString ScopeToString() const {
		switch (ScopeType) {
			case EMonoScopeType::Member:			return TEXT("Method"); break;
			case EMonoScopeType::Static:			return TEXT("Static"); break;
			case EMonoScopeType::Virtual:			return TEXT("Virtual"); break;
			case EMonoScopeType::Internal:			return TEXT("Internal"); break;
		default: break;}
		//
		return TEXT("");
	}///
};

USTRUCT(meta=(BlueprintInternalUseOnly=true))
struct FMonoClassDefinition {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY() FName Name;
	UPROPERTY() TSet<FMonoFieldDefinition>Props;
	UPROPERTY() TSet<FMonoFieldDefinition>Fields;
	UPROPERTY() TSet<FMonoMethodDefinition>Methods;
	//
	UPROPERTY() FString Namespace;
	UPROPERTY() FString ParentClass;
	UPROPERTY() FString Description;
	//
	UPROPERTY() EMonoAccessType AccessType;
	UPROPERTY() EMonoScopeType ScopeType;
public:
	FMonoClassDefinition()
		: Name(NAME_None)
		, Props()
		, Fields()
		, Methods()
		, Namespace(TEXT(""))
		, ParentClass(TEXT(""))
		, Description(TEXT(""))
		, AccessType(EMonoAccessType::Private)
		, ScopeType(EMonoScopeType::Member)
	{}///
public:
	friend inline uint32 GetTypeHash(const FMonoClassDefinition &DEF) {
		return FCrc::MemCrc32(&DEF,sizeof(FMonoClassDefinition));
	}///
	//
	inline FMonoClassDefinition &operator = (const FMonoClassDefinition Other) {
		Name = Other.Name;
		Props = Other.Props;
		Fields = Other.Fields;
		Methods = Other.Methods;
		Namespace = Other.Namespace;
		ParentClass = Other.ParentClass;
		Description = Other.Description;
		AccessType = Other.AccessType;
		ScopeType = Other.ScopeType;
		//
		return *this;
	}///
	//
	bool operator == (const FMonoClassDefinition &Other) const {
		return (
			Props.GetAllocatedSize() == Other.Props.GetAllocatedSize() &&
			Fields.GetAllocatedSize() == Other.Fields.GetAllocatedSize() &&
			Methods.GetAllocatedSize() == Other.Methods.GetAllocatedSize() &&
			//
			Name == Other.Name &&
			Namespace == Other.Namespace &&
			ParentClass == Other.ParentClass &&
			Description == Other.Description &&
			AccessType == Other.AccessType &&
			ScopeType == Other.ScopeType
		);//
	}///
	//
	bool operator != (const FMonoClassDefinition &Other) const {
		return !(*this==Other);
	}///
public:
	bool IsValid() const {
		return !(
			Name.IsNone() &&
			Namespace.IsEmpty() &&
			ParentClass.IsEmpty() &&
			Description.IsEmpty()
		);//
	}///
public:
	void Reset() {
		Name = NAME_None;
		//
		Props.Empty();
		Fields.Empty();
		Methods.Empty();
		//
		Namespace.Empty();
		ParentClass.Empty();
		Description.Empty();
		//
		ScopeType = EMonoScopeType::Member;
		AccessType = EMonoAccessType::Private;
	}///
public:
	FString AccessToString() const {
		switch (AccessType) {
			case EMonoAccessType::Private:			return TEXT("Private"); break;
			case EMonoAccessType::Protected:		return TEXT("Protected"); break;
			case EMonoAccessType::Public:			return TEXT("Public"); break;
		default: break;}
		//
		return TEXT("");
	}///
	//
	FString ScopeToString() const {
		switch (ScopeType) {
			case EMonoScopeType::Member:			return TEXT("Class"); break;
			case EMonoScopeType::Static:			return TEXT("Static"); break;
			case EMonoScopeType::Virtual:			return TEXT("Virtual"); break;
			case EMonoScopeType::Internal:			return TEXT("Internal"); break;
		default: break;}
		//
		return TEXT("");
	}///
};

USTRUCT(meta=(BlueprintInternalUseOnly=true))
struct FMonoNamespaceDefinition {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY() FName Name;
	UPROPERTY() TSet<FString>Namespaces;
public:
	FMonoNamespaceDefinition()
		: Name(NAME_None)
		, Namespaces()
	{}///
public:
	friend inline uint32 GetTypeHash(const FMonoNamespaceDefinition &DEF) {
		return FCrc::MemCrc32(&DEF,sizeof(FMonoNamespaceDefinition));
	}///
	//
	inline FMonoNamespaceDefinition &operator = (const FMonoNamespaceDefinition &Other) {
		Name = Other.Name;
		Namespaces = Other.Namespaces;
		//
		return *this;
	}///
	//
	bool operator == (const FMonoNamespaceDefinition &Other) const {
		return (
			Namespaces.Array() == Other.Namespaces.Array() &&
			Name == Other.Name
		);//
	}///
	//
	bool operator != (const FMonoNamespaceDefinition &Other) const {
		return !(*this==Other);
	}///
public:
	bool IsValid() const {
		return !Name.IsNone();
	}///
public:
	void Reset() {
		Name = NAME_None;
		Namespaces.Empty();
	}///
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////