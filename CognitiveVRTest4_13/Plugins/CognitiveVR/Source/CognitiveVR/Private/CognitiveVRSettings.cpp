// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"
#include "CognitiveVRSettings.h"

#define LOCTEXT_NAMESPACE "CognitiveVRLoc"

UCognitiveVRSettings::UCognitiveVRSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SettingsDisplayName = LOCTEXT("SettingsDisplayName", "Cognitive VR");
	SettingsTooltip = LOCTEXT("SettingsTooltip", "Cognitive VR analytics configuration settings");
}

void UCognitiveVRSettings::ReadConfigSettings()
{

}

void UCognitiveVRSettings::WriteConfigSettings()
{
	//full
	if (EnableFullDebugLogging)
	{
		FAnalytics::Get().WriteConfigValueToIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugAll"), "true");
	}
	else
	{
		FAnalytics::Get().WriteConfigValueToIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugAll"), "");
	}
												
	//errors only					
	if (EnableErrorDebugLogging)			
	{										
		FAnalytics::Get().WriteConfigValueToIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugError"), "true");
	}										
	else									
	{			
		FAnalytics::Get().WriteConfigValueToIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugError"), "");
	}

	GConfig->Flush(false, "DefaultEngine");
}

void UCognitiveVRSettings::Export_Selected()
{
	//SelectExportMeshes();

	FEditorFileUtils::Export(true);

	//ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	//RunBlenderCleanup();
}

void UCognitiveVRSettings::Export_All()
{
	//SelectExportMeshes();

	FEditorFileUtils::Export(false);

	//ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	//RunBlenderCleanup();
}

void UCognitiveVRSettings::Select_Export_Meshes()
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

		//get meshes in size range
		FVector origin;
		FVector boxBounds;

		ObstacleItr->GetActorBounds(false, origin, boxBounds);
		double magnitude = FMath::Sqrt(boxBounds.X*boxBounds.X + boxBounds.Y*boxBounds.Y + boxBounds.Z*boxBounds.Z);

		if (magnitude < MinimumSize)
		{
			continue;
		}
		if (magnitude > MaximumSize)
		{
			continue;
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


//open fiel type
void UCognitiveVRSettings::Select_Blender()
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

bool UCognitiveVRSettings::PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
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

void* UCognitiveVRSettings::ChooseParentWindowHandle()
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
void UCognitiveVRSettings::Reduce_Meshes()
{
	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("DecimateExportedScene.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	UE_LOG(LogTemp, Warning, TEXT("Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

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

	FString productID = GetProductID();

	FString stringparams = " -P " + escapedPythonPath + " " + escapedOutPath + " " + MinPolyCount + " " + MaxPolyCount + " " + SceneName + " " + productID;

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
}

FString UCognitiveVRSettings::GetProductID()
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
void UCognitiveVRSettings::Reduce_Textures()
{
	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ConvertTextures.py");
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

	FString MaxPolyCount = FString::FromInt(MaxPolygons);

	FString resizeFactor = FString::FromInt(TextureResizeFactor);

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
}

void UCognitiveVRSettings::Http_Request()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	//JsonObject->SetStringField(TEXT("some_string_field"), *FString::Printf(TEXT("%s"), *SomeFStringVariable));

	FString OutputString;

	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);

	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetVerb("GET");

	//HttpRequest->SetHeader("Content-Type", "application/json");

	HttpRequest->SetURL("https://s3.amazonaws.com/cvr-test/sdkversion.txt");

	//HttpRequest->SetContentAsString(OutputString);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCognitiveVRSettings::OnYourFunctionCompleted);

	HttpRequest->ProcessRequest();
}

void UCognitiveVRSettings::OnYourFunctionCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		//TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		//TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create();

		//FJsonSerializer::Deserialize(JsonReader, JsonObject);
		//Response->GetContentAsString()
		UE_LOG(LogTemp, Warning, TEXT("Response is %s"), *Response->GetContentAsString());

		//SomeOtherVariable = JsonObject->GetStringField("some_response_field");

	}
	else
	{
		// Handle error here
	}
}

#undef LOCTEXT_NAMESPACE