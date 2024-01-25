
#include "DynamicObjectManagerWidget.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

TArray<FDashboardObject> SDynamicObjectManagerWidget::dashboardObjects = TArray<FDashboardObject>();

TArray<TSharedPtr<FDynamicData>> SDynamicObjectManagerWidget::GetSceneDynamics()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneDynamics();
}

void SDynamicObjectManagerWidget::CheckForExpiredDeveloperKey()
{
	if (FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		GConfig->Flush(true, GEngineIni);
		auto Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &SDynamicObjectManagerWidget::OnDeveloperKeyResponseReceived);
		FString gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
		FString url = "https://" + gateway + "/v0/apiKeys/verify";
		Request->SetURL(url);
		Request->SetVerb("GET");
		Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + FCognitiveEditorTools::GetInstance()->DeveloperKey);
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

void SDynamicObjectManagerWidget::GetDashboardManifest()
{
	if (FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		auto currentSceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
		if (!currentSceneData.IsValid())
		{
			GLog->Log("SDynamicObjectManagerWidget::GetDashboardManifest failed. current scene is null");
			return;
		}

		auto Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &SDynamicObjectManagerWidget::OnDashboardManifestResponseReceived);
		FString gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
		FString versionid = FString::FromInt(currentSceneData->VersionId);
		FString url = "https://" + gateway + "/v0/versions/"+versionid+"/objects";
		Request->SetURL(url);
		Request->SetVerb("GET");
		Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + FCognitiveEditorTools::GetInstance()->DeveloperKey);
		Request->ProcessRequest();
	}
	else
	{
		GLog->Log("SDynamicObjectManagerWidget::GetDashboardManifest failed. developer key missing");
	}
}

void SDynamicObjectManagerWidget::OnDashboardManifestResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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
		auto content = Response->GetContentAsString();
		if (FJsonObjectConverter::JsonArrayStringToUStruct(content, &dashboardObjects, 0, 0))
		{

		}
		else
		{
			GLog->Log("SDynamicObjectManagerWidget::OnDashboardManifestResponseReceived failed to deserialize dynamic object list");
			GLog->Log(content);
		}
	}
	else
	{
		GLog->Log("SDynamicObjectManagerWidget::OnDashboardManifestResponseReceived response code " + FString::FromInt(responseCode));
	}
}

void SDynamicObjectManagerWidget::Construct(const FArguments& Args)
{
	float padding = 10;

	FCognitiveEditorTools::CheckIniConfigured();
	CheckForExpiredDeveloperKey();

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot() //if the scene isn't valid, display the onboarding prompt
			.Padding(0, 10, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					[
						SNew(STextBlock)
						.Visibility(this, &SDynamicObjectManagerWidget::GetSceneWarningVisibility)
						.Text(this, &SDynamicObjectManagerWidget::GetUploadInvalidCause)
					]
				]
			]
			+ SVerticalBox::Slot() //text to scene setup window
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Visibility(this, &SDynamicObjectManagerWidget::SceneNotUploadedVisibility)
					.AutoWrapText(true)
					.Justification(ETextJustify::Center)
					.Text(FText::FromString("This window is also accessible from the Cognitive3D menu.\n\nYou can review and upload Dynamic Objects from here.\n\n You will be prompted to export mesh geometry when you continue to the Scene Setup Window."))
				]
			]
			+ SVerticalBox::Slot() //button to scene setup window
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(SBox)
				.Visibility(this, &SDynamicObjectManagerWidget::SceneNotUploadedVisibility)
				.WidthOverride(256)
				.HeightOverride(32)
				[
					SNew(SButton)
					.Text(FText::FromString("Open Scene Setup Window"))
					.OnClicked(this,&SDynamicObjectManagerWidget::ExportAndOpenSceneSetupWindow)
				]
			]

			+ SVerticalBox::Slot() //warning for invalid scenes
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SDynamicObjectManagerWidget::SceneUploadedVisibility)
					[
						SNew(STextBlock)
						.Text(this, &SDynamicObjectManagerWidget::GetSceneText)
					]
				]
			]

			+SVerticalBox::Slot() //invalid dynamic data warning
			.AutoHeight()
			.Padding(FMargin(64, 24, 64, 24))
			[
				SNew(SBorder)
				.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.LightGroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.8f, 0, 0))
				.Padding(8.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("SettingsEditor.WarningIcon"))
					]
						// Notice
					+SHorizontalBox::Slot()
					.Padding(16.0f, 0.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(FText::FromString("Dynamic Objects must have a valid Mesh Name\nTo have data aggregated, Dynamic Objects must have a Unique Id"))
					]
					+SHorizontalBox::Slot()
					[
						SNew(SButton)
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

			+SVerticalBox::Slot() //refresh button and dynamic object count
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(128)
				[
					SNew(SButton)
					.Text(FText::FromString("Refresh"))
					.OnClicked(this, &SDynamicObjectManagerWidget::RefreshDisplayDynamicObjectsCountInScene)
				]
			]

			+ SVerticalBox::Slot() //list widget
			.FillHeight(1)
			.MaxHeight(500)
			.Padding(0, 0, 0, padding)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				]
				+ SOverlay::Slot()
				[
					SNew(SBox)
					[
						SAssignNew(SceneDynamicObjectTable,SDynamicObjectTableWidget)
					]
				]
			]

			
			+ SVerticalBox::Slot() //upload buttons
			.Padding(0, 0, 0, padding)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					.HeightOverride(32)
					.WidthOverride(256)
					[
						SNew(SButton)
						.IsEnabled(this, &SDynamicObjectManagerWidget::IsUploadSelectedEnabled)
						.Text_Raw(this, &SDynamicObjectManagerWidget::UploadSelectedText)
						.ToolTipText(this, &SDynamicObjectManagerWidget::UploadSelectedMeshesTooltip)
						.OnClicked_Raw(this, &SDynamicObjectManagerWidget::UploadSelectedDynamicObjects)
					]
				]
				+SHorizontalBox::Slot()
				[
					SNew(SBox)
					.HeightOverride(32)
					.WidthOverride(256)
					[
						SNew(SButton)
						.IsEnabled(this, &SDynamicObjectManagerWidget::IsUploadAllEnabled)
						.Text(FText::FromString("Upload All Dynamics"))
						.ToolTipText(this, &SDynamicObjectManagerWidget::UploadAllMeshesTooltip)
						.OnClicked_Raw(this, &SDynamicObjectManagerWidget::UploadAllDynamicObjects)
					]
				]
			]
			]
		];

		FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
		RefreshDisplayDynamicObjectsCountInScene();
}

void SDynamicObjectManagerWidget::RefreshList()
{
	SceneDynamicObjectTable->TableViewWidget->RequestListRefresh();
}

FReply SDynamicObjectManagerWidget::SelectDynamic(TSharedPtr<FDynamicData> data)
{
	GEditor->SelectNone(false, true, false);

	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
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

FReply SDynamicObjectManagerWidget::RefreshDisplayDynamicObjectsCountInScene()
{
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	GetDashboardManifest();
	
	if (!SceneDynamicObjectTable.IsValid())
	{
		GLog->Log("SceneDynamicObjectTable invalid!");
	}
	else
	{
		SceneDynamicObjectTable->RefreshTable();
	}

	return FReply::Handled();
}

EVisibility SDynamicObjectManagerWidget::GetDuplicateDyanmicObjectVisibility() const
{
	return FCognitiveEditorTools::GetInstance()->GetDuplicateDyanmicObjectVisibility();
}

FReply SDynamicObjectManagerWidget::UploadAllDynamicObjects()
{
	//find the corresponding asset
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;

#if ENGINE_MAJOR_VERSION == 4
	Filter.ClassNames.Add(UDynamicIdPoolAsset::StaticClass()->GetFName());
#elif ENGINE_MAJOR_VERSION == 5 && (ENGINE_MINOR_VERSION == 0 || ENGINE_MINOR_VERSION == 1)
	Filter.ClassNames.Add(UDynamicIdPoolAsset::StaticClass()->GetFName());
#elif ENGINE_MAJOR_VERSION == 5 && (ENGINE_MINOR_VERSION == 2 || ENGINE_MINOR_VERSION == 3)
	Filter.ClassPaths.Add(UDynamicIdPoolAsset::StaticClass()->GetClassPathName());
#endif

	Filter.bRecursiveClasses = true; // Set to true if you want to include subclasses

	TArray<FAssetData> AssetData;
	AssetRegistry.GetAssets(Filter, AssetData);

	if (AssetData.Num() > 0)
	{
		FSuppressableWarningDialog::FSetupInfo Info1(LOCTEXT("UploadIdsForAggregation", "Do you want to Upload all Dynamic Object Id Pool's Ids for Aggregation?"), LOCTEXT("UploadIdsForAggregationTitle", "Upload Ids For Aggregation"), "UploadIdsForAggregationBody");
		Info1.ConfirmText = LOCTEXT("Yes", "Yes");
		Info1.CancelText = LOCTEXT("No", "No");
		Info1.CheckBoxText = FText();
		FSuppressableWarningDialog UploadSelectedIdPools(Info1);
		FSuppressableWarningDialog::EResult result1 = UploadSelectedIdPools.ShowModal();

		if (result1 == FSuppressableWarningDialog::EResult::Confirm)
		{
			for (const FAssetData& Asset : AssetData)
			{
				//get the actual asset from the asset data
				UObject* IdPoolObject = Asset.GetAsset();
				//cast it to a dynamic id pool asset
				UDynamicIdPoolAsset* IdPoolAsset = Cast<UDynamicIdPoolAsset>(IdPoolObject);

				UE_LOG(LogTemp, Warning, TEXT("Found dynamic id pool asset %s, uploading ids"), *IdPoolAsset->PrefabName);
				FReply uploadReply = FCognitiveEditorTools::GetInstance()->UploadDynamicsManifestIds(IdPoolAsset->Ids, IdPoolAsset->MeshName, IdPoolAsset->PrefabName);
				if (uploadReply.IsEventHandled())
				{

				}

			}
		}
	}

		
	

	//popup asking if meshes should be exported too
	FSuppressableWarningDialog::FSetupInfo Info(LOCTEXT("ExportSelectedDynamicsBody", "Do you want to export all Dynamic Object meshes before uploading to Scene Explorer?"), LOCTEXT("ExportSelectedDynamicsTitle", "Export Selected Dynamic Objects"), "ExportSelectedDynamicsBody");
	Info.ConfirmText = LOCTEXT("Yes", "Yes");
	Info.CancelText = LOCTEXT("No", "No");
	Info.CheckBoxText = FText();
	FSuppressableWarningDialog ExportSelectedDynamicMeshes(Info);
	FSuppressableWarningDialog::EResult result = ExportSelectedDynamicMeshes.ShowModal();

	if (result == FSuppressableWarningDialog::EResult::Confirm)
	{
		FProcHandle fph = FCognitiveEditorTools::GetInstance()->ExportAllDynamics();
	}

	FSuppressableWarningDialog::FSetupInfo Info2(LOCTEXT("UploadSelectedDynamicsBody", "Do you want to upload all Dynamic Object to Scene Explorer? Note: This will only upload meshes that have been exported."), LOCTEXT("UploadSelectedDynamicsTitle", "Upload Selected Dynamic Objects"), "UploadSelectedDynamicsBody");
	Info2.ConfirmText = LOCTEXT("Yes", "Yes");
	Info2.CancelText = LOCTEXT("No", "No");
	Info2.CheckBoxText = FText();
	FSuppressableWarningDialog UploadSelectedDynamicMeshes(Info2);
	FSuppressableWarningDialog::EResult result2 = UploadSelectedDynamicMeshes.ShowModal();

	if (result2 == FSuppressableWarningDialog::EResult::Confirm)
	{
		//then upload all
		FCognitiveEditorTools::GetInstance()->UploadDynamics();

		//upload aggregation manifest data
		FCognitiveEditorTools::GetInstance()->UploadDynamicsManifest();
	}

	return FReply::Handled();
}

FReply SDynamicObjectManagerWidget::ValidateAndRefresh()
{
	FCognitiveEditorTools::GetInstance()->SetUniqueDynamicIds();
	RefreshDisplayDynamicObjectsCountInScene();
	return FReply::Handled();
}

void SDynamicObjectManagerWidget::OnExportPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BaseExportDirectory = Text.ToString();
	RefreshList();
}

FReply SDynamicObjectManagerWidget::UploadSelectedDynamicObjects()
{
	auto selected = SceneDynamicObjectTable->TableViewWidget->GetSelectedItems();


	for (auto& dynamic : selected)
	{
		//dynamic with id pool, export mesh and upload ids for aggregation
		if (dynamic->DynamicType == EDynamicTypes::DynamicIdPool)
		{
			//popup asking if meshes should be exported too
			FSuppressableWarningDialog::FSetupInfo Info(LOCTEXT("ExportSelectedDynamicsBody", "Do you want to export the selected Dynamic Object meshes before uploading to Scene Explorer?"), LOCTEXT("ExportSelectedDynamicsTitle", "Export Selected Dynamic Objects"), "ExportSelectedDynamicsBody");
			Info.ConfirmText = LOCTEXT("Yes", "Yes");
			Info.CancelText = LOCTEXT("No", "No");
			Info.CheckBoxText = FText();
			FSuppressableWarningDialog ExportSelectedDynamicMeshes(Info);
			FSuppressableWarningDialog::EResult result = ExportSelectedDynamicMeshes.ShowModal();
			TArray<TSharedPtr<FDynamicData>> meshOnly;
			meshOnly.Add(dynamic);
			if (result == FSuppressableWarningDialog::EResult::Confirm)
			{
				FProcHandle fph = FCognitiveEditorTools::GetInstance()->ExportDynamicData(meshOnly);
				if (fph.IsValid())
				{
					FPlatformProcess::WaitForProc(fph);
				}
			}

			FSuppressableWarningDialog::FSetupInfo Info1(LOCTEXT("UploadIdsForAggregation", "Do you want to Upload the selected Dynamic Object Id Pool's Ids for Aggregation?"), LOCTEXT("UploadIdsForAggregationTitle", "Upload Ids For Aggregation"), "UploadIdsForAggregationBody");
			Info1.ConfirmText = LOCTEXT("Yes", "Yes");
			Info1.CancelText = LOCTEXT("No", "No");
			Info1.CheckBoxText = FText();
			FSuppressableWarningDialog UploadSelectedIdPools(Info1);
			FSuppressableWarningDialog::EResult result1 = UploadSelectedIdPools.ShowModal();

			if (result1 == FSuppressableWarningDialog::EResult::Confirm)
			{
				FReply uploadReply = FCognitiveEditorTools::GetInstance()->UploadDynamicsManifestIds(dynamic->DynamicPoolIds, dynamic->MeshName, dynamic->Name);
				if (uploadReply.IsEventHandled())
				{

				}
			}
		}
		//id pool asset, upload ids for aggregation
		else if (dynamic->DynamicType == EDynamicTypes::DynamicIdPoolAsset)
		{
			FSuppressableWarningDialog::FSetupInfo Info1(LOCTEXT("UploadIdsForAggregation", "Do you want to Upload the selected Dynamic Object Id Pool's Ids for Aggregation?"), LOCTEXT("UploadIdsForAggregationTitle", "Upload Ids For Aggregation"), "UploadIdsForAggregationBody");
			Info1.ConfirmText = LOCTEXT("Yes", "Yes");
			Info1.CancelText = LOCTEXT("No", "No");
			Info1.CheckBoxText = FText();
			FSuppressableWarningDialog UploadSelectedIdPools(Info1);
			FSuppressableWarningDialog::EResult result1 = UploadSelectedIdPools.ShowModal();

			if (result1 == FSuppressableWarningDialog::EResult::Confirm)
			{
				FReply uploadReply = FCognitiveEditorTools::GetInstance()->UploadDynamicsManifestIds(dynamic->DynamicPoolIds, dynamic->MeshName, dynamic->Name);
				if (uploadReply.IsEventHandled())
				{

				}
			}
		}
		//else its a normal dynamic object, we export
		else
		{
			//popup asking if meshes should be exported too
			FSuppressableWarningDialog::FSetupInfo Info(LOCTEXT("ExportSelectedDynamicsBody", "Do you want to export the selected Dynamic Object meshes before uploading to Scene Explorer?"), LOCTEXT("ExportSelectedDynamicsTitle", "Export Selected Dynamic Objects"), "ExportSelectedDynamicsBody");
			Info.ConfirmText = LOCTEXT("Yes", "Yes");
			Info.CancelText = LOCTEXT("No", "No");
			Info.CheckBoxText = FText();
			FSuppressableWarningDialog ExportSelectedDynamicMeshes(Info);
			FSuppressableWarningDialog::EResult result = ExportSelectedDynamicMeshes.ShowModal();
			TArray<TSharedPtr<FDynamicData>> meshOnly;
			meshOnly.Add(dynamic);
			if (result == FSuppressableWarningDialog::EResult::Confirm)
			{
				FProcHandle fph = FCognitiveEditorTools::GetInstance()->ExportDynamicData(meshOnly);
				if (fph.IsValid())
				{
					FPlatformProcess::WaitForProc(fph);
				}
			}
		}

	}
	
	//then perform upload
	FSuppressableWarningDialog::FSetupInfo Info2(LOCTEXT("UploadSelectedDynamicsBody", "Do you want to upload the selected Dynamics to Scene Explorer? Note: This will only upload meshes that have been exported."), LOCTEXT("UploadSelectedDynamicsTitle", "Upload Selected Dynamic Objects"), "UploadSelectedDynamicsBody");
	Info2.ConfirmText = LOCTEXT("Yes", "Yes");
	Info2.CancelText = LOCTEXT("No", "No");
	Info2.CheckBoxText = FText();
	FSuppressableWarningDialog UploadSelectedDynamicMeshes(Info2);
	FSuppressableWarningDialog::EResult result2 = UploadSelectedDynamicMeshes.ShowModal();

	if (result2 == FSuppressableWarningDialog::EResult::Confirm)
	{
		//then upload
		for (auto& elem : selected)
		{
			FCognitiveEditorTools::GetInstance()->UploadDynamic(elem->MeshName);
		}

		//upload aggregation manifest data of selected objects

		TArray<UDynamicObject*> dynamics;

		//get all the dynamic objects in the scene
		for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
		{
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

			if (dynamic->IdSourceType == EIdSourceType::CustomId && dynamic->CustomId != "")
			{
				FString findId = dynamic->CustomId;

				auto isDynamicSelected = selected.ContainsByPredicate([findId](const TSharedPtr<FDynamicData> InItem) {return InItem->Id == findId;});

				if (isDynamicSelected)
				{
					dynamics.Add(dynamic);
				}
			}
		}

		FCognitiveEditorTools::GetInstance()->UploadSelectedDynamicsManifest(dynamics);
	}

	return FReply::Handled();
}

FText SDynamicObjectManagerWidget::UploadSelectedMeshesTooltip() const
{
	if (IsUploadSelectedEnabled())
	{
		return FText::FromString("");
	}
	return FText::FromString("Must export meshes first to upload");
}

FText SDynamicObjectManagerWidget::UploadAllMeshesTooltip() const
{
	if (IsUploadAllEnabled())
	{
		return FText::FromString("");
	}
	return FText::FromString("Must export meshes first to upload");
}

bool SDynamicObjectManagerWidget::IsUploadAllEnabled() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId()) { return false; }
	return true;
}

FText SDynamicObjectManagerWidget::GetUploadInvalidCause() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return FText::FromString("Developer Key is not set"); }
	if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory()) { return FText::FromString("Export Path is invalid"); }
	return FText::GetEmpty();
}

EVisibility SDynamicObjectManagerWidget::GetSceneWarningVisibility() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return EVisibility::Visible; }
	if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory()) { return EVisibility::Visible; }
	return EVisibility::Collapsed;
}

bool SDynamicObjectManagerWidget::IsUploadSelectedEnabled() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId()) { return false; }
	
	//use the selection in the table, not in the scene
	auto data = SceneDynamicObjectTable->GetSelectedDataCount();
	return data > 0;
}

FText SDynamicObjectManagerWidget::UploadSelectedText() const
{
	auto selected = SceneDynamicObjectTable->TableViewWidget->GetSelectedItems();

	//should export all dynamic objects that need it

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

	//return FText::FromString("Upload " + FString::FromInt(dynamicMeshNames.Num()) + " Selected Dynamics");
	return FText::FromString("Upload " + FString::FromInt(selected.Num()) + " Selected Dynamics");
}

//if the scene has not been exported, the window displays the onboarding text instead of this scene text
FText SDynamicObjectManagerWidget::GetSceneText() const
{
	auto tools = FCognitiveEditorTools::GetInstance();
	if (tools->CurrentSceneHasSceneId())
	{
		auto data = tools->GetCurrentSceneData();
		return FText::FromString("Scene: " + data->Name + "   Version: " + FString::FromInt(data->VersionNumber));
	}
	return FText::FromString("Scene has not been exported!");
}

EVisibility SDynamicObjectManagerWidget::SceneNotUploadedVisibility() const
{
	if (FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId())
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

EVisibility SDynamicObjectManagerWidget::SceneUploadedVisibility() const
{
	if (FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

FReply SDynamicObjectManagerWidget::ExportAndOpenSceneSetupWindow()
{
	//popup asking if meshes should be exported too
	FSuppressableWarningDialog::FSetupInfo Info(LOCTEXT("ExportSelectedDynamicsBody", "Do you want to export all Dynamic Object meshes?"), LOCTEXT("ExportSelectedDynamicsTitle", "Export all Dynamic Objects"), "ExportSelectedDynamicsBody");
	Info.ConfirmText = LOCTEXT("Yes", "Yes");
	Info.CancelText = LOCTEXT("No", "No");
	Info.CheckBoxText = FText();
	FSuppressableWarningDialog ExportSelectedDynamicMeshes(Info);
	FSuppressableWarningDialog::EResult result = ExportSelectedDynamicMeshes.ShowModal();

	if (result == FSuppressableWarningDialog::EResult::Confirm)
	{
		//try to export all dynamics that don't have directories
		FProcHandle fph = FCognitiveEditorTools::GetInstance()->ExportAllDynamics();
		if (fph.IsValid())
		{
			FPlatformProcess::WaitForProc(fph);
		}
	}

	//close this window and open the scene setup window
	FCognitiveVREditorModule::CloseDynamicObjectWindow();
	FCognitiveVREditorModule::SpawnCognitiveSceneSetupTab();

	return FReply::Handled();
}