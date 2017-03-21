
#include "CognitiveVREditorPrivatePCH.h"
#include "BaseEditorToolCustomization.h"
#include "CognitiveVRSettings.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

void FBaseEditorToolCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
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
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Commands"));

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

			+ SHorizontalBox::Slot()
			//.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
			[
				SNew(SButton)
				.IsEnabled(true)
				.Text(FText::FromString("Select Blender"))
				.OnClicked(this, &FBaseEditorToolCustomization::Select_Blender)
			]

			+ SHorizontalBox::Slot()
			.Padding(FMargin(4.0f, 0.0f, 30.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(this, &FBaseEditorToolCustomization::GetBlenderPath)
			]
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

	Category.AddCustomRow(FText::FromString("Select Export Directory Horizontal"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			//.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
			[
				SNew(SButton)
				.IsEnabled(true)
				.Text(FText::FromString("Select Export Directory"))
				.OnClicked(this, &FBaseEditorToolCustomization::Select_Export_Directory)
			]

			+ SHorizontalBox::Slot()
			.Padding(FMargin(4.0f, 0.0f, 30.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(this, &FBaseEditorToolCustomization::GetExportDirectory)
			]
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
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
		.IsEnabled(true)
		.Text(FText::FromString("Upload Scene"))
		.OnClicked(this, &FBaseEditorToolCustomization::UploadScene)
		];

	IDetailCategoryBuilder& SceneKeyCategory = DetailBuilder.EditCategory(TEXT("Scene Keys"));

	SceneKeysProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, SceneKeyPair));

	SceneKeyCategory.AddProperty(SceneKeysProperty);
}

float FBaseEditorToolCustomization::GetMinimumSize()
{
	float MinSize = 0;
	MinSizeProperty->GetValue(MinSize);
	return MinSize;
}

float FBaseEditorToolCustomization::GetMaximumSize()
{
	float MaxSize = 0;
	MaxSizeProperty->GetValue(MaxSize);
	return MaxSize;
}

bool FBaseEditorToolCustomization::GetStaticOnly()
{
	bool staticOnly = false;
	StaticOnlyProperty->GetValue(staticOnly);
	return staticOnly;
}

int FBaseEditorToolCustomization::GetMinPolygon()
{
	int MinCount = 0;
	MinPolygonProperty->GetValue(MinCount);
	return MinCount;
}

int FBaseEditorToolCustomization::GetMaxPolygon()
{
	int MaxCount = 0;
	MaxPolygonProperty->GetValue(MaxCount);
	return MaxCount;
}

int FBaseEditorToolCustomization::GetTextureRefacor()
{
	int TextureRefactor = 0;
	TextureResizeProperty->GetValue(TextureRefactor);
	return TextureRefactor;
}

FReply FBaseEditorToolCustomization::Export_Selected()
{
	FEditorFileUtils::Export(true);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	return FReply::Handled();
}

FReply FBaseEditorToolCustomization::Export_All()
{
	FEditorFileUtils::Export(false);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

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

	FString MinPolyCount = FString::FromInt(GetMinPolygon());
	FString MaxPolyCount = FString::FromInt(GetMaxPolygon());

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString productID = GetProductID();

	FString stringparams = " -P " + escapedPythonPath + " " + escapedOutPath + " " + MinPolyCount + " " + MaxPolyCount + " " + SceneName + " " + productID + " " + COGNITIVEVR_SDK_VERSION;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	UE_LOG(LogTemp, Warning, TEXT("Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);

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

FReply FBaseEditorToolCustomization::UploadScene()
{
	//TODO get all the exported files
	//make a multipart form

	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	//FString fileExtensions = TEXT("obj");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(ExportDirectory, true, filesStartingWith, filesStartingWith,pngextension);

	TArray<FString> imagesInDirectory = GetAllFilesInDirectory(ExportDirectory, true, filesStartingWith, pngextension,filesStartingWith);
	
	FString Content;

	UE_LOG(LogTemp, Log, TEXT("image count%d"),imagesInDirectory.Num());
	UE_LOG(LogTemp, Log, TEXT("file count%d"), filesInDirectory.Num());

	for (int32 i = 0; i < filesInDirectory.Num(); i++)
	{
		FString result;
		//FString filesStartingWith;
		//const TCHAR* dirchars = *ExportDirectory;
		//const TCHAR* filechars = *filesInDirectory[i];
		//FString totalfilechars = FPaths::Combine(dirchars, filechars);
		if (FFileHelper::LoadFileToString(result, *filesInDirectory[i]))
		{
			//loaded the file
			Content = Content.Append("\n\nboundary=--kdETdJKGXvWOQpWe1pJ9Qe43dYBmJJzcs39Zhqwa\n");

			FString left;
			FString right;
			filesInDirectory[i].Split(".", &left, &right);

			Content = Content.Append("Cotent-Type: application/octet-stream\n");
			Content = Content.Append("Content-Disposition: form-data; name=\"file\";filename=" + FPaths::GetCleanFilename(filesInDirectory[i]) +"\"\n");
			Content = Content.Append(result);
			Content = Content.Append("\n");
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("failed to load %s"), *filesInDirectory[i]);
		}
	}

	for (int32 i = 0; i < imagesInDirectory.Num(); i++)
	{
		FString result;
		//const TCHAR* dirchars = *ExportDirectory;
		//const TCHAR* filechars = *imagesInDirectory[i];
		//const TCHAR* totalfilechars = *FPaths::Combine(dirchars, filechars);
		if (FFileHelper::LoadFileToString(result, *imagesInDirectory[i]))
		{
			//loaded the file
			Content = Content.Append("\n\nboundary=--kdETdJKGXvWOQpWe1pJ9Qe43dYBmJJzcs39Zhqwa\n");

			FString left;
			FString right;
			imagesInDirectory[i].Split(".", &left, &right);
			

			Content = Content.Append("Cotent-Type: image/png\n");
			Content = Content.Append("Content-Disposition: form-data; name=\"file\";filename=" + FPaths::GetCleanFilename(imagesInDirectory[i]) + "\"\n");
			Content = Content.Append(result);
			Content = Content.Append("\n");
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("failed to load %s"), *imagesInDirectory[i]);
		}
	}

	Content = Content.Append("\nboundary=--kdETdJKGXvWOQpWe1pJ9Qe43dYBmJJzcs39Zhqwa--");



	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL("https://sceneexplorer.com/api/scenes/");

	HttpRequest->SetVerb("POST");


	//set content as uint
	TArray<uint8> OutBytes;
	TArray<TCHAR> InStrCharArray = Content.GetCharArray();

	for (int i = 0; i<Content.Len(); i++)
	{
		uint8 CharBytes[sizeof(TCHAR)];
		FMemory::Memcpy(&CharBytes[0], &InStrCharArray[i], sizeof(TCHAR));

		for (int CharIdx = 0; CharIdx<sizeof(TCHAR); CharIdx++)
		{
			OutBytes.Add(CharBytes[CharIdx]);
		}
	}
	HttpRequest->SetContent(OutBytes);


	//set content as string

	//HttpRequest->SetContentAsString(Content);

	HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=--kdETdJKGXvWOQpWe1pJ9Qe43dYBmJJzcs39Zhqwa");
	HttpRequest->SetHeader("Content-Length", FString::FromInt(OutBytes.Num()));
	

	HttpRequest->OnProcessRequestComplete().BindSP(this, &FBaseEditorToolCustomization::OnUploadSceneCompleted);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Content);

	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

void FBaseEditorToolCustomization::OnUploadSceneCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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

//https://answers.unrealengine.com/questions/212791/how-to-get-file-list-in-a-directory.html
/**
Gets all the files in a given directory.
@param directory The full path of the directory we want to iterate over.
@param fullpath Whether the returned list should be the full file paths or just the filenames.
@param onlyFilesStartingWith Will only return filenames starting with this string. Also applies onlyFilesEndingWith if specified.
@param onlyFilesEndingWith Will only return filenames ending with this string (it looks at the extension as well!). Also applies onlyFilesStartingWith if specified.
@return A list of files (including the extension).
*/
TArray<FString> FBaseEditorToolCustomization::GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension, const FString ignoreExtension)
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