/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CS_SourceTreeView.h"
#include "CS_CodeEditorCore.h"
#include "CS_EditorCommands.h"

#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"

#include "Runtime/CoreUObject/Public/UObject/GCObject.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

#include "Runtime/SlateCore/Public/Widgets/SBoxPanel.h"
#include "Runtime/SlateCore/Public/Widgets/Images/SImage.h"

#include "Runtime/Core/Public/Logging/TokenizedMessage.h"
#include "Runtime/Slate/Public/Widgets/Text/STextBlock.h"
#include "Runtime/Slate/Public/Widgets/Views/STreeView.h"
#include "Runtime/Slate/Public/Widgets/Input/SSearchBox.h"
#include "Runtime/Slate/Public/Widgets/Docking/SDockTab.h"

#include "Editor/UnrealEd/Public/Editor.h"
#include "Editor/UnrealEd/Public/GraphEditor.h"
#include "Editor/UnrealEd/Public/EditorUndoClient.h"
#include "Editor/UnrealEd/Public/Toolkits/AssetEditorToolkit.h"
#include "Editor/EditorFramework/Public/Toolkits/IToolkitHost.h"

#include "Developer/DirectoryWatcher/Public/IDirectoryWatcher.h"
#include "Developer/DirectoryWatcher/Public/DirectoryWatcherModule.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UMagicNodeSharpSource;
class FMagicNodeSharpEditor;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FCodeEditorTAB {
	static const FName TAB_TreeView;
	static const FName TAB_Details;
	static const FName TAB_Script;
	static const FName TAB_Logs;
};

const FName FCodeEditorTAB::TAB_TreeView(TEXT("CS_TreeView"));
const FName FCodeEditorTAB::TAB_Details(TEXT("CS_Details"));
const FName FCodeEditorTAB::TAB_Script(TEXT("CS_Script"));
const FName FCodeEditorTAB::TAB_Logs(TEXT("CS_Logs"));

const FName CS_APP = FName(TEXT("CS_CodeEditor"));

static FString LaunchTargetSource("");

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CS Asset Editor Toolkit:

class FCS_Toolkit : public FAssetEditorToolkit, public FEditorUndoClient, public FGCObject {
private:
	FDelegateHandle WatcherHandle;
private:
	UMagicNodeSharpSource* ScriptSource;
	TWeakObjectPtr<UMagicNodeSharp>Instance = nullptr;
private:
	TSharedPtr<FTabManager>CS_TabManager;
	TSharedPtr<SCS_CodeEditorCore>CS_CodeEditor;
private:
	TSharedPtr<FString>Search;
	TArray<TSharedPtr<FSourceTreeNode>>SourceViewSearch;
	TArray<TSharedPtr<FCompilerResults>>CompilerResults;
protected:
	TSharedPtr<SNotificationList>ScriptNotify;
	TSharedPtr<SSearchBox>SourceViewSearchBox;
	TSharedPtr<STreeView<TSharedPtr<FSourceTreeNode>>>SourceTreeWidget;
	TSharedPtr<STreeView<TSharedPtr<FSourceTreeNode>>>SourceSearchWidget;
	TSharedPtr<SListView<TSharedPtr<FCompilerResults>>>CompilerResultsWidget;
protected:
	TSharedRef<SDockTab>TABSpawn_Logs(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab>TABSpawn_Script(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab>TABSpawn_Details(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab>TABSpawn_TreeView(const FSpawnTabArgs &Args);
protected:
	TSharedRef<ITableRow>OnGenerateSourceViewRow(TSharedPtr<FSourceTreeNode>InItem, const TSharedRef<STableViewBase>&OwnerTable);
	TSharedRef<ITableRow>OnGenerateCompilerResultRow(TSharedPtr<FCompilerResults>InItem, const TSharedRef<STableViewBase>&OwnerTable);
protected:
	EVisibility GetSourceTreeViewVisibility() const;
	EVisibility GetSourceTreeSearchVisibility() const;
protected:
	FText GetToolkitTitle() const;
protected:
	void ExtendMenu();
	void BindCommands();
	void ExtendToolbar();
protected:
	void FocusToolkitTab(const FName &Tab);
	void CloseToolkitTab(const FName &Tab);
protected:
	void OnCompilationFinished(const UMagicNodeSharpSource* Script, const FCompilerResults Results);
protected:
	void OnClickedSourceViewItem(TSharedPtr<FSourceTreeNode>TreeItem);
	void OnClickedCompilerResultItem(TSharedPtr<FCompilerResults>Item);
	void OnExpansionChanged(TSharedPtr<FSourceTreeNode>InItem, bool WasExpanded);
	void OnSelectedSourceViewItem(TSharedPtr<FSourceTreeNode>TreeItem, ESelectInfo::Type SelectInfo);
	void OnSourceViewCheckStatusChanged(ECheckBoxState NewCheckState, TSharedPtr<FSourceTreeNode>NodeChanged);
	void OnGetSourceViewChildren(TSharedPtr<FSourceTreeNode>InItem, TArray<TSharedPtr<FSourceTreeNode>>&OutChildren);
protected:
	void AppendCompilerResults(const FCompilerResults &Result);
	void AddNotification(FNotificationInfo &Info, bool Success);
protected:
	void CompileScript();
protected:
	void LaunchVSCode();
	void LaunchVStudio();
	void GenerateVSCode();
	void LaunchHelpWiki();
	void LaunchSourceCodeDIFF();
	void LaunchSourceCodeEditor();
public:
	FCS_Toolkit();
	virtual ~FCS_Toolkit();
public:
	UMagicNodeSharp* GetScriptCDO();
	UMagicNodeSharpSource* GET() const;
public:
	void SET(UMagicNodeSharpSource* NewScriptObject);
	void INIT(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>&InitToolkitHost, UMagicNodeSharpSource* WithScriptObject);
public:
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>&TABManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>&TABManager) override;
public:
	virtual FText GetToolkitName() const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetDocumentationLink() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
public:
	virtual void OnToolkitHostingStarted(const TSharedRef<IToolkit>&Toolkit) override;
	virtual void OnToolkitHostingFinished(const TSharedRef<IToolkit>&Toolkit) override;
public:
	virtual void AddReferencedObjects(FReferenceCollector &Collector) override;
public:
	bool IsCompiling() const;
	bool CanCompileScript() const;
public:
	void OnScriptDeleted();
	void OnSearchChanged(const FText &Filter);
	void OnScriptExported(UMagicNodeSharpSource* Source);
	void OnAssetDeleted(const TArray<UClass*>&DeletedAssetClasses);
	void OnProjectDirectoryChanged(const TArray<FFileChangeData>&Data);
	void OnSearchCommitted(const FText &NewText, ETextCommit::Type CommitInfo);
public:
	static bool IsSourceFile(const FString &Path);
	static int32 SourceViewCount() {return ScriptSourcePaths.Num();}
public:
	static void RefreshScriptTreeView();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////