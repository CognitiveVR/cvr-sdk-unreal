#include "CognitiveVREditorPrivatePCH.h"
#include "SSceneSetupWidget.h"

void SSceneSetupWidget::Construct(const FArguments& Args)
{
	ChildSlot
		[
			SNew(SVerticalBox)
#pragma region "intro screen"
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.Text(FText::FromString("Welcome to Cognitive3D Scene Setup"))
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsIntroVisible)
				.Text(FText::FromString("This Setup wizard will walk you through exporting and uploading your scene"))
			]
#pragma endregion

#pragma region "key screen"

			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
					.Text(FText::FromString("copy your DEVELOPER and API keys here"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.MaxWidth(96)
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
						.Text(FText::FromString("Developer Key"))
					]

					+ SHorizontalBox::Slot()
					.MaxWidth(128)
					[
						SNew(SEditableTextBox)
						.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
						.MinDesiredWidth(128)
						.Text(this, &SSceneSetupWidget::GetDeveloperKey)
						.OnTextChanged(this,&SSceneSetupWidget::OnDeveloperKeyChanged)
					]
				]
			+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.MaxWidth(96)
					[
						SNew(STextBlock)
						.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
						.Text(FText::FromString("API Key"))
					]

					+ SHorizontalBox::Slot()
					.MaxWidth(128)
					[
						SNew(SEditableTextBox)
						.Visibility(this, &SSceneSetupWidget::IsKeysVisible)
						.MinDesiredWidth(128)
						.Text(this, &SSceneSetupWidget::GetAPIKey)
						.OnTextChanged(this,&SSceneSetupWidget::OnAPIKeyChanged)
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
					.Text(FText::FromString("please add some dynamic object components onto your actors"))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
					.Text(FText::FromString("TODO put a list of dynamic objects here"))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.Visibility(this, &SSceneSetupWidget::IsDynamicsVisible)
					.Text(FText::FromString("when you're done, press the next button to export your dynamic objects"))
				]

#pragma endregion

#pragma region "export screen"
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(SButton)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				.Text(FText::FromString("Select Export Meshes"))
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(SButton)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				.Text(FText::FromString("Export Scene from Selection"))
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(SButton)
				.Visibility(this, &SSceneSetupWidget::IsExportVisible)
				.Text(FText::FromString("Export Everything"))
			]
#pragma endregion

#pragma region "upload screen"
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsUploadVisible)
				.Text(FText::FromString("upload everything?"))
			]
#pragma endregion

#pragma region "upload screen"
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(STextBlock)
				.Visibility(this, &SSceneSetupWidget::IsCompleteVisible)
				.Text(FText::FromString("you're done!"))
			]
#pragma endregion

#pragma region "footer"

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Text(FText::FromString("Back"))
					.OnClicked(this, &SSceneSetupWidget::LastPage)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Text(FText::FromString("Next"))
					.OnClicked(this, &SSceneSetupWidget::NextPage)
				]
			]

#pragma endregion
		];
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
	APIKey = Text.ToString();
	FString EngineIni = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), APIKey, EngineIni);
}

FText SSceneSetupWidget::GetAPIKey() const
{
	return FText::FromString(APIKey);
}

FText SSceneSetupWidget::GetDeveloperKey() const
{
	return FText::FromString(FAnalyticsCognitiveVR::Get().DeveloperKey);
}

void SSceneSetupWidget::OnDeveloperKeyChanged(const FText& Text)
{
	FAnalyticsCognitiveVR::Get().DeveloperKey = Text.ToString();
	FString EditorIni = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEditor.ini"));
	GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FAnalyticsCognitiveVR::Get().DeveloperKey, EditorIni);
}

void SSceneSetupWidget::RefreshList()
{
	ListViewWidget->RequestListRefresh();
}

FReply SSceneSetupWidget::NextPage()
{
	if (CurrentPage == 3)
	{
		GLog->Log("do export");
	}
	else if (CurrentPage == 4)
	{
		GLog->Log("upload scene, then dynamics, then manifest");
	}
	else if (CurrentPage == 5)
	{
		GLog->Log("complete scene setup, close window");
	}

	if (CurrentPage != 5)
	{
		CurrentPage++;
	}
	GLog->Log("pressed next button " + FString::FromInt(CurrentPage));
	return FReply::Handled();
}

FReply SSceneSetupWidget::LastPage()
{
	if (CurrentPage == 0) { return FReply::Handled(); }
	CurrentPage--;
	GLog->Log("pressed last button " + FString::FromInt(CurrentPage));
	return FReply::Handled();
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