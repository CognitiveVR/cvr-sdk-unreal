// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CognitiveVRSettings.generated.h"


/**
 * Implements the settings for the Slate Remote plug-in.
 */
UCLASS(config=Engine)
class UCognitiveVRSettings
	: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/** Display all info, warning and error messages from cognitiveVR. */
	UPROPERTY(config, EditAnywhere, Category=General)
	bool EnableFullDebugLogging;

	/** Display only error messages from cognitiveVR. */
	UPROPERTY(config, EditAnywhere, Category = General)
	bool EnableErrorDebugLogging;

	/** The unique identifier for your company and product. 'companyname1234-productname-test' */
	UPROPERTY(config, EditAnywhere, Category= General)
	FString CustomerID;

	/** The IP endpoint to listen to when the Remote Server runs in a game. */
	//UPROPERTY(config, EditAnywhere, Category= General)
	//FString GameServerEndpoint;
};
