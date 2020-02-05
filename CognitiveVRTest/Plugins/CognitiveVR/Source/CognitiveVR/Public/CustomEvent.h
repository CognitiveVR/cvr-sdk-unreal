/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "CognitiveVR.h"
#include "Private/util/util.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "Private/api/customeventrecorder.h"
#include "Private/api/sensor.h"
#include "CustomEvent.generated.h"

class UDynamicObject;

USTRUCT(BlueprintType)
struct COGNITIVEVR_API FCustomEvent
{
	friend class CustomEventRecorder;
	friend class FAnalyticsProviderCognitiveVR;

	GENERATED_BODY()

private:
	double StartTime;
	FString Category;
	FString DynamicId;
	FVector Position = FVector(0, 0, 0);
	TMap<FString, FString> StringProperties;
	TMap<FString, int32> IntegerProperties;
	TMap<FString, float> FloatProperties;
	TMap<FString, bool> BoolProperties;

	static TSharedPtr<FAnalyticsProviderCognitiveVR> cog;

public:

	FCustomEvent();
	FCustomEvent(FString category);
	
	void Send();
	//appends the most recent value of all sensors as properties
	void AppendSensors();
	//appends the most recent value of a sensor by name as a property
	void AppendSensor(FString sensorName);
	//appends the most recent value of each sensor specified by name as a property
	void AppendSensors(TArray<FString> sensorNames);
	
	//set this custom event to be related to a dynamic object
	void SetDynamicObject(UDynamicObject* dynamicObject);
	//set this custom event to be related to a dynamic object by ObjectId
	void SetDynamicObject(FString dynamicObjectId);
	//set the position this event occurs at in world space
	void SetPosition(FVector position);
	//get the id that is currently associated with this event
	FString GetDynamicId();
	
	void SetProperty(FString key, FString value);
	void SetProperty(FString key, int32 value);
	void SetProperty(FString key, float value);
	void SetProperty(FString key, bool value);
};