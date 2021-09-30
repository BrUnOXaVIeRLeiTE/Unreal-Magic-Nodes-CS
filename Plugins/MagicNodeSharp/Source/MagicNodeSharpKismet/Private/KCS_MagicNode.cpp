//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KCS_MagicNode.h"
#include "IMagicNodeSharpKismet.h"

#include "Engine/EngineTypes.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Runtime/Core/Public/Async/TaskGraphInterfaces.h"

#include "K2Node_CallFunction.h"
#include "K2Node_MakeArray.h"
#include "K2Node_MakeSet.h"
#include "K2Node_MakeMap.h"

#include "KismetCompiler.h"
#include "KismetCompilerMisc.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

#include "Editor.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "AssetRegistryModule.h"
#include "EditorCategoryUtils.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "Editor/BlueprintGraph/Classes/K2Node_Self.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CSHelper {
	static FName END;
	static FName EXE;
	static FName LOAD;
	static FName POST;
	static FName PN_Node;
	static FName PN_Script;
	static FName PN_Context;
};

FName CSHelper::END(TEXT("Exit"));
FName CSHelper::EXE(TEXT("Execute"));
FName CSHelper::LOAD(TEXT("LoadScript"));
FName CSHelper::POST(TEXT("PostLoadScript"));

FName CSHelper::PN_Node(TEXT("Node"));
FName CSHelper::PN_Script(TEXT("Script"));
FName CSHelper::PN_Context(TEXT("Context"));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UKCS_MagicNode::UKCS_MagicNode(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer) {
	NodeTooltip = LOCTEXT("CS_NodeTooltip","Magic Node (C#)");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FText UKCS_MagicNode::GetNodeTitle(ENodeTitleType::Type TType) const {
	FText Title = NSLOCTEXT("K2Node","KCS_MagicNode_BaseTitle","Magic Node (C#)");
	//
	if (TType!=ENodeTitleType::MenuTitle) {
		if (GetScriptSource()==nullptr) {return Title;}
		//
		FString SName = GetScriptSource()->GetName();
		FString SCaps; SCaps.AppendChar(SName[0]);
		//
		SName.RemoveFromEnd(TEXT("_C"));
		SName.RemoveFromStart(TEXT("Default__"));
		SName.ReplaceInline(TEXT("_"),TEXT(" "));
		//
		for (int32 I=1; I < SName.Len(); I++) {
			if (FChar::IsUpper(SName[I])) {
				if (SName[I-1] != TEXT(' ') && !FChar::IsUpper(SName[I-1])) {SCaps.AppendChar(TEXT(' '));}
			} SCaps.AppendChar(SName[I]);
		}///
		//
		FText Name = FText::FromString(SCaps);
		//
		FFormatNamedArguments Args;
		Args.Add(TEXT("Name"),Name);
		//
		CachedNodeTitle.SetCachedText(FText::Format(NSLOCTEXT("K2Node","KCS_MagicNode_Title","(C#) :: {Name}"),Args),this);
		//
		Title = CachedNodeTitle;
	}///
	//
	return Title;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::AllocateDefaultPins() {
	Super::AllocateDefaultPins();
	//
	//
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	ErrorType = EMessageSeverity::Info;
	//
	//
	/// Pin Type Definitions.
	static FEdGraphPinType PNT_Exec(KSchema->PC_Exec,TEXT(""),nullptr,EPinContainerType::None,false,FEdGraphTerminalType());
	static FEdGraphPinType PNT_Object(KSchema->PC_Object,TEXT(""),UObject::StaticClass(),EPinContainerType::None,false,FEdGraphTerminalType());
	static FEdGraphPinType PNT_Script(KSchema->PC_Object,TEXT(""),UMagicNodeSharpSource::StaticClass(),EPinContainerType::None,false,FEdGraphTerminalType());
	//
	//
	/// Node's Execution Pins.
	UEdGraphPin* PIN_Finish = GetThenPin();
	UEdGraphPin* PIN_Execute = GetExecPin();
	if (PIN_Finish==nullptr) {PIN_Finish=CreatePin(EGPD_Output,PNT_Exec,KSchema->PN_Then);}
	if (PIN_Execute==nullptr) {PIN_Execute=CreatePin(EGPD_Input,PNT_Exec,KSchema->PN_Execute);}
	//
	/// Node's Context Pin.
	UEdGraphPin* PIN_Context = GetContextPin();
	if (PIN_Context==nullptr) {PIN_Context=CreatePin(EGPD_Input,PNT_Object,CSHelper::PN_Context);}
	//
	/// Node's Script Pin.
	UEdGraphPin* PIN_Script = GetScriptPin();
	if (PIN_Script==nullptr) {PIN_Script=CreatePin(EGPD_Input,PNT_Script,CSHelper::PN_Script);}
	//
	//
	PIN_Script->bHidden = true;
	PIN_Script->bAdvancedView = true;
	//
	//
	if (GetBlueprint()==nullptr) {return;}
	if (!GetBlueprint()->ParentClass->HasMetaDataHierarchical(FBlueprintMetadata::MD_ShowWorldContextPin)) {PIN_Context->bHidden=true;}
	//
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
	//
	//
	if (CollapsePanel) {
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
	} else {AdvancedPinDisplay=ENodeAdvancedPins::Shown;}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::ExpandNode(FKismetCompilerContext &CompilerContext, UEdGraph* SourceGraph) {
	Super::ExpandNode(CompilerContext,SourceGraph);
	//
	if (HasAnyFlags(RF_ClassDefaultObject|RF_ArchetypeObject)) {return;}
	if (SourceGraph==nullptr||SourceGraph->IsPendingKill()) {return;}
	//
	//
	GetScriptCDO();
	UKCS_MagicNode* Self = this;
	UMagicNodeSharpSource* ScriptSource = GetScriptSource();
	//
	UEdGraphPin* PIN_Node = nullptr;
	UEdGraphPin* PIN_Exec = Self->GetExecPin();
	UEdGraphPin* PIN_Then = Self->GetThenPin();
	UEdGraphPin* PIN_Script = Self->GetScriptPin();
	UEdGraphPin* PIN_Context = Self->FindPinChecked(CSHelper::PN_Context,EGPD_Input);
	//
	if (ScriptSource==nullptr) {
		CompilerContext.MessageLog.Error(*LOCTEXT("KCS_MagicNode_Script_Error","@@  ::  must have a @@ specified!").ToString(),Self,PIN_Script);
		Self->BreakAllNodeLinks();
	return;}
	//
	if (Instance.Get()==nullptr) {
		CompilerContext.MessageLog.Error(*LOCTEXT("KCS_MagicNode_Script_Error","@@  ::  must have a @@ specified!").ToString(),Self,PIN_Script);
		Self->BreakAllNodeLinks();
	return;}
	//
	if (!ErrorMsg.IsEmpty()) {
		FString Err = ErrorMsg; Err.RemoveFromStart(FString::Printf(TEXT("%s  ::  "),*ScriptSource->GetScriptName()));
		CompilerContext.MessageLog.Error(*LOCTEXT("KCS_MagicNode_Script_Error","@@  ::  @@").ToString(),Self,*Err);
		Self->BreakAllNodeLinks();
	return;}
	//
	//
	UFunction* Function_LOAD = ExpandScriptCall(Instance.Get(),CSHelper::LOAD);
	UFunction* Function_POST = ExpandScriptCall(Instance.Get(),CSHelper::POST);
	UFunction* Function_EXEC = ExpandScriptCall(Instance.Get(),CSHelper::EXE);
	UFunction* Function_EXIT = ExpandScriptCall(Instance.Get(),CSHelper::END);
	//
	//
	if (Function_LOAD==nullptr) {
		CompilerContext.MessageLog.Error(*LOCTEXT("KCS_MagicNode_Function_LoadError","@@  ::  unable to allocate Script Loader Function; Compile the Script Class before attaching this node.").ToString(),Self);
	} else if (Function_POST==nullptr) {
		CompilerContext.MessageLog.Error(*LOCTEXT("KCS_MagicNode_Function_PostLoadError","@@  ::  unable to allocate Script Configuration Function; Compile the Script Class before attaching this node.").ToString(),Self);
	} else if (Function_EXEC==nullptr) {
		CompilerContext.MessageLog.Error(*LOCTEXT("KCS_MagicNode_Function_ExecError","@@  ::  unable to allocate Script Executable Function; Compile the Script Class before attaching this node.").ToString(),Self);
	} else if (Function_EXIT==nullptr) {
		CompilerContext.MessageLog.Error(*LOCTEXT("KCS_MagicNode_Function_ExitError","@@  ::  unable to allocate Script Exit Function; Compile the Script Class before attaching this node.").ToString(),Self);
	} else {
		UK2Node_CallFunction* CS_Load = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(Self,SourceGraph);
		CS_Load->SetFromFunction(Function_LOAD);
		CS_Load->AllocateDefaultPins();
		{
			UEdGraphPin* PUT_Exec = CS_Load->GetExecPin();
			UEdGraphPin* PUT_Then = CS_Load->GetThenPin();
			UEdGraphPin* PUT_Script = CS_Load->FindPinChecked(CSHelper::PN_Script,EGPD_Input);
			UEdGraphPin* PUT_Context = CS_Load->FindPinChecked(CSHelper::PN_Context,EGPD_Input);
			//
			CompilerContext.MovePinLinksToIntermediate(*PIN_Exec,*PUT_Exec);
			CompilerContext.MovePinLinksToIntermediate(*PIN_Then,*PUT_Then);
			CompilerContext.CopyPinLinksToIntermediate(*PIN_Script,*PUT_Script);
			CompilerContext.CopyPinLinksToIntermediate(*PIN_Context,*PUT_Context);
			//
			if (PIN_Context->LinkedTo.Num()>0) {
				PIN_Context = PIN_Context->LinkedTo[0];
			} else {
				UK2Node_Self* CS_Self = Self->GetGraph()->CreateIntermediateNode<UK2Node_Self>();
				CS_Self->AllocateDefaultPins();
				//
				UEdGraphPin* PIN_Self = CS_Self->FindPinChecked(UEdGraphSchema_K2::PN_Self,EGPD_Output);
				GetSchema()->TryCreateConnection(PIN_Self,PIN_Context);
			}///
			//
			PIN_Node = CS_Load->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue,EGPD_Output);
			PIN_Exec = PUT_Exec; PIN_Then = PUT_Then;
		}
		//
		//
		for (int32 I=0; I<Pins.Num(); I++) {
			UEdGraphPin* PIN = Pins[I];
			//
			FMonoPropertyInfo PIN_Data{};
			if (IsDefaultPin(PIN)) {continue;}
			if (PIN->IsPendingKill()) {continue;}
			//
			if (PIN->Direction==EGPD_Input) {
				UFunction* Function_SET = nullptr;
				//
				bool IsArray = false;
				bool IsSet = false;
				bool IsMap = false;
				//
				const FString FieldName = PIN->GetName().Replace(TEXT("_"),TEXT(""));
				for (const auto &Info : ScriptSource->GetScriptData().PropertyInfo) {
					if (Info.Name.ToString()==FieldName) {PIN_Data=Info; break;}
				} if (PIN_Data.Name.IsNone()) {continue;}
				//
				switch(PIN_Data.ListType) {
					case EMonoListType::None:
					{
						switch(PIN_Data.DataType) {
							case EMonoDataType::Bool:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Bool"));
							} break;
							//
							case EMonoDataType::Byte:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Byte"));
							} break;
							//
							case EMonoDataType::Int:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Int"));
							} break;
							//
							case EMonoDataType::Int64:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Int64"));
							} break;
							//
							case EMonoDataType::Float:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Float"));
							} break;
							//
							case EMonoDataType::String:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_String"));
							} break;
							//
							case EMonoDataType::Name:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Name"));
							} break;
							//
							case EMonoDataType::Text:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Text"));
							} break;
							//
							case EMonoDataType::Vector2D:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Vector2D"));
							} break;
							//
							case EMonoDataType::Vector3D:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Vector3D"));
							} break;
							//
							case EMonoDataType::Rotator:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Rotator"));
							} break;
							//
							case EMonoDataType::Color:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Color"));
							} break;
							//
							case EMonoDataType::Transform:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Transform"));
							} break;
							//
							case EMonoDataType::Class:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Class"));
							} break;
							//
							case EMonoDataType::Object:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Object"));
							} break;
							//
							case EMonoDataType::Actor:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Actor"));
							} break;
							//
							case EMonoDataType::Component:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoProperty_Component"));
							} break;
						default: break;}
					} break;
					//
					case EMonoListType::Array:
					{
						IsArray = true;
						switch(PIN_Data.DataType) {
							case EMonoDataType::Bool:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Bool"));
							} break;
							//
							case EMonoDataType::Byte:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Byte"));
							} break;
							//
							case EMonoDataType::Int:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Int"));
							} break;
							//
							case EMonoDataType::Int64:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Int64"));
							} break;
							//
							case EMonoDataType::Float:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Float"));
							} break;
							//
							case EMonoDataType::String:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_String"));
							} break;
							//
							case EMonoDataType::Name:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Name"));
							} break;
							//
							case EMonoDataType::Text:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Text"));
							} break;
							//
							case EMonoDataType::Vector2D:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Vector2D"));
							} break;
							//
							case EMonoDataType::Vector3D:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Vector3D"));
							} break;
							//
							case EMonoDataType::Rotator:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Rotator"));
							} break;
							//
							case EMonoDataType::Color:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Color"));
							} break;
							//
							case EMonoDataType::Transform:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Transform"));
							} break;
							//
							case EMonoDataType::Class:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Class"));
							} break;
							//
							case EMonoDataType::Object:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Object"));
							} break;
							//
							case EMonoDataType::Actor:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Actor"));
							} break;
							//
							case EMonoDataType::Component:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoArray_Component"));
							} break;
						default: break;}
					} break;
					//
					case EMonoListType::Set:
					{
						IsSet = true;
						switch(PIN_Data.DataType) {
							case EMonoDataType::Byte:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Byte"));
							} break;
							//
							case EMonoDataType::Int:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Int"));
							} break;
							//
							case EMonoDataType::Int64:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Int64"));
							} break;
							//
							case EMonoDataType::Float:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Float"));
							} break;
							//
							case EMonoDataType::String:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_String"));
							} break;
							//
							case EMonoDataType::Name:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Name"));
							} break;
							//
							case EMonoDataType::Vector2D:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Vector2D"));
							} break;
							//
							case EMonoDataType::Vector3D:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Vector3D"));
							} break;
							//
							case EMonoDataType::Color:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Color"));
							} break;
							//
							case EMonoDataType::Class:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Class"));
							} break;
							//
							case EMonoDataType::Object:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Object"));
							} break;
							//
							case EMonoDataType::Actor:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Actor"));
							} break;
							//
							case EMonoDataType::Component:
							{
								Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoSet_Component"));
							} break;
						default: break;}
					} break;
					//
					case EMonoListType::Map:
					{
						IsMap = true;
						switch(PIN_Data.DataType) {
							case EMonoDataType::Byte:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ByteComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Int:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_IntComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Int64:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Bool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Byte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Int"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Int64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Float"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64String"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Name"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Text"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Color"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Vector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Vector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Rotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Transform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Class"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Object"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Actor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Int64Component"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Float:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_FloatComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::String:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_StringComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Name:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_NameComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Vector2D:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector2DComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Vector3D:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_Vector3DComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Color:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ColorComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Class:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ClassComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Object:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ObjectComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Actor:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ActorComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Component:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_SET = ExpandScriptCall(Instance.Get(),TEXT("SetMonoMap_ComponentComponent"));
									} break;
								default: break;}
							} break;
						default: break;}
					} break;
				default: break;}
				//
				UK2Node_CallFunction* CS_SET_Property = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(Self,SourceGraph);
				CS_SET_Property->SetFromFunction(Function_SET);
				CS_SET_Property->AllocateDefaultPins();
				//
				UEdGraphPin* PUT_Exec = CS_SET_Property->GetExecPin();
				UEdGraphPin* PUT_Then = CS_SET_Property->GetThenPin();
				UEdGraphPin* PUT_Field = CS_SET_Property->FindPinChecked(TEXT("Field"),EGPD_Input);
				UEdGraphPin* PUT_Input = CS_SET_Property->FindPinChecked(TEXT("Input"),EGPD_Input);
				UEdGraphPin* PUT_Node = CS_SET_Property->FindPinChecked(CSHelper::PN_Node,EGPD_Input);
				UEdGraphPin* PUT_Context = CS_SET_Property->FindPinChecked(CSHelper::PN_Context,EGPD_Input);
				//
				PUT_Input->PinType = PIN->PinType;
				PUT_Input->DefaultValue = PIN->DefaultValue;
				PUT_Input->DefaultObject = PIN->DefaultObject;
				PUT_Input->DefaultTextValue = PIN->DefaultTextValue;
				//
				CompilerContext.MovePinLinksToIntermediate(*PIN,*PUT_Input);
				CompilerContext.MovePinLinksToIntermediate(*PIN_Then,*PUT_Then);
				CompilerContext.CopyPinLinksToIntermediate(*PIN_Context,*PUT_Context);
				//
				GetSchema()->TryCreateConnection(PIN_Node,PUT_Node);
				GetSchema()->TrySetDefaultValue(*PUT_Field,FieldName);
				GetSchema()->TryCreateConnection(PIN_Then,PUT_Exec);
				//
				if (IsArray && (PUT_Input->LinkedTo.Num()==0)) {
					UK2Node_MakeArray* MakeArray = CompilerContext.SpawnIntermediateNode<UK2Node_MakeArray>(Self,SourceGraph);
					MakeArray->NumInputs = 0; MakeArray->AllocateDefaultPins();
					//
					UEdGraphPin* ArrOut = MakeArray->GetOutputPin();
					ArrOut->PinType = PUT_Input->PinType;
					ArrOut->MakeLinkTo(PUT_Input);
					//
					MakeArray->PinConnectionListChanged(ArrOut);
				}///
				//
				if (IsSet && (PUT_Input->LinkedTo.Num()==0)) {
					UK2Node_MakeSet* MakeSet = CompilerContext.SpawnIntermediateNode<UK2Node_MakeSet>(Self,SourceGraph);
					MakeSet->NumInputs = 0; MakeSet->AllocateDefaultPins();
					//
					UEdGraphPin* SetOut = MakeSet->GetOutputPin();
					SetOut->PinType = PUT_Input->PinType;
					SetOut->MakeLinkTo(PUT_Input);
					//
					MakeSet->PinConnectionListChanged(SetOut);
				}///
				//
				if (IsMap && (PUT_Input->LinkedTo.Num()==0)) {
					UK2Node_MakeMap* MakeMap = CompilerContext.SpawnIntermediateNode<UK2Node_MakeMap>(Self,SourceGraph);
					MakeMap->NumInputs = 0; MakeMap->AllocateDefaultPins();
					//
					UEdGraphPin* MapOut = MakeMap->GetOutputPin();
					MapOut->PinType = PUT_Input->PinType;
					MapOut->MakeLinkTo(PUT_Input);
					//
					MakeMap->PinConnectionListChanged(MapOut);
				}///
				//
				PIN_Exec = PUT_Exec;
				PIN_Then = PUT_Then;
			}///
		}///
		//
		//
		UK2Node_CallFunction* CS_PostLoad = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(Self,SourceGraph);
		CS_PostLoad->SetFromFunction(Function_POST);
		CS_PostLoad->AllocateDefaultPins();
		{
			UEdGraphPin* PUT_Exec = CS_PostLoad->GetExecPin();
			UEdGraphPin* PUT_Then = CS_PostLoad->GetThenPin();
			UEdGraphPin* PUT_Node = CS_PostLoad->FindPinChecked(CSHelper::PN_Node,EGPD_Input);
			UEdGraphPin* PUT_Context = CS_PostLoad->FindPinChecked(CSHelper::PN_Context,EGPD_Input);
			//
			CompilerContext.MovePinLinksToIntermediate(*PIN_Then,*PUT_Then);
			CompilerContext.CopyPinLinksToIntermediate(*PIN_Context,*PUT_Context);
			//
			GetSchema()->TryCreateConnection(PIN_Node,PUT_Node);
			GetSchema()->TryCreateConnection(PIN_Then,PUT_Exec);
			PIN_Exec = PUT_Exec; PIN_Then = PUT_Then;
		}
		//
		//
		UK2Node_CallFunction* CS_Execute = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(Self,SourceGraph);
		CS_Execute->SetFromFunction(Function_EXEC);
		CS_Execute->AllocateDefaultPins();
		{
			UEdGraphPin* PUT_Exec = CS_Execute->GetExecPin();
			UEdGraphPin* PUT_Then = CS_Execute->GetThenPin();
			UEdGraphPin* PUT_Node = CS_Execute->FindPinChecked(CSHelper::PN_Node,EGPD_Input);
			UEdGraphPin* PUT_Context = CS_Execute->FindPinChecked(CSHelper::PN_Context,EGPD_Input);
			//
			CompilerContext.MovePinLinksToIntermediate(*PIN_Then,*PUT_Then);
			CompilerContext.CopyPinLinksToIntermediate(*PIN_Context,*PUT_Context);
			//
			GetSchema()->TryCreateConnection(PIN_Node,PUT_Node);
			GetSchema()->TryCreateConnection(PIN_Then,PUT_Exec);
			PIN_Exec = PUT_Exec; PIN_Then = PUT_Then;
		}
		//
		//
		for (int32 I=0; I<Pins.Num(); I++) {
			UEdGraphPin* PIN = Pins[I];
			//
			FMonoPropertyInfo PIN_Data{};
			if (IsDefaultPin(PIN)) {continue;}
			if (PIN->IsPendingKill()) {continue;}
			//
			if (PIN->Direction==EGPD_Output) {
				UFunction* Function_GET = nullptr;
				//
				bool IsArray = false;
				bool IsSet = false;
				bool IsMap = false;
				//
				const FString FieldName = PIN->GetName().Replace(TEXT("_"),TEXT(""));
				for (const auto &Info : ScriptSource->GetScriptData().PropertyInfo) {
					if (Info.Name.ToString()==FieldName) {PIN_Data=Info; break;}
				} if (PIN_Data.Name.IsNone()) {continue;}
				//
				switch(PIN_Data.ListType) {
					case EMonoListType::None:
					{
						switch(PIN_Data.DataType) {
							case EMonoDataType::Bool:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Bool"));
							} break;
							//
							case EMonoDataType::Byte:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Byte"));
							} break;
							//
							case EMonoDataType::Int:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Int"));
							} break;
							//
							case EMonoDataType::Int64:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Int64"));
							} break;
							//
							case EMonoDataType::Float:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Float"));
							} break;
							//
							case EMonoDataType::String:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_String"));
							} break;
							//
							case EMonoDataType::Name:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Name"));
							} break;
							//
							case EMonoDataType::Text:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Text"));
							} break;
							//
							case EMonoDataType::Vector2D:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Vector2D"));
							} break;
							//
							case EMonoDataType::Vector3D:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Vector3D"));
							} break;
							//
							case EMonoDataType::Rotator:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Rotator"));
							} break;
							//
							case EMonoDataType::Color:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Color"));
							} break;
							//
							case EMonoDataType::Transform:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Transform"));
							} break;
							//
							case EMonoDataType::Class:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Class"));
							} break;
							//
							case EMonoDataType::Object:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Object"));
							} break;
							//
							case EMonoDataType::Actor:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Actor"));
							} break;
							//
							case EMonoDataType::Component:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoProperty_Component"));
							} break;
						default: break;}
					} break;
					//
					case EMonoListType::Array:
					{
						IsArray = true;
						switch(PIN_Data.DataType) {
							case EMonoDataType::Bool:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Bool"));
							} break;
							//
							case EMonoDataType::Byte:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Byte"));
							} break;
							//
							case EMonoDataType::Int:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Int"));
							} break;
							//
							case EMonoDataType::Int64:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Int64"));
							} break;
							//
							case EMonoDataType::Float:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Float"));
							} break;
							//
							case EMonoDataType::String:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_String"));
							} break;
							//
							case EMonoDataType::Name:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Name"));
							} break;
							//
							case EMonoDataType::Text:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Text"));
							} break;
							//
							case EMonoDataType::Vector2D:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Vector2D"));
							} break;
							//
							case EMonoDataType::Vector3D:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Vector3D"));
							} break;
							//
							case EMonoDataType::Rotator:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Rotator"));
							} break;
							//
							case EMonoDataType::Color:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Color"));
							} break;
							//
							case EMonoDataType::Transform:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Transform"));
							} break;
							//
							case EMonoDataType::Class:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Class"));
							} break;
							//
							case EMonoDataType::Object:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Object"));
							} break;
							//
							case EMonoDataType::Actor:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Actor"));
							} break;
							//
							case EMonoDataType::Component:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoArray_Component"));
							} break;
						default: break;}
					} break;
					//
					case EMonoListType::Set:
					{
						IsSet = true;
						switch(PIN_Data.DataType) {
							case EMonoDataType::Byte:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Byte"));
							} break;
							//
							case EMonoDataType::Int:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Int"));
							} break;
							//
							case EMonoDataType::Int64:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Int64"));
							} break;
							//
							case EMonoDataType::Float:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Float"));
							} break;
							//
							case EMonoDataType::String:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_String"));
							} break;
							//
							case EMonoDataType::Name:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Name"));
							} break;
							//
							case EMonoDataType::Text:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Text"));
							} break;
							//
							case EMonoDataType::Vector2D:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Vector2D"));
							} break;
							//
							case EMonoDataType::Vector3D:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Vector3D"));
							} break;
							//
							case EMonoDataType::Color:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Color"));
							} break;
							//
							case EMonoDataType::Class:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Class"));
							} break;
							//
							case EMonoDataType::Object:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Object"));
							} break;
							//
							case EMonoDataType::Actor:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Actor"));
							} break;
							//
							case EMonoDataType::Component:
							{
								Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoSet_Component"));
							} break;
						default: break;}
					} break;
					//
					case EMonoListType::Map:
					{
						IsMap = true;
						switch(PIN_Data.DataType) {
							case EMonoDataType::Byte:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ByteComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Int:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_IntComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Int64:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Bool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Byte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Int"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Int64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Float"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64String"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Name"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Text"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Color"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Vector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Vector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Rotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Transform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Class"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Object"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Actor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Int64Component"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Float:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_FloatComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::String:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_StringComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Name:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_NameComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Vector2D:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector2DComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Vector3D:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_Vector3DComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Color:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ColorComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Class:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ClassComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Object:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ObjectComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Actor:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ActorComponent"));
									} break;
								default: break;}
							} break;
							//
							case EMonoDataType::Component:
							{
								switch(PIN_Data.SubType) {
									case EMonoDataType::Bool:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentBool"));
									} break;
									//
									case EMonoDataType::Byte:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentByte"));
									} break;
									//
									case EMonoDataType::Int:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentInt"));
									} break;
									//
									case EMonoDataType::Int64:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentInt64"));
									} break;
									//
									case EMonoDataType::Float:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentFloat"));
									} break;
									//
									case EMonoDataType::String:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentString"));
									} break;
									//
									case EMonoDataType::Name:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentName"));
									} break;
									//
									case EMonoDataType::Text:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentText"));
									} break;
									//
									case EMonoDataType::Color:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentColor"));
									} break;
									//
									case EMonoDataType::Vector2D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentVector2D"));
									} break;
									//
									case EMonoDataType::Vector3D:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentVector3D"));
									} break;
									//
									case EMonoDataType::Rotator:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentRotator"));
									} break;
									//
									case EMonoDataType::Transform:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentTransform"));
									} break;
									//
									case EMonoDataType::Class:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentClass"));
									} break;
									//
									case EMonoDataType::Object:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentObject"));
									} break;
									//
									case EMonoDataType::Actor:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentActor"));
									} break;
									//
									case EMonoDataType::Component:
									{
										Function_GET = ExpandScriptCall(Instance.Get(),TEXT("GetMonoMap_ComponentComponent"));
									} break;
								default: break;}
							} break;
						default: break;}
					} break;
				default: break;}
				//
				UK2Node_CallFunction* CS_GET_Property = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(Self,SourceGraph);
				CS_GET_Property->SetFromFunction(Function_GET);
				CS_GET_Property->AllocateDefaultPins();
				//
				UEdGraphPin* PUT_Exec = CS_GET_Property->GetExecPin();
				UEdGraphPin* PUT_Then = CS_GET_Property->GetThenPin();
				UEdGraphPin* PUT_Field = CS_GET_Property->FindPinChecked(TEXT("Field"),EGPD_Input);
				UEdGraphPin* PUT_Output = CS_GET_Property->FindPinChecked(TEXT("Output"),EGPD_Output);
				UEdGraphPin* PUT_Node = CS_GET_Property->FindPinChecked(CSHelper::PN_Node,EGPD_Input);
				UEdGraphPin* PUT_Context = CS_GET_Property->FindPinChecked(CSHelper::PN_Context,EGPD_Input);
				//
				PUT_Output->PinType = PIN->PinType;
				PUT_Output->DefaultValue = PIN->DefaultValue;
				PUT_Output->DefaultObject = PIN->DefaultObject;
				PUT_Output->DefaultTextValue = PIN->DefaultTextValue;
				//
				CompilerContext.MovePinLinksToIntermediate(*PIN,*PUT_Output);
				CompilerContext.MovePinLinksToIntermediate(*PIN_Then,*PUT_Then);
				CompilerContext.CopyPinLinksToIntermediate(*PIN_Context,*PUT_Context);
				//
				GetSchema()->TrySetDefaultValue(*PUT_Field,FieldName);
				GetSchema()->TryCreateConnection(PIN_Node,PUT_Node);
				GetSchema()->TryCreateConnection(PIN_Then,PUT_Exec);
				//
				PIN_Exec = PUT_Exec;
				PIN_Then = PUT_Then;
			}///
		}///
		//
		//
		if (!ScriptSource->GetScriptData().Asynchronous) {
			UK2Node_CallFunction* CS_Exit = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(Self,SourceGraph);
			CS_Exit->SetFromFunction(Function_EXIT);
			CS_Exit->AllocateDefaultPins();
			{
				UEdGraphPin* PUT_Exec = CS_Exit->GetExecPin();
				UEdGraphPin* PUT_Then = CS_Exit->GetThenPin();
				UEdGraphPin* PUT_Node = CS_Exit->FindPinChecked(CSHelper::PN_Node,EGPD_Input);
				UEdGraphPin* PUT_Context = CS_Exit->FindPinChecked(CSHelper::PN_Context,EGPD_Input);
				//
				CompilerContext.MovePinLinksToIntermediate(*PIN_Then,*PUT_Then);
				CompilerContext.CopyPinLinksToIntermediate(*PIN_Context,*PUT_Context);
				//
				GetSchema()->TryCreateConnection(PIN_Node,PUT_Node);
				GetSchema()->TryCreateConnection(PIN_Then,PUT_Exec);
				PIN_Exec = PUT_Exec; PIN_Then = PUT_Then;
			}
		}///
	}///
	//
	//
	Instance.Reset();
	Self->BreakAllNodeLinks();
}

UFunction* UKCS_MagicNode::ExpandScriptCall(const UMagicNodeSharp* Script, const FName &Function) {
	if (Script==nullptr) {return nullptr;}
	//
	return Script->FindFunction(Function);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>&OldPins) {
	AllocateDefaultPins();
	//
	CreatePinsForScript(GetScriptSource());
	//
	RestoreSplitPins(OldPins);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::OnScriptRuntimeException(UMagicNodeSharpSource* Script, const TCHAR* Error) {
	if (!HasScript()) {return;}
	//
	if (Script==GetScriptSource()) {
		bCommentBubbleVisible = true;
		NodeComment = FString(Error);
		bCommentBubblePinned = true;
		//
		FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(this);
	}///
}

void UKCS_MagicNode::OnScriptPinChanged() {
	UMagicNodeSharpSource* Script = GetScriptSource();
	if (Script==nullptr) {return;}
	//
	const FMonoScriptData &Data = Script->GetScriptData();
	//
	for (int32 I=Pins.Num()-1; I>=0; --I) {
		if (IsDefaultPin(Pins[I])) {continue;}
		if (Pins[I]->IsPendingKill()) {Pins[I]->BreakAllPinLinks(true);}
		//
		bool IsValid = false;
		for (const auto &Info : Data.PropertyInfo) {
			if (Info.Name.ToString()==Pins[I]->GetName().Replace(TEXT("_"),TEXT(""))) {IsValid=true; break;}
		}///
		//
		if (!IsValid) {
			UEdGraphPin* Pin = Pins[I];
			Pin->BreakAllPinLinks(true);
			Pins.Remove(Pin);
			DestroyPin(Pin);
		}///
	}///
	//
	AllocateDefaultPins();
	CreatePinsForScript(Script);
	//
	UEdGraph* Graph = GetGraph();
	Graph->NotifyGraphChanged();
	//
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::PinConnectionListChanged(UEdGraphPin* ChangedPin) {
	if (ChangedPin && (ChangedPin->PinName==CSHelper::PN_Script)) {ReloadScript(); return;}
	//
	if (ChangedPin) {
		if (ScriptNodeWidget.IsValid()) {
			ScriptNodeWidget->UpdateGraphNode();
		}///
	}///
}

void UKCS_MagicNode::PinDefaultValueChanged(UEdGraphPin* ChangedPin) {
	if (ChangedPin && (ChangedPin->PinName==CSHelper::PN_Script)) {ReloadScript(); return;}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::CreatePinsForScript(UMagicNodeSharpSource* Script) {
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	if (Script==nullptr||!Script->IsValidLowLevelFast()) {return;}
	//
	//
	if (Script==nullptr) {
		for (int32 I=Pins.Num()-1; I>=0; --I) {
			UEdGraphPin* Pin = Pins[I];
			//
			if (!IsDefaultPin(Pin)) {
				Pin->BreakAllPinLinks();
				Pins.Remove(Pin);
				DestroyPin(Pin);
			}///
		} return;
	}///
	//
	TArray<UEdGraphPin*>NewClassPins;
	TArray<UEdGraphPin*>OldClassPins;
	TArray<UEdGraphPin*>OldPins=Pins;
	//
	for (int32 I=0; I<OldPins.Num(); I++) {
		UEdGraphPin* OldPin = OldPins[I];
		//
		if (!IsDefaultPin(OldPin)) {
			OldClassPins.Add(OldPin);
			Pins.Remove(OldPin);
		}///
	}///
	//
	//
	Script->LoadIntermediateData();
	for (auto Info : Script->GetScriptData().PropertyInfo) {
		if (Info.ScopeType==EMonoScopeType::Static) {
			if (auto*Dirty=FindPin(Info.Name)) {
				Dirty->BreakAllPinLinks();
			} continue;
		}///
		//
		if (Info.ParamType==EMonoParamType::Input) {
			if ((Info.DataType!=EMonoDataType::Void)&&(Info.DataType!=EMonoDataType::Unknown)) {
				if (FindPin(Info.Name)==nullptr) {
					if (UEdGraphPin*Pin=CreatePin(EGPD_Input,NAME_None,Info.Name)) {
						SetFieldArchetype(Pin,Info.DataType,Info.SubType,Info.ListType);
						NewClassPins.Add(Pin);
					}///
				}///
			}///
			//
			if (FindPin(Info.Name)) {SetPinDefaultValue(FindPin(Info.Name),Info.DataType,Info.Value);}
			if (FindPin(Info.Name)) {KSchema->ConstructBasicPinTooltip(*FindPin(Info.Name),FText::FromString(Info.Description),FindPin(Info.Name)->PinToolTip);}
		} else if (Info.ParamType==EMonoParamType::Output) {
			if ((Info.DataType!=EMonoDataType::Void)&&(Info.DataType!=EMonoDataType::Unknown)) {
				if (FindPin(Info.Name)==nullptr) {
					if (UEdGraphPin*Pin=CreatePin(EGPD_Output,NAME_None,Info.Name)) {
						SetFieldArchetype(Pin,Info.DataType,Info.SubType,Info.ListType);
						NewClassPins.Add(Pin);
					}///
				}///
			}///
			//
			if (FindPin(Info.Name)) {KSchema->ConstructBasicPinTooltip(*FindPin(Info.Name),FText::FromString(Info.Description),FindPin(Info.Name)->PinToolTip);}
		} else if (Info.ParamType==EMonoParamType::InOut) {
			FString In = Info.Name.ToString(); In.InsertAt(0,TEXT("_"));
			FString Out = Info.Name.ToString().Append(TEXT("_"));
			//
			if ((Info.DataType!=EMonoDataType::Void)&&(Info.DataType!=EMonoDataType::Unknown)) {
				if (FindPin(*In)==nullptr) {
					if (UEdGraphPin*Pin=CreatePin(EGPD_Input,NAME_None,*In)) {
						SetFieldArchetype(Pin,Info.DataType,Info.SubType,Info.ListType);
						NewClassPins.Add(Pin);
					}///
				}///
			}///
			//
			if ((Info.DataType!=EMonoDataType::Void)&&(Info.DataType!=EMonoDataType::Unknown)) {
				if (FindPin(*Out)==nullptr) {
					if (UEdGraphPin*Pin=CreatePin(EGPD_Output,NAME_None,*Out)) {
						SetFieldArchetype(Pin,Info.DataType,Info.SubType,Info.ListType);
						NewClassPins.Add(Pin);
					}///
				}///
			}///
			//
			if (FindPin(*In)) {SetPinDefaultValue(FindPin(*In),Info.DataType,Info.Value);}
			if (FindPin(*In)) {KSchema->ConstructBasicPinTooltip(*FindPin(*In),FText::FromString(Info.Description),FindPin(*In)->PinToolTip);}
			if (FindPin(*Out)) {KSchema->ConstructBasicPinTooltip(*FindPin(*Out),FText::FromString(Info.Description),FindPin(*Out)->PinToolTip);}
		}///
	}///
	//
	//
	TMap<UEdGraphPin*,UEdGraphPin*>NewPins;
	RewireOldPinsToNewPins(OldClassPins,NewClassPins,&NewPins);
	//
	for (int32 I=0; I<OldClassPins.Num(); I++) {
		OldClassPins[I]->BreakAllPinLinks();
	} DestroyPinList(OldClassPins);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::PostReconstructNode() {
	CollapsePanel = true;
	//
	AllocateDefaultPins();
	CreatePinsForScript(GetScriptSource());
	//
	UMagicNodeSharp::OnScriptRuntimeException.AddUObject(this,&UKCS_MagicNode::OnScriptRuntimeException);
	//
	Super::PostReconstructNode();
}

void UKCS_MagicNode::PostPlacedNewNode() {
	CollapsePanel = true;
	//
	AllocateDefaultPins();
	CreatePinsForScript(GetScriptSource());
	//
	Super::PostPlacedNewNode();
}

void UKCS_MagicNode::PostInitProperties() {
	Super::PostInitProperties();
	//
	if (CollapsePanel) {
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
	} else {AdvancedPinDisplay=ENodeAdvancedPins::Shown;}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::SetFieldArchetype(UEdGraphPin* Field, const EMonoDataType &TypeInfo, const EMonoDataType &SubTypeInfo, const EMonoListType &ListInfo) {
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	//
	Field->PinType = FEdGraphPinType(KSchema->PC_Wildcard,TEXT(""),nullptr,EPinContainerType::None,false,FEdGraphTerminalType());
	//
	switch(ListInfo) {
		case EMonoListType::None:
		{
			switch(TypeInfo) {
				case EMonoDataType::Bool:
				{
					auto Property = FindFieldChecked<FBoolProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Bool"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Byte:
				{
					auto Property = FindFieldChecked<FByteProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Byte"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Int:
				{
					auto Property = FindFieldChecked<FIntProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Int64:
				{
					auto Property = FindFieldChecked<FInt64Property>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Float:
				{
					auto Property = FindFieldChecked<FFloatProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Float"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::String:
				{
					auto Property = FindFieldChecked<FStrProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_String"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Name:
				{
					auto Property = FindFieldChecked<FNameProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Name"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Text:
				{
					auto Property = FindFieldChecked<FTextProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Text"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Vector2D:
				{
					auto Property = FindFieldChecked<FStructProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2D"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Vector3D:
				{
					auto Property = FindFieldChecked<FStructProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3D"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Rotator:
				{
					auto Property = FindFieldChecked<FStructProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Rotator"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Color:
				{
					auto Property = FindFieldChecked<FStructProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Color"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Transform:
				{
					auto Property = FindFieldChecked<FStructProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Transform"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Class:
				{
					auto Property = FindFieldChecked<FClassProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Class"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Object:
				{
					auto Property = FindFieldChecked<FObjectProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Object"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Actor:
				{
					Field->PinType = FEdGraphPinType(KSchema->PC_Object,TEXT(""),AActor::StaticClass(),EPinContainerType::None,false,FEdGraphTerminalType());
				} break;
				//
				case EMonoDataType::Component:
				{
					Field->PinType = FEdGraphPinType(KSchema->PC_Object,TEXT(""),UActorComponent::StaticClass(),EPinContainerType::None,false,FEdGraphTerminalType());
				} break;
			default: break;}
		} break;
		//
		case EMonoListType::Array:
		{
			switch(TypeInfo) {
				case EMonoDataType::Bool:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_BoolArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Byte:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Int:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Int64:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64Array"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Float:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::String:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Name:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Text:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_TextArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Vector2D:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Vector3D:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Rotator:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_RotatorArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Color:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Transform:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_TransformArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Class:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Object:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Actor:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Component:
				{
					auto Property = FindFieldChecked<FArrayProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentArray"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
			default: break;}
		} break;
		//
		case EMonoListType::Set:
		{
			switch(TypeInfo) {
				case EMonoDataType::Byte:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Int:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Int64:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64Set"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Float:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::String:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Name:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Vector2D:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Vector3D:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Color:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Class:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Object:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Actor:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
				//
				case EMonoDataType::Component:
				{
					auto Property = FindFieldChecked<FSetProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentSet"));
					KSchema->ConvertPropertyToPinType(Property,Field->PinType);
				} break;
			default: break;}
		} break;
		//
		case EMonoListType::Map:
		{
			switch(TypeInfo) {
				case EMonoDataType::Byte:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ByteComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Int:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_IntComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Int64:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64BoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64ByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64IntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64Int64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64FloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64StringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64NameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64TextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64ColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64Vector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64Vector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64RotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64TransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64ClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64ObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64ActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Int64ComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Float:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_FloatComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::String:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_StringComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Name:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_NameComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Vector2D:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector2DComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Vector3D:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_Vector3DComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Color:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ColorComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Class:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ClassComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Object:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ObjectComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Actor:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ActorComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
				//
				case EMonoDataType::Component:
				{
					switch(SubTypeInfo) {
						case EMonoDataType::Bool:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentBoolMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Byte:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentByteMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentIntMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Int64:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentInt64Map"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Float:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentFloatMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::String:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentStringMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Name:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentNameMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Text:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentTextMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Color:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentColorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector2D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentVector2DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Vector3D:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentVector3DMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Rotator:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentRotatorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Transform:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentTransformMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Class:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentClassMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Object:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentObjectMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Actor:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentActorMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
						//
						case EMonoDataType::Component:
						{
							auto Property = FindFieldChecked<FMapProperty>(UMagicNodeSharpMarshal::StaticClass(),TEXT("_ComponentComponentMap"));
							KSchema->ConvertPropertyToPinType(Property,Field->PinType);
						} break;
					default: break;}
				} break;
			default: break;}
		} break;
	default: break;}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UEdGraphPin* UKCS_MagicNode::GetExecPin() const {
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	//
	UEdGraphPin* Pin = FindPin(KSchema->PN_Execute,EGPD_Input);
	check(Pin==nullptr||Pin->Direction==EGPD_Input); return Pin;
}

UEdGraphPin* UKCS_MagicNode::GetThenPin() const {
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	//
	UEdGraphPin* Pin = FindPin(KSchema->PN_Then,EGPD_Output);
	check(Pin==nullptr||Pin->Direction==EGPD_Output); return Pin;
}

UEdGraphPin* UKCS_MagicNode::GetReturnPin() const {
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	//
	UEdGraphPin* Pin = FindPin(KSchema->PN_ReturnValue,EGPD_Output);
	check(Pin==nullptr||Pin->Direction==EGPD_Output); return Pin;
}

UEdGraphPin* UKCS_MagicNode::GetScriptPin(const TArray<UEdGraphPin*>* InPinsToSearch) const {
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;
	//
	UEdGraphPin* Pin = nullptr;
	for (auto IT = PinsToSearch->CreateConstIterator(); IT; ++IT) {
		UEdGraphPin* Source = *IT;
		if (Source && (Source->PinName==CSHelper::PN_Script)) {Pin=Source; break;}
	} check(Pin==nullptr||Pin->Direction==EGPD_Input);
	//
	return Pin;
}

UEdGraphPin* UKCS_MagicNode::GetContextPin(const TArray<UEdGraphPin*>* InPinsToSearch) const {
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;
	//
	UEdGraphPin* Pin = nullptr;
	for (auto IT = PinsToSearch->CreateConstIterator(); IT; ++IT) {
		UEdGraphPin* Source = *IT;
		if (Source && (Source->PinName==CSHelper::PN_Context)) {Pin=Source; break;}
	} check(Pin==nullptr||Pin->Direction==EGPD_Input);
	//
	return Pin;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UKCS_MagicNode::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const  {
	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
	//
	return (
		Super::IsCompatibleWithGraph(TargetGraph) &&
		(
			(Blueprint==nullptr) ||
			(
				(Blueprint->GeneratedClass) &&
				(Blueprint->GeneratedClass->GetDefaultObject()) &&
				(Blueprint->GeneratedClass->GetDefaultObject()->ImplementsGetWorld()) &&
				(FBlueprintEditorUtils::FindUserConstructionScript(Blueprint)!=TargetGraph)
			)
		)
	);//
}

bool UKCS_MagicNode::IsDefaultPin(UEdGraphPin* Pin) {
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	//
	return (
		Pin->PinName == KSchema->PN_Then ||
		Pin->PinName == KSchema->PN_Execute ||
		Pin->PinName == CSHelper::PN_Script ||
		Pin->PinName == CSHelper::PN_Context
	);//
}

bool UKCS_MagicNode::IsDefaultProp(const FName &Name) {
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	//
	return (
		Name == KSchema->PN_Then ||
		Name == KSchema->PN_Execute ||
		Name == CSHelper::PN_Script ||
		Name == CSHelper::PN_Context
	);//
}

bool UKCS_MagicNode::IsDefaultParam(const TFieldPath<FProperty>Param) {
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	//
	return (
		Param->GetFName()==KSchema->PN_Then ||
		Param->GetFName()==KSchema->PN_Execute ||
		Param->GetFName()==CSHelper::PN_Script ||
		Param->GetFName()==CSHelper::PN_Context
	);//
}

bool UKCS_MagicNode::HasScript() const {
	return (
		(GetGraph()!=nullptr) &&
		(GetScriptSource()!=nullptr)
	);//
}

bool UKCS_MagicNode::CanCompileScript() const {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	const auto &BP = GetBlueprint();
	//
	return (
		HasScript() && MonoKismet.CanCompile() &&
		(BP && BP->ParentClass && (!BP->bBeingCompiled)) &&
		(GEditor==nullptr||!GEditor->IsPlaySessionInProgress())
	);//
}

bool UKCS_MagicNode::IsCompiling() const {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	return MonoKismet.IsCompiling();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FName UKCS_MagicNode::GetCornerIcon() const {
	return TEXT("KCS.MagicNodeSharpIcon");
}

FText UKCS_MagicNode::GetTooltipText() const {
	return NodeTooltip;
}

FLinearColor UKCS_MagicNode::GetNodeTitleColor() const {
	if (!HasScript()) {return FLinearColor(FColor(5,15,25));}
	//
	UMagicNodeSharpSource* Script = GetScriptSource();
	//
	return Script->NodeColor;
}

FSlateIcon UKCS_MagicNode::GetIconAndTint(FLinearColor &OutColor) const {
	static FSlateIcon Icon(TEXT("MagicNodeSharpEditorStyle"),TEXT("ClassIcon.MagicNodeSharpSource"));
	//
	return Icon;
}

FText UKCS_MagicNode::GetMenuCategory() const {
	return LOCTEXT("MagicNodeCategory","MagicNode");
}

void UKCS_MagicNode::GetMenuActions(FBlueprintActionDatabaseRegistrar &ActionRegistrar) const {
	UClass* ActionKey = GetClass();
	//
	if (ActionRegistrar.IsOpenForRegistration(ActionKey)) {
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		if (NodeSpawner==nullptr) {return;}
		//
		ActionRegistrar.AddBlueprintAction(ActionKey,NodeSpawner);
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UMagicNodeSharpSource* UKCS_MagicNode::GetScriptSource() const {
	const TArray<UEdGraphPin*>* PinsToSearch = &Pins;
	//
	UEdGraphPin* PIN_Script = GetScriptPin(PinsToSearch);
	UMagicNodeSharpSource* Source = nullptr;
	//
	if (PIN_Script==nullptr) {return nullptr;}
	Source = Cast<UMagicNodeSharpSource>(PIN_Script->DefaultObject);
	//
	return Source;
}

UMagicNodeSharp* UKCS_MagicNode::GetScriptCDO() {
	UMagicNodeSharpSource* Source = GetScriptSource();
	//
	if (Source==nullptr) {return nullptr;}
	if (!Instance.IsValid()||Instance.IsStale()) {
		const FName ID = MakeUniqueObjectName(GetTransientPackage(),UMagicNodeSharp::StaticClass(),*Source->GetScriptName());
		Instance = NewObject<UMagicNodeSharp>(GetTransientPackage(),UMagicNodeSharp::StaticClass(),ID,RF_Transient);
	}///
	//
	return Instance.Get();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const TCHAR* UKCS_MagicNode::GetScriptText() const {
	UMagicNodeSharpSource* Script = GetScriptSource();
	//
	if (Script==nullptr) {return TEXT("");}
	//
	return *Script->GetSource();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::SetScriptSource(UMagicNodeSharpSource* New) {
	const TArray<UEdGraphPin*>* PinsToSearch = &Pins;
	//
	UEdGraphPin* PIN_Script = GetScriptPin(PinsToSearch);
	if (PIN_Script==nullptr) {return;}
	//
	PIN_Script->DefaultObject = New;
	if (New) {NodeTooltip=FText::FromString(New->GetSource());}
	//
	ReloadScript();
}

void UKCS_MagicNode::SetTooltip(const FString &New) {
	if (!ErrorMsg.IsEmpty()) {
		NodeTooltip = FText::FromString(ErrorMsg);
	} if (!HasScript()) {return;}
	//
	NodeTooltip = FText::FromString(New);
}

void UKCS_MagicNode::SetPinDefaultValue(UEdGraphPin* Pin, const EMonoDataType DataType, const FString &NewValue) const {
	const UEdGraphSchema_K2* KSchema = GetDefault<UEdGraphSchema_K2>();
	//
	if (NewValue.IsEmpty()) {return;}
	//
	if (DataType==EMonoDataType::Bool) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Byte) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Int) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Int64) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Float) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::String) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Name) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Text) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Color) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Class) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Object) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Actor) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);} else
	if (DataType==EMonoDataType::Component) {KSchema->SetPinAutogeneratedDefaultValue(Pin,NewValue);}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::CompileScript() {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	NodeComment.Empty();
	bCommentBubblePinned = false;
	bCommentBubbleVisible = false;
	//
	FCompilerResults Fail;
	Fail.Result = EMonoCompilerResult::Error;
	Fail.ErrorMessage = TEXT("{C#}:: Compiler unreachable, Script is invalid.");
	//
	auto* Source = GetScriptSource();
	if (GetBlueprint()==nullptr||Source==nullptr) {
		LOG::CS_CHAR(ESeverity::Error,TEXT("{C#}:: Compiler unreachable, Script is invalid.")); return;
	}///
	//
	if (!Source->IsValidLowLevel()) {
		ErrorMsg = Fail.ErrorMessage;
		GetBlueprint()->Message_Error(ErrorMsg);
		ScriptNodeWidget->SetErrorMessage(Fail); return;
	} else if (Source==UMagicNodeSharpSource::StaticClass()->ClassDefaultObject) {
		GetBlueprint()->Message_Error(TEXT("{C#}:: Script is invalid (referencing Base Script Class is not allowed).")); return;
	}///
	//
	if (!MonoKismet.MonoKismet_INIT()) {
		Fail.ErrorMessage = TEXT("{C#}:: Compiler unreachable, Mono is busy or not properly initialized.");
		//
		ErrorMsg = Fail.ErrorMessage;
		GetBlueprint()->Message_Error(ErrorMsg);
		ScriptNodeWidget->SetErrorMessage(Fail);
		//
		LOG::CS_STR(ESeverity::Error,ErrorMsg); return;
	}///
	//
	MonoKismet.CompilerResult.BindUObject(this,&UKCS_MagicNode::OnCompilationFinished);
	MonoKismet.CompileNode(Source,GetScriptCDO());
}

void UKCS_MagicNode::OnCompilationFinished(const UMagicNodeSharpSource* Script, const FCompilerResults Results) {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	if (!HasScript()||Script==nullptr) {return;}
	if (HasAnyFlags(RF_ClassDefaultObject|RF_ArchetypeObject|RF_BeginDestroyed)) {return;}
	//
	if (!Results.ErrorMessage.IsEmpty()) {
		switch (Results.Result) {
			case EMonoCompilerResult::Success:
			{
				if (GEditor) {
					GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
				}///
				//
				if (ScriptNodeWidget.IsValid()) {
					ScriptNodeWidget->SetErrorMessage(Results);
				}///
				//
				ReloadScript();
				ReconstructNode();
			} break;
			//
			case EMonoCompilerResult::Warning:
			{
				ErrorType = EMessageSeverity::Warning;
				//
				ErrorMsg = Results.ErrorMessage;
				SetTooltip(Results.ErrorMessage);
				GetBlueprint()->Message_Warn(Results.ErrorMessage);
				//
				NodeComment = ErrorMsg;
				bCommentBubbleVisible = true;
				//
				if (ScriptNodeWidget.IsValid()) {
					ScriptNodeWidget->SetErrorMessage(Results);
				}///
				//
				if (GEditor) {
					GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
				}///
			} break;
			//
			case EMonoCompilerResult::Error:
			{
				ErrorType = EMessageSeverity::Error;
				//
				ErrorMsg = Results.ErrorMessage;
				SetTooltip(Results.ErrorMessage);
				GetBlueprint()->Message_Error(Results.ErrorMessage);
				//
				NodeComment = ErrorMsg;
				bCommentBubblePinned = true;
				bCommentBubbleVisible = true;
				//
				if (ScriptNodeWidget.IsValid()) {
					ScriptNodeWidget->SetErrorMessage(Results);
				}///
				//
				if (GEditor) {
					GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
				}///
			}///
		break;}
	} else {
		if (GEditor) {GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));}
		//
		ErrorMsg.Empty();
		FCompilerResults Clean;
		//
		if (ScriptNodeWidget.IsValid()) {
			ScriptNodeWidget->SetErrorMessage(Clean);
		}///
		//
		ReloadScript();
		ReconstructNode();
	}///
	//
	MonoKismet.CompilerResult.Unbind();
	MonoKismet.MonoKismetDomain_STOP();
}

void UKCS_MagicNode::ReloadScript() {
	UMagicNodeSharpSource* Source = GetScriptSource();
	//
	if (Source==nullptr) {OnScriptPinChanged(); return;}
	if (GetBlueprint()==nullptr) {OnScriptPinChanged(); return;}
	//
	if (!Source->IsValidLowLevel()) {
		ErrorMsg = TEXT("{C#}:: Reload unreachable, Script is invalid.");
		GetBlueprint()->Message_Error(ErrorMsg);
		//
		ScriptNodeWidget->SetErrorMessage(FString::Printf(TEXT("{C#}:: %s"),*ErrorMsg));
	return;} else if (Source==UMagicNodeSharpSource::StaticClass()->ClassDefaultObject) {
		GetBlueprint()->Message_Error(TEXT("{C#}:: Reload unreachable, Script is invalid (referencing base Magic Node Script Class is not allowed)."));
	return;}
	//
	Source->LoadScript();
	//
	ErrorMsg.Empty();
	SetTooltip(TEXT(""));
	GetBlueprint()->Message_Note(TEXT(""));
	//
	NodeComment.Empty();
	bCommentBubblePinned = false;
	bCommentBubbleVisible = false;
	//
	OnScriptPinChanged();
	{
		const auto &GP = GetGraph();
		//
		for (const auto &Node : GP->Nodes) {
			if (!Node->IsA(UKCS_MagicNode::StaticClass())) {continue;}
			const auto &NCS = CastChecked<UKCS_MagicNode>(Node);
			//
			if (NCS==this) {continue;}
			if (!NCS->HasScript()) {continue;}
			if (NCS->GetScriptSource()!=GetScriptSource()) {continue;}
			//
			NCS->OnScriptPinChanged();
		}///
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UKCS_MagicNode::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) {
	const FName PName = (PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None);
	//
	///if (!IsDefaultProp(PName)) {ReconstructNode(); return;}
	//
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSharedPtr<SGraphNode>UKCS_MagicNode::CreateVisualWidget() {
	return SAssignNew(ScriptNodeWidget,SKCS_MagicNodeWidget,this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////