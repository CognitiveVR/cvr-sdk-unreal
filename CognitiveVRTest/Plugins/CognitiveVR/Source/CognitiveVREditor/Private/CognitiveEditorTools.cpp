
#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveEditorTools.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

//TSharedRef<FCognitiveEditorTools> ToolsInstance;
FCognitiveEditorTools* CognitiveEditorToolsInstance;
FString Gateway;

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
	auto sessionviewer = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "SessionViewer", false);
	return "https://"+sessionviewer + sceneid;
}


void FCognitiveEditorTools::Initialize()
{
	CognitiveEditorToolsInstance = new FCognitiveEditorTools;

	//should be able to update gateway while unreal is running, but cache if not in editor since that's nuts
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);

	//CognitiveEditorToolsInstance->BaseExportDirectory = FPaths::GameDir();

	//should update both editor urls and session data urls
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	CognitiveEditorToolsInstance->ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
}

//at any step in the uploading process
bool WizardUploading = false;

bool FCognitiveEditorTools::IsWizardUploading()
{
	return WizardUploading;
}

void FCognitiveEditorTools::Tick(float deltatime)
{
	if (WizardUploading)
	{

	}
}



TArray<TSharedPtr<cognitivevrapi::FDynamicData>> SceneExplorerDynamics;
TArray<TSharedPtr<FString>> SubDirectoryNames;

//deals with all the exporting and non-display stuff in the editor preferences

TArray<TSharedPtr<cognitivevrapi::FDynamicData>> FCognitiveEditorTools::GetSceneDynamics()
{
	return SceneDynamics;
}

bool FCognitiveEditorTools::HasDeveloperKey() const
{
	return FAnalyticsCognitiveVR::Get().DeveloperKey.Len() > 0;
}

bool FCognitiveEditorTools::HasAPIKey() const
{
	return APIKey.Len() > 0;
}

FReply FCognitiveEditorTools::ExportAllDynamics()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorToolsCustomization::ExportDynamics world is null"));
		return FReply::Handled();
	}

	TArray<FString> meshNames;
	TArray<UDynamicObject*> exportObjects;

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
		if (!meshNames.Contains(dynamic->MeshName))
		{
			exportObjects.Add(dynamic);
			meshNames.Add(dynamic->MeshName);
		}
	}

	if (meshNames.Num() == 0)
	{
		return FReply::Handled();
	}

	if (BaseExportDirectory.Len() == 0)
	{
		GLog->Log("base directory not selected");
		return FReply::Handled();
	}

	for (TObjectIterator<UDynamicObject> It; It; ++It)
	{
		UDynamicObject* TempObject = *It;
		if (TempObject != NULL)
		{
			if (!meshNames.Contains(TempObject->MeshName))
			{
				exportObjects.Add(TempObject);
				meshNames.Add(TempObject->MeshName);
			}
		}
	}

	

	ExportDynamicObjectArray(exportObjects);

	return FReply::Handled();
}

FReply FCognitiveEditorTools::ExportSelectedDynamics()
{
	TArray<FString> meshNames;
	TArray<UDynamicObject*> SelectionSetCache;
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
			if (!meshNames.Contains(dynamicComponent->MeshName))
			{
				SelectionSetCache.Add(dynamicComponent);
				meshNames.Add(dynamicComponent->MeshName);
			}
		}
	}

	ExportDynamicObjectArray(SelectionSetCache);

	return FReply::Handled();
}

void FCognitiveEditorTools::ExportDynamicObjectArray(TArray<UDynamicObject*> exportObjects)
{
	if (!HasFoundBlender())
	{
		UE_LOG(CognitiveVR_Log, Error, TEXT("Could not complete Export - Must have Blender installed to convert images"));
		return;
	}

	FVector originalLocation;
	FRotator originalRotation;
	FVector originalScale;
	int32 ActorsExported = 0;

	TArray<FString> DynamicMeshNames;

	for (int32 i = 0; i < exportObjects.Num(); i++)
	{
		DynamicMeshNames.Add(exportObjects[i]->MeshName);

		GEditor->SelectNone(false, true, false);

		if (exportObjects[i] == NULL)
		{
			continue;
		}
		if (exportObjects[i]->GetOwner() == NULL)
		{
			continue;
		}
		originalLocation = exportObjects[i]->GetOwner()->GetActorLocation();
		originalRotation = exportObjects[i]->GetOwner()->GetActorRotation();
		//originalScale = tempactor->GetActorScale();

		exportObjects[i]->GetOwner()->SetActorLocation(FVector::ZeroVector);
		exportObjects[i]->GetOwner()->SetActorRotation(FQuat::Identity);
		//tempactor->SetActorScale3D(originalScale*0.01);

		FString ExportFilename = exportObjects[i]->MeshName + ".obj";

		GEditor->SelectActor(exportObjects[i]->GetOwner(), true, false, true);
		//ActorsExported++;

		FString tempObject = GetDynamicsExportDirectory() + "/" + exportObjects[i]->MeshName + "/" + exportObjects[i]->MeshName + ".obj";

		GLog->Log("FCognitiveEditorTools::ExportDynamicObjectArray dynamic output directory " + tempObject);
		GLog->Log("FCognitiveEditorTools::ExportDynamicObjectArray exporting DynamicObject " + ExportFilename);
		
		GUnrealEd->ExportMap(GWorld, *tempObject, true);

		exportObjects[i]->GetOwner()->SetActorLocation(originalLocation);
		exportObjects[i]->GetOwner()->SetActorRotation(originalRotation);
		//tempactor->SetActorScale3D(originalScale);

		UActorComponent* actorComponent = exportObjects[i]->GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass());

		UStaticMeshComponent* mesh = Cast<UStaticMeshComponent>(actorComponent);

		if (mesh != NULL)
		{
			//bake + export materials
			TArray< UStaticMeshComponent*> meshes;
			meshes.Add(mesh);
			WizardExportMaterials(GetDynamicsExportDirectory() + "/" + exportObjects[i]->MeshName + "/", meshes);
		}
		//List_MaterialArgs(exportObjects[i]->MeshName, GetDynamicsExportDirectory());
	}
	GLog->Log("FCognitiveEditorTools::ExportDynamicObjectArray Found " + FString::FromInt(ActorsExported) + " meshes for export");



	//convert all the dynamic objs to gltf

	//TODO export transparent textures for dynamic objects
	//
	//ConvertDynamicTextures();

	ConvertDynamicsToGLTF(DynamicMeshNames);

	FindAllSubDirectoryNames();
}

void FCognitiveEditorTools::ConvertDynamicsToGLTF(TArray<FString> meshnames)
{
	//open blender
	//pass in array of meshnames comma separated

	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ConvertDynamicToGLTF.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::ConvertToGLTF Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();


	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::ConvertToGLTF Could not find python script at path. Canceling"));
		return;
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::ConvertToGLTF No path set for Blender.exe. Canceling"));
		return;
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::ConvertToGLTF World is null. canceling"));
		return;
	}

	FString ObjPath = GetDynamicsExportDirectory();

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::ConvertToGLTF No know export directory. Canceling"));
		return;
	}

	FString escapedMeshNameList = "'";
	for (int i = 0; i < meshnames.Num(); i++)
	{
		if (i != 0)
			escapedMeshNameList += ",";
		escapedMeshNameList += meshnames[i];
	}
	escapedMeshNameList += "'";


	//FString resizeFactor = FString::FromInt(TextureRefactor);

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedTargetPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString stringparams = " -P " + escapedPythonPath + " " + escapedTargetPath + " " + escapedMeshNameList;// +" " + MaxPolyCount + " " + SceneName;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	//UE_LOG(LogTemp, Warning, TEXT("Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle process = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);
}

FReply FCognitiveEditorTools::SetUniqueDynamicIds()
{
	//loop thorugh all dynamics in the scene
	TArray<UDynamicObject*> dynamics;

	//make a list of all the used objectids

	TArray<cognitivevrapi::FDynamicObjectId> usedIds;

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

		cognitivevrapi::FDynamicObjectId* FoundId = usedIds.FindByPredicate([findId](const cognitivevrapi::FDynamicObjectId& InItem)
		{
			return InItem.Id == findId;
		});

		if (FoundId == NULL && dynamic->CustomId != "")
		{
			usedIds.Add(cognitivevrapi::FDynamicObjectId(dynamic->CustomId, dynamic->MeshName));
		}
		else
		{
			//assign a new and unused id
			UnassignedDynamics.Add(dynamic);
		}
	}

	for (auto& dynamic : UnassignedDynamics)
	{
		dynamic->GenerateCustomId();
		usedIds.Add(cognitivevrapi::FDynamicObjectId(dynamic->CustomId, dynamic->MeshName));
		changedDynamics++;
	}

	GLog->Log("CognitiveVR Tools set " + FString::FromInt(changedDynamics) + " dynamic ids");

	GWorld->MarkPackageDirty();
	//save the scene? mark the scene as changed?

	RefreshDisplayDynamicObjectsCountInScene();

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

	GLog->Log("CognitiveVR Tools uploading manifest for " +FString::FromInt(dynamics.Num())+ " objects");

	bool wroteAnyObjects = false;
	FString objectManifest = "{\"objects\":[";
	//write preset customids to manifest
	for (int32 i = 0; i < dynamics.Num(); i++)
	{
		//if they have a customid -> add them to the objectmanifest string
		if (dynamics[i]->UseCustomId && dynamics[i]->CustomId != "")
		{
			wroteAnyObjects = true;
			objectManifest += "{";
			objectManifest += "\"id\":\"" + dynamics[i]->CustomId + "\",";
			objectManifest += "\"mesh\":\"" + dynamics[i]->MeshName + "\",";
			objectManifest += "\"name\":\"" + dynamics[i]->GetOwner()->GetName() + "\"";
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
	TSharedPtr<cognitivevrapi::FEditorSceneData> currentSceneData = GetCurrentSceneData();
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
	FString AuthValue = "APIKEY:DEVELOPER " + FAnalyticsCognitiveVR::Get().DeveloperKey;
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
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
	if (bWasSuccessful)
		GLog->Log("FCognitiveEditorTools::OnUploadManifestCompleted response code " + FString::FromInt(Response->GetResponseCode()));
	else
	{
		GLog->Log("FCognitiveEditorTools::OnUploadManifestCompleted failed to connect");
		WizardUploading = false;
		WizardUploadError = "FCognitiveEditorTools::OnUploadManifestCompleted failed to connect";
		return;
	}

	if (bWasSuccessful && Response->GetResponseCode() < 300)
	{
		GetDynamicsManifest();
	}
	else
	{
		WizardUploading = false;
		WizardUploadError = FString::FromInt(Response->GetResponseCode());
	}

	if (WizardUploading)
	{
		WizardUploading = false;
	}
}

FReply FCognitiveEditorTools::GetDynamicsManifest()
{
	TSharedPtr<cognitivevrapi::FEditorSceneData> currentSceneData = GetCurrentSceneData();
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

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(GetDynamicObjectManifest(FString::FromInt(currentSceneData->VersionId)));

	HttpRequest->SetHeader("X-HTTP-Method-Override", TEXT("GET"));
	FString AuthValue = "APIKEY:DEVELOPER " + FAnalyticsCognitiveVR::Get().DeveloperKey;
	HttpRequest->SetHeader("Authorization", AuthValue);

	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FCognitiveEditorTools::OnDynamicManifestResponse);
	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

void FCognitiveEditorTools::OnDynamicManifestResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
		GLog->Log("FCognitiveEditorTools::OnDynamicManifestResponse response code " + FString::FromInt(Response->GetResponseCode()));
	else
	{
		GLog->Log("FCognitiveEditorTools::OnDynamicManifestResponse failed to connect");
		WizardUploading = false;
		WizardUploadError = "FCognitiveEditorTools::OnDynamicManifestResponse failed to connect";
		return;
	}

	if (bWasSuccessful && Response->GetResponseCode()<300)
	{
		//GLog->Log("CognitiveTools::OnDynamicManifestResponse content: " + Response->GetContentAsString());

		//do json stuff to this

		TSharedPtr<FJsonValue> JsonDynamics;

		TSharedRef<TJsonReader<>>Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonDynamics))
		{
			int32 count = JsonDynamics->AsArray().Num();
			GLog->Log("FCognitiveEditorTools::OnDynamicManifestResponse returned " + FString::FromInt(count) + " objects");
			for (int i = 0; i < count; i++)
			{
				TSharedPtr<FJsonObject> jsonobject = JsonDynamics->AsArray()[i]->AsObject();
				FString name = jsonobject->GetStringField("name");
				FString meshname = jsonobject->GetStringField("meshName");
				FString id = jsonobject->GetStringField("sdkId");

				SceneExplorerDynamics.Add(MakeShareable(new cognitivevrapi::FDynamicData(name, meshname, id)));
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
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	TArray<FString> DirectoriesToSkip;
	TArray<FString> DirectoriesToNotRecurse;

	// use the timestamp grabbing visitor (include directories)
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToNotRecurse, true);
	Visitor.Visit(*GetDynamicsExportDirectory(), true);

	TArray<FString> dynamicNames;
	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		if (TimestampIt.Key() == GetDynamicsExportDirectory() || (!TimestampIt.Key().EndsWith(".png") && !TimestampIt.Key().EndsWith(".obj") && !TimestampIt.Key().EndsWith(".mtl")))
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
	TSharedPtr<cognitivevrapi::FEditorSceneData> currentSceneData = GetCurrentSceneData();

	if (!currentSceneData.IsValid())
	{
		GLog->Log("FCognitiveEditorToolsCustomization::UploadDynamics can't find current scene!");
		return FReply::Handled();
	}

	//no matches anywhere
	//CognitiveLog::Warning("UPlayerTracker::GetSceneKey ------- no matches in ini");

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

	return FReply::Handled();
}

FReply FCognitiveEditorTools::UploadDynamic(FString directory)
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");

	// Get all files in directory
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	TArray<FString> DirectoriesToSkip;
	TArray<FString> DirectoriesToNotRecurse;

	FString singleDir = GetDynamicsExportDirectory()+"/" + directory;

	// use the timestamp grabbing visitor (include directories)
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToNotRecurse, true);
	Visitor.Visit(*singleDir, true);

	TArray<FString> dynamicNames;
	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		if (TimestampIt.Key().EndsWith(".png") || TimestampIt.Key().EndsWith(".obj") || TimestampIt.Key().EndsWith(".mtl"))
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
	TSharedPtr<cognitivevrapi::FEditorSceneData> currentSceneData = GetCurrentSceneData();

	if (!currentSceneData.IsValid())
	{
		GLog->Log("FCognitiveEditorToolsCustomization::UploadDynamics can't find current scene!");
		return FReply::Handled();
	}

	//no matches anywhere
	//CognitiveLog::Warning("UPlayerTracker::GetSceneKey ------- no matches in ini");

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
		GLog->Log("FCognitiveEditorTools::UploadDynamics uploaded a thing");
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
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

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

FReply FCognitiveEditorTools::Select_Blender()
{
	FString title = "Select Blender.exe";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickFile(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		BlenderPath = outFilename;
	}
	return FReply::Handled();
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
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::Select_Export_Directory - picked a directory"));
		BaseExportDirectory = outFilename;
	}
	else
	{
		GLog->Log("FCognitiveEditorTools::Select_Export_Directory cancelled?");
		BaseExportDirectory = "";
	}
	return FReply::Handled();
}


//used to select blender
bool FCognitiveEditorTools::PickFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
{
	OutFilename = FString();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bFileChosen = false;
	TArray<FString> OutFilenames;
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = ChooseParentWindowHandle();

		bFileChosen = DesktopPlatform->OpenFileDialog(
			ParentWindowWindowHandle,
			Title,
			InOutLastPath,
			DefaultFile,
			FileTypes,
			EFileDialogFlags::None,
			OutFilenames
		);
	}

	bFileChosen = (OutFilenames.Num() > 0);

	if (bFileChosen)
	{
		// User successfully chose a file; remember the path for the next time the dialog opens.
		InOutLastPath = OutFilenames[0];
		OutFilename = OutFilenames[0];
	}

	return bFileChosen;
}

FReply FCognitiveEditorTools::TakeScreenshot()
{
	FString dir = BaseExportDirectory+"/"+GetCurrentSceneName()+"/screenshot/";
	if (VerifyOrCreateDirectory(dir))
	{
		FScreenshotRequest::RequestScreenshot(dir + "screenshot", false, false);
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
	TSharedPtr<cognitivevrapi::FEditorSceneData> sceneData = GetCurrentSceneData();
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
	//TODO listen for response. when the response returns, request the scene version with auth token
	UploadFromDirectory(url, GetCurrentSceneExportDirectory(), "scene");

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

void FCognitiveEditorTools::RefreshDynamicUploadFiles()
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(BaseExportDirectory+"/dynamics", true, filesStartingWith, filesStartingWith, pngextension, false);

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

void FCognitiveEditorTools::UploadFromDirectory(FString url, FString directory, FString expectedResponseType)
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(directory, true, filesStartingWith, filesStartingWith, filesStartingWith,true);

	//TArray<FString> imagesInDirectory = GetAllFilesInDirectory(directory, true, filesStartingWith, pngextension, filesStartingWith, true);

	filesInDirectory.Remove(directory + "/screenshot/screenshot.png");
	//imagesInDirectory.Remove(directory + "/screenshot/screenshot.png");
	FString screenshotPath = directory + "/screenshot/screenshot.png";

	TArray<cognitivevrapi::FContentContainer> contentArray;

	//UE_LOG(LogTemp, Log, TEXT("UploadScene image count%d"), imagesInDirectory.Num());
	UE_LOG(LogTemp, Log, TEXT("UploadScene all file count%d"), filesInDirectory.Num());

	for (int32 i = 0; i < filesInDirectory.Num(); i++)
	{
		FString Content;
		TArray<uint8> byteResult;
		if (FFileHelper::LoadFileToArray(byteResult, *filesInDirectory[i]))
		{
			cognitivevrapi::FContentContainer container = cognitivevrapi::FContentContainer();
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
			cognitivevrapi::FContentContainer container = cognitivevrapi::FContentContainer();
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
			GLog->Log("couldn't find screenshot to upload");
		}
	}
	else
	{
		GLog->Log("screenshot path doesn't exist -------- " + screenshotPath);
	}

	TArray<uint8> AllBytes;
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

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
	FString AuthValue = "APIKEY:DEVELOPER " + FAnalyticsCognitiveVR::Get().DeveloperKey;
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
		return;
	}

	if (bWasSuccessful && Response->GetResponseCode() < 300)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Upload Scene Response is %s"), *Response->GetContentAsString());

		UWorld* myworld = GWorld->GetWorld();
		if (myworld == NULL)
		{
			UE_LOG(LogTemp, Warning, TEXT("Upload Scene - No world!"));
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
			//ReadSceneDataFromFile();

			//UploadDynamics();
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
		GLog->Log("FCognitiveEditorTools::OnUploadObjectCompleted response code " + FString::FromInt(Response->GetResponseCode()));
	else
	{
		GLog->Log("FCognitiveEditorTools::OnUploadObjectCompleted failed to connect");
		WizardUploading = false;
		WizardUploadError = "FCognitiveEditorTools::OnUploadObjectCompleted failed to connect";
		return;
	}

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
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
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

bool FCognitiveEditorTools::HasFoundBlender() const
{
	if (!HasDeveloperKey()) { return false; }
	return FCognitiveEditorTools::GetBlenderPath().ToString().Contains("blender.exe");
}

bool FCognitiveEditorTools::HasFoundBlenderAndHasSelection() const
{
	if (!HasDeveloperKey()) { return false; }
	return FCognitiveEditorTools::GetBlenderPath().ToString().Contains("blender.exe") && GEditor->GetSelectedActorCount() > 0;
}

//checks for json and no bmps files in export directory
bool FCognitiveEditorTools::HasConvertedFilesInDirectory() const
{
	if (!HasSetExportDirectory()) { return false; }

	FString filesStartingWith = TEXT("");
	FString bmpextension = TEXT("bmp");
	FString jsonextension = TEXT("json");

	TArray<FString> imagesInDirectory = GetAllFilesInDirectory(BaseExportDirectory, true, filesStartingWith, bmpextension, filesStartingWith,false);
	TArray<FString> jsonInDirectory = GetAllFilesInDirectory(BaseExportDirectory, true, filesStartingWith, jsonextension, filesStartingWith, false);
	if (imagesInDirectory.Num() > 0) { return false; }
	if (jsonInDirectory.Num() == 0) { return false; }

	return true;
}

bool FCognitiveEditorTools::CanUploadSceneFiles() const
{
	return HasConvertedFilesInDirectory() && HasDeveloperKey();
}

bool FCognitiveEditorTools::LoginAndCustonerIdAndBlenderExportDir() const
{
	return HasDeveloperKey() && HasFoundBlenderAndExportDir();
}

bool FCognitiveEditorTools::HasFoundBlenderAndExportDir() const
{
	if (GetBaseExportDirectory().Len() == 0) { return false; }
	return FCognitiveEditorTools::GetBlenderPath().ToString().Contains("blender.exe");
}

bool FCognitiveEditorTools::HasFoundBlenderAndDynamicExportDir() const
{
	if (GetBaseExportDirectory().Len() == 0) { return false; }
	return FCognitiveEditorTools::GetBlenderPath().ToString().Contains("blender.exe");
}

bool FCognitiveEditorTools::HasFoundBlenderDynamicExportDirSelection() const
{
	if (GetBaseExportDirectory().Len() == 0) { return false; }
	if (GEditor->GetSelectedActorCount() == 0) { return false; }
	return FCognitiveEditorTools::GetBlenderPath().ToString().Contains("blender.exe");
}

bool FCognitiveEditorTools::CurrentSceneHasSceneId() const
{
	if (!HasDeveloperKey()) { return false; }
	TSharedPtr<cognitivevrapi::FEditorSceneData> currentscene = GetCurrentSceneData();
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
	return !FCognitiveEditorTools::GetBaseExportDirectory().Len() == 0;
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
		return FText::FromString("Scene does not have valid data. Must export your scene before uploading dynamics!");
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

bool FCognitiveEditorTools::HasSetDynamicExportDirectoryHasSceneId() const
{
	if (!HasDeveloperKey()) { return false; }
	auto scenedata = GetCurrentSceneData();
	if (!scenedata.IsValid()) { return false; }
	if (GetBaseExportDirectory().Len() == 0) { return false; }
	return true;
}

bool FCognitiveEditorTools::HasFoundBlenderHasSelection() const
{
	if (GEditor->GetSelectedActorCount() == 0) { return false; }
	return HasFoundBlender();
}

FText FCognitiveEditorTools::GetBlenderPath() const
{
	return FText::FromString(BlenderPath);
}

void FCognitiveEditorTools::SearchForBlender()
{
	//try to find blender in program files
	FString testApp = "C:/Program Files/Blender Foundation/Blender/blender.exe";

	if (VerifyFileExists(testApp))
	{
		UE_LOG(LogTemp, Warning, TEXT("SearchForBlender - Found Blender in Program Files"));
		BlenderPath = testApp;
	}
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
	DynamicCountInScene = FText::FromString("Found "+ FString::FromInt(CountDynamicObjectsInScene()) + " Dynamic Objects in scene");
	DuplicateDyanmicObjectVisibility = EVisibility::Hidden;
	//SceneDynamicObjectList->RefreshList();

	GLog->Log("FCognitiveEditorTools::RefreshDisplayDynamicObjectsCountInScene");

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
		SceneDynamics.Add(MakeShareable(new cognitivevrapi::FDynamicData(dynamic->GetOwner()->GetName(), dynamic->MeshName, dynamic->CustomId)));
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
	TArray<cognitivevrapi::FDynamicObjectId> usedIds;

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

		cognitivevrapi::FDynamicObjectId* FoundId = usedIds.FindByPredicate([findId](const cognitivevrapi::FDynamicObjectId& InItem)
		{
			return InItem.Id == findId;
		});

		if (FoundId == NULL && dynamic->CustomId != "")
		{
			usedIds.Add(cognitivevrapi::FDynamicObjectId(dynamic->CustomId, dynamic->MeshName));
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


void FCognitiveEditorTools::OnAPIKeyChanged(const FText& Text)
{
	APIKey = Text.ToString();
	//FAnalyticsCognitiveVR::GetCognitiveVRProvider()->APIKey = APIKey;
	//FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "ApiKey", false);
}

FText FCognitiveEditorTools::GetAPIKey() const
{
	return FText::FromString(APIKey);
}

FText FCognitiveEditorTools::GetDeveloperKey() const
{
	return FText::FromString(FAnalyticsCognitiveVR::Get().DeveloperKey);
}

void FCognitiveEditorTools::OnDeveloperKeyChanged(const FText& Text)
{
	FAnalyticsCognitiveVR::Get().DeveloperKey = Text.ToString();
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

	return FText::FromString("Upload " + FString::FromInt(SubDirectoryNames.Num()) + " Dynamic Object Meshes to " + data->Name + " Version " + FString::FromInt(data->VersionNumber));
}

FText FCognitiveEditorTools::GetDynamicsFromManifest() const
{
	return FText::FromString("DYNAMICS");
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
	TSharedPtr<cognitivevrapi::FEditorSceneData> scenedata = GetCurrentSceneData();

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
	TSharedPtr<cognitivevrapi::FEditorSceneData> scenedata = GetCurrentSceneData();

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

	for (int i = 0; i < scenstrings.Num(); i++)
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

		cognitivevrapi::FEditorSceneData* tempscene = new cognitivevrapi::FEditorSceneData(Array[0], Array[1], FCString::Atoi(*Array[2]), FCString::Atoi(*Array[3]));
		SceneData.Add(MakeShareable(tempscene));
	}

	GLog->Log("FCognitiveTools::RefreshSceneData found this many scenes: " + FString::FromInt(SceneData.Num()));
	//ConfigFileHasChanged = true;

	//return FReply::Handled();
}

void FCognitiveEditorTools::SceneVersionRequest(cognitivevrapi::FEditorSceneData data)
{
	if (!HasDeveloperKey())
	{
		GLog->Log("FCognitiveTools::SceneVersionRequest no developer key set!");
		//auto httprequest = RequestAuthTokenCallback();
		
		return;
	}

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(GetSceneVersion(data.Id));

	HttpRequest->SetHeader("X-HTTP-Method-Override", TEXT("GET"));
	//HttpRequest->SetHeader("Authorization", TEXT("Data " + FAnalyticsCognitiveVR::Get().EditorAuthToken));
	FString AuthValue = "APIKEY:DEVELOPER " + FAnalyticsCognitiveVR::Get().DeveloperKey;
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
		return;
	}

	int32 responseCode = Response->GetResponseCode();

	if (responseCode >= 500)
	{
		//internal server error
		GLog->Log("FCognitiveTools::SceneVersionResponse 500-ish internal server error");
		WizardUploadError = "FCognitiveEditorTools::SceneVersionResponse response code " + FString::FromInt(Response->GetResponseCode());
		return;
	}
	if (responseCode >= 400)
	{
		WizardUploading = false;
		if (responseCode == 401)
		{
			//not authorized
			GLog->Log("FCognitiveTools::SceneVersionResponse not authorized!");
			WizardUploadError = "FCognitiveEditorTools::SceneVersionResponse response code " + FString::FromInt(Response->GetResponseCode());
			//DEBUG_RequestAuthToken();
			//auto httprequest = RequestAuthTokenCallback();
			//if (httprequest)
			//{
			
			//}
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

	//parse response content to json

	TSharedPtr<FJsonObject> JsonSceneSettings;

	TSharedRef<TJsonReader<>>Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(Reader, JsonSceneSettings))
	{
		//get the latest version of the scene
		int32 versionNumber = 0;
		int32 versionId = 0;
		TArray<TSharedPtr<FJsonValue>> versions = JsonSceneSettings->GetArrayField("versions");
		for (int i = 0; i < versions.Num(); i++) {

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
		TSharedPtr<cognitivevrapi::FEditorSceneData> currentSceneData = GetCurrentSceneData();
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
		for (int i = 0; i < iniscenedata.Num(); i++)
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
			WizardUploading = false;
		}
	}
}

TArray<TSharedPtr<cognitivevrapi::FEditorSceneData>> FCognitiveEditorTools::GetSceneData() const
{
	return SceneData;
}

ECheckBoxState FCognitiveEditorTools::HasFoundBlenderCheckbox() const
{
	return (HasFoundBlender())
		? ECheckBoxState::Checked
		: ECheckBoxState::Unchecked;
}

FReply FCognitiveEditorTools::SaveAPIDeveloperKeysToFile()
{
	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));
	//GLog->Log("FCognitiveTools::SaveAPIDeveloperKeysToFile save: " + CustomerId);

	//GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
	GConfig->SetString(TEXT("Analytics"), TEXT("ApiKey"), *APIKey, EngineIni);

	GConfig->SetString(TEXT("Analytics"), TEXT("DeveloperKey"), *FAnalyticsCognitiveVR::Get().DeveloperKey, EditorIni);

	GConfig->Flush(false, GEngineIni);

	ConfigFileHasChanged = true;

	return FReply::Handled();
}

void FCognitiveEditorTools::SaveAPIKeyToFile(FString key)
{
	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->SetString(TEXT("Analytics"), TEXT("ApiKey"), *key, EngineIni);
	GConfig->Flush(false, GEngineIni);
	ConfigFileHasChanged = true;

	APIKey = key;
}

void FCognitiveEditorTools::SaveDeveloperKeyToFile(FString key)
{
	//FString EngineIni = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
	FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));
	//GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
	GConfig->SetString(TEXT("Analytics"), TEXT("DeveloperKey"), *key, EditorIni);
	GConfig->Flush(false, GEngineIni);
	ConfigFileHasChanged = true;

	FAnalyticsCognitiveVR::Get().DeveloperKey = key;
}

TSharedPtr<cognitivevrapi::FEditorSceneData> FCognitiveEditorTools::GetCurrentSceneData() const
{
	UWorld* myworld = GWorld->GetWorld();

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	return GetSceneData(currentSceneName);
}

FString lastSceneName;
TSharedPtr<cognitivevrapi::FEditorSceneData> FCognitiveEditorTools::GetSceneData(FString scenename) const
{
	for (int i = 0; i < SceneData.Num(); i++)
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

TArray<FString> MaterialLine;

void FCognitiveEditorTools::WizardExportMaterials(FString directory, TArray<UStaticMeshComponent*> meshes)
{
	//TODO figure out how to deal with name collisions

	//UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	//FMaterialBakingModule& BakingModule = FModuleManager::GetModuleChecked<FMaterialBakingModule>(TEXT("MaterialBaking"));
	IMaterialBakingModule& MaterialBakingModule = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");

	FIntPoint resolution = FIntPoint(256, 256);

	TArray<FString>ExportedMaterialNames;
	
	//FString dynamicDirectory = BaseExportDirectory + "/Dynamics/";
	//FString searchDirectory = BaseExportDirectory + "/" + GetCurrentSceneName() + "/";



	//for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
	//{

	MaterialLine.Empty();
	MaterialLine.Add("");

	for (int i = 0; i < meshes.Num(); i++)
	{
		if (meshes[i] == NULL) { continue; }
		UStaticMeshComponent* TempObject = meshes[i];
		if (TempObject == NULL) { continue; }
		TArray<UMaterialInterface*> mats = TempObject->GetMaterials();
		for (int i = 0; i < mats.Num(); i++)
		{
			if (mats[i] != NULL)
			{
				if (ExportedMaterialNames.Contains(mats[i]->GetName())) { continue; }

				TArray<FMeshData*> MeshSettingPtrs;
				TArray<FMaterialData*> MaterialSettingPtrs;
				GLog->Log(mats[i]->GetFullName());
				FString n = mats[i]->GetPathName().Replace(TEXT("."), TEXT("_"));
				MaterialLine.Add("newmtl " + n);

				ExportedMaterialNames.Add(mats[i]->GetName());

				FMaterialData MaterialSettings;
				MaterialSettings.Material = mats[i];
				//MaterialSettings.bPerformBorderSmear = false;
				MaterialSettings.PropertySizes.Add(EMaterialProperty::MP_BaseColor, resolution);
				MaterialSettings.PropertySizes.Add(EMaterialProperty::MP_Normal, resolution);

				if (mats[i]->GetBlendMode() == EBlendMode::BLEND_Masked)
					MaterialSettings.PropertySizes.Add(EMaterialProperty::MP_OpacityMask, resolution);
				else if (mats[i]->GetBlendMode() == EBlendMode::BLEND_Translucent)
					MaterialSettings.PropertySizes.Add(EMaterialProperty::MP_Opacity, resolution);
				//MaterialSettings.PropertySizes.Add(EMaterialProperty::MP_EmissiveColor, resolution);
				MaterialSettingPtrs.Add(&MaterialSettings);



				FMeshData meshdata;
				//meshdata.RawMeshDescription = nullptr;
				meshdata.TextureCoordinateBox = FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));
				meshdata.TextureCoordinateIndex = 0;
				MeshSettingPtrs.Add(&meshdata);

				TArray<FBakeOutput> BakeOutputs;
				MaterialBakingModule.BakeMaterials(MaterialSettingPtrs, MeshSettingPtrs, BakeOutputs);

				for (FBakeOutput& output : BakeOutputs)
				{
					//GLog->Log(FString::FromInt(output.PropertySizes[EMaterialProperty::MP_BaseColor].X) + "   " + FString::FromInt(output.PropertySizes[EMaterialProperty::MP_BaseColor].Y));

					//writing materials with wrong uvs

					FString BMPFilename;
					//diffuse
					BMPFilename = directory + mats[i]->GetName().Replace(TEXT("."), TEXT("_")) + TEXT("_D.bmp");
					FFileHelper::CreateBitmap(*BMPFilename, resolution.X, resolution.Y, output.PropertyData[EMaterialProperty::MP_BaseColor].GetData());
					MaterialLine.Add("map_Kd " + BMPFilename);

					//normal
					BMPFilename = directory + mats[i]->GetName().Replace(TEXT("."), TEXT("_")) + TEXT("_N.bmp");
					FFileHelper::CreateBitmap(*BMPFilename, resolution.X, resolution.Y, output.PropertyData[EMaterialProperty::MP_Normal].GetData());
					MaterialLine.Add("bump " + BMPFilename);

					if (mats[i]->GetBlendMode() == EBlendMode::BLEND_Masked)
					{
						//	//mask
						BMPFilename = directory + mats[i]->GetName().Replace(TEXT("."), TEXT("_")) + TEXT("_OM.bmp");
						FFileHelper::CreateBitmap(*BMPFilename, resolution.X, resolution.Y, output.PropertyData[EMaterialProperty::MP_OpacityMask].GetData());
						MaterialLine.Add("illum 4");
						MaterialLine.Add("map_D " + BMPFilename);
					}
					else if (mats[i]->GetBlendMode() == EBlendMode::BLEND_Translucent)
					{
						//opacity
						BMPFilename = directory + mats[i]->GetName().Replace(TEXT("."), TEXT("_")) + TEXT("_O.bmp");
						FFileHelper::CreateBitmap(*BMPFilename, resolution.X, resolution.Y, output.PropertyData[EMaterialProperty::MP_Opacity].GetData());
						MaterialLine.Add("illum 4");
						MaterialLine.Add("map_D " + BMPFilename);
					}

					//save to disk
				}
				MaterialLine.Add("");
				MaterialLine.Add("");
			}
		}
	}


	//write MaterialLine over top mtl file
	//IFileManager& FileManager = IFileManager::Get();


	FString mtlfilename = directory + GetCurrentSceneName() + ".mtl";
	FFileHelper::SaveStringArrayToFile(MaterialLine, *mtlfilename);

	//if (FFileHelper::LoadFileToString(contents, *fullPath))

}

void FCognitiveEditorTools::WizardPostSceneExport()
{
	//anything that needs to happen automatically after the scene has been exported
	//TODO exporting 
}

void FCognitiveEditorTools::WizardConvertScene()
{
	FProcHandle fph = ConvertSceneToGLTF();

	if (fph.IsValid())
	{
		FPlatformProcess::WaitForProc(fph);

		TakeScreenshot();
	}
}

void FCognitiveEditorTools::WizardUpload()
{
	WizardUploading = true;
	WizardUploadError = "";
	UploadScene();

	//scene
	//scene version
	//dynamics meshes
	//dynamic manifest
}

//run this as the next step after exporting the scene
FProcHandle FCognitiveEditorTools::ConvertSceneToGLTF()
{
	FProcHandle BlenderReduceAllWizardProc;

	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ReduceWizardExportedScene.py");
	const TCHAR* charPath = *pythonscriptpath;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::Reduce_Meshes_And_Textures - Could not find script at path. Canceling"));
		return BlenderReduceAllWizardProc;
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::Reduce_Meshes_And_Textures - No path set for Blender.exe. Canceling"));
		return BlenderReduceAllWizardProc;
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::Reduce_Meshes_And_Textures - World is null. canceling"));
		return BlenderReduceAllWizardProc;
	}

	FString SceneName = tempworld->GetMapName();
	FString ObjPath = FPaths::Combine(BaseExportDirectory, GetCurrentSceneName());

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::Reduce_Meshes_And_Textures No know export directory. Canceling"));
		return BlenderReduceAllWizardProc;
	}

	FString MinPolyCount = FString::FromInt(MinPolygon);
	FString MaxPolyCount = FString::FromInt(MaxPolygon);

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString escapedExcludeMeshes = "IGNOREEXCLUDEMESHES";

	FConfigSection* ExportSettings = GConfig->GetSectionPrivate(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), false, true, GEngineIni);
	if (ExportSettings != NULL)
	{
		for (FConfigSection::TIterator It(*ExportSettings); It; ++It)
		{
			if (It.Key() == TEXT("ExcludeMeshes"))
			{
				FName nameEscapedExcludeMeshes;
				escapedExcludeMeshes = It.Value().GetValue();
				break;
			}
		}
	}

	escapedExcludeMeshes = escapedExcludeMeshes.Replace(TEXT(" "), TEXT("\" \""));

	FString productID = "UNUSED";

	FString stringparams = " -P " + escapedPythonPath + " " + escapedOutPath + " " + MinPolyCount + " " + MaxPolyCount + " " + SceneName + " " + productID + " " + COGNITIVEVR_SDK_VERSION + " " + escapedExcludeMeshes;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	UE_LOG(LogTemp, Warning, TEXT("FCognitiveEditorTools::Reduce_Meshes_And_Textures Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	BlenderReduceAllWizardProc = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);

	//TODO when procHandle is complete, upload exported files to sceneexplorer.com
	return BlenderReduceAllWizardProc;
}


FText FCognitiveEditorTools::GetBaseExportDirectoryDisplay() const
{
	return FText::FromString(BaseExportDirectory);
}

//c:/users/me/desktop/export/scenename/
FText FCognitiveEditorTools::GetSceneExportDirectoryDisplay(FString scenename) const
{
	return FText::FromString(FPaths::Combine(BaseExportDirectory,scenename));
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

EVisibility FCognitiveEditorTools::BlenderValidVisibility() const
{
	if (HasFoundBlender())
		return EVisibility::Visible;
	return EVisibility::Collapsed;
}

EVisibility FCognitiveEditorTools::BlenderInvalidVisibility() const
{
	if (HasFoundBlender())
		return EVisibility::Collapsed;
	return EVisibility::Visible;
}

#undef LOCTEXT_NAMESPACE