/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Public/Cognitive3D.h"
#include "AnalyticsSettings.h"
#include "Cognitive3DSettings.generated.h"

UCLASS()
class COGNITIVE3D_API UCognitive3DSettings : public UAnalyticsSettingsBase
{
	GENERATED_UCLASS_BODY()

	/** Show all info and warning messages from Cognitive3D. */
	UPROPERTY(EditAnywhere, Category = Cognitive3D)
		bool EnableLogging = true;

	/** Show extra developer specific messages */
	UPROPERTY(EditAnywhere, Category = Cognitive3D)
		bool EnableDevLogging = false;

	/** Save recorded data to a file if no internet connection exists */
	UPROPERTY(EditAnywhere, Category = Cognitive3D)
		bool EnableLocalCache = true;
	/** How much space (in MB) to allocate to the local cache */
	UPROPERTY(EditAnywhere, Category = Cognitive3D)
		int32 LocalCacheSize = 100;

	UPROPERTY(EditAnywhere, Category = Cognitive3D)
		FString Gateway = "data.cognitive3d.com";

	UPROPERTY(EditAnywhere, Category = Cognitive3D)
		bool AutomaticallySetTrackingScene = true;

	/** The number of player snapshots that will be collected together before being sent to analytics server and scene explorer*/
	UPROPERTY(EditAnywhere, Category = "Cognitive_3D_Data")
		int32 GazeBatchSize = 256;

	/** The number of Custom Events that will be collected together before being sent to analytics server and scene explorer*/
	UPROPERTY(EditAnywhere, Category = "Cognitive_3D_Data")
		int32 CustomEventBatchSize = 256;
	//The time (in seconds) to automatically send any outstanding Event data
	UPROPERTY(EditAnywhere, Category = "Cognitive_3D_Data")
		int32 CustomEventAutoTimer = 10;

	/** The number of dynamic object snapshots that will be collected together before being sent to scene explorer */
	UPROPERTY(EditAnywhere, Category = "Cognitive_3D_Data")
		int32 DynamicDataLimit = 512;
	//The time (in seconds) to automatically send any outstanding Dynamic snapshots or Manifest entries
	UPROPERTY(EditAnywhere, Category = "Cognitive_3D_Data")
		int32 DynamicAutoTimer = 10;

	/** The number of sensor data points that will be collected together before being sent to scene explorer */
	UPROPERTY(EditAnywhere, Category = "Cognitive_3D_Data")
		int32 SensorDataLimit = 512;
	//The time (in seconds) to automatically send any outstanding Sensor data
	UPROPERTY(EditAnywhere, Category = "Cognitive_3D_Data")
		int32 SensorAutoTimer = 10;

	/** The number of fixations that will be collected together before being sent to analytics server and scene explorer*/
	UPROPERTY(EditAnywhere, Category = "Cognitive_3D_Data")
		int32 FixationBatchSize = 256;
	//The time (in seconds) to automatically send any outstanding Fixations
	UPROPERTY(EditAnywhere, Category = "Cognitive_3D_Data")
		int32 FixationAutoTimer = 10;


		FString GetIniName() const;

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
