/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "SceneSetupWidget.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

TArray<TSharedPtr<FDynamicData>> SSceneSetupWidget::GetSceneDynamics()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneDynamics();
}
FOptionalSize SSceneSetupWidget::GetScreenshotWidth() const
{
	return FOptionalSize(ScreenshotWidth);
}

FOptionalSize SSceneSetupWidget::GetScreenshotHeight() const
{
	return FOptionalSize(ScreenshotHeight);
}

void SSceneSetupWidget::CheckForExpiredDeveloperKey()
{
	if (FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		GConfig->Flush(true, GEngineIni);
		auto Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &SSceneSetupWidget::OnDeveloperKeyResponseReceived);
		FString gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
		FString url = "https://" + gateway + "/v0/apiKeys/verify";
		Request->SetURL(url);
		Request->SetVerb("GET");
		Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + FCognitiveEditorTools::GetInstance()->DeveloperKey);
		Request->ProcessRequest();
	}
	else //no developer key found
	{
		CurrentPageEnum = ESceneSetupPage::Invalid;
		GLog->Log("Developer Key Response Code is not 200. Developer key may be invalid or expired");
	}
}

void SSceneSetupWidget::OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid() == false)
	{
		SGenericDialogWidget::OpenDialog(FText::FromString("Your developer key has expired"), SNew(STextBlock).Text(FText::FromString("Please log in to the dashboard, select your project, and generate a new developer key.\n\nNote:\nDeveloper keys allow you to upload and modify Scenes, and the keys expire after 90 days.\nApplication keys authorize your app to send data to our server, and they never expire.")));
		GLog->Log("Developer Key Response is invalid. Developer key may be invalid or expired. Check your internet connection");
		return;
	}

	int32 responseCode = Response->GetResponseCode();
	if (responseCode == 200)
	{
		GLog->Log("Developer Key Response Code is 200");
		if (CurrentPageEnum == ESceneSetupPage::Invalid)
		{
			CurrentPageEnum = ESceneSetupPage::Intro;
		}
	}
	else
	{
		CurrentPageEnum = ESceneSetupPage::Invalid;
		//SGenericDialogWidget::OpenDialog(FText::FromString("Your developer key has expired"), SNew(STextBlock).Text(FText::FromString("Please log in to the dashboard, select your project, and generate a new developer key.\n\nNote:\nDeveloper keys allow you to upload and modify Scenes, and the keys expire after 90 days.\nApplication keys authorize your app to send data to our server, and they never expire.")));
		GLog->Log("Developer Key Response Code is not 200. Developer key may be invalid or expired");
	}
}

void SSceneSetupWidget::OnSceneUploaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	SceneUploadStatus = ESceneUploadStatus::Completed;
	CurrentPageEnum = ESceneSetupPage::Complete;
}

void SSceneSetupWidget::Construct(const FArguments& Args)
{
	float padding = 10;
	FCognitiveEditorTools::CheckIniConfigured();
	CheckForExpiredDeveloperKey();
	FCognitiveEditorTools::GetInstance()->WizardUploadError = "";
	FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode = 0;

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					//.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SRichTextBlock)
						.Justification(ETextJustify::Left)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(this, &SSceneSetupWidget::GetHeaderTitle)
					]
				]
			]

			+ SOverlay::Slot()
			[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Text(FText::FromString(""))
			]

#pragma region invalid screen
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0,10,0, padding)
			[
				SNew(SRichTextBlock)
				.Justification(ETextJustify::Center)
				.Visibility(this, &SSceneSetupWidget::IsInvalidVisible)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("The developer key is invalid. Please set the developer key in the Project Setup Window"))
			]

			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.MaxHeight(30)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsInvalidVisible)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsInvalidVisible)
					.HeightOverride(32)
					.WidthOverride(256)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Visibility(this,&SSceneSetupWidget::IsInvalidVisible)
						.Text(FText::FromString("Open Project Setup Window"))
						.OnClicked(this, &SSceneSetupWidget::OpenProjectSetupWindow)
					]
				]
			]

#pragma endregion

#pragma region "intro screen"
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0,0,0, padding)
			[
				SNew(SRichTextBlock)
				.Justification(ETextJustify::Center)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.AutoWrapText(true)
				.Text(FText::FromString("Welcome to the <RichTextBlock.BoldHighlight>Cognitive3D Level Setup</>. This window will guide you through a basic configuration to ensure your level is ready to record data."))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.Text(FText::FromString("This will include:\n\n-Setting up your Player Actor\n\n-Setting up controller inputs\n\n-Exporting Level Geometry to SceneExplorer"))
			]

#pragma endregion

#pragma region "controller screen"

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
				+SHorizontalBox::Slot()
				[
					SNew(SRichTextBlock)
					.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
					.AutoWrapText(true)
					.Justification(ETextJustify::Center)
					.DecoratorStyleSet(&FEditorStyle::Get())
					.Text(FText::FromString("Open the VR Pawn blueprint that is spawned for your player.\n\n\n\nAttach Dynamic Object components as children of each MotionController actor component."))
				]
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.WidthOverride(265)
					.HeightOverride(215)
					.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
					[
						SNew(SImage)
						.Image(this, &SSceneSetupWidget::GetControllerComponentBrush)
					]
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SSeparator)
				.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
				+SHorizontalBox::Slot()
				[
					SNew(SRichTextBlock)
					.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
					.AutoWrapText(true)
					.Justification(ETextJustify::Center)
					.DecoratorStyleSet(&FEditorStyle::Get())
					.Text(FText::FromString("Press the Setup Left Controller and Setup Right Controller buttons on the appropriate actors.\n\n\n\nSave your changes."))
				]
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.WidthOverride(265)
					.HeightOverride(138)
					.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
					[
						SNew(SImage)
						.Image(this, &SSceneSetupWidget::GetControllerConfigureBrush)
					]
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SSeparator)
				.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::GetAppendedInputsFoundHidden)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("You can append inputs to your DefaultInput.ini file. This will allow you to visualize the button presses of the player."))
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::GetAppendedInputsFoundVisibility)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("The Cognitive3D action maps have been added to DefaultInputs.ini"))
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(SBox)
				.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
						.WidthOverride(256)
						.HeightOverride(32)
						[
							SNew(SButton)
							.Text(FText::FromString("Append Input Data to Input.ini"))
							.OnClicked(this, &SSceneSetupWidget::AppendInputs)
						]
					]
				]
			]

#pragma endregion

#pragma region "export screen"

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("The current level geometry will be exported and uploaded to our dashboard. This will provide context for the spatial data points we automatically collect."))
			]

			//path to export directory
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			//.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				+SHorizontalBox::Slot()
				.MaxWidth(200)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsExportVisible)
					.HeightOverride(32)
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsExportVisible)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.Text(FText::FromString("Path to Export Directory"))
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(1)
				.FillWidth(3)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsExportVisible)
					.HeightOverride(32)
					.MaxDesiredHeight(32)
					[
						SNew(SEditableTextBox)
						.Visibility(this, &SSceneSetupWidget::IsExportVisible)
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBaseExportDirectoryDisplay)
						.OnTextChanged(this, &SSceneSetupWidget::OnExportPathChanged)
					]
				]
				+SHorizontalBox::Slot()
				.MaxWidth(17)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.Visibility(this, &SSceneSetupWidget::IsExportVisible)
						.HeightOverride(17)
						.WidthOverride(17)
						[
							SNew(SButton)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							//PickerWidget = SAssignNew(BrowseButton, SButton)
							.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
							.ToolTipText(LOCTEXT("FolderButtonToolTipText", "Choose a directory from this computer"))
							.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SelectBaseExportDirectory)
							.ContentPadding(2.0f)
							.ForegroundColor(FSlateColor::UseForeground())
							.IsFocusable(false)
							[
								SNew(SImage)
								.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Ellipsis"))
								.ColorAndOpacity(FSlateColor::UseForeground())
							]
						]
					]
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SSeparator)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
			]

			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsNotOnlyExportSelected)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("SettingsEditor.GoodIcon"))
				]
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Visibility(this, &SSceneSetupWidget::IsNotOnlyExportSelected)
					.Text(FText::FromString("All geometry in the scene will be exported"))
				]
			]

			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsOnlyExportSelected)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("SettingsEditor.WarningIcon"))
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Visibility(this, &SSceneSetupWidget::IsOnlyExportSelected)
					.Text(FText::FromString("Only selected items will be exported. This might cause loss of context when viewing in Scene Explorer"))
				]
			]

			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				[
					SNew(SCheckBox)
					.Visibility(this, &SSceneSetupWidget::IsExportVisible)
					.IsChecked(this,&SSceneSetupWidget::GetOnlyExportSelectedCheckbox)
					.OnCheckStateChanged(this,&SSceneSetupWidget::OnChangeOnlyExportSelected)
				]
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Visibility(this, &SSceneSetupWidget::IsExportVisible)
					.Text(FText::FromString("Only Export Selected"))
				]
			]

			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.MaxHeight(40)
			.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(256)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(128)
						.HeightOverride(32)
						[
							SNew(SButton)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							.Text(FText::FromString("Select All"))
							.OnClicked(this,&SSceneSetupWidget::SelectAll)
						]
					]

					+SHorizontalBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(128)
						.HeightOverride(32)
						[
							SNew(SButton)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							.Text(this, &SSceneSetupWidget::ExportButtonText)
							.OnClicked(this, &SSceneSetupWidget::EvaluateSceneExport)
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::HasExportedSceneTextVisibility)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(this, &SSceneSetupWidget::ExportedSceneText)
			]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, padding)
				[
					SNew(SSeparator)
					.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				]

#pragma endregion

#pragma region "upload screen"

			+ SVerticalBox::Slot() //upload number of dynamic objects to scene
			.HAlign(HAlign_Center)
				.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsNewSceneUpload)
				.AutoWrapText(true)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>Upload a New Level to the Dashboard</>"))
			]
			
			+ SVerticalBox::Slot() //upload number of dynamic objects to scene
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsSceneVersionUpload)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>Upload a New Version of this Level to the Dashboard</>. This will archive the previous version of this level."))
			]

			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.WidthOverride(this,&SSceneSetupWidget::GetScreenshotWidth)
				.HeightOverride(this, &SSceneSetupWidget::GetScreenshotHeight)
				.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
				[
					SNew(SImage)
					.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
					.Image(this,&SSceneSetupWidget::GetScreenshotBrushTexture)
				]
			]

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.WidthOverride(256)
				.HeightOverride(40)
				.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Text(FText::FromString("Take Screenshot From Current View to set \na Dashboard Thumbnail"))
					.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
					.OnClicked(this, &SSceneSetupWidget::TakeScreenshot)
				]
			]

			+ SVerticalBox::Slot() //upload scene geometry to scene
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, 0)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
				.AutoWrapText(true)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>The Scene Geometry</>"))
			]
			+ SVerticalBox::Slot() //upload scene geometry to scene
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(this, &SSceneSetupWidget::GetSceneVersionToUploadText)
			]

			+ SVerticalBox::Slot() //upload number of dynamic objects to scene
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, 0)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
				.AutoWrapText(true)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>Dynamic Object Meshes</>"))
			]
			+ SVerticalBox::Slot() //upload scene geometry to scene
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(this, &SSceneSetupWidget::GetDynamicObjectCountToUploadText)
			]

			+ SVerticalBox::Slot() //upload thumbnail to scene
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, 0)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::UploadThumbnailTextVisibility)
				.AutoWrapText(true)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>Screenshot</>"))
			]
			+ SVerticalBox::Slot() //upload scene geometry to scene
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::UploadThumbnailTextVisibility)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Level Viewport Screenshot"))
			]

#pragma endregion

#pragma region uploading

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(0, 0, 0, padding)
			[
				SNew(SThrobber)
				.NumPieces(7)
				.Visibility(this,&SSceneSetupWidget::DisplayWizardThrobber)
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::DisplayWizardThrobber)
				.Text(FText::FromString("Uploading"))
			]

#pragma endregion

#pragma region "done screen"
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::UploadErrorVisibility)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("There was an error while uploading. Check the <RichTextBlock.BoldHighlight>Output Log</> for details"))
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::UploadErrorVisibility)
				.AutoWrapText(true)
				.ColorAndOpacity(FLinearColor::Red)
				.Justification(ETextJustify::Center)
				.Text(this,&SSceneSetupWidget::UploadErrorText)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("That's it!\n\nAfter saving your project, we recommend restarting the Unreal Editor. You will be recording user position, gaze and basic device information. Simply press play in the Unreal Editor or make a build for your target platform.\n\nPlease note that sessions run in the editor won't count towards aggregate metrics.\n\nYou can view sessions from the Dashboard."))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0, 5, 0, 5)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString("Open Dashboard"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenURL, ConstructDashboardURL())
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(64)
				.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("You can continue your integration to get more insights including:"))//\n\nCustom Events\n\nExitPoll surveys\n\nDynamic Objects\n\nMultiplayer
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0,5,0,5)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString("Custom Events"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenURL, FString("https://docs.cognitive3d.com/unreal/customevents/"))
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0,5,0,5)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString("ExitPoll surveys"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenURL, FString("https://docs.cognitive3d.com/unreal/exitpoll/"))
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0,5,0,5)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString("Dynamic Objects"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenURL, FString("https://docs.cognitive3d.com/unreal/dynamic-objects/"))
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0,5,0,5)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString("Multiplayer"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenURL, FString("https://docs.cognitive3d.com/unreal/multiplayer/"))
					]
				]
			]
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Or check out the getting started guide:"))
				]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0,5,0,5)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString("Open Documentation"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenURL, FString("https://docs.cognitive3d.com/unreal/get-started/"))
					]
				]
			]

			]
#pragma endregion

#pragma region "footer"

			+ SOverlay::Slot()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.Text(FText::FromString("Back"))
						.Visibility(this,&SSceneSetupWidget::BackButtonVisibility)
						.OnClicked(this, &SSceneSetupWidget::LastPage)
					]
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.Text(this,&SSceneSetupWidget::NextButtonText)
						.IsEnabled(this,&SSceneSetupWidget::NextButtonEnabled)
						.Visibility(this, &SSceneSetupWidget::NextButtonVisibility)
						.ToolTipText(this,&SSceneSetupWidget::GetNextButtonTooltipText)
						.OnClicked(this, &SSceneSetupWidget::NextPage)
					]
				]
			]
			//DEBUG NEXT/BACK BUTTONS
			//+ SOverlay::Slot()
			//.VAlign(VAlign_Bottom)
			//.HAlign(HAlign_Left)
			//[
			//	SNew(SHorizontalBox)
			//	+ SHorizontalBox::Slot()
			//	[
			//		SNew(SBox)
			//		.WidthOverride(128)
			//		.HeightOverride(32)
			//		[
			//			SNew(SButton)
			//			.Text(FText::FromString("Debug Back"))
			//			.OnClicked(this, &SSceneSetupWidget::DebugPreviousPage)
			//		]
			//	]
			//	+ SHorizontalBox::Slot()
			//	[
			//		SNew(SBox)
			//		.WidthOverride(128)
			//		.HeightOverride(32)
			//		[
			//			SNew(SButton)
			//			.Text(FText::FromString("Debug Next"))
			//			.OnClicked(this, &SSceneSetupWidget::DebugNextPage)
			//		]
			//	]
			//]

#pragma endregion
		];

		FString texturepath = IPluginManager::Get().FindPlugin(TEXT("Cognitive3D"))->GetBaseDir() / TEXT("Resources") / TEXT("controller-components.png");
		FName BrushName = FName(*texturepath);
		ControllerComponentBrush = new FSlateDynamicImageBrush(BrushName, FVector2D(265, 215));

		texturepath = IPluginManager::Get().FindPlugin(TEXT("Cognitive3D"))->GetBaseDir() / TEXT("Resources") / TEXT("controller-configure.png");
		BrushName = FName(*texturepath);
		ControllerConfigureBrush = new FSlateDynamicImageBrush(BrushName, FVector2D(265, 138));

		FCognitiveEditorTools::GetInstance()->RefreshSceneUploadFiles();
		FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
		FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
		SceneUploadStatus = ESceneUploadStatus::NotStarted;
}

FReply SSceneSetupWidget::EvaluateSceneExport()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorToolsCustomization::Select_Export_Meshes world is null"));
		return FReply::Handled();
	}

	//prepare actors in the scene to be exported
	//if actor has a skeletal mesh component, convert to static mesh and export the static mesh version of the actor
	//clean up after to maintain original level

	TArray<AActor*> ToBeExportedFinal = FCognitiveEditorTools::GetInstance()->PrepareSceneForExport(OnlyExportSelected);

	FCognitiveEditorTools::GetInstance()->ExportScene(ToBeExportedFinal);
	

	SceneWasExported = true;
	FCognitiveEditorTools::GetInstance()->RefreshSceneUploadFiles();
	return FReply::Handled();
}

FReply SSceneSetupWidget::SelectAll()
{
	UWorld* World = GEditor->GetLevelViewportClients()[0]->GetWorld();
	GEditor->Exec(World, TEXT("actor select all"));
	return FReply::Handled();
}

ECheckBoxState SSceneSetupWidget::GetOnlyExportSelectedCheckbox() const
{
	if (OnlyExportSelected)return ECheckBoxState::Checked;
	return ECheckBoxState::Unchecked;
}

void SSceneSetupWidget::GenerateScreenshotBrush()
{
	FString ScreenshotPath = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory() + "/screenshot/screenshot.png";
	FName BrushName = FName(*ScreenshotPath);

	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *ScreenshotPath))
	{
		return;
	}

	if (ScreenshotTexture != nullptr)
	{
		delete ScreenshotTexture;
	}

	auto imageWrapper = FCognitiveEditorTools::GetInstance()->ImageWrapper;

	if (imageWrapper.IsValid() && imageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		ScreenshotTexture = new FSlateDynamicImageBrush(BrushName, FVector2D(ScreenshotWidth, ScreenshotHeight));
	}
}

const FSlateBrush* SSceneSetupWidget::GetScreenshotBrushTexture() const
{
	return ScreenshotTexture;
}

EVisibility SSceneSetupWidget::IsSceneVersionUpload() const
{
	if (CurrentPageEnum != ESceneSetupPage::UploadChecklist) { return EVisibility::Collapsed; }
	TSharedPtr<FEditorSceneData> sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SSceneSetupWidget::IsIntroNewVersionVisible() const
{
	if (CurrentPageEnum != ESceneSetupPage::Intro) { return EVisibility::Collapsed; }

	TSharedPtr<FEditorSceneData> sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SSceneSetupWidget::IsNewSceneUpload() const
{
	if (CurrentPageEnum != ESceneSetupPage::UploadChecklist) { return EVisibility::Collapsed; }
	
	TSharedPtr<FEditorSceneData> sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

EVisibility SSceneSetupWidget::IsInvalidVisible() const
{
	return CurrentPageEnum == ESceneSetupPage::Invalid ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsIntroVisible() const
{
	return CurrentPageEnum == ESceneSetupPage::Intro ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsControllerVisible() const
{
	return CurrentPageEnum == ESceneSetupPage::Controller ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsExportVisible() const
{
	return CurrentPageEnum == ESceneSetupPage::Export ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsUploadProgressVisible() const
{
	return CurrentPageEnum == ESceneSetupPage::UploadChecklist ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsUploadChecklistVisible() const
{
	return CurrentPageEnum == ESceneSetupPage::UploadChecklist ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsCompleteVisible() const
{
	return CurrentPageEnum == ESceneSetupPage::Complete ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsUploadComplete() const
{
	if (SceneUploadStatus == ESceneUploadStatus::NotStarted)
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}
EVisibility SSceneSetupWidget::IsOnlyExportSelected() const
{
	if (CurrentPageEnum == ESceneSetupPage::Export)
	{

		if (OnlyExportSelected)
		{
			return EVisibility::Visible;
		}
		return EVisibility::Collapsed;
	}
	return EVisibility::Collapsed;
}

EVisibility SSceneSetupWidget::IsNotOnlyExportSelected() const
{
	if (CurrentPageEnum == ESceneSetupPage::Export)
	{
		if (!OnlyExportSelected)
		{
			return EVisibility::Visible;
		}
		return EVisibility::Collapsed;
	}
	return EVisibility::Collapsed;
}

FText SSceneSetupWidget::ExportButtonText()const
{
	if (CurrentPageEnum == ESceneSetupPage::Export)
	{
		if (OnlyExportSelected)
		{
			return FText::FromString(TEXT("Export Selected"));
		}
		return FText::FromString(TEXT("Export All"));
	}
	return FText::FromString(TEXT("Export All"));
}


FString SSceneSetupWidget::ConstructDashboardURL()
{
	FString outputString = "";
	//scene settings (name, id, version)
	for (auto& elem : FCognitiveEditorTools::GetInstance()->SceneData)
	{
		if (elem->Name == UGameplayStatics::GetCurrentLevelName(GWorld))
		{
			outputString += FString("https://app.cognitive3d.com/scenes/") + elem->Id;
			outputString += FString("/v/") + elem->Id;
			outputString += FString::FromInt(elem->VersionNumber);
			outputString += FString("/insights");
		}
	}

	return outputString;
}

FReply SSceneSetupWidget::TakeScreenshot()
{

	UE_LOG(LogTemp, Warning, TEXT("taking screenshot"));

	FCognitiveEditorTools::GetInstance()->SaveScreenshotToFile();


	FViewport* CurrentViewport = GEditor->GetActiveViewport();
	auto size = CurrentViewport->GetSizeXY();
	auto width = size.X;
	auto height = size.Y;

	int32 maxPixelSize = 384;

	if (height > width)
	{
		ScreenshotWidth = (int32)(((float)width / (float)height) * maxPixelSize);
		ScreenshotHeight = maxPixelSize;
	}
	else if (height < width)
	{
		ScreenshotHeight = (int32)(((float)height / (float)width) * maxPixelSize);
		ScreenshotWidth = maxPixelSize;
	}
	else
	{
		ScreenshotHeight = maxPixelSize;
		ScreenshotWidth = maxPixelSize;
	}

	//where its loaded
	GenerateScreenshotBrush();
	return FReply::Handled();
}

FReply SSceneSetupWidget::DebugNextPage()
{
	CurrentPageEnum = (ESceneSetupPage)(((uint8)CurrentPageEnum) + 1);
	return FReply::Handled();
}
FReply SSceneSetupWidget::DebugPreviousPage()
{
	CurrentPageEnum = (ESceneSetupPage)(((uint8)CurrentPageEnum) - 1);
	return FReply::Handled();
}

FText SSceneSetupWidget::GetNextButtonTooltipText() const
{
	if (CurrentPageEnum == ESceneSetupPage::Export)
	{
		if (FCognitiveEditorTools::GetInstance()->GetSceneExportFileCount() == 0)
		{
			return FText::FromString("You must export the scene geometry to continue");
		}
		else if (FCognitiveEditorTools::GetInstance()->HasSettingsJsonFile() == false)
		{
			return FText::FromString("settings.json not found in export directory, please try exporting the level again. If the issue persists, please try using a different export directory that allows file writing.");
		}
	}
	if (CurrentPageEnum == ESceneSetupPage::UploadChecklist)
	{
		if (FCognitiveEditorTools::GetInstance()->HasSettingsJsonFile() == false)
		{
			return FText::FromString("settings.json not found in export directory, please try exporting the level again. If the issue persists, please try using a different export directory that allows file writing.");
		}
	}
	return FText::FromString("");
}

FReply SSceneSetupWidget::NextPage()
{
	if (CurrentPageEnum == ESceneSetupPage::Intro)
	{
		FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();

		//save keys to ini
		SpawnCognitive3DActor();
	}
	else if (CurrentPageEnum == ESceneSetupPage::Controller)
	{
		//set default export directory if it isnt set
		FCognitiveEditorTools::GetInstance()->SetDefaultIfNoExportDirectory();
	}
	else if (CurrentPageEnum == ESceneSetupPage::Export)
	{
		GLog->Log("set dynamic and scene export directories. create if needed");
		FCognitiveEditorTools::GetInstance()->CreateExportFolderStructure();
		FCognitiveEditorTools::GetInstance()->RefreshSceneUploadFiles();
		FCognitiveEditorTools::GetInstance()->RefreshDynamicUploadFiles();
		TakeScreenshot();
		FCognitiveEditorTools::GetInstance()->SaveScreenshotToFile();
	}
	else if (CurrentPageEnum == ESceneSetupPage::UploadChecklist)
	{
		FCognitiveEditorTools::GetInstance()->OnUploadSceneGeometry.BindSP(this, &SSceneSetupWidget::OnSceneUploaded);

		FCognitiveEditorTools::GetInstance()->WizardUpload();
	}
	else if (CurrentPageEnum == ESceneSetupPage::UploadProgress)
	{

	}

	if (CurrentPageEnum != ESceneSetupPage::Complete)
	{
		CurrentPageEnum = (ESceneSetupPage)(((uint8)CurrentPageEnum) + 1);
	}
	if (CurrentPageEnum == ESceneSetupPage::Complete)
	{
		FCognitive3DEditorModule::CloseSceneSetupWindow();
	}

	return FReply::Handled();
}

FReply SSceneSetupWidget::LastPage()
{
	if (CurrentPageEnum == ESceneSetupPage::Complete) 
	{ 
		return FReply::Handled(); 
	}
	else if (CurrentPageEnum == ESceneSetupPage::Export)
	{
		//set default export directory if it isnt set
		FCognitiveEditorTools::GetInstance()->SetDefaultIfNoExportDirectory();
	}
	CurrentPageEnum = (ESceneSetupPage)(((uint8)CurrentPageEnum) - 1);
	return FReply::Handled();
}

EVisibility SSceneSetupWidget::DisplayWizardThrobber() const
{
	if (CurrentPageEnum == ESceneSetupPage::UploadProgress)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SSceneSetupWidget::NextButtonVisibility() const
{
	if (CurrentPageEnum == ESceneSetupPage::Invalid)
	{
		return EVisibility::Hidden;
	}
	if (CurrentPageEnum == ESceneSetupPage::UploadProgress)
	{
		return EVisibility::Hidden;
	}
	return EVisibility::Visible;
}
FText SSceneSetupWidget::NextButtonText() const
{
	if (CurrentPageEnum == ESceneSetupPage::Export)
	{
		return FText::FromString("Next");
	}
	else if (CurrentPageEnum == ESceneSetupPage::UploadChecklist)
	{
		return FText::FromString("Upload");
	}
	else if (CurrentPageEnum == ESceneSetupPage::Complete)
	{
		return FText::FromString("Close");
	}
	return FText::FromString("Next");
}

bool SSceneSetupWidget::NextButtonEnabled() const
{
	if (CurrentPageEnum == ESceneSetupPage::Intro)
	{
		//disable if no valid dev key
		if (FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if (CurrentPageEnum == ESceneSetupPage::Export)
	{
		//disable if no scene has been exported or if no settings json file generated
		if (FCognitiveEditorTools::GetInstance()->GetSceneExportFileCount() == 0)
		{
			return false;
		}
		else if (FCognitiveEditorTools::GetInstance()->GetSceneExportFileCount() > 0)
		{
			if (FCognitiveEditorTools::GetInstance()->HasSettingsJsonFile() == false)
			{
				FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode = 0;
				FCognitiveEditorTools::GetInstance()->WizardUploadError = "settings.json not found in export directory, please try exporting the level again";
				return false;
			}
		}
		return true;
	}

	if (CurrentPageEnum == ESceneSetupPage::UploadChecklist)
	{
		if (FCognitiveEditorTools::GetInstance()->HasSettingsJsonFile() == false)
		{
			FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode = 0;
			FCognitiveEditorTools::GetInstance()->WizardUploadError = "settings.json not found in export directory, please try exporting the level again";
			return false;
		}
		return true;
	}

	if (CurrentPageEnum == ESceneSetupPage::UploadProgress)
	{
		//refresh the upload filename lists
		FCognitiveEditorTools::GetInstance()->RefreshDynamicUploadFiles();
		FCognitiveEditorTools::GetInstance()->RefreshSceneUploadFiles();

		FString sceneExportDir = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory();
		if (!FCognitiveEditorTools::VerifyDirectoryExists(sceneExportDir))
		{
			return false;
		}

		if (FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode == 200) { return true; }
		if (FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode == 201) { return true; }

		if (FCognitiveEditorTools::GetInstance()->WizardUploadError.Len() > 0)
		{
			return false;
		}
	}

	return true;
}

EVisibility SSceneSetupWidget::BackButtonVisibility() const
{
	if (CurrentPageEnum == ESceneSetupPage::Intro)
	{
		return EVisibility::Hidden;
	}
	if (CurrentPageEnum == ESceneSetupPage::Invalid)
	{
		return EVisibility::Hidden;
	}
	if (CurrentPageEnum == ESceneSetupPage::Complete)
	{
		return EVisibility::Hidden;
	}
	if (CurrentPageEnum == ESceneSetupPage::UploadProgress)
	{
		return EVisibility::Hidden;
	}
	return EVisibility::Visible;
}

int32 SSceneSetupWidget::CountDynamicObjectsInScene() const
{
	//loop thorugh all dynamics in the scene
	TArray<UDynamicObject*> dynamics;

	//get all the dynamic objects in the scene
	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		//AStaticMeshActor *Mesh = *ActorItr;

		UActorComponent* actorComponent = (*ActorItr)->GetComponentByClass(UDynamicObject::StaticClass());
		if (actorComponent == NULL)
		{
			continue;
		}
		UDynamicObject* dynamic = Cast<UDynamicObject>(actorComponent);
		if (dynamic == NULL)
		{
			continue;
		}
		dynamics.Add(dynamic);
	}

	return dynamics.Num();
}

FText SSceneSetupWidget::ExportedSceneText() const
{
	int32 fileCount = FCognitiveEditorTools::GetInstance()->GetSceneExportFileCount();
	if (fileCount == 0)
	{
		return FText::FromString("No files were exported");
	}
	else
	{
		return FText::FromString("Scene has been exported with a total of " + FString::FromInt(fileCount) + " files");
	}
}

EVisibility SSceneSetupWidget::HasExportedSceneTextVisibility() const
{
	if (CurrentPageEnum != ESceneSetupPage::Export)
	{
		return EVisibility::Collapsed;
	}
	if (FCognitiveEditorTools::GetInstance()->GetSceneExportFileCount() == 0)
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return EVisibility::Visible;
	}
}

EVisibility SSceneSetupWidget::UploadErrorVisibility() const
{
	if (FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode == 200) { return EVisibility::Collapsed; }
	if (FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode == 201) { return EVisibility::Collapsed; }
	if (FCognitiveEditorTools::GetInstance()->WizardUploadError.Len() > 0)
	{
		if (FCognitiveEditorTools::GetInstance()->WizardUploadError.Contains("OnUploadObjectCompleted"))
		{
			if (FCognitiveEditorTools::GetInstance()->HasExportedAnyDynamicMeshes())
			{
				return EVisibility::Visible;
			}
			else
			{
				return EVisibility::Collapsed;
			}
		}
		else
		{
			return EVisibility::Visible;
		}
	}
	return EVisibility::Collapsed;
}

FText SSceneSetupWidget::UploadErrorText() const
{
	return FText::FromString(FCognitiveEditorTools::GetInstance()->WizardUploadError);
}

FText SSceneSetupWidget::GetHeaderTitle() const
{
	if (CurrentPageEnum == ESceneSetupPage::Invalid)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>INVALID DEVELOPER KEY</>");
	}
	if (CurrentPageEnum == ESceneSetupPage::Intro)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 1 - INTRO</>");
	}
	if (CurrentPageEnum == ESceneSetupPage::Controller)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 2 - CONTROLLER SETUP</>");
	}
	if (CurrentPageEnum == ESceneSetupPage::Export)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 3 - EXPORT SCENE GEOMETRY</>");
	}
	if (CurrentPageEnum == ESceneSetupPage::UploadChecklist)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 4 - UPLOAD TO DASHBOARD</>");
	}
	if (CurrentPageEnum == ESceneSetupPage::UploadProgress)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 4 - UPLOAD IN PROGRESS</>");
	}
	if (CurrentPageEnum == ESceneSetupPage::Complete)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 5 - DONE</>");
	}
	return FText::FromString("<RichTextBlock.BoldHighlight>STEP 5 - DONE</>");
}

void SSceneSetupWidget::OnExportPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BaseExportDirectory = Text.ToString();
	if (Text.IsEmpty())
	{
		FCognitiveEditorTools::GetInstance()->SetDefaultIfNoExportDirectory();
		FCognitiveEditorTools::GetInstance()->RefreshSceneUploadFiles();
	}
	FCognitiveEditorTools::GetInstance()->RefreshSceneUploadFiles();
}

void SSceneSetupWidget::SpawnCognitive3DActor()
{
	//get the level editor world
	UWorld* levelEditorWorld = nullptr;
	for (int32 j = 0; j < GEditor->GetLevelViewportClients().Num(); j++)
	{
		if (!GEditor->GetLevelViewportClients()[j]->IsLevelEditorClient()) { continue; }
		levelEditorWorld = GEditor->GetLevelViewportClients()[j]->GetWorld();
		break;
	}
	if (levelEditorWorld == nullptr)
	{
		return;
	}

	//check if there's a Cognitive3DActor already in the world
	for (TObjectIterator<ACognitive3DActor> Itr; Itr; ++Itr)
	{
		if (Itr->IsPendingKill())
		{
			//if a ACognitive3DActor was deleted from the world, it sticks around but is pending a kill. possibly in some undo buffer?
			continue;
		}
		UWorld* tempWorld = Itr->GetWorld();
		if (tempWorld == NULL) { continue; }
		if (tempWorld != levelEditorWorld) { continue; }
		return;
	}

	//spawn a Cognitive3DActor blueprint
	UClass* classPtr = LoadObject<UClass>(nullptr, TEXT("/Cognitive3D/BP_Cognitive3DActor.BP_Cognitive3DActor_C"));
	if (classPtr)
	{
		AActor* obj = levelEditorWorld->SpawnActor<AActor>(classPtr);
		obj->OnConstruction(obj->GetTransform());
		obj->PostActorConstruction();
		GLog->Log("SSceneSetupWidget::SpawnCognitive3DActor spawned BP_Cognitive3DActor in world");
	}
	else
	{
		GLog->Log("SSceneSetupWidget::SpawnCognitive3DActor couldn't find BP_Cognitive3DActor class");
	}
}

FReply SSceneSetupWidget::OpenProjectSetupWindow()
{
	FCognitive3DEditorModule::SpawnCognitiveProjectSetupTab();
	return FReply::Handled();
}

const FSlateBrush* SSceneSetupWidget::GetControllerConfigureBrush() const
{
	return ControllerConfigureBrush;
}

const FSlateBrush* SSceneSetupWidget::GetControllerComponentBrush() const
{
	return ControllerComponentBrush;
}

EVisibility SSceneSetupWidget::GetAppendedInputsFoundHidden() const
{
	if (CurrentPageEnum != ESceneSetupPage::Controller)
	{
		return EVisibility::Collapsed;
	}

	FString InputIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultInput.ini"));
	//TODO IMPROVEMENT instead of hard coding strings here, should append a list from the resources folder
	TArray<FString> actionMapping;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	const FString NormalizedInputIni = GConfig->NormalizeConfigIniPath(InputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, NormalizedInputIni);
#else
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, InputIni);
#endif

	if (actionMapping.Contains("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Grip_Click)"))
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

EVisibility SSceneSetupWidget::GetAppendedInputsFoundVisibility() const
{
	if (CurrentPageEnum != ESceneSetupPage::Controller)
	{
		return EVisibility::Collapsed;
	}

	FString InputIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultInput.ini"));
	//TODO IMPROVEMENT instead of hard coding strings here, should append a list from the resources folder
	TArray<FString> actionMapping;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	const FString NormalizedInputIni = GConfig->NormalizeConfigIniPath(InputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, NormalizedInputIni);
#else
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, InputIni);
#endif

	if (actionMapping.Contains("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Grip_Click)"))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

FReply SSceneSetupWidget::AppendInputs()
{
	FString InputIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultInput.ini"));
	
	//TODO IMPROVEMENT instead of hard coding strings here, should append a list from the resources folder

	TArray<FString> actionMapping;
	TArray<FString> axisMapping;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	const FString NormalizedInputIni = GConfig->NormalizeConfigIniPath(InputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, NormalizedInputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+AxisMappings"), axisMapping, NormalizedInputIni);
#else
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, InputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+AxisMappings"), axisMapping, InputIni);
#endif

	if (actionMapping.Contains("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Grip_Click)"))
	{
		GLog->Log("SSceneSetupWidget::AppendInputs already includes Cognitive3D Inputs");
		//already added! don't append again
		return FReply::Handled();
	}

#if defined PICOMOBILE_API
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_TriggerAxis)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_LGrip)");
	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_RGrip)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_TriggerAxis)");
	actionMapping.Add("(ActionName=\"C3D_LeftJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_RockerC)");
	actionMapping.Add("(ActionName=\"C3D_RightJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_RockerC)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_X)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_Y)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_A)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_B)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_Home)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_Home)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickH\",Scale=1.000000,Key=PicoNeoController_L_RockerX)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickV\",Scale=-1.000000,Key=PicoNeoController_L_RockerY)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickH\",Scale=1.000000,Key=PicoNeoController_R_RockerX)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickV\",Scale=-1.000000,Key=PicoNeoController_R_RockerY)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=PicoNeoController_L_TriggerAxis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=PicoNeoController_R_TriggerAxis)");
#endif

	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Grip_Axis)");
	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Grip_Axis)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trackpad_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Trackpad_Force)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Trackpad_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trackpad_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Trackpad_Force)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Trackpad_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trackpad_Up)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_X_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_A_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trackpad_Right)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Y_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_B_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trackpad_Up)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_A_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_A_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trackpad_Right)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_B_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_B_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_System_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_System_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Menu_Click)");
	
	axisMapping.Add("(AxisName=\"C3D_LeftTouchpadH\",Scale=1.000000,Key=MixedReality_Left_Trackpad_X)");
	axisMapping.Add("(AxisName=\"C3D_LeftTouchpadH\",Scale=1.000000,Key=Vive_Left_Trackpad_X)");
	axisMapping.Add("(AxisName=\"C3D_LeftTouchpadV\",Scale=1.000000,Key=MixedReality_Left_Trackpad_Y)");
	axisMapping.Add("(AxisName=\"C3D_LeftTouchpadV\",Scale=1.000000,Key=Vive_Left_Trackpad_Y)");
	axisMapping.Add("(AxisName=\"C3D_RightTouchpadH\",Scale=1.000000,Key=MixedReality_Right_Trackpad_X)");
	axisMapping.Add("(AxisName=\"C3D_RightTouchpadH\",Scale=1.000000,Key=Vive_Right_Trackpad_X)");
	axisMapping.Add("(AxisName=\"C3D_RightTouchpadV\",Scale=1.000000,Key=MixedReality_Right_Trackpad_Y)");
	axisMapping.Add("(AxisName=\"C3D_RightTouchpadV\",Scale=1.000000,Key=Vive_Right_Trackpad_Y)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickH\",Scale=1.000000,Key=OculusTouch_Left_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickH\",Scale=1.000000,Key=ValveIndex_Left_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickH\",Scale=1.000000,Key=MixedReality_Left_Thumbstick_X)");	
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickV\",Scale=1.000000,Key=OculusTouch_Left_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickV\",Scale=1.000000,Key=ValveIndex_Left_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickV\",Scale=1.000000,Key=MixedReality_Left_Thumbstick_Y)");	
	axisMapping.Add("(AxisName=\"C3D_RightJoystickH\",Scale=1.000000,Key=OculusTouch_Right_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickH\",Scale=1.000000,Key=ValveIndex_Right_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickH\",Scale=1.000000,Key=MixedReality_Right_Thumbstick_X)");	
	axisMapping.Add("(AxisName=\"C3D_RightJoystickV\",Scale=1.000000,Key=OculusTouch_Right_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickV\",Scale=1.000000,Key=ValveIndex_Right_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickV\",Scale=1.000000,Key=MixedReality_Right_Thumbstick_Y)");	
	axisMapping.Add("(AxisName=\"C3D_LeftGripAxis\",Scale=1.000000,Key=OculusTouch_Left_Grip_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftGripAxis\",Scale=1.000000,Key=ValveIndex_Left_Grip_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftGripAxis\",Scale=1.000000,Key=MixedReality_Left_Grip_Click)");
	axisMapping.Add("(AxisName=\"C3D_LeftGripAxis\",Scale=1.000000,Key=Vive_Left_Grip_Click)");
	axisMapping.Add("(AxisName=\"C3D_RightGripAxis\",Scale=1.000000,Key=OculusTouch_Right_Grip_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightGripAxis\",Scale=1.000000,Key=ValveIndex_Right_Grip_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightGripAxis\",Scale=1.000000,Key=MixedReality_Right_Grip_Click)");
	axisMapping.Add("(AxisName=\"C3D_RightGripAxis\",Scale=1.000000,Key=Vive_Right_Grip_Click)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=OculusTouch_Left_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=ValveIndex_Left_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=MixedReality_Left_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=Vive_Left_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=OculusTouch_Right_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=ValveIndex_Right_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=MixedReality_Right_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=Vive_Right_Trigger_Axis)");

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	GConfig->SetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, NormalizedInputIni);
	GConfig->SetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+AxisMappings"), axisMapping, NormalizedInputIni);

	GConfig->Flush(false, NormalizedInputIni);
#else
	GConfig->SetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, InputIni);
	GConfig->SetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+AxisMappings"), axisMapping, InputIni);

	GConfig->Flush(false, InputIni);
#endif

	GLog->Log("SSceneSetupWidget::AppendInputs complete");

	return FReply::Handled();
}

EVisibility SSceneSetupWidget::UploadThumbnailTextVisibility() const
{
	if (CurrentPageEnum != ESceneSetupPage::UploadChecklist)
	{
		return EVisibility::Collapsed;
	}

	FString ScreenshotPath = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory() + "/screenshot/screenshot.png";
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*ScreenshotPath))
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}	
}

FText SSceneSetupWidget::GetDynamicObjectCountToUploadText() const
{
	FString dynamicCount = FString::FromInt(FCognitiveEditorTools::GetInstance()->GetDynamicObjectExportedCount());
	return FText::FromString("Upload " + dynamicCount + " Dynamic Object Meshes");
}

FText SSceneSetupWidget::GetSceneVersionToUploadText() const
{
	auto sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	int32 fileCount = FCognitiveEditorTools::GetInstance()->GetSceneExportFileCount();
	if (sceneData.IsValid())
	{
		
		return FText::FromString("Upload " +FString::FromInt(fileCount)+" files for Scene Geometry (Version " + FString::FromInt(sceneData->VersionNumber+1) + ")");
	}
	else
	{
		return FText::FromString("Upload " + FString::FromInt(fileCount) + " files for Scene Geometry (Version 1)");
	}
}