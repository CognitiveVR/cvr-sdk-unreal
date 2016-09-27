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
	UPROPERTY(EditAnywhere, Config, Category = "Settings")
	int32 MinPolygons = 500;
	
	UPROPERTY(EditAnywhere, Config, Category = "Settings")
	int32 MaxPolygons = 20000;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	FString Directory;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool staticOnly = true;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float MinimumSize;

	//UPROPERTY(EditAnywhere, Config, Category = "Settings")
	//FString DebugCmdPath;
	
	//UPROPERTY(Config, Category = "Settings")
	//FString DebugCmdParams;

	UFUNCTION(Exec, Category = "Settings")
	void SelectBlender();

	UFUNCTION(Exec, Category = "Settings")
	void SelectExportMeshes();

	UFUNCTION(Exec, Category = "Export")
	void ExportScene();

	UFUNCTION(Exec, Category = "Export")
	void RunBlenderCleanup();
	
	bool PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename);

	void* ChooseParentWindowHandle();

	//UPROPERTY(EditAnywhere, Config, Category = "Settings")
	FString BlenderPath;

	//UPROPERTY(EditAnywhere, Config, Category = "Settings")
	FString ExportDirectory;
};
