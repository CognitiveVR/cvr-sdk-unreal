/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h" 
#include "PropertyEditing.h" 
#include "PropertyCustomizationHelpers.h" 
#include "Json.h" 
#include "JsonObjectConverter.h" 
#include "UnrealEd.h" 
#include "GLTFExportOptions.h" 
#include "EngineUtils.h" 
#include "PlatformProcess.h" 
#include "MainFrame.h" 
#include "AssetRegistryModule.h" 
#include "IAssetRegistry.h" 
#include "MeshUtilities.h" 
#include "UObject/Object.h" 
#include "Http.h" 
#include "UnrealClient.h" 
#include "RenderingThread.h"
#include "C3DCommonEditorTypes.h" 



//forward declarations
class FDynamicData;
class FEditorSceneData;
class UDynamicObject;
class IImageWrapper;
class IPlatformFile;

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
	bool SceneHasSceneId(const FString& SceneName) const;

	bool HasSetExportDirectory() const;

	bool HasSetDynamicExportDirectory() const;

	int32 CountDynamicObjectsInScene() const;
	bool DuplicateDynamicIdsInScene() const;
	FText DynamicCountInScene;
	FText DisplayDynamicObjectsCountInScene() const;
	FText DisplayDynamicObjectsCountOnWeb() const;
	FText GetDynamicObjectUploadText(FString LevelName) const;

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

	FReply UploadScene(const FString& LevelName);

	TArray<AActor*> PrepareSceneForExport(bool OnlyExportSelected);
	
	bool CompressExportedFiles = false;
	void CompressTexturesInExportFolder(const FString& ExportFolder, int32 MaxSize);
	void CompressAndSaveTexture(const FString& SourcePath, const FString& DestinationPath, int32 MaxSize);

	bool ExportDynamicsWithScene = false;
	int32 CountUnexportedDynamics();
	int32 CountUnexportedDynamicsNotUnique();

	void UploadFromDirectory(FString LevelName, FString url, FString directory, FString expectedResponseType);

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
		FReply UploadDynamics(FString LevelName);
	UFUNCTION(Exec, Category = "Dynamics")
		FReply UploadDynamic(FString LevelName, FString directory);

	//this is for aggregating dynamic objects
	UFUNCTION(Exec, Category = "Dynamics Manifest")
		FReply UploadDynamicsManifest(FString LevelName);

	//this is for aggregating dynamic objects
	UFUNCTION(Exec, Category = "Dynamics Manifest")
		FReply UploadSelectedDynamicsManifest(FString LevelName, TArray<UDynamicObject*> selectedData);

	UFUNCTION(Exec, Category = "Dynamics Manifest")
		FReply UploadDynamicsManifestIds(FString LevelName, TArray<FString> ids, FString meshName, FString prefabName);

	UFUNCTION(Exec, Category = "Dynamics Manifest")
		FReply SetUniqueDynamicIds();

	FReply GetDynamicsManifest(FString LevelName);
	void OnDynamicManifestResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void OnUploadSceneCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString LevelName);
	void OnUploadObjectCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString LevelName);
	void OnUploadManifestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString LevelName);
	void OnUploadManifestIdsCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString LevelName);
	void OnUploadScreenshotCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	bool PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename);
	//bool PickFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename);
	void* ChooseParentWindowHandle();

	//default base export directory
	FString BaseExportDirectory;

	//c:/users/me/desktop/export/
	FText GetBaseExportDirectoryDisplay() const;
	FString GetBaseExportDirectory() const
	{
		return BaseExportDirectory;
	}
	void SetDefaultIfNoExportDirectory();
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
	void SceneNameVersionRequest(const FString& LevelName);

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

	//reads scene data from ini
	void ReadSceneDataFromFile();
		TArray<TSharedPtr<FEditorSceneData>> SceneData;
	//returns SceneData array
	TArray<TSharedPtr<FEditorSceneData>> GetSceneData() const;
	//returns data about a scene by name
	TSharedPtr<FEditorSceneData> GetSceneData(FString scenename) const;
	//returns data about a scene by the currently open scene
	TSharedPtr<FEditorSceneData> GetCurrentSceneData() const;


	//Third Party SDKs
	TArray<TSharedPtr<FString>> ThirdPartySDKData;
	TArray<TSharedPtr<FString>> GetThirdPartySDKData() const;
	void ReadThirdPartySDKData();

	FReply OpenSceneInBrowser(FString sceneid);
	FReply OpenCurrentSceneInBrowser();
	bool HasDeveloperKey() const;
	bool HasApplicationKey() const;

	FString GetSettingsFilePath() const;
	FString GetKeysFilePath() const;

	//send a http request to get the scene version data for current scene from sceneexplorer
	FReply ButtonCurrentSceneVersionRequest();
	
	//has json file and no bmp files in export directory
	bool HasConvertedFilesInDirectory() const;
	bool CanUploadSceneFiles() const;

	void SceneVersionRequest(const FEditorSceneData& data);
	void SceneVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString LevelName);

	TArray<TSharedPtr<FDynamicData>> SceneExplorerDynamics;
	TArray<TSharedPtr<FString>> SubDirectoryNames;

	void RefreshSceneUploadFiles(const FString& SceneName);
	int32 GetSceneExportFileCount();
	int32 GetDynamicObjectFileExportedCount();
	int32 GetDynamicObjectExportedCount();

	void RefreshDynamicUploadFiles();
	TArray<TSharedPtr<FString>> SceneUploadFiles;
	TArray<TSharedPtr<FString>> DynamicUploadFiles;
	
	TArray<FString> GetValidDirectories(const FString& Directory);
	int32 DynamicObjectExportedCount;

	FText UploadSceneNameFiles(FString LevelName) const;
	FText OpenSceneNameInBrowser(FString LevelName) const;
	FReply OpenURL(FString url);
	void FindAllSubDirectoryNames();
	TArray<TSharedPtr<FString>> GetSubDirectoryNames();
	FReply SaveScreenshotToFile(const FString& levelName);
	FReply TakeDynamicScreenshot(FString dynamicName);

	void DelayScreenshot(FString filePath, FLevelEditorViewportClient* perspectiveView, FVector startPos, FRotator startRot);
	
	FText GetDynamicsOnSceneExplorerTooltip(FString LevelName) const;
	FText SendDynamicsToSceneExplorerTooltip() const;
	FReply RefreshDynamicSubDirectory();
	bool ConfigFileHasChanged = false;
	EVisibility ConfigFileChangedVisibility() const;

	bool HasSetDynamicExportDirectoryHasSceneId(FString LevelName) const;
	FReply SaveAPIDeveloperKeysToFile();

	void SaveApplicationKeyToFile(FString key);
	void SaveDeveloperKeyToFile(FString key);

	void WizardUpload(const FString& LevelName);
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
	void ExportScene(FString LevelName, TArray<AActor*> actorsToExport);

	void ValidateGeneratedFiles(const FString LevelName);

	bool RenameFile(FString oldPath, FString newPath);

	void ModifyGLTFContent(FString FilePath);

	bool GenerateSettingsJsonFile(const FString& LevelName);
	bool HasSettingsJsonFile(const FString& LevelName) const;

	const FSlateBrush* GetBoxEmptyIcon() const;
	FSlateBrush* BoxEmptyIcon;
	const FSlateBrush* GetBoxCheckIcon() const;
	FSlateBrush* BoxCheckIcon;
	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");

	static const FSlateBrush* GetBrush(FName brushName);
	static const ISlateStyle& GetSlateStyle();

	//notifications
	void ShowNotification(FString Message, bool bSuccessful = true);

	bool bIsRestartEditorAfterSetup = false;
};

//used for uploading multiple dynamics at once
class FContentContainer
{
public:
	FString Headers;
	FString BodyText;
	TArray<uint8> BodyBinary;
};