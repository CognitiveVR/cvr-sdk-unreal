/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CognitiveEditorTools.h"
#include "CognitiveEditorData.h"
#include "Cognitive3DSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
#include "DynamicObjectTableWidget.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"
#include "SCheckBox.h"
#include "STableRow.h"
#include "STextComboBox.h"
#include "SListView.h"
#include "SThrobber.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "Runtime/SlateCore/Public/Layout/Visibility.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "C3DCommonEditorTypes.h"
#include "Cognitive3DEditorModule.h"

class FCognitiveTools;
class SDynamicObjectTableWidget;
class FCognitive3DEditorModule;

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

	//TODO make export path configurable here
	void OnExportPathChanged(const FText& Text);

	FReply UploadSelectedDynamicObjects();
	FReply UploadAllDynamicObjects();

	//assign dynamics to objects in the scene
	FReply AssignDynamicsToActors();
	bool IsActorInSceneSelected() const;
	FText AssignDynamicTooltip() const;

	EVisibility GetSceneWarningVisibility() const;

	EVisibility SceneNotUploadedVisibility() const;
	EVisibility SceneUploadedVisibility() const;

	bool IsUploadAllEnabled() const;
	bool IsUploadSelectedEnabled() const;
	FText GetUploadInvalidCause() const;

	FText UploadSelectedText() const;
	FText GetSceneText() const;

	FText UploadSelectedMeshesTooltip() const;
	FText UploadAllMeshesTooltip() const;

	//static so its easier to set content from response without getting a reference to this window. especially for updating the sub-widgets!
	static TArray<FDashboardObject> dashboardObjects;
	void GetDashboardManifest();
	void OnDashboardManifestResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FReply ExportAndOpenSceneSetupWindow();
};