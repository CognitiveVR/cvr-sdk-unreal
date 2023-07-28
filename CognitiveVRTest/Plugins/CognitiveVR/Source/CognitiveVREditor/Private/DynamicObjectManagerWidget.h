#pragma once

#include "CognitiveEditorTools.h"
#include "CognitiveEditorData.h"
#include "CognitiveVRSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
#include "SDynamicObjectTableWidget.h"
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
#include "C3DCommonEditorTypes.h"

class FCognitiveTools;
class SDynamicObjectTableWidget;
class FCognitiveVREditorModule;

class SDynamicObjectManagerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDynamicObjectManagerWidget){}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);
	void CheckForExpiredDeveloperKey();
	void OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FString DisplayDeveloperKey;
	FText GetDisplayDeveloperKey() const;
	void OnDeveloperKeyChanged(const FText& Text);

	TArray<TSharedPtr<FDynamicData>> GetSceneDynamics();
	TSharedPtr<SDynamicObjectTableWidget> SceneDynamicObjectTable;
	bool SceneWasExported = false;

	EVisibility UploadErrorVisibility() const;
	FText UploadErrorText() const;

	FText DisplayDynamicObjectsCountInScene() const;
	FReply RefreshDisplayDynamicObjectsCountInScene();
	EVisibility GetDuplicateDyanmicObjectVisibility() const;
	
	int32 CountDynamicObjectsInScene() const;

	FText DynamicCountInScene;

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

	FReply UploadSelectedDynamicObjects();
	FReply UploadAllDynamicObjects();

	bool IsExportAllEnabled() const;
	bool IsExportSelectedEnabled() const;

	bool IsUploadAllEnabled() const;
	bool IsUploadSelectedEnabled() const;
	bool IsUploadIdsEnabled() const;
	bool IsUploadInvalid() const;
	FText GetUploadInvalidCause() const;

	FText UploadSelectedText() const;
	FText ExportSelectedText() const;
	FText GetSettingsButtonText() const;
	FText GetSceneText() const;

	static TArray<FDashboardObject> dashboardObjects;
	void GetDashboardManifest();
	void OnDashboardManifestResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};