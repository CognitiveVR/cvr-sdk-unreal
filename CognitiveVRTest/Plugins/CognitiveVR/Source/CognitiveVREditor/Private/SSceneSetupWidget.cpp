#include "CognitiveVREditorPrivatePCH.h"
#include "SSceneSetupWidget.h"

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

void SSceneSetupWidget::Construct(const FArguments& Args)
{
	DisplayAPIKey = FCognitiveEditorTools::GetInstance()->GetAPIKey().ToString();
	DisplayDeveloperKey = FCognitiveEditorTools::GetInstance()->GetDeveloperKey().ToString();

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
#pragma region "intro screen"
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Welcome to Cognitive3D Scene Setup"))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.Text(FText::FromString("This will guide you through the initial setup of your scene and will have production ready analytics at the end of this setup."))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroNewVersionVisible)
				.AutoWrapText(true)
				.ColorAndOpacity(FLinearColor::Yellow)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("This scene has already been uploaded to SceneExplorer.\n\nUnless there are meaningful changes to the static scene geometry you probably don't need to upload this scene again."))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Visibility(this, &SSceneSetupWidget::IsIntroNewVersionVisible)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Use the options in the Cognitive3D section of Project Settings if you want to upload new Dynamic Objects to your existing scene."))
			]

#pragma endregion

#pragma region "key screen"

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Please add your Cognitive3D authorization keys below to continue"))
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
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
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
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
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.Visibility(this, &SSceneSetupWidget::IsExplainDynamicsVisible)
				.AutoWrapText(true)
				.Text(FText::FromString("A Dynamic Object is an object that moves around during a scene which you wish to track"))
			]

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.AutoHeight()
			.Padding(0,0,0,80)
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
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExplainDynamicsVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("You must attach Dynamic Object Components onto any actors you wish to track in your scene. These objects must also have colliders attached to them so we can track user gaze on them."))
			]

#pragma endregion

#pragma region "explain scene"

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExplainSceneVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("A Scene is the base geometry of your level. A scene does not require colliders on it to detect user gaze."))
			]

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.AutoHeight()
			.Padding(0, 0, 0, 80)
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
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExplainSceneVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("The Scene will be uploaded in one large step, and can be updated at a later date, resulting in a new Scene Version."))
			]

#pragma endregion

#pragma region "dynamics screen"

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Blender is used to convert exported images to .png"))
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsEnabled(false)
					.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
					.IsChecked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlenderCheckbox)
				]
				+SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
					.Text(FText::FromString("Select Blender.exe"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Blender)
				]
			]
			+ SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				.Text_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::GetBlenderPath)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 24.0f, 0.0f, 24.0f))
			[
				SNew(SSeparator)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("These are the Dynamic Object Components currently found in your scene"))
			]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SButton)
						.Text(FText::FromString("Refresh"))
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.OnClicked(this, &SSceneSetupWidget::RefreshDisplayDynamicObjectsCountInScene)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 4)
					.HAlign(EHorizontalAlignment::HAlign_Center)
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.Text(FText::FromString("Dynamic Objects In Scene"))
					]
					+ SVerticalBox::Slot()
					.FillHeight(0.5)
					.VAlign(VAlign_Fill)
					[
						SNew(SBox)
						.HeightOverride(300)
						[
							SAssignNew(SceneDynamicObjectList,SDynamicObjectListWidget)
							.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
							.Items(GetSceneDynamics())
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.Text(this,&SSceneSetupWidget::DisplayDynamicObjectsCountInScene)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::Red)
						.Visibility(this,&SSceneSetupWidget::GetDuplicateDyanmicObjectVisibility)
						.Text(FText::FromString("Scene contains some duplicate Dynamic Object Ids"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SButton)
						.Visibility(this, &SSceneSetupWidget::GetDuplicateDyanmicObjectVisibility)
						.Text(FText::FromString("Set Unique Dynamic Ids"))
						.OnClicked(this,&SSceneSetupWidget::SetUniqueDynamicIds)
					]
				]

#pragma endregion

#pragma region "export screen"

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("All geometry without Dynamic Object Components will be uploaded to Scene Explorer. This will give you spatial context when you record player sessions"))
			]

			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 4)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::Yellow)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				.Text(FText::FromString("Important - Export as \"*.obj\"!"))
			]

			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				+SHorizontalBox::Slot() //select export meshes
				.Padding(10, 0, 10, 0)
				.FillWidth(0.5)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(0, 0, 0, 4)
					[
						SNew(SBox)
						.HeightOverride(64)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							.Justification(ETextJustify::Center)
							.Text(FText::FromString("Choose which meshes will be exported for your scene. This is the recommended option. This should skip any skyboxes, cameras and CSG"))
						]
					]
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Top)
					[
						SNew(SBox)
						.Visibility(this, &SSceneSetupWidget::IsExportVisible)
						.HeightOverride(32)
						[
							SNew(SButton)
							.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlender)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							.Text(FText::FromString("Select Export Meshes"))
							.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Export_Meshes)
						]
					]

					+ SVerticalBox::Slot()
					[
						SNew(SBox)
						.Visibility(this, &SSceneSetupWidget::IsExportVisible)
						.HeightOverride(32)
						[
							SNew(SButton)
							.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlenderAndHasSelection)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							.Text(FText::FromString("Export Selected Scene Actors"))
							.OnClicked(this, &SSceneSetupWidget::Export_Selected)
						]
					]
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(24.0f, 0.0f, 24.0f, 0.0f))
				[
					SNew(SSeparator)
					.Orientation(Orient_Vertical)
					.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				]
				+SHorizontalBox::Slot() //export all
				.Padding(10,0,10,0)
				.FillWidth(0.5)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(0, 0, 0, 4)
					[
						SNew(SBox)
						.Visibility(this, &SSceneSetupWidget::IsExportVisible)
						.HeightOverride(64)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							.Justification(ETextJustify::Center)
							.Text(FText::FromString("This will export every mesh from your scene, including Dynamic Objects, CSG and cameras"))
						]
					]
					+ SVerticalBox::Slot()
					[
						SNew(SBox)
						.Visibility(this, &SSceneSetupWidget::IsExportVisible)
						.HeightOverride(32)
						[
							SNew(SButton)
							.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlender)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							.Text(FText::FromString("Export All Scene Actors"))
							.OnClicked(this, &SSceneSetupWidget::Export_All)
							//.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Export_All)
						]
					]
				]
			]


#pragma endregion

#pragma region "upload screen"
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
				.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsNewSceneUpload)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Upload New Scene. After this scene is uploaded, you can still upload Dynamic Objects from the Cognitive3D section in Project Settings"))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
				.AutoHeight()
				.Padding(0,0,0,10)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsSceneVersionUpload)
				.ColorAndOpacity(FLinearColor::Yellow)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Upload New Version of this Scene. This will archive the previous version of this scene. If you only want to upload new Dynamic Objects, see the Cognitive3D section in Project Settings"))
			]

			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoHeight()
				.Padding(0, 0, 0, 10)
			[
				SNew(SBox)
				.WidthOverride(this,&SSceneSetupWidget::GetScreenshotWidth)
				.HeightOverride(this, &SSceneSetupWidget::GetScreenshotHeight)
				.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
				[
					//SAssignNew(ScreenshotImage, SImage)
					SNew(SImage)
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
					.Image(this,&SSceneSetupWidget::GetScreenshotBrushTexture)
				]
			]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
			[
				SNew(SBox)
				.HeightOverride(32)
				.WidthOverride(64)
				.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
				[
					SNew(SButton)
					.Text(FText::FromString("Take Screenshot"))
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
					.OnClicked(this, &SSceneSetupWidget::TakeScreenshot)
				]
			]

			+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.MaxHeight(200)
				.AutoHeight()
				.Padding(0, 0, 0, 10)
			[
				SNew(SListView<TSharedPtr<FString>>)
					.ItemHeight(16.0f)
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
					.ListItemsSource(&FCognitiveEditorTools::GetInstance()->AllUploadFiles)
					.OnGenerateRow(this, &SSceneSetupWidget::OnGenerateSceneExportFileRow)
					.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("name")
						.FillWidth(1)
						[
							SNew(STextBlock)
							.Text(FText::FromString("Scene Files"))
						]
					)
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.AutoHeight()
				.MaxHeight(200)
			.Padding(0, 0, 0, 10)
			[
				SNew(SBox)
				.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
				//.HeightOverride(400)
				[
				SNew(SListView<TSharedPtr<FString>>)
					.ItemHeight(16.0f)
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
					.ListItemsSource(&FCognitiveEditorTools::GetInstance()->AllDynamicFiles)
					.OnGenerateRow(this, &SSceneSetupWidget::OnGenerateSceneExportFileRow)
					.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("name")
						.FillWidth(1)
						[
							SNew(STextBlock)
							.Text(FText::FromString("Dynamic Mesh Files"))
						]
					)
				]
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("You can add ExitPoll Surveys, update Dynamic Objects and add user engagement scripts after this process is complete"))
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
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::UploadErrorVisibility)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("There was an error while uploading. Check the Output Log for details"))
			]

			+SVerticalBox::Slot()
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
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadComplete)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Just add a Player Tracker Component to your player character actor and that's it!\n\n\n\nYou will be recording user position, gaze and basic device information.\n\nYou can view sessions from the Dashboard"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(200)
			]

			]
#pragma endregion

#pragma region "footer"

			+ SOverlay::Slot()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				/*+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(SBox)
					.WidthOverride(512)
					.HeightOverride(32)
					[
						SNew(STextBlock)
						.Text_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::GetDynamicsExportDirectory)
					]
				]*/
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
					.Visibility(this,&SSceneSetupWidget::ARButtonVisibility)
					[
						SNew(SButton)
						.Text(FText::FromString("AR Scene. Skip"))
						.OnClicked(this, &SSceneSetupWidget::ARSkipExport)
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

#pragma endregion
		];

		FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();
		FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
		FCognitiveEditorTools::GetInstance()->SearchForBlender();

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
}

void SSceneSetupWidget::GetScreenshotBrush()
{
	FString ScreenshotPath = FPaths::Combine(*(FCognitiveEditorTools::GetInstance()->BaseExportDirectory), TEXT("screenshot"), TEXT("screenshot.png"));
	FName BrushName = FName(*ScreenshotPath);

	FCognitiveVREditorModule& c3dmod = FModuleManager::GetModuleChecked< FCognitiveVREditorModule >("CognitiveVREditor");

	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *ScreenshotPath)) return;

	if (ScreenshotTexture != nullptr)
	{
		delete ScreenshotTexture;
	}

	if (c3dmod.ImageWrapper.IsValid() && c3dmod.ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		ScreenshotWidth = c3dmod.ImageWrapper->GetWidth();
		ScreenshotHeight = c3dmod.ImageWrapper->GetHeight();

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

FReply SSceneSetupWidget::Export_Selected()
{
	FCognitiveEditorTools::GetInstance()->WizardExport(false);
	SceneWasExported = true;

	return FReply::Handled();
}

FText SSceneSetupWidget::GetDisplayAPIKey() const
{
	return FText::FromString(DisplayAPIKey);
}

FText SSceneSetupWidget::GetDisplayDeveloperKey() const
{
	return FText::FromString(DisplayDeveloperKey);
}

FReply SSceneSetupWidget::Export_All()
{
	FCognitiveEditorTools::GetInstance()->WizardExport(true);
	SceneWasExported = true;

	return FReply::Handled();
}

EVisibility SSceneSetupWidget::IsSceneVersionUpload() const
{
	if (CurrentPage != 6) { return EVisibility::Collapsed; }
	

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
	if (CurrentPage != 6) { return EVisibility::Collapsed; }
	
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
EVisibility SSceneSetupWidget::IsExplainDynamicsVisible() const
{
	return 2 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsExplainSceneVisible() const
{
	return 3 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsDynamicsVisible() const
{
	return 4 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsExportVisible() const
{
	return 5 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsUploadVisible() const
{
	return 6 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsCompleteVisible() const
{
	return 7 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
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

FReply SSceneSetupWidget::NextPage()
{
	if (CurrentPage == 1)
	{
		FCognitiveEditorTools::GetInstance()->SaveAPIKeyToFile(DisplayAPIKey);
		FCognitiveEditorTools::GetInstance()->SaveDeveloperKeyToFile(DisplayDeveloperKey);
	}
	if (CurrentPage == 4)
	{
		FCognitiveEditorTools::GetInstance()->ExportDynamics();
	}
	else if (CurrentPage == 5)
	{
		FCognitiveEditorTools::GetInstance()->RefreshAllUploadFiles();
		GetScreenshotBrush();
	}
	else if (CurrentPage == 6)
	{
		FCognitiveEditorTools::GetInstance()->WizardUpload();
	}
	else if (CurrentPage == 7)
	{

	}

	if (CurrentPage != 7)
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
	if (FCognitiveEditorTools::GetInstance()->IsWizardUploading() && 7 == CurrentPage)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

/*TArray<TSharedPtr<FString>> SSceneSetupWidget::GetAllUploadFiles() const
{
	return FCognitiveEditorTools::GetInstance()->GetAllUploadFiles();
}*/

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
	if (CurrentPage == 7)
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
	else if (CurrentPage == 4)
	{
		return FText::FromString("Export Dynamics");
	}
	else if (CurrentPage == 5)
	{
		return FText::FromString("Next");
	}
	else if (CurrentPage == 6)
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

	if (CurrentPage == 5)
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
	if (CurrentPage == 5)
	{
		return SceneWasExported;
	}

	return true;
}

EVisibility SSceneSetupWidget::BackButtonVisibility() const
{
	if (CurrentPage == 0)
	{
		return EVisibility::Hidden;
	}
	if (CurrentPage == 7)
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

//this should probably be in cognitiveeditortools
FReply SSceneSetupWidget::SetUniqueDynamicIds()
{
	FCognitiveEditorTools::GetInstance()->SetUniqueDynamicIds();
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	SceneDynamicObjectList->RefreshList();
	return FReply::Handled();
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