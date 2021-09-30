//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "IMagicNodeSharpEditor.h"
#include "IContentBrowserSingleton.h"

#include "DiffTool_MainWidget.h"
#include "MagicNodeSharp.h"

#include "CS_Toolkit.h"
#include "CS_EditorStyle.h"
#include "CS_ActionStyle.h"
#include "DiffTool_EditorStyle.h"

#include "LevelEditor.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
T* GetCDO(IDetailLayoutBuilder* DetailBuilder) {
	TArray<TWeakObjectPtr<UObject>>OutObjects;
	//
	DetailBuilder->GetObjectsBeingCustomized(OutObjects);
	//
	T* OBJ = nullptr;
	if (OutObjects.Num()>0) {
		OBJ = Cast<T>(OutObjects[0].Get());
	} return OBJ;
}

template<typename T>
T* GetBlueprintCDO(UBlueprint* Blueprint) {
	if (Blueprint==nullptr) {return nullptr;}
	//
	const UClass* BlueprintClass = Blueprint->GeneratedClass;
	return Cast<T>(BlueprintClass->ClassDefaultObject);
}

UBlueprint* GetBlueprintFromCDO(UObject* OBJ) {
	UBlueprint* BP = nullptr;
	//
	if (OBJ != nullptr) {
		BP = Cast<UBlueprint>(OBJ);
		//
		if (BP == nullptr) {
			BP = Cast<UBlueprint>(OBJ->GetClass()->ClassGeneratedBy);
		}///
	} return BP;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FMagicNodeSharpEditor::StartupModule() {
	FMagicNodeSharpEditorStyle::Initialize();
	FDiffToolEditorStyle::Initialize();
	FCSharpActionStyle::Initialize();
	//
	FMagicNodeSharpEditorCommands::Register();
	//
	//
	FLevelEditorModule &LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	FPropertyEditorModule &PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		PropertyEditorModule.RegisterCustomClassLayout("MagicNodeSharpSource",FOnGetDetailCustomizationInstance::CreateStatic(&FMagicNodeSharpCustomDetails::MakeInstance));
	}
	//
	IAssetTools &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	SY_AssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Synaptech")),LOCTEXT("SynaptechCategory","Synaptech"));
	//
	{
		TSharedRef<IAssetTypeActions>ACT_CS = MakeShareable(new FATA_MagicNodeSharp(FCSharpActionStyle::Get().ToSharedRef()));
		AssetTools.RegisterAssetTypeActions(ACT_CS);
	}
	//
	//
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
	//
	MainMenuExtender = MakeShareable(new FExtender());
	MainMenuExtender->AddMenuExtension("FileProject",EExtensionHook::After,TSharedPtr<FUICommandList>(),FMenuExtensionDelegate::CreateStatic(&FMagicNodeSharpEditor::ExtendMainMenu));
	//
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MainMenuExtender);
	//
	{
		FGlobalTabmanager::Get()->RegisterTabSpawner(DIFFToolTAB,FOnSpawnTab::CreateRaw(this,&FMagicNodeSharpEditor::OnSpawnDiffToolTab))
		.SetDisplayName(LOCTEXT("DIFFToolTAB_Title","Compare")).SetMenuType(ETabSpawnerMenuType::Hidden);
	}
	//
	//
	FCS_Toolkit::RefreshScriptTreeView();
}

void FMagicNodeSharpEditor::ShutdownModule() {
	MainMenuExtender.Reset();
	//
	FMagicNodeSharpEditorCommands::Unregister();
	FMagicNodeSharpEditorStyle::Shutdown();
	FDiffToolEditorStyle::Shutdown();
	FCSharpActionStyle::Shutdown();
	//
	FGlobalTabmanager::Get()->UnregisterTabSpawner(DIFFToolTAB);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FMagicNodeSharpEditor::ExtendMainMenu(FMenuBuilder &MenuBuilder) {
	MenuBuilder.AddMenuEntry (
		LOCTEXT("CS_NewScriptAsset","New Magic Node (C#)..." ),
		LOCTEXT("CS_NewScriptAsset_Tooltip","Create a new 'Magic Node' script class.\nA scriptable node that can execute C# libraries."),
		FSlateIcon(FMagicNodeSharpEditorStyle::Get().Get()->GetStyleSetName(),"ClassIcon.MagicNodeSharpSource"),
		FUIAction(FExecuteAction::CreateStatic(&FMagicNodeSharpEditor::CreateNewScriptAsset))
	);//
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FMagicNodeSharpEditor::CreateNewScriptAsset() {
	IAssetTools &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	//
	FString PackageName = FString(TEXT("/Game/"));
	FString AssetName = FString(TEXT("CS_Script"));
	AssetTools.CreateUniqueAssetName(PackageName,AssetName,PackageName,AssetName);
	//
	UPackage* Package = CreatePackage(*PackageName);
	UPackage* OuterPack = Package->GetOutermost();
	//
	auto ScriptFactory = NewObject<UCS_ScriptFactoryNew>();
	UObject* NewScript = ScriptFactory->FactoryCreateNew(UMagicNodeSharpSource::StaticClass(),OuterPack,*AssetName,RF_Standalone|RF_Public,nullptr,GWarn);
	//
	FAssetRegistryModule::AssetCreated(NewScript);
	//
	NewScript->MarkPackageDirty();
	NewScript->PostEditChange();
	NewScript->AddToRoot();
	//
	Package->SetDirtyFlag(true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UObject* FMagicNodeSharpEditor::GetSelectedAsset() {
	FContentBrowserModule &ContentBrowser = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	//
	TArray<FAssetData>Selection;
	ContentBrowser.Get().GetSelectedAssets(Selection);
	//
	for (const FAssetData &SLC : Selection) {
		if (SLC.GetAsset()==nullptr) {continue;}
		//
		return SLC.GetAsset();
	} return nullptr;
}

UObject* FMagicNodeSharpEditor::GetObjectFromBP(UBlueprint* OBJ) {
	return GetBlueprintCDO<UObject>(OBJ);
}

UBlueprint* FMagicNodeSharpEditor::GetBlueprintFromOBJ(UObject* OBJ) {
	return GetBlueprintFromCDO(OBJ);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<SDockTab>FMagicNodeSharpEditor::OnSpawnDiffToolTab(const FSpawnTabArgs &SpawnTabArgs) {
	TSharedRef<SDockTab>TAB = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	//
	TAB->SetTabIcon(FDiffToolEditorStyle::Get()->GetBrush(TEXT("DIFF.Window.Tab")));
	TAB->SetContent(SNew(SDIFFMainWidget));
	//
	return TAB;
}

void FMagicNodeSharpEditor::DIFF_InvokeTAB(UMagicNodeSharpSource* Left, UMagicNodeSharpSource* Right) {
	TSharedPtr<SDockTab>TAB = FGlobalTabmanager::Get()->TryInvokeTab(DIFFToolTAB);
	//
	TSharedRef<SDIFFMainWidget>Content = StaticCastSharedRef<SDIFFMainWidget>(TAB->GetContent());
	//
	Content->RebuildListViews(Left,Right);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<IDetailCustomization>FMagicNodeSharpCustomDetails::MakeInstance() {
	return MakeShareable(new FMagicNodeSharpCustomDetails);
}

void FMagicNodeSharpCustomDetails::CustomizeDetails(IDetailLayoutBuilder &DetailBuilder) {
	UMagicNodeSharpSource* CS = GetCDO<UMagicNodeSharpSource>(&DetailBuilder);
	if (CS==nullptr||!CS->IsValidLowLevelFast()) {return;}
	//
	//
	/*IDetailCategoryBuilder &Category = DetailBuilder.EditCategory("MagicNodeSharp");
	Category.AddCustomRow(FText::FromString(""))
	.WholeRowContent().MinDesiredWidth(200)
	[
		SAssignNew(SNodeScript,SNodeScriptPropertyEditor,CS)
	];*/
	//
	/*DetailBuilder.ForceRefreshDetails();*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_MODULE(FMagicNodeSharpEditor,MagicNodeSharpEditor);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////