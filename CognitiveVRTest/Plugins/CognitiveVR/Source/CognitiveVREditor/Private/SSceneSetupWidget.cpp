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
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.Text(FText::FromString("Welcome to Cognitive3D Scene Setup"))
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.Text(FText::FromString("This Setup wizard will walk you through exporting and uploading your scene"))
			]
#pragma endregion

#pragma region "key screen"

			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
					.Text(FText::FromString("copy your DEVELOPER and API keys here"))
				]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
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
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
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

#pragma region "dynamics screen"

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
				.Text(FText::FromString("please add some dynamic object components onto your actors. Blender is used to convert exported images to .png"))
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
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SVerticalBox)
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
						.HeightOverride(500)
						[
							SAssignNew(SceneDynamicObjectList,SDynamicObjectListWidget)
							.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
							.Items(GetSceneDynamics())
						]
					]
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
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.Text(this,&SSceneSetupWidget::DisplayDynamicObjectsCountInScene)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::Yellow)
						.Visibility(this,&SSceneSetupWidget::GetDuplicateDyanmicObjectVisibility)
						.Text(FText::FromString("Scene contains some duplicate Dynamic Object Ids"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SButton)
						.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
						.Text(FText::FromString("Set Unique Dynamic Ids"))
						.OnClicked(this, &SSceneSetupWidget::SetUniqueDynamicIds)
					]
				]

#pragma endregion

#pragma region "export screen"

			+ SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				.Text(FText::FromString("Export your scene geometry so it can be uploaded to Scene Explorer. This will give you spatial context when you record player sessions"))
			]

			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 4)
			.HAlign(EHorizontalAlignment::HAlign_Center)
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
				+SHorizontalBox::Slot() //select export meshes
				.Padding(10, 0, 10, 0)
				.FillWidth(0.5)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(0, 0, 0, 4)
					.HAlign(EHorizontalAlignment::HAlign_Center)
					[
						SNew(SBox)
						.HeightOverride(64)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							.Text(FText::FromString("Choose which meshes will be exported for your scene. This is the recommended option. This should skip any skyboxes, cameras and CSG"))
						]
					]
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Top)
					[
						SNew(SBox)
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
					.HAlign(EHorizontalAlignment::HAlign_Center)
					[
						SNew(SBox)
						.HeightOverride(64)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Visibility(this, &SSceneSetupWidget::IsExportVisible)
							.Text(FText::FromString("This will export every mesh from your scene, including CSG and cameras"))
						]
					]
					+ SVerticalBox::Slot()
					[
						SNew(SBox)
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
			/*+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
				.Text(FText::FromName(BrushName))
			]*/

			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(this,&SSceneSetupWidget::GetScreenshotWidth)
				.HeightOverride(this, &SSceneSetupWidget::GetScreenshotHeight)
				[
					//SAssignNew(ScreenshotImage, SImage)
					SNew(SImage)
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
					.Image(this,&SSceneSetupWidget::GetScreenshotBrushTexture)
				]
			]

			+ SVerticalBox::Slot()
				.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(32)
				.WidthOverride(64)
				[
					SNew(SButton)
					.Text(FText::FromString("Take Screenshot"))
					.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
					.OnClicked(this, &SSceneSetupWidget::TakeScreenshot)
				]
			]

			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
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

#pragma endregion

#pragma region "done screen"
			+ SVerticalBox::Slot()
			[
				SNew(SThrobber)
				.Visibility(this,&SSceneSetupWidget::DisplayWizardThrobber)
			]

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsCompleteVisible)
				.Text(FText::FromString("add a player tracker component to your player actor"))
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

		FCognitiveEditorTools::GetInstance()->RefreshSceneData();
		FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
		FCognitiveEditorTools::GetInstance()->SearchForBlender();
}

void SSceneSetupWidget::GetScreenshotBrush()
{
	FString ScreenshotPath = FPaths::Combine(*(FCognitiveEditorTools::GetInstance()->ExportDirectory), TEXT("screenshot"), TEXT("screenshot.png"));
	FName BrushName = FName(*ScreenshotPath);

	GLog->Log(ScreenshotPath);

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

EVisibility SSceneSetupWidget::IsKeysVisible() const
{
	return 1 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsDynamicsVisible() const
{
	return 2 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsIntroVisible() const
{
	return 0 == CurrentPage?EVisibility::Visible:EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsExportVisible() const
{
	return 3 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsUploadVisible() const
{
	return 4 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SSceneSetupWidget::IsCompleteVisible() const
{
	return 5 == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
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
	if (CurrentPage == 2)
	{
		//FCognitiveVREditorModule& tools = FModuleManager::GetModuleChecked< FCognitiveVREditorModule >("CognitiveVREditor");// ->CognitiveEditorTools->ExportDynamics();
		//FCognitiveTools::GetInstance().ExportDynamics();
		//FCognitiveTools::GetInstance().Get().ExportDynamics();
		FCognitiveEditorTools::GetInstance()->ExportDynamics();
		//tools.CognitiveEditorTools->ExportDynamics();
		GLog->Log("export dynamic objects");
		//CognitiveTools ExportDynamics
	}
	else if (CurrentPage == 3)
	{
		FCognitiveEditorTools::GetInstance()->RefreshAllUploadFiles();
		GetScreenshotBrush();
	}
	else if (CurrentPage == 4)
	{
		GLog->Log("upload scene, then dynamics, then manifest");
		FCognitiveEditorTools::GetInstance()->WizardUpload();
	}
	else if (CurrentPage == 5)
	{
		GLog->Log("complete scene setup, close window");
	}

	if (CurrentPage != 5)
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
	if (FCognitiveEditorTools::GetInstance()->IsWizardUploading() && 5 == CurrentPage)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Hidden;
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
	if (CurrentPage == 5)
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
	else if (CurrentPage == 2)
	{
		return FText::FromString("Export Dynamics");
	}
	else if (CurrentPage == 3)
	{
		return FText::FromString("Next");
	}
	else if (CurrentPage == 4)
	{
		return FText::FromString("Upload");
	}
	return FText::FromString("Next");
}

//at the moment, AR scene export is not supported in unreal
//ar scene needs a settings.json file, but there isn't a clear place to write this without user input
EVisibility SSceneSetupWidget::ARButtonVisibility() const
{
	return EVisibility::Collapsed;

	if (CurrentPage == 3)
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
	if (CurrentPage == 3)
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
	if (CurrentPage == 5)
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
	//loop thorugh all dynamics in the scene
	TArray<UDynamicObject*> dynamics;

	//make a list of all the used objectids

	TArray<FDynamicObjectId> usedIds;

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

	//create objectids for each dynamic that's already set
	for (auto& dynamic : dynamics)
	{
		FString finalMeshName = dynamic->MeshName;
		if (!dynamic->UseCustomMeshName)
		{
			if (dynamic->CommonMeshName == ECommonMeshName::ViveController) { finalMeshName = "ViveController"; }
			if (dynamic->CommonMeshName == ECommonMeshName::ViveTracker) { finalMeshName = "ViveTracker"; }
			if (dynamic->CommonMeshName == ECommonMeshName::OculusTouchRight) { finalMeshName = "OculusTouchRight"; }
			if (dynamic->CommonMeshName == ECommonMeshName::OculusTouchLeft) { finalMeshName = "OculusTouchLeft"; }
		}
	}

	int32 currentUniqueId = 1;
	int32 changedDynamics = 0;

	//unassigned or invalid numbers
	TArray<UDynamicObject*> UnassignedDynamics;

	//try to put all ids back where they were
	for (auto& dynamic : dynamics)
	{
		//id dynamic custom id is not in usedids - add it

		FString findId = dynamic->CustomId;

		FDynamicObjectId* FoundId = usedIds.FindByPredicate([findId](const FDynamicObjectId& InItem)
		{
			return InItem.Id == findId;
		});

		if (FoundId == NULL && dynamic->CustomId != "")
		{
			usedIds.Add(FDynamicObjectId(dynamic->CustomId, dynamic->MeshName));
		}
		else
		{
			//assign a new and unused id
			UnassignedDynamics.Add(dynamic);
		}
	}

	for (auto& dynamic : UnassignedDynamics)
	{
		for (currentUniqueId; currentUniqueId < 1000; currentUniqueId++)
		{
			//find some unused id number
			FDynamicObjectId* FoundId = usedIds.FindByPredicate([currentUniqueId](const FDynamicObjectId& InItem)
			{
				return InItem.Id == FString::FromInt(currentUniqueId);
			});

			if (FoundId == NULL)
			{
				dynamic->CustomId = FString::FromInt(currentUniqueId);
				dynamic->UseCustomId = true;
				changedDynamics++;
				currentUniqueId++;
				usedIds.Add(FDynamicObjectId(dynamic->CustomId, dynamic->MeshName));
				break;
			}
		}
	}

	GLog->Log("CognitiveVR Tools set " + FString::FromInt(changedDynamics) + " dynamic ids");

	GWorld->MarkPackageDirty();
	//save the scene? mark the scene as changed?

	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();

	//SceneDynamicObjectList->RefreshList();

	return FReply::Handled();
}

FReply SSceneSetupWidget::RefreshDisplayDynamicObjectsCountInScene()
{
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	SceneDynamicObjectList->RefreshList();

	return FReply::Handled();
}

bool SSceneSetupWidget::DuplicateDynamicIdsInScene() const
{
	//loop thorugh all dynamics in the scene
	TArray<UDynamicObject*> dynamics;

	//make a list of all the used objectids
	TArray<FDynamicObjectId> usedIds;

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

	int32 currentUniqueId = 1;
	int32 changedDynamics = 0;

	//unassigned or invalid numbers
	TArray<UDynamicObject*> UnassignedDynamics;

	//try to put all ids back where they were
	for (auto& dynamic : dynamics)
	{
		//id dynamic custom id is not in usedids - add it

		FString findId = dynamic->CustomId;

		FDynamicObjectId* FoundId = usedIds.FindByPredicate([findId](const FDynamicObjectId& InItem)
		{
			return InItem.Id == findId;
		});

		if (FoundId == NULL && dynamic->CustomId != "")
		{
			usedIds.Add(FDynamicObjectId(dynamic->CustomId, dynamic->MeshName));
		}
		else
		{
			//assign a new and unused id
			UnassignedDynamics.Add(dynamic);
			break;
		}
	}

	if (UnassignedDynamics.Num() > 0)
	{
		return true;
	}
	return false;
}

EVisibility SSceneSetupWidget::GetDuplicateDyanmicObjectVisibility() const
{
	if (IsDynamicsVisible() != EVisibility::Visible)
	{
		return EVisibility::Collapsed;
	}
	return DuplicateDyanmicObjectVisibility;
}