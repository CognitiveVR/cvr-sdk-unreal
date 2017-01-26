// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CognitiveVR.h"
#include "AnalyticsSettings.h"

#include "Engine.h"
#include "Editor.h"
#include "SlateBasics.h"
#include "UnrealEd.h"

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
#include "Editor/UnrealEd/Public/FileHelpers.h"

//#include "ExportSceneTool.generated.h"

#include "CognitiveVRSettings.generated.h"

USTRUCT()
struct FSceneKeyPair
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		FString SceneName;
	UPROPERTY(EditAnywhere)
		FString SceneKey;

	FSceneKeyPair()
	{

	}

	FSceneKeyPair(const FString& InName, const FString& InKey)
	{
		SceneName = InName;
		SceneKey = InKey;
	}
};

UCLASS(config = Engine, defaultconfig)
class UCognitiveVRSettings
	: public UAnalyticsSettingsBase
{
	GENERATED_UCLASS_BODY()

	/** Display all info, warning and error messages from cognitiveVR. */
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
	bool EnableFullDebugLogging = true;

	/** Display only error messages from cognitiveVR. */
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
	bool EnableErrorDebugLogging;

	/** The unique identifier for your company and product. 'companyname1234-productname-test' */
	//UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
	//FString CustomerID;

	UPROPERTY(config, EditAnywhere, Category = "Scene Keys")
	TArray<FSceneKeyPair> SceneKeyPair;

	// UAnalyticsSettingsBase interface
protected:
	/**
	* Provides a mechanism to read the section based information into this UObject's properties
	*/
	virtual void ReadConfigSettings();
	/**
	* Provides a mechanism to save this object's properties to the section based ini values
	*/
	virtual void WriteConfigSettings();

public:
	//The threshold Blender will reduce. Anything with a polygon count below this number will not be reduced
	UPROPERTY(EditAnywhere, Config, Category = "Selection Settings")
		int32 MinPolygons = 500;

	//The upper threshold Blender will reduce. Any mesh with a polygon count more than this number will be reduced to 10%
	UPROPERTY(EditAnywhere, Config, Category = "Selection Settings")
		int32 MaxPolygons = 20000;

	//UPROPERTY(EditAnywhere, Category = "Settings")
	//FString Directory;

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
		void Select_Blender();

	//Select meshes that match settings - Above Minimum Size? Static?
	UFUNCTION(Exec, Category = "Export")
		void Select_Export_Meshes();

	//Runs the built-in obj exporter with the selected meshses
	UFUNCTION(Exec, Category = "Export")
		void Export_Selected();

	//Runs the built-in obj exporter with all meshses
	UFUNCTION(Exec, Category = "Export")
		void Export_All();

	//Runs a python script in blender to reduce the polygon count, clean up the mtl file and copy textures into a convient folder
	UFUNCTION(Exec, Category = "Export")
		void Reduce_Meshes();

	//Runs a python script in blender to reduce the polygon count, clean up the mtl file and copy textures into a convient folder
	UFUNCTION(Exec, Category = "Export")
		void Reduce_Textures();

	UFUNCTION(Exec, Category = "Export")
		void Http_Request();

	void OnYourFunctionCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	bool PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename);
	void* ChooseParentWindowHandle();

	//UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional, meta = (DisplayName = "BlenderPath", ShowForTools = "SceneExport"))
	UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional)
		FString BlenderPath;
	UPROPERTY(Category = "Scene Export Settings", EditAnywhere, NonTransactional)
		FString ExportDirectory;

	FString GetProductID();
};
