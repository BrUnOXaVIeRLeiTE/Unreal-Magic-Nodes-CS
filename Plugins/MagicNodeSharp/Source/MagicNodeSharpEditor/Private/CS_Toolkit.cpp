//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CS_Toolkit.h"
#include "CS_EditorStyle.h"
#include "CS_SourceTreeView.h"

#include "KCS_MonoAnalyzer.h"

#include "IMagicNodeSharpKismet.h"
#include "IMagicNodeSharpEditor.h"

#include "MagicNodeSharpEditor_Shared.h"

#include "Runtime/Core/Public/Misc/DateTime.h"
#include "Runtime/SlateCore/Public/Widgets/SOverlay.h"
#include "Runtime/CoreUObject/Public/UObject/Package.h"
#include "Runtime/Slate/Public/Widgets/Notifications/SNotificationList.h"

#include "EditorReimportHandler.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Editor/UnrealEd/Public/SourceCodeNavigation.h"
#include "Editor/KismetWidgets/Public/SPinTypeSelector.h"
#include "Editor/KismetWidgets/Public/SSingleObjectDetailsPanel.h"

#include "Interfaces/IPluginManager.h"
#include "GenericPlatform/GenericPlatformFile.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CS Widget Extensions:

class SCS_DetailsTab : public SSingleObjectDetailsPanel {
private:
	TWeakPtr<FCS_Toolkit>CodeEditor;
public:
	SLATE_BEGIN_ARGS(SCS_DetailsTab)
	{}
	SLATE_END_ARGS()
public:
	virtual UObject* GetObjectToObserve() const override {
		return CodeEditor.Pin()->GET();
	}///
public:
	void Construct(const FArguments &InArgs,TSharedPtr<FCS_Toolkit>CS_Toolkit) {
		CodeEditor = CS_Toolkit;
		//
		SSingleObjectDetailsPanel::Construct(
			SSingleObjectDetailsPanel::FArguments()
			.HostCommandList(CS_Toolkit->GetToolkitCommands()).HostTabManager(CS_Toolkit->GetTabManager()), true, true
		);//
	}///
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CS Toolkit Constructors:

FCS_Toolkit::FCS_Toolkit() {
	static FDirectoryWatcherModule &DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	WatcherHandle = FDelegateHandle();
	//
	UMagicNodeSharpSource::OnScriptSourceExported.AddRaw(this,&FCS_Toolkit::OnScriptExported);
	UMagicNodeSharpSource::OnScriptSourceDeleted.AddRaw(this,&FCS_Toolkit::OnScriptDeleted);
	FEditorDelegates::OnAssetsDeleted.AddRaw(this,&FCS_Toolkit::OnAssetDeleted);
	//
	DirectoryWatcherModule.Get()->RegisterDirectoryChangedCallback_Handle(
		CS_SCRIPT_DIR,IDirectoryWatcher::FDirectoryChanged::CreateRaw(
			this, &FCS_Toolkit::OnProjectDirectoryChanged
		), WatcherHandle, IDirectoryWatcher::WatchOptions::IncludeDirectoryChanges
	);//
	//
	//
	if (FCS_Toolkit::SourceViewCount()==0) {
		RefreshScriptTreeView();
	}///
	//
	Search.Reset();
}

FCS_Toolkit::~FCS_Toolkit() {
	static FDirectoryWatcherModule &DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	DirectoryWatcherModule.Get()->UnregisterDirectoryChangedCallback_Handle(CS_SCRIPT_DIR,WatcherHandle);
	//
	UMagicNodeSharpSource::OnScriptSourceExported.RemoveAll(this);
	UMagicNodeSharpSource::OnScriptSourceDeleted.RemoveAll(this);
	//
	FEditorDelegates::OnAssetsDeleted.RemoveAll(this);
	//
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);
	FReimportManager::Instance()->OnPreReimport().RemoveAll(this);
	//
	GEditor->UnregisterForUndo(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CS Toolkit API:

void FCS_Toolkit::INIT(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>&InitToolkitHost, UMagicNodeSharpSource* WithScriptSource) {
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(WithScriptSource,this);
	GEditor->RegisterForUndo(this); check(WithScriptSource);
	//
	ScriptSource = WithScriptSource;
	ScriptSource->SetFlags(RF_Transactional);
	//
	//
	const TSharedRef<FTabManager::FLayout>FCodeEditorLayout = FTabManager::NewLayout("CS_CodeEditorLayout_V0001")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetHideTabWell(true)->SetSizeCoefficient(0.1f)
			->AddTab(GetToolbarTabId(),ETabState::OpenedTab)
		)
		->Split
		(
			FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)
			->SetSizeCoefficient(0.7f)
			->Split
			(
				FTabManager::NewStack()
				->SetHideTabWell(true)->SetSizeCoefficient(0.15f)
				->AddTab(FCodeEditorTAB::TAB_TreeView,ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.8f)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)->SetSizeCoefficient(0.75f)
					->AddTab(FCodeEditorTAB::TAB_Script,ETabState::OpenedTab)
				)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.2f)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)->SetSizeCoefficient(1.f)
					->AddTab(FCodeEditorTAB::TAB_Details,ETabState::OpenedTab)
				)
			)
		)
		->Split
		(
			FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)
			->SetSizeCoefficient(0.1f)
			->Split
			(
				FTabManager::NewStack()
				->SetHideTabWell(false)->SetSizeCoefficient(1.f)
				->AddTab(FCodeEditorTAB::TAB_Logs,ETabState::OpenedTab)
			)
		)
	);//
	//
	//
	InitAssetEditor(Mode,InitToolkitHost,CS_APP,FCodeEditorLayout,true,true,ScriptSource);
	//
	BindCommands();
	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();
	//
	//
	RefreshScriptTreeView();
	if (SourceTreeWidget.IsValid()) {
		SourceTreeWidget->RequestTreeRefresh();
		for (const TSharedPtr<FSourceTreeNode>&Node : ScriptSourcePaths) {
			SourceTreeWidget->SetItemExpansion(Node,true);
		}///
	}///
}

void FCS_Toolkit::SET(UMagicNodeSharpSource* NewScriptSource) {
	if ((NewScriptSource!=ScriptSource)&&(NewScriptSource!=nullptr)) {
		UMagicNodeSharpSource* OldScriptSource = ScriptSource;
		ScriptSource = NewScriptSource;
		//
		RemoveEditingObject(OldScriptSource);
		AddEditingObject(NewScriptSource);
	}///
}

UMagicNodeSharpSource* FCS_Toolkit::GET() const {
	return ScriptSource;
}

UMagicNodeSharp* FCS_Toolkit::GetScriptCDO() {
	if (ScriptSource==nullptr) {return nullptr;}
	//
	if (!Instance.IsValid()||Instance.IsStale()) {
		const FName CheckedID = MakeUniqueObjectName(GetTransientPackage(),UMagicNodeSharp::StaticClass(),*ScriptSource->GetScriptName());
		Instance = NewObject<UMagicNodeSharp>(GetTransientPackage(),UMagicNodeSharp::StaticClass(),CheckedID,RF_Transient);
	}///
	//
	return Instance.Get();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::RegisterTabSpawners(const TSharedRef<FTabManager>&TABManager) {
	WorkspaceMenuCategory = TABManager->AddLocalWorkspaceMenuCategory(LOCTEXT("CS_MagicNode_Workspace","Code Editor"));
	auto Workspace = WorkspaceMenuCategory.ToSharedRef();
	FAssetEditorToolkit::RegisterTabSpawners(TABManager);
	//
	TABManager->RegisterTabSpawner(FCodeEditorTAB::TAB_TreeView,FOnSpawnTab::CreateSP(this,&FCS_Toolkit::TABSpawn_TreeView))
	.SetDisplayName(LOCTEXT("CS_Types_TabName","Source")).SetGroup(Workspace);
	//
	TABManager->RegisterTabSpawner(FCodeEditorTAB::TAB_Script,FOnSpawnTab::CreateSP(this,&FCS_Toolkit::TABSpawn_Script))
	.SetDisplayName(LOCTEXT("CS_Script_TabName","Script")).SetGroup(Workspace);
	//
	TABManager->RegisterTabSpawner(FCodeEditorTAB::TAB_Details,FOnSpawnTab::CreateSP(this,&FCS_Toolkit::TABSpawn_Details))
	.SetDisplayName(LOCTEXT("CS_Details_TabName","Details")).SetGroup(Workspace);
	//
	TABManager->RegisterTabSpawner(FCodeEditorTAB::TAB_Logs,FOnSpawnTab::CreateSP(this,&FCS_Toolkit::TABSpawn_Logs))
	.SetDisplayName(LOCTEXT("CS_Logs_TabName","Results")).SetGroup(Workspace);
	//
	CS_TabManager = TABManager;
}

void FCS_Toolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>&TABManager) {
	FAssetEditorToolkit::UnregisterTabSpawners(TABManager);
	//
	TABManager->UnregisterTabSpawner(FCodeEditorTAB::TAB_TreeView);
	TABManager->UnregisterTabSpawner(FCodeEditorTAB::TAB_Details);
	TABManager->UnregisterTabSpawner(FCodeEditorTAB::TAB_Script);
	TABManager->UnregisterTabSpawner(FCodeEditorTAB::TAB_Logs);
}

TSharedRef<SDockTab>FCS_Toolkit::TABSpawn_Script(const FSpawnTabArgs &Args) {
	SAssignNew(CS_CodeEditor,SCS_CodeEditorCore,ScriptSource);
	//
	const auto Label = FText(LOCTEXT("CS_Script.Watermark","C#"));
	//
	return SNew(SDockTab)
	.Label(LOCTEXT("CS_ScriptTitle","Script"))
	.IsEnabled(this,&FCS_Toolkit::CanCompileScript)
	.Icon(FMagicNodeSharpEditorStyle::Get().Get()->GetBrush("SourceView.Script"))
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			CS_CodeEditor.ToSharedRef()
		]
		+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(4,4,4,30)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBox)
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					SAssignNew(ScriptNotify,SNotificationList)
					.Visibility(EVisibility::SelfHitTestInvisible)
				]
			]
		]
		+SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(4,4,10,20)
		[
			SNew(STextBlock).Text(Label)
			.Visibility(EVisibility::HitTestInvisible)
			.TextStyle(FEditorStyle::Get(),"Graph.CornerText")
		]
	];
}

TSharedRef<SDockTab>FCS_Toolkit::TABSpawn_Details(const FSpawnTabArgs &Args) {
	TSharedPtr<FCS_Toolkit>FCodeEditor = SharedThis(this);
	//
	return SNew(SDockTab)
	.Label(LOCTEXT("CS_DetailsTitle","Details"))
	.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
	[
		SNew(SCS_DetailsTab,FCodeEditor)
	];
}

TSharedRef<SDockTab>FCS_Toolkit::TABSpawn_Logs(const FSpawnTabArgs &Args) {
	return SNew(SDockTab).Label(LOCTEXT("CS_LogsTitle","Results"))
	.Icon(FEditorStyle::GetBrush(TEXT("LevelEditor.Tabs.StatsViewer")))
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
		[
			SAssignNew(CompilerResultsWidget,SListView<TSharedPtr<FCompilerResults>>)
			.OnMouseButtonDoubleClick(this,&FCS_Toolkit::OnClickedCompilerResultItem)
			.OnGenerateRow(this,&FCS_Toolkit::OnGenerateCompilerResultRow)
			.SelectionMode(ESelectionMode::Single)
			.ListItemsSource(&CompilerResults)
		]
	];
}

TSharedRef<SDockTab>FCS_Toolkit::TABSpawn_TreeView(const FSpawnTabArgs &Args) {
	TSharedPtr<FCS_Toolkit>FCodeEditor = SharedThis(this);
	Search = MakeShared<FString>(TEXT(""));
	//
	//
	SAssignNew(SourceTreeWidget,STreeView<TSharedPtr<FSourceTreeNode>>)
	.OnMouseButtonDoubleClick(this,&FCS_Toolkit::OnClickedSourceViewItem)
	///.OnContextMenuOpening(this,&FCS_Toolkit::OnGetSourceViewContextMenu)
	.OnSelectionChanged(this,&FCS_Toolkit::OnSelectedSourceViewItem)
	.OnExpansionChanged(this,&FCS_Toolkit::OnExpansionChanged)
	.OnGenerateRow(this,&FCS_Toolkit::OnGenerateSourceViewRow)
	.OnGetChildren(this,&FCS_Toolkit::OnGetSourceViewChildren)
	.SelectionMode(ESelectionMode::Single)
	.TreeItemsSource(&ScriptSourcePaths);
	//
	RefreshScriptTreeView();
	SourceTreeWidget->RequestTreeRefresh();
	for (const TSharedPtr<FSourceTreeNode>&Node : ScriptSourcePaths) {
		SourceTreeWidget->SetItemExpansion(Node,true);
	}///
	//
	//
	return SNew(SDockTab)
	.Label(LOCTEXT("CS_TreeViewTitle","Source"))
	.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight().Padding(0,2,0,2)
		.VAlign(VAlign_Top).HAlign(HAlign_Fill)
		[
			SAssignNew(SourceViewSearchBox,SSearchBox)
			.OnTextCommitted(this,&FCS_Toolkit::OnSearchCommitted)
			.OnTextChanged(this,&FCS_Toolkit::OnSearchChanged)
			.SelectAllTextWhenFocused(true)
		]
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
				.Visibility(this,&FCS_Toolkit::GetSourceTreeViewVisibility)
				[
					SourceTreeWidget.ToSharedRef()
				]
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
				.Visibility(this,&FCS_Toolkit::GetSourceTreeSearchVisibility)
				[
					SAssignNew(SourceSearchWidget,STreeView<TSharedPtr<FSourceTreeNode>>)
					.OnMouseButtonDoubleClick(this,&FCS_Toolkit::OnClickedSourceViewItem)
					.OnGenerateRow(this,&FCS_Toolkit::OnGenerateSourceViewRow)
					.OnGetChildren(this,&FCS_Toolkit::OnGetSourceViewChildren)
					.SelectionMode(ESelectionMode::Single)
					.TreeItemsSource(&SourceViewSearch)
				]
			]
		]
	];//
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<ITableRow>FCS_Toolkit::OnGenerateSourceViewRow(TSharedPtr<FSourceTreeNode>InItem, const TSharedRef<STableViewBase>&OwnerTable) {
	FText Tooltip;
	//
	TSharedPtr<SImage>Icon = SNew(SImage)
	.Image(FMagicNodeSharpEditorStyle::Get()->GetBrush("SourceView.FolderClosed"));
	//
	if (InItem.IsValid()) {
		Tooltip = FText::FromString(InItem->FullPath);
		//
		if (InItem->Path.Len()>InItem->FullPath.Len()){
			Tooltip=FText::FromString(InItem->Path);
		}///
		//
		if (InItem->Path==TEXT("SCRIPTS")) {
			Icon = SNew(SImage).Image(FMagicNodeSharpEditorStyle::Get()->GetBrush("SourceView.CsApp"));
		}///
		//
		if (InItem->Path.EndsWith(".cs")) {
			Icon = SNew(SImage).Image(FMagicNodeSharpEditorStyle::Get()->GetBrush("SourceView.Script"));
		}///
		//
		if (InItem->Path.EndsWith(".txt")) {
			Icon = SNew(SImage).Image(FMagicNodeSharpEditorStyle::Get()->GetBrush("SourceView.Text"));
		}///
		//
		if (InItem->Path.EndsWith(".ini")||InItem->Path.EndsWith(".uproject")||InItem->Path.EndsWith(".uplugin")) {
			Icon = SNew(SImage).Image(FMagicNodeSharpEditorStyle::Get()->GetBrush("SourceView.ConfigFile"));
		}///
	}///
	//
	//
	return SNew(STableRow<TSharedPtr<FSourceTreeNode>>,OwnerTable)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.Padding(1.f,0,1.f,0.f)
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			SNew(SBorder)
			.Padding(FMargin(0.f))
			.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			[
				Icon.ToSharedRef()
			]
		]
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
			[
				SNew(STextBlock)
				.Text(FText::FromString(InItem->Path))
				.ToolTip(FSlateApplication::Get().MakeToolTip(Tooltip))
			]
		]
	];//
}

TSharedRef<ITableRow>FCS_Toolkit::OnGenerateCompilerResultRow(TSharedPtr<FCompilerResults>InItem, const TSharedRef<STableViewBase>&OwnerTable) {
	const FString Date = FDateTime::Now().GetTimeOfDay().ToString().RightChop(1).LeftChop(4);
	//
	const FText Label = FText::FromString(FString::Printf(TEXT("%s     %s"),*Date,*InItem->ErrorMessage));
	//
	TSharedPtr<SImage>Icon = SNew(SImage).Image(FMagicNodeSharpEditorStyle::Get()->GetBrush("CS.Error"));
	if (InItem->Result==EMonoCompilerResult::Warning) {Icon=SNew(SImage).Image(FMagicNodeSharpEditorStyle::Get()->GetBrush("CS.Warning"));}
	if (InItem->Result==EMonoCompilerResult::Success) {Icon=SNew(SImage).Image(FMagicNodeSharpEditorStyle::Get()->GetBrush("CS.Success"));}
	//
	return SNew(STableRow<TSharedPtr<FCompilerResults>>,OwnerTable)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.Padding(1.f,0,1.f,0.f)
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			SNew(SBorder)
			.Padding(FMargin(0.f))
			.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			[
				Icon.ToSharedRef()
			]
		]
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
			[
				SNew(STextBlock).Text(Label)
				.ToolTip(FSlateApplication::Get().MakeToolTip(FText::FromString(InItem->ErrorMessage)))
			]
		]
	];//
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::BindCommands() {
	const FMagicNodeSharpEditorCommands &Commands = FMagicNodeSharpEditorCommands::Get();
	const TSharedRef<FUICommandList>&UICommandList = GetToolkitCommands();
	{
		UICommandList->MapAction(Commands.Compile,
			FExecuteAction::CreateSP(this,&FCS_Toolkit::CompileScript),
			FCanExecuteAction::CreateSP(this,&FCS_Toolkit::CanCompileScript)
		);//
		//
		UICommandList->MapAction(Commands.VSCode,
			FExecuteAction::CreateSP(this,&FCS_Toolkit::LaunchVSCode),
			FCanExecuteAction::CreateSP(this,&FCS_Toolkit::CanCompileScript)
		);//
		//
		UICommandList->MapAction(Commands.VSGen,
			FExecuteAction::CreateSP(this,&FCS_Toolkit::GenerateVSCode),
			FCanExecuteAction::CreateSP(this,&FCS_Toolkit::CanCompileScript)
		);//
		//
		UICommandList->MapAction(Commands.VSLaunch,
			FExecuteAction::CreateSP(this,&FCS_Toolkit::LaunchVStudio),
			FCanExecuteAction::CreateSP(this,&FCS_Toolkit::CanCompileScript)
		);//
		//
		UICommandList->MapAction(Commands.DiffTool,
			FExecuteAction::CreateSP(this,&FCS_Toolkit::LaunchSourceCodeDIFF),
			FCanExecuteAction::CreateSP(this,&FCS_Toolkit::CanCompileScript)
		);//
		//
		UICommandList->MapAction(Commands.Help,
			FExecuteAction::CreateSP(this,&FCS_Toolkit::LaunchHelpWiki),
			FCanExecuteAction::CreateSP(this,&FCS_Toolkit::CanCompileScript)
		);//
	}///
}

void FCS_Toolkit::ExtendMenu() {
	////... @ToDo
}

void FCS_Toolkit::ExtendToolbar() {
	struct Local {
		static void FillToolbar(FToolBarBuilder &ToolbarBuilder) {
			//if (FSourceCodeNavigation::IsCompilerAvailable()) {
				ToolbarBuilder.BeginSection("Compile");
				{
					ToolbarBuilder.AddToolBarButton(FMagicNodeSharpEditorCommands::Get().Compile);
				}
				ToolbarBuilder.EndSection();
				//
				ToolbarBuilder.BeginSection("Tools");
				{
					ToolbarBuilder.AddToolBarButton(FMagicNodeSharpEditorCommands::Get().VSCode);
					ToolbarBuilder.AddToolBarButton(FMagicNodeSharpEditorCommands::Get().VSGen);
					ToolbarBuilder.AddToolBarButton(FMagicNodeSharpEditorCommands::Get().VSLaunch);
				}
				ToolbarBuilder.EndSection();
				//
				ToolbarBuilder.BeginSection("Utility");
				{
					ToolbarBuilder.AddToolBarButton(FMagicNodeSharpEditorCommands::Get().DiffTool);
				}
				ToolbarBuilder.EndSection();
				//
				ToolbarBuilder.BeginSection("Documentation");
				{
					ToolbarBuilder.AddToolBarButton(FMagicNodeSharpEditorCommands::Get().Help);
				}
				ToolbarBuilder.EndSection();
			//}///
		}///
	};//
	//
	TSharedPtr<FExtender>ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension(
		"Asset", EExtensionHook::After, GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
	);//
	//
	AddToolbarExtender(ToolbarExtender);
	IMagicNodeSharpEditor* EditorModule = &FModuleManager::LoadModuleChecked<IMagicNodeSharpEditor>("MagicNodeSharpEditor");
	AddToolbarExtender(EditorModule->GetMagicNodeSharpEditorToolBarExtensibilityManager()->GetAllExtenders());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::FocusToolkitTab(const FName &Tab) {
	FGlobalTabmanager::Get()->DrawAttentionToTabManager(CS_TabManager.ToSharedRef());
	TSharedPtr<SDockTab>TAB = CS_TabManager->FindExistingLiveTab(Tab);
	//
	if (TAB.IsValid()) {CS_TabManager->DrawAttention(TAB.ToSharedRef());} else {
		TAB = CS_TabManager->TryInvokeTab(Tab);
		if (TAB.IsValid()) {CS_TabManager->DrawAttention(TAB.ToSharedRef());}
	}///
}

void FCS_Toolkit::CloseToolkitTab(const FName &Tab) {
	FGlobalTabmanager::Get()->DrawAttentionToTabManager(CS_TabManager.ToSharedRef());
	TSharedPtr<SDockTab>TAB = CS_TabManager->FindExistingLiveTab(Tab);
	//
	if (TAB.IsValid()) {TAB->RequestCloseTab();}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::CompileScript() {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	if (!MonoKismet.CanCompile()) {return;}
	if (!CS_CodeEditor.IsValid()) {return;}
	//
	if (GET()==nullptr) {
		LOG::CS_CHAR(ESeverity::Error,TEXT("{C#}:: Compiler unreachable, Script is invalid.")); return;
	}///
	//
	if (CS_TabManager.IsValid()) {
		FGlobalTabmanager::Get()->DrawAttentionToTabManager(CS_TabManager.ToSharedRef());
		TSharedPtr<SDockTab>TAB = CS_TabManager->FindExistingLiveTab(FCodeEditorTAB::TAB_Script);
		//
		if (TAB.IsValid()) {CS_TabManager->DrawAttention(TAB.ToSharedRef());} else {
			TSharedPtr<SDockTab>SCRIPT = CS_TabManager->TryInvokeTab(FCodeEditorTAB::TAB_Script);
			if (SCRIPT.IsValid()) {CS_TabManager->DrawAttention(SCRIPT.ToSharedRef());} else {
				LOG::CS_CHAR(ESeverity::Error,TEXT("Core Script Tab is unreachable or invalid. Compilation aborted."));
				if (GEditor){GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));}
			return;}
		}///
	} else {
		LOG::CS_CHAR(ESeverity::Error,TEXT("Core Script Tab Manager is unreachable or invalid. Compilation aborted."));
		if (GEditor) {GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));}
	return;}
	//
	FCompilerResults Fail;
	Fail.Result = EMonoCompilerResult::Error;
	Fail.ErrorMessage = TEXT("{C#}:: Compiler unreachable, Script is invalid.");
	//
	if (!GET()->IsValidLowLevel()) {
		AppendCompilerResults(Fail); return;
	} else if (GET()==UMagicNodeSharpSource::StaticClass()->ClassDefaultObject) {
		Fail.ErrorMessage = TEXT("{C#}:: Script is invalid (referencing Base Script Class is not allowed).");
		AppendCompilerResults(Fail); return;
	}///
	//
	if (!MonoKismet.MonoKismet_INIT()) {
		Fail.ErrorMessage = TEXT("{C#}:: Compiler unreachable, Mono is busy or not properly initialized.");
		AppendCompilerResults(Fail); LOG::CS_STR(ESeverity::Error,Fail.ErrorMessage); return;
	}///
	//
	MonoKismet.CompilerResult.BindRaw(this,&FCS_Toolkit::OnCompilationFinished);
	MonoKismet.CompileNode(GET(),GetScriptCDO());
}

void FCS_Toolkit::OnCompilationFinished(const UMagicNodeSharpSource* Script, const FCompilerResults Results) {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	if (Script==nullptr) {return;}
	if (Script->HasAnyFlags(RF_ClassDefaultObject|RF_ArchetypeObject|RF_BeginDestroyed)) {return;}
	{
		if (!Results.ErrorMessage.IsEmpty()) {
			FNotificationInfo NInfo = FNotificationInfo(FText::FromString(Results.ErrorMessage));
			NInfo.Image = FEditorStyle::GetBrush(TEXT("NotificationList.DefaultMessage"));
			NInfo.bUseSuccessFailIcons = false;
			NInfo.bFireAndForget = true;
			NInfo.bUseThrobber = false;
			//
			NInfo.ExpireDuration = (Results.ErrorMessage.Len()<=11) ? Results.ErrorMessage.Len() : 10.f;
			//
			if (Results.Result==EMonoCompilerResult::Error) {NInfo.Image=FEditorStyle::GetBrush(TEXT("Kismet.Status.Error"));}
			if (Results.Result==EMonoCompilerResult::Warning) {NInfo.Image=FEditorStyle::GetBrush(TEXT("Kismet.Status.Warning"));}
			//
			if (CS_CodeEditor.IsValid()) {CS_CodeEditor->SetScriptError(Results.Result,Results.ErrorInfo);}
			if ((GEditor)&&(Results.Result==EMonoCompilerResult::Error)) {GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));}
			if ((GEditor)&&(Results.Result==EMonoCompilerResult::Warning)) {GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));}
			//
			AddNotification(NInfo,false);
			AppendCompilerResults(Results);
		} else {
			FNotificationInfo NInfo = FNotificationInfo(FText::FromString(TEXT("Success")));
			NInfo.Image = FEditorStyle::GetBrush(TEXT("NotificationList.SuccessImage"));
			NInfo.bUseSuccessFailIcons = false;
			NInfo.bFireAndForget = true;
			NInfo.bUseThrobber = false;
			NInfo.ExpireDuration = 2.f;
			//
			if (Results.Result==EMonoCompilerResult::Success) {NInfo.Image=FEditorStyle::GetBrush(TEXT("Kismet.Status.Good"));}
			if (Results.Result==EMonoCompilerResult::Warning) {NInfo.Image=FEditorStyle::GetBrush(TEXT("Kismet.Status.Warning"));}
			//
			if (CS_CodeEditor.IsValid()) {CS_CodeEditor->SetScriptError(Results.Result,Results.ErrorInfo);}
			if (GEditor) {GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));}
			//
			AddNotification(NInfo,true);
			AppendCompilerResults(Results);
		}///
	}
	//
	if (Results.Result != EMonoCompilerResult::Error) {
		FocusToolkitTab(FCodeEditorTAB::TAB_Logs);
		//
		UPackage* Package = GET()->GetOutermost();
		//
		if (Package->IsDirty()) {
			TArray<UPackage*>PackagesToSave; PackagesToSave.Add(Package);
			FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave,true,false);
		}///
	}///
	//
	//
	RefreshScriptTreeView();
	//
	if (SourceTreeWidget.IsValid()) {
		SourceTreeWidget->RequestTreeRefresh();
		for (const TSharedPtr<FSourceTreeNode>&Node : ScriptSourcePaths) {
			SourceTreeWidget->SetItemExpansion(Node,true);
		}///
	}///
	//
	//
	MonoKismet.CompilerResult.Unbind();
	MonoKismet.MonoKismetDomain_STOP();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::LaunchSourceCodeEditor() {
	IPlatformFile &PlatformFM = FPlatformFileManager::Get().GetPlatformFile();
	FAssetRegistryModule &AssetRegistry=FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	//
	TArray<FAssetData>AssetData;
	FARFilter AssetFilter;
	//
	AssetFilter.bRecursiveClasses = true;
	AssetFilter.ClassNames.Add(UMagicNodeSharpSource::StaticClass()->GetFName());
	//
	if (AssetRegistry.Get().GetAssets(AssetFilter,AssetData)&&(AssetData.Num()>0)) {
		for (auto &Data : AssetData) {
			if (UObject*CDO=Data.GetAsset()) {
				LaunchTargetSource.RemoveFromEnd(CS_SCRIPT_EXT);
				if (LaunchTargetSource.Equals(CDO->GetName(),ESearchCase::CaseSensitive)) {
					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(CDO);
				}///
			}///
		}///
	}///
}

void FCS_Toolkit::LaunchVSCode() {
	if (!ScriptSource->IsValidLowLevelFast()) {return;}
	//
	const FString SCN = ScriptSource->GetScriptName();
	//
	const FString LC = FString::Printf(TEXT(":%i:%i"),CS_CodeEditor->GetCursorOffset().GetLineIndex()+1,CS_CodeEditor->GetCursorOffset().GetOffset()+1);
	const FString IURL = FPaths::Combine(TEXT("vscode://file"),CS_SCRIPT_DIR,SCN+CS_SCRIPT_EXT+LC);
	//
	LOG::CS_STR(ESeverity::Info,IURL);
	UKismetSystemLibrary::LaunchURL(IURL);
}

void FCS_Toolkit::GenerateVSCode() {
	IPlatformFile &PlatformManager = FPlatformFileManager::Get().GetPlatformFile();
	if (!ScriptSource->IsValidLowLevelFast()) {return;}
	//
	static const FString Content = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(CS_PLUGIN_NAME)->GetContentDir());
	static const FString SGuid = TEXT("F0000000-0000-0000-0000-000000000000");
	static const FString PGuid = TEXT("E0000000-0000-0000-0000-000000000000");
	//
	static const FString CINC = TEXT("<Compile Include=\"..\\{PROJECT}.cs\" />");
	static const FString RINC = TEXT("<Reference Include=\"Microsoft.CSharp\" />");
	//
	const FString SCN = ScriptSource->GetScriptName();
	const FString DIR = FPaths::Combine(CS_SCRIPT_DIR,TEXT("SLN_")+SCN);
	//
	if (!PlatformManager.DirectoryExists(*DIR)) {
		PlatformManager.CreateDirectory(*DIR);
		PlatformManager.CopyFile(
			*FPaths::Combine(CS_SCRIPT_DIR,TEXT(".vscode"),TEXT("settings.json")),
			*FPaths::Combine(Content,TEXT("Gen"),TEXT("settings.json"))
		);///
	}///
	//
	FString SLN = TEXT("\0");
	FString PRJ = TEXT("\0");
	FString INF = TEXT("\0");
	FString SET = TEXT("\0");
	//
	bool Generated = true;
	const bool INF_Loaded = FFileHelper::LoadFileToString(INF,*FPaths::Combine(Content,TEXT("Gen"),TEXT("INFO.gen")));
	const bool PRJ_Loaded = FFileHelper::LoadFileToString(PRJ,*FPaths::Combine(Content,TEXT("Gen"),TEXT("PROJECT.gen")));
	const bool SLN_Loaded = FFileHelper::LoadFileToString(SLN,*FPaths::Combine(Content,TEXT("Gen"),TEXT("SOLUTION.gen")));
	const bool SET_Loaded = FFileHelper::LoadFileToString(SET,*FPaths::Combine(Content,TEXT("Gen"),TEXT("SETTINGS.gen")));
	//
	if (SLN_Loaded && PRJ_Loaded && INF_Loaded) {
		const FString SLNG = IKCS_MonoAnalyzer::GenerateGUID(SGuid);
		const FString PRJG = IKCS_MonoAnalyzer::GenerateGUID(PGuid);
		const FString INFG = IKCS_MonoAnalyzer::GenerateGUID(PGuid);
		//
		for (const auto &INC : ScriptSource->Include) {
			if (INC==nullptr||INC->GetScriptName().Equals(SCN)) {continue;}
			//
			const FString Include = FString::Printf(TEXT("    <Compile Include=\"..\\%s.cs\" />"),*INC->GetScriptName());
			//
			PRJ.ReplaceInline(*CINC,*(CINC+TEXT("\n")+Include));
		}///
		//
		for (const auto &REF : ScriptSource->References) {
			if (REF.FilePath.EndsWith(TEXT("UnrealEngine.dll"))) {continue;}
			if (REF.FilePath.EndsWith(TEXT("MagicNodes.dll"))) {continue;}
			//
			const FString PATH = FPaths::ConvertRelativePathToFull(REF.FilePath);
			const FString LIB = FPaths::GetBaseFilename(PATH);
			//
			const FString Reference = FString::Printf(
				TEXT("    <Reference Include=\"%s, Version=1.0.0.0, Culture=neutral, processorArchitecture=AMD64\">\n      <SpecificVersion>False</SpecificVersion>\n      <HintPath>%s</HintPath>\n    </Reference>"),
				*LIB, *PATH
			);/// 
			//
			PRJ.ReplaceInline(*RINC,*(RINC+TEXT("\n")+Reference));
		}///
		//
		INF.ReplaceInline(*PGuid,*PRJG,ESearchCase::CaseSensitive);
		PRJ.ReplaceInline(*PGuid,*PRJG,ESearchCase::CaseSensitive);
		SLN.ReplaceInline(*PGuid,*PRJG,ESearchCase::CaseSensitive);
		SLN.ReplaceInline(*SGuid,*SLNG,ESearchCase::CaseSensitive);
		//
		INF.ReplaceInline(TEXT("{PROJECT}"),*SCN,ESearchCase::CaseSensitive);
		PRJ.ReplaceInline(TEXT("{PROJECT}"),*SCN,ESearchCase::CaseSensitive);
		SLN.ReplaceInline(TEXT("{PROJECT}"),*SCN,ESearchCase::CaseSensitive);
		SLN.ReplaceInline(TEXT("{SOLUTION}"),*(TEXT("SLN_")+SCN),ESearchCase::CaseSensitive);
		//
		Generated = (Generated && FFileHelper::SaveStringToFile(PRJ,*FPaths::Combine(DIR,SCN+TEXT(".csproj"))));
		Generated = (Generated && FFileHelper::SaveStringToFile(SLN,*FPaths::Combine(DIR,TEXT("SLN_")+SCN+TEXT(".sln"))));
		Generated = (Generated && FFileHelper::SaveStringToFile(INF,*FPaths::Combine(DIR,TEXT("Properties"),TEXT("AssemblyInfo.cs"))));
		//
		FFileHelper::SaveStringToFile(SET,*FPaths::Combine(CS_SCRIPT_DIR,TEXT(".vscode"),TEXT("settings.json")));
	}///
	//
	if (Generated) {
		LOG::CS_STR(ESeverity::Info,DIR);
		//
		LOG::CS_CHAR(ESeverity::Info,TEXT("--------------------------------------------------"));
		LOG::CS_STR(ESeverity::Info,SLN);
		LOG::CS_CHAR(ESeverity::Info,TEXT("--------------------------------------------------"));
		LOG::CS_STR(ESeverity::Info,PRJ);
		LOG::CS_CHAR(ESeverity::Info,TEXT("--------------------------------------------------"));
		LOG::CS_STR(ESeverity::Info,INF);
		LOG::CS_CHAR(ESeverity::Info,TEXT("--------------------------------------------------"));
		//
		static FString IURL = FPaths::Combine(TEXT("vscode://file"),CS_SCRIPT_DIR);
		UKismetSystemLibrary::LaunchURL(IURL);
	}///
}

void FCS_Toolkit::LaunchVStudio() {
	IPlatformFile &PlatformManager = FPlatformFileManager::Get().GetPlatformFile();
	if (!ScriptSource->IsValidLowLevelFast()) {return;}
	//
	const FString SCN = ScriptSource->GetScriptName();
	const FString DIR = FPaths::Combine(CS_SCRIPT_DIR,TEXT("SLN_")+SCN);
	const FString TARGET = FPaths::Combine(DIR,TEXT("SLN_")+SCN+TEXT(".sln"));
	//
	if (PlatformManager.FileExists(*TARGET)) {
		UKismetSystemLibrary::LaunchURL(FPaths::Combine(TEXT("file:///")+TARGET));
	}///
}

void FCS_Toolkit::LaunchSourceCodeDIFF() {
	if (UObject*CDO=FMagicNodeSharpEditor::GetSelectedAsset()) {
		if (UMagicNodeSharpSource*Source=Cast<UMagicNodeSharpSource>(CDO)) {
			if (Source!=GET()) {FMagicNodeSharpEditor::DIFF_InvokeTAB(Source,GET());}
			else {LOG::CS_CHAR(ESeverity::Info,TEXT("Script and selection are the same."));}
		} else {LOG::CS_CHAR(ESeverity::Info,TEXT("Selection is not a script to compare."));}
	} else {LOG::CS_CHAR(ESeverity::Info,TEXT("No script selected to compare sources."));}
}

void FCS_Toolkit::LaunchHelpWiki() {
	UKismetSystemLibrary::LaunchURL(TEXT("https://github.com/BrUnOXaVIeRLeiTE/Unreal-Magic-Nodes/wiki"));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::OnScriptExported(UMagicNodeSharpSource* Source) {
	RefreshScriptTreeView();
	//
	if (SourceTreeWidget.IsValid()) {
		SourceTreeWidget->RequestTreeRefresh();
		for (const TSharedPtr<FSourceTreeNode>&Node : ScriptSourcePaths) {
			SourceTreeWidget->SetItemExpansion(Node,true);
		}///
	}///
}

void FCS_Toolkit::OnScriptDeleted() {
	RefreshScriptTreeView();
	//
	if (SourceTreeWidget.IsValid()) {
		SourceTreeWidget->RequestTreeRefresh();
		for (const TSharedPtr<FSourceTreeNode>&Node : ScriptSourcePaths) {
			SourceTreeWidget->SetItemExpansion(Node,true);
		}///
	}///
}

void FCS_Toolkit::OnAssetDeleted(const TArray<UClass*>&DeletedAssetClasses) {
	if (DeletedAssetClasses.Num()>0) {
		RefreshScriptTreeView();
		//
		if (SourceTreeWidget.IsValid()) {
			SourceTreeWidget->RequestTreeRefresh();
			for (const TSharedPtr<FSourceTreeNode>&Node : ScriptSourcePaths) {
				SourceTreeWidget->SetItemExpansion(Node,true);
			}///
		}///
	}///
}

void FCS_Toolkit::OnProjectDirectoryChanged(const TArray<FFileChangeData>&Data) {
	RefreshScriptTreeView();
}

void FCS_Toolkit::OnToolkitHostingStarted(const TSharedRef<IToolkit>&Toolkit) {}
void FCS_Toolkit::OnToolkitHostingFinished(const TSharedRef<IToolkit>&Toolkit){}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::AddNotification(FNotificationInfo &Info, bool Success) {
	if (!ScriptNotify.IsValid()) {return;}
	//
	Info.bUseLargeFont = true;
	//
	TSharedPtr<SNotificationItem>SNotify=ScriptNotify->AddNotification(Info);
	if (SNotify.IsValid()) {SNotify->SetCompletionState((Success) ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);}
}

void FCS_Toolkit::AppendCompilerResults(const FCompilerResults &Result) {
	if (GET()==nullptr) {return;}
	//
	TSharedPtr<FCompilerResults>NewResult = MakeShareable(new FCompilerResults(Result));
	//
	if (NewResult->ErrorMessage.IsEmpty()) {
		NewResult->ErrorMessage = FString::Printf(TEXT("%s:  Success."),*(GET()->GetScriptName()));
	}///
	//
	CompilerResults.Add(NewResult);
	//
	if (CompilerResultsWidget.IsValid()) {CompilerResultsWidget->RequestListRefresh();}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EVisibility FCS_Toolkit::GetSourceTreeViewVisibility() const {
	if (Search.IsValid()&&(Search->Len()>=2)) {
		return EVisibility::Collapsed;
	}///
	//
	return EVisibility::Visible;
}

EVisibility FCS_Toolkit::GetSourceTreeSearchVisibility() const {
	if (Search.IsValid()&&(Search->Len()>=2)) {
		return EVisibility::Visible;
	}///
	//
	return EVisibility::Collapsed;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::OnClickedSourceViewItem(TSharedPtr<FSourceTreeNode>TreeItem) {
	if (!TreeItem.IsValid()) {return;}
	//
	if (IsSourceFile(TreeItem->FullPath)) {
		LaunchTargetSource = TreeItem->Path;
		LaunchSourceCodeEditor();
	} else {SourceTreeWidget->SetItemExpansion(TreeItem,true);}
}

void FCS_Toolkit::OnClickedCompilerResultItem(TSharedPtr<FCompilerResults>Item) {
	LOG::CS_CHAR(ESeverity::Warning,TEXT("OnClickedCompilerResultItem()!"));
	/// @ToDo...
}

void FCS_Toolkit::OnSelectedSourceViewItem(TSharedPtr<FSourceTreeNode>TreeItem, ESelectInfo::Type SelectInfo) {
	if (!TreeItem.IsValid()) {return;}
	//
	if (IsSourceFile(TreeItem->FullPath)) {
		LaunchTargetSource = TreeItem->Path;
	}///
}

void FCS_Toolkit::OnGetSourceViewChildren(TSharedPtr<FSourceTreeNode>InItem, TArray<TSharedPtr<FSourceTreeNode>>&OutChildren) {
	if (SourceViewSearch.Num()==0) {OutChildren=InItem->ChildNodes;}
}

void FCS_Toolkit::OnExpansionChanged(TSharedPtr<FSourceTreeNode>InItem, bool WasExpanded){}
void FCS_Toolkit::OnSourceViewCheckStatusChanged(ECheckBoxState NewCheckState, TSharedPtr<FSourceTreeNode>NodeChanged){}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::AddReferencedObjects(FReferenceCollector &Collector) {
	Collector.AddReferencedObject(ScriptSource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FName FCS_Toolkit::GetToolkitFName() const {
	return FName("CS_CodeEditor");
}

FText FCS_Toolkit::GetBaseToolkitName() const {
	return LOCTEXT("CS_CodeEditor.Label","Magic Node Editor");
}

FText FCS_Toolkit::GetToolkitName() const {
	const bool DirtyState = ScriptSource->GetOutermost()->IsDirty();
	//
	FFormatNamedArguments Args;
	Args.Add(TEXT("McsName"),FText::FromString(ScriptSource->GetName()));
	Args.Add(TEXT("DirtyState"),(DirtyState)?FText::FromString(TEXT("*")):FText::GetEmpty());
	//
	return FText::Format(LOCTEXT("CS_CodeEditor.Label","{McsName}{DirtyState}"),Args);
}

FText FCS_Toolkit::GetToolkitTitle() const {
	FText Title = LOCTEXT("CS_CodeEditor_BaseTitle","Magic Node (C#)");
	//
	if (GET()==nullptr) {return Title;}
	//		
	FString SName = GET()->GetScriptName();
	FString SCaps; SCaps.AppendChar(SName[0]);
	SName.ReplaceInline(TEXT("_"),TEXT(" "));
	//
	for (int32 I=1; I < SName.Len(); I++) {
		if (FChar::IsUpper(SName[I])) {
			   if (SName[I-1] != TEXT(' ') && !FChar::IsUpper(SName[I-1])) {SCaps.AppendChar(TEXT(' '));}
		} SCaps.AppendChar(SName[I]);
	   }///
	//
	FText Name = FText::FromString(SCaps);
	FFormatNamedArguments Args; Args.Add(TEXT("Name"),Name);
	//
	Title = FText::Format(LOCTEXT("CS_CodeEditor_Title","{Name}"),Args);
	//
	return Title;
}

FText FCS_Toolkit::GetToolkitToolTipText() const {
	return GetToolTipTextForObject(ScriptSource);
}

FString FCS_Toolkit::GetWorldCentricTabPrefix() const {
	return TEXT("CS_CodeEditor");
}

FString FCS_Toolkit::GetDocumentationLink() const {
	return TEXT("https://brunoxavierleite.wordpress.com/2019/01/16/unreal-magic-nodes-programming/");
}

FLinearColor FCS_Toolkit::GetWorldCentricTabColorScale() const {
	return FLinearColor::White;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FCS_Toolkit::IsCompiling() const {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	return MonoKismet.IsCompiling();
}

bool FCS_Toolkit::CanCompileScript() const {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	//
	return (
		MonoKismet.CanCompile() && ScriptSource &&
		(GEditor==nullptr||(!GEditor->IsPlaySessionInProgress()))
	);//
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::RefreshScriptTreeView() {
	static FString ProjectRoot = FPaths::GameSourceDir();
	static TArray<FString>ProjectSource;
	//
	IFileManager &FileManager = IFileManager::Get();
	FileManager.FindFilesRecursive(ProjectSource,*(ProjectRoot+TEXT("Scripts")),*FString::Printf(TEXT("/*%s"),*CS_SCRIPT_EXT),true,false);
	if (ProjectSource.Num()==0) {return;}
	//
	FString RootPath; 
	FString FullPath = FPaths::ConvertRelativePathToFull(ProjectSource[0]);
	//
	FullPath.Split(TEXT("/Scripts/"),&RootPath,nullptr);
	RootPath += TEXT("/Scripts/");
	//
	TSharedPtr<FSourceTreeNode>RootNode = MakeShared<FSourceTreeNode>();
	RootNode->Path = TEXT("SCRIPTS"); RootNode->FullPath = RootPath;
	//
	TSharedRef<FSourceTreeNode>OldRoot = RootNode.ToSharedRef();
	for (const TSharedPtr<FSourceTreeNode>&Old : ScriptSourcePaths) {
		if (Old->Path==RootNode->Path) {OldRoot=Old.ToSharedRef(); break;}
	} ScriptSourcePaths.Remove(OldRoot);
	//
	ScriptSourcePaths.Add(RootNode);
	TSharedRef<FSourceTreeNode>ParentNode = RootNode.ToSharedRef();
	//
	for (FString &Path : ProjectSource) {
		Path = FPaths::ConvertRelativePathToFull(Path);
		Path.ReplaceInline(*RootPath,TEXT(""));
	}///
	//
	for (const FString &Path : ProjectSource) {
		if (!IsSourceFile(Path)) {continue;}
		if (Path.IsEmpty()) {continue;}
		//
		TArray<FString>Nodes;
		Path.ParseIntoArray(Nodes,TEXT("/"));
		FString Source = ParentNode->FullPath;
		//
		for (int32 I=0; I<Nodes.Num(); I++) {
			const FString &Node = Nodes[I];
			Source = FPaths::Combine(Source,Node);
			TSharedPtr<FSourceTreeNode>TreeNode = ParentNode->FindNode(Node);
			//
			if (!TreeNode.IsValid()) {
				TreeNode = MakeShared<FSourceTreeNode>();
				TreeNode->ParentNode = ParentNode;
				TreeNode->FullPath = Source;
				TreeNode->Path = Node;
				//
				ParentNode->ChildNodes.Add(TreeNode);
			} ParentNode = TreeNode.ToSharedRef();
		}///
		//
		ParentNode = RootNode.ToSharedRef();
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FCS_Toolkit::OnSearchChanged(const FText &Filter) {
	Search = MakeShared<FString>(Filter.ToString());
	SourceViewSearch.Empty();
	//
	for (const TSharedPtr<FSourceTreeNode>&Node : ScriptSourcePaths) {
		if (Node->Path.Contains(**Search.Get())) {
			SourceViewSearch.Add(Node);
		}///
		//
		for (const TSharedPtr<FSourceTreeNode>&N1 : Node->ChildNodes) {
			if (N1->Path.Contains(**Search.Get())) {
				SourceViewSearch.Add(N1);
			}///
			//
			for (const TSharedPtr<FSourceTreeNode>&N2 : N1->ChildNodes) {
				if (N2->Path.Contains(**Search.Get())) {
					SourceViewSearch.Add(N2);
				}///
				//
				for (const TSharedPtr<FSourceTreeNode>&N3 : N2->ChildNodes) {
					if (N3->Path.Contains(**Search.Get())) {
						SourceViewSearch.Add(N3);
					}///
					//
					for (const TSharedPtr<FSourceTreeNode>&N4 : N3->ChildNodes) {
						if (N4->Path.Contains(**Search.Get())) {
							SourceViewSearch.Add(N4);
						}///
						//
						for (const TSharedPtr<FSourceTreeNode>&N5 : N4->ChildNodes) {
							if (N5->Path.Contains(**Search.Get())) {
								SourceViewSearch.Add(N5);
							}///
							//
							for (const TSharedPtr<FSourceTreeNode>&N6 : N5->ChildNodes) {
								if (N6->Path.Contains(**Search.Get())) {
									SourceViewSearch.Add(N6);
								}///
							}///
						}///
					}///
				}///
			}///
		}///
	}///
	//
	//
	SourceSearchWidget->RequestListRefresh();
}

void FCS_Toolkit::OnSearchCommitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	if (NewText.ToString().Len()<2) {
		SourceViewSearch.Empty();
	}///
	//
	SourceSearchWidget->RequestListRefresh();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FCS_Toolkit::IsSourceFile(const FString &Path) {
	return (Path.EndsWith(TEXT(".cs")));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////