

#include "CognitiveEditorTools.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"



//TSharedRef<FCognitiveEditorTools> ToolsInstance;
FCognitiveEditorTools* FCognitiveEditorTools::CognitiveEditorToolsInstance;
FString FCognitiveEditorTools::Gateway;

FCognitiveEditorTools* FCognitiveEditorTools::GetInstance()
{
	return CognitiveEditorToolsInstance;
}

//GET dynamic object manifest
FString FCognitiveEditorTools::GetDynamicObjectManifest(FString versionid)
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	if (Gateway.Len() == 0)
	{
		Gateway = "data.cognitive3d.com";
	}
	return "https://" + Gateway + "/v0/versions/" + versionid + "/objects";
}

//POST dynamic object manifest
FString FCognitiveEditorTools::PostDynamicObjectManifest(FString sceneid, int32 versionnumber)
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	if (Gateway.Len() == 0)
	{
		Gateway = "data.cognitive3d.com";
	}
	return "https://" + Gateway + "/v0/objects/" + sceneid + "?version=" + FString::FromInt(versionnumber);
}

//POST dynamic object mesh data
FString FCognitiveEditorTools::PostDynamicObjectMeshData(FString sceneid, int32 versionnumber, FString exportdirectory)
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	if (Gateway.Len() == 0)
	{
		Gateway = "data.cognitive3d.com";
	}
	return "https://" + Gateway + "/v0/objects/" + sceneid + "/" + exportdirectory + "?version=" + FString::FromInt(versionnumber);
}

//GET scene settings and read scene version
FString FCognitiveEditorTools::GetSceneVersion(FString sceneid)
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	if (Gateway.Len() == 0)
	{
		Gateway = "data.cognitive3d.com";
	}
	return "https://" + Gateway + "/v0/scenes/" + sceneid;
}

//POST scene screenshot
FString FCognitiveEditorTools::PostScreenshot(FString sceneid, FString versionnumber)
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	if (Gateway.Len() == 0)
	{
		Gateway = "data.cognitive3d.com";
	}
	return "https://" + Gateway + "/v0/scenes/" + sceneid + "/screenshot?version=" + versionnumber;
}

//POST upload decimated scene
FString FCognitiveEditorTools::PostNewScene()
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	if (Gateway.Len() == 0)
	{
		Gateway = "data.cognitive3d.com";
	}
	return "https://" + Gateway + "/v0/scenes";
}

//POST upload and replace existing scene
FString FCognitiveEditorTools::PostUpdateScene(FString sceneid)
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	if (Gateway.Len() == 0)
	{
		Gateway = "data.cognitive3d.com";
	}
	return "https://" + Gateway + "/v0/scenes/" + sceneid;
}

//WEB used to open scenes on sceneexplorer or custom session viewer
FString FCognitiveEditorTools::SceneExplorerOpen(FString sceneid)
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	if (Gateway.Len() == 0)
	{
		Gateway = "data.cognitive3d.com";
	}
	FString split = Gateway.RightChop(5);
	FString url = "https://viewer." + split + "/scene/" + sceneid;
	return url;
}


void FCognitiveEditorTools::Initialize()
{
	CognitiveEditorToolsInstance = new FCognitiveEditorTools;

	//should be able to update gateway while unreal is running, but cache if not in editor since that's nuts
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);

	//should update both editor urls and session data urls
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	CognitiveEditorToolsInstance->ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	FString texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("box_empty.png");
	FName BrushName = FName(*texturepath);
	CognitiveEditorToolsInstance->BoxEmptyIcon = new FSlateDynamicImageBrush(BrushName, FVector2D(20, 20));

	texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("box_check.png");
	BrushName = FName(*texturepath);
	CognitiveEditorToolsInstance->BoxCheckIcon = new FSlateDynamicImageBrush(BrushName, FVector2D(20, 20));
}

void FCognitiveEditorTools::CheckIniConfigured()
{
	GConfig->Flush(true, GEngineIni);
	FString tempGateway;
	GConfig->GetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("Gateway"), tempGateway, GEngineIni);

	if (tempGateway.IsEmpty())
	{
		GLog->Log("FCognitiveEditorTools::CheckIniConfigured write defaults to ini");
		FString defaultgateway = "data.cognitive3d.com";
		FString trueString = "True";
		GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("Gateway"), *defaultgateway, GEngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("GazeBatchSize"), 256, GEngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("CustomEventBatchSize"), 256, GEngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("CustomEventAutoTimer"), 10, GEngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicDataLimit"), 512, GEngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicAutoTimer"), 10, GEngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SensorDataLimit"), 512, GEngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SensorAutoTimer"), 10, GEngineIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), GEngineIni);
		GConfig->SetString(TEXT("AnalyticsDebug"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), GEngineIni);
		GConfig->SetString(TEXT("AnalyticsTest"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), GEngineIni);
		GConfig->SetString(TEXT("AnalyticsDevelopment"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), GEngineIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), GEngineIni);
		GConfig->SetString(TEXT("AnalyticsDebug"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), GEngineIni);
		GConfig->SetString(TEXT("AnalyticsTest"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), GEngineIni);
		GConfig->SetString(TEXT("AnalyticsDevelopment"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), GEngineIni);

		GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("EnableLocalCache"), *trueString, GEngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("LocalCacheSize"), 100, GEngineIni);
		GConfig->Flush(false, GEngineIni);
	}

	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->Flush(true, EngineIni);
	FString tempGatewaydefault;
	GConfig->GetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("Gateway"), tempGatewaydefault, EngineIni);

	if (tempGatewaydefault.IsEmpty())
	{
		GLog->Log("FCognitiveEditorTools::CheckIniConfigured write defaults to ini");
		FString defaultgateway = "data.cognitive3d.com";
		FString trueString = "True";
		GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("Gateway"), *defaultgateway, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("GazeBatchSize"), 256, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("CustomEventBatchSize"), 256, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("CustomEventAutoTimer"), 10, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicDataLimit"), 512, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicAutoTimer"), 10, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SensorDataLimit"), 512, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SensorAutoTimer"), 10, EngineIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsDebug"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsTest"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsDevelopment"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsDebug"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsTest"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsDevelopment"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);

		GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("EnableLocalCache"), *trueString, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("LocalCacheSize"), 100, EngineIni);
		GConfig->Flush(false, EngineIni);
	}
}

//at any step in the uploading process
bool WizardUploading = false;

bool FCognitiveEditorTools::IsWizardUploading()
{
	return WizardUploading;
}


TArray<TSharedPtr<FDynamicData>> SceneExplorerDynamics;
TArray<TSharedPtr<FString>> SubDirectoryNames;

//deals with all the exporting and non-display stuff in the editor preferences

TArray<TSharedPtr<FDynamicData>> FCognitiveEditorTools::GetSceneDynamics()
{
	return SceneDynamics;
}

bool FCognitiveEditorTools::HasDeveloperKey() const
{
	return DeveloperKey.Len() > 0;
}

bool FCognitiveEditorTools::HasApplicationKey() const
{
	return ApplicationKey.Len() > 0;
}

FProcHandle FCognitiveEditorTools::ExportNewDynamics()
{
	FProcHandle fph;
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorToolsCustomization::ExportDynamics world is null"));
		return fph;
	}

	if (BaseExportDirectory.Len() == 0)
	{
		GLog->Log("base directory not selected");
		return fph;
	}

	TArray<FString> meshNames;
	TArray<UDynamicObject*> exportObjects;

	//get all dynamic object components in scene. add names/pointers to array
	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		for (UActorComponent* actorComponent : ActorItr->GetComponents())
		{
			if (actorComponent->IsA(UDynamicObject::StaticClass()))
			{
				UDynamicObject* dynamicComponent = Cast<UDynamicObject>(actorComponent);
				if (dynamicComponent == NULL)
				{
					continue;
				}
				FString path = GetDynamicsExportDirectory() + "/" + dynamicComponent->MeshName + "/" + dynamicComponent->MeshName;
				FString gltfpath = path + ".gltf";
				if (FPaths::FileExists(*gltfpath))
				{
					//already exported
					continue;
				}
				if (!meshNames.Contains(dynamicComponent->MeshName))
				{
					exportObjects.Add(dynamicComponent);
					meshNames.Add(dynamicComponent->MeshName);
				}
			}
		}
	}

	if (meshNames.Num() == 0)
	{
		return fph;
	}

	return ExportDynamicObjectArray(exportObjects);
}

FProcHandle FCognitiveEditorTools::ExportAllDynamics()
{
	FProcHandle fph;
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorToolsCustomization::ExportDynamics world is null"));
		return fph;
	}

	if (BaseExportDirectory.Len() == 0)
	{
		GLog->Log("base directory not selected");
		return fph;
	}

	TArray<FString> meshNames;
	TArray<UDynamicObject*> exportObjects;

	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		//AStaticMeshActor *Mesh = *ActorItr;

		for (UActorComponent* actorComponent : ActorItr->GetComponents())
		{
			if (actorComponent->IsA(UDynamicObject::StaticClass()))
			{
				UDynamicObject* dynamicComponent = Cast<UDynamicObject>(actorComponent);
				if (dynamicComponent == NULL)
				{
					continue;
				}
				if (!meshNames.Contains(dynamicComponent->MeshName))
				{
					exportObjects.Add(dynamicComponent);
					meshNames.Add(dynamicComponent->MeshName);
				}
			}
		}
	}

	if (meshNames.Num() == 0)
	{
		return fph;
	}

	return ExportDynamicObjectArray(exportObjects);
}

FReply FCognitiveEditorTools::ExportSelectedDynamics()
{
	TArray<FString> meshNames;
	TArray<UDynamicObject*> SelectionSetCache;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			for (UActorComponent* actorComponent : Actor->GetComponents())
			{
				if (actorComponent->IsA(UDynamicObject::StaticClass()))
				{
					UDynamicObject* dynamicComponent = Cast<UDynamicObject>(actorComponent);
					if (dynamicComponent == NULL)
					{
						continue;
					}
					if (!meshNames.Contains(dynamicComponent->MeshName))
					{
						SelectionSetCache.Add(dynamicComponent);
						meshNames.Add(dynamicComponent->MeshName);
					}
				}
			}

		}
	}

	ExportDynamicObjectArray(SelectionSetCache);

	return FReply::Handled();
}

FProcHandle FCognitiveEditorTools::ExportDynamicData(TArray< TSharedPtr<FDynamicData>> dynamicData)
{
	//find all meshes in scene that are contained in the dynamicData list

	TArray<FString> meshNames;
	TArray<UDynamicObject*> SelectionSetCache;
	for (TActorIterator<AActor> It(GWorld); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			for (UActorComponent* actorComponent : Actor->GetComponents())
			{
				if (actorComponent->IsA(UDynamicObject::StaticClass()))
				{
					UDynamicObject* dynamicComponent = Cast<UDynamicObject>(actorComponent);
					if (dynamicComponent == NULL)
					{
						continue;
					}
					if (meshNames.Contains(dynamicComponent->MeshName))
					{
						continue;
					}
					bool exportActor = false;
					for (auto& elem : dynamicData)
					{
						if (elem->MeshName == dynamicComponent->MeshName)
						{
							exportActor = true;
							break;
						}
					}
					if (exportActor)
					{
						SelectionSetCache.Add(dynamicComponent);
						meshNames.Add(dynamicComponent->MeshName);
					}
				}
			}
		}
	}

	return ExportDynamicObjectArray(SelectionSetCache);

	//return FReply::Handled();
}

FProcHandle FCognitiveEditorTools::ExportDynamicObjectArray(TArray<UDynamicObject*> exportObjects)
{
	FProcHandle fph;

	FVector originalLocation;
	FRotator originalRotation;
	FVector originalScale;
	int32 ActorsExported = 0;

	TArray<FString> DynamicMeshNames;

	//used to check that export popup was confirmed, rather than canceled
	int32 RawExportFilesFound = 0;

	for (int32 i = 0; i < exportObjects.Num(); i++)
	{
		GEditor->SelectNone(false, true, false);

		if (exportObjects[i] == NULL)
		{
			continue;
		}
		if (exportObjects[i]->GetOwner() == NULL)
		{
			continue;
		}
		if (exportObjects[i]->MeshName.IsEmpty())
		{
			continue;
		}

		DynamicMeshNames.Add(exportObjects[i]->MeshName);

		originalLocation = exportObjects[i]->GetOwner()->GetActorLocation();
		originalRotation = exportObjects[i]->GetOwner()->GetActorRotation();
		originalScale = exportObjects[i]->GetOwner()->GetActorScale();

		exportObjects[i]->GetOwner()->SetActorLocation(FVector::ZeroVector);
		exportObjects[i]->GetOwner()->SetActorRotation(FQuat::Identity);
		exportObjects[i]->GetOwner()->SetActorScale3D(FVector::OneVector);

		FString justDirectory = GetDynamicsExportDirectory() + "/" + exportObjects[i]->MeshName;
		FString tempObject;
		FString ExportFilename;

		FSuppressableWarningDialog::FSetupInfo ExportSettingsInfo(LOCTEXT("ExportSettingsBody", "Make sure the scale is set to 1.0 to get the correct representation on the Dashboard"), LOCTEXT("ExportSettingsTitle", "Recommended Export Settings"), "ExportSelectedDynamicsBody");
		ExportSettingsInfo.ConfirmText = LOCTEXT("Ok", "Ok");
		ExportSettingsInfo.CheckBoxText = FText::FromString("Don't show again");
		FSuppressableWarningDialog ExportSelectedDynamicMeshes(ExportSettingsInfo);
		ExportSelectedDynamicMeshes.ShowModal();
		ExportFilename = exportObjects[i]->MeshName + ".gltf";
		tempObject = GetDynamicsExportDirectory() + "/" + exportObjects[i]->MeshName + "/" + exportObjects[i]->MeshName + ".gltf";
		
		//create directory before export
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// Dynamic Directory Exists?
		if (!PlatformFile.DirectoryExists(*GetDynamicsExportDirectory()))
		{
			PlatformFile.CreateDirectory(*GetDynamicsExportDirectory());
		}

		// Object Directory Exists?
		if (!PlatformFile.DirectoryExists(*justDirectory))
		{
			PlatformFile.CreateDirectory(*justDirectory);
		}


		GEditor->SelectActor(exportObjects[i]->GetOwner(), true, false, true);
		ActorsExported++;
		GLog->Log("FCognitiveEditorTools::ExportDynamicObjectArray dynamic output directory " + tempObject);



		int DynamicObjectCount = 0;
		//check if this D.O. is on an object with another D.O.
		for (UActorComponent* Component : exportObjects[i]->GetOwner()->GetComponents())
		{
			if (Component->IsA(UDynamicObject::StaticClass()))
			{
				DynamicObjectCount++;
			}
		}

		if (DynamicObjectCount > 1) //multiple D.O.s
		{

			UE_LOG(LogTemp, Warning, TEXT("FOUND MULTIPLE DYNAMIC OBJECTS"));

			//if so, we check that each D.O. has a mesh parents
			UMeshComponent* ParentMesh = Cast<UMeshComponent>(exportObjects[i]->GetAttachParent());

			if (ParentMesh)
			{
				// Check for Static Mesh Component
				UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(ParentMesh);
				if (StaticMeshComp && StaticMeshComp->GetStaticMesh())
				{
					UStaticMesh* StaticMeshAsset = StaticMeshComp->GetStaticMesh();
					// Do something with StaticMeshAsset
					UExporter::ExportToFile(StaticMeshAsset, NULL, *tempObject, true);
				}

				// Check for Skeletal Mesh Component
				USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(ParentMesh);
				if (SkeletalMeshComp && SkeletalMeshComp->SkeletalMesh)
				{
					USkeletalMesh* SkeletalMeshAsset = SkeletalMeshComp->SkeletalMesh;
					// Do something with SkeletalMeshAsset
					UExporter::ExportToFile(SkeletalMeshAsset, NULL, *tempObject, true);
				}

			}
			else //incorrect configuration of multiple D.O.s
			{
				FSuppressableWarningDialog::FSetupInfo DOExportSettingsInfo(LOCTEXT("DynamicObjectExportSettingsBody", "You are exporting a dynamic object on an actor with multiple dynamic objects, but they are not configured correctly. If you want to track multiple meshes on an actor with multiple dynamic objects, make sure each dynamic object is a child of its respective mesh"), LOCTEXT("DynamicObjectExportSettingsTitle", "Incorrect Dynamic Object Export Configuration"), "ExportSelectedDynamicsObjectsBody");
				DOExportSettingsInfo.ConfirmText = LOCTEXT("Ok", "Ok");
				DOExportSettingsInfo.CheckBoxText = FText::FromString("Don't show again");
				FSuppressableWarningDialog DOExportSelectedDynamicMeshes(DOExportSettingsInfo);
				DOExportSelectedDynamicMeshes.ShowModal();

				continue;
			}
		}
		else //only 1 D.O. proceed as normal
		{
			//if theres only 1 D.O. on the object, continue normally

			//pawn export process needs to be able to group meshes somehow on D.O.s with 1 D.O. and multiple meshes
			if (exportObjects[i]->GetOwner()->IsA(APawn::StaticClass()))
			{
				UE_LOG(LogTemp, Warning, TEXT("FOUND PAWN CLASS, EXPORTING MESH"));
				UMeshComponent* pawnMesh = Cast<UMeshComponent>(exportObjects[i]->GetOwner()->GetComponentByClass(UMeshComponent::StaticClass()));
				TArray<UActorComponent*> pawnMeshes;
				exportObjects[i]->GetOwner()->GetComponents(UMeshComponent::StaticClass(), pawnMeshes);

				TArray< UMeshComponent*> meshes;
				for (int32 j = 0; j < pawnMeshes.Num(); j++)
				{
					UStaticMeshComponent* mesh = Cast<UStaticMeshComponent>(pawnMeshes[j]);
					if (mesh == NULL)
					{
						continue;
					}

					ULevel* componentLevel = pawnMeshes[j]->GetComponentLevel();
					if (componentLevel->bIsVisible == 0)
					{
						continue;
						//not visible! possibly on a disabled sublevel
					}

					meshes.Add(mesh);
				}

				TArray<UActorComponent*> actorSkeletalComponents;
				exportObjects[i]->GetOwner()->GetComponents(USkeletalMeshComponent::StaticClass(), actorSkeletalComponents);
				for (int32 j = 0; j < actorSkeletalComponents.Num(); j++)
				{
					USkeletalMeshComponent* mesh = Cast<USkeletalMeshComponent>(actorSkeletalComponents[j]);
					if (mesh == NULL)
					{
						continue;
					}

					ULevel* componentLevel = actorSkeletalComponents[j]->GetComponentLevel();
					if (componentLevel->bIsVisible == 0)
					{
						continue;
						//not visible! possibly on a disabled sublevel
					}

					meshes.Add(mesh);
				}

				//setup temp meshes package
				UPackage* NewPackageName = CreatePackage(TEXT("/Game/TempMesh"));

				if (meshes.Num() > 0)
				{
					//take the skeletal meshes that we set up earlier and use them to create a static mesh
					UStaticMesh* tmpStatMesh = MeshUtilities.ConvertMeshesToStaticMesh(meshes, exportObjects[i]->GetOwner()->GetTransform(), NewPackageName->GetName());
					UExporter::ExportToFile(tmpStatMesh, NULL, *tempObject, true);
					TempAssetsToDelete.Add(tmpStatMesh);
				}
			}
			else
			{
				//exports the currently selected actor(s)
				UE_LOG(LogTemp, Warning, TEXT("DOING REGULAR MAP EXPORT"));
				GUnrealEd->ExportMap(GWorld, *tempObject, true);
			}
		}


		//reset dynamic back to original transform
		exportObjects[i]->GetOwner()->SetActorLocation(originalLocation);
		exportObjects[i]->GetOwner()->SetActorRotation(originalRotation);
		exportObjects[i]->GetOwner()->SetActorScale3D(originalScale);

		//check that files were actually exported
		if (PlatformFile.FileExists(*tempObject))
		{
			RawExportFilesFound++;
		}
		else
		{
			//don't export screenshots or save materials unless export popup was confirmed
			continue;
		}

		//automatic screenshot
		FLevelEditorViewportClient* perspectiveView = NULL;

		for (int32 j = 0; j < GEditor->GetLevelViewportClients().Num(); j++)
		{
			if (GEditor->GetLevelViewportClients()[j]->ViewportType == LVT_Perspective)
			{
				perspectiveView = GEditor->GetLevelViewportClients()[j];
				break;
			}
		}
		if (perspectiveView != NULL)
		{
			FVector startPosition = perspectiveView->GetViewLocation();
			FRotator startRotation = perspectiveView->GetViewRotation();
			FString dir = BaseExportDirectory + "/dynamics/" + exportObjects[i]->MeshName + "/";
			FTimerHandle DelayScreenshotHandle;

			//calc position
			FVector origin;
			FVector extents;
			exportObjects[i]->GetOwner()->GetActorBounds(false, origin, extents);

			float radius = extents.Size();
			FVector calculatedPosition = exportObjects[i]->GetComponentLocation() + (FVector(-1, -1, 1) * radius);
			FVector calcDir = exportObjects[i]->GetComponentLocation() - calculatedPosition;

			FRotator calcRot = FRotator(calcDir.ToOrientationQuat());

			perspectiveView->SetViewLocation(calculatedPosition);
			perspectiveView->SetViewRotation(calcRot);

			perspectiveView->bNeedsRedraw = true;
			perspectiveView->Viewport->Draw(false);
			FCognitiveEditorTools::DelayScreenshot(dir, perspectiveView, startPosition, startRotation);
		}
	}

	if (RawExportFilesFound == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("FCognitiveEditorTools::ExportDynamicObjectArray Cancelled mesh export"));
		return fph;
	}

	if (ActorsExported > 0)
	{
		GLog->Log("FCognitiveEditorTools::ExportDynamicObjectArray Found " + FString::FromInt(ActorsExported) + " meshes for export");
		FindAllSubDirectoryNames();
	}

	ObjectTools::DeleteAssets(TempAssetsToDelete, false);
	TempAssetsToDelete.Empty();
	return fph;
}


void FCognitiveEditorTools::DelayScreenshot(FString filePath, FLevelEditorViewportClient* perspectiveView, FVector startPos, FRotator startRot)
{
	UThumbnailManager::CaptureProjectThumbnail(perspectiveView->Viewport, filePath + "cvr_object_thumbnail.png", false);

	perspectiveView->SetViewLocation(startPos);
	perspectiveView->SetViewRotation(startRot);
	perspectiveView->bNeedsRedraw = true;
	perspectiveView->RedrawRequested(perspectiveView->Viewport);
}

FReply FCognitiveEditorTools::SetUniqueDynamicIds()
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

	//int32 currentUniqueId = 1;
	int32 changedDynamics = 0;

	//unassigned or invalid numbers
	TArray<UDynamicObject*> UnassignedDynamics;

	//try to put all ids back where they were
	for (auto& dynamic : dynamics)
	{
		//id dynamic custom id is not in usedids - add it

		if (dynamic->MeshName.IsEmpty())
		{
			dynamic->TryGenerateMeshName();
		}
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
		dynamic->CustomId = FGuid::NewGuid().ToString();
		dynamic->IdSourceType = EIdSourceType::CustomId;
		usedIds.Add(FDynamicObjectId(dynamic->CustomId, dynamic->MeshName));
		changedDynamics++;
	}

	GLog->Log("CognitiveVR Tools set " + FString::FromInt(changedDynamics) + " dynamic ids");

	GWorld->MarkPackageDirty();
	//save the scene? mark the scene as changed?

	return FReply::Handled();
}

FReply FCognitiveEditorTools::UploadDynamicsManifest()
{
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

	GLog->Log("CognitiveVR Tools uploading manifest for " + FString::FromInt(dynamics.Num()) + " objects");

	UploadSelectedDynamicsManifest(dynamics);

	return FReply::Handled();
}

FReply FCognitiveEditorTools::UploadSelectedDynamicsManifest(TArray<UDynamicObject*> dynamics)
{
	bool wroteAnyObjects = false;
	FString objectManifest = "{\"objects\":[";
	//write preset customids to manifest
	for (int32 i = 0; i < dynamics.Num(); i++)
	{
		//if they have a customid -> add them to the objectmanifest string
		if (dynamics[i]->IdSourceType == EIdSourceType::CustomId && dynamics[i]->CustomId != "")
		{
			FVector location = dynamics[i]->GetComponentLocation();
			FQuat rotation = dynamics[i]->GetComponentQuat();
			FVector scale = dynamics[i]->GetComponentScale();

			wroteAnyObjects = true;
			objectManifest += "{";
			objectManifest += "\"id\":\"" + dynamics[i]->CustomId + "\",";
			objectManifest += "\"mesh\":\"" + dynamics[i]->MeshName + "\",";
			objectManifest += "\"name\":\"" + dynamics[i]->GetOwner()->GetName() + "\",";
			objectManifest += "\"scaleCustom\":[" + FString::SanitizeFloat(scale.X) + "," + FString::SanitizeFloat(scale.Z) + "," + FString::SanitizeFloat(scale.Y) + "],";
			objectManifest += "\"initialPosition\":[" + FString::SanitizeFloat(-location.X) + "," + FString::SanitizeFloat(location.Z) + "," + FString::SanitizeFloat(location.Y) + "],";
			objectManifest += "\"initialRotation\":[" + FString::SanitizeFloat(-rotation.X) + "," + FString::SanitizeFloat(rotation.Z) + "," + FString::SanitizeFloat(-rotation.Y) + "," + FString::SanitizeFloat(rotation.W) + "]";
			objectManifest += "},";
		}
	}
	if (!wroteAnyObjects)
	{
		GLog->Log("Couldn't find any dynamic objects to put into the aggregation manifest!");
		FCognitiveEditorTools::OnUploadManifestCompleted(NULL, NULL, true);
		return FReply::Handled();
	}
	//remove last comma
	objectManifest.RemoveFromEnd(",");
	//add ]}
	objectManifest += "]}";


	//get scene id
	TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();
	if (!currentSceneData.IsValid())
	{
		GLog->Log("FCognitiveEditorTools::UploadDynamicObjectManifest could not find current scene id");
		return FReply::Handled();
	}

	if (currentSceneData->Id == "")
	{
		GLog->Log("CognitiveToolsCustomization::UploadDynamicsManifest couldn't find sceneid for current scene");
		return FReply::Handled();
	}
	if (currentSceneData->VersionNumber == 0)
	{
		GLog->Log("CognitiveTools::UploadDynamicsManifest current scene does not have valid version number. GetSceneVersions and try again");
		return FReply::Handled();
	}

	FString url = PostDynamicObjectManifest(currentSceneData->Id, currentSceneData->VersionNumber);

	//send manifest to api/objects/sceneid

	GLog->Log("CognitiveTools::UploadDynamicsManifest send dynamic object aggregation manifest");
	FString AuthValue = "APIKEY:DEVELOPER " + DeveloperKey;
	auto HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetContentAsString(objectManifest);
	GLog->Log(objectManifest);

	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FCognitiveEditorTools::OnUploadManifestCompleted);

	HttpRequest->ProcessRequest();

	return FReply::Handled();
}

FReply FCognitiveEditorTools::UploadDynamicsManifestIds(TArray<FString> ids, FString meshName, FString prefabName)
{
	//this is used by dynamic object id pool. doesn't have initial positions, rotations, scale
	GLog->Log("CognitiveVR Tools uploading manifest for " + FString::FromInt(ids.Num()) + " objects");

	bool wroteAnyObjects = false;
	FString objectManifest = "{\"objects\":[";
	//write preset customids to manifest
	for (int32 i = 0; i < ids.Num(); i++)
	{
		//if they have a customid -> add them to the objectmanifest string
		if (ids[i] != "")
		{
			wroteAnyObjects = true;
			objectManifest += "{";
			objectManifest += "\"id\":\"" + ids[i] + "\",";
			objectManifest += "\"mesh\":\"" + meshName + "\",";
			objectManifest += "\"name\":\"" + prefabName + "\",";

			objectManifest += "\"scaleCustom\":[1,1,1],";
			objectManifest += "\"initialPosition\":[0,0,0],";
			objectManifest += "\"initialRotation\":[0,0,0,1]";
			objectManifest += "},";
		}
	}
	if (!wroteAnyObjects)
	{
		GLog->Log("Couldn't find any dynamic objects to put into the aggregation manifest!");
		return FReply::Handled();
	}
	//remove last comma
	objectManifest.RemoveFromEnd(",");
	//add ]}
	objectManifest += "]}";


	//get scene id
	TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();
	if (!currentSceneData.IsValid())
	{
		GLog->Log("FCognitiveEditorTools::UploadDynamicObjectManifest could not find current scene id");
		return FReply::Handled();
	}

	if (currentSceneData->Id == "")
	{
		GLog->Log("CognitiveToolsCustomization::UploadDynamicsManifest couldn't find sceneid for current scene");
		return FReply::Handled();
	}
	if (currentSceneData->VersionNumber == 0)
	{
		GLog->Log("CognitiveTools::UploadDynamicsManifest current scene does not have valid version number. GetSceneVersions and try again");
		return FReply::Handled();
	}

	FString url = PostDynamicObjectManifest(currentSceneData->Id, currentSceneData->VersionNumber);

	//send manifest to api/objects/sceneid

	GLog->Log("CognitiveTools::UploadDynamicsManifest send dynamic object aggregation manifest");
	FString AuthValue = "APIKEY:DEVELOPER " + DeveloperKey;
	auto HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetContentAsString(objectManifest);

	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FCognitiveEditorTools::OnUploadManifestCompleted);

	HttpRequest->ProcessRequest();

	return FReply::Handled();
}

void FCognitiveEditorTools::OnUploadManifestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	WizardUploading = false;
	if (Response.Get() == NULL) //likely no aggregation manifest to upload. no request, no response
	{
		GetDynamicsManifest();
		return;
	}

	if (bWasSuccessful && Response->GetResponseCode() < 300) //successfully uploaded
	{
		GetDynamicsManifest();
		WizardUploadError = FString::FromInt(Response->GetResponseCode());
		WizardUploadResponseCode = Response->GetResponseCode();

		FNotificationInfo NotifyInfo(FText::FromString(TEXT("Successfully Uploaded Ids for Aggregation")));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
	else //upload failed
	{
		WizardUploadError = FString("FCognitiveEditorTools::OnUploadManifestCompleted response code ") + FString::FromInt(Response->GetResponseCode());
		WizardUploadResponseCode = Response->GetResponseCode();
		GLog->Log("FCognitiveEditorTools::OnUploadManifestCompleted response code " + FString::FromInt(Response->GetResponseCode()));
	}
}

FReply FCognitiveEditorTools::GetDynamicsManifest()
{
	TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();
	if (!currentSceneData.IsValid())
	{
		GLog->Log("CognitiveTools::GetDyanmicManifest could not find current scene data");
		return FReply::Handled();
	}
	if (currentSceneData->VersionId == 0)
	{
		GLog->Log("CognitiveTools::GetDyanmicManifest version id is not set! Makes sure the scene has updated scene version");
		return FReply::Handled();
	}
	if (!HasDeveloperKey())
	{
		GLog->Log("CognitiveTools::GetDyanmicManifest auth token is empty. Must log in!");
		return FReply::Handled();
	}

	auto HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(GetDynamicObjectManifest(FString::FromInt(currentSceneData->VersionId)));

	HttpRequest->SetHeader("X-HTTP-Method-Override", TEXT("GET"));
	FString AuthValue = "APIKEY:DEVELOPER " + DeveloperKey;
	HttpRequest->SetHeader("Authorization", AuthValue);

	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FCognitiveEditorTools::OnDynamicManifestResponse);
	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

void FCognitiveEditorTools::OnDynamicManifestResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GLog->Log("FCognitiveEditorTools::OnDynamicManifestResponse response code " + FString::FromInt(Response->GetResponseCode()));
		
		FNotificationInfo NotifyInfo(FText::FromString(TEXT("FCognitiveEditorTools::OnDynamicManifestResponse response code " + FString::FromInt(Response->GetResponseCode()))));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
	else
	{
		GLog->Log("FCognitiveEditorTools::OnDynamicManifestResponse failed to connect");
		WizardUploading = false;
		WizardUploadError = "FCognitiveEditorTools::OnDynamicManifestResponse failed to connect";
		WizardUploadResponseCode = 0;
		return;
	}

	WizardUploadResponseCode = Response->GetResponseCode();

	if (bWasSuccessful && Response->GetResponseCode() < 300)
	{
		//GLog->Log("CognitiveTools::OnDynamicManifestResponse content: " + Response->GetContentAsString());

		//do json stuff to this

		TSharedPtr<FJsonValue> JsonDynamics;

		TSharedRef<TJsonReader<>>Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonDynamics))
		{
			int32 count = JsonDynamics->AsArray().Num();
			GLog->Log("FCognitiveEditorTools::OnDynamicManifestResponse returned " + FString::FromInt(count) + " objects");
			for (int32 i = 0; i < count; i++)
			{
				TSharedPtr<FJsonObject> jsonobject = JsonDynamics->AsArray()[i]->AsObject();
				FString name = jsonobject->GetStringField("name");
				FString meshname = jsonobject->GetStringField("meshName");
				FString id = jsonobject->GetStringField("sdkId");

				SceneExplorerDynamics.Add(MakeShareable(new FDynamicData(name, meshname, id)));
			}
		}
	}
	else
	{
		WizardUploadError = "FCognitiveEditorTools::OnDynamicManifestResponse response code " + FString::FromInt(Response->GetResponseCode());
	}
	if (WizardUploading)
	{
		WizardUploading = false;
	}
}
int32 OutstandingDynamicUploadRequests = 0;

FReply FCognitiveEditorTools::UploadDynamics()
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");

	// Get all files in directory
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	TArray<FString> DirectoriesToSkip;
	TArray<FString> DirectoriesToNotRecurse;

	// use the timestamp grabbing visitor (include directories)
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToNotRecurse, true);
	Visitor.Visit(*GetDynamicsExportDirectory(), true);

	TArray<FString> dynamicNames;
	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		if (TimestampIt.Key() == GetDynamicsExportDirectory() || (!TimestampIt.Key().EndsWith(".png")))
		{
			GLog->Log("skip file - not dynamic " + TimestampIt.Key());
		}
		else
		{
			GLog->Log("upload dynamic " + FPaths::GetCleanFilename(TimestampIt.Key()));
			dynamicNames.Add(FPaths::GetCleanFilename(TimestampIt.Key()));
		}
	}

	ReadSceneDataFromFile();

	GLog->Log("FCognitiveEditorTools::UploadDynamics found " + FString::FromInt(dynamicNames.Num()) + " exported dynamic objects");
	TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();

	if (!currentSceneData.IsValid())
	{
		GLog->Log("FCognitiveEditorToolsCustomization::UploadDynamics can't find current scene!");
		return FReply::Handled();
	}

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString filePath = TimestampIt.Key();
		const FString fileName = FPaths::GetCleanFilename(filePath);

		if (GetDynamicsExportDirectory() == filePath)
		{
			//GLog->Log("root found " + filePath);
		}
		else if (FPaths::DirectoryExists(filePath))
		{
			//GLog->Log("directory found " + filePath);
			FString url = PostDynamicObjectMeshData(currentSceneData->Id, currentSceneData->VersionNumber, fileName);
			GLog->Log("dynamic upload to url " + url);

			UploadFromDirectory(url, filePath, "object");
			OutstandingDynamicUploadRequests++;
		}
		else
		{
			//GLog->Log("file found " + filePath);
		}
	}

	if (OutstandingDynamicUploadRequests == 0 && WizardUploading)
	{
		GLog->Log("FCognitiveEditorTools::UploadDynamics has no dynamics to upload!");
		WizardUploading = false;
	}

	OnUploadSceneGeometry.ExecuteIfBound(nullptr, nullptr, true);

	return FReply::Handled();
}

FReply FCognitiveEditorTools::UploadDynamic(FString directory)
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");

	// Get all files in directory
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	TArray<FString> DirectoriesToSkip;
	TArray<FString> DirectoriesToNotRecurse;

	FString singleDir = GetDynamicsExportDirectory() + "/" + directory;

	// use the timestamp grabbing visitor (include directories)
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToNotRecurse, true);
	Visitor.Visit(*singleDir, true);

	TArray<FString> dynamicNames;
	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		if (TimestampIt.Key().EndsWith(".png"))
		{
			GLog->Log("upload dynamic " + FPaths::GetCleanFilename(TimestampIt.Key()));
			dynamicNames.Add(FPaths::GetCleanFilename(TimestampIt.Key()));
		}
		else
		{
			GLog->Log("skip file - not dynamic " + TimestampIt.Key());
		}
	}

	ReadSceneDataFromFile();

	GLog->Log("FCognitiveEditorTools::UploadDynamics found " + FString::FromInt(dynamicNames.Num()) + " exported dynamic objects");
	TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();

	if (!currentSceneData.IsValid())
	{
		GLog->Log("FCognitiveEditorToolsCustomization::UploadDynamics can't find current scene!");
		return FReply::Handled();
	}

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString filePath = TimestampIt.Key();
		const FString fileName = FPaths::GetCleanFilename(filePath);

		if (GetDynamicsExportDirectory() == filePath)
		{
			GLog->Log("root found " + filePath);
		}
		else if (FPaths::DirectoryExists(filePath))
		{
			//GLog->Log("directory found " + filePath);
			FString url = PostDynamicObjectMeshData(currentSceneData->Id, currentSceneData->VersionNumber, fileName);
			GLog->Log("dynamic upload to url " + url);

			UploadFromDirectory(url, filePath, "object");
			OutstandingDynamicUploadRequests++;
		}
		else
		{
			//GLog->Log("file found " + filePath);
		}
	}

	if (OutstandingDynamicUploadRequests == 0)
	{
		GLog->Log("FCognitiveEditorTools::UploadDynamics has no dynamics to upload!");
	}
	else
	{
		FNotificationInfo NotifyInfo(FText::FromString(TEXT("Mesh Uploaded")));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
		GLog->Log("FCognitiveEditorTools::UploadDynamics uploaded a Mesh");
	}

	return FReply::Handled();
}

TArray<TSharedPtr<FString>> FCognitiveEditorTools::GetSubDirectoryNames()
{
	return SubDirectoryNames;
}

void FCognitiveEditorTools::FindAllSubDirectoryNames()
{
	// Get all files in directory
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	TArray<FString> DirectoriesToSkip;
	TArray<FString> DirectoriesToNotRecurse;

	// use the timestamp grabbing visitor (include directories)
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToNotRecurse, true);
	Visitor.Visit(*GetDynamicsExportDirectory(), true);

	//no matches anywhere
	SubDirectoryNames.Empty();

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		if (PlatformFile.DirectoryExists(*TimestampIt.Key()) && TimestampIt.Key().Contains("dynamics") && !TimestampIt.Key().EndsWith("dynamics"))
		{
			SubDirectoryNames.Add(MakeShareable(new FString(FPaths::GetCleanFilename(TimestampIt.Key()))));
		}
	}
}

void FCognitiveEditorTools::CreateExportFolderStructure()
{
	VerifyOrCreateDirectory(BaseExportDirectory);
	FString temp = GetDynamicsExportDirectory();
	VerifyOrCreateDirectory(temp);
}

FReply FCognitiveEditorTools::SelectBaseExportDirectory()
{
	FString title = "Select Export Directory";
	FString fileTypes = ".exe";
	FString lastPath = FPaths::ProjectDir();
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		BaseExportDirectory = outFilename;
		FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));
		GConfig->SetString(TEXT("Analytics"), TEXT("ExportPath"), *FCognitiveEditorTools::GetInstance()->BaseExportDirectory, EditorIni);
	}
	else
	{
		BaseExportDirectory = "";
		FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));
		GConfig->SetString(TEXT("Analytics"), TEXT("ExportPath"), *FCognitiveEditorTools::GetInstance()->BaseExportDirectory, EditorIni);
	}
	return FReply::Handled();
}


FReply FCognitiveEditorTools::SaveScreenshotToFile()
{
	FString dir = BaseExportDirectory + "/" + GetCurrentSceneName() + "/screenshot/";
	if (VerifyOrCreateDirectory(dir))
	{
		FScreenshotRequest::RequestScreenshot(dir + "screenshot", false, false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FCognitiveEditorTools::TakeScreenshot unable create directory for screenshot"));
	}
	return FReply::Handled();
}

FReply FCognitiveEditorTools::TakeDynamicScreenshot(FString dynamicName)
{
	FString dir = BaseExportDirectory + "/dynamics/" + dynamicName + "/";
	if (VerifyOrCreateDirectory(dir))
	{
		FScreenshotRequest::RequestScreenshot(dir + "cvr_object_thumbnail", false, false);
	}
	return FReply::Handled();
}

void FCognitiveEditorTools::OnUploadScreenshotCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
		GLog->Log("FCognitiveEditorTools::OnUploadScreenshotCompleted response code " + FString::FromInt(Response->GetResponseCode()));
	else
	{
		GLog->Log("FCognitiveEditorTools::OnUploadScreenshotCompleted failed to connect");
		return;
	}
}

bool FCognitiveEditorTools::PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
{
	OutFilename = FString();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool directoryChosen = false;
	TArray<FString> OutFilenames;
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = ChooseParentWindowHandle();

		directoryChosen = DesktopPlatform->OpenDirectoryDialog(
			ParentWindowWindowHandle,
			Title,
			InOutLastPath,
			OutFilename
		);
	}

	return directoryChosen;
}

void* FCognitiveEditorTools::ChooseParentWindowHandle()
{
	void* ParentWindowWindowHandle = NULL;
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
	if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
	{
		ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
	}

	return ParentWindowWindowHandle;
}

FReply FCognitiveEditorTools::UploadScene()
{
	FString url = "";

	//get scene name
	//look if scene name has an entry in the scene datas
	TSharedPtr<FEditorSceneData> sceneData = GetCurrentSceneData();
	if (sceneData.IsValid() && sceneData->Id.Len() > 0)
	{
		//GLog->Log("post update existing scene");
		//existing uploaded scene
		url = PostUpdateScene(sceneData->Id);
	}
	else
	{
		//GLog->Log("post new scene");
		//new scene
		url = PostNewScene();
	}

	GLog->Log("FCognitiveEditorTools::UploadScene upload scene to " + url);
	UploadFromDirectory(url, GetCurrentSceneExportDirectory(), "scene");
	//IMPROVEMENT listen for response. when the response returns, request the scene version with auth token

	return FReply::Handled();
}

void FCognitiveEditorTools::RefreshSceneUploadFiles()
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(GetCurrentSceneExportDirectory(), true, filesStartingWith, filesStartingWith, pngextension, false);

	TArray<FString> imagesInDirectory = GetAllFilesInDirectory(GetCurrentSceneExportDirectory(), true, filesStartingWith, pngextension, filesStartingWith, false);
	imagesInDirectory.Remove(GetCurrentSceneExportDirectory() + "/screenshot/screenshot.png");

	SceneUploadFiles.Empty();
	for (int32 i = 0; i < filesInDirectory.Num(); i++)
	{
		SceneUploadFiles.Add(MakeShareable(new FString(filesInDirectory[i])));
	}
	for (int32 i = 0; i < imagesInDirectory.Num(); i++)
	{
		SceneUploadFiles.Add(MakeShareable(new FString(imagesInDirectory[i])));
	}
}

int32 FCognitiveEditorTools::GetSceneExportFileCount()
{
	return SceneUploadFiles.Num();
}

void FCognitiveEditorTools::RefreshDynamicUploadFiles()
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(BaseExportDirectory + "/dynamics", true, filesStartingWith, filesStartingWith, pngextension, false);

	TArray<FString> imagesInDirectory = GetAllFilesInDirectory(BaseExportDirectory + "/dynamics", true, filesStartingWith, pngextension, filesStartingWith, false);

	DynamicUploadFiles.Empty();
	for (int32 i = 0; i < filesInDirectory.Num(); i++)
	{
		DynamicUploadFiles.Add(MakeShareable(new FString(filesInDirectory[i])));
	}
	for (int32 i = 0; i < imagesInDirectory.Num(); i++)
	{
		DynamicUploadFiles.Add(MakeShareable(new FString(imagesInDirectory[i])));
	}
}

TArray<FString> FCognitiveEditorTools::GetValidDirectories(const FString& Directory)
{
	TArray<FString> FoundFolders;
	if (FPaths::DirectoryExists(Directory))
	{
		FFileManagerGeneric::Get().FindFilesRecursive(FoundFolders, *Directory, TEXT("*"), false, true, true);
		for (int i = 0; i < FoundFolders.Num(); i++)
		{
			FoundFolders[i] = Directory + FoundFolders[i];
		}
	}
	return FoundFolders;
}

int32 FCognitiveEditorTools::GetDynamicObjectFileExportedCount()
{
	return DynamicUploadFiles.Num();
}

int32 FCognitiveEditorTools::GetDynamicObjectExportedCount()
{
	auto folders = GetValidDirectories(BaseExportDirectory + "/dynamics");
	return folders.Num();
}

void FCognitiveEditorTools::UploadFromDirectory(FString url, FString directory, FString expectedResponseType)
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(directory, true, filesStartingWith, filesStartingWith, filesStartingWith, true);

	//TArray<FString> imagesInDirectory = GetAllFilesInDirectory(directory, true, filesStartingWith, pngextension, filesStartingWith, true);

	filesInDirectory.Remove(directory + "/screenshot/screenshot.png");
	//imagesInDirectory.Remove(directory + "/screenshot/screenshot.png");
	FString screenshotPath = directory + "/screenshot/screenshot.png";

	TArray<FContentContainer> contentArray;

	//UE_LOG(LogTemp, Log, TEXT("UploadScene image count%d"), imagesInDirectory.Num());
	UE_LOG(LogTemp, Log, TEXT("UploadScene all file count%d"), filesInDirectory.Num());

	for (int32 i = 0; i < filesInDirectory.Num(); i++)
	{
		FString Content;
		TArray<uint8> byteResult;
		if (FFileHelper::LoadFileToArray(byteResult, *filesInDirectory[i]))
		{
			FContentContainer container = FContentContainer();
			UE_LOG(LogTemp, Log, TEXT("Loaded file %s"), *filesInDirectory[i]);
			//loaded the file

			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("--cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-Type: application/octet-stream");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-disposition: form-data; name=\"file\"; filename=\"" + FPaths::GetCleanFilename(filesInDirectory[i]) + "\"");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append(TEXT("\r\n"));

			container.Headers = Content;
			container.BodyBinary = byteResult;

			contentArray.Add(container);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("failed to load %s"), *filesInDirectory[i]);
		}
	}

	//append screenshot
	FString Content;
	TArray<uint8> byteResult;
	if (FPaths::FileExists(*screenshotPath))
	{
		if (FFileHelper::LoadFileToArray(byteResult, *screenshotPath))
		{
			FContentContainer container = FContentContainer();
			//UE_LOG(LogTemp, Log, TEXT("Loaded image %s"), *imagesInDirectory[i]);
			//loaded the file
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("--cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-Type: image/png");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-disposition: form-data; name=\"screenshot\"; filename=\"screenshot.png\"");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append(TEXT("\r\n"));

			container.Headers = Content;
			container.BodyBinary = byteResult;

			contentArray.Add(container);
		}
		else
		{
			//GLog->Log("couldn't find screenshot to upload");
		}
	}
	else
	{
		//GLog->Log("screenshot path doesn't exist -------- " + screenshotPath);
	}

	TArray<uint8> AllBytes;
	auto HttpRequest = FHttpModule::Get().CreateRequest();

	for (int32 i = 0; i < contentArray.Num(); i++)
	{
		//reference
		//RequestPayload.SetNumUninitialized(Converter.Length());
		//FMemory::Memcpy(RequestPayload.GetData(), (const uint8*)Converter.Get(), RequestPayload.Num());


		//headers
		FTCHARToUTF8 Converter(*contentArray[i].Headers);
		auto data = (const uint8*)Converter.Get();
		AllBytes.Append(data, Converter.Length());

		//content
		if (contentArray[i].BodyText.Len() > 0)
		{
			FTCHARToUTF8 ConverterBody(*contentArray[i].BodyText);
			auto bodydata = (const uint8*)ConverterBody.Get();
			//TArray<uint8> outbytes;
			//StringToBytes((ANSI_TO_TCHAR(contentArray[i].BodyText*)), outbytes);

			AllBytes.Append(bodydata, ConverterBody.Length());
		}
		else
		{
			AllBytes.Append(contentArray[i].BodyBinary);
		}
	}



	TArray<uint8> EndBytes;
	FString EndString;

	EndString = TEXT("\r\n");
	FTCHARToUTF8 ConverterEnd1(*EndString);
	auto enddata1 = (const uint8*)ConverterEnd1.Get();
	AllBytes.Append(enddata1, ConverterEnd1.Length());

	EndString = TEXT("--cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i--");
	FTCHARToUTF8 ConverterEnd2(*EndString);
	auto enddata2 = (const uint8*)ConverterEnd2.Get();
	AllBytes.Append(enddata2, ConverterEnd2.Length());

	EndString = TEXT("\r\n");
	FTCHARToUTF8 ConverterEnd3(*EndString);
	auto enddata3 = (const uint8*)ConverterEnd3.Get();
	AllBytes.Append(enddata3, ConverterEnd3.Length());

	HttpRequest->SetURL(url);
	HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=\"cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i\"");
	HttpRequest->SetHeader("Accept-Encoding", "identity");
	FString AuthValue = "APIKEY:DEVELOPER " + DeveloperKey;
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetContent(AllBytes);

	FHttpModule::Get().SetHttpTimeout(0);

	if (expectedResponseType == "scene")
	{
		HttpRequest->OnProcessRequestComplete().BindRaw(this, &FCognitiveEditorTools::OnUploadSceneCompleted);
	}
	if (expectedResponseType == "object")
	{
		HttpRequest->OnProcessRequestComplete().BindRaw(this, &FCognitiveEditorTools::OnUploadObjectCompleted);
	}

	HttpRequest->ProcessRequest();
}

void FCognitiveEditorTools::OnUploadSceneCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
		GLog->Log("FCognitiveEditorTools::OnUploadSceneCompleted response code " + FString::FromInt(Response->GetResponseCode()));
	else
	{
		GLog->Log("FCognitiveEditorTools::OnUploadSceneCompleted failed to connect");
		WizardUploadError = "FCognitiveEditorTools::OnUploadSceneCompleted failed to connect";
		WizardUploading = false;
		WizardUploadResponseCode = 0;
		return;
	}
	WizardUploadResponseCode = Response->GetResponseCode();

	if (bWasSuccessful && Response->GetResponseCode() < 300)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Upload Scene Response is %s"), *Response->GetContentAsString());

		UWorld* myworld = GWorld->GetWorld();
		if (myworld == NULL)
		{
			UE_LOG(LogTemp, Error, TEXT("Upload Scene - No world!"));
			WizardUploadError = "FCognitiveEditorTools::OnUploadSceneCompleted no world";
			return;
		}

		FString currentSceneName = myworld->GetMapName();
		currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

		FString responseNoQuotes = *Response->GetContentAsString().Replace(TEXT("\""), TEXT(""));

		if (responseNoQuotes.Len() > 0)
		{
			SaveSceneData(currentSceneName, responseNoQuotes);
			ReadSceneDataFromFile();
		}
		else
		{
			//successfully uploaded a scene but no response - updated an existing scene version
			ReadSceneDataFromFile();
		}
		ConfigFileHasChanged = true;
		if (WizardUploading)
		{
			CurrentSceneVersionRequest();
		}
	}
	else
	{
		WizardUploading = false;
		WizardUploadError = "FCognitiveEditorTools::OnUploadSceneCompleted response code " + FString::FromInt(Response->GetResponseCode());
	}
}

void FCognitiveEditorTools::OnUploadObjectCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	OutstandingDynamicUploadRequests--;

	if (bWasSuccessful)
	{
		GLog->Log("FCognitiveEditorTools::OnUploadObjectCompleted response code " + FString::FromInt(Response->GetResponseCode()));

		FNotificationInfo NotifyInfo(FText::FromString(TEXT("FCognitiveEditorTools::OnUploadObjectCompleted response code " + FString::FromInt(Response->GetResponseCode()))));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
	else
	{
		GLog->Log("FCognitiveEditorTools::OnUploadObjectCompleted failed to connect");
		WizardUploading = false;
		WizardUploadError = "FCognitiveEditorTools::OnUploadObjectCompleted failed to connect";
		WizardUploadResponseCode = 0;
		return;
	}

	WizardUploadResponseCode = Response->GetResponseCode();

	if (bWasSuccessful && Response->GetResponseCode() < 300)
	{
		FString responseNoQuotes = *Response->GetContentAsString().Replace(TEXT("\""), TEXT(""));
		//GLog->Log("Upload Dynamic Complete " + Request->GetURL());
	}
	else
	{
		WizardUploading = false;
		WizardUploadError = "FCognitiveEditorTools::OnUploadObjectCompleted response code " + FString::FromInt(Response->GetResponseCode());
	}

	if (WizardUploading && OutstandingDynamicUploadRequests <= 0)
	{
		//upload manifest
		UploadDynamicsManifest();
	}
}

//https://answers.unrealengine.com/questions/212791/how-to-get-file-list-in-a-directory.html
/**
Gets all the files in a given directory.
@param directory The full path of the directory we want to iterate over.
@param fullpath Whether the returned list should be the full file paths or just the filenames.
@param onlyFilesStartingWith Will only return filenames starting with this string. Also applies onlyFilesEndingWith if specified.
@param onlyFilesEndingWith Will only return filenames ending with this string (it looks at the extension as well!). Also applies onlyFilesStartingWith if specified.
@return A list of files (including the extension).
*/
TArray<FString> FCognitiveEditorTools::GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension, const FString ignoreExtension, bool skipsubdirectory) const
{
	// Get all files in directory
	TArray<FString> directoriesToSkip;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, directoriesToSkip, directoriesToSkip, false);
	//PlatformFile.IterateDirectoryStat(*directory, Visitor);
	Visitor.Visit(*directory, true);
	TArray<FString> files;

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		if (skipsubdirectory)
		{
			//check if subdirectory
			FString basicPath = TimestampIt.Key();
			basicPath.RemoveFromStart(directory + "/");
			if (basicPath.Contains("\"") || basicPath.Contains("/"))
			{
				continue;
			}
		}

		const FString filePath = TimestampIt.Key();
		const FString fileName = FPaths::GetCleanFilename(filePath);
		bool shouldAddFile = true;

		// Check if filename starts with required characters
		if (!onlyFilesStartingWith.IsEmpty())
		{
			const FString left = fileName.Left(onlyFilesStartingWith.Len());

			if (!(fileName.Left(onlyFilesStartingWith.Len()).Equals(onlyFilesStartingWith)))
				shouldAddFile = false;
		}

		// Check if file extension is required characters
		if (!onlyFilesWithExtension.IsEmpty())
		{
			if (!(FPaths::GetExtension(fileName, false).Equals(onlyFilesWithExtension, ESearchCase::IgnoreCase)))
				shouldAddFile = false;
		}

		if (!ignoreExtension.IsEmpty())
		{
			if ((FPaths::GetExtension(fileName, false).Equals(ignoreExtension, ESearchCase::IgnoreCase)))
				shouldAddFile = false;
		}

		// Add full path to results
		if (shouldAddFile)
		{
			files.Add(fullPath ? filePath : fileName);
		}
	}

	return files;
}

//checks for json and no bmps files in export directory
bool FCognitiveEditorTools::HasConvertedFilesInDirectory() const
{
	if (!HasSetExportDirectory()) { return false; }

	FString filesStartingWith = TEXT("");
	FString bmpextension = TEXT("bmp");
	FString jsonextension = TEXT("json");

	TArray<FString> imagesInDirectory = GetAllFilesInDirectory(BaseExportDirectory, true, filesStartingWith, bmpextension, filesStartingWith, false);
	TArray<FString> jsonInDirectory = GetAllFilesInDirectory(BaseExportDirectory, true, filesStartingWith, jsonextension, filesStartingWith, false);
	if (imagesInDirectory.Num() > 0) { return false; }
	if (jsonInDirectory.Num() == 0) { return false; }

	return true;
}

bool FCognitiveEditorTools::CanUploadSceneFiles() const
{
	return HasConvertedFilesInDirectory() && HasDeveloperKey();
}

bool FCognitiveEditorTools::CurrentSceneHasSceneId() const
{
	if (!HasDeveloperKey()) { return false; }
	TSharedPtr<FEditorSceneData> currentscene = GetCurrentSceneData();
	if (!currentscene.IsValid())
	{
		return false;
	}
	if (currentscene->Id.Len() > 0)
	{
		return true;
	}
	return false;
}

bool FCognitiveEditorTools::HasSetExportDirectory() const
{
	if (!HasDeveloperKey()) { return false; }
	return FCognitiveEditorTools::GetBaseExportDirectory().Len() != 0;
}


FText FCognitiveEditorTools::GetDynamicsOnSceneExplorerTooltip() const
{
	if (!HasDeveloperKey())
	{
		return FText::FromString("Must log in to get Dynamic Objects List from SceneExplorer");
	}
	auto scene = GetCurrentSceneData();
	if (!scene.IsValid())
	{
		return FText::FromString("Scene does not have valid data. Must export your level before uploading dynamics!");
	}
	return FText::FromString("Something went wrong!");
}

EVisibility FCognitiveEditorTools::ConfigFileChangedVisibility() const
{
	if (ConfigFileHasChanged)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Hidden;
}

FText FCognitiveEditorTools::SendDynamicsToSceneExplorerTooltip() const
{
	if (HasDeveloperKey())
	{
		return FText::FromString("");
	}
	return FText::FromString("Must log in to send Dynamic Objects List to SceneExplorer");
}

bool FCognitiveEditorTools::HasSetDynamicExportDirectory() const
{
	if (!HasDeveloperKey()) { return false; }
	if (GetBaseExportDirectory().Len() == 0) { return false; }
	return true;
}

bool FCognitiveEditorTools::HasExportedAnyDynamicMeshes() const
{
	if (GetBaseExportDirectory().Len() == 0) { return false; }

	FString filesStartingWith = TEXT("");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(BaseExportDirectory + "/dynamics", true, filesStartingWith, filesStartingWith, "", false);
	for (int32 i = 0; i < filesInDirectory.Num(); i++)
	{
		return true;
	}
	return false;
}

bool FCognitiveEditorTools::HasSetDynamicExportDirectoryHasSceneId() const
{
	if (!HasDeveloperKey()) { return false; }
	auto scenedata = GetCurrentSceneData();
	if (!scenedata.IsValid()) { return false; }
	if (GetBaseExportDirectory().Len() == 0) { return false; }
	return true;
}

int32 FCognitiveEditorTools::CountDynamicObjectsInScene() const
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

FText FCognitiveEditorTools::DisplayDynamicObjectsCountInScene() const
{
	return DynamicCountInScene;
}

FText FCognitiveEditorTools::DisplayDynamicObjectsCountOnWeb() const
{
	FString outstring = "Found " + FString::FromInt(SceneExplorerDynamics.Num()) + " Dynamic Objects on SceneExplorer";
	return FText::FromString(outstring);
}

FReply FCognitiveEditorTools::RefreshDisplayDynamicObjectsCountInScene()
{
	DynamicCountInScene = FText::FromString("Found " + FString::FromInt(CountDynamicObjectsInScene()) + " Dynamic Objects in level");
	DuplicateDyanmicObjectVisibility = EVisibility::Hidden;

	SceneDynamics.Empty();
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
		//populate id based on id type
		if (dynamic->IdSourceType == EIdSourceType::CustomId)
		{
			SceneDynamics.Add(MakeShareable(new FDynamicData(dynamic->GetOwner()->GetName(), dynamic->MeshName, dynamic->CustomId, EDynamicTypes::CustomId)));
		}
		else if (dynamic->IdSourceType == EIdSourceType::GeneratedId)
		{
			FString idMessage = TEXT("Id generated during runtime");
			//SceneDynamics.Add(MakeShareable(new FDynamicData(dynamic->GetOwner()->GetName(), dynamic->MeshName, *idMessage)));
			SceneDynamics.Add(MakeShareable(new FDynamicData(dynamic->GetOwner()->GetName(), dynamic->MeshName, idMessage, EDynamicTypes::GeneratedId)));
		}
		else if (dynamic->IdSourceType == EIdSourceType::PoolId)
		{
			
			//construct a string for the number of ids in the pool
			FString IdString = FString::Printf(TEXT("Id Pool(%d)"), dynamic->IDPool->Ids.Num());

			//add it as TSharedPtr<FDynamicData> to the SceneDynamics
			SceneDynamics.Add(MakeShareable(new FDynamicData(dynamic->GetOwner()->GetName(), dynamic->MeshName, IdString, dynamic->IDPool->Ids, EDynamicTypes::DynamicIdPool)));
		}
		//dynamics.Add(dynamic);
	}

	if (DuplicateDynamicIdsInScene())
	{
		DuplicateDyanmicObjectVisibility = EVisibility::Visible;
	}
	else
	{
		DuplicateDyanmicObjectVisibility = EVisibility::Collapsed;
	}

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

	for (const FAssetData& Asset : AssetData)
	{
		// Do something with each asset, e.g., log its name
		UE_LOG(LogTemp, Log, TEXT("Found Asset: %s"), *Asset.AssetName.ToString());

		//get the actual asset from the asset data
		UObject* IdPoolObject = Asset.GetAsset();
		//cast it to a dynamic id pool asset
		UDynamicIdPoolAsset* IdPoolAsset = Cast<UDynamicIdPoolAsset>(IdPoolObject);

		//construct a string for the number of ids in the pool
		FString IdString = FString::Printf(TEXT("Id Pool Asset(%d)"), IdPoolAsset->Ids.Num());

		//add it as TSharedPtr<FDynamicData> to the SceneDynamics
		SceneDynamics.Add(MakeShareable(new FDynamicData(IdPoolAsset->PrefabName, IdPoolAsset->MeshName, IdString, IdPoolAsset->Ids, EDynamicTypes::DynamicIdPoolAsset)));
	}



	return FReply::Handled();
}

EVisibility FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility() const
{
	return DuplicateDyanmicObjectVisibility;
}

FText FCognitiveEditorTools::GetUploadDynamicsToSceneText() const
{
	return UploadDynamicsToSceneText;
}

bool FCognitiveEditorTools::DuplicateDynamicIdsInScene() const
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


void FCognitiveEditorTools::OnApplicationKeyChanged(const FText& Text)
{
	ApplicationKey = Text.ToString();
	//FAnalyticsCognitiveVR::GetCognitiveVRProvider()->APIKey = APIKey;
	//FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "ApiKey", false);
}

FText FCognitiveEditorTools::GetApplicationKey() const
{
	return FText::FromString(ApplicationKey);
}

FText FCognitiveEditorTools::GetDeveloperKey() const
{
	return FText::FromString(DeveloperKey);
}

FText FCognitiveEditorTools::GetAttributionKey() const
{
	return FText::FromString(AttributionKey);
}

void FCognitiveEditorTools::OnDeveloperKeyChanged(const FText& Text)
{
	DeveloperKey = Text.ToString();
}

void FCognitiveEditorTools::OnAttributionKeyChanged(const FText& Text)
{
	AttributionKey = Text.ToString();
}


void FCognitiveEditorTools::OnExportPathChanged(const FText& Text)
{
	BaseExportDirectory = Text.ToString();
}

FText FCognitiveEditorTools::UploadSceneNameFiles() const
{
	auto currentscenedata = GetCurrentSceneData();
	if (!currentscenedata.IsValid())
	{
		UWorld* myworld = GWorld->GetWorld();

		FString currentSceneName = myworld->GetMapName();
		currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

		return FText::FromString("Upload Files for " + currentSceneName);
	}
	FString outstring = "Upload Files for " + currentscenedata->Name;

	return FText::FromString(outstring);
}

FText FCognitiveEditorTools::OpenSceneNameInBrowser() const
{
	auto currentscenedata = GetCurrentSceneData();
	if (!currentscenedata.IsValid())
	{
		UWorld* myworld = GWorld->GetWorld();

		FString currentSceneName = myworld->GetMapName();
		currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

		return FText::FromString("Open" + currentSceneName + " in Browser...");
	}
	FString outstring = "Open " + currentscenedata->Name + " in Browser...";

	return FText::FromString(outstring);
}

FText FCognitiveEditorTools::GetDynamicObjectUploadText() const
{
	auto data = GetCurrentSceneData();
	if (!data.IsValid())
	{
		return FText::FromString("No Scene Data found - Have you used Cognitive Scene Setup to export this scene?");
	}

	//get selected dynamic data
	//for each unique mesh name

	return FText::FromString("Upload " + FString::FromInt(SubDirectoryNames.Num()) + " Dynamic Object Meshes to " + data->Name + " Version " + FString::FromInt(data->VersionNumber));
}

FReply FCognitiveEditorTools::RefreshDynamicSubDirectory()
{
	FindAllSubDirectoryNames();
	//SubDirectoryListWidget->RefreshList();
	return FReply::Handled();
}

FReply FCognitiveEditorTools::ButtonCurrentSceneVersionRequest()
{
	CurrentSceneVersionRequest();

	return FReply::Handled();
}

void FCognitiveEditorTools::CurrentSceneVersionRequest()
{
	TSharedPtr<FEditorSceneData> scenedata = GetCurrentSceneData();

	if (scenedata.IsValid())
	{
		SceneVersionRequest(*scenedata);
	}
}

FReply FCognitiveEditorTools::OpenURL(FString url)
{
	FPlatformProcess::LaunchURL(*url, nullptr, nullptr);
	return FReply::Handled();
}

FReply FCognitiveEditorTools::OpenSceneInBrowser(FString sceneid)
{
	FString url = SceneExplorerOpen(sceneid);

	FPlatformProcess::LaunchURL(*url, nullptr, nullptr);

	return FReply::Handled();
}

FReply FCognitiveEditorTools::OpenCurrentSceneInBrowser()
{
	TSharedPtr<FEditorSceneData> scenedata = GetCurrentSceneData();

	if (!scenedata.IsValid())
	{
		return FReply::Handled();
	}

	FString url = SceneExplorerOpen(scenedata->Id);

	return FReply::Handled();
}

void FCognitiveEditorTools::ReadSceneDataFromFile()
{
	SceneData.Empty();

	TArray<FString>scenstrings;
	FString TestSyncFile = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenstrings, TestSyncFile);

	for (int32 i = 0; i < scenstrings.Num(); i++)
	{
		TArray<FString> Array;
		scenstrings[i].ParseIntoArray(Array, TEXT(","), true);

		if (Array.Num() == 2) //scenename,sceneid
		{
			//old scene data. append versionnumber and versionid
			Array.Add("1");
			Array.Add("0");
		}

		if (Array.Num() != 4)
		{
			GLog->Log("FCognitiveTools::RefreshSceneData failed to parse " + scenstrings[i]);
			continue;
		}

		SceneData.Add(MakeShareable(new FEditorSceneData(Array[0], Array[1], FCString::Atoi(*Array[2]), FCString::Atoi(*Array[3]))));
	}

	GLog->Log("FCognitiveTools::RefreshSceneData found this many scenes: " + FString::FromInt(SceneData.Num()));
	//ConfigFileHasChanged = true;
}

void FCognitiveEditorTools::SceneVersionRequest(FEditorSceneData data)
{
	if (!HasDeveloperKey())
	{
		GLog->Log("FCognitiveTools::SceneVersionRequest no developer key set!");
		//auto httprequest = RequestAuthTokenCallback();

		return;
	}

	auto HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(GetSceneVersion(data.Id));

	HttpRequest->SetHeader("X-HTTP-Method-Override", TEXT("GET"));
	//HttpRequest->SetHeader("Authorization", TEXT("Data " + FAnalyticsCognitiveVR::Get().EditorAuthToken));
	FString AuthValue = "APIKEY:DEVELOPER " + DeveloperKey;
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetHeader("Content-Type", "application/json");

	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FCognitiveEditorTools::SceneVersionResponse);
	HttpRequest->ProcessRequest();
}

void FCognitiveEditorTools::SceneVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
		GLog->Log("FCognitiveEditorTools::SceneVersionResponse response code " + FString::FromInt(Response->GetResponseCode()));
	else
	{
		GLog->Log("FCognitiveEditorTools::SceneVersionResponse failed to connect");
		WizardUploading = false;
		WizardUploadError = "FCognitiveEditorTools::SceneVersionResponse failed to connect";
		WizardUploadResponseCode = 0;
		return;
	}

	WizardUploadResponseCode = Response->GetResponseCode();

	if (WizardUploadResponseCode >= 500)
	{
		//internal server error
		GLog->Log("FCognitiveTools::SceneVersionResponse 500-ish internal server error");
		WizardUploadError = "FCognitiveEditorTools::SceneVersionResponse response code " + FString::FromInt(Response->GetResponseCode());
		return;
	}
	if (WizardUploadResponseCode >= 400)
	{
		WizardUploading = false;
		if (WizardUploadResponseCode == 401)
		{
			//not authorized or scene id does not exist
			GLog->Log("FCognitiveTools::SceneVersionResponse not authorized or scene doesn't exist!");
			WizardUploadError = "FCognitiveEditorTools::SceneVersionResponse response code " + FString::FromInt(Response->GetResponseCode()) + "\nThe Developer Key: " + DeveloperKey + " does not have access to the scene";
			return;
		}
		else
		{
			//maybe no scene?
			GLog->Log("FCognitiveTools::SceneVersionResponse some error. Maybe no scene?");
			WizardUploadError = "FCognitiveEditorTools::SceneVersionResponse response code " + FString::FromInt(Response->GetResponseCode());
			return;
		}
	}
	WizardUploadError = "";

	//parse response content to json

	TSharedPtr<FJsonObject> JsonSceneSettings;

	TSharedRef<TJsonReader<>>Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(Reader, JsonSceneSettings))
	{
		//get the latest version of the scene
		int32 versionNumber = 0;
		int32 versionId = 0;
		TArray<TSharedPtr<FJsonValue>> versions = JsonSceneSettings->GetArrayField("versions");
		for (int32 i = 0; i < versions.Num(); i++) {

			int32 tempversion = versions[i]->AsObject()->GetNumberField("versionnumber");
			if (tempversion > versionNumber)
			{
				versionNumber = tempversion;
				versionId = versions[i]->AsObject()->GetNumberField("id");
			}
		}
		if (versionNumber + versionId == 0)
		{
			GLog->Log("FCognitiveTools::SceneVersionResponse couldn't find a latest version in SceneVersion data");
			WizardUploadError = "FCognitiveTools::SceneVersionResponse couldn't find a latest version in SceneVersion data";
			return;
		}

		//check that there is scene data in ini
		TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();
		if (!currentSceneData.IsValid())
		{
			GLog->Log("FCognitiveTools::SceneVersionResponse can't find current scene data in ini files");
			WizardUploadError = "FCognitiveTools::SceneVersionResponse can't find current scene data in ini files";
			return;
		}

		//get array of scene data
		TArray<FString> iniscenedata;

		FString TestSyncFile = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
		GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), iniscenedata, TestSyncFile);

		//GLog->Log("found this many scene datas in ini " + FString::FromInt(iniscenedata.Num()));
		//GLog->Log("looking for scene " + currentSceneData->Name);

		int32 lastVersionNumber = 0;

		//update current scene
		for (int32 i = 0; i < iniscenedata.Num(); i++)
		{
			//GLog->Log("looking at data " + iniscenedata[i]);

			TArray<FString> entryarray;
			iniscenedata[i].ParseIntoArray(entryarray, TEXT(","), true);

			if (entryarray[0] == currentSceneData->Name && versionNumber > lastVersionNumber)
			{
				lastVersionNumber = versionNumber;
				iniscenedata[i] = entryarray[0] + "," + entryarray[1] + "," + FString::FromInt(versionNumber) + "," + FString::FromInt(versionId);
				//GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse overwriting scene data to append versionnumber and versionid");
				//GLog->Log(iniscenedata[i]);
			}
			else
			{
				//GLog->Log("found scene " + entryarray[0]);
			}
		}

		GLog->Log("FCognitiveTools::SceneVersionResponse successful. Write scene data to config file");

		//set array to config
		GConfig->RemoveKey(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), TestSyncFile);
		GConfig->SetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), iniscenedata, TestSyncFile);

		//FConfigCacheIni::LoadGlobalIniFile(GEngineIni, TEXT("Engine"));

		//GConfig->RemoveKey(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), GEngineIni);
		//GConfig->SetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), iniscenedata, GEngineIni);


		GConfig->Flush(false, TestSyncFile);
		//GConfig->LoadFile(TestSyncFile);
		ConfigFileHasChanged = true;
		ReadSceneDataFromFile();

		if (WizardUploading)
		{
			UploadDynamics();
		}
	}
	else
	{
		GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse failed to parse json response");
		if (WizardUploading)
		{
			WizardUploadError = "FCognitiveToolsCustomization::SceneVersionResponse failed to parse json response";
			WizardUploadResponseCode = 0;
			WizardUploading = false;
		}
	}
}

TArray<TSharedPtr<FEditorSceneData>> FCognitiveEditorTools::GetSceneData() const
{
	return SceneData;
}

FReply FCognitiveEditorTools::SaveAPIDeveloperKeysToFile()
{
	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));
	GConfig->SetString(TEXT("Analytics"), TEXT("ApiKey"), *ApplicationKey, EngineIni);
	GConfig->SetString(TEXT("Analytics"), TEXT("AttributionKey"), *AttributionKey, EngineIni);
	GConfig->SetString(TEXT("Analytics"), TEXT("DeveloperKey"), *DeveloperKey, EditorIni);

	GConfig->Flush(false, GEngineIni);
	GConfig->Flush(false, GEditorIni);
	ConfigFileHasChanged = true;

	return FReply::Handled();
}

void FCognitiveEditorTools::SaveApplicationKeyToFile(FString key)
{
	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->SetString(TEXT("Analytics"), TEXT("ApiKey"), *key, EngineIni);
	GConfig->Flush(false, GEngineIni);
	ConfigFileHasChanged = true;

	ApplicationKey = key;
}

void FCognitiveEditorTools::SaveDeveloperKeyToFile(FString key)
{
	FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));
	GConfig->SetString(TEXT("Analytics"), TEXT("DeveloperKey"), *key, EditorIni);
	GConfig->Flush(false, GEngineIni);
	ConfigFileHasChanged = true;

	DeveloperKey = key;
}

TSharedPtr<FEditorSceneData> FCognitiveEditorTools::GetCurrentSceneData() const
{
	UWorld* myworld = GWorld->GetWorld();

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	return GetSceneData(currentSceneName);
}

FString lastSceneName;
TSharedPtr<FEditorSceneData> FCognitiveEditorTools::GetSceneData(FString scenename) const
{
	for (int32 i = 0; i < SceneData.Num(); i++)
	{
		if (!SceneData[i].IsValid()) { continue; }
		if (SceneData[i]->Name == scenename)
		{
			return SceneData[i];
		}
	}
	if (lastSceneName != scenename)
	{
		GLog->Log("FCognitiveToolsCustomization::GetSceneData couldn't find SceneData for scene " + scenename);
		lastSceneName = scenename;
	}
	return NULL;
}

void FCognitiveEditorTools::SaveSceneData(FString sceneName, FString sceneKey)
{
	FString keyValue = sceneName + "," + sceneKey;

	TArray<FString> scenePairs = TArray<FString>();

	FString TestSyncFile = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, TestSyncFile);

	bool didSetKey = false;
	for (int32 i = 0; i < scenePairs.Num(); i++)
	{
		FString name;
		FString key;
		scenePairs[i].Split(TEXT(","), &name, &key);
		if (*name == sceneName)
		{
			scenePairs[i] = keyValue;
			didSetKey = true;
			GLog->Log("FCognitiveToolsCustomization::SaveSceneData - found and replace key for scene " + name + " new value " + keyValue);
			break;
		}
	}
	if (!didSetKey)
	{
		scenePairs.Add(keyValue);
		GLog->Log("FCognitiveToolsCustomization::SaveSceneData - added new scene value and key for " + sceneName);
	}

	//remove scene names that don't have keys!
	for (int32 i = scenePairs.Num() - 1; i >= 0; i--)
	{
		FString name;
		FString key;
		if (!scenePairs[i].Split(TEXT(","), &name, &key))
		{
			scenePairs.RemoveAt(i);
		}
	}

	GConfig->SetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, TestSyncFile);

	GConfig->Flush(false, GEngineIni);
}




TArray<AActor*> FCognitiveEditorTools::PrepareSceneForExport(bool OnlyExportSelected)
{
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

	return ToBeExportedFinal;
}

void FCognitiveEditorTools::ExportScene(TArray<AActor*> actorsToExport)
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::ExportScene world is null"));
		return;
	}

	GEditor->SelectNone(false, true, false);

	for (int32 i = 0; i < actorsToExport.Num(); i++)
	{
		GEditor->SelectActor((actorsToExport[i]), true, false, true);
	}

	//create directory at scene name path
	FString sceneDirectory = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory() + "/";
	FCognitiveEditorTools::VerifyOrCreateDirectory(sceneDirectory);

	//create screenshot directory
	FString dir = BaseExportDirectory + "/" + GetCurrentSceneName() + "/screenshot/";
	FCognitiveEditorTools::VerifyOrCreateDirectory(dir);
	
	FSuppressableWarningDialog::FSetupInfo ExportSettingsInfo(LOCTEXT("ExportSettingsBody", "Make sure the scale is set to 1.0 to get the correct representation on the Dashboard"), LOCTEXT("ExportSettingsTitle", "Recommended Export Settings"), "ExportSelectedDynamicsBody");
	ExportSettingsInfo.ConfirmText = LOCTEXT("Ok", "Ok");
	ExportSettingsInfo.CheckBoxText = FText::FromString("Don't show again");
	FSuppressableWarningDialog ExportSelectedDynamicMeshes(ExportSettingsInfo);
	ExportSelectedDynamicMeshes.ShowModal();
	FString ExportedSceneFile2 = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory() + "/" + FCognitiveEditorTools::GetInstance()->GetCurrentSceneName() + ".gltf";


	
	GEditor->ExportMap(tempworld, *ExportedSceneFile2, true);

	//check that the map was actually exported and generated temporary files
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*ExportedSceneFile2))
	{
		UE_LOG(LogTemp, Error, TEXT("FCognitiveEditorTools::ExportScene Cancelled scene geometry export"));
		return;
	}

	//export materials
	TArray< UStaticMeshComponent*> sceneMeshes;

	for (auto& elem : actorsToExport)
	{
		auto TempObject = elem->GetComponentByClass(UStaticMeshComponent::StaticClass());
		if (TempObject == NULL) { continue; }
		auto staticTempObject = (UStaticMeshComponent*)TempObject;

		if (staticTempObject->GetOwner() == NULL) { continue; }

		UActorComponent* dynamic = staticTempObject->GetOwner()->GetComponentByClass(UDynamicObject::StaticClass());
		if (dynamic != NULL) { continue; }

		sceneMeshes.Add(staticTempObject);
	}

	//SaveScreenshotToFile();

	FString ObjPath = FPaths::Combine(BaseExportDirectory, GetCurrentSceneName());
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	//create settings.json
	FString settingsContents = "{\"scale\":100,\"sdkVersion\":\"" + FString(COGNITIVEVR_SDK_VERSION) + "\",\"sceneName\":\"" + GetCurrentSceneName() + "\"}";
	FString settingsFullPath = escapedOutPath + "/settings.json";
	bool writeSettingsJsonSuccess = FFileHelper::SaveStringToFile(settingsContents, *settingsFullPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	if (writeSettingsJsonSuccess == false)
	{
		UE_LOG(LogTemp, Error, TEXT("FCognitiveEditorTools::ExportScene unable to save settings.json"));
	}

	//write debug.log including unreal data, scene contents, folder contents
	//should happen after next button is pressed
	FString fullPath = escapedOutPath + "/debug.log";
	FString fileContents = BuildDebugFileContents();
	FFileHelper::SaveStringToFile(fileContents, *fullPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	ValidateGeneratedFiles();
	

}

void FCognitiveEditorTools::ValidateGeneratedFiles()
{
	//validate other files:
	FString ExportDirPath = FCognitiveEditorTools::GetInstance()->GetCurrentSceneExportDirectory() + "/";
	TArray<FString> FoundFiles;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Get all files with .gltf extension in the directory
	PlatformFile.FindFiles(FoundFiles, *ExportDirPath, TEXT("gltf"));
	//bin
	PlatformFile.FindFiles(FoundFiles, *ExportDirPath, TEXT("bin"));
	//json
	PlatformFile.FindFiles(FoundFiles, *ExportDirPath, TEXT("json"));
	//fbx
	PlatformFile.FindFiles(FoundFiles, *ExportDirPath, TEXT("fbx"));
	//bmp
	PlatformFile.FindFiles(FoundFiles, *ExportDirPath, TEXT("bmp"));

	bool FoundErrorFile = false;

	//see if the size is more than or equal to 3, meaning we found the gltf, the bin and json files
	if (FoundFiles.Num() >= 3)
	{
		// Found at least one .txt file in the directory.
		for (FString File : FoundFiles)
		{
			// Do something with each file, if needed.
			UE_LOG(LogTemp, Warning, TEXT("Found file: %s"), *File);
			if (File.Contains(".fbx"))
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Warning!! Found fbx file in export folder. Please try exporting again."));
				UE_LOG(LogTemp, Error, TEXT("Warning!! Found fbx file: %s. Please try exporting again."), *File);
				FoundErrorFile = true;
			}
			if (File.Contains(".bmp"))
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Warning!! Found bmp file in export folder. Please try exporting again."));
				UE_LOG(LogTemp, Error, TEXT("Warning!! Found bmp file: %s. Please try exporting again."), *File);
				FoundErrorFile = true;
			}
		}
		if (!FoundErrorFile)
		{
			UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::ExportScene verified generated files"));
		}
		
	}
	else
	{
		// No required files found in the directory.
		UE_LOG(LogTemp, Error, TEXT("FCognitiveEditorTools::ExportScene could not validate generated files"));
	}

	//rename exports and .bin reference in the .gltf when exporting with arbitrary names to be scene.gltf and scene.bin
	FString levelName = GWorld->GetMapName(); //GetWorld()->GetMapName();
	levelName.RemoveFromStart(GWorld->StreamingLevelsPrefix);

	FString scenegltfPath = ExportDirPath + "/scene.gltf";
	FString levelgltfPath = ExportDirPath + "/" + levelName +  ".gltf";

	FString scenebinPath = ExportDirPath + "/scene.bin";
	FString levelbinPath = ExportDirPath + "/" + levelName + ".bin";

	FString scenemtlPath = ExportDirPath + "/scene.mtl";
	FString levelmtlPath = ExportDirPath + "/" + levelName + ".mtl";



	RenameFile(levelgltfPath, scenegltfPath);
	RenameFile(levelbinPath, scenebinPath);
	RenameFile(levelmtlPath, scenemtlPath);

	ModifyGLTFContent(scenegltfPath);
}

bool FCognitiveEditorTools::RenameFile(FString oldPath, FString newPath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	// Check if the old file exists
	if (PlatformFile.FileExists(*oldPath))
	{
		// Rename the file
		bool bFileExists = PlatformFile.FileExists(*newPath);
		if (bFileExists)
		{
			PlatformFile.DeleteFile(*newPath);
		}
		bool bRenameSuccess = PlatformFile.MoveFile(*newPath, *oldPath);
		if (bRenameSuccess)
		{
			UE_LOG(LogSlate, Log, TEXT("File renamed successfully!"));
			return true;
		}
		else
		{
			UE_LOG(LogSlate, Log, TEXT("Failed to rename the file!"));
			return false;
		}
	}
	else
	{
		UE_LOG(LogSlate, Log, TEXT("Old file does not exist!"));
	}

	return false;
}

void FCognitiveEditorTools::ModifyGLTFContent(FString FilePath)
{
	FString FileContent;

	// Load the .gltf file content
	if (FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContent);

		if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
			// find the uri under buffers and change it
			if (JsonObject->HasField("buffers"))
			{
				TArray<TSharedPtr<FJsonValue>> Buffers = JsonObject->GetArrayField("buffers");
				if (Buffers.Num() > 0)
				{
					TSharedPtr<FJsonObject> FirstBuffer = Buffers[0]->AsObject();
					if (FirstBuffer->HasField("uri"))
					{
						FirstBuffer->SetStringField("uri", "scene.bin");
					}
				}
			}

			// Save modified content back to file
			FString OutputString;
			TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
			FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

			FFileHelper::SaveStringToFile(OutputString, *FilePath);
		}
	}
}

void FCognitiveEditorTools::GenerateSettingsJsonFile()
{
	FString ObjPath = FPaths::Combine(BaseExportDirectory, GetCurrentSceneName());
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	//create settings.json
	FString settingsContents = "{\"scale\":100,\"sdkVersion\":\"" + FString(COGNITIVEVR_SDK_VERSION) + "\",\"sceneName\":\"" + GetCurrentSceneName() + "\"}";
	FString settingsFullPath = escapedOutPath + "/settings.json";
	bool writeSettingsJsonSuccess = FFileHelper::SaveStringToFile(settingsContents, *settingsFullPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	if (writeSettingsJsonSuccess == false)
	{
		UE_LOG(LogTemp, Error, TEXT("FCognitiveEditorTools::ExportScene unable to save settings.json"));
	}
}

bool FCognitiveEditorTools::HasSettingsJsonFile() const
{
	FString ObjPath = FPaths::Combine(BaseExportDirectory, GetCurrentSceneName());
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));
	FString settingsFullPath = escapedOutPath + "/settings.json";

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.FileExists(*settingsFullPath);
}

void FCognitiveEditorTools::WizardUpload()
{
	WizardUploading = true;
	WizardUploadError = "";
	WizardUploadResponseCode = 0;

	if (!HasSettingsJsonFile())
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorToolsCustomization::WizardUpload settings.json file missing! Creating for current scene"));
		GenerateSettingsJsonFile();
	}

	UploadScene();
}

FText FCognitiveEditorTools::GetBaseExportDirectoryDisplay() const
{
	return FText::FromString(BaseExportDirectory);
}

//c:/users/me/desktop/export/scenename/
FText FCognitiveEditorTools::GetSceneExportDirectoryDisplay(FString scenename) const
{
	return FText::FromString(FPaths::Combine(BaseExportDirectory, scenename));
}

//c:/users/me/desktop/export/scenename/
FText FCognitiveEditorTools::GetCurrentSceneExportDirectoryDisplay() const
{
	return FText::FromString(FPaths::Combine(BaseExportDirectory, FCognitiveEditorTools::GetInstance()->GetCurrentSceneName()));
}

//c:/users/me/desktop/export/dynamics/
FText FCognitiveEditorTools::GetDynamicsExportDirectoryDisplay() const
{
	return FText::FromString(FPaths::Combine(BaseExportDirectory, "dynamics"));
}

FString FCognitiveEditorTools::BuildDebugFileContents() const
{
	FString outputString;

	outputString += "*****************************\n";
	outputString += "***********SYSTEM************\n";
	outputString += "*****************************\n";

	//unreal version
	outputString += FString("Unreal Engine Version: ") + FEngineVersion::Current().ToString();
	outputString += "\n";

	//os name
	outputString += FString("Platform Name: ") + FPlatformProperties::IniPlatformName();
	outputString += "\n";

	//date + time
	outputString += FString("System Time: ") + FDateTime::Now().ToString();
	outputString += "\n";

	outputString += "*****************************\n";
	outputString += "***********PROJECT***********\n";
	outputString += "*****************************\n";

	//sdk version
	outputString += FString("Cognitive SDK version: ") + FString(COGNITIVEVR_SDK_VERSION);
	outputString += "\n";

	//plugin folder contents
	FString pluginDir = FPaths::ProjectDir() + "/Plugins/";
	//TArray<FString> DirectoriesToSkip;
	//TArray<FString> DirectoriesToNotRecurse;
	//IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	//FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToNotRecurse, true);
	//IFileManager::Get().IterateDirectory(*pluginDir, Visitor);
	//Visitor.Visit(*pluginDir, true);
	//for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)

	if (IFileManager::Get().DirectoryExists(*(pluginDir + "Varjo")))
	{
		outputString += FString("Plugins: Varjo");
		outputString += "\n";
	}
	if (IFileManager::Get().DirectoryExists(*(pluginDir + "TobiiEyeTracking")))
	{
		outputString += FString("Plugins: TobiiEyeTracking");
		outputString += "\n";
	}
	if (IFileManager::Get().DirectoryExists(*(pluginDir + "SRanipal")))
	{
		outputString += FString("Plugins: SRanipal");
		outputString += "\n";
	}
	if (IFileManager::Get().DirectoryExists(*(pluginDir + "PicoMobile")))
	{
		outputString += FString("Plugins: PicoMobile");
		outputString += "\n";
	}

	//project directory
	outputString += FString("Project Name: ") + FPaths::ProjectDir();
	outputString += "\n";

	//gateway
	FString gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	outputString += "Gateway: " + gateway;
	outputString += "\n";

	//api key ****
	FString tempApiKey = ApplicationKey;
	outputString += FString("API Key: ****") + tempApiKey.RightChop(tempApiKey.Len() - 4);
	outputString += "\n";

	//dev key ****
	FString tempDevKey = DeveloperKey;
	outputString += FString("DEV Key: ****") + tempDevKey.RightChop(tempDevKey.Len() - 4);
	outputString += "\n";

	//config settings (batch sizes, etc)
	FString ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "CustomEventBatchSize", false);
	outputString += "Event Snapshot Batch Size: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "CustomEventExtremeLimit", false);
	outputString += "Event Extreme Batch Size: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "CustomEventMinTimer", false);
	outputString += "Event Minimum Send Timer: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "CustomEventAutoTimer", false);
	outputString += "Event Auto Send Timer: " + ValueReceived;
	outputString += "\n";

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "SensorBatchSize", false);
	outputString += "Sensor Snapshot Batch Size: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "SensorExtremeLimit", false);
	outputString += "Sensor Extreme Batch Size: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "SensorMinTimer", false);
	outputString += "Sensor Minimum Send Timer: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "SensorAutoTimer", false);
	outputString += "Sensor Auto Send Timer: " + ValueReceived;
	outputString += "\n";

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "DynamicBatchSize", false);
	outputString += "Dynamic Snapshot Batch Size: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "DynamicExtremeLimit", false);
	outputString += "Dynamic Extreme Batch Size: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "DynamicMinTimer", false);
	outputString += "Dynamic Minimum Send Timer: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "DynamicAutoTimer", false);
	outputString += "Dynamic Auto Send Timer: " + ValueReceived;
	outputString += "\n";

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "GazeBatchSize", false);
	outputString += "Gaze Snapshot Batch Size: " + ValueReceived;
	outputString += "\n";

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "FixationBatchSize", false);
	outputString += "Fixation Snapshot Batch Size: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "FixationExtremeLimit", false);
	outputString += "Fixation Extreme Batch Size: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "FixationMinTimer", false);
	outputString += "Fixation Minimum Send Timer: " + ValueReceived;
	outputString += "\n";
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "FixationAutoTimer", false);
	outputString += "Fixation Auto Send Timer: " + ValueReceived;
	outputString += "\n";

	outputString += "Scene Settings:";
	outputString += "\n";

	TSharedPtr<FEditorSceneData> currentLevelSceneData;
	//scene settings (name, id, version)
	for (auto& elem : FCognitiveEditorTools::GetInstance()->SceneData)
	{
		outputString += FString("    Scene Name: ") + elem->Name;
		outputString += "\n";
		outputString += FString("        Scene Id: ") + elem->Id;
		outputString += "\n";
		outputString += FString("        Version Number: ") + FString::FromInt(elem->VersionNumber);
		outputString += "\n";
		outputString += FString("        Version Id: ") + FString::FromInt(elem->VersionId);
		outputString += "\n";

		if (elem->Name == UGameplayStatics::GetCurrentLevelName(GWorld))
		{
			currentLevelSceneData = elem;
		}
	}

	outputString += "*****************************\n";
	outputString += "********CURRENT SCENE********\n";
	outputString += "*****************************\n";

	//current scene info
	if (currentLevelSceneData.IsValid())
	{
		outputString += FString("Scene Name: ") + currentLevelSceneData->Name;
		outputString += "\n";
		outputString += FString("Scene Id: ") + currentLevelSceneData->Id;
		outputString += "\n";
		outputString += FString("Version Number: ") + FString::FromInt(currentLevelSceneData->VersionNumber);
		outputString += "\n";
		outputString += FString("Version Id: ") + FString::FromInt(currentLevelSceneData->VersionId);
		outputString += "\n";
	}

	outputString += "*****************************\n";
	outputString += "****CURRENT SCENE OBJECTS****\n";
	outputString += "*****************************\n";

	//find dynamics in scene
	TArray<UDynamicObject*> foundDynamics;
	TArray<AActor*> foundActors;
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
		foundDynamics.Add(dynamic);
		foundActors.Add(*ActorItr);
	}

	//dynamics in scene count
	outputString += FString("Dynamic Object Count: ") + FString::FromInt(foundDynamics.Num());
	outputString += "\n";

	for (int32 i = 0; i < foundDynamics.Num(); i++)
	{
		outputString += FString("    ") + foundActors[i]->GetFName().ToString();
		outputString += "\n";
		outputString += FString("        Mesh Name: ") + foundDynamics[i]->MeshName;
		outputString += "\n";
		if (foundDynamics[i]->IdSourceType == EIdSourceType::CustomId)
		{
			outputString += FString("        Id: ") + foundDynamics[i]->CustomId;
			outputString += "\n";
		}
	}

	outputString += "*****************************\n";
	outputString += "********EXPORT FOLDER********\n";
	outputString += "*****************************\n";

	//tree list folders in export directory + contents
	FString dir = FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory();

	FCognitiveEditorTools::GetInstance()->AppendDirectoryContents(dir, 0, outputString);

	return outputString;
}

void FCognitiveEditorTools::AppendDirectoryContents(FString FullPath, int32 depth, FString& outputString)
{
	FString searchPath = FullPath + "/*";

	//add all files to output
	TArray<FString> files;
	IFileManager::Get().FindFiles(files, *searchPath, true, false);

	for (auto& elem : files)
	{
		for (int32 i = 0; i < depth; i++)
			outputString += "    ";

		//file size
		int32 bytecount = IFileManager::Get().FileSize(*(FullPath + "/" + elem));
		float mbcount = bytecount * 0.000001;
		TArray<FString> parseArray;
		FString sizeString = FString::SanitizeFloat(mbcount, 2);
		sizeString.ParseIntoArray(parseArray, TEXT("."));
		FString finalString = parseArray[0] + "." + parseArray[1].Left(2) + "mb";

		outputString += elem + " (" + finalString + ")";
		outputString += "\n";
	}

	//call this again for each directory
	TArray<FString> directories;
	IFileManager::Get().FindFiles(directories, *searchPath, false, true);

	for (auto& elem : directories)
	{
		for (int32 i = 0; i < depth; i++)
			outputString += "    ";

		//directory size returns -1?
		//FFileStatData dirStats = IFileManager::Get().GetStatData(*(FullPath + "/" + elem));

		outputString += "/" + elem;
		outputString += "\n";
		AppendDirectoryContents(FullPath + "/" + elem, depth + 1, outputString);
	}
}

#undef LOCTEXT_NAMESPACE
