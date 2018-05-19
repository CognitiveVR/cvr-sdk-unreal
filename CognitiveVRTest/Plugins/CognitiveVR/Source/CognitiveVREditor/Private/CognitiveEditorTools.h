#pragma once

#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveVRSettings.h"
#include "CognitiveEditorData.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
//#include "DetailCustomizationsPrivatePCH.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"

#include "UnrealEd.h"
#include "Engine.h"
#include "Editor.h"
#include "FileHelpers.h"
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
#include "MaterialUtilities.h"
#include "DynamicObject.h"
#include "GenericPlatformFile.h"
#include "Http.h"
#include "UnrealClient.h"

//all sorts of functionality for Cognitive SDK


class FCognitiveEditorTools
{
public:

	static void Initialize();

	static FCognitiveEditorTools* GetInstance();

	void Tick(float deltatime);

	void SaveSceneData(FString sceneName, FString sceneKey);

	//gets all the dynamics in the scene and saves them to SceneDynamics
	TArray<TSharedPtr<FDynamicData>> SceneDynamics;
	TArray<TSharedPtr<FDynamicData>> GetSceneDynamics();

	void OnAPIKeyChanged(const FText& Text);
	void OnDeveloperKeyChanged(const FText& Text);

	FText GetAPIKey() const;
	FText GetDeveloperKey() const;

	FString APIKey;
	//FString DeveloperKey;

	//GET dynamic object manifest                           https ://api.sceneexplorer.com/versions/:versionId/objects
	FORCEINLINE static FString GetDynamicObjectManifest(FString versionid)
	{
		return "https://data.cognitive3d.com/v0/versions/" + versionid + "/objects";
	}

	//POST dynamic object manifest                          https://data.sceneexplorer.com/objects/:sceneId?version=:versionNumber
	FORCEINLINE static FString PostDynamicObjectManifest(FString sceneid, int32 versionnumber)
	{
		return "https://data.cognitive3d.com/v0/objects/" + sceneid + "?version=" + FString::FromInt(versionnumber);
	}

	//POST dynamic object mesh data							https://data.sceneexplorer.com/objects/:sceneId/:exportDirectory?version=:versionNumber
	FORCEINLINE static FString PostDynamicObjectMeshData(FString sceneid, int32 versionnumber, FString exportdirectory)
	{
		return "https://data.cognitive3d.com/v0/objects/" + sceneid + "/" + exportdirectory + "?version=" + FString::FromInt(versionnumber);
	}

	//GET scene settings and read scene version             https://api.sceneexplorer.com/scenes/:sceneId
	FORCEINLINE static FString GetSceneVersion(FString sceneid)
	{
		return "https://data.cognitive3d.com/v0/scenes/" + sceneid;
	}

	//POST scene screenshot                                 https://data.sceneexplorer.com/scenes/:sceneId/screenshot?version=:versionNumber
	FORCEINLINE static FString PostScreenshot(FString sceneid, FString versionnumber)
	{
		return "https://data.cognitive3d.com/v0/scenes/" + sceneid + "/screenshot?version=" + versionnumber;
	}

	//POST upload decimated scene                           https://data.sceneexplorer.com/scenes
	FORCEINLINE static FString PostNewScene()
	{
		return "https://data.cognitive3d.com/v0/scenes";
	}

	//POST upload and replace existing scene                https://data.sceneexplorer.com/scenes/:sceneId
	FORCEINLINE static FString PostUpdateScene(FString sceneid)
	{
		return "https://data.cognitive3d.com/v0/scenes/" + sceneid;
	}

	//POST auth token from dynamic object manifest response https://api.sceneexplorer.com/tokens/:sceneId
	FORCEINLINE static FString PostAuthToken(FString sceneid)
	{
		return "";
	}

	//WEB used to open scenes on sceneexplorer              https://sceneexplorer.com/scene/ :sceneId
	FORCEINLINE static FString SceneExplorerOpen(FString sceneid)
	{
		return "https://sceneexplorer.com/scene/" + sceneid;
	}

	//POST used to log into the editor						https://api.cognitivevr.io/sessions
	FORCEINLINE static FString APISessions()
	{
		return "";
	}

	//WEB opens dashboard page to create a new product
	FORCEINLINE static FString DashboardNewProduct()
	{
		return "https://dashboard.cognitivevr.io/admin/products/create";
	}

	//WEB open dashboard page to product
	FORCEINLINE static FString DashboardNewProduct(FString customerid)
	{
		return "https://dashboard.cognitivevr.io/dashboard?pid=" + customerid;
	}


	bool HasSearchedForBlender = false; //to limit the searching directories. possibly not required

	void SearchForBlender();
	bool HasFoundBlender() const;
	bool HasFoundBlenderAndHasSelection() const;
	bool CurrentSceneHasSceneId() const;

	bool HasFoundBlenderAndExportDir() const;
	bool HasSetExportDirectory() const;
	bool HasFoundBlenderAndDynamicExportDir() const;
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
	//FReply RefreshUploadDynamicsToSceneText();
	void RefreshUploadDynamicsToSceneText();
	//TSharedPtr<SVerticalBox> SetDynamicBoxContent();

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

	FText GetBlenderPath() const;

	//Select Blender.exe. Used to reduce polygon count of the exported scene
		FReply Select_Blender();

	//Select meshes that match settings - Above Minimum Size? Static?
		FReply Select_Export_Meshes();

	//Runs the built-in obj exporter with the selected meshses
		FReply Export_Selected();

	//Runs the built-in obj exporter with all meshses
		FReply Export_All();

	//Runs a python script in blender to reduce the polygon count, clean up the mtl file and copy textures into a convient folder
		FReply Reduce_Meshes();

	//Runs a python script in blender to reduce the polygon count, clean up the mtl file and copy textures into a convient folder
		FReply Reduce_Textures();

		FReply UploadScene();

	void WizardExport(bool all);
	FProcHandle Reduce_Meshes_And_Textures();

	//void UploadMultipartData(FString url, TArray<FString> files, TArray<FString> images);
	void UploadFromDirectory(FString url, FString directory, FString expectedResponseType);

		FReply List_Materials();
	void List_MaterialArgs(FString subdirectory,FString searchDirectory);
	void ReexportDynamicMeshes(FString directory);

	FReply ReexportDynamicMeshesCmd();

	//dynamic objects
	//Runs the built-in obj exporter with all meshses
		FReply ExportDynamics();

		FReply ExportDynamicTextures();

	//Runs the built-in obj exporter with selected meshes
		FReply ExportSelectedDynamics();
	
	void ExportDynamicObjectArray(TArray<UDynamicObject*> exportObjects);

	//uploads each dynamic object using its directory to the current scene
		FReply SelectDynamicsDirectory();

	void ConvertDynamicTextures();

	//uploads each dynamic object using its directory to the current scene
	UFUNCTION(Exec, Category = "Dynamics")
		FReply UploadDynamics();

	//this is for aggregating dynamic objects
	UFUNCTION(Exec, Category = "Dynamics Manifest")
		FReply UploadDynamicsManifest();

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

	//UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional, meta = (DisplayName = "BlenderPath", ShowForTools = "SceneExport"))
	UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional)
		FString BlenderPath;
	UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional)
		FString ExportDirectory;
	UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional)
		FString ExportDynamicsDirectory;
	FText GetExportDirectory() const;
	FText GetDynamicExportDirectory() const;

	UFUNCTION(Exec, Category = "Export")
		FReply Select_Export_Directory();

	TArray<FString> GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension, const FString ignoreExtension) const;

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

	//If this function cannot find or create the directory, returns false.
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


	TArray<TSharedPtr<FEditorSceneData>> SceneData;
	//returns SceneData array
	TArray<TSharedPtr<FEditorSceneData>> GetSceneData() const;

	
	void SaveOrganizationNameToFile(FString organization);
	TSharedPtr<FString> GetOrganizationNameFromFile();
	void SaveProductNameToFile(FString product);
	TSharedPtr< FString > GetProductNameFromFile();

	FReply OpenSceneInBrowser(FString sceneid);
	FReply OpenCurrentSceneInBrowser();

	bool HasSelectedValidProduct() const;
	bool HasLoadedOrSelectedValidProduct() const;
	//bool HasLoggedIn() const;
	bool HasDeveloperKey() const;
	bool HasAPIKey() const;
	EVisibility GetLoginButtonState() const;
	EVisibility GetLogoutButtonState() const;

	//EVisibility ExportSettingsVisibility() const;
	//EVisibility OptimizeSettingsVisibility() const;
	
	//returns true if email + password fields lengths both greater than 0
	//bool HasValidLogInFields() const;

	//reads scene data from ini
	FReply RefreshSceneData();
	FReply DebugRefreshCurrentScene();
	
	//returns data about a scene by name
	TSharedPtr<FEditorSceneData> GetSceneData(FString scenename) const;
	//returns data about a scene by the currently open scene
	TSharedPtr<FEditorSceneData> GetCurrentSceneData() const;

	//has json file and no bmp files in export directory
	bool HasConvertedFilesInDirectory() const;
	bool CanUploadSceneFiles() const;
	//returns true if customerid has been saved
	//bool HasSavedCustomerId() const;
	bool CustomerIdDoesntMatchFile() const;
	bool LoginAndCustonerIdAndBlenderExportDir() const;
	//FText GetCustomerId() const;

	ECheckBoxState HasFoundBlenderCheckbox() const;

	void SceneVersionRequest(FEditorSceneData data);
	void SceneVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FText GetDynamicsFromManifest() const;
	TArray<TSharedPtr<FDynamicData>> SceneExplorerDynamics;
	TArray<TSharedPtr<FString>> SubDirectoryNames;

	FString GetProductID();

	void RefreshAllUploadFiles();
	TArray<TSharedPtr<FString>> AllUploadFiles;

	FText UploadSceneNameFiles() const;
	FText OpenSceneNameInBrowser() const;
	void FindAllSubDirectoryNames();
	TArray<TSharedPtr<FString>> GetSubDirectoryNames();
	FReply SelectUploadScreenshot();
	FReply TakeScreenshot();
	
	//bool HasEditorAuthToken() const;
	FText GetDynamicsOnSceneExplorerTooltip() const;
	FText SendDynamicsToSceneExplorerTooltip() const;
	FReply RefreshDynamicSubDirectory();
	bool ConfigFileHasChanged = false;
	EVisibility ConfigFileChangedVisibility() const;

	//FReply OpenProductOnDashboard();
	//bool EnableOpenProductOnDashboard() const;

	bool HasFoundBlenderHasSelection() const;
	bool HasSetDynamicExportDirectoryHasSceneId() const;
	FReply SaveAPIDeveloperKeysToFile();

	void SaveAPIKeyToFile(FString key);
	void SaveDeveloperKeyToFile(FString key);

	void WizardUpload();
	bool IsWizardUploading();
};

//used for uploading multiple dynamics at once
class FContentContainer
{
public:
	FString Headers;
	FString BodyText;
	TArray<uint8> BodyBinary;
};