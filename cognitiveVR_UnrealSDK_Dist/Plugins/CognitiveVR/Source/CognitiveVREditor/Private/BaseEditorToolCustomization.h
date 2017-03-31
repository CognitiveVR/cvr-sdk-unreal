#pragma once

#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveVRSettings.h"
#include "SlateBasics.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
//#include "DetailCustomizationsPrivatePCH.h"
#include "PropertyCustomizationHelpers.h"

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
//
//#include "ExportSceneTool.generated.h"

class UCognitiveVRSettings;

class FBaseEditorToolCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> MakeInstance();

	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute);

private:
	bool HasSearchedForBlender = false; //to limit the searching directories. possibly not required

	void SearchForBlender();
	bool HasFoundBlender() const;
	bool HasFoundBlenderAndExportDir() const;
	bool HasSetExportDirectory() const;

	FText GetBlenderPath() const;

	UCognitiveVRSettings *Settings;
	IDetailLayoutBuilder *DetailLayoutPtr;

	float GetMinimumSize();
	float GetMaximumSize();
	int GetMinPolygon();
	int GetMaxPolygon();
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

	UFUNCTION(Exec, Category = "Export")
		FReply List_Materials();

	void OnUploadSceneCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void OnYourFunctionCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	bool PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename);
	bool PickFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename);
	void* ChooseParentWindowHandle();

	//UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional, meta = (DisplayName = "BlenderPath", ShowForTools = "SceneExport"))
	UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional)
		FString BlenderPath;
	UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional)
		FString ExportDirectory;
	FText GetExportDirectory() const;

	UFUNCTION(Exec, Category = "Export")
		FReply Select_Export_Directory();

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
};