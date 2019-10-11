/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "CognitiveVR.h"
#include "CustomEvent.h"

TSharedPtr<FAnalyticsProviderCognitiveVR> FCustomEvent::cog;

FCustomEvent::FCustomEvent(FString category)
{
	Category = category;
	StartTime = cognitivevrapi::Util::GetTimestamp();
}

FCustomEvent::FCustomEvent()
{
	StartTime = cognitivevrapi::Util::GetTimestamp();
}

void FCustomEvent::Send()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}

	float duration = cognitivevrapi::Util::GetTimestamp() - StartTime;
	FloatProperties.Add("duration", duration);
	cog->customEventRecorder->Send(this);
}

void FCustomEvent::AppendSensors()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	auto sensorValues = cog->sensors->GetLastSensorValues();
	for (auto& Elem : sensorValues)
	{
		FloatProperties.Add(Elem.Key, Elem.Value);
	}
}
void FCustomEvent::AppendSensors(TArray<FString> sensorNames)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	auto sensorValues = cog->sensors->GetLastSensorValues();
	for (auto& Elem : sensorValues)
	{
		if (sensorNames.Contains(Elem.Key))
		{
			FloatProperties.Add(Elem.Key, Elem.Value);
		}
	}
}

void FCustomEvent::AppendSensor(FString sensorName)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	auto sensorValues = cog->sensors->GetLastSensorValues();
	
	if (sensorValues.Contains(sensorName))
		FloatProperties.Add(sensorName, sensorValues[sensorName]);
}

void FCustomEvent::SetDynamicObject(FString dynamicObjectId)
{
	DynamicId = dynamicObjectId;
}

void FCustomEvent::SetDynamicObject(UDynamicObject* dynamicObject)
{
	if (dynamicObject == NULL) { return; }
	DynamicId = dynamicObject->GetObjectId()->Id;
}

void FCustomEvent::SetPosition(FVector position)
{
	Position = position;
}

FString FCustomEvent::GetDynamicId()
{
	return DynamicId;
}

void FCustomEvent::SetProperty(FString key, FString value)
{
	StringProperties[key] = value;
}
void FCustomEvent::SetProperty(FString key, int32 value)
{
	IntegerProperties[key] = value;
}
void FCustomEvent::SetProperty(FString key, float value)
{
	FloatProperties[key] = value;
}
void FCustomEvent::SetProperty(FString key, bool value)
{
	BoolProperties[key] = value;
}