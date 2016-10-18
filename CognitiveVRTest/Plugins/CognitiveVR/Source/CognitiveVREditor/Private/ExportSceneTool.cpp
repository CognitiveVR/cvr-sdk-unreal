#include "CognitiveVREditorPrivatePCH.h"
#include "ExportSceneTool.h"

#define LOCTEXT_NAMESPACE "DemoTools"

UExportSceneTool::UExportSceneTool()
	: Super(FObjectInitializer::Get())
{
	//MirrorPlane = FPlane(1.0, 0.0f, 0.0f, 0.0f);
}

void UExportSceneTool::ExportScene()
{
	//SelectExportMeshes();

	FEditorFileUtils::Export(true);

	//ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	//RunBlenderCleanup();
}

void UExportSceneTool::SelectExportMeshes()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("world is null"));
		return;
	}

	//also grab landscapes

	GEditor->GetSelectedActors()->DeselectAll();
	GEditor->GetSelectedObjects()->DeselectAll();

	int32 ActorsExported = 0;

	for (TActorIterator<AStaticMeshActor> ObstacleItr(tempworld); ObstacleItr; ++ObstacleItr)
	{
		//get non-moveable static meshes only
		if (staticOnly)
		{
			EComponentMobility::Type mobility = EComponentMobility::Static;
			const USceneComponent* sc = Cast<USceneComponent>(ObstacleItr->GetStaticMeshComponent());
			if (sc == NULL) { continue; }
			if (sc->Mobility == EComponentMobility::Movable) { continue; }
		}

		//get static meshes above certain size
		if (MinimumSize > 1)
		{
			FVector origin;
			FVector boxBounds;

			ObstacleItr->GetActorBounds(false, origin, boxBounds);

			double magnitude = FMath::Sqrt(boxBounds.X*boxBounds.X + boxBounds.Y*boxBounds.Y + boxBounds.Z*boxBounds.Z);
			//UE_LOG(LogTemp, Warning, TEXT("bounds magnitude %f"), magnitude);

			if (magnitude < MinimumSize)
			{
				continue;
			}
		}

		//get the selectable bit
		AStaticMeshActor *tempactor = *ObstacleItr;
		if (!tempactor)
		{
			continue;
		}

		GUnrealEd->SelectActor(tempactor, true, false, true);
		ActorsExported++;
	}
	UE_LOG(LogTemp, Warning, TEXT("Found %d Static Meshes for Export"), ActorsExported);
}

/*void UExportSceneTool::ExportTexturesOnly()
{
	//get all the selected meshes
	//GEditor->GetSelectedActors()->GetSelectedObjects
	
	//for (TActorIterator<AStaticMeshActor> ObstacleItr(GEditor->GetSelectedActorIterator()); ObstacleItr; ++ObstacleItr)

	TArray<UTexture*> outTextures;
	int32 texturesCount = 0;
	TArray<UObject*> ObjectsToExport;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AStaticMeshActor* sma = Cast<AStaticMeshActor>(*It);
		if (!sma)
		{
			UE_LOG(LogTemp, Warning, TEXT("selected is not a StaticMeshActor"));
			continue;
		}
		UStaticMeshComponent* smc = sma->GetStaticMeshComponent();
		if (!smc)
		{
			UE_LOG(LogTemp, Warning, TEXT("selected missing StaticMeshComponent"));
			continue;
		}
		UMaterialInterface* matint = smc->GetMaterial(0);
		if (!matint)
		{
			UE_LOG(LogTemp, Warning, TEXT("MaterialInterface[0] not found"));
			continue;
		}
		UMaterial* mat = matint->GetMaterial();
		

		mat->GetUsedTextures(outTextures, EMaterialQualityLevel::Low, true, ERHIFeatureLevel::ES2, true);
		
		for (auto& tex : outTextures)
		{
			//auto Texture = UTexture2D::CreateTransient(SizeX, SizeY);
			//FTexture2DMipMap& Mip = [Texture]->PlatformData->Mips[Level];
			//void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
			//FMemory::Memcpy(Data, NewData, DataSize);
			//Mip.BulkData.Unlock();
			//Texture->UpdateResource();


			//TArray<UObject*> TextureToExport;
			//TextureToExport.Add(tex);

			//ObjectTools::ExportObjects(TextureToExport, false, &ExportDirectory, true);



			//UE_LOG(LogTemp, Warning, TEXT("exporting tex"));
			ObjectsToExport.Add(tex);
			texturesCount++;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Found %d textures to Export"), texturesCount);



	if (ObjectsToExport.Num() > 0)
	{
		ObjectTools::ExportObjects(ObjectsToExport, false); //bPromptForEachFileName=false
	}
}*/

//open fiel type
void UExportSceneTool::SelectBlender()
{
	FString title = "Select Blender.exe";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		BlenderPath = outFilename;
		//UE_LOG(LogTemp, Warning, TEXT("selected blender at path: %s"), *BlenderPath);
	}
}

bool UExportSceneTool::PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
{
	OutFilename = FString();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bFileChosen = false;
	TArray<FString> OutFilenames;
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = ChooseParentWindowHandle();

		//opendirectorydialog
		/*bFileChosen = DesktopPlatform->SaveFileDialog(
			ParentWindowWindowHandle,
			Title,
			InOutLastPath,
			DefaultFile,
			FileTypes,
			EFileDialogFlags::None,
			OutFilenames
		);*/

		bFileChosen = DesktopPlatform->OpenFileDialog(
		ParentWindowWindowHandle,
		Title,
		InOutLastPath,
		DefaultFile,
		FileTypes,
		EFileDialogFlags::None,
		OutFilenames
		);
		
		/*
		bFileChosen = DesktopPlatform->OpenDirectoryDialog(
			ParentWindowWindowHandle,
			Title,
			InOutLastPath,
			OutFilename
		);*/
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

void* UExportSceneTool::ChooseParentWindowHandle()
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

//run this as the next step after exporting the scene
void UExportSceneTool::RunBlenderCleanup()
{
	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("DemoEditorExtensions"))->GetBaseDir() / TEXT("Resources") / TEXT("DecimateExportedScene.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	UE_LOG(LogTemp, Warning, TEXT("Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Need to do this if running in the editor with -game to make sure that the assets in the following path are available
	//TArray<FString> PathsToScan;
	//FString path = pcd;// +"decimageall.py";
	//PathsToScan.Add(path);
	//AssetRegistry.ScanPathsSynchronous(PathsToScan);

	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find decimateall.py script at path. Canceling"));
		return;
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No path set for Blender.exe. Canceling"));
		return;
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("World is null. canceling"));
		return;
	}

	//const TCHAR* url = *stringurl;
	FString SceneName = tempworld->GetMapName();
	FString ObjPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No know export directory. Canceling"));
		return;
	}

	FString MinPolyCount = FString::FromInt(MinPolygons);
	FString MaxPolyCount = FString::FromInt(MaxPolygons);

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString stringparams = " -P " + escapedPythonPath + " " + escapedOutPath + " " + MinPolyCount + " " + MaxPolyCount + " " + SceneName;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"),TEXT("/"));
	
	UE_LOG(LogTemp, Warning, TEXT("Params: %s"),*stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);
	
	//FString cmdPath = "C:\\Windows\\System32\\cmd.exe";
	//FString cmdPathS = "cmd.exe";
	//FProcHandle procHandle = FPlatformProcess::CreateProc(*cmdPath, NULL, false, false, false, NULL, priorityMod, 0, nullptr);
	
	//TODO can i just create a process and add parameters or do i need to run through cmd line??
	//system("cmd.exe");

	//TODO when procHandle is complete, upload exported files to sceneexplorer.com
}


#undef LOCTEXT_NAMESPACE