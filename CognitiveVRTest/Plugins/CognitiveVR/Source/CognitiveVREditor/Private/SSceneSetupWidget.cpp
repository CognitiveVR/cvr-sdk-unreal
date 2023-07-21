
#include "SSceneSetupWidget.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

TArray<TSharedPtr<FDynamicData>> SSceneSetupWidget::GetSceneDynamics()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneDynamics();
}
FOptionalSize SSceneSetupWidget::GetScreenshotWidth() const
{
	return FOptionalSize(FMath::Max(ScreenshotWidth, ScreenshotHeight));
}

FOptionalSize SSceneSetupWidget::GetScreenshotHeight() const
{
	return FOptionalSize(FMath::Max(ScreenshotWidth, ScreenshotHeight));
}

void SSceneSetupWidget::CheckForExpiredDeveloperKey()
{
	if (FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		auto Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &SSceneSetupWidget::OnDeveloperKeyResponseReceived);
		FString gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
		FString url = "https://" + gateway + "/v0/apiKeys/verify";
		Request->SetURL(url);
		Request->SetVerb("GET");
		Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + FAnalyticsCognitiveVR::Get().DeveloperKey);
		Request->ProcessRequest();
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
		if (CurrentPageEnum == EPage::Invalid)
		{
			CurrentPageEnum = EPage::Intro;
		}
	}
	else
	{
		CurrentPageEnum = EPage::Invalid;
		//SGenericDialogWidget::OpenDialog(FText::FromString("Your developer key has expired"), SNew(STextBlock).Text(FText::FromString("Please log in to the dashboard, select your project, and generate a new developer key.\n\nNote:\nDeveloper keys allow you to upload and modify Scenes, and the keys expire after 90 days.\nApplication keys authorize your app to send data to our server, and they never expire.")));
		GLog->Log("Developer Key Response Code is not 200. Developer key may be invalid or expired");
	}
}

void SSceneSetupWidget::Construct(const FArguments& Args)
{
	float padding = 10;

	CheckForExpiredDeveloperKey();

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
				//.Padding(0, 0, 0, 40)
			[
				SNew(STextBlock)
				//.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.Text(FText::FromString(""))
			]

#pragma region invalid screen
			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0,0,0, padding)
			[
				SNew(SRichTextBlock)
				.Justification(ETextJustify::Center)
				.Visibility(this, &SSceneSetupWidget::IsInvalidVisible)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("The developer key is invalid. Please set the developer key in the Project Setup Window"))
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.MaxHeight(30)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsInvalidVisible)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Visibility(this,&SSceneSetupWidget::IsInvalidVisible)
					.Text(FText::FromString("open project setup window"))
					.OnClicked(this, &SSceneSetupWidget::OpenProjectSetupWindow)
					//.OnClicked_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OpenURL,FString("https://www.blender.org"))
				]
			]

#pragma endregion

#pragma region "intro screen"
			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0,0,0, padding)
			[
				SNew(SRichTextBlock)
				.Justification(ETextJustify::Center)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("Welcome to the <RichTextBlock.BoldHighlight>Cognitive3D Scene Setup</>"))
			]
			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.Text(FText::FromString("This will guide you through the initial setup of your scene and will have produciton ready analytics at the end of this setup."))
			]

			+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, padding)
				.HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Visibility(this, &SSceneSetupWidget::IsIntroNewVersionVisible)
						.Image(FEditorStyle::GetBrush("SettingsEditor.WarningIcon"))
					]
				]

			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroNewVersionVisible)
				.AutoWrapText(true)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>This scene has already been uploaded to SceneExplorer</>.\n\nUnless there are meaningful changes to the static scene geometry you probably don't need to upload this scene again."))
			]
			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.AutoWrapText(true)
				.Visibility(this, &SSceneSetupWidget::IsIntroNewVersionVisible)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("If you want to upload new Dynamic Objects to your existing scene, see the <RichTextBlock.BoldHighlight>CognitiveVR section in Project Settings</>."))
			]

#pragma endregion

#pragma region "controller screen"

			+ SVerticalBox::Slot()
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("Open the VR Pawn blueprint that is spawned for your player.\n\nAttach Dynamic Object components as children of each MotionController actor component.\n\nPress the Setup Left Controller and Setup Right Controller buttons on the appropriate actors.\n\nSave your changes.\n\nOptionally, you can append inputs to your DefaultInput.ini file. This will allow you to visualize the button presses of the player."))
				//TODO add pictures
			]

			+SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
				[
					SNew(SHorizontalBox)
					.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString("Append Input Data to Input.ini"))
						.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
						//.OnClicked(this, &SSceneSetupWidget::RefreshDisplayDynamicObjectsCountInScene)
					]
					//+SHorizontalBox::Slot()
					//[
					//	SNew(STextBlock)
					//	.Visibility(this, &SSceneSetupWidget::IsControllerVisible)
					//	.AutoWrapText(true)
					//	.Justification(ETextJustify::Center)
					//	.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::DisplayDynamicObjectsCountInScene)
					//]
				]
			]

#pragma endregion

#pragma region "export screen"

			//path to blender
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			.AutoHeight()
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
						.Text(FText::FromString("Path to Blender.exe"))
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
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBlenderPath)
						.OnTextChanged(this, &SSceneSetupWidget::OnBlenderPathChanged)
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

			//path to export directory
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			.AutoHeight()
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
			.VAlign(VAlign_Center)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("The current level will be exported and prepared to be uploaded to SceneExplorer."))
			]
			/*+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				[
					SNew(SCheckBox)
					.Visibility(this, &SSceneSetupWidget::IsExportVisible)
					.IsChecked(this, &SSceneSetupWidget::GetNoExportGameplayMeshCheckbox)
					.OnCheckStateChanged(this, &SSceneSetupWidget::OnChangeNoExportGameplayMesh)
				]
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Visibility(this, &SSceneSetupWidget::IsExportVisible)
					.Text(FText::FromString("Do not export Skybox, Cameras or Dynamics"))
				]
			]*/
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
			.VAlign(VAlign_Center)
				.MaxHeight(40)
				.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(256)
				[
					SNew(SButton)
					.Visibility(this, &SSceneSetupWidget::IsExportVisible)
					.Text(FText::FromString("Select All"))
					.OnClicked(this,&SSceneSetupWidget::SelectAll)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SSeparator)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
			]

			+ SVerticalBox::Slot()
				.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(VAlign_Center)
				.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					.HeightOverride(64)
					.WidthOverride(128)
					[
						SNew(SButton)
						.Visibility(this, &SSceneSetupWidget::IsExportVisible)
						.Text(FText::FromString("Export"))
						.OnClicked(this,&SSceneSetupWidget::EvaluateSceneExport)
					]
				]
			]
#pragma endregion

#pragma region "upload screen"
			+ SVerticalBox::Slot()
			.AutoHeight()
				.Padding(0, 0, 0, padding)
				.VAlign(VAlign_Top)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsNewSceneUpload)
				.AutoWrapText(true)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>Upload New Scene</> including a screenshot, all the Scene Files listed below and all Dynamic Mesh Files listed below."))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
				.Padding(0, 0, 0, padding)
				.VAlign(VAlign_Top)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsSceneVersionUpload)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>Upload New Version of this Scene</> This will archive the previous version of this scene."))
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
				.HeightOverride(32)
				.WidthOverride(256)
				.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
				[
					SNew(SButton)
					.Text(FText::FromString("Take Screenshot From Current View"))
					.Visibility(this, &SSceneSetupWidget::IsUploadChecklistVisible)
					.OnClicked(this, &SSceneSetupWidget::TakeScreenshot)
				]
			]


			//TODO checkboxes for stuff to upload
			//TODO number of dynamic objects
			//TODO number of scene files

#pragma endregion

#pragma region "done screen"
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
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

			+SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
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
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::UploadErrorVisibility)
				.AutoWrapText(true)
				.ColorAndOpacity(FLinearColor::Red)
				.Justification(ETextJustify::Center)
				.Text(this,&SSceneSetupWidget::UploadErrorText)
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsCompleteVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("That's it!\n\nAfter saving your project, you will be recording user position, gaze and basic device information. Simply press play in the Unreal Editor or make a build for your target platform.\n\nPlease note that sessions run in the editor won't count towards aggregate metrics.\n\nYou can view sessions from the Dashboard."))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0, 5, 0, 5)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Visibility(this, &SSceneSetupWidget::IsCompleteVisible)
					.Text(FText::FromString("Open Dashboard"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenURL, "https://" + FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Dashboard", false))
				]
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(64)
				.Visibility(this, &SSceneSetupWidget::IsCompleteVisible)
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsCompleteVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("You can continue your integration to get more insights including:\n\nCustom Events\n\nExitPoll surveys\n\nDynamic Objects\n\nMultiplayer"))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0,5,0,5)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Visibility(this, &SSceneSetupWidget::IsCompleteVisible)
					.Text(FText::FromString("Open Documentation"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenURL, FString("https://docs.cognitive3d.com/unreal/get-started/"))
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
						.Text(this, &SSceneSetupWidget::NextButtonText)
						.IsEnabled(this,&SSceneSetupWidget::NextButtonEnabled)
						.Visibility(this, &SSceneSetupWidget::NextButtonVisibility)
						.OnClicked(this, &SSceneSetupWidget::NextPage)
					]
				]
			]
			//DEBUG NEXT/BACK BUTTONS
			+ SOverlay::Slot()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.Text(FText::FromString("Debug Back"))
						.OnClicked(this, &SSceneSetupWidget::DebugPreviousPage)
					]
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.Text(FText::FromString("Debug Next"))
						.OnClicked(this, &SSceneSetupWidget::DebugNextPage)
					]
				]
			]

#pragma endregion
		];

		FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
		FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
		FCognitiveEditorTools::GetInstance()->WizardUploadError = "";
		FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode = 0;
}

FReply SSceneSetupWidget::EvaluateSceneExport()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorToolsCustomization::Select_Export_Meshes world is null"));
		return FReply::Handled();
	}


	//should put this all in a CognitiveEditorTools export function
	//take array of actors to be exported
	TArray<AActor*> ToBeExported;
	if (OnlyExportSelected) //only export selected
	{
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			if (AActor* Actor = Cast<AActor>(*It))
			{
				ToBeExported.Add(Actor);
			}
		}
	}
	else //select all
	{
		UWorld* World = GEditor->GetLevelViewportClients()[0]->GetWorld();
		GEditor->Exec(World, TEXT("actor select all"));
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			if (AActor* Actor = Cast<AActor>(*It))
			{
				ULevel* level = Actor->GetLevel();
				if (level->bIsVisible == 0) { continue; } //sublevel probably. invisible
				ToBeExported.Add(Actor);
			}
		}
	}

	TArray<AActor*> ToBeExportedFinal;
	for (int32 i = 0; i < ToBeExported.Num(); i++)
	{
		if (ToBeExported[i]->GetName().StartsWith("SkySphereBlueprint"))
		{
			continue;
		}
		UActorComponent* cameraComponent = ToBeExported[i]->GetComponentByClass(UCameraComponent::StaticClass());
		if (cameraComponent != NULL)
		{
			continue;
		}

		UActorComponent* actorComponent = ToBeExported[i]->GetComponentByClass(UDynamicObject::StaticClass());
		if (actorComponent != NULL)
		{
			continue;
		}
		ToBeExportedFinal.Add(ToBeExported[i]);
	}
	FCognitiveEditorTools::GetInstance()->ExportScene(ToBeExportedFinal);

	SceneWasExported = true;

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

void SSceneSetupWidget::GetScreenshotBrush()
{
	FString ScreenshotPath = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory() + "/screenshot/screenshot.png";
	FName BrushName = FName(*ScreenshotPath);

	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *ScreenshotPath)) return;

	if (ScreenshotTexture != nullptr)
	{
		delete ScreenshotTexture;
	}

	auto imageWrapper = FCognitiveEditorTools::GetInstance()->ImageWrapper;

	if (imageWrapper.IsValid() && imageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		ScreenshotWidth = imageWrapper->GetWidth();
		ScreenshotHeight = imageWrapper->GetHeight();

		if (ScreenshotHeight > ScreenshotWidth)
		{
			ScreenshotWidth = (int32)(((float)ScreenshotWidth / (float)ScreenshotHeight) * 256);
			ScreenshotHeight = 256;
		}
		else if (ScreenshotHeight < ScreenshotWidth)
		{
			ScreenshotHeight = (int32)(((float)ScreenshotHeight / (float)ScreenshotWidth) * 256);
			ScreenshotWidth = 256;
		}
		else
		{
			ScreenshotHeight = 256;
			ScreenshotWidth = 256;
		}
		//ScreenshotImage->SetImage(new FSlateDynamicImageBrush(BrushName, FVector2D(ScreenshotWidth, ScreenshotHeight)));
		ScreenshotTexture = new FSlateDynamicImageBrush(BrushName, FVector2D(ScreenshotWidth, ScreenshotHeight));
	}
	else
	{
		GLog->Log("image wrap screenshot failed!");
	}
}

const FSlateBrush* SSceneSetupWidget::GetScreenshotBrushTexture() const
{
	return ScreenshotTexture;
}

EVisibility SSceneSetupWidget::IsSceneVersionUpload() const
{
	if (CurrentPageEnum != EPage::UploadChecklist) { return EVisibility::Collapsed; }
	TSharedPtr<FEditorSceneData> sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SSceneSetupWidget::IsIntroNewVersionVisible() const
{
	if (CurrentPageEnum != EPage::Intro) { return EVisibility::Collapsed; }

	TSharedPtr<FEditorSceneData> sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SSceneSetupWidget::IsNewSceneUpload() const
{
	if (CurrentPageEnum != EPage::UploadChecklist) { return EVisibility::Collapsed; }
	
	TSharedPtr<FEditorSceneData> sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

EVisibility SSceneSetupWidget::IsInvalidVisible() const
{
	return CurrentPageEnum == EPage::Invalid ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsIntroVisible() const
{
	return CurrentPageEnum == EPage::Intro ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsControllerVisible() const
{
	return CurrentPageEnum == EPage::Controller ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsExportVisible() const
{
	return CurrentPageEnum == EPage::Export ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsUploadProgressVisible() const
{
	return CurrentPageEnum == EPage::UploadChecklist ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsUploadChecklistVisible() const
{
	return CurrentPageEnum == EPage::UploadChecklist ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsCompleteVisible() const
{
	return CurrentPageEnum == EPage::Complete ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SSceneSetupWidget::TakeScreenshot()
{
	FCognitiveEditorTools::GetInstance()->TakeScreenshot();
	GetScreenshotBrush();
	return FReply::Handled();
}

FReply SSceneSetupWidget::DebugNextPage()
{
	CurrentPageEnum = (EPage)(((uint8)CurrentPageEnum) + 1);
	return FReply::Handled();
}
FReply SSceneSetupWidget::DebugPreviousPage()
{
	CurrentPageEnum = (EPage)(((uint8)CurrentPageEnum) - 1);
	return FReply::Handled();
}

FReply SSceneSetupWidget::NextPage()
{
	if (CurrentPageEnum == EPage::Intro)
	{
		FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();

		//save keys to ini
		SpawnCognitiveVRActor();
	}
	else if (CurrentPageEnum == EPage::Export)
	{
		GLog->Log("set dynamic and scene export directories. create if needed");
		FCognitiveEditorTools::GetInstance()->CreateExportFolderStructure();
		FCognitiveEditorTools::GetInstance()->RefreshSceneUploadFiles();
		FCognitiveEditorTools::GetInstance()->RefreshDynamicUploadFiles();

		if (FCognitiveEditorTools::GetInstance()->GetSceneExportFileCount() == 0)
		{
			SGenericDialogWidget::OpenDialog(FText::FromString("No exported files found"),
				SNew(STextBlock).Text(FText::FromString("No exported files were found. Are you sure you want to skip this step?")));
		}

		GetScreenshotBrush();
	}
	else if (CurrentPageEnum == EPage::UploadChecklist)
	{
		FCognitiveEditorTools::GetInstance()->WizardUpload();
	}
	else if (CurrentPageEnum == EPage::UploadProgress)
	{

	}

	if (CurrentPageEnum != EPage::Complete)
	{
		CurrentPageEnum = (EPage)(((uint8)CurrentPageEnum) + 1);
	}

	return FReply::Handled();
}

FReply SSceneSetupWidget::LastPage()
{
	if (CurrentPageEnum == EPage::Complete) { return FReply::Handled(); }
	CurrentPageEnum = (EPage)(((uint8)CurrentPageEnum) - 1);
	return FReply::Handled();
}

EVisibility SSceneSetupWidget::DisplayWizardThrobber() const
{
	if (CurrentPageEnum != EPage::UploadProgress)
	{
		return EVisibility::Collapsed;
	}

	if (FCognitiveEditorTools::GetInstance()->IsWizardUploading())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SSceneSetupWidget::NextButtonVisibility() const
{
	if (CurrentPageEnum == EPage::Invalid)
	{
		return EVisibility::Hidden;
	}
	if (CurrentPageEnum == EPage::Complete)
	{
		return EVisibility::Hidden;
	}
	return EVisibility::Visible;
}
FText SSceneSetupWidget::NextButtonText() const
{
	if (CurrentPageEnum == EPage::Export)
	{
		if (FCognitiveEditorTools::GetInstance()->SubDirectoryNames.Num() > 0)
		{
			return FText::FromString("Next");
		}
		else
		{
			//TODO this shows even after the scene has been exported
			return FText::FromString("Skip");
		}		
	}
	else if (CurrentPageEnum == EPage::UploadChecklist)
	{
		return FText::FromString("Upload");
	}
	return FText::FromString("Next");
}

bool SSceneSetupWidget::NextButtonEnabled() const
{
	if (CurrentPageEnum == EPage::Intro)
	{
		return true;
	}

	if (CurrentPageEnum == EPage::Export)
	{
		if (FCognitiveEditorTools::GetInstance()->HasFoundBlender() && FCognitiveEditorTools::GetInstance()->BaseExportDirectory.Len() > 0)
		{
			return true;
		}
		return false;
	}

	if (CurrentPageEnum == EPage::Export)
	{
		FCognitiveEditorTools::GetInstance()->FindAllSubDirectoryNames();
	}

	if (CurrentPageEnum == EPage::Export)
	{
		FString sceneExportDir = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory();

		return FCognitiveEditorTools::VerifyDirectoryExists(sceneExportDir);
	}

	if (CurrentPageEnum == EPage::UploadProgress)
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
	if (CurrentPageEnum == EPage::Intro)
	{
		return EVisibility::Hidden;
	}
	if (CurrentPageEnum == EPage::Invalid)
	{
		return EVisibility::Hidden;
	}
	if (CurrentPageEnum == EPage::Complete)
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

FText SSceneSetupWidget::DisplayDynamicObjectsCountInScene() const
{
	return DynamicCountInScene;
}

EVisibility SSceneSetupWidget::UploadErrorVisibility() const
{
	if (FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode == 200) { return EVisibility::Collapsed; }
	if (FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode == 201) { return EVisibility::Collapsed; }
	return FCognitiveEditorTools::GetInstance()->WizardUploadError.Len() == 0 ? EVisibility::Collapsed : EVisibility::Visible;
}

FText SSceneSetupWidget::UploadErrorText() const
{
	return FText::FromString(FCognitiveEditorTools::GetInstance()->WizardUploadError);
}

FText SSceneSetupWidget::GetHeaderTitle() const
{
	if (CurrentPageEnum == EPage::Invalid)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>INVALID DEVELOPER KEY</>");
	}
	if (CurrentPageEnum == EPage::Intro)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 1 - INTRO</>");
	}
	if (CurrentPageEnum == EPage::Controller)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 2 - CONTROLLER SETUP</>");
	}
	if (CurrentPageEnum == EPage::Export)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 3 - EXPORT SCENE GEOMETRY</>");
	}
	if (CurrentPageEnum == EPage::UploadChecklist)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 4 - UPLOAD</>");
	}
	if (CurrentPageEnum == EPage::UploadProgress)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 4 - UPLOAD IN PROGRESS</>");
	}
	if (CurrentPageEnum == EPage::Complete)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 5 - DONE</>");
	}
	return FText::FromString("<RichTextBlock.BoldHighlight>STEP 5 - DONE</>");
}

void SSceneSetupWidget::OnExportPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BaseExportDirectory = Text.ToString();
}

void SSceneSetupWidget::OnBlenderPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BlenderPath = Text.ToString();
}

void SSceneSetupWidget::SpawnCognitiveVRActor()
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

	//check if there's a CognitiveVRActor already in the world
	for (TObjectIterator<ACognitiveVRActor> Itr; Itr; ++Itr)
	{
		if (Itr->IsPendingKill())
		{
			//if a ACognitiveVRActor was deleted from the world, it sticks around but is pending a kill. possibly in some undo buffer?
			continue;
		}
		UWorld* tempWorld = Itr->GetWorld();
		if (tempWorld == NULL) { continue; }
		if (tempWorld != levelEditorWorld) { continue; }
		return;
	}

	//spawn a CognitiveVRActor blueprint
	UClass* classPtr = LoadObject<UClass>(nullptr, TEXT("/CognitiveVR/BP_CognitiveVRActor.BP_CognitiveVRActor_C"));
	if (classPtr)
	{
		AActor* obj = levelEditorWorld->SpawnActor<AActor>(classPtr);
		obj->OnConstruction(obj->GetTransform());
		obj->PostActorConstruction();
		GLog->Log("SSceneSetupWidget::SpawnCognitiveVRActor spawned BP_CognitiveVRActor in world");
	}
	else
	{
		GLog->Log("SSceneSetupWidget::SpawnCognitiveVRActor couldn't find BP_CognitiveVRActor class");
	}
}

FReply SSceneSetupWidget::OpenProjectSetupWindow()
{
	FCognitiveVREditorModule::SpawnCognitiveProjectSetupTab();
	return FReply::Handled();
}