#pragma once

#include "CognitiveEditorTools.h"
#include "CognitiveEditorData.h"
#include "CognitiveVRSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
#include "SDynamicObjectListWidget.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"
#include "SCheckBox.h"
#include "STableRow.h"
#include "STextComboBox.h"
#include "SListView.h"
#include "SThrobber.h"
#include "Runtime/SlateCore/Public/Layout/Visibility.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Runtime/Online/HTTP/Public/Http.h"

class FCognitiveTools;
class FCognitiveVREditorModule;

class SDynamicObjectManagerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDynamicObjectManagerWidget){}
	SLATE_ARGUMENT(TArray<TSharedPtr<FDynamicData>>, Items)
	//SLATE_ARGUMENT(FCognitiveEditorTools*, CognitiveEditorTools)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);
	void CheckForExpiredDeveloperKey();
	void OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FString DisplayDeveloperKey;
	FText GetDisplayDeveloperKey() const;
	void OnDeveloperKeyChanged(const FText& Text);

	TArray<TSharedPtr<FDynamicData>> GetSceneDynamics();

	/* Adds a new textbox with the string to the list */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FDynamicData> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/* The list of strings */
	TArray<TSharedPtr<FDynamicData>> Items;
	//FCognitiveTools* CognitiveTools;

	int32 CurrentPage = 0;
	bool SceneWasExported = false;

	EVisibility UploadErrorVisibility() const;
	FText UploadErrorText() const;

	//TSharedRef<ITableRow> OnGenerateSceneExportFileRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedPtr<SDynamicObjectListWidget> SceneDynamicObjectList;
	TSharedPtr<SImage> ScreenshotImage;

	FText DisplayDynamicObjectsCountInScene() const;
	FReply RefreshDisplayDynamicObjectsCountInScene();
	EVisibility GetDuplicateDyanmicObjectVisibility() const;
	
	int32 CountDynamicObjectsInScene() const;

	FText DynamicCountInScene;

	/* The actual UI list */
	//TSharedPtr< SListView< TSharedPtr<FDynamicData> > > ListViewWidget;

	FReply SelectDynamic(TSharedPtr<FDynamicData> data);

	FReply SelectAll();

	void RefreshList();

	FReply ValidateAndRefresh();

	//FReply EvaluateSceneExport();
	bool NoExportGameplayMeshes = true;
	ECheckBoxState GetNoExportGameplayMeshCheckbox() const;
	void OnChangeNoExportGameplayMesh(ECheckBoxState newstate)
	{
		if (newstate == ECheckBoxState::Checked)
		{
			NoExportGameplayMeshes = true;
		}
		else
		{
			NoExportGameplayMeshes = false;
		}
	}

	bool bSettingsVisible = true;
	FReply ToggleSettingsVisible();

	bool OnlyExportSelected;
	ECheckBoxState GetOnlyExportSelectedCheckbox() const;
	void OnChangeOnlyExportSelected(ECheckBoxState newstate)
	{
		if (newstate == ECheckBoxState::Checked)
		{
			OnlyExportSelected = true;
		}
		else
		{
			OnlyExportSelected = false;
		}
	}

	void OnExportPathChanged(const FText& Text);
	void OnBlenderPathChanged(const FText& Text);
	EVisibility AreSettingsVisible() const;

	FReply ExportSelectedDynamicData();
	FReply UploadSelectedDynamicData();

	bool IsExportAllEnabled() const;
	bool IsExportSelectedEnabled() const;

	bool IsUploadAllEnabled() const;
	bool IsUploadSelectedEnabled() const;
	bool IsUploadIdsEnabled() const;

	FText UploadAllText() const;
	FText UploadSelectedText() const;
	FText ExportSelectedText() const;
	FText GetSettingsButtonText() const;
	FText GetSceneText() const;
};