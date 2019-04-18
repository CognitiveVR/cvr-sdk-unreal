// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

//#include "CognitiveVREditorPrivatePCH.h"
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

	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
		FString Gateway = "data.cognitive3d.com";

	UPROPERTY(config, EditAnywhere, Category = CognitiveVR)
		FString SessionViewer = "sceneexplorer.com/scene/";

	/** The number of player snapshots that will be collected together before being sent to analytics server and scene explorer*/
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 GazeBatchSize = 64;

	/** The number of Custom Events that will be collected together before being sent to analytics server and scene explorer*/
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 CustomEventBatchSize = 64;
	//Threshold for ignoring the Event Minimum Timer. If this many Events have been recorded, immediately send
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 CustomEventExtremeLimit = 128;
	//Time (in seconds) that must be elapsed before sending a new batch of Event data. Ignored if the batch size reaches Event Extreme Limit
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 CustomEventMinTimer = 2;
	//The time (in seconds) to automatically send any outstanding Event data
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 CustomEventAutoTimer = 30;

	/** The number of dynamic object snapshots that will be collected together before being sent to scene explorer */
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 DynamicDataLimit = 64;
	//Threshold for ignoring the Dynamic Minimum Timer. If this many Dynamic snapshots have been recorded, immediately send
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 DynamicExtremeLimit = 128;
	//Time (in seconds) that must be elapsed before sending a new batch of Dynamic data. Ignored if the batch size reaches Dynamic Extreme Limit
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 DynamicMinTimer = 2;
	//The time (in seconds) to automatically send any outstanding Dynamic snapshots or Manifest entries
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 DynamicAutoTimer = 30;

	/** The number of sensor data points that will be collected together before being sent to scene explorer */
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 SensorDataLimit = 64;
	//Threshold for ignoring the Sensor Minimum Timer. If this many Sensor data points have been recorded, immediately send
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 SensorExtremeLimit = 128;
	//Time (in seconds) that must be elapsed before sending a new batch of Sensor data. Ignored if the batch size reaches Sensor Extreme Limit
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 SensorMinTimer = 2;
	//The time (in seconds) to automatically send any outstanding Sensor data
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 SensorAutoTimer = 30;

	/** The number of fixations that will be collected together before being sent to analytics server and scene explorer*/
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 FixationBatchSize = 64;
	//Threshold for ignoring the Event Minimum Timer. If this many Fixations have been recorded, immediately send
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 FixationExtremeLimit = 128;
	//Time (in seconds) that must be elapsed before sending a new batch of Fixation data. Ignored if the batch size reaches Event Extreme Limit
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 FixationMinTimer = 2;
	//The time (in seconds) to automatically send any outstanding Fixations
	UPROPERTY(config, EditAnywhere, Category = Cognitive_VR_Data)
		int32 FixationAutoTimer = 30;

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
