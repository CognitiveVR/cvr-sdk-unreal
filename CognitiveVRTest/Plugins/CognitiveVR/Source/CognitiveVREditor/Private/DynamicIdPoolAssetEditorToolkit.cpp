// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DynamicIdPoolAssetEditorToolkit.h"

#include "Editor.h"
#include "EditorReimportHandler.h"
#include "EditorStyleSet.h"
//#include "SDynamicIdPoolAssetEditor.h"
#include "DynamicIdPoolAsset.h"
#include "UObject/NameTypes.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FTextAssetEditorToolkit"

DEFINE_LOG_CATEGORY_STATIC(LogTextAssetEditor, Log, All);


/* Local constants
 *****************************************************************************/

namespace TextAssetEditor
{
	static const FName AppIdentifier("TextAssetEditorApp");
	static const FName TabId("TextEditor");
}


/* FTextAssetEditorToolkit structors
 *****************************************************************************/

FDynamicIdPoolAssetEditorToolkit::FDynamicIdPoolAssetEditorToolkit(const TSharedRef<ISlateStyle>& InStyle)
	: DynamicIdPoolAsset(nullptr)
	, Style(InStyle)
{ }


FDynamicIdPoolAssetEditorToolkit::~FDynamicIdPoolAssetEditorToolkit()
{
	FReimportManager::Instance()->OnPreReimport().RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);

	GEditor->UnregisterForUndo(this);
}


/* FTextAssetEditorToolkit interface
 *****************************************************************************/

void FDynamicIdPoolAssetEditorToolkit::Initialize(UDynamicIdPoolAsset* InTextAsset, const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost)
{
	DynamicIdPoolAsset = InTextAsset;

	// Support undo/redo
	DynamicIdPoolAsset->SetFlags(RF_Transactional);
	GEditor->RegisterForUndo(this);

	// create tab layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_TextAssetEditor")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
						->SetOrientation(Orient_Vertical)
						->SetSizeCoefficient(0.66f)
						->Split
						(
							FTabManager::NewStack()
								->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.1f)
								
						)
						->Split
						(
							FTabManager::NewStack()
								->AddTab(TextAssetEditor::TabId, ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.9f)
						)
				)
		);

	//FAssetEditorToolkit::InitAssetEditor(
	//	InMode,
	//	InToolkitHost,
	//	TextAssetEditor::AppIdentifier,
	//	Layout,
	//	true /*bCreateDefaultStandaloneMenu*/,
	//	true /*bCreateDefaultToolbar*/,
	//	InTextAsset
	//);

	RegenerateMenusAndToolbars();
}


/* FAssetEditorToolkit interface
 *****************************************************************************/

FString FDynamicIdPoolAssetEditorToolkit::GetDocumentationLink() const
{
	return FString(TEXT("https://github.com/ue4plugins/TextAsset"));
}


void FDynamicIdPoolAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_TextAssetEditor", "Text Asset Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(TextAssetEditor::TabId, FOnSpawnTab::CreateSP(this, &FDynamicIdPoolAssetEditorToolkit::HandleTabManagerSpawnTab, TextAssetEditor::TabId))
		.SetDisplayName(LOCTEXT("TextEditorTabName", "Text Editor"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
}


void FDynamicIdPoolAssetEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(TextAssetEditor::TabId);
}


/* IToolkit interface
 *****************************************************************************/

FText FDynamicIdPoolAssetEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Text Asset Editor");
}


FName FDynamicIdPoolAssetEditorToolkit::GetToolkitFName() const
{
	return FName("TextAssetEditor");
}


FLinearColor FDynamicIdPoolAssetEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}


FString FDynamicIdPoolAssetEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "TextAsset ").ToString();
}


/* FGCObject interface
 *****************************************************************************/

void FDynamicIdPoolAssetEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(DynamicIdPoolAsset);
}


/* FEditorUndoClient interface
*****************************************************************************/

void FDynamicIdPoolAssetEditorToolkit::PostUndo(bool bSuccess)
{ }


void FDynamicIdPoolAssetEditorToolkit::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}


/* FTextAssetEditorToolkit callbacks
 *****************************************************************************/

TSharedRef<SDockTab> FDynamicIdPoolAssetEditorToolkit::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier)
{
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

	//if (TabIdentifier == TextAssetEditor::TabId)
	//{
	//	TabWidget = SNew(SDynamicIdPoolAssetEditor, DynamicIdPoolAsset, Style);
	//}

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			TabWidget.ToSharedRef()
		];
}


#undef LOCTEXT_NAMESPACE
