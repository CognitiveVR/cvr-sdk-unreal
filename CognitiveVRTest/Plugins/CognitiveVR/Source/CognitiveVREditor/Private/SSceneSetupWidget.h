#pragma once

#include "CognitiveEditorTools.h"
#include "CognitiveEditorData.h"
#include "CognitiveVRSettings.h"
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
#include "CognitiveVRActor.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "C3DCommonEditorTypes.h"
#include "CognitiveVREditorModule.h"

class FCognitiveTools;
class FCognitiveVREditorModule;

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


	UENUM()
	enum class EPage : uint8
	{
		Invalid, //dev key isn't set
		Intro,
		Controller,
		Export,
		UploadChecklist,
		UploadProgress,
		Complete
	};
	EPage CurrentPageEnum = EPage::Intro;

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


	EVisibility IsNewSceneUpload() const;
	EVisibility IsSceneVersionUpload() const;
	EVisibility IsIntroNewVersionVisible() const;
	EVisibility UploadErrorVisibility() const;



	FReply NextPage();
	EVisibility NextButtonVisibility() const;
	bool NextButtonEnabled() const;
	FText NextButtonText() const;
	EVisibility BackButtonVisibility() const;
	FReply LastPage();
	
	FText UploadErrorText() const;
	EVisibility DisplayWizardThrobber() const;

	TSharedPtr<SImage> ScreenshotImage;

	FText ExportNextButtonText;

	FText DisplayDynamicObjectsCountInScene() const;
	
	int32 CountDynamicObjectsInScene() const;

	FText DynamicCountInScene;

	FReply SelectAll();


	void GetScreenshotBrush();
	FSlateBrush* ScreenshotTexture;
	const FSlateBrush* GetScreenshotBrushTexture() const;
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

	void OnExportPathChanged(const FText& Text);
	void OnBlenderPathChanged(const FText& Text);

	/// <summary>
	/// checks if there's a BP_CognitiveVRActor in the world. spawns one if not
	/// </summary>
	void SpawnCognitiveVRActor();
	FReply OpenProjectSetupWindow();

	const FSlateBrush* GetControllerConfigureBrush() const;
	FSlateBrush* ControllerConfigureBrush;
	const FSlateBrush* GetControllerComponentBrush() const;
	FSlateBrush* ControllerComponentBrush;
};