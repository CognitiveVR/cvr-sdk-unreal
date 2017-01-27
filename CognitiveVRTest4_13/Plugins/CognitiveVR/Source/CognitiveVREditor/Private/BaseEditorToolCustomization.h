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
//
//#include "ExportSceneTool.generated.h"


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

								  //The threshold Blender will reduce. Anything with a polygon count below this number will not be reduced
	UPROPERTY(EditAnywhere, Config, Category = "Selection Settings")
		int32 MinPolygons = 500;

	//The upper threshold Blender will reduce. Any mesh with a polygon count more than this number will be reduced to 10%
	UPROPERTY(EditAnywhere, Config, Category = "Selection Settings")
		int32 MaxPolygons = 20000;

	//Only export StaticMeshComponents with set to be non-movable
	UPROPERTY(EditAnywhere, Category = "Selection Settings")
		bool staticOnly = true;

	//Ignore meshes with bounding size less than this value
	UPROPERTY(EditAnywhere, Category = "Selection Settings")
		float MinimumSize = 100;

	//Ignore meshes with bounding size larger than this value
	UPROPERTY(EditAnywhere, Category = "Selection Settings")
		float MaximumSize = 10000;

	//Textures size is divided by this amount. MUST be a power of two greater than 0!
	UPROPERTY(EditAnywhere, Config, Category = "Texture Settings")
		int32 TextureResizeFactor = 4;

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