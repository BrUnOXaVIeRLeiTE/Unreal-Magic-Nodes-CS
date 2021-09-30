//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MagicNodeSharp.h"
#include "MCS_Types.h"

#include "KCS_NodeWidget.h"

#include "K2Node.h"

#include "Textures/SlateIcon.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNodeUtils.h"

#include "KCS_MagicNode.generated.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UEdGraph;
class IMonoKismet;
class FBlueprintActionDatabaseRegistrar;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class MAGICNODESHARPKISMET_API UKCS_MagicNode : public UK2Node {
	GENERATED_UCLASS_BODY()
	//
	friend class IMonoKismet;
protected:
	UPROPERTY() bool CollapsePanel;
protected:
	TWeakObjectPtr<UMagicNodeSharp>Instance = nullptr;
protected:
	TSharedPtr<SKCS_MagicNodeWidget>ScriptNodeWidget;
	FNodeTextCache CachedNodeTitle;
	FText NodeTooltip;
protected:
	void SetPinDefaultValue(UEdGraphPin* Pin, const EMonoDataType DataType, const FString& NewValue) const;
protected:
	void OnScriptPinChanged();
	void OnScriptRuntimeException(UMagicNodeSharpSource* Script, const TCHAR* Error);
	void OnCompilationFinished(const UMagicNodeSharpSource* Script, const FCompilerResults Results);
public:
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>&OldPins) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
	virtual void ExpandNode(FKismetCompilerContext &CompilerContext, UEdGraph* SourceGraph) override;
public:
	virtual UFunction* ExpandScriptCall(const UMagicNodeSharp* Script, const FName &Function);
public:
	virtual void PostPlacedNewNode() override;
	virtual void PostInitProperties() override;
	virtual void PostReconstructNode() override;
public:
	virtual void AllocateDefaultPins() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar &ActionRegistrar) const override;
public:
	virtual bool ShowPaletteIconOnNode() const override {return true;}
	virtual bool ShouldShowNodeProperties() const override {return true;}
	virtual bool NodeCausesStructuralBlueprintChange() const override {return true;}
public:
	virtual FName GetCornerIcon() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor &OutColor) const override;
public:
	virtual TSharedPtr<SGraphNode>CreateVisualWidget() override;
public:
	bool HasScript() const;
	bool IsCompiling() const;
	bool CanCompileScript() const;
	bool IsDefaultPin(UEdGraphPin* Pin);
	bool IsDefaultProp(const FName &Name);
	bool IsDefaultParam(const TFieldPath<FProperty>Param);
public:
	void CreatePinsForScript(UMagicNodeSharpSource* Script);
public:
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetReturnPin() const;
	UEdGraphPin* GetScriptPin(const TArray<UEdGraphPin*>*InPinsToSearch=nullptr) const;
	UEdGraphPin* GetContextPin(const TArray<UEdGraphPin*>*InPinsToSearch=nullptr) const;
public:
	const TCHAR* GetScriptText() const;
public:
	UMagicNodeSharp* GetScriptCDO();
	UMagicNodeSharpSource* GetScriptSource() const;
public:
	void SetTooltip(const FString &New);
	void SetScriptSource(UMagicNodeSharpSource* New);
	void SetFieldArchetype(UEdGraphPin* Field, const EMonoDataType &TypeInfo, const EMonoDataType &SubTypeInfo, const EMonoListType &ListInfo);
public:
	void CompileScript();
	void ReloadScript();
public:
	//UPROPERTY(Category="Source",EditAnywhere)
	//int32 TempNodeVarTEST;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////