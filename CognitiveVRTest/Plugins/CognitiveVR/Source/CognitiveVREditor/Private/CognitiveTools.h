#pragma once

#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveVRSettings.h"
#include "SlateBasics.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
//#include "DetailCustomizationsPrivatePCH.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"
#include "SCheckBox.h"
#include "STableRow.h"

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
#include "STextComboBox.h"
//
//#include "ExportSceneTool.generated.h"

//https://forums.unrealengine.com/unreal-engine/marketplace/125106-configbp-ini-configuration-files-the-easy-way?p=1385756#post1385756

class UCognitiveVRSettings;

class FOrganizationData
{
public:
	FString id = "";
	FString name = "";
	FString prefix = "";
};

class FProductData
{
public:
	FString id = "";
	FString name = "";
	FString orgId = "";
	FString customerId = "";
};

class FEditorSceneData
{
public:
	FString Name = "";
	FString Id = "";
	int32 VersionNumber = 1;
	int32 VersionId = 0;

	FEditorSceneData(FString name, FString id, int32 versionnumber, int32 versionid)
	{
		Name = name;
		Id = id;
		VersionNumber = versionnumber;
		VersionId = versionid;
	}
};

class FCognitiveTools : public IDetailCustomization
{

	// Enumerates radio button choices.
	enum EReleaseType
	{
		Test,
		Production
	};

	EReleaseType RadioChoice = EReleaseType::Test;

public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> MakeInstance();

	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute);

	void SaveSceneData(FString sceneName, FString sceneKey);

private:


	//GET dynamic object manifest                           https ://api.sceneexplorer.com/versions/:versionId/objects
	FORCEINLINE static FString GetDynamicObjectManifest(FString versionid)
	{
		return "https://api.sceneexplorer.com/versions/" + versionid + "/objects";
	}

	//POST dynamic object manifest                          https://data.sceneexplorer.com/objects/:sceneId?version=:versionNumber
	FORCEINLINE static FString PostDynamicObjectManifest(FString sceneid, int32 versionnumber)
	{
		return "https://data.sceneexplorer.com/objects/" + sceneid + "?version=" + FString::FromInt(versionnumber);
	}

	//POST dynamic object mesh data							https://data.sceneexplorer.com/objects/:sceneId/:exportDirectory?version=:versionNumber
	FORCEINLINE static FString PostDynamicObjectMeshData(FString sceneid, int32 versionnumber, FString exportdirectory)
	{
		return "https://data.sceneexplorer.com/objects/" + sceneid + "/" + exportdirectory + "?version=" + FString::FromInt(versionnumber);
	}

	//GET scene settings and read scene version             https://api.sceneexplorer.com/scenes/:sceneId
	FORCEINLINE static FString GetSceneVersion(FString sceneid)
	{
		return "https://api.sceneexplorer.com/scenes/" + sceneid;
	}

	//POST scene screenshot                                 https://data.sceneexplorer.com/scenes/:sceneId/screenshot?version=:versionNumber
	FORCEINLINE static FString PostScreenshot(FString sceneid, FString versionnumber)
	{
		return "https://data.sceneexplorer.com/scenes/" + sceneid + "/screenshot?version=" + versionnumber;
	}

	//POST upload decimated scene                           https://data.sceneexplorer.com/scenes
	FORCEINLINE static FString PostNewScene()
	{
		return "https://data.sceneexplorer.com/scenes";
	}

	//POST upload and replace existing scene                https://data.sceneexplorer.com/scenes/:sceneId
	FORCEINLINE static FString PostUpdateScene(FString sceneid)
	{
		return "https://data.sceneexplorer.com/scenes/" + sceneid;
	}

	//POST auth token from dynamic object manifest response https://api.sceneexplorer.com/tokens/:sceneId
	FORCEINLINE static FString PostAuthToken(FString sceneid)
	{
		return "https://api.sceneexplorer.com/tokens/" + sceneid;
	}

	//WEB used to open scenes on sceneexplorer              https://sceneexplorer.com/scene/ :sceneId
	FORCEINLINE static FString SceneExplorerOpen(FString sceneid)
	{
		return "https://sceneexplorer.com/scene/" + sceneid;
	}

	//POST used to log into the editor						https://api.cognitivevr.io/sessions
	FORCEINLINE static FString APISessions()
	{
		return "http://api.cognitivevr.io/sessions";
	}

	//WEB opens dashboard page to create a new product
	FORCEINLINE static FString DashboardNewProduct()
	{
		return "https://dashboard.cognitivevr.io/admin/products/create";
	}


	bool HasSearchedForBlender = false; //to limit the searching directories. possibly not required

	void SearchForBlender();
	bool HasFoundBlender() const;
	bool HasFoundBlenderAndExportDir() const;
	bool HasSetExportDirectory() const;
	bool HasFoundBlenderAndDynamicExportDir() const;
	bool HasSetDynamicExportDirectory() const;


	FText GetBlenderPath() const;

	UCognitiveVRSettings *Settings;
	IDetailLayoutBuilder *DetailLayoutPtr;

	float GetMinimumSize();
	float GetMaximumSize();
	int32 GetMinPolygon();
	int32 GetMaxPolygon();
	int32 GetTextureRefacor();
	bool GetStaticOnly();

	TSharedPtr<IPropertyHandle> MinSizeProperty;
	TSharedPtr<IPropertyHandle> MaxSizeProperty;
	TSharedPtr<IPropertyHandle> MinPolygonProperty;
	TSharedPtr<IPropertyHandle> MaxPolygonProperty;
	TSharedPtr<IPropertyHandle> StaticOnlyProperty;
	TSharedPtr<IPropertyHandle> TextureResizeProperty;
	TSharedPtr<IPropertyHandle> SceneKeysProperty;

	//Select Blender.exe. Used to reduce polygon count of the exported scene
	UFUNCTION(Exec, Category = "Export")
		FReply Select_Blender();

	//Select meshes that match settings - Above Minimum Size? Static?
	UFUNCTION(Exec, Category = "Export")
		FReply Select_Export_Meshes();

	//Runs the built-in obj exporter with the selected meshses
	UFUNCTION(Exec, Category = "Export")
		FReply Export_Selected();

	//Runs the built-in obj exporter with all meshses
	UFUNCTION(Exec, Category = "Export")
		FReply Export_All();

	//Runs a python script in blender to reduce the polygon count, clean up the mtl file and copy textures into a convient folder
	UFUNCTION(Exec, Category = "Export")
		FReply Reduce_Meshes();

	//Runs a python script in blender to reduce the polygon count, clean up the mtl file and copy textures into a convient folder
	UFUNCTION(Exec, Category = "Export")
		FReply Reduce_Textures();

	UFUNCTION(Exec, Category = "Export")
		FReply Http_Request();

	UFUNCTION(Exec, Category = "Export")
		FReply UploadScene();

	void UploadMultipartData(FString url, TArray<FString> files, TArray<FString> images);
	void UploadFromDirectory(FString url, FString directory, FString expectedResponseType);

	UFUNCTION(Exec, Category = "Export")
		FReply List_Materials();
	void List_MaterialArgs(FString subdirectory,FString searchDirectory);
	void ReexportDynamicMeshes(FString directory);

	UFUNCTION(Exec, Category = "Export")
	FReply ReexportDynamicMeshesCmd();

	//dynamic objects
	//Runs the built-in obj exporter with all meshses
	UFUNCTION(Exec, Category = "Dynamics")
		FReply ExportDynamics();

	UFUNCTION(Exec, Category = "Dynamics")
		FReply ExportDynamicTextures();

	//Runs the built-in obj exporter with selected meshes
	UFUNCTION(Exec, Category = "Dynamics")
		FReply ExportSelectedDynamics();
	
	void ExportDynamicObjectArray(TArray<UDynamicObject*> exportObjects);

	//uploads each dynamic object using its directory to the current scene
	UFUNCTION(Exec, Category = "Dynamics")
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

	UFUNCTION(Exec, Category = "Export")
		FReply DebugSendSceneData();



	TArray<FString> GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension,const FString ignoreExtension);

	FString GetProductID();

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

	FReply SetRandomSessionId();
	FReply PrintSessionId();
	FString Email;
	void OnEmailChanged(const FText& Text);
	FString Password;
	void OnPasswordChanged(const FText& Text);

	TSharedPtr<FJsonObject> JsonUserData;

	FReply DEBUG_RequestAuthToken();
	void AuthTokenRequest();
	void AuthTokenResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	//TArray<TSharedPtr<FString>> OrganizationNames;
	TArray<FOrganizationData> OrganizationInfos;
	void OnOrganizationChanged(TSharedPtr<FString> Selection, ESelectInfo::Type SelectInfo);

	TArray<FProductData> ProductInfos;
	void OnProductChanged(TSharedPtr<FString> Selection, ESelectInfo::Type SelectInfo);

	//TSharedPtr<FString> SelectedOrgName;
	//TSharedPtr<FString> GetSelectedOrganizationName();

	//all products for the currently selected organization
	TArray<TSharedPtr<FString>> AllProductNames;
	TSharedPtr<FString> SelectedProductName;

	FProductData SelectedProduct;

	TArray<TSharedPtr<FString>> AllOrgNames;
	TArray<TSharedPtr<FString>> GetOrganizationNames();

	ECheckBoxState FCognitiveTools::HandleRadioButtonIsChecked(EReleaseType ButtonId) const;
	void FCognitiveTools::HandleRadioButtonCheckStateChanged(ECheckBoxState NewRadioState, EReleaseType RadioThatChanged);

	TArray<TSharedPtr<FEditorSceneData>> SceneData;
	TArray<TSharedPtr<FEditorSceneData>> GetSceneData() const;

	TSharedRef<ITableRow> OnGenerateWorkspaceRow(TSharedPtr<FEditorSceneData> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	FReply SaveCustomerIdToFile();

	FString GetCustomerIdFromFile() const;
	EReleaseType GetReleaseTypeFromFile();
	
	void SaveOrganizationNameToFile(FString organization);
	TSharedPtr<FString> GetOrganizationNameFromFile();
	void SaveProductNameToFile(FString product);
	TSharedPtr< FString > GetProductNameFromFile();

	FReply OpenSceneInBrowser(FString sceneid);

	bool HasSelectedValidProduct() const;
	bool HasLoggedIn() const;

	//reads scene data from ini
	FReply RefreshSceneData();
	FReply DebugRefreshCurrentScene();
	
	TSharedPtr<FEditorSceneData> GetSceneData(FString scenename);
	TSharedPtr<FEditorSceneData> GetCurrentSceneData();

	void SceneVersionRequest(FEditorSceneData data);
	void SceneVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FReply LogIn();
	void LogInResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};

//used for uploading multiple dynamics at once
class FContentContainer
{
public:
	FString Headers;
	FString BodyText;
	TArray<uint8> BodyBinary;
};