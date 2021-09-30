//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MagicNodeSharpEditor.h"

#include "CS_Toolkit.h"

#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/SlateCore/Public/Widgets/SOverlay.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

#include "Kismet2/KismetEditorUtilities.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FATA_MagicNodeSharp::FATA_MagicNodeSharp(const TSharedRef<ISlateStyle>&InStyle) : ActionStyle(InStyle){}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCS_ScriptFactory::UCS_ScriptFactory(const class FObjectInitializer &OBJ) : Super(OBJ) {
	Formats.Add(FString(TEXT("cs;"))+NSLOCTEXT("UCS_ScriptFactory","FormatCS","CS Script").ToString());
	SupportedClass = UMagicNodeSharpSource::StaticClass();
	bEditAfterNew = false;
	bEditorImport = true;
	bCreateNew = false;
}

UCS_ScriptFactoryNew::UCS_ScriptFactoryNew(const class FObjectInitializer &OBJ) : Super(OBJ) {
	SupportedClass = UMagicNodeSharpSource::StaticClass();
	bEditorImport = false;
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UCS_ScriptFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString &Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool &bOutOperationCanceled) {
	UMagicNodeSharpSource* Source = nullptr;
	FString ScriptData;
	//
	if (FFileHelper::LoadFileToString(ScriptData,*Filename)) {
		Source = NewObject<UMagicNodeSharpSource>(InParent,InClass,InName,Flags);
		Source->SetSource(ScriptData);
	}///
	//
	bOutOperationCanceled = false;
	//
	return Source;
}

UObject* UCS_ScriptFactoryNew::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) {
	check(Class->IsChildOf(UMagicNodeSharpSource::StaticClass()));
	//
	return NewObject<UMagicNodeSharpSource>(InParent,Class,Name,Flags,Context);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FText FATA_MagicNodeSharp::GetAssetDescription(const FAssetData &AssetData) const {
	return FText::FromString(FString(TEXT("Magic Node Script (C#).")));
}

void FATA_MagicNodeSharp::OpenAssetEditor(const TArray<UObject*> &InObjects, TSharedPtr<IToolkitHost>EditWithinLevelEditor) {
	for (auto IT = InObjects.CreateConstIterator(); IT; ++IT) {
		if (UMagicNodeSharpSource* ScriptObject = Cast<UMagicNodeSharpSource>(*IT)) {
			TSharedRef<FCS_Toolkit>CodeEditor(new FCS_Toolkit());
			//
			CodeEditor->INIT(EToolkitMode::Standalone,EditWithinLevelEditor,ScriptObject);
		}///
	}///
}

void FATA_MagicNodeSharp::GetActions(const TArray<UObject*>&InObjects, FMenuBuilder &MenuBuilder) {
	FAssetTypeActions_Base::GetActions(InObjects,MenuBuilder);
	//
	auto ScriptAssets = GetTypedWeakObjectPtrs<UMagicNodeSharpSource>(InObjects);
/*	//
	MenuBuilder.AddMenuEntry(
		LOCTEXT("TextAsset_ReverseText", "Reverse Text"),
		LOCTEXT("TextAsset_ReverseTextToolTip", "Reverse the text stored in the selected text asset(s)."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([=]{
				for (auto& TextAsset : ScriptAssets)
				{
					if (TextAsset.IsValid() && !TextAsset->Text.IsEmpty())
					{
						TextAsset->Text = FText::FromString(TextAsset->Text.ToString().Reverse());
						TextAsset->PostEditChange();
						TextAsset->MarkPackageDirty();
					}
				}
			}),
			//
			FCanExecuteAction::CreateLambda([=] {
				for (auto& TextAsset : ScriptAssets) {
					if (TextAsset.IsValid() && !TextAsset->Text.IsEmpty()) {return true;}
				} return false;
			})//
		)///
	);///
*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////