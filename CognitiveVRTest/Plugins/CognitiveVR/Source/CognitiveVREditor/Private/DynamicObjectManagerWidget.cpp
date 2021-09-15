
#include "DynamicObjectManagerWidget.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

TArray<TSharedPtr<FDynamicData>> SDynamicObjectManagerWidget::GetSceneDynamics()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneDynamics();
}

void SDynamicObjectManagerWidget::CheckForExpiredDeveloperKey()
{
	if (FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		auto Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &SDynamicObjectManagerWidget::OnDeveloperKeyResponseReceived);
		FString gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
		FString url = "https://" + gateway + "/v0/apiKeys/verify";
		Request->SetURL(url);
		Request->SetVerb("GET");
		Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + FAnalyticsCognitiveVR::Get().DeveloperKey);
		Request->ProcessRequest();
	}
}

void SDynamicObjectManagerWidget::OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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
	}
	else
	{
		SGenericDialogWidget::OpenDialog(FText::FromString("Your developer key has expired"), SNew(STextBlock).Text(FText::FromString("Please log in to the dashboard, select your project, and generate a new developer key.\n\nNote:\nDeveloper keys allow you to upload and modify Scenes, and the keys expire after 90 days.\nApplication keys authorize your app to send data to our server, and they never expire.")));
		GLog->Log("Developer Key Response Code is not 200. Developer key may be invalid or expired");
	}
}

void SDynamicObjectManagerWidget::Construct(const FArguments& Args)
{
	DisplayDeveloperKey = FCognitiveEditorTools::GetInstance()->GetDeveloperKey().ToString();

	float padding = 10;

	CheckForExpiredDeveloperKey();

	ChildSlot
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
//warning if the scene has not been exported! + no scene id!
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
					//.Visibility(this, &SDynamicObjectManagerWidget::IsUploadInvalid)
					//.HeightOverride(64)
					//.WidthOverride(128)
					[
						SNew(STextBlock)
						//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
						.IsEnabled(this, &SDynamicObjectManagerWidget::IsUploadInvalid)
						.Text(this, &SDynamicObjectManagerWidget::GetUploadInvalidCause)
					]
				]
			]


#pragma region "dynamics screen"

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
				[
					SNew(SHorizontalBox)
					//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString("Refresh"))
						//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
						.OnClicked(this, &SDynamicObjectManagerWidget::RefreshDisplayDynamicObjectsCountInScene)
					]
					+SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::DisplayDynamicObjectsCountInScene)
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(250)
			.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
				[
					SAssignNew(SceneDynamicObjectList,SDynamicObjectListWidget)
					//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
					.Items(GetSceneDynamics())
				]
			]

			//upload buttons
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
					.HeightOverride(64)
					.WidthOverride(128)
					[
						SNew(SButton)
						//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
						.IsEnabled(this, &SDynamicObjectManagerWidget::IsUploadSelectedEnabled)
						//.Text(FText::FromString("Upload Selected Meshes")) //data selected in the list
						.Text_Raw(this, &SDynamicObjectManagerWidget::UploadSelectedText)
						//.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::ExportSelectedDynamics)
						.OnClicked_Raw(this, &SDynamicObjectManagerWidget::UploadSelectedDynamicData)
					]
				]
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
					.HeightOverride(64)
					.WidthOverride(128)
					[
						SNew(SButton)
						//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
						.IsEnabled(this, &SDynamicObjectManagerWidget::IsUploadAllEnabled)
						.Text(FText::FromString("Upload All Meshes"))
						//.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::UploadDynamics)
						.OnClicked_Raw(this, &SDynamicObjectManagerWidget::ExportAndUploadDynamics)
					]
				]
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(64, 24, 64, 24))
			[
				SNew(SBorder)
				//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.LightGroupBorder"))
				.Padding(8.0f)
				[
					SNew(SHorizontalBox)
					//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
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
						//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
						.ColorAndOpacity(FLinearColor::Black)
						//.ShadowColorAndOpacity(FLinearColor::Black)
						//.ShadowOffset(FVector2D::UnitVector)
						.AutoWrapText(true)
						.Text(FText::FromString("Dynamic Objects must have a valid Mesh Name\nTo have data aggregated, Dynamic Objects must have a Unique Id"))
					]
					+SHorizontalBox::Slot()
					[
						SNew(SButton)
						//.Visibility(this, &SDynamicObjectManagerWidget::IsDynamicsVisible)
						.OnClicked_Raw(this, &SDynamicObjectManagerWidget::ValidateAndRefresh)
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
							.Text(FText::FromString("Validate Mesh Names and Unique Ids"))
						]
					]
				]
			]
			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			//.VAlign(valign_c)
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(64)
				[
					SNew(SButton)
					.IsEnabled_Raw(this,&SDynamicObjectManagerWidget::IsUploadIdsEnabled)
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::UploadDynamicsManifest)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString("Upload Dynamic Objects Ids to SceneExplorer for Aggregation"))
					]
				]
			]

#pragma endregion
			]
#pragma endregion
		];

		FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
		FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();

}

FReply SDynamicObjectManagerWidget::SelectAll()
{
	UWorld* World = GEditor->LevelViewportClients[0]->GetWorld();
	GEditor->Exec(World, TEXT("actor select all"));
	return FReply::Handled();
}

ECheckBoxState SDynamicObjectManagerWidget::GetNoExportGameplayMeshCheckbox() const
{
	if (NoExportGameplayMeshes)return ECheckBoxState::Checked;
	return ECheckBoxState::Unchecked;
}

ECheckBoxState SDynamicObjectManagerWidget::GetOnlyExportSelectedCheckbox() const
{
	if (OnlyExportSelected)return ECheckBoxState::Checked;
	return ECheckBoxState::Unchecked;
}

FText SDynamicObjectManagerWidget::GetDisplayDeveloperKey() const
{
	return FText::FromString(DisplayDeveloperKey);
}

void SDynamicObjectManagerWidget::OnDeveloperKeyChanged(const FText& Text)
{
	DisplayDeveloperKey = Text.ToString();
}

void SDynamicObjectManagerWidget::RefreshList()
{
	SceneDynamicObjectList->ListViewWidget->RequestListRefresh();
}

FReply SDynamicObjectManagerWidget::SelectDynamic(TSharedPtr<FDynamicData> data)
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

int32 SDynamicObjectManagerWidget::CountDynamicObjectsInScene() const
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

FText SDynamicObjectManagerWidget::DisplayDynamicObjectsCountInScene() const
{
	return DynamicCountInScene;
}

FReply SDynamicObjectManagerWidget::RefreshDisplayDynamicObjectsCountInScene()
{
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	SceneDynamicObjectList->RefreshList();

	return FReply::Handled();
}

EVisibility SDynamicObjectManagerWidget::GetDuplicateDyanmicObjectVisibility() const
{
	return FCognitiveEditorTools::GetInstance()->GetDuplicateDyanmicObjectVisibility();
}

EVisibility SDynamicObjectManagerWidget::UploadErrorVisibility() const
{
	if (FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode == 200) { return EVisibility::Collapsed; }
	if (FCognitiveEditorTools::GetInstance()->WizardUploadResponseCode == 201) { return EVisibility::Collapsed; }
	return EVisibility::Visible;
}

FText SDynamicObjectManagerWidget::UploadErrorText() const
{
	return FText::FromString(FCognitiveEditorTools::GetInstance()->WizardUploadError);
}

FReply SDynamicObjectManagerWidget::ExportAndUploadDynamics()
{
	//try to export all dynamics that don't have directories
	FProcHandle fph = FCognitiveEditorTools::GetInstance()->ExportNewDynamics();
	if (fph.IsValid())
	{
		FPlatformProcess::WaitForProc(fph);
	}

	//then upload all
	FCognitiveEditorTools::GetInstance()->UploadDynamics();

	return FReply::Handled();
}

FReply SDynamicObjectManagerWidget::ValidateAndRefresh()
{
	FCognitiveEditorTools::GetInstance()->SetUniqueDynamicIds();
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	SceneDynamicObjectList->RefreshList();

	return FReply::Handled();
}

void SDynamicObjectManagerWidget::OnBlenderPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BlenderPath = Text.ToString();
}

void SDynamicObjectManagerWidget::OnExportPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BaseExportDirectory = Text.ToString();
	RefreshList();
}

EVisibility SDynamicObjectManagerWidget::AreSettingsVisible() const
{
	return bSettingsVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SDynamicObjectManagerWidget::ToggleSettingsVisible()
{
	bSettingsVisible = !bSettingsVisible;
	return FReply::Handled();
}

FReply SDynamicObjectManagerWidget::ExportSelectedDynamicData()
{
	//get data from selected items in list
	auto selected = SceneDynamicObjectList->ListViewWidget->GetSelectedItems();
	TArray<TSharedPtr<FDynamicData>> dynamicData;
	for (auto &elem : selected)
	{
		GLog->Log("export dynamic data mesh name " + elem->MeshName);
		dynamicData.Add(elem);
	}
	FCognitiveEditorTools::GetInstance()->ExportDynamicData(dynamicData);
	return FReply::Handled();
}

FReply SDynamicObjectManagerWidget::UploadSelectedDynamicData()
{
	auto selected = SceneDynamicObjectList->ListViewWidget->GetSelectedItems();

	//should export all dynamic objects that need it, then wait for blender to fix them all

	TArray<TSharedPtr<FDynamicData>> dynamicData;
	
	for (auto &elem : selected)
	{
		//if files dont exist, export first
		FString path = FCognitiveEditorTools::GetInstance()->GetDynamicsExportDirectory() + "/" + elem->MeshName + "/" + elem->MeshName;
		FString gltfpath = path + ".gltf";
		if (!FPaths::FileExists(*gltfpath))
		{
			//not exported
			dynamicData.Add(elem);
		}
	}

	FProcHandle fph = FCognitiveEditorTools::GetInstance()->ExportDynamicData(dynamicData);
	if (fph.IsValid())
	{
		FPlatformProcess::WaitForProc(fph);
	}

	//then upload them all
	for (auto& elem : selected)
	{
		FCognitiveEditorTools::GetInstance()->UploadDynamic(elem->MeshName);
	}
	return FReply::Handled();
}

bool SDynamicObjectManagerWidget::IsExportAllEnabled() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasFoundBlenderAndExportDir()) { return false; }
	return true;
}

bool SDynamicObjectManagerWidget::IsExportSelectedEnabled() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasFoundBlenderAndExportDir()) { return false; }
	
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			//SelectionSetCache.Add(Actor);
			UActorComponent* actorComponent = Actor->GetComponentByClass(UDynamicObject::StaticClass());
			if (actorComponent == NULL)
			{
				continue;
			}
			UDynamicObject* dynamicComponent = Cast<UDynamicObject>(actorComponent);
			if (dynamicComponent == NULL)
			{
				continue;
			}

			//check directory for obj/gltf files

			//auto tools = FCognitiveEditorTools::GetInstance();
			////
			//FString path = tools->GetDynamicsExportDirectory() + "/" + dynamicComponent->MeshName + "/" + dynamicComponent->MeshName;
			//FString objpath = path + ".obj";
			//FString gltfpath = path + ".gltf";
			//
			//if (!FPaths::FileExists(*objpath) && !FPaths::FileExists(*gltfpath))
			//{
			//	continue;
			//}
			return true;
		}
	}

	return false;
}

bool SDynamicObjectManagerWidget::IsUploadAllEnabled() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->HasFoundBlender()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId()) { return false; }
	//if (!FCognitiveEditorTools::GetInstance()->HasExportedAnyDynamicMeshes()) { return false; }
	return true;
}

bool SDynamicObjectManagerWidget::IsUploadInvalid() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return true; }
	if (!FCognitiveEditorTools::GetInstance()->HasFoundBlender()) { return true; }
	if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory()) { return true; }
	if (!FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId()) { return true; }
	//if (!FCognitiveEditorTools::GetInstance()->HasExportedAnyDynamicMeshes()) { return true; }
	return false;
}

FText SDynamicObjectManagerWidget::GetUploadInvalidCause() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return FText::FromString("Upload Invalid. Developer Key is not set"); }
	if (!FCognitiveEditorTools::GetInstance()->HasFoundBlender()) { return FText::FromString("Upload Invalid. Blender path is invalid"); }
	if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory()) { return FText::FromString("Upload Invalid. Export Path is invalid"); }
	if (!FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId()) { return FText::FromString("Upload Invalid. Scene does not have a Scene ID"); }
	return FText::GetEmpty();
}

bool SDynamicObjectManagerWidget::IsUploadIdsEnabled() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId()) { return false; }
	return true;
}

bool SDynamicObjectManagerWidget::IsUploadSelectedEnabled() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->HasFoundBlenderAndExportDir()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId()) { return false; }
	
	auto tools = FCognitiveEditorTools::GetInstance();
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			//SelectionSetCache.Add(Actor);
			UActorComponent* actorComponent = Actor->GetComponentByClass(UDynamicObject::StaticClass());
			if (actorComponent == NULL)
			{
				continue;
			}
			UDynamicObject* dynamicComponent = Cast<UDynamicObject>(actorComponent);
			if (dynamicComponent == NULL)
			{
				continue;
			}
			return true;
		}
	}

	return false;
}

FText SDynamicObjectManagerWidget::UploadSelectedText() const
{
	auto selected = SceneDynamicObjectList->ListViewWidget->GetSelectedItems();

	//should export all dynamic objects that need it, then wait for blender to fix them all

	TArray<FString> dynamicMeshNames;

	for (auto& elem : selected)
	{
		if (dynamicMeshNames.Contains(elem->MeshName))
		{
			continue;
		}
		dynamicMeshNames.Add(elem->MeshName);
	}

	//get selected dynamic data
	//for each unique mesh name

	return FText::FromString("Upload " + FString::FromInt(dynamicMeshNames.Num()) + " Meshes");
}

FText SDynamicObjectManagerWidget::ExportSelectedText() const
{
	int32 validExportCount=0;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			//SelectionSetCache.Add(Actor);
			UActorComponent* actorComponent = Actor->GetComponentByClass(UDynamicObject::StaticClass());
			if (actorComponent == NULL)
			{
				continue;
			}
			UDynamicObject* dynamicComponent = Cast<UDynamicObject>(actorComponent);
			if (dynamicComponent == NULL)
			{
				continue;
			}
			validExportCount++;
		}
	}

	return FText::FromString("Export " + FString::FromInt(validExportCount) + " Selected");
}

FText SDynamicObjectManagerWidget::GetSettingsButtonText() const
{
	if (bSettingsVisible)
		return FText::FromString("Hide Settings");
	else
		return FText::FromString("Show Settings");
}

FText SDynamicObjectManagerWidget::GetSceneText() const
{
	auto tools = FCognitiveEditorTools::GetInstance();
	if (tools->CurrentSceneHasSceneId())
	{
		auto data = tools->GetCurrentSceneData();
		return FText::FromString("SceneName: " + data->Name + "   Id: " + data->Id);
	}
	return FText::FromString("Scene has not been exported!");
}