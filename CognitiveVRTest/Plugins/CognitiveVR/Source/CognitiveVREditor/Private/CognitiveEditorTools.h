#pragma once

#include "CoreMinimal.h"

#include "CognitiveVRSettings.h"
#include "CognitiveEditorData.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "UnrealEd.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "BusyCursor.h"
#include "Classes/Components/SceneComponent.h"
#include "EngineUtils.h"
#include "EditorDirectories.h"
#include "AssetTypeActions_Base.h"
#include "ObjectTools.h"
#include "PlatformProcess.h"
#include "DesktopPlatformModule.h"
#include "MainFrame.h"
#include "IPluginManager.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "MaterialUtilities.h"
#include "MaterialBakingStructures.h"
#include "IMaterialBakingModule.h"
#include "MeshUtilities.h"
#include "UObject/Object.h"
#include "MaterialOptions.h"
#include "DynamicObject.h"
#include "GenericPlatformFile.h"
#include "Http.h"
#include "UnrealClient.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RenderingThread.h"
#include "Classes/Engine/Level.h"
#include "CoreMisc.h"
#include "C3DCommonEditorTypes.h"

//all sorts of functionality for Cognitive SDK


class FCognitiveEditorTools
{
public:
	static void Initialize();
	static FString Gateway;

	static void CheckIniConfigured();

	static FCognitiveEditorTools* CognitiveEditorToolsInstance;
	static FCognitiveEditorTools* GetInstance();

	FOnUploadSceneGeometry OnUploadSceneGeometry;

	void SaveSceneData(FString sceneName, FString sceneKey);

	//gets all the dynamics in the scene and saves them to SceneDynamics
	TArray<TSharedPtr<FDynamicData>> SceneDynamics;
	TArray<TSharedPtr<FDynamicData>> GetSceneDynamics();

	void OnApplicationKeyChanged(const FText& Text);
	void OnDeveloperKeyChanged(const FText& Text);
	void OnAttributionKeyChanged(const FText& Text);
	void OnExportPathChanged(const FText& Text);

	FText GetApplicationKey() const;
	FText GetDeveloperKey() const;
	FText GetAttributionKey() const;

	FString ApplicationKey;
	FString AttributionKey;
	FString DeveloperKey;

	FString GetDynamicObjectManifest(FString versionid);

	//POST dynamic object manifest
	FString PostDynamicObjectManifest(FString sceneid, int32 versionnumber);

	//POST dynamic object mesh data
	FString PostDynamicObjectMeshData(FString sceneid, int32 versionnumber, FString exportdirectory);

	//GET scene settings and read scene version
	FString GetSceneVersion(FString sceneid);

	//POST scene screenshot
	FString PostScreenshot(FString sceneid, FString versionnumber);

	//POST upload decimated scene
	FString PostNewScene();

	//POST upload and replace existing scene
	FString PostUpdateScene(FString sceneid);

	//WEB used to open scenes on sceneexplorer
	FString SceneExplorerOpen(FString sceneid);

	TSharedPtr<IImageWrapper> ImageWrapper;


	bool CurrentSceneHasSceneId() const;


	bool HasSetExportDirectory() const;

	bool HasSetDynamicExportDirectory() const;

	int32 CountDynamicObjectsInScene() const;
	bool DuplicateDynamicIdsInScene() const;
	FText DynamicCountInScene;
	FText DisplayDynamicObjectsCountInScene() const;
	FText DisplayDynamicObjectsCountOnWeb() const;
	FText GetDynamicObjectUploadText() const;

	FReply RefreshDisplayDynamicObjectsCountInScene();

	EVisibility GetDuplicateDyanmicObjectVisibility() const;
	EVisibility DuplicateDyanmicObjectVisibility = EVisibility::Hidden;

	FText UploadDynamicsToSceneText;
	FText GetUploadDynamicsToSceneText() const;

	float MinimumSize = 1;
	float MaximumSize = 10000;
	bool StaticOnly = true;
	int32 MinPolygon = 65536;
	int32 MaxPolygon = 65536;
	int32 TextureRefactor = 1;
	FString ExcludeMeshes = "Camera,Player,SkySphereBlueprint";

	float GetMinimumSize() const { return MinimumSize; }
	float GetMaximumSize() const { return MaximumSize; }
	bool GetStaticOnly() const { return StaticOnly; }
	ECheckBoxState GetStaticOnlyCheckboxState() const
	{
		return (StaticOnly)
			? ECheckBoxState::Checked
			: ECheckBoxState::Unchecked;
	}
	void OnStaticOnlyCheckboxChanged(ECheckBoxState newstate)
	{
		if (newstate == ECheckBoxState::Checked)
		{
			StaticOnly = true;
		}
		else
		{
			StaticOnly = false;
		}
	}

	int32 GetMinPolygon() const { return MinPolygon; }
	int32 GetMaxPolygon() const { return MaxPolygon; }
	int32 GetTextureRefactor() const { return TextureRefactor; }
	FText GetExcludeMeshes() const { return FText::FromString(ExcludeMeshes); }

		FReply UploadScene();

	TArray<AActor*> PrepareSceneForExport(bool OnlyExportSelected);
	
	void UploadFromDirectory(FString url, FString directory, FString expectedResponseType);

	//dynamic objects
	//Runs the built-in gltf exporter with all meshes
	FProcHandle ExportAllDynamics();
	//Runs the built-in gltf exporter with all meshes that don't have an exported .gltf
		FProcHandle ExportNewDynamics();

	//Runs the built-in gltf exporter with selected meshes
		FReply ExportSelectedDynamics();
		FProcHandle ExportDynamicData(TArray< TSharedPtr<FDynamicData>> dynamicData);
	

	//this is the important function for exporting dynamics. all other other dynamic export functions lead to this
		//sets position to origin, export as gltf, generate screenshot
	FProcHandle ExportDynamicObjectArray(TArray<UDynamicObject*> exportObjects);

	TArray<FAssetData> TempAssetsToDelete;

	//uploads each dynamic object using its directory to the current scene
	UFUNCTION(Exec, Category = "Dynamics")
		FReply UploadDynamics();
	UFUNCTION(Exec, Category = "Dynamics")
		FReply UploadDynamic(FString directory);

	//this is for aggregating dynamic objects
	UFUNCTION(Exec, Category = "Dynamics Manifest")
		FReply UploadDynamicsManifest();

	//this is for aggregating dynamic objects
	UFUNCTION(Exec, Category = "Dynamics Manifest")
		FReply UploadSelectedDynamicsManifest(TArray<UDynamicObject*> selectedData);

	UFUNCTION(Exec, Category = "Dynamics Manifest")
		FReply UploadDynamicsManifestIds(TArray<FString> ids, FString meshName, FString prefabName);

	UFUNCTION(Exec, Category = "Dynamics Manifest")
		FReply SetUniqueDynamicIds();

	FReply GetDynamicsManifest();
	void OnDynamicManifestResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void OnUploadSceneCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnUploadObjectCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnUploadManifestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnUploadScreenshotCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	bool PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename);
	bool PickFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename);
	void* ChooseParentWindowHandle();


	FString BaseExportDirectory;


	//c:/users/me/desktop/export/
	FText GetBaseExportDirectoryDisplay() const;
	FString GetBaseExportDirectory() const
	{
		return BaseExportDirectory;
	}
	//c:/users/me/desktop/export/scenename/
	FText GetSceneExportDirectoryDisplay(FString scenename) const;
	FString GetSceneExportDirectory(FString scenename)
	{
		return FPaths::Combine(BaseExportDirectory, scenename);
	}
	//c:/users/me/desktop/export/scenename/
	FText GetCurrentSceneExportDirectoryDisplay() const;
	FString GetCurrentSceneExportDirectory()
	{
		UWorld* myworld = GWorld->GetWorld();

		FString currentSceneName = myworld->GetMapName();
		return FPaths::Combine(BaseExportDirectory, currentSceneName);
	}
	//c:/users/me/desktop/export/dynamics/
	FText GetDynamicsExportDirectoryDisplay() const;
	FString GetDynamicsExportDirectory()
	{
		return FPaths::Combine(BaseExportDirectory, TEXT("dynamics"));
	}
	FReply SelectBaseExportDirectory();

	FString GetCurrentSceneName() const
	{
		UWorld* myworld = GWorld->GetWorld();

		return myworld->GetMapName();
	}

	bool DynamicMeshDirectoryExists(FString meshname)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString path = FPaths::Combine(GetDynamicsExportDirectory(), meshname);
		return PlatformFile.DirectoryExists(*path);
	}

	bool CurrentSceneDirectoryExists()
	{
		FString scenePath = GetSceneExportDirectory(GetCurrentSceneName());
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		return PlatformFile.DirectoryExists(*scenePath);
	}

	void CurrentSceneVersionRequest();

	TArray<FString> GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension, const FString ignoreExtension, bool skipsubdirectory) const;

	//If this function cannot find or create the directory, returns false.
	static FORCEINLINE bool VerifyOrCreateDirectory(FString& TestDir)
	{
		// Every function call, unless the function is inline, adds a small
		// overhead which we can avoid by creating a local variable like so.
		// But beware of making every function inline!
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// Directory Exists?
		if (!PlatformFile.DirectoryExists(*TestDir))
		{
			PlatformFile.CreateDirectory(*TestDir);

			if (!PlatformFile.DirectoryExists(*TestDir))
			{
				return false;
				//~~~~~~~~~~~~~~
			}
		}
		return true;
	}

	//If this function cannot find the file, returns false.
	static FORCEINLINE bool VerifyFileExists(FString& TestPath)
	{
		// Every function call, unless the function is inline, adds a small
		// overhead which we can avoid by creating a local variable like so.
		// But beware of making every function inline!
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.FileExists(*TestPath))
		{
			return true;
		}
		return false;
	}

	//If this function cannot find the file, returns false.
	static FORCEINLINE bool VerifyDirectoryExists(FString& TestPath)
	{
		// Every function call, unless the function is inline, adds a small
		// overhead which we can avoid by creating a local variable like so.
		// But beware of making every function inline!
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.DirectoryExists(*TestPath))
		{
			return true;
		}
		return false;
	}


	TArray<TSharedPtr<FEditorSceneData>> SceneData;
	//returns SceneData array
	TArray<TSharedPtr<FEditorSceneData>> GetSceneData() const;

	FReply OpenSceneInBrowser(FString sceneid);
	FReply OpenCurrentSceneInBrowser();
	bool HasDeveloperKey() const;
	bool HasApplicationKey() const;
	
	//reads scene data from ini
	void ReadSceneDataFromFile();

	//send a http request to get the scene version data for current scene from sceneexplorer
	FReply ButtonCurrentSceneVersionRequest();
	
	//returns data about a scene by name
	TSharedPtr<FEditorSceneData> GetSceneData(FString scenename) const;
	//returns data about a scene by the currently open scene
	TSharedPtr<FEditorSceneData> GetCurrentSceneData() const;

	//has json file and no bmp files in export directory
	bool HasConvertedFilesInDirectory() const;
	bool CanUploadSceneFiles() const;

	void SceneVersionRequest(FEditorSceneData data);
	void SceneVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	TArray<TSharedPtr<FDynamicData>> SceneExplorerDynamics;
	TArray<TSharedPtr<FString>> SubDirectoryNames;

	void RefreshSceneUploadFiles();
	int32 GetSceneExportFileCount();
	int32 GetDynamicObjectFileExportedCount();
	int32 GetDynamicObjectExportedCount();

	void RefreshDynamicUploadFiles();
	TArray<TSharedPtr<FString>> SceneUploadFiles;
	TArray<TSharedPtr<FString>> DynamicUploadFiles;
	
	TArray<FString> GetValidDirectories(const FString& Directory);
	int32 DynamicObjectExportedCount;

	FText UploadSceneNameFiles() const;
	FText OpenSceneNameInBrowser() const;
	FReply OpenURL(FString url);
	void FindAllSubDirectoryNames();
	TArray<TSharedPtr<FString>> GetSubDirectoryNames();
	FReply SaveScreenshotToFile();
	FReply TakeDynamicScreenshot(FString dynamicName);

	void DelayScreenshot(FString filePath, FLevelEditorViewportClient* perspectiveView, FVector startPos, FRotator startRot);
	
	FText GetDynamicsOnSceneExplorerTooltip() const;
	FText SendDynamicsToSceneExplorerTooltip() const;
	FReply RefreshDynamicSubDirectory();
	bool ConfigFileHasChanged = false;
	EVisibility ConfigFileChangedVisibility() const;

	bool HasSetDynamicExportDirectoryHasSceneId() const;
	FReply SaveAPIDeveloperKeysToFile();

	void SaveApplicationKeyToFile(FString key);
	void SaveDeveloperKeyToFile(FString key);

	void WizardUpload();
	bool IsWizardUploading();

	//set to 500, 404, 401 if uploading from the wizard encountered and error
	FString WizardUploadError;
	int32 WizardUploadResponseCode;
	void CreateExportFolderStructure();

	bool HasExportedAnyDynamicMeshes() const;

	FString BuildDebugFileContents() const;

	void AppendDirectoryContents(FString FullPath, int32 depth, FString& outputString);


	//exporting scene.
	//create directory
	//export scene as gltf
	void ExportScene(TArray<AActor*> actorsToExport);

	void ValidateGeneratedFiles();

	bool RenameFile(FString oldPath, FString newPath);

	void ModifyGLTFContent(FString FilePath);

	void GenerateSettingsJsonFile();
	bool HasSettingsJsonFile() const;

	const FSlateBrush* GetBoxEmptyIcon() const;
	FSlateBrush* BoxEmptyIcon;
	const FSlateBrush* GetBoxCheckIcon() const;
	FSlateBrush* BoxCheckIcon;
	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
};

//used for uploading multiple dynamics at once
class FContentContainer
{
public:
	FString Headers;
	FString BodyText;
	TArray<uint8> BodyBinary;
};