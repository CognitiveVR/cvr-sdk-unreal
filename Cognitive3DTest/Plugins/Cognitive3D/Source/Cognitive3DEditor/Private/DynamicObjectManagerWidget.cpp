/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "DynamicObjectManagerWidget.h"
#include "Analytics.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

TArray<FDashboardObject> SDynamicObjectManagerWidget::dashboardObjects = TArray<FDashboardObject>();
TSharedPtr<FString> SDynamicObjectManagerWidget::SceneDisplayName = TSharedPtr<FString>();
TSharedPtr<FString> SDynamicObjectManagerWidget::AdjustedSceneDisplayName = TSharedPtr<FString>();

TArray<TSharedPtr<FDynamicData>> SDynamicObjectManagerWidget::GetSceneDynamics()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneDynamics();
}

void SDynamicObjectManagerWidget::CheckForExpiredDeveloperKey()
{
	if (FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
		GConfig->LoadFile(C3DSettingsPath);
		auto Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &SDynamicObjectManagerWidget::OnDeveloperKeyResponseReceived);
		FString gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
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

		UE_LOG(LogTemp, Error, TEXT("Developer Key Response is invalid. Developer key may be invalid or expired. Check your internet connection"));
		return;
	}

	int32 responseCode = Response->GetResponseCode();
	if (responseCode == 200)
	{
	}
	else
	{
		SGenericDialogWidget::OpenDialog(FText::FromString("Your developer key has expired"), SNew(STextBlock).Text(FText::FromString("Please log in to the dashboard, select your project, and generate a new developer key.\n\nNote:\nDeveloper keys allow you to upload and modify Scenes, and the keys expire after 90 days.\nApplication keys authorize your app to send data to our server, and they never expire.")));
		UE_LOG(LogTemp, Error, TEXT("Developer Key Response Code: %d. Developer key may be invalid or expired"), responseCode);
	}
}

void SDynamicObjectManagerWidget::GetDashboardManifest()
{
	TSharedPtr<FEditorSceneData> currentSceneData;

	if (AdjustedSceneDisplayName.IsValid())
	{
		currentSceneData = FCognitiveEditorTools::GetInstance()->GetSceneData(*AdjustedSceneDisplayName);
	}
	else
	{
		currentSceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	}

	if (FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		//auto currentSceneData = FCognitiveEditorTools::GetInstance()->GetSceneData(*SceneDisplayName);
		if (!currentSceneData.IsValid())
		{
			GLog->Log("SDynamicObjectManagerWidget::GetDashboardManifest failed. Selected Scene isn't uploaded");
			return;
		}

		auto Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &SDynamicObjectManagerWidget::OnDashboardManifestResponseReceived);
		FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
		GConfig->LoadFile(C3DSettingsPath);
		FString gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
		FString versionid = FString::FromInt(currentSceneData->VersionId);
		FString url = "https://" + gateway + "/v0/versions/"+versionid+"/objects";
		Request->SetURL(url);
		Request->SetVerb("GET");
		Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + FCognitiveEditorTools::GetInstance()->DeveloperKey);
		Request->ProcessRequest();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SDynamicObjectManagerWidget::GetDashboardManifest failed. developer key missing"));
	}
}

void SDynamicObjectManagerWidget::OnDashboardManifestResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid() == false)
	{
		SGenericDialogWidget::OpenDialog(FText::FromString("Your developer key has expired"), SNew(STextBlock).Text(FText::FromString("Please log in to the dashboard, select your project, and generate a new developer key.\n\nNote:\nDeveloper keys allow you to upload and modify Scenes, and the keys expire after 90 days.\nApplication keys authorize your app to send data to our server, and they never expire.")));

		UE_LOG(LogTemp, Error, TEXT("Developer Key Response is invalid. Developer key may be invalid or expired. Check your internet connection"));
		return;
	}

	int32 responseCode = Response->GetResponseCode();
	if (responseCode == 200)
	{		
		auto content = Response->GetContentAsString();
		if (FJsonObjectConverter::JsonArrayStringToUStruct(content, &dashboardObjects, 0, 0))
		{
			SceneDynamicObjectTable->RefreshTable();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SDynamicObjectManagerWidget::OnDashboardManifestResponseReceived failed to deserialize dynamic object list"));
			GLog->Log(content);
			SceneDynamicObjectTable->RefreshTable();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SDynamicObjectManagerWidget::OnDashboardManifestResponseReceived response code %d"), responseCode);
	}
}

void SDynamicObjectManagerWidget::Construct(const FArguments& Args)
{
	float padding = 10;

	FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
	FCognitiveEditorTools::CheckIniConfigured();
	CheckForExpiredDeveloperKey();

	SceneDisplayName = MakeShareable(new FString(""));
	AdjustedSceneDisplayName = MakeShareable(new FString(""));

	SceneNamesComboList.Empty();
	for (auto sceneData : FCognitiveEditorTools::GetInstance()->GetSceneData())
	{
		SceneNamesComboList.Add(MakeShareable(new FString(sceneData->Path + "/" + sceneData->Name)));
	}
	TSharedPtr<FEditorSceneData> tempSceneData = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	TSharedPtr<FString> currentSceneFullPath = MakeShareable(new FString(tempSceneData->Path + "/" + tempSceneData->Name));

	if (tempSceneData.IsValid())
	{
		SceneDisplayName = MakeShareable(new FString(tempSceneData->Path + "/" + tempSceneData->Name));
		//AdjustedSceneDisplayName = MakeShareable(new FString(SceneDisplayName->Replace(TEXT("/"), TEXT("_"))));
		AdjustedSceneDisplayName = MakeShareable(new FString(FCognitiveEditorTools::GetInstance()->AdjustPathName(*SceneDisplayName)));
	}

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot() //scene selection dropdown and text
				.Padding(0, 0, 0, padding)
				//.HAlign(EHorizontalAlignment::HAlign_Center)
				//.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1)
					[
						SNew(SBox)
						.MaxDesiredHeight(32)
						.HeightOverride(32)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
							.Text(FText::FromString("Select the Scene Data for uploading:"))
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1)
					[
						SNew(SBox)
						.MaxDesiredHeight(20)
						.HeightOverride(20)
						//.Visibility(this, &SDynamicObjectManagerWidget::SceneUploadedVisibility)
						[
							SAssignNew(SceneNamesComboBox, SComboBox< TSharedPtr<FString> >)
							.OptionsSource(&SceneNamesComboList)
							.OnGenerateWidget(this, &SDynamicObjectManagerWidget::MakeSceneNamesComboWidget)
							.OnSelectionChanged(this, &SDynamicObjectManagerWidget::OnSceneNamesChanged)
							.OnComboBoxOpening(this, &SDynamicObjectManagerWidget::OnSceneNamesComboOpening)
							.InitiallySelectedItem(SceneDisplayName)
							.Content()
							[
								SNew(STextBlock)
								.Text(this, &SDynamicObjectManagerWidget::GetSceneNamesComboBoxContent)
								.Font(IDetailLayoutBuilder::GetDetailFont())
								//.ToolTipText(this, &FBodyInstanceCustomization::GetCollisionProfileComboBoxToolTip)
							]
						]
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

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, padding)
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
					.Text(FText::FromString("To easily assign Dynamic Objects to actors in your level, select them then press the button below."))
				]

			+ SVerticalBox::Slot() //upload buttons //
				.Padding(0, 0, 0, padding)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
						.HeightOverride(32)
						.WidthOverride(256)
						[
							SNew(SButton)
							.IsEnabled(this, &SDynamicObjectManagerWidget::IsActorInSceneSelected)
							.Text(FText::FromString("Add Dynamic Object Component(s)"))
							.ToolTipText(this, &SDynamicObjectManagerWidget::AssignDynamicTooltip)
							.OnClicked_Raw(this, &SDynamicObjectManagerWidget::AssignDynamicsToActors)
						]
					]
			]

			]
		];

		//calling refresh immediately when the window opens crashes if the current scene has scene data
		//RefreshDisplayDynamicObjectsCountInScene();
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
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2 
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
				FCognitiveEditorTools::GetInstance()->UploadDynamicsManifestIds(*AdjustedSceneDisplayName, IdPoolAsset->Ids, IdPoolAsset->MeshName, IdPoolAsset->PrefabName);
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
		FCognitiveEditorTools::GetInstance()->UploadDynamics(*AdjustedSceneDisplayName);

		//upload aggregation manifest data
		FCognitiveEditorTools::GetInstance()->UploadDynamicsManifest(*AdjustedSceneDisplayName);
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
			FText ExportText = LOCTEXT("ExportSelectedDynamicsBody", "Do you want to export the selected Dynamic Object meshes ({0}) before uploading to Scene Explorer?");
			FText dynMeshName = FText::FromString(dynamic->MeshName);
			FSuppressableWarningDialog::FSetupInfo Info(FText::Format(ExportText, dynMeshName), LOCTEXT("ExportSelectedDynamicsTitle", "Export Selected Dynamic Objects"), "ExportSelectedDynamicsBody");
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
					FCognitiveEditorTools::GetInstance()->ShowNotification(TEXT("Mesh Exported"));
				}
			}

			FText UploadText = LOCTEXT("UploadIdsForAggregation", "Do you want to Upload the selected Dynamic Object ({0}) Id Pool's Ids for Aggregation?");
			FText dynName = FText::FromString(dynamic->Name);
			FSuppressableWarningDialog::FSetupInfo Info1(FText::Format(UploadText, dynName), LOCTEXT("UploadIdsForAggregationTitle", "Upload Ids For Aggregation"), "UploadIdsForAggregationBody");
			Info1.ConfirmText = LOCTEXT("Yes", "Yes");
			Info1.CancelText = LOCTEXT("No", "No");
			Info1.CheckBoxText = FText();
			FSuppressableWarningDialog UploadSelectedIdPools(Info1);
			FSuppressableWarningDialog::EResult result1 = UploadSelectedIdPools.ShowModal();

			if (result1 == FSuppressableWarningDialog::EResult::Confirm)
			{
				FCognitiveEditorTools::GetInstance()->UploadDynamicsManifestIds(*AdjustedSceneDisplayName, dynamic->DynamicPoolIds, dynamic->MeshName, dynamic->Name);
			}
		}
		//id pool asset, upload ids for aggregation
		else if (dynamic->DynamicType == EDynamicTypes::DynamicIdPoolAsset)
		{
			FText UploadText = LOCTEXT("UploadIdsForAggregation", "Do you want to Upload the selected Dynamic Object ({0}) Id Pool's Ids for Aggregation?");
			FText dynName = FText::FromString(dynamic->Name);
			FSuppressableWarningDialog::FSetupInfo Info1(FText::Format(UploadText, dynName), LOCTEXT("UploadIdsForAggregationTitle", "Upload Ids For Aggregation"), "UploadIdsForAggregationBody");
			Info1.ConfirmText = LOCTEXT("Yes", "Yes");
			Info1.CancelText = LOCTEXT("No", "No");
			Info1.CheckBoxText = FText();
			FSuppressableWarningDialog UploadSelectedIdPools(Info1);
			FSuppressableWarningDialog::EResult result1 = UploadSelectedIdPools.ShowModal();

			if (result1 == FSuppressableWarningDialog::EResult::Confirm)
			{
				FCognitiveEditorTools::GetInstance()->UploadDynamicsManifestIds(*AdjustedSceneDisplayName, dynamic->DynamicPoolIds, dynamic->MeshName, dynamic->Name);
			}
		}
		//else its a normal dynamic object, we export
		else
		{
			//popup asking if meshes should be exported too
			FText ExportText = LOCTEXT("ExportSelectedDynamicsBody", "Do you want to export the selected Dynamic Object meshes ({0}) before uploading to Scene Explorer?");
			FText dynMeshName = FText::FromString(dynamic->MeshName);
			FSuppressableWarningDialog::FSetupInfo Info(FText::Format(ExportText, dynMeshName), LOCTEXT("ExportSelectedDynamicsTitle", "Export Selected Dynamic Objects"), "ExportSelectedDynamicsBody");
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
					FCognitiveEditorTools::GetInstance()->ShowNotification(TEXT("Mesh Exported"));
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
		int32 uploadCount = 0;
		for (auto& elem : selected)
		{
			FCognitiveEditorTools::GetInstance()->UploadDynamic(*AdjustedSceneDisplayName, elem->MeshName);
		}

		//upload aggregation manifest data of selected objects

		TArray<UDynamicObject*> dynamics;

		//get all the dynamic objects in the scene
		for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
		{
			for (UActorComponent* actorComponent : ActorItr->GetComponents())
			{
				if (actorComponent->IsA(UDynamicObject::StaticClass()))
				{
					UDynamicObject* dynamic = Cast<UDynamicObject>(actorComponent);
					if (dynamic == NULL)
					{
						continue;
					}

					if (dynamic->IdSourceType == EIdSourceType::CustomId && dynamic->CustomId != "")
					{
						FString findId = dynamic->CustomId;

						auto isDynamicSelected = selected.ContainsByPredicate([findId](const TSharedPtr<FDynamicData> InItem) {return InItem->Id == findId; });

						if (isDynamicSelected)
						{
							dynamics.Add(dynamic);
						}
					}
				}
			}
		}

		//get the blueprint dynamics in the project and add them to the list
		for (const TSharedPtr<FDynamicData>& data : FCognitiveEditorTools::GetInstance()->SceneDynamics)
		{
			// Iterate over all blueprints in the project
			for (TObjectIterator<UBlueprint> It; It; ++It)
			{
				UBlueprint* Blueprint = *It;
				if (Blueprint->GetName() == data->Name)
				{
					// Get the generated class from the blueprint
					UClass* BlueprintClass = Blueprint->GeneratedClass;
					if (BlueprintClass)
					{
						// Now, get the default object and access its components
						AActor* DefaultActor = Cast<AActor>(BlueprintClass->GetDefaultObject());
						if (DefaultActor)
						{
							// Use Simple Construction Script to inspect the blueprint's components
							if (UBlueprintGeneratedClass* BPGeneratedClass = Cast<UBlueprintGeneratedClass>(BlueprintClass))
							{
								const TArray<USCS_Node*>& SCSNodes = BPGeneratedClass->SimpleConstructionScript->GetAllNodes();
								// Iterate over the SCS nodes to find UDynamicObject components
								for (USCS_Node* SCSNode : SCSNodes)
								{
									if (SCSNode && SCSNode->ComponentTemplate)
									{
										// Check if the component is a UDynamicObject
										UDynamicObject* dynamicComponent = Cast<UDynamicObject>(SCSNode->ComponentTemplate);
										if (dynamicComponent)
										{
											if (dynamicComponent->MeshName == data->MeshName && !dynamics.Contains(dynamicComponent))
											{
												dynamics.Add(dynamicComponent);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		FCognitiveEditorTools::GetInstance()->UploadSelectedDynamicsManifest(*AdjustedSceneDisplayName, dynamics);
	}

	return FReply::Handled();
}

FText SDynamicObjectManagerWidget::UploadSelectedMeshesTooltip() const
{
	if (IsUploadSelectedEnabled())
	{
		return FText::FromString("");
	}
	else if (!FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId())
	{
		return FText::FromString("Use the Open Scene Setup Window above to export these meshes and continue the guided setup to the Scene Setup Window");
	}
	else if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory())
	{
		return FText::FromString("No Export Directory found. Please use the Project Setup Window to set one.");
	}
	else if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		return FText::FromString("No Developer Key found. Please use the Project Setup Window to set one.");
	}
	return FText::FromString("Must export meshes first to upload");
}

FText SDynamicObjectManagerWidget::UploadAllMeshesTooltip() const
{
	if (IsUploadAllEnabled())
	{
		return FText::FromString("");
	}
	else if (!SceneDisplayName.IsValid())
	{
		return FText::FromString("Use the Open Scene Setup Window above to export these meshes and continue the guided setup to the Scene Setup Window");
	}
	else if (!FCognitiveEditorTools::GetInstance()->SceneHasSceneId(*AdjustedSceneDisplayName))
	{
		return FText::FromString("Use the Open Scene Setup Window above to export these meshes and continue the guided setup to the Scene Setup Window");
	}
	else if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory())
	{
		return FText::FromString("No Export Directory found. Please use the Project Setup Window to set one.");
	}
	else if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		return FText::FromString("No Developer Key found. Please use the Project Setup Window to set one.");
	}
	return FText::FromString("Must export meshes first to upload");
}

bool SDynamicObjectManagerWidget::IsUploadAllEnabled() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory()) { return false; }
	if (!SceneDisplayName.IsValid()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->SceneHasSceneId(*AdjustedSceneDisplayName)) { return false; }
	return true;
}

FText SDynamicObjectManagerWidget::GetUploadInvalidCause() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return FText::FromString("Developer Key is not set"); }
	if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory()) { return FText::FromString("Export Path is invalid"); }
	return FText::GetEmpty();
}

FReply SDynamicObjectManagerWidget::AssignDynamicsToActors()
{
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			bool bAttachedToMeshComponent = false;

			// Get all components of the actor
			TArray<UActorComponent*> Components;
			Actor->GetComponents(Components);
			bool bActorHasDynamic = false;
			// Iterate over components to find mesh components
			for (UActorComponent* Component : Components)
			{
				UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component);
				if (MeshComponent)
				{
					// Get direct children of the mesh component
					TArray<USceneComponent*> MeshChildren;
					MeshComponent->GetChildrenComponents(false, MeshChildren);

					bool bHasDynamicObjectChild = false;
					for (USceneComponent* Child : MeshChildren)
					{
						if (Child && Child->IsA<UDynamicObject>())
						{
							bHasDynamicObjectChild = true;
							bActorHasDynamic = true;
							break;
						}
					}

					if (!bHasDynamicObjectChild)
					{
						// Attach the DynamicObject to this mesh component
						UDynamicObject* NewComponent = NewObject<UDynamicObject>(Actor);
						if (NewComponent)
						{
							NewComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
							NewComponent->RegisterComponent();
							Actor->AddInstanceComponent(NewComponent);
							bAttachedToMeshComponent = true;
							bActorHasDynamic = true;
							//break; // Exit the loop since we've attached our component
						}
					}
				}
			}

			if (!bAttachedToMeshComponent && !bActorHasDynamic)
			{
				// No suitable mesh component found and actor has no dynamics attach to the root component
				UDynamicObject* NewComponent = NewObject<UDynamicObject>(Actor);
				if (NewComponent)
				{
					NewComponent->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
					NewComponent->RegisterComponent();
					Actor->AddInstanceComponent(NewComponent);
				}
			}
		}
	}

	return FReply::Handled();
}

bool SDynamicObjectManagerWidget::IsActorInSceneSelected() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey()) { return false; }

	USelection* SelectedActors = GEditor->GetSelectedActors();
	return SelectedActors->Num() > 0;
}

FText SDynamicObjectManagerWidget::AssignDynamicTooltip() const
{
	if (!FCognitiveEditorTools::GetInstance()->HasDeveloperKey())
	{
		return FText::FromString("Developer Key is not set");
	}
	return FText::FromString("Add Dynamic Object Components To Selected Actors in Scene");
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
	if (!SceneDisplayName.IsValid()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->SceneHasSceneId(*AdjustedSceneDisplayName)) { return false; }
	
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
	if (!SceneDisplayName.IsValid()) { return FText::FromString("Scene has not been exported!"); }
	if (tools->SceneHasSceneId(*AdjustedSceneDisplayName))
	{
		auto data = tools->GetSceneData(*AdjustedSceneDisplayName);
		//return FText::FromString("Scene: " + data->Name + "   Version: " + FString::FromInt(data->VersionNumber));
		return FText::FromString("Scene Version: " + FString::FromInt(data->VersionNumber));
	}
	return FText::FromString("Scene has not been exported!");
}

EVisibility SDynamicObjectManagerWidget::SceneNotUploadedVisibility() const
{
	if (SceneDisplayName.IsValid())
	{
		if (FCognitiveEditorTools::GetInstance()->SceneHasSceneId(*AdjustedSceneDisplayName))
		{
			return EVisibility::Collapsed;
		}
		return EVisibility::Visible;
	}
	else
	{
		if (FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId())
		{
			return EVisibility::Collapsed;
		}
		return EVisibility::Visible;
	}
}

EVisibility SDynamicObjectManagerWidget::SceneUploadedVisibility() const
{
	if (SceneDisplayName.IsValid())
	{
		if (FCognitiveEditorTools::GetInstance()->SceneHasSceneId(*AdjustedSceneDisplayName))
		{
			return EVisibility::Visible;
		}
		return EVisibility::Collapsed;
	}
	else
	{
		if (FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId())
		{
			return EVisibility::Visible;
		}
		return EVisibility::Collapsed;
	}
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
	FCognitive3DEditorModule::CloseDynamicObjectWindow();
	FCognitive3DEditorModule::SpawnCognitiveSceneSetupTab();

	return FReply::Handled();
}

TSharedRef<SWidget> SDynamicObjectManagerWidget::MakeSceneNamesComboWidget(TSharedPtr<FString> InItem)
{
	FString ProfileMessage = FString("temporary tooltip");

	return
		SNew(STextBlock)
		.Text(FText::FromString(*InItem))
		.ToolTipText(FText::FromString(ProfileMessage))
		.Font(IDetailLayoutBuilder::GetDetailFont());
}

void SDynamicObjectManagerWidget::OnSceneNamesChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	// if it's set from code, we did that on purpose
	//if (SelectInfo != ESelectInfo::Direct)
	{
		SceneDisplayName = NewSelection;
		//AdjustedSceneDisplayName = MakeShareable(new FString(SceneDisplayName->Replace(TEXT("/"), TEXT("_"))));
		AdjustedSceneDisplayName = MakeShareable(new FString(FCognitiveEditorTools::GetInstance()->AdjustPathName(*SceneDisplayName)));
	}
	RefreshDisplayDynamicObjectsCountInScene();
}

void SDynamicObjectManagerWidget::OnSceneNamesComboOpening()
{
	SceneNamesComboList.Empty();
	for (auto sceneData : FCognitiveEditorTools::GetInstance()->GetSceneData())
	{
		SceneNamesComboList.Add(MakeShareable(new FString(sceneData->Path + "/" + sceneData->Name)));
	}

	if (SceneNamesComboList.Num() == 0)
	{
		//no scenes uploaded
		return;
	}


	TSharedPtr<FString> ComboStringPtr = SceneNamesComboList[0];
	if (ComboStringPtr.IsValid())
	{
		SceneNamesComboBox->SetSelectedItem(ComboStringPtr);
		return;
	}
}

FText SDynamicObjectManagerWidget::GetSceneNamesComboBoxContent() const
{
	if (SceneDisplayName.IsValid())
	{
		return FText::FromString(*SceneDisplayName);
	}
	
	return FText::FromString("no scene uploaded");
}