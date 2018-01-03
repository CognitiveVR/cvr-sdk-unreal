
#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveTools.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

//deals with all the exporting and non-display stuff in the editor preferences

float FCognitiveTools::GetMinimumSize()
{
	float MinSize = 0;
	MinSizeProperty->GetValue(MinSize);
	return MinSize;
}

float FCognitiveTools::GetMaximumSize()
{
	float MaxSize = 0;
	MaxSizeProperty->GetValue(MaxSize);
	return MaxSize;
}

bool FCognitiveTools::GetStaticOnly()
{
	bool staticOnly = false;
	StaticOnlyProperty->GetValue(staticOnly);
	return staticOnly;
}

int32 FCognitiveTools::GetMinPolygon()
{
	int32 MinCount = 0;
	MinPolygonProperty->GetValue(MinCount);
	return MinCount;
}

int32 FCognitiveTools::GetMaxPolygon()
{
	int32 MaxCount = 0;
	MaxPolygonProperty->GetValue(MaxCount);
	return MaxCount;
}

int32 FCognitiveTools::GetTextureRefacor()
{
	int32 TextureRefactor = 0;
	TextureResizeProperty->GetValue(TextureRefactor);
	return TextureRefactor;
}

FReply FCognitiveTools::ExportDynamics()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveToolsCustomization::ExportDynamics world is null"));
		return FReply::Handled();
	}

	FString title = "Select Root Dynamic Directory";
	FString fileTypes = "";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveToolsCustomization::ExportDynamics - picked a directory"));
		ExportDynamicsDirectory = outFilename;
	}
	else
	{
		return FReply::Handled();
	}

	TArray<FString> meshNames;
	TArray<UDynamicObject*> exportObjects;

	for (TActorIterator<AStaticMeshActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		AStaticMeshActor *Mesh = *ActorItr;
		
		UActorComponent* actorComponent = Mesh->GetComponentByClass(UDynamicObject::StaticClass());
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

FReply FCognitiveTools::ExportSelectedDynamics()
{
	UWorld* World = GWorld;
	FString title = "Select Root Dynamic Directory";
	FString fileTypes = "";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveToolsCustomization::ExportDynamics - picked a directory"));
		ExportDynamicsDirectory = outFilename;
	}
	else
	{
		return FReply::Handled();
	}

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

void FCognitiveTools::ExportDynamicObjectArray(TArray<UDynamicObject*> exportObjects)
{
	FVector originalLocation;
	FRotator originalRotation;
	FVector originalScale;
	int32 ActorsExported = 0;

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
		originalLocation = exportObjects[i]->GetOwner()->GetActorLocation();
		originalRotation = exportObjects[i]->GetOwner()->GetActorRotation();
		//originalScale = tempactor->GetActorScale();

		exportObjects[i]->GetOwner()->SetActorLocation(FVector::ZeroVector);
		exportObjects[i]->GetOwner()->SetActorRotation(FQuat::Identity);
		//tempactor->SetActorScale3D(originalScale*0.01);

		FString ExportFilename = exportObjects[i]->MeshName + ".obj";

		GEditor->SelectActor(exportObjects[i]->GetOwner(), true, false, true);
		//ActorsExported++;

		ExportDynamicsDirectory += "/" + exportObjects[i]->MeshName + "/" + exportObjects[i]->MeshName + ".obj";

		GLog->Log("FCognitiveTools::ExportDynamicObjectArray root output directory " + ExportDynamicsDirectory);
		GLog->Log("FCognitiveTools::ExportDynamicObjectArray dynamic output directory " + ExportDynamicsDirectory);
		GLog->Log("FCognitiveTools::ExportDynamicObjectArray exporting DynamicObject " + ExportFilename);

		// @todo: extend this to multiple levels.
		//UWorld* World = GWorld;
		const FString LevelFilename = exportObjects[i]->MeshName;// FileHelpers::GetFilename(World);//->GetOutermost()->GetName() );

		FString LastUsedPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

		//FString FilterString = TEXT("Object (*.obj)|*.obj|Unreal Text (*.t3d)|*.t3d|Stereo Litho (*.stl)|*.stl|LOD Export (*.lod.obj)|*.lod.obj");

		GUnrealEd->ExportMap(GWorld, *ExportDynamicsDirectory, true);

		//exported
		//move textures to root. want to do this in python, but whatever

		//run python on them after everything is finished? need to convert texture anyway

		ExportDynamicsDirectory.RemoveFromEnd("/" + exportObjects[i]->MeshName + "/" + exportObjects[i]->MeshName + ".obj");

		exportObjects[i]->GetOwner()->SetActorLocation(originalLocation);
		exportObjects[i]->GetOwner()->SetActorRotation(originalRotation);
		//tempactor->SetActorScale3D(originalScale);

		List_MaterialArgs(exportObjects[i]->MeshName, ExportDynamicsDirectory);
	}
	GLog->Log("FCognitiveTools::ExportDynamicObjectArray Found " + FString::FromInt(ActorsExported) + " meshes for export");


	//TODO export transparent textures for dynamic objects
	//
	ConvertDynamicTextures();
}

FReply FCognitiveTools::SetUniqueDynamicIds()
{
	//loop thorugh all dynamics in the scene
	TArray<UDynamicObject*> dynamics;

	//make a list of all the used objectids

	TArray<FDynamicObjectId> usedIds;

	//get all the dynamic objects in the scene
	for (TActorIterator<AStaticMeshActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		AStaticMeshActor *Mesh = *ActorItr;

		UActorComponent* actorComponent = Mesh->GetComponentByClass(UDynamicObject::StaticClass());
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

		int32 findId = dynamic->CustomId;

		FDynamicObjectId* FoundId = usedIds.FindByPredicate([findId](const FDynamicObjectId& InItem)
		{
			return InItem.Id == findId;
		});

		if (FoundId == NULL && dynamic->CustomId > 0)
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
				return InItem.Id == currentUniqueId;
			});

			if (FoundId == NULL)
			{
				dynamic->CustomId = currentUniqueId;
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

	RefreshDisplayDynamicObjectsCountInScene();

	SceneDynamicObjectList->RefreshList();

	return FReply::Handled();
}

FReply FCognitiveTools::UploadDynamicsManifest()
{
	TArray<UDynamicObject*> dynamics;

	//get all the dynamic objects in the scene
	for (TActorIterator<AStaticMeshActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		AStaticMeshActor *Mesh = *ActorItr;

		UActorComponent* actorComponent = Mesh->GetComponentByClass(UDynamicObject::StaticClass());
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
		if (dynamics[i]->UseCustomId && dynamics[i]->CustomId != 0)
		{
			wroteAnyObjects = true;
			objectManifest += "{";
			objectManifest += "\"id\":\"" + FString::FromInt(dynamics[i]->CustomId) + "\",";
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
	TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();
	if (!currentSceneData.IsValid())
	{
		GLog->Log("FCognitiveTools::UploadDynamicObjectManifest could not find current scene id");
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

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->SetContentAsString(objectManifest);
	HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::OnUploadManifestCompleted);

	HttpRequest->ProcessRequest();

	return FReply::Handled();
}

void FCognitiveTools::OnUploadManifestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GLog->Log("FCognitiveTools::OnUploadManifestCompleted success!");
		GetDynamicsManifest();
	}
	else
	{
		GLog->Log("FCognitiveTools::OnUploadManifestCompleted failed!");
	}
}

FReply FCognitiveTools::GetDynamicsManifest()
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
	if (FAnalyticsCognitiveVR::Get().EditorAuthToken.Len() == 0)
	{
		GLog->Log("CognitiveTools::GetDyanmicManifest auth token is empty. Must log in!");
		return FReply::Handled();
	}

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(GetDynamicObjectManifest(FString::FromInt(currentSceneData->VersionId)));

	HttpRequest->SetHeader("X-HTTP-Method-Override", TEXT("GET"));
	HttpRequest->SetHeader("Authorization", TEXT("Bearer " + FAnalyticsCognitiveVR::Get().EditorAuthToken));

	HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::OnDynamicManifestResponse);
	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

void FCognitiveTools::OnDynamicManifestResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		//GLog->Log("CognitiveTools::OnDynamicManifestResponse content: " + Response->GetContentAsString());

		SceneExplorerDynamics.Empty();

		//do json stuff to this

		TSharedPtr<FJsonValue> JsonDynamics;

		TSharedRef<TJsonReader<>>Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonDynamics))
		{
			int32 count = JsonDynamics->AsArray().Num();
			GLog->Log("FCognitiveTools::OnDynamicManifestResponse returned " + FString::FromInt(count) + " objects");
			for (int i = 0; i < count; i++)
			{
				TSharedPtr<FJsonObject> jsonobject = JsonDynamics->AsArray()[i]->AsObject();
				FString name = jsonobject->GetStringField("name");
				FString meshname = jsonobject->GetStringField("meshName");
				int32 id = FCString::Atoi(*jsonobject->GetStringField("sdkId"));

				SceneExplorerDynamics.Add(MakeShareable(new FDynamicData(name, meshname, id)));
			}
		}
		if (WebDynamicList.IsValid())
		{
			WebDynamicList->RefreshList();
		}
	}
}

FReply FCognitiveTools::UploadDynamics()
{	
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");

	// Get all files in directory
	TArray<FString> directoriesToSkip;
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	TArray<FString> DirectoriesToSkip;
	TArray<FString> DirectoriesToNotRecurse;

	// use the timestamp grabbing visitor (include directories)
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToNotRecurse, true);
	Visitor.Visit(*ExportDynamicsDirectory, true);

	GLog->Log("FCognitiveTools::UploadDynamics found " + FString::FromInt(Visitor.FileTimes.Num()) + " exported dynamic objects");
	TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();

	if (!currentSceneData.IsValid())
	{
		GLog->Log("FCognitiveToolsCustomization::UploadDynamics can't find current scene!");
		return FReply::Handled();
	}

	//no matches anywhere
	//CognitiveLog::Warning("UPlayerTracker::GetSceneKey ------- no matches in ini");

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString filePath = TimestampIt.Key();
		const FString fileName = FPaths::GetCleanFilename(filePath);

		if (ExportDynamicsDirectory == filePath)
		{
			//GLog->Log("root found " + filePath);
		}
		else if (FPaths::DirectoryExists(filePath))
		{
			//GLog->Log("directory found " + filePath);
			FString url = PostDynamicObjectMeshData(currentSceneData->Id, currentSceneData->VersionNumber, fileName);

			UploadFromDirectory(url, filePath, "object");
		}
		else
		{
			//GLog->Log("file found " + filePath);
		}
	}

	return FReply::Handled();
}

TArray<TSharedPtr<FString>> FCognitiveTools::GetSubDirectoryNames()
{
	return SubDirectoryNames;
}

void FCognitiveTools::FindAllSubDirectoryNames()
{
	// Get all files in directory
	TArray<FString> directoriesToSkip;
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	TArray<FString> DirectoriesToSkip;
	TArray<FString> DirectoriesToNotRecurse;

	// use the timestamp grabbing visitor (include directories)
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToNotRecurse, true);
	Visitor.Visit(*ExportDynamicsDirectory, true);
	
	//no matches anywhere
	SubDirectoryNames.Empty();
	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString filePath = TimestampIt.Key();
		const FString fileName = FPaths::GetCleanFilename(filePath);

		if (ExportDynamicsDirectory == filePath)
		{
			//GLog->Log("root found " + filePath);
		}
		else if (FPaths::DirectoryExists(filePath))
		{
			SubDirectoryNames.Add(MakeShareable(new FString(fileName)));
		}
		else
		{
			//GLog->Log("file found " + filePath);
		}
	}
}

void FCognitiveTools::ReexportDynamicMeshes(FString directory)
{
	//open blender and run a script
	//TODO make openblender and run a script into a function, because this is used in a bunch of places

	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ExportDynamicMesh.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ReexportDynamicMeshes Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();


	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ReexportDynamicMeshes Could not find ExportDynamicMesh.py script at path. Canceling"));
		return;
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ReexportDynamicMeshes No path set for Blender.exe. Canceling"));
		return;
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ReexportDynamicMeshes World is null. canceling"));
		return;
	}

	FString ObjPath = directory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ReexportDynamicMeshes No know export directory. Canceling"));
		return;
	}

	FString resizeFactor = FString::FromInt(GetTextureRefacor());

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedTargetPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString stringparams = " -P " + escapedPythonPath + " " + escapedTargetPath;// +" " + resizeFactor;// +" " + MaxPolyCount + " " + SceneName;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	//UE_LOG(LogTemp, Warning, TEXT("Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);
}

void FCognitiveTools::ConvertDynamicTextures()
{
	//open blender and run a script
	//TODO make openblender and run a script into a function, because this is used in a bunch of places

	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ConvertDynamicTextures.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ConvertDynamicTextures Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();


	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ConvertDynamicTextures Could not find ConvertDynamicTextures.py script at path. Canceling"));
		return;
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ConvertDynamicTextures No path set for Blender.exe. Canceling"));
		return;
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ConvertDynamicTextures World is null. canceling"));
		return;
	}

	FString ObjPath = ExportDynamicsDirectory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::ConvertDynamicTextures No know export directory. Canceling"));
		return;
	}

	FString resizeFactor = FString::FromInt(GetTextureRefacor());

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedTargetPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString stringparams = " -P " + escapedPythonPath + " " + escapedTargetPath + " " + resizeFactor;// +" " + MaxPolyCount + " " + SceneName;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	//UE_LOG(LogTemp, Warning, TEXT("Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle process = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);
}

FReply FCognitiveTools::Export_Selected()
{
	FEditorFileUtils::Export(true);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	ExportDirectory = FPaths::ConvertRelativePathToFull(ExportDirectory);

	return FReply::Handled();
}

FReply FCognitiveTools::Export_All()
{
	FEditorFileUtils::Export(false);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	ExportDirectory = FPaths::ConvertRelativePathToFull(ExportDirectory);

	return FReply::Handled();
}

FReply FCognitiveTools::Select_Export_Meshes()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveToolsCustomization::Select_Export_Meshes world is null"));
		return FReply::Handled();
	}

	//TODO also grab landscapes

	GEditor->SelectNone(false, true, false);

	int32 ActorsExported = 0;

	for (TActorIterator<AStaticMeshActor> ObstacleItr(tempworld); ObstacleItr; ++ObstacleItr)
	{
		//get non-moveable static meshes only
		if (GetStaticOnly())
		{
			EComponentMobility::Type mobility = EComponentMobility::Static;
			const USceneComponent* sc = Cast<USceneComponent>(ObstacleItr->GetStaticMeshComponent());
			if (sc == NULL) { continue; }
			if (sc->Mobility == EComponentMobility::Movable) { continue; }
		}

		//get meshes in size range
		FVector origin;
		FVector boxBounds;

		ObstacleItr->GetActorBounds(false, origin, boxBounds);
		double magnitude = FMath::Sqrt(boxBounds.X*boxBounds.X + boxBounds.Y*boxBounds.Y + boxBounds.Z*boxBounds.Z);

		if (magnitude < GetMinimumSize())
		{
			continue;
		}

		if (magnitude > GetMaximumSize())
		{
			continue;
		}

		//get the selectable bit
		AStaticMeshActor *tempactor = *ObstacleItr;
		if (!tempactor)
		{
			continue;
		}
		UActorComponent* actorComponent = tempactor->GetComponentByClass(UStaticMeshComponent::StaticClass());
		if (actorComponent == NULL)
		{
			continue;
		}
		USceneComponent* sceneComponent = Cast<USceneComponent>(actorComponent);
		if (sceneComponent == NULL)
		{
			continue;
		}
		if (!sceneComponent->bVisible || sceneComponent->bHiddenInGame)
		{
			continue;
		}
		

		GEditor->SelectActor(tempactor, true, false, true);
		ActorsExported++;
	}
	UE_LOG(LogTemp, Warning, TEXT("Found %d Static Meshes for Export"), ActorsExported);
	return FReply::Handled();
}

FReply FCognitiveTools::Select_Blender()
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

FReply FCognitiveTools::Select_Export_Directory()
{
	FString title = "Select Export Directory";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Select_Export_Directory - picked a directory"));
		ExportDirectory = outFilename;
	}
	else
	{
		GLog->Log("FCognitiveTools::Select_Export_Directory cancelled?");
		ExportDirectory = "";
	}
	return FReply::Handled();
}

FReply FCognitiveTools::SelectDynamicsDirectory()
{
	FString title = "Select Dynamc Export Root Directory";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::SelectDynamicsDirectory - picked a directory"));
		ExportDynamicsDirectory = outFilename;
		FindAllSubDirectoryNames();
		SubDirectoryListWidget->RefreshList();
	}
	return FReply::Handled();
}

bool FCognitiveTools::PickFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
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

FReply FCognitiveTools::SelectUploadScreenshot()
{
	FString title = "Select Screenshot";
	FString fileTypes = ".png";
	FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_OPEN, FPaths::ScreenShotDir());
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_OPEN);
	//FString lastPath = FEditorDirectories::Get().

	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickFile(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		//read outfilename bytes
		TArray<uint8> byteResult;
		FFileHelper::LoadFileToArray(byteResult, *outFilename);

		//put in http request
		if (byteResult.Num() == 0)
		{
			return FReply::Handled();
		}


		FString PngHeader;
		TArray<uint8> AllBytes;

		PngHeader.Append(TEXT("\r\n"));
		PngHeader.Append("--cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i");
		PngHeader.Append(TEXT("\r\n"));
		PngHeader.Append("Content-Type: image/png");
		PngHeader.Append(TEXT("\r\n"));
		PngHeader.Append("Content-disposition: form-data; name=\"file\"; filename=\"screenshot.png\"");
		PngHeader.Append(TEXT("\r\n"));
		PngHeader.Append(TEXT("\r\n"));

		FTCHARToUTF8 ConverterBody(*PngHeader);
		auto bodydata = (const uint8*)ConverterBody.Get();

		//file header
		AllBytes.Append(bodydata, ConverterBody.Length());
		//file binary
		AllBytes.Append(byteResult);
		//file end
		FString EndString = TEXT("\r\n--cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i--\r\n");
		FTCHARToUTF8 ConverterEnd2(*EndString);
		auto enddata = (const uint8*)ConverterEnd2.Get();
		
		AllBytes.Append(enddata, ConverterEnd2.Length());



		//send screenshot
		TSharedPtr<FEditorSceneData> currentdata = GetCurrentSceneData();
		FString url = PostScreenshot(currentdata->Id, FString::FromInt(currentdata->VersionNumber));

		TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
		HttpRequest->SetURL(url);
		HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=\"cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i\"");
		HttpRequest->SetHeader("Accept-Encoding", "identity");
		HttpRequest->SetVerb("POST");
		HttpRequest->SetContent(AllBytes);

		HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::OnUploadScreenshotCompleted);
		HttpRequest->ProcessRequest();
	}
	return FReply::Handled();
}

void FCognitiveTools::OnUploadScreenshotCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GLog->Log("FCognitiveTools::OnUploadScreenshotCompleted successful!");
	}
}

bool FCognitiveTools::PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
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

void* FCognitiveTools::ChooseParentWindowHandle()
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

FReply FCognitiveTools::List_Materials()
{
	List_MaterialArgs("",ExportDirectory);
	return FReply::Handled();
}

void FCognitiveTools::List_MaterialArgs(FString subdirectory, FString searchDirectory)
{
	//look at export directory. find mtl file

	FString result;
	FString Ext;
	//get all files in the export directory
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> Files;

	if (Ext == "")
	{
		Ext = "*.*";
	}
	else
	{
		Ext = (Ext.Left(1) == ".") ? "*" + Ext : "*." + Ext;
	}

	FString FinalPath = searchDirectory;

	if (!subdirectory.IsEmpty())
	{
		FinalPath += "/"+subdirectory;
	}
	FinalPath += "/" + Ext;

	FileManager.FindFiles(Files, *FinalPath, true, false);

	TArray<FColor> colors;
	FIntPoint point = FIntPoint(256, 256);

	for (int32 i = 0; i < Files.Num(); i++)
	{
		if (Files[i].EndsWith(".mtl"))
		{

			FString fullPath = searchDirectory+ "/" + subdirectory + "/" + Files[i];
			FString contents;

			if (FFileHelper::LoadFileToString(contents, *fullPath))
			{
				//GLog->Log("loaded " + Files[i] + "\n" = contents);
			}
			else
			{
				//GLog->Log("failed to load " + Files[i]);
			}
			TArray<FString> lines;
			int32 lineCount = contents.ParseIntoArray(lines, TEXT("\n"), true);

			for (int32 j = 0; j < lines.Num(); j++)
			{
				if (lines[j].Contains("newmtl"))
				{
					TArray<FString> matDirectories;
					lines[j].RemoveFromStart("newmtl ");
					lines[j].ParseIntoArray(matDirectories, TEXT("/"));

					TArray<FString> lineParts;
					matDirectories[matDirectories.Num() - 1].ParseIntoArray(lineParts, TEXT("_"));

					int32 floor_count = FMath::FloorToInt(lineParts.Num() / (float)2);

					FString trueMatName;
					for (int32 k = 0; k < lineParts.Num(); k++)
					{
						if (k + 1 == floor_count)
						{
							trueMatName = trueMatName + lineParts[k] + ".";
						}
						else
						{
							trueMatName = trueMatName + lineParts[k] + "_";
						}
					}
					trueMatName.RemoveFromEnd("_");

					FString finalMatPath;
					FString finalDirectory;
					for (int32 l = 0; l < matDirectories.Num() - 1; l++)
					{
						finalMatPath = finalMatPath + matDirectories[l] + "/";
						finalDirectory = finalMatPath + matDirectories[l] + "/";
					}
					finalMatPath = "/" + finalMatPath + trueMatName;
					finalMatPath.RemoveAt(finalMatPath.Len() - 1);

					FStringAssetReference assetRef = FStringAssetReference(finalMatPath);

					UObject* assetRefLoaded = assetRef.TryLoad();

					if (assetRefLoaded != NULL)
					{
						UMaterial* m = Cast<UMaterial>(assetRefLoaded);
						UMaterialInstance* mi = Cast<UMaterialInstance>(assetRefLoaded);

						if (m != NULL)
						{
							if (m->GetBlendMode() == EBlendMode::BLEND_Opaque)
							{
								continue;
							}

							//TODO export material property transparency
							if (FMaterialUtilities::ExportMaterialProperty(m, EMaterialProperty::MP_BaseColor, point, colors))
							{
								FString BMPFilename;
								if (subdirectory.IsEmpty())
								{
									BMPFilename = searchDirectory + finalMatPath.Replace(TEXT("."), TEXT("_")) + TEXT("_D.bmp");
								}
								else
								{
									BMPFilename = searchDirectory+"/"+subdirectory + finalMatPath.Replace(TEXT("."), TEXT("_")) + TEXT("_D.bmp");
								}

								//GLog->Log("writing base color for transparent material " + BMPFilename);
								FFileHelper::CreateBitmap(*BMPFilename, point.X, point.Y, colors.GetData());
							}
						}
						else
						{
							if (mi->GetBlendMode() == EBlendMode::BLEND_Opaque)
							{
								continue;
							}

							//TODO export material property transparency
							if (FMaterialUtilities::ExportMaterialProperty(mi, EMaterialProperty::MP_BaseColor, point, colors))
							{

								FString BMPFilename;
								if (subdirectory.IsEmpty())
								{
									BMPFilename = searchDirectory + finalMatPath.Replace(TEXT("."), TEXT("_")) + TEXT("_D.bmp");
								}
								else
								{
									BMPFilename = searchDirectory + "/" + subdirectory + finalMatPath.Replace(TEXT("."), TEXT("_")) + TEXT("_D.bmp");
								}

								//GLog->Log("writing base color for transparent material instance " + BMPFilename);
								FFileHelper::CreateBitmap(*BMPFilename, point.X, point.Y, colors.GetData());
							}
						}
					}
				}
			}
		}
	}
}

//run this as the next step after exporting the scene
FReply FCognitiveTools::Reduce_Meshes()
{
	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("DecimateExportedScene.py");
	const TCHAR* charPath = *pythonscriptpath;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Reduce_Meshes - Could not find decimateall.py script at path. Canceling"));
		return FReply::Handled();
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Reduce_Meshes - No path set for Blender.exe. Canceling"));
		return FReply::Handled();
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Reduce_Meshes - World is null. canceling"));
		return FReply::Handled();
	}

	FString SceneName = tempworld->GetMapName();
	FString ObjPath = ExportDirectory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Reduce_Meshes No know export directory. Canceling"));
		return FReply::Handled();
	}

	FString MinPolyCount = FString::FromInt(GetMinPolygon());
	FString MaxPolyCount = FString::FromInt(GetMaxPolygon());

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

	FString productID = GetProductID();

	FString stringparams = " -P " + escapedPythonPath + " " + escapedOutPath + " " + MinPolyCount + " " + MaxPolyCount + " " + SceneName + " " + productID + " " + COGNITIVEVR_SDK_VERSION + " " + escapedExcludeMeshes;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Reduce_Meshes Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);

	//TODO when procHandle is complete, upload exported files to sceneexplorer.com
	return FReply::Handled();
}

FString FCognitiveTools::GetProductID()
{
	FString ValueReceived;
	GConfig->GetString(
		TEXT("Analytics"),
		TEXT("CognitiveVRApiKey"),
		ValueReceived,
		GEngineIni
	);
	return ValueReceived;
}

FReply FCognitiveTools::UploadScene()
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

	GLog->Log("FCognitiveTools::UploadScene upload scene to " + url);
	//TODO listen for response. when the response returns, request the scene version with auth token
	UploadFromDirectory(url, ExportDirectory, "scene");

	return FReply::Handled();
}

//run this as the next step after exporting the scene
FReply FCognitiveTools::Reduce_Textures()
{
	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ConvertTextures.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	//UE_LOG(LogTemp, Warning, TEXT("Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();


	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Reduce_Textures Could not find decimateall.py script at path. Canceling"));
		return FReply::Handled();
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Reduce_Textures No path set for Blender.exe. Canceling"));
		return FReply::Handled();
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Reduce_Textures World is null. canceling"));
		return FReply::Handled();
	}

	FString SceneName = tempworld->GetMapName();
	FString ObjPath = ExportDirectory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveTools::Reduce_Textures No know export directory. Canceling"));
		return FReply::Handled();
	}


	FString MaxPolyCount = FString::FromInt(0);
	FString resizeFactor = FString::FromInt(GetTextureRefacor());

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString stringparams = " -P " + escapedPythonPath + " " + escapedOutPath + " " + resizeFactor + " " + MaxPolyCount + " " + SceneName;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);

	//FString cmdPath = "C:\\Windows\\System32\\cmd.exe";
	//FString cmdPathS = "cmd.exe";
	//FProcHandle procHandle = FPlatformProcess::CreateProc(*cmdPath, NULL, false, false, false, NULL, priorityMod, 0, nullptr);

	//TODO can i just create a process and add parameters or do i need to run through cmd line??
	//system("cmd.exe");

	//TODO when procHandle is complete, upload exported files to sceneexplorer.com
	return FReply::Handled();
}

void FCognitiveTools::UploadFromDirectory(FString url, FString directory, FString expectedResponseType)
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(directory, true, filesStartingWith, filesStartingWith, pngextension);

	TArray<FString> imagesInDirectory = GetAllFilesInDirectory(directory, true, filesStartingWith, pngextension, filesStartingWith);

	TArray<FContentContainer> contentArray;

	UE_LOG(LogTemp, Log, TEXT("UploadScene image count%d"), imagesInDirectory.Num());
	UE_LOG(LogTemp, Log, TEXT("UploadScene file count%d"), filesInDirectory.Num());

	for (int32 i = 0; i < filesInDirectory.Num(); i++)
	{
		FString Content;
		FString temp;
		FString result;
		if (FFileHelper::LoadFileToString(result, *filesInDirectory[i]))
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
			container.BodyText = *result;

			contentArray.Add(container);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("failed to load %s"), *filesInDirectory[i]);
		}
	}

	for (int32 i = 0; i < imagesInDirectory.Num(); i++)
	{
		FString Content;
		TArray<uint8> byteResult;
		if (FFileHelper::LoadFileToArray(byteResult, *imagesInDirectory[i]))
		{
			FContentContainer container = FContentContainer();
			UE_LOG(LogTemp, Log, TEXT("Loaded image %s"), *imagesInDirectory[i]);
			//loaded the file
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("--cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-Type: image/png");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-disposition: form-data; name=\"file\"; filename=\"" + FPaths::GetCleanFilename(imagesInDirectory[i]) + "\"");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append(TEXT("\r\n"));

			container.Headers = Content;
			container.BodyBinary = byteResult;

			contentArray.Add(container);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("failed to load image %s"), *imagesInDirectory[i]);
		}
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
	HttpRequest->SetVerb("POST");
	HttpRequest->SetContent(AllBytes);

	FHttpModule::Get().SetHttpTimeout(0);

	if (expectedResponseType == "scene")
	{
		HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::OnUploadSceneCompleted);
	}
	if (expectedResponseType == "object")
	{
		HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::OnUploadObjectCompleted);
	}

	//DEBUGGING write http request contents to file
	/*

	FString SaveDirectory = FString("C:/Users/calder/Desktop");
	FString FileName = FString("UploadContent.txt");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
	{
	// Get absolute file path
	FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

	//FFileHelper::SaveStringToFile(Content, *AbsoluteFilePath);
	FFileHelper::SaveArrayToFile(AllBytes, *AbsoluteFilePath);
	}
	*/

	HttpRequest->ProcessRequest();
}

void FCognitiveTools::OnUploadSceneCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Upload Scene Response is %s"), *Response->GetContentAsString());

		UWorld* myworld = GWorld->GetWorld();
		if (myworld == NULL)
		{
			UE_LOG(LogTemp, Warning, TEXT("Upload Scene - No world!"));
			return;
		}

		FString currentSceneName = myworld->GetMapName();
		currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

		//FConfigSection* ScenePairs = GConfig->GetSectionPrivate(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), false, true, GEngineIni);
		//GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), *currentSceneName, *Response->GetContentAsString(), GEngineIni);


		//GLog->Log(currentSceneName + " scene set with SceneKey " + *Response->GetContentAsString());

		FString responseNoQuotes = *Response->GetContentAsString().Replace(TEXT("\""), TEXT(""));

		if (responseNoQuotes.Len() > 0)
		{
			SaveSceneData(currentSceneName, responseNoQuotes);
			RefreshSceneData();
		}
		else
		{
			//successfully uploaded a scene but no response - updated an existing scene version
			RefreshSceneData();
		}
		ConfigFileHasChanged = true;
	}
}

void FCognitiveTools::OnUploadObjectCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		FString responseNoQuotes = *Response->GetContentAsString().Replace(TEXT("\""), TEXT(""));
		GLog->Log("Upload Dynamic Complete " + Request->GetURL());
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
TArray<FString> FCognitiveTools::GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension, const FString ignoreExtension) const
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

bool FCognitiveTools::HasFoundBlender() const
{
	if (!HasLoggedIn()) { return false; }
	return FCognitiveTools::GetBlenderPath().ToString().Contains("blender.exe");
}

bool FCognitiveTools::HasFoundBlenderAndHasSelection() const
{
	if (!HasLoggedIn()) { return false; }
	return FCognitiveTools::GetBlenderPath().ToString().Contains("blender.exe") && GEditor->GetSelectedActorCount() > 0;
}

//checks for json and no bmps files in export directory
bool FCognitiveTools::HasConvertedFilesInDirectory() const
{
	if (!HasSetExportDirectory()) { return false; }

	FString filesStartingWith = TEXT("");
	FString bmpextension = TEXT("bmp");
	FString jsonextension = TEXT("json");

	TArray<FString> imagesInDirectory = GetAllFilesInDirectory(ExportDirectory, true, filesStartingWith, bmpextension, filesStartingWith);
	TArray<FString> jsonInDirectory = GetAllFilesInDirectory(ExportDirectory, true, filesStartingWith, jsonextension, filesStartingWith);
	if (imagesInDirectory.Num() > 0) { return false; }
	if (jsonInDirectory.Num() == 0) { return false; }

	return true;
}

bool FCognitiveTools::CanUploadSceneFiles() const
{
	return HasConvertedFilesInDirectory() && HasLoggedIn();
}

bool FCognitiveTools::LoginAndCustonerIdAndBlenderExportDir() const
{
	return HasLoggedIn() && HasFoundBlenderAndExportDir() && HasSavedCustomerId();
}

bool FCognitiveTools::HasFoundBlenderAndExportDir() const
{
	return FCognitiveTools::GetBlenderPath().ToString().Contains("blender.exe") && !FCognitiveTools::GetExportDirectory().EqualTo(FText::FromString(""));
}

bool FCognitiveTools::HasFoundBlenderAndDynamicExportDir() const
{
	return FCognitiveTools::GetBlenderPath().ToString().Contains("blender.exe") && !FCognitiveTools::GetDynamicExportDirectory().EqualTo(FText::FromString(""));
}

bool FCognitiveTools::CurrentSceneHasSceneId() const
{
	if (!HasLoggedIn()) { return false; }
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

bool FCognitiveTools::HasSetExportDirectory() const
{
	if (!HasLoggedIn()) { return false; }
	return !FCognitiveTools::GetExportDirectory().EqualTo(FText::FromString(""));
}

bool FCognitiveTools::HasEditorAuthToken() const
{
	return FAnalyticsCognitiveVR::Get().EditorAuthToken.Len() > 0;
}

FText FCognitiveTools::GetDynamicsOnSceneExplorerTooltip() const
{
	if (!HasLoggedIn())
	{
		return FText::FromString("Must log in to get Dynamic Objects List from SceneExplorer");
	}
	auto scene = GetCurrentSceneData();
	if (!scene.IsValid())
	{
		return FText::FromString("Scene does not have valid data. Must export your scene before uploading dynamics!");
	}
	if (HasEditorAuthToken())
	{
		return FText::FromString("");
	}
	return FText::FromString("Something went wrong!");
}

EVisibility FCognitiveTools::ConfigFileChangedVisibility() const
{
	if (ConfigFileHasChanged)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Hidden;
}

FText FCognitiveTools::SendDynamicsToSceneExplorerTooltip() const
{
	if (HasEditorAuthToken())
	{
		return FText::FromString("");
	}
	return FText::FromString("Must log in to send Dynamic Objects List to SceneExplorer");
}

FText FCognitiveTools::GetCustomerId() const
{
	if (SelectedProduct.customerId.Len() > 0)
	{
		return FText::FromString(SelectedProduct.customerId);
	}
	//FString customerid = GetCustomerIdFromFile();
	return FText::FromString("");
}

bool FCognitiveTools::HasSetDynamicExportDirectory() const
{
	if (!HasLoggedIn()) { return false; }
	return !FCognitiveTools::GetDynamicExportDirectory().EqualTo(FText::FromString(""));
}

bool FCognitiveTools::HasSetDynamicExportDirectoryHasSceneId() const
{
	if (!HasLoggedIn()) { return false; }
	auto scenedata = GetCurrentSceneData();
	if (!scenedata.IsValid()) { return false; }
	return !FCognitiveTools::GetDynamicExportDirectory().EqualTo(FText::FromString(""));
}

bool FCognitiveTools::HasFoundBlenderHasSelection() const
{
	if (GEditor->GetSelectedActorCount() == 0) { return false; }
	return HasFoundBlender();
}

FText FCognitiveTools::GetBlenderPath() const
{
	return FText::FromString(BlenderPath);
}

FText FCognitiveTools::GetExportDirectory() const
{
	return FText::FromString(ExportDirectory);
}

FText FCognitiveTools::GetDynamicExportDirectory() const
{
	return FText::FromString(ExportDynamicsDirectory);
}

void FCognitiveTools::SearchForBlender()
{
	//try to find blender in program files
	FString testApp = "C:/Program Files/Blender Foundation/Blender/blender.exe";

	if (VerifyFileExists(testApp))
	{
		UE_LOG(LogTemp, Warning, TEXT("SearchForBlender - Found Blender in Program Files"));
		BlenderPath = testApp;
	}
}

TSharedRef<IDetailCustomization> FCognitiveTools::MakeInstance()
{
	return MakeShareable(new FCognitiveTools);
}

FReply FCognitiveTools::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder->GetObjectsBeingCustomized(ObjectsBeingCustomized);

	for (auto WeakObject : ObjectsBeingCustomized)
	{
		if (UObject* instance = WeakObject.Get())
		{
			instance->CallFunctionByNameWithArguments(*MethodToExecute->GetName(), *GLog, nullptr, true);
		}
	}


	return FReply::Handled();
}

int32 FCognitiveTools::CountDynamicObjectsInScene() const
{
	//loop thorugh all dynamics in the scene
	TArray<UDynamicObject*> dynamics;

	//get all the dynamic objects in the scene
	for (TActorIterator<AStaticMeshActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		AStaticMeshActor *Mesh = *ActorItr;

		UActorComponent* actorComponent = Mesh->GetComponentByClass(UDynamicObject::StaticClass());
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

FText FCognitiveTools::DisplayDynamicObjectsCountInScene() const
{
	return DynamicCountInScene;
}

FText FCognitiveTools::DisplayDynamicObjectsCountOnWeb() const
{
	FString outstring = "Found " + FString::FromInt(SceneExplorerDynamics.Num()) + " Dynamic Objects on SceneExplorer";
	return FText::FromString(outstring);
}

FReply FCognitiveTools::RefreshDisplayDynamicObjectsCountInScene()
{
	DynamicCountInScene = FText::FromString("Found "+ FString::FromInt(CountDynamicObjectsInScene()) + " Dynamic Objects in scene");
	DuplicateDyanmicObjectVisibility = EVisibility::Hidden;
	SceneDynamicObjectList->RefreshList();

	SceneDynamics.Empty();
	//get all the dynamic objects in the scene
	for (TActorIterator<AStaticMeshActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		AStaticMeshActor *Mesh = *ActorItr;

		UActorComponent* actorComponent = Mesh->GetComponentByClass(UDynamicObject::StaticClass());
		if (actorComponent == NULL)
		{
			continue;
		}
		UDynamicObject* dynamic = Cast<UDynamicObject>(actorComponent);
		if (dynamic == NULL)
		{
			continue;
		}
		SceneDynamics.Add(MakeShareable(new FDynamicData(dynamic->GetOwner()->GetName(), dynamic->MeshName, dynamic->CustomId)));
		//dynamics.Add(dynamic);
	}

	if (DuplicateDynamicIdsInScene())
	{
		DuplicateDyanmicObjectVisibility = EVisibility::Visible;
	}

	return FReply::Handled();
}

EVisibility FCognitiveTools::GetDuplicateDyanmicObjectVisibility() const
{
	return DuplicateDyanmicObjectVisibility;
}

FText FCognitiveTools::GetUploadDynamicsToSceneText() const
{
	return UploadDynamicsToSceneText;
}

void FCognitiveTools::RefreshUploadDynamicsToSceneText()
{
	TSharedPtr<FEditorSceneData> currentScene = GetCurrentSceneData();
	if (!currentScene.IsValid())
	{
		UploadDynamicsToSceneText = FText::FromString("current scene is not valid!");
		return;
	}

	//UploadDynamicsToSceneText = FText::FromString("Upload " + "numberofdirectoriesinexportdirectory" + " dynamic object meshes to " + currentScene->Name + " version " + currentScene->VersionNumber + " on Scene Explorer");
	UploadDynamicsToSceneText = FText::FromString("upload x dynamic objects to current scene version y on scene explorer");
}

bool FCognitiveTools::DuplicateDynamicIdsInScene() const
{
	//loop thorugh all dynamics in the scene
	TArray<UDynamicObject*> dynamics;

	//make a list of all the used objectids
	TArray<FDynamicObjectId> usedIds;

	//get all the dynamic objects in the scene
	for (TActorIterator<AStaticMeshActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		AStaticMeshActor *Mesh = *ActorItr;

		UActorComponent* actorComponent = Mesh->GetComponentByClass(UDynamicObject::StaticClass());
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

		int32 findId = dynamic->CustomId;

		FDynamicObjectId* FoundId = usedIds.FindByPredicate([findId](const FDynamicObjectId& InItem)
		{
			return InItem.Id == findId;
		});

		if (FoundId == NULL && dynamic->CustomId > 0)
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

#undef LOCTEXT_NAMESPACE