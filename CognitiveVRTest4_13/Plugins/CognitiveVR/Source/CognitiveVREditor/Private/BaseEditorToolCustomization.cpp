
#include "CognitiveVREditorPrivatePCH.h"
#include "BaseEditorToolCustomization.h"


#define LOCTEXT_NAMESPACE "BaseToolEditor"

void FBaseEditorToolCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSet<UClass*> Classes;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	

	UClass* Class;

	for (auto WeakObject : ObjectsBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			Class = Instance->GetClass();
			break;
			//Classes.Add(Instance->GetClass());
			//BlenderPathProperty = DetailBuilder.GetProperty("BlenderPath", Instance->GetClass());
		}
	}


	// Create a commands category
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Commands"));

	FText p = GetBlenderPath();
	if (p.EqualTo(FText::FromString("")) && !HasSearchedForBlender)
	{
		HasSearchedForBlender = true;
		UE_LOG(LogTemp, Warning, TEXT("blender path is empty. search for blender"));
		SearchForBlender();
	}


	Category.AddCustomRow(FText::FromString("Settings"))
	.WholeRowContent()
	.HAlign(HAlign_Center)
	[
		SNew(STextBlock)
		.Text(this, &FBaseEditorToolCustomization::GetBlenderPath)
	];


	//select blender
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(true)
		.Text(FText::FromString("Select Blender"))
		.OnClicked(this, &FBaseEditorToolCustomization::Select_Blender)
		];

	//select export meshes
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlender)
		.Text(FText::FromString("Select Export Meshes"))
		.OnClicked(this, &FBaseEditorToolCustomization::Select_Export_Meshes)
		];

	//export selected scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlender)
		.Text(FText::FromString("Export Selected"))
		.OnClicked(this, &FBaseEditorToolCustomization::Export_Selected)
		];

	//export whole scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlender)
		.Text(FText::FromString("Export All"))
		.OnClicked(this, &FBaseEditorToolCustomization::Export_All)
		];

	Category.AddCustomRow(FText::FromString("Settings"))
		.WholeRowContent()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(this, &FBaseEditorToolCustomization::GetExportDirectory)
		];

	//export selected scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.Text(FText::FromString("Select Export Directory"))
		.OnClicked(this, &FBaseEditorToolCustomization::Select_Export_Directory)
		];

	//Reduce Meshes
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlenderAndExportDir)
		.Text(FText::FromString("Reduce Meshes"))
		.OnClicked(this, &FBaseEditorToolCustomization::Reduce_Meshes)
		];

	//Reduce Textures
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlenderAndExportDir)
		.Text(FText::FromString("Reduce Textures"))
		.OnClicked(this, &FBaseEditorToolCustomization::Reduce_Textures)
		];



	//http request
	/*Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(true)
		.Text(FText::FromString("http request"))
		.OnClicked(this, &FBaseEditorToolCustomization::Http_Request)
		];*/
}


FReply FBaseEditorToolCustomization::Export_Selected()
{
	//SelectExportMeshes();

	FEditorFileUtils::Export(true);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	//RunBlenderCleanup();
	return FReply::Handled();
}

FReply FBaseEditorToolCustomization::Export_All()
{
	//SelectExportMeshes();

	FEditorFileUtils::Export(false);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	//RunBlenderCleanup();
	return FReply::Handled();
}

FReply FBaseEditorToolCustomization::Select_Export_Meshes()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("world is null"));
		return FReply::Handled();
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

		

		GEditor->SelectActor(tempactor, true, false, true);
		ActorsExported++;
	}
	UE_LOG(LogTemp, Warning, TEXT("Found %d Static Meshes for Export"), ActorsExported);
	return FReply::Handled();
}


//open fiel type
FReply FBaseEditorToolCustomization::Select_Blender()
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
FReply FBaseEditorToolCustomization::Select_Export_Directory()
{
	FString title = "Select Export Directory";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		UE_LOG(LogTemp, Warning, TEXT("picked a directory"));
		ExportDirectory = outFilename;
		//UE_LOG(LogTemp, Warning, TEXT("selected blender at path: %s"), *BlenderPath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("somehow failed to pick a directory"));
	}
	return FReply::Handled();
}

bool FBaseEditorToolCustomization::PickFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
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

bool FBaseEditorToolCustomization::PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
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

	/*if (directoryChosen)
	{
		// User successfully chose a file; remember the path for the next time the dialog opens.
		InOutLastPath = OutFilenames[0];
		OutFilename = OutFilenames[0];
	}*/

	return directoryChosen;
}

void* FBaseEditorToolCustomization::ChooseParentWindowHandle()
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
FReply FBaseEditorToolCustomization::Reduce_Meshes()
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
	return FReply::Handled();
}

FString FBaseEditorToolCustomization::GetProductID()
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
FReply FBaseEditorToolCustomization::Reduce_Textures()
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
	return FReply::Handled();
}

FReply FBaseEditorToolCustomization::Http_Request()
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

	//HttpRequest->OnProcessRequestComplete().BindUObject(this, &FBaseEditorToolCustomization::OnYourFunctionCompleted);
	HttpRequest->OnProcessRequestComplete().BindSP(this, &FBaseEditorToolCustomization::OnYourFunctionCompleted);

	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

void FBaseEditorToolCustomization::OnYourFunctionCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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

bool FBaseEditorToolCustomization::HasFoundBlender() const
{
	return FBaseEditorToolCustomization::GetBlenderPath().ToString().Contains("blender.exe");
}

bool FBaseEditorToolCustomization::HasFoundBlenderAndExportDir() const
{
	return FBaseEditorToolCustomization::GetBlenderPath().ToString().Contains("blender.exe") && !FBaseEditorToolCustomization::GetExportDirectory().EqualTo(FText::FromString(""));
}

bool FBaseEditorToolCustomization::HasSetExportDirectory() const
{
	return !FBaseEditorToolCustomization::GetExportDirectory().EqualTo(FText::FromString(""));
}

FText FBaseEditorToolCustomization::GetBlenderPath() const
{
	return FText::FromString(BlenderPath);

	//FString blendPath = "";
	//BlenderPathProperty.Get()->GetValue(blendPath);
	
	//return FText::FromString(blendPath);
}

FText FBaseEditorToolCustomization::GetExportDirectory() const
{
	return FText::FromString(ExportDirectory);
}

void FBaseEditorToolCustomization::SearchForBlender()
{
	//try to find blender in program files
	FString testApp = "C:/Program Files/Blender Foundation/Blender/blender.exe";

	if (VerifyFileExists(testApp))
	{
		UE_LOG(LogTemp, Warning, TEXT("found blender at program files"));
		BlenderPath = testApp;
		//BlenderPathProperty.Get()->SetValue(testApp);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("didnt find blender at program files"));
	}
}

TSharedRef<IDetailCustomization> FBaseEditorToolCustomization::MakeInstance()
{
	return MakeShareable(new FBaseEditorToolCustomization);
}

FReply FBaseEditorToolCustomization::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute)
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