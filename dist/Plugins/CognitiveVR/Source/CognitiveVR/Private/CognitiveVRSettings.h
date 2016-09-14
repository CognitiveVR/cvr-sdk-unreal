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

	/** Display all info, warning and error messages from cognitiveVR. */
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR, meta = (ConfigRestartRequired = true))
	bool EnableFullDebugLogging;

	/** Display only error messages from cognitiveVR. */
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR, meta = (ConfigRestartRequired = true))
	bool EnableErrorDebugLogging;

	/** The unique identifier for your company and product. 'companyname1234-productname-test' */
	UPROPERTY(config, EditAnywhere, Category = CognitiveVR, meta = (ConfigRestartRequired = true))
	FString CustomerID;

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
