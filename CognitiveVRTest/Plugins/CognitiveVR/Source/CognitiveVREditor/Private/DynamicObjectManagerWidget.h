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

	TArray<TSharedPtr<FDynamicData>> GetSceneDynamics();
	TSharedPtr<SDynamicObjectTableWidget> SceneDynamicObjectTable;

	FReply RefreshDisplayDynamicObjectsCountInScene();
	EVisibility GetDuplicateDyanmicObjectVisibility() const;

	FReply SelectDynamic(TSharedPtr<FDynamicData> data);

	void RefreshList();

	FReply ValidateAndRefresh();

	//TODO make export path and blender path configurable here
	void OnExportPathChanged(const FText& Text);
	void OnBlenderPathChanged(const FText& Text);

	FReply UploadSelectedDynamicObjects();
	FReply UploadAllDynamicObjects();

	EVisibility GetSceneWarningVisibility() const;

	bool IsUploadAllEnabled() const;
	bool IsUploadSelectedEnabled() const;
	bool IsUploadIdsEnabled() const;
	bool IsUploadInvalid() const;
	FText GetUploadInvalidCause() const;

	FText UploadSelectedText() const;
	FText ExportSelectedText() const;
	FText GetSceneText() const;

	static TArray<FDashboardObject> dashboardObjects;
	void GetDashboardManifest();
	void OnDashboardManifestResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};