// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CognitiveVR.h"
#include "AnalyticsSettings.h"
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
	bool EnableFullDebugLogging;

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
};
