// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CognitiveVR.h"
#include "AnalyticsSettings.h"
#include "CognitiveVRSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class UCognitiveVRSettings
	: public UAnalyticsSettingsBase
{
	GENERATED_UCLASS_BODY()

	/** Mute all info and warning messages from cognitiveVR. */
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
	bool MuteInfoMessages = false;

	/** Mute error messages from cognitiveVR. */
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
	bool MuteErrorMessages = false;

	/** The number of sensor data points that will be collected together before being sent to scene explorer */
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
	int32 SensorDataLimit = 64;

	/** The number of dynamic object snapshots that will be collected together before being sent to scene explorer */
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
		int32 DynamicDataLimit = 64;

	/** The number of transactions that will be collected together before being sent to analytics server and scene explorer*/
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
	int32 TransactionBatchSize = 64;

	/** The number of player snapshots that will be collected together before being sent to analytics server and scene explorer*/
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
		int32 GazeBatchSize = 64;

	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
	bool GazeFromPhysicsRaycast = false;

	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
	bool GazeFromVisualRaycast = true;

public:
	//Mesh names separated by ','. These will be removed in the Reduce Meshes step
	UPROPERTY(config, EditAnywhere, Category = "Export Settings")
		FString ExcludeMeshes = "VRPawn,SkySphere,Camera";

	//The threshold Blender will reduce. Anything with a polygon count below this number will not be reduced
	UPROPERTY(EditAnywhere, Config, Category = "Export Settings")
		int32 MinPolygons = 500;

	//The upper threshold Blender will reduce. Any mesh with a polygon count more than this number will be reduced to 10%
	UPROPERTY(EditAnywhere, Config, Category = "Export Settings")
		int32 MaxPolygons = 20000;

	//Only export StaticMeshComponents with set to be non-movable
	UPROPERTY(EditAnywhere, Category = "Export Settings")
		bool staticOnly = true;

	//Ignore meshes with bounding size less than this value
	UPROPERTY(EditAnywhere, Category = "Export Settings")
		float MinimumSize = 100;

	//Ignore meshes with bounding size larger than this value
	UPROPERTY(EditAnywhere, Category = "Export Settings")
		float MaximumSize = 10000;

	//Textures size is divided by this amount. MUST be a power of two greater than 0!
	UPROPERTY(EditAnywhere, Config, Category = "Export Settings")
		int32 TextureResizeFactor = 4;

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

};
