
#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveToolsCustomization.h"
#include "CognitiveVRSettings.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

void FCognitiveToolsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSet<UClass*> Classes;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	DetailLayoutPtr = &DetailBuilder;

	UClass* Class = NULL;

	for (auto WeakObject : ObjectsBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			Class = Instance->GetClass();
			break;
		}
	}

	IDetailCategoryBuilder& SettingsCategory = DetailBuilder.EditCategory(TEXT("Export Settings"));

	MinPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinPolygons));
	MaxPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaxPolygons));
	StaticOnlyProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, staticOnly));
	MinSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinimumSize));
	MaxSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaximumSize));
	TextureResizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, TextureResizeFactor));

	SettingsCategory.AddProperty(MinPolygonProperty);
	SettingsCategory.AddProperty(MaxPolygonProperty);
	SettingsCategory.AddProperty(StaticOnlyProperty);
	SettingsCategory.AddProperty(MinSizeProperty);
	SettingsCategory.AddProperty(MaxSizeProperty);
	SettingsCategory.AddProperty(TextureResizeProperty);

	// Create a commands category
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Scene Commands"));

	FText p = GetBlenderPath();
	if (p.EqualTo(FText::FromString("")) && !HasSearchedForBlender)
	{
		HasSearchedForBlender = true;
		UE_LOG(LogTemp, Warning, TEXT("blender path is empty. search for blender"));
		SearchForBlender();
	}

	Category.AddCustomRow(FText::FromString("Select Blender Horizontal"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
				//button
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
				[
					SNew(SButton)
					.IsEnabled(true)
					.Text(FText::FromString("Select Blender"))
					.OnClicked(this, &FCognitiveToolsCustomization::Select_Blender)
				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(this, &FCognitiveToolsCustomization::GetBlenderPath)
				]
		];

	//select export meshes
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasFoundBlender)
			.Text(FText::FromString("Select Export Meshes"))
			.OnClicked(this, &FCognitiveToolsCustomization::Select_Export_Meshes)
		];

	//export selected scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasFoundBlender)
			.Text(FText::FromString("Export Selected"))
			.OnClicked(this, &FCognitiveToolsCustomization::Export_Selected)
		];

	//export whole scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasFoundBlender)
			.Text(FText::FromString("Export All"))
			.OnClicked(this, &FCognitiveToolsCustomization::Export_All)
		];


	Category.AddCustomRow(FText::FromString("Select Export Directory"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			//button
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
			[
				SNew(SButton)
				.IsEnabled(true)
				.Text(FText::FromString("Select Export Directory"))
				.OnClicked(this, &FCognitiveToolsCustomization::Select_Export_Directory)
			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(this, &FCognitiveToolsCustomization::GetExportDirectory)
			]
		];

	//List Materials
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasFoundBlenderAndExportDir)
			.Text(FText::FromString("Export Transparent Textures"))
			.OnClicked(this, &FCognitiveToolsCustomization::List_Materials)
		];

	//Reduce Meshes
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasFoundBlenderAndExportDir)
			.Text(FText::FromString("Reduce Meshes"))
			.OnClicked(this, &FCognitiveToolsCustomization::Reduce_Meshes)
		];

	//Reduce Textures
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasFoundBlenderAndExportDir)
			.Text(FText::FromString("Reduce Textures"))
			.OnClicked(this, &FCognitiveToolsCustomization::Reduce_Textures)
		];



	//upload scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasFoundBlenderAndExportDir)
			.Text(FText::FromString("Upload Scene"))
			.OnClicked(this, &FCognitiveToolsCustomization::UploadScene)
		];

	// Create a commands category
	IDetailCategoryBuilder& DynamicsCategory = DetailBuilder.EditCategory(TEXT("Dynamic Object Commands"));

	//export all dynamics
	DynamicsCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasFoundBlender)
			.Text(FText::FromString("Export All Dynamic Objects"))
			.OnClicked(this, &FCognitiveToolsCustomization::ExportDynamics)
		];

	//export selected dynamics
	DynamicsCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasFoundBlender)
			.Text(FText::FromString("Export Selected Dynamic Objects"))
			.OnClicked(this, &FCognitiveToolsCustomization::ExportSelectedDynamics)
		];

	//select dynamic export directory
	DynamicsCategory.AddCustomRow(FText::FromString("Select Dynamic Directory"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			//button
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
			[
				SNew(SButton)
				.IsEnabled(true)
				.Text(FText::FromString("Select Dynamic Directory"))
				.OnClicked(this, &FCognitiveToolsCustomization::SelectDynamicsDirectory)
			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(this, &FCognitiveToolsCustomization::GetDynamicExportDirectory)
			]
		];

	//upload dynamics
	DynamicsCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveToolsCustomization::HasSetDynamicExportDirectory)
			.Text(FText::FromString("Upload Dynamic Objects"))
			.OnClicked(this, &FCognitiveToolsCustomization::UploadDynamics)
		];
}

float FCognitiveToolsCustomization::GetMinimumSize()
{
	float MinSize = 0;
	MinSizeProperty->GetValue(MinSize);
	return MinSize;
}

float FCognitiveToolsCustomization::GetMaximumSize()
{
	float MaxSize = 0;
	MaxSizeProperty->GetValue(MaxSize);
	return MaxSize;
}

bool FCognitiveToolsCustomization::GetStaticOnly()
{
	bool staticOnly = false;
	StaticOnlyProperty->GetValue(staticOnly);
	return staticOnly;
}

int32 FCognitiveToolsCustomization::GetMinPolygon()
{
	int32 MinCount = 0;
	MinPolygonProperty->GetValue(MinCount);
	return MinCount;
}

int32 FCognitiveToolsCustomization::GetMaxPolygon()
{
	int32 MaxCount = 0;
	MaxPolygonProperty->GetValue(MaxCount);
	return MaxCount;
}

int32 FCognitiveToolsCustomization::GetTextureRefacor()
{
	int32 TextureRefactor = 0;
	TextureResizeProperty->GetValue(TextureRefactor);
	return TextureRefactor;
}

FReply FCognitiveToolsCustomization::ExportDynamics()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("world is null"));
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

FReply FCognitiveToolsCustomization::ExportSelectedDynamics()
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

void FCognitiveToolsCustomization::ExportDynamicObjectArray(TArray<UDynamicObject*> exportObjects)
{
	FVector originalLocation;
	FRotator originalRotation;
	FVector originalScale;
	int32 ActorsExported = 0;

	for (int32 i = 0; i < exportObjects.Num(); i++)
	{
		GEditor->SelectNone(false, true, false);// ->GetSelectedActors()->DeselectAll();


		originalLocation = exportObjects[i]->GetOwner()->GetActorLocation();
		originalRotation = exportObjects[i]->GetOwner()->GetActorRotation();
		//originalScale = tempactor->GetActorScale();

		exportObjects[i]->GetOwner()->SetActorLocation(FVector::ZeroVector);
		exportObjects[i]->GetOwner()->SetActorRotation(FQuat::Identity);
		//tempactor->SetActorScale3D(originalScale*0.01);

		FString ExportFilename = exportObjects[i]->MeshName + ".obj";

		GEditor->SelectActor(exportObjects[i]->GetOwner(), true, false, true);
		//ActorsExported++;

		GLog->Log("root output directory " + ExportDynamicsDirectory);

		ExportDynamicsDirectory += "/" + exportObjects[i]->MeshName + "/" + exportObjects[i]->MeshName + ".obj";

		GLog->Log("dynamic output directory " + ExportDynamicsDirectory);

		GLog->Log("exporting DynamicObject " + ExportFilename);
		//GUnrealEd->ExportMap(World, *ExportFilename, true);


		// @todo: extend this to multiple levels.
		//UWorld* World = GWorld;
		const FString LevelFilename = exportObjects[i]->MeshName;// FileHelpers::GetFilename(World);//->GetOutermost()->GetName() );
															   //FString ExportFilename;
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
	UE_LOG(LogTemp, Warning, TEXT("Found %d Static Meshes for Export"), ActorsExported);


	//TODO export transparent textures for dynamic objects
	//
	ConvertDynamicTextures();
}

FReply FCognitiveToolsCustomization::ReexportDynamicMeshesCmd()
{
	ReexportDynamicMeshes(ExportDynamicsDirectory);
	return FReply::Handled();
}

FReply FCognitiveToolsCustomization::ExportDynamicTextures()
{
	ConvertDynamicTextures();
	return FReply::Handled();
}

FReply FCognitiveToolsCustomization::UploadDynamics()
{	
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	//FString fileExtensions = TEXT("obj");
	//TArray<FString> filesInDirectory = GetAllFilesInDirectory(ExportDynamicsDirectory, true, filesStartingWith, filesStartingWith, pngextension);

	// Get all files in directory
	TArray<FString> directoriesToSkip;
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	TArray<FString> DirectoriesToSkip;
	TArray<FString> DirectoriesToNotRecurse;

	// use the timestamp grabbing visitor (include directories)
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, DirectoriesToSkip, DirectoriesToNotRecurse, true);
	Visitor.Visit(*ExportDynamicsDirectory, true);

	GLog->Log("UploadDynamics found this many files " + Visitor.FileTimes.Num());

	FString sceneID = "";
	FString currentSceneName = GWorld->GetMapName();
	currentSceneName.RemoveFromStart(GWorld->StreamingLevelsPrefix);

	//GConfig->GetArray()
	FConfigSection* Section = GConfig->GetSectionPrivate(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), false, true, GEngineIni);
	if (Section == NULL)
	{
		GLog->Log("can't upload dynamic objects. sceneid not set");
		return FReply::Handled();
	}
	for (FConfigSection::TIterator It(*Section); It; ++It)
	{
		if (It.Key() == TEXT("SceneData"))
		{
			FString name;
			FString key;
			It.Value().GetValue().Split(TEXT(","), &name, &key);
			if (*name == currentSceneName)
			{
				GLog->Log("-----> UPlayerTracker::GetSceneKey found key for scene " + name);
				sceneID = key;
				break;
			}
			else
			{
				GLog->Log("UPlayerTracker::GetSceneKey found key for scene " + name);
			}
		}
	}



	//no matches anywhere
	//CognitiveLog::Warning("UPlayerTracker::GetSceneKey ------- no matches in ini");

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString filePath = TimestampIt.Key();
		const FString fileName = FPaths::GetCleanFilename(filePath);

		if (ExportDynamicsDirectory == filePath)
		{
			GLog->Log("root found " + filePath);
		}
		else if (FPaths::DirectoryExists(filePath))
		{
			GLog->Log("directory found " + filePath);
			FString url = "https://sceneexplorer.com/api/objects/"+sceneID+"/"+fileName;

			UploadFromDirectory(url, filePath, "object");
		}
		else
		{
			//GLog->Log("file found " + filePath);
		}
	}

	return FReply::Handled();
}

void FCognitiveToolsCustomization::ReexportDynamicMeshes(FString directory)
{
	//open blender and run a script
	//TODO make openblender and run a script into a function, because this is used in a bunch of places

	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ExportDynamicMesh.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	UE_LOG(LogTemp, Warning, TEXT("Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();


	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find ExportDynamicMesh.py script at path. Canceling"));
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
	//FString SceneName = tempworld->GetMapName();
	FString ObjPath = directory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No know export directory. Canceling"));
		return;
	}


	//FString MaxPolyCount = FString::FromInt(0);
	FString resizeFactor = FString::FromInt(GetTextureRefacor());

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedTargetPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString stringparams = " -P " + escapedPythonPath + " " + escapedTargetPath;// +" " + resizeFactor;// +" " + MaxPolyCount + " " + SceneName;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	UE_LOG(LogTemp, Warning, TEXT("Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);

	//FString cmdPath = "C:\\Windows\\System32\\cmd.exe";
	//FString cmdPathS = "cmd.exe";
	//FProcHandle procHandle = FPlatformProcess::CreateProc(*cmdPath, NULL, false, false, false, NULL, priorityMod, 0, nullptr);

	//TODO can i just create a process and add parameters or do i need to run through cmd line??
	//system("cmd.exe");
}

void FCognitiveToolsCustomization::ConvertDynamicTextures()
{
	//open blender and run a script
	//TODO make openblender and run a script into a function, because this is used in a bunch of places

	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ConvertDynamicTextures.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	UE_LOG(LogTemp, Warning, TEXT("Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();


	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find ConvertDynamicTextures.py script at path. Canceling"));
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
	//FString SceneName = tempworld->GetMapName();
	FString ObjPath = ExportDynamicsDirectory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No know export directory. Canceling"));
		return;
	}


	//FString MaxPolyCount = FString::FromInt(0);
	FString resizeFactor = FString::FromInt(GetTextureRefacor());

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedTargetPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString stringparams = " -P " + escapedPythonPath + " " + escapedTargetPath + " " + resizeFactor;// +" " + MaxPolyCount + " " + SceneName;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	UE_LOG(LogTemp, Warning, TEXT("Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);

	//FString cmdPath = "C:\\Windows\\System32\\cmd.exe";
	//FString cmdPathS = "cmd.exe";
	//FProcHandle procHandle = FPlatformProcess::CreateProc(*cmdPath, NULL, false, false, false, NULL, priorityMod, 0, nullptr);

	//TODO can i just create a process and add parameters or do i need to run through cmd line??
	//system("cmd.exe");
}

FReply FCognitiveToolsCustomization::Export_Selected()
{
	FEditorFileUtils::Export(true);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	ExportDirectory = FPaths::ConvertRelativePathToFull(ExportDirectory);

	return FReply::Handled();
}

FReply FCognitiveToolsCustomization::Export_All()
{
	FEditorFileUtils::Export(false);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	ExportDirectory = FPaths::ConvertRelativePathToFull(ExportDirectory);

	return FReply::Handled();
}

FReply FCognitiveToolsCustomization::Select_Export_Meshes()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("world is null"));
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


//open fiel type
FReply FCognitiveToolsCustomization::Select_Blender()
{
	FString title = "Select Blender.exe";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickFile(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		BlenderPath = outFilename;
		//UE_LOG(LogTemp, Warning, TEXT("selected blender at path: %s"), *BlenderPath);
	}
	return FReply::Handled();
}

//open fiel type
FReply FCognitiveToolsCustomization::Select_Export_Directory()
{
	FString title = "Select Export Directory";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveToolsCustomization::Select_Export_Directory - picked a directory"));
		ExportDirectory = outFilename;
	}
	return FReply::Handled();
}

//open fiel type
FReply FCognitiveToolsCustomization::SelectDynamicsDirectory()
{
	FString title = "Select Dynamc Export Root Directory";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveToolsCustomization::SelectDynamicsDirectory - picked a directory"));
		ExportDynamicsDirectory = outFilename;
	}
	return FReply::Handled();
}

bool FCognitiveToolsCustomization::PickFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
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

bool FCognitiveToolsCustomization::PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
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

void* FCognitiveToolsCustomization::ChooseParentWindowHandle()
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

FReply FCognitiveToolsCustomization::List_Materials()
{
	List_MaterialArgs("",ExportDirectory);
	return FReply::Handled();
}

void FCognitiveToolsCustomization::List_MaterialArgs(FString subdirectory, FString searchDirectory)
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

	GLog->Log("find mtl in " + FinalPath);

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
				GLog->Log("loaded " + Files[i] + "\n" = contents);
			}
			else
			{
				GLog->Log("failed to load " + Files[i]);
			}
			TArray<FString> lines;
			int32 lineCount = contents.ParseIntoArray(lines, TEXT("\n"), true);

			//GLog->Log("material line count " + lineCount);
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

					//GLog->Log("true material name " + trueMatName);

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
						//GLog->Log("asset ref loaded! " + assetRef.ToString());
						UMaterial* m = Cast<UMaterial>(assetRefLoaded);
						UMaterialInstance* mi = Cast<UMaterialInstance>(assetRefLoaded);

						if (m != NULL)
						{
							if (m->GetBlendMode() == EBlendMode::BLEND_Opaque)
							{
								//GLog->Log("opaque material should already have textures exported correctly");
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

								GLog->Log("writing base color for transparent material " + BMPFilename);
								FFileHelper::CreateBitmap(*BMPFilename, point.X, point.Y, colors.GetData());
							}
						}
						else
						{
							if (mi->GetBlendMode() == EBlendMode::BLEND_Opaque)
							{
								//GLog->Log("opaque material should already have textures exported correctly");
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

								GLog->Log("writing base color for transparent material instance " + BMPFilename);
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
FReply FCognitiveToolsCustomization::Reduce_Meshes()
{
	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("DecimateExportedScene.py");
	const TCHAR* charPath = *pythonscriptpath;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("Reduce_Meshes - Could not find decimateall.py script at path. Canceling"));
		return FReply::Handled();
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reduce_Meshes - No path set for Blender.exe. Canceling"));
		return FReply::Handled();
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reduce_Meshes - World is null. canceling"));
		return FReply::Handled();
	}

	//const TCHAR* url = *stringurl;
	FString SceneName = tempworld->GetMapName();
	FString ObjPath = ExportDirectory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FCognitiveToolsCustomization::Reduce_Meshes No know export directory. Canceling"));
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

	UE_LOG(LogTemp, Warning, TEXT("FCognitiveToolsCustomization::Reduce_Meshes Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);

	//TODO when procHandle is complete, upload exported files to sceneexplorer.com
	return FReply::Handled();
}

FString FCognitiveToolsCustomization::GetProductID()
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

//run this as the next step after exporting the scene
FReply FCognitiveToolsCustomization::Reduce_Textures()
{
	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ConvertTextures.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	UE_LOG(LogTemp, Warning, TEXT("Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();


	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find decimateall.py script at path. Canceling"));
		return FReply::Handled();
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No path set for Blender.exe. Canceling"));
		return FReply::Handled();
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("World is null. canceling"));
		return FReply::Handled();
	}

	//const TCHAR* url = *stringurl;
	FString SceneName = tempworld->GetMapName();
	FString ObjPath = ExportDirectory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No know export directory. Canceling"));
		return FReply::Handled();
	}


	FString MaxPolyCount = FString::FromInt(0);
	FString resizeFactor = FString::FromInt(GetTextureRefacor());

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString stringparams = " -P " + escapedPythonPath + " " + escapedOutPath + " " + resizeFactor + " " + MaxPolyCount + " " + SceneName;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	UE_LOG(LogTemp, Warning, TEXT("Params: %s"), *stringParamSlashed);


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

FReply FCognitiveToolsCustomization::Http_Request()
{
	//TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	//JsonObject->SetStringField(TEXT("some_string_field"), *FString::Printf(TEXT("%s"), *SomeFStringVariable));

	//FString OutputString;

	//TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);

	//FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetVerb("GET");

	//HttpRequest->SetHeader("Content-Type", "application/json");

	HttpRequest->SetURL("https://s3.amazonaws.com/cvr-test/sdkversion.txt");

	//HttpRequest->SetContentAsString(OutputString);

	//HttpRequest->OnProcessRequestComplete().BindUObject(this, &FCognitiveToolsCustomization::OnYourFunctionCompleted);
	//HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveToolsCustomization::OnYourFunctionCompleted);

	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

void FCognitiveToolsCustomization::UploadFromDirectory(FString url, FString directory, FString expectedResponseType)
{
	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	//FString fileExtensions = TEXT("obj");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(directory, true, filesStartingWith, filesStartingWith, pngextension);

	TArray<FString> imagesInDirectory = GetAllFilesInDirectory(directory, true, filesStartingWith, pngextension, filesStartingWith);

	//UploadMultipartData(url, filesInDirectory, imagesInDirectory);

	GLog->Log("url " + url);

	//FString httpbody;
	//FString Content;
	//TArray<uint8> ContentBytes;
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
			UE_LOG(LogTemp, Log, TEXT("Loaded file %s"), *imagesInDirectory[i]);
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

	//HttpRequest->SetURL("http://192.168.1.145:3000/api/scenes");
	HttpRequest->SetURL(url);
	HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=\"cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i\"");
	HttpRequest->SetHeader("Accept-Encoding", "identity");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetContent(AllBytes);

	FHttpModule::Get().SetHttpTimeout(0);

	if (expectedResponseType == "scene")
	{
		HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveToolsCustomization::OnUploadSceneCompleted);
	}
	if (expectedResponseType == "object")
	{
		HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveToolsCustomization::OnUploadObjectCompleted);
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

FReply FCognitiveToolsCustomization::UploadScene()
{
	FString url = "https://sceneexplorer.com/api/scenes";

	UploadFromDirectory(url, ExportDirectory,"scene");

	return FReply::Handled();
}

void FCognitiveToolsCustomization::OnUploadSceneCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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

		SaveSceneData(currentSceneName, responseNoQuotes);
	}
}

void FCognitiveToolsCustomization::OnUploadObjectCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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

		GLog->Log("Upload Dynamic Complete " + Request->GetURL());

		//SaveSceneData(currentSceneName, responseNoQuotes);
	}
}

FReply FCognitiveToolsCustomization::DebugSendSceneData()
{
	//+ FString::FromInt(FMath::Rand())
	SaveSceneData("FirstPersonExampleMap", "1234-asdf-5678-hjkl");
	return FReply::Handled();
}

void FCognitiveToolsCustomization::SaveSceneData(FString sceneName, FString sceneKey)
{
	FString keyValue = sceneName + "," + sceneKey;
	UE_LOG(LogTemp, Warning, TEXT("Upload complete! Add this into the SceneData array in Project Settings:      %s"),*keyValue);


	//TODO why can't i save to an ini file that automatically gets included in a build? why is that so hard?
	return;

	//FString keyValue = sceneName + "," + sceneKey;

	GConfig->Flush(true, "DefaultEngineIni");
	TArray<FString> scenePairs;

	GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, "DefaultEngineIni");

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
	for (int32 i = scenePairs.Num()-1; i >= 0; i--)
	{
		FString name;
		FString key;
		if (!scenePairs[i].Split(TEXT(","), &name, &key))
		{
			scenePairs.RemoveAt(i);
		}
	}

	GConfig->SetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, "DefaultEngineIni");

	GConfig->Flush(false, "DefaultEngineIni");
	//GConfig->UnloadFile(GEngineIni);
	//GConfig->LoadFile(GEngineIni);
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
TArray<FString> FCognitiveToolsCustomization::GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension, const FString ignoreExtension)
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

bool FCognitiveToolsCustomization::HasFoundBlender() const
{
	return FCognitiveToolsCustomization::GetBlenderPath().ToString().Contains("blender.exe");
}

bool FCognitiveToolsCustomization::HasFoundBlenderAndExportDir() const
{
	return FCognitiveToolsCustomization::GetBlenderPath().ToString().Contains("blender.exe") && !FCognitiveToolsCustomization::GetExportDirectory().EqualTo(FText::FromString(""));
}

bool FCognitiveToolsCustomization::HasFoundBlenderAndDynamicExportDir() const
{
	return FCognitiveToolsCustomization::GetBlenderPath().ToString().Contains("blender.exe") && !FCognitiveToolsCustomization::GetDynamicExportDirectory().EqualTo(FText::FromString(""));
}

bool FCognitiveToolsCustomization::HasSetExportDirectory() const
{
	return !FCognitiveToolsCustomization::GetExportDirectory().EqualTo(FText::FromString(""));
}

bool FCognitiveToolsCustomization::HasSetDynamicExportDirectory() const
{
	return !FCognitiveToolsCustomization::GetDynamicExportDirectory().EqualTo(FText::FromString(""));
}

FText FCognitiveToolsCustomization::GetBlenderPath() const
{
	return FText::FromString(BlenderPath);
}

FText FCognitiveToolsCustomization::GetExportDirectory() const
{
	return FText::FromString(ExportDirectory);
}

FText FCognitiveToolsCustomization::GetDynamicExportDirectory() const
{
	return FText::FromString(ExportDynamicsDirectory);
}

void FCognitiveToolsCustomization::SearchForBlender()
{
	//try to find blender in program files
	FString testApp = "C:/Program Files/Blender Foundation/Blender/blender.exe";

	if (VerifyFileExists(testApp))
	{
		UE_LOG(LogTemp, Warning, TEXT("SearchForBlender - Found Blender in Program Files"));
		BlenderPath = testApp;
	}
}

TSharedRef<IDetailCustomization> FCognitiveToolsCustomization::MakeInstance()
{
	return MakeShareable(new FCognitiveToolsCustomization);
}

FReply FCognitiveToolsCustomization::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder->GetObjectsBeingCustomized(ObjectsBeingCustomized);

	for (auto WeakObject : ObjectsBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			Instance->CallFunctionByNameWithArguments(*MethodToExecute->GetName(), *GLog, nullptr, true);
		}
	}


	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE