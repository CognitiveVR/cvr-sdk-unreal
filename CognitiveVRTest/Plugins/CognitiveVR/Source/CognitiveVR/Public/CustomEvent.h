/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "Private/CognitiveVRPrivatePCH.h"
#include "CognitiveVR.h"
#include "Private/util/util.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "Private/api/customeventrecorder.h"
#include "Private/api/sensor.h"
#include "CustomEvent.generated.h"

class FAnalyticsProviderCognitiveVR;

USTRUCT(BlueprintType)
struct FCustomEvent
{
	GENERATED_BODY()

private:
	double StartTime;

public:
	FString Category;
	FString DynamicId;
	FVector Position = FVector(0, 0, 0);
	TMap<FString, FString> StringProperties;
	TMap<FString, int32> IntegerProperties;
	TMap<FString, float> FloatProperties;
	TMap<FString, bool> BoolProperties;

	FCustomEvent();
	FCustomEvent(FString category);
	
	void Send();
	void Send(FVector position);
	void AppendSensors();
	void AppendSensor(FString sensorName);
	void AppendSensors(TArray<FString> sensorNames);
	
	void SetDynamicObject(FString dynamicObjectId);
	
	void SetProperty(FString key, FString value);
	void SetProperty(FString key, int32 value);
	void SetProperty(FString key, float value);
	void SetProperty(FString key, bool value);
};