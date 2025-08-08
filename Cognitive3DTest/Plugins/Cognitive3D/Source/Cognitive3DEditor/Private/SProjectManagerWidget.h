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

	// near the top of the class
private:
	// Combined list to drive the Slate ListView
	TArray<TSharedPtr<FEditorSceneData>> SceneListItems;

	// The actual ListView widget
	TSharedPtr< SListView< TSharedPtr<FEditorSceneData> > > SceneListView;

	// Row-generation callback
	TSharedRef<ITableRow> OnGenerateSceneRow(
		TSharedPtr<FEditorSceneData> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

public:
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
	void FinalizeProjectSetup();

	TMap<TSharedPtr<FEditorSceneData>, FString> SceneItemToPath;

	//compression toggle helpers
	ECheckBoxState IsCompressTexturesChecked() const { return FCognitiveEditorTools::GetInstance()->CompressExportedFiles ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }
	void OnCompressTexturesChanged(ECheckBoxState NewState) { FCognitiveEditorTools::GetInstance()->CompressExportedFiles = (NewState == ECheckBoxState::Checked); }

	//delegates
	FOnUploadAllSceneGeometry OnUploadAllSceneGeometry;
	FOnExportAllSceneGeometry OnExportAllSceneGeometry;
	//delegate callbacks
	void OnLevelsExported(bool bWasSuccessful);
	void OnLevelsUploaded(bool bWasSuccessful);

	//third party SDK setup
	void ApplySDKToggle(const FString& SDKName, bool bEnable);
	bool IsSDKEnabledInBuildCs(const FString& MethodName);
	void RestartEditor();

	bool bDidChangeSDKs = false;
};
