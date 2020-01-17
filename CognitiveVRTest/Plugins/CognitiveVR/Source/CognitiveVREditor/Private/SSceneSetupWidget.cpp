//#include "CognitiveVREditorPrivatePCH.h"
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

void SSceneSetupWidget::Construct(const FArguments& Args)
{
	DisplayAPIKey = FCognitiveEditorTools::GetInstance()->GetAPIKey().ToString();
	DisplayDeveloperKey = FCognitiveEditorTools::GetInstance()->GetDeveloperKey().ToString();

	float padding = 10;

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

#pragma region "key screen"

			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("Please add your <RichTextBlock.BoldHighlight>Cognitive3D Project API Keys</> below to continue\nThese are available on the Project Dashboard"))
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
						.Text(FText::FromString("Developer Key"))
					]
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SEditableTextBox)
						.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
						.Text(this, &SSceneSetupWidget::GetDisplayDeveloperKey)
						.OnTextChanged(this,&SSceneSetupWidget::OnDeveloperKeyChanged)
						//.OnTextCommitted()
					]
				]
			]
			+ SVerticalBox::Slot()
			.MaxHeight(8)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
						.Text(FText::FromString("API Key"))
					]
				]

				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SEditableTextBox)
						.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
						.Text(this, &SSceneSetupWidget::GetDisplayAPIKey)
						.OnTextChanged(this,&SSceneSetupWidget::OnAPIKeyChanged)
					]
				]
			]

#pragma endregion

#pragma region "explain dynamics"

			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Justification(ETextJustify::Center)
				.Visibility(this, &SSceneSetupWidget::IsExplainDynamicsVisible)
				.AutoWrapText(true)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("A <RichTextBlock.BoldHighlight>Dynamic Object</> is an object that moves around during an experience which you wish to track"))
			]

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.AutoHeight()
			.Padding(0,0,0,padding)
			[
				SNew(SBox)
				.Visibility(this, &SSceneSetupWidget::IsExplainDynamicsVisible)
				.HeightOverride(180)
				.WidthOverride(256)				
				[
					SNew(SOverlay)
					+SOverlay::Slot()
					[
						SNew(SBox)
						.WidthOverride(256)
						.HeightOverride(180)
						[
							SNew(SImage)
							.Visibility(this, &SSceneSetupWidget::IsExplainDynamicsVisible)
							.Image(this, &SSceneSetupWidget::GetSceneGreyTexture)
						]
					]
					+ SOverlay::Slot()
					[
						SNew(SBox)
						.WidthOverride(256)
						.HeightOverride(180)
						[
							SNew(SImage)
							.Visibility(this, &SSceneSetupWidget::IsExplainDynamicsVisible)
							.Image(this, &SSceneSetupWidget::GetDynamicsBlueTexture)
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExplainDynamicsVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>You can add or remove Dynamic Objects without uploading a new Scene Version</>.\n\nYou must attach <RichTextBlock.BoldHighlight>Dynamic Object Components</> onto each actor you wish to track in your project. These objects must have collision so we can track user gaze on them."))
			]

#pragma endregion

#pragma region "explain scene"

			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExplainSceneVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("A <RichTextBlock.BoldHighlight>Scene</> is an approximation of your Unreal level and is uploaded to the Dashboard. It is all the static meshes"))
			]

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.Visibility(this, &SSceneSetupWidget::IsExplainSceneVisible)
				.HeightOverride(180)
				.WidthOverride(256)				
				[
					SNew(SOverlay)
					+SOverlay::Slot()
					[
						SNew(SBox)
						.WidthOverride(256)
						.HeightOverride(180)
						[
							SNew(SImage)
							.Visibility(this, &SSceneSetupWidget::IsExplainSceneVisible)
							.Image(this, &SSceneSetupWidget::GetSceneBlueTexture)
						]
					]
					+ SOverlay::Slot()
					[
						SNew(SBox)
						.WidthOverride(256)
						.HeightOverride(180)
						[
							SNew(SImage)
							.Visibility(this, &SSceneSetupWidget::IsExplainSceneVisible)
							.Image(this, &SSceneSetupWidget::GetDynamicsGreyTexture)
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExplainSceneVisible)
				.AutoWrapText(true)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("This will provide context to the data collected in your experience.\n\nIf you decide to change the scene in your Unreal project (such as moving a wall), the data you collect may no longer represent your experience. You can upload a new <RichTextBlock.BoldHighlight>Scene Version</> by running this setup again."))
			]

#pragma endregion

#pragma region "blender"
			+SVerticalBox::Slot()
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.WidthOverride(256)
				.HeightOverride(78)
				.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
				[
					SNew(SImage)
					.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
					.Image(this, &SSceneSetupWidget::GetBlenderLogo)
				]
			]


			+ SVerticalBox::Slot()
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("When uploading your level to the dashboard, we use Blender to automatically prepare the scene.\nThis includes converting exported images to .pngs\nand reducing the polygon count of large meshes.\n\nWe also need a temporary Export Directory to save Unreal files to while we process them."))
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.MaxHeight(30)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Blender is free and open source."))
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.MaxHeight(30)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Visibility(this,&SSceneSetupWidget::IsBlenderVisible)
					.Text(FText::FromString("www.blender.org"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OpenURL,FString("https://www.blender.org"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SSeparator)
				.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
			]

			//path to blender
			+ SVerticalBox::Slot()
			.MaxHeight(17)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
				+SHorizontalBox::Slot()
				.MaxWidth(200)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
					.HeightOverride(17)
					[
						SNew(STextBlock)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.Text(FText::FromString("Path to Blender.exe"))
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(1)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
					.HeightOverride(17)
					.MaxDesiredHeight(17)
					[
						SNew(SEditableTextBox)
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBlenderPath)
						.OnTextChanged(this, &SSceneSetupWidget::OnBlenderPathChanged)
					]
				]
				+SHorizontalBox::Slot()
				.MaxWidth(17)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
					.HeightOverride(17)
					.WidthOverride(17)
					[
						SNew(SButton)
						.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
						//PickerWidget = SAssignNew(BrowseButton, SButton)
						.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
						.ToolTipText(LOCTEXT("FolderButtonToolTipText", "Choose a directory from this computer"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Blender)
						.ContentPadding(2.0f)
						.ForegroundColor(FSlateColor::UseForeground())
						.IsFocusable(false)
						[
							SNew(SImage)
							.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
							.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Ellipsis"))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
				]
			]

			//path to export directory
			+ SVerticalBox::Slot()
			.MaxHeight(17)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
				+SHorizontalBox::Slot()
				.MaxWidth(200)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
					.HeightOverride(17)
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.Text(FText::FromString("Path to Export Directory"))
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(1)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
					.HeightOverride(17)
					.MaxDesiredHeight(17)
					[
						SNew(SEditableTextBox)
						.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBaseExportDirectoryDisplay)
						.OnTextChanged(this, &SSceneSetupWidget::OnExportPathChanged)
						//SNew(STextBlock)
						//
					]
				]
				+SHorizontalBox::Slot()
				.MaxWidth(17)
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
					.HeightOverride(17)
					.WidthOverride(17)
					[
						SNew(SButton)
						.Visibility(this, &SSceneSetupWidget::IsBlenderVisible)
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
#pragma endregion

#pragma region "dynamics screen"

			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				//SNew(STextBlock)
				//.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				//.Justification(ETextJustify::Center)
				//.Text(FText::FromString("These are all the Dynamic Objects components currently found in your scene."))

				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("These are all the Dynamic Objects components currently found in your scene."))

			]

			+SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				[
					SNew(SHorizontalBox)
					.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString("Refresh"))
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.OnClicked(this, &SSceneSetupWidget::RefreshDisplayDynamicObjectsCountInScene)
					]
					+SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::DisplayDynamicObjectsCountInScene)
					]
				]
			]

			/*+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				.Text(FText::FromString("Dynamic Objects In Scene"))
			]*/
			+ SVerticalBox::Slot()
				.AutoHeight()
				.MaxHeight(250)
				.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				[
					SAssignNew(SceneDynamicObjectList,SDynamicObjectListWidget)
					.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
					.Items(GetSceneDynamics())
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(64, 24, 64, 24))
			[
				SNew(SBorder)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.LightGroupBorder"))
				.Padding(8.0f)
				[
					SNew(SHorizontalBox)
					.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
						.Image(FEditorStyle::GetBrush("SettingsEditor.WarningIcon"))
					]
						// Notice
					+SHorizontalBox::Slot()
					.Padding(16.0f, 0.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.ColorAndOpacity(FLinearColor::Black)
						//.ShadowColorAndOpacity(FLinearColor::Black)
						//.ShadowOffset(FVector2D::UnitVector)
						.AutoWrapText(true)
						.Text(FText::FromString("Dynamic Objects must have a valid Mesh Name\nTo have data aggregated, Dynamic Objects must have a Unique Id"))
					]
					+SHorizontalBox::Slot()
					[
						SNew(SButton)
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.OnClicked_Raw(this, &SSceneSetupWidget::ValidateAndRefresh)
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
							.Text(FText::FromString("Validate Mesh Names and Unique Ids"))
						]
					]
				]
			]
			+ SVerticalBox::Slot()
				.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(VAlign_Center)
				.AutoHeight()
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
					.HeightOverride(64)
					.WidthOverride(128)
					[
						SNew(SButton)
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.Text(FText::FromString("Export All Meshes"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::ExportAllDynamics)
					]
				]
			]
			+ SVerticalBox::Slot()
				.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(VAlign_Center)
				.AutoHeight()
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
					.HeightOverride(64)
					.WidthOverride(128)
					[
						SNew(SButton)
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.Text(FText::FromString("Export Selected Meshes"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::ExportSelectedDynamics)
					]
				]
			]


#pragma endregion

#pragma region "export screen"

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

			//+ SVerticalBox::Slot()
			//.Padding(0, 0, 0, 4)
			//.HAlign(EHorizontalAlignment::HAlign_Center)
			//.VAlign(VAlign_Center)
			//[
			//	SNew(SHorizontalBox)
			//	+SHorizontalBox::Slot()
			//	[
			//		SNew(SBox)
			//		.HeightOverride(64)
			//		.WidthOverride(128)
			//		[
			//			SNew(SButton)
			//			.Visibility(this, &SSceneSetupWidget::IsExportVisible)
			//			.Text(FText::FromString("Export Scene Materials"))
			//			.OnClicked(this,&SSceneSetupWidget::ExportSceneMaterials)
			//		]
			//	]
			//]

			//+ SVerticalBox::Slot()
			//.Padding(0, 0, 0, 4)
			//.HAlign(EHorizontalAlignment::HAlign_Center)
			//.VAlign(VAlign_Center)
			//[
			//	SNew(SHorizontalBox)
			//	+SHorizontalBox::Slot()
			//	[
			//		SNew(SBox)
			//		.HeightOverride(64)
			//		.WidthOverride(128)
			//		[
			//			SNew(SButton)
			//			.Visibility(this, &SSceneSetupWidget::IsExportVisible)
			//			.Text(FText::FromString("ConvertToGLTF"))
			//			.OnClicked(this,&SSceneSetupWidget::ConvertSceneToGLTF)
			//		]
			//	]
			//]


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
				.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
				[
					SNew(SImage)
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
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
				.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
				[
					SNew(SButton)
					.Text(FText::FromString("Take Screenshot From Current View"))
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
					.OnClicked(this, &SSceneSetupWidget::TakeScreenshot)
				]
			]

			+ SVerticalBox::Slot()
			.MaxHeight(250)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SListView<TSharedPtr<FString>>)
					.ItemHeight(16.0f)
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
					.ListItemsSource(&FCognitiveEditorTools::GetInstance()->SceneUploadFiles)
					.OnGenerateRow(this, &SSceneSetupWidget::OnGenerateSceneExportFileRow)
					.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("name")
						[
							SNew(SRichTextBlock)
							.Justification(ETextJustify::Center)
							.DecoratorStyleSet(&FEditorStyle::Get())
							.Text(FText::FromString("<RichTextBlock.BoldHighlight>Scene Files</>"))
						]
					)
			]
			+ SVerticalBox::Slot()
				.AutoHeight()
			.MaxHeight(250)
				.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
				[
					SNew(SListView<TSharedPtr<FString>>)
					.ItemHeight(16.0f)
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
					.ListItemsSource(&FCognitiveEditorTools::GetInstance()->DynamicUploadFiles)
					.OnGenerateRow(this, &SSceneSetupWidget::OnGenerateSceneExportFileRow)
					.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("name")
						[
							SNew(SRichTextBlock)
							.Justification(ETextJustify::Center)
							.DecoratorStyleSet(&FEditorStyle::Get())
							.Text(FText::FromString("<RichTextBlock.BoldHighlight>Dynamic Mesh Files</>"))
						]
					)
				]
			]

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
				SNew(SRichTextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("Add a <RichTextBlock.BoldHighlight>PlayerTracker Component</> to your Player Actor and add the following to your Level Blueprint:"))
			]

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(SBox)
				.WidthOverride(321)
				.HeightOverride(128)
				.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
				[
					SNew(SImage)
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
					.Image(this, &SSceneSetupWidget::GetBlueprintStartTexture)
				]
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("That's it!\n\nYou will be recording user position, gaze and basic device information.\n\nYou can view sessions from the Dashboard"))
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
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
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
				.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Want to ask users about their experience?\n\nNeed to add more Dynamic Objects?\n\nHave some Sensors?\n\nMultiplayer?"))
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
					.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
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
			//			//.Visibility(this,&SSceneSetupWidget::BackButtonVisibility)
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
			//			//.IsEnabled(this,&SSceneSetupWidget::NextButtonEnabled)
			//			//.Visibility(this, &SSceneSetupWidget::NextButtonVisibility)
			//			.OnClicked(this, &SSceneSetupWidget::DebugNextPage)
			//		]
			//	]
			//]

#pragma endregion
		];

		FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
		FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
		FCognitiveEditorTools::GetInstance()->SearchForBlender();
		FCognitiveEditorTools::GetInstance()->WizardUploadError = "";

		FString texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("objects_grey.png");
		FName BrushName = FName(*texturepath);
		DynamicsGreyTexture = new FSlateDynamicImageBrush(BrushName, FVector2D(256, 180));
		
		texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("scene_blue.png");
		BrushName = FName(*texturepath);
		SceneBlueTexture = new FSlateDynamicImageBrush(BrushName, FVector2D(256, 180));

		texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("objects_blue.png");
		BrushName = FName(*texturepath);
		DynamicsBlueTexture = new FSlateDynamicImageBrush(BrushName, FVector2D(256, 180));

		texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("scene_grey.png");
		BrushName = FName(*texturepath);
		SceneGreyTexture = new FSlateDynamicImageBrush(BrushName, FVector2D(256, 180));

		texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("bp_startsession.png");
		BrushName = FName(*texturepath);
		BlueprintStartTexture = new FSlateDynamicImageBrush(BrushName, FVector2D(256, 180));

		texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("blender_logo_socket_small.png");
		BrushName = FName(*texturepath);
		BlenderLogoTexture = new FSlateDynamicImageBrush(BrushName, FVector2D(256, 78));
}

FReply SSceneSetupWidget::EvaluateSceneExport()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorToolsCustomization::Select_Export_Meshes world is null"));
		return FReply::Handled();
	}

	TArray<AActor*> ToBeExported;
	TArray<AActor*> ToBeExportedFinal;

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
		UWorld* World = GEditor->LevelViewportClients[0]->GetWorld();
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


	if (NoExportGameplayMeshes)
	{
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
	}
	else
	{
		//export everything, it's fine
		for (int32 i = 0; i < ToBeExported.Num(); i++)
		{
			ToBeExportedFinal.Add(ToBeExported[i]);
		}
	}

	
	GEditor->SelectNone(false, true, false);
	for (int32 i = 0; i < ToBeExportedFinal.Num(); i++)
	{
		GEditor->SelectActor((ToBeExportedFinal[i]), true, false, true);
	}
	FString ExportedSceneFile = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory() + "/" + FCognitiveEditorTools::GetInstance()->GetCurrentSceneName() + ".obj";

	GEditor->ExportMap(tempworld, *ExportedSceneFile, true);


	//always export all bsp geometry brushes
	//export geometry as fbx
	GEditor->SelectNone(false, true, false);

	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		ABrush* obj = Cast<ABrush>((*ActorItr));
		AVolume* vol = Cast<AVolume>((*ActorItr));

		if (obj == nullptr) { continue; } //skip non-brushes
		if (vol != nullptr) { continue; } //skip volumes

		GEditor->SelectActor((*ActorItr), true, true, true, true);
	}

	FString ExportedSceneFile2 = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory() + "/" + FCognitiveEditorTools::GetInstance()->GetCurrentSceneName() + ".fbx";

	GEditor->ExportMap(tempworld, *ExportedSceneFile2, true);

	FCognitiveEditorTools::GetInstance()->WizardPostSceneExport();
	SceneWasExported = true;


	//Export Scene Materials
	TArray< UStaticMeshComponent*> sceneMeshes;
	//iterate over all scene static meshes
	for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
	{
		//
		UStaticMeshComponent* TempObject = *It;
		if (TempObject == NULL) { continue; }

		if (TempObject->GetOwner() == NULL) { continue; }

		UActorComponent* dynamic = TempObject->GetOwner()->GetComponentByClass(UDynamicObject::StaticClass());
		if (dynamic != NULL) { continue; }

		sceneMeshes.Add(TempObject);
	}

	//FString sceneDirectory = BaseExportDirectory + "/" + GetCurrentSceneName() + "/";
	FString sceneDirectory = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory()+"/";

	FCognitiveEditorTools::GetInstance()->WizardExportMaterials(sceneDirectory, sceneMeshes, FCognitiveEditorTools::GetInstance()->GetCurrentSceneName());



	//Convert scene to GLTF
	FCognitiveEditorTools::GetInstance()->WizardConvertScene();
	SceneWasExported = true;

	return FReply::Handled();
}

FReply SSceneSetupWidget::SelectAll()
{
	UWorld* World = GEditor->LevelViewportClients[0]->GetWorld();
	GEditor->Exec(World, TEXT("actor select all"));
	return FReply::Handled();
}

ECheckBoxState SSceneSetupWidget::GetNoExportGameplayMeshCheckbox() const
{
	if (NoExportGameplayMeshes)return ECheckBoxState::Checked;
	return ECheckBoxState::Unchecked;
}

ECheckBoxState SSceneSetupWidget::GetOnlyExportSelectedCheckbox() const
{
	if (OnlyExportSelected)return ECheckBoxState::Checked;
	return ECheckBoxState::Unchecked;
}

void SSceneSetupWidget::GetScreenshotBrush()
{
	FString ScreenshotPath = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory() + "/screenshot/screenshot.png";

	//FString ScreenshotPath = FPaths::Combine(FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory(), "screenshot", "screenshot.png");
	FName BrushName = FName(*ScreenshotPath);

	FCognitiveVREditorModule& c3dmod = FModuleManager::GetModuleChecked< FCognitiveVREditorModule >("CognitiveVREditor");

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

const FSlateBrush* SSceneSetupWidget::GetBlenderLogo() const
{
	return BlenderLogoTexture;
}

const FSlateBrush* SSceneSetupWidget::GetDynamicsGreyTexture() const
{
	return DynamicsGreyTexture;
}
const FSlateBrush* SSceneSetupWidget::GetSceneBlueTexture() const
{
	return SceneBlueTexture;
}
const FSlateBrush* SSceneSetupWidget::GetDynamicsBlueTexture() const
{
	return DynamicsBlueTexture;
}
const FSlateBrush* SSceneSetupWidget::GetSceneGreyTexture() const
{
	return SceneGreyTexture;
}

const FSlateBrush* SSceneSetupWidget::GetScreenshotBrushTexture() const
{
	return ScreenshotTexture;
}
const FSlateBrush* SSceneSetupWidget::GetBlueprintStartTexture() const
{
	return BlueprintStartTexture;
}

FText SSceneSetupWidget::GetDisplayAPIKey() const
{
	return FText::FromString(DisplayAPIKey);
}

FText SSceneSetupWidget::GetDisplayDeveloperKey() const
{
	return FText::FromString(DisplayDeveloperKey);
}

EVisibility SSceneSetupWidget::IsSceneVersionUpload() const
{
	if (CurrentPage != 7) { return EVisibility::Collapsed; }
	

	TSharedPtr<FEditorSceneData> sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SSceneSetupWidget::IsIntroNewVersionVisible() const
{
	if (CurrentPage != 0) { return EVisibility::Collapsed; }


	TSharedPtr<FEditorSceneData> sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SSceneSetupWidget::IsNewSceneUpload() const
{
	if (CurrentPage != 7) { return EVisibility::Collapsed; }
	
	TSharedPtr<FEditorSceneData> sceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

EVisibility SSceneSetupWidget::IsIntroVisible() const
{
	return 0 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsKeysVisible() const
{
	return 1 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsBlenderVisible() const
{
	return 2 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsExplainDynamicsVisible() const
{
	return 4 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsExplainSceneVisible() const
{
	return 3 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsDynamicsVisible() const
{
	return 5 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsExportVisible() const
{
	return 6 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsUploadVisible() const
{
	return 7 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsCompleteVisible() const
{
	return 8 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsUploadComplete() const
{
	if (FCognitiveEditorTools::GetInstance()->IsWizardUploading())
	{
		return EVisibility::Collapsed;
	}
	if (FCognitiveEditorTools::GetInstance()->WizardUploadError.Len() > 0)
	{
		return EVisibility::Collapsed;
	}
	return IsCompleteVisible();
}

void SSceneSetupWidget::OnAPIKeyChanged(const FText& Text)
{
	DisplayAPIKey = Text.ToString();
}

void SSceneSetupWidget::OnDeveloperKeyChanged(const FText& Text)
{
	DisplayDeveloperKey = Text.ToString();
}

void SSceneSetupWidget::RefreshList()
{
	ListViewWidget->RequestListRefresh();
}

FReply SSceneSetupWidget::TakeScreenshot()
{
	FCognitiveEditorTools::GetInstance()->TakeScreenshot();
	GetScreenshotBrush();
	return FReply::Handled();
}

FReply SSceneSetupWidget::DebugNextPage()
{
	CurrentPage++;
	return FReply::Handled();
}
FReply SSceneSetupWidget::DebugPreviousPage()
{
	CurrentPage--;
	return FReply::Handled();
}

FReply SSceneSetupWidget::NextPage()
{
	if (CurrentPage == 1)
	{
		FCognitiveEditorTools::GetInstance()->SaveAPIKeyToFile(DisplayAPIKey);
		FCognitiveEditorTools::GetInstance()->SaveDeveloperKeyToFile(DisplayDeveloperKey);
		FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();
	}
	if (CurrentPage == 2)
	{
		GLog->Log("set dynamic and scene export directories. create if needed");
		FCognitiveEditorTools::GetInstance()->CreateExportFolderStructure();
	}
	else if (CurrentPage == 6)
	{
		FCognitiveEditorTools::GetInstance()->RefreshSceneUploadFiles();
		FCognitiveEditorTools::GetInstance()->RefreshDynamicUploadFiles();
		GetScreenshotBrush();
	}
	else if (CurrentPage == 7)
	{
		FCognitiveEditorTools::GetInstance()->WizardUpload();
	}
	else if (CurrentPage == 8)
	{

	}

	if (CurrentPage != 8)
	{
		CurrentPage++;
	}

	return FReply::Handled();
}

FReply SSceneSetupWidget::LastPage()
{
	if (CurrentPage == 0) { return FReply::Handled(); }
	CurrentPage--;
	return FReply::Handled();
}

EVisibility SSceneSetupWidget::DisplayWizardThrobber() const
{
	if (FCognitiveEditorTools::GetInstance()->IsWizardUploading() && 8 == CurrentPage)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

TSharedRef<ITableRow> SSceneSetupWidget::OnGenerateSceneExportFileRow(TSharedPtr<FString>InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<FString> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(*InItem))
			]
		];
}

TSharedRef<ITableRow> SSceneSetupWidget::OnGenerateRowForList(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<FDynamicData> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.MaxWidth(16)
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SBox)
				.HeightOverride(16)
				.HeightOverride(16)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateSP(this, &SSceneSetupWidget::SelectDynamic, InItem))
				]
			]
			+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->Name))
		]
	+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->MeshName))
		]
	+ SHorizontalBox::Slot()
		.FillWidth(0.3)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->Id))
		]
		];
}

FReply SSceneSetupWidget::SelectDynamic(TSharedPtr<FDynamicData> data)
{
	GEditor->SelectNone(false, true, false);

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
		if (dynamic->GetOwner()->GetName() != data->Name) { continue; }
		if (dynamic->CustomId != data->Id) { continue; }
		if (dynamic->MeshName != data->MeshName) { continue; }

		GEditor->SelectActor((*ActorItr), true, true, true, true);

		break;
	}

	return FReply::Handled();
}

EVisibility SSceneSetupWidget::NextButtonVisibility() const
{
	if (CurrentPage == 8)
	{
		return EVisibility::Hidden;
	}
	return EVisibility::Visible;
}
FText SSceneSetupWidget::NextButtonText() const
{
	if (CurrentPage == 1)
	{
		return FText::FromString("Save");
	}
	else if (CurrentPage == 5)
	{
		if (FCognitiveEditorTools::GetInstance()->SubDirectoryNames.Num() > 0)
		{
			return FText::FromString("Next");
		}
		else
		{
			return FText::FromString("Skip");
		}		
	}
	else if (CurrentPage == 6)
	{
		return FText::FromString("Next");
	}
	else if (CurrentPage == 7)
	{
		return FText::FromString("Upload");
	}
	return FText::FromString("Next");
}

//at the moment, AR scene export is not supported in unreal
//TODO ar scene needs a settings.json file, but there isn't a clear place to write this without user input
EVisibility SSceneSetupWidget::ARButtonVisibility() const
{
	return EVisibility::Collapsed;

	if (CurrentPage == 6)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

FReply SSceneSetupWidget::ARSkipExport()
{
	//set a working directory somewhere. write a settings.json file
	//{ "scale":100,"sceneName":"SpeakerScene","sdkVersion":"0.7.4"}
	CurrentPage++;

	return FReply::Handled();
}

bool SSceneSetupWidget::NextButtonEnabled() const
{
	if (CurrentPage == 1)
	{
		if (DisplayDeveloperKey.Len() > 0 && DisplayAPIKey.Len() > 0)
			return true;
		return false;
	}

	if (CurrentPage == 2)
	{
		if (FCognitiveEditorTools::GetInstance()->HasFoundBlender() && FCognitiveEditorTools::GetInstance()->BaseExportDirectory.Len() > 0)
		{
			return true;
		}
		return false;
	}

	if (CurrentPage == 5)
	{
		FCognitiveEditorTools::GetInstance()->FindAllSubDirectoryNames();
	}

	if (CurrentPage == 6)
	{
		FString sceneExportDir = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory();

		return FCognitiveEditorTools::VerifyDirectoryExists(sceneExportDir);
	}

	if (CurrentPage == 7)
	{
		//refresh the upload filename lists
		FCognitiveEditorTools::GetInstance()->RefreshDynamicUploadFiles();
		FCognitiveEditorTools::GetInstance()->RefreshSceneUploadFiles();
	}

	return true;
}

EVisibility SSceneSetupWidget::BackButtonVisibility() const
{
	if (CurrentPage == 0)
	{
		return EVisibility::Hidden;
	}
	if (CurrentPage == 8)
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

FReply SSceneSetupWidget::RefreshDisplayDynamicObjectsCountInScene()
{
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	SceneDynamicObjectList->RefreshList();

	return FReply::Handled();
}

EVisibility SSceneSetupWidget::GetDuplicateDyanmicObjectVisibility() const
{
	if (IsDynamicsVisible() != EVisibility::Visible)
	{
		return EVisibility::Collapsed;
	}
	return FCognitiveEditorTools::GetInstance()->GetDuplicateDyanmicObjectVisibility();
}

EVisibility SSceneSetupWidget::UploadErrorVisibility() const
{
	return FCognitiveEditorTools::GetInstance()->WizardUploadError.Len() == 0 ? EVisibility::Collapsed : EVisibility::Visible;
}

FText SSceneSetupWidget::UploadErrorText() const
{
	return FText::FromString(FCognitiveEditorTools::GetInstance()->WizardUploadError);
}

FReply SSceneSetupWidget::ValidateAndRefresh()
{
	FCognitiveEditorTools::GetInstance()->SetUniqueDynamicIds();
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	SceneDynamicObjectList->RefreshList();
	return FReply::Handled();
}

FText SSceneSetupWidget::GetHeaderTitle() const
{
	if (CurrentPage == 0)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 1 - WELCOME</>");
	}
	if (CurrentPage == 1)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 2 - AUTHENTICATION</>"); //api keys
	}
	if (CurrentPage == 2)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 3 - BLENDER</>"); //set up blender
	}
	if (CurrentPage == 4)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 4a - WHAT IS A DYNAMIC OBJECT?</>"); //dynamic explainer
	}
	if (CurrentPage == 3)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 4a - WHAT IS A SCENE?</>"); //scene explainer
	}
	if (CurrentPage == 5)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 6 - PREPARE DYNAMIC OBJECTS</>"); //dynamic object list + export
	}
	if (CurrentPage == 6)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 7 - PREPARE SCENE</>"); //scene export
	}
	if (CurrentPage == 7)
	{
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 8 - UPLOAD</>"); //scene upload
	}
	return FText::FromString("<RichTextBlock.BoldHighlight>STEP 9 - DONE</>"); //final page 
}

void SSceneSetupWidget::OnBlenderPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BlenderPath = Text.ToString();
}

void SSceneSetupWidget::OnExportPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BaseExportDirectory = Text.ToString();
}
