// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "CognitiveEditorData.h"
#include "Cognitive3DSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
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
#include "Cognitive3DActor.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "C3DCommonEditorTypes.h"
#include "Misc/DateTime.h"
#include "Cognitive3DEditorModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorLevelUtils.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Misc/ScopedSlowTask.h"

class FCognitiveTools;
class FCognitive3DEditorModule;
class FCognitiveEditorTools;

/**
 * 
 */
class SProjectManagerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SProjectManagerWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	FReply OpenFullC3DSetupWindow();

	//project setup
	//dev and api key

	FReply ValidateKeys();

	void CheckForExpiredDeveloperKey(FString developerKey);
	void OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void FetchApplicationKey(FString developerKey);
	void GetApplicationKeyResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void FetchOrganizationDetails(FString developerKey);
	void GetOrganizationDetailsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	TMap<FString, bool> SDKCheckboxStates;
	//void SProjectManagerWidget::OnSDKCheckedChanged(ECheckBoxState NewState, FString SDKName);

	FString DisplayAPIKey;
	FText GetDisplayAPIKey() const;
	void OnAPIKeyChanged(const FText& Text);

	FString DisplayDeveloperKey;
	FText GetDisplayDeveloperKey() const;
	void OnDeveloperKeyChanged(const FText& Text);

	TSharedPtr<STextBlock> OrgNameTextBlock;
	TSharedPtr<STextBlock> OrgSubscriptionTextBlock;
	TSharedPtr<STextBlock> OrgExpiryTextBlock;
	TSharedPtr<STextBlock> OrgTrialTextBlock;

	//export path
	FString DisplayExportDirectory;
	const FSlateBrush* GetExportPathStateIcon() const;

	FText GetExportPathTooltipText() const;

	void OnExportPathChanged(const FText& Text);

	//player setup
	FText GetInputClassText() const;
	FReply AppendInputs();
	EVisibility GetAppendedInputsFoundVisibility() const;
	EVisibility GetAppendedInputsFoundHidden() const;
	EVisibility GetDefaultInputClassEnhanced() const;

	//scene setup
	TSharedPtr<SVerticalBox> SceneChecklistContainer;
	// Store checkbox state per level
	TMap<FString, bool> LevelSelectionMap;
	void CollectAllMaps();
	void ExportSelectedMaps();
	void RebuildSceneChecklist();

	//third party SDK setup
	void ApplySDKToggle(const FString& SDKName, bool bEnable);
	bool IsSDKEnabledInBuildCs(const FString& MethodName);
	void RestartEditor();
};
