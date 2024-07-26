/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CognitiveEditorTools.h"
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
#include "Cognitive3DEditorModule.h"
#include "Editor/EditorEngine.h"

class FCognitiveTools;
class FCognitive3DEditorModule;

//screens:
//intro + dev key check
//controller setup, input.ini
//export and export path
//upload checklist
//upload progress bar
//complete

class SSceneSetupWidget : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SSceneSetupWidget){}
	SLATE_ARGUMENT(FSlateBrush*,ScreenshotTexture)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);
	void CheckForExpiredDeveloperKey();
	void OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void OnSceneUploaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UENUM()
	enum class ESceneSetupPage : uint8
	{
		Invalid, //dev key isn't set
		Intro,
		Controller,
		Export,
		UploadChecklist,
		UploadProgress,
		Complete
	};
	ESceneSetupPage CurrentPageEnum = ESceneSetupPage::Intro;

	UENUM()
	enum class ESceneUploadStatus : uint8
	{
		NotStarted,
		Uploading,
		Completed
	};
	ESceneUploadStatus SceneUploadStatus = ESceneUploadStatus::NotStarted;

	TArray<TSharedPtr<FDynamicData>> GetSceneDynamics();

	bool SceneWasExported = false;

	FReply DebugNextPage();
	FReply DebugPreviousPage();

	
	FText GetHeaderTitle() const;
	EVisibility IsInvalidVisible() const;
	EVisibility IsIntroVisible() const;
	EVisibility IsControllerVisible() const;
	EVisibility IsExportVisible() const;
	EVisibility IsUploadChecklistVisible() const;
	EVisibility IsUploadProgressVisible() const;
	EVisibility IsCompleteVisible() const;
	EVisibility IsUploadComplete() const;

	EVisibility IsOnlyExportSelected() const;
	EVisibility IsNotOnlyExportSelected() const;

	FText ExportButtonText() const;

	EVisibility IsNewSceneUpload() const;
	EVisibility IsSceneVersionUpload() const;
	EVisibility IsIntroNewVersionVisible() const;
	EVisibility UploadErrorVisibility() const;
	EVisibility UploadThumbnailTextVisibility() const;


	FReply NextPage();
	EVisibility NextButtonVisibility() const;
	bool NextButtonEnabled() const;
	FText NextButtonText() const;
	FText GetNextButtonTooltipText() const;

	EVisibility BackButtonVisibility() const;
	FReply LastPage();
	
	FText UploadErrorText() const;
	EVisibility DisplayWizardThrobber() const;

	TSharedPtr<SImage> ScreenshotImage;
	int32 CountDynamicObjectsInScene() const;
	EVisibility HasExportedSceneTextVisibility() const;
	FText ExportedSceneText() const;

	FReply SelectAll();


	void GenerateScreenshotBrush();
	FSlateBrush* ScreenshotTexture;
	const FSlateBrush* GetScreenshotBrushTexture() const;
	FString ConstructDashboardURL();
	FReply TakeScreenshot();
	int32 ScreenshotWidth = 256;
	int32 ScreenshotHeight = 256;
	FOptionalSize GetScreenshotWidth() const;
	FOptionalSize GetScreenshotHeight() const;

	FReply EvaluateSceneExport();
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

	ECheckBoxState GetCompressFilesCheckbox() const;
	void OnChangeCompressFilesCheckbox(ECheckBoxState newstate)
	{
		if (newstate == ECheckBoxState::Checked)
		{
			FCognitiveEditorTools::GetInstance()->CompressExportedFiles = true;
		}
		else
		{
			FCognitiveEditorTools::GetInstance()->CompressExportedFiles = false;
		}
	}

	ECheckBoxState GetExportDynamicsCheckbox() const;
	void OnChangeExportDynamicsCheckbox(ECheckBoxState newstate)
	{
		if (newstate == ECheckBoxState::Checked)
		{
			FCognitiveEditorTools::GetInstance()->ExportDynamicsWithScene = true;
		}
		else
		{
			FCognitiveEditorTools::GetInstance()->ExportDynamicsWithScene = false;
		}
	}

	void OnExportPathChanged(const FText& Text);

	/// <summary>
	/// checks if there's a BP_Cognitive3DActor in the world. spawns one if not
	/// </summary>
	void SpawnCognitive3DActor();
	FReply OpenProjectSetupWindow();

	const FSlateBrush* GetControllerConfigureBrush() const;
	FSlateBrush* ControllerConfigureBrush;
	const FSlateBrush* GetControllerComponentBrush() const;
	FSlateBrush* ControllerComponentBrush;

	FReply AppendInputs();
	EVisibility GetAppendedInputsFoundVisibility() const;
	EVisibility GetAppendedInputsFoundHidden() const;

	FText GetDynamicObjectCountToUploadText() const;
	FText GetSceneVersionToUploadText() const;
};