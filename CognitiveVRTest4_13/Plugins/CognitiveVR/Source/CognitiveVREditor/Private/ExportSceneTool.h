#pragma once

#include "BaseEditorTool.h"
#include "Engine.h"
#include "Editor.h"
#include "SlateBasics.h"

#include "BusyCursor.h"
#include "Classes/Components/SceneComponent.h"
#include "EngineUtils.h"
#include "AssetTypeActions_Base.h"
#include "ObjectTools.h"
#include "PlatformProcess.h"
#include "DesktopPlatformModule.h"
#include "MainFrame.h"
#include "IPluginManager.h"
#include "AssetRegistryModule.h"

#include "ExportSceneTool.generated.h"

UCLASS(Blueprintable, Config=Editor, defaultconfig)
class UExportSceneTool : public UBaseEditorTool
{
	GENERATED_BODY()

public:
	UExportSceneTool();

public:
	//The threshold blender will decimate. Anything with a polygon count below this number will not be decimated
	UPROPERTY(EditAnywhere, Config, Category = "Settings")
	int32 MinPolygons = 500;
	
	//The upper threshold blender will decimate. Any mesh with a polygon count more than this number will be decimate to 10%
	UPROPERTY(EditAnywhere, Config, Category = "Settings")
	int32 MaxPolygons = 20000;
	
	//UPROPERTY(EditAnywhere, Category = "Settings")
	//FString Directory;
	
	//Only export StaticMeshComponents with set to be non-movable
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool staticOnly = true;

	//Ignore meshes with bounding size less than this value
	UPROPERTY(EditAnywhere, Category = "Settings")
	float MinimumSize;

	//Select blender.exe. Used to reduce polygon count of the exported scene
	UFUNCTION(Exec, Category = "Settings")
	void SelectBlender();

	//Select meshes that match settings - Above Minimum Size? Static?
	UFUNCTION(Exec, Category = "Settings")
	void SelectExportMeshes();

	//Runs the built-in obj exporter with the selected meshses
	UFUNCTION(Exec, Category = "Export")
	void ExportScene();

	//Runs a python script in blender to reduce the polygon count, clean up the mtl file and copy textures into a convient folder
	UFUNCTION(Exec, Category = "Export")
	void RunBlenderCleanup();
	
	//Runs a python script in blender to reduce the polygon count, clean up the mtl file and copy textures into a convient folder
	UFUNCTION(Exec, Category = "Export")
	void ConvertTextures();

	bool PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename);
	void* ChooseParentWindowHandle();

	FString BlenderPath;
	FString ExportDirectory;
};
