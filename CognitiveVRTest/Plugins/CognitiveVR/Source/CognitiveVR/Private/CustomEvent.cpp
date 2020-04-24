/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#include "CustomEvent.h"

TSharedPtr<FAnalyticsProviderCognitiveVR> UCustomEvent::cog;

UCustomEvent::UCustomEvent()
{
	StartTime = Util::GetTimestamp();
}

void UCustomEvent::SetCategory(FString category)
{
	Category = category;
}

void UCustomEvent::Send()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	if (!cog.IsValid())
	{
		CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}

	float duration = Util::GetTimestamp() - StartTime;
	FloatProperties.Add("duration", duration);
	cog->customEventRecorder->Send(this);
}

void UCustomEvent::AppendSensors()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	
	if (!cog.IsValid())
	{
		CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	auto sensorValues = cog->sensors->GetLastSensorValues();
	for (auto& Elem : sensorValues)
	{
		FloatProperties.Add(Elem.Key, Elem.Value);
	}
}
void UCustomEvent::AppendSensors(TArray<FString> sensorNames)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	if (!cog.IsValid())
	{
		CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
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

void UCustomEvent::AppendSensor(FString sensorName)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	if (!cog.IsValid())
	{
		CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	auto sensorValues = cog->sensors->GetLastSensorValues();
	
	if (sensorValues.Contains(sensorName))
		FloatProperties.Add(sensorName, sensorValues[sensorName]);
}

void UCustomEvent::SetDynamicObject(FString dynamicObjectId)
{
	DynamicId = dynamicObjectId;
}

void UCustomEvent::SetDynamicObject(UDynamicObject* dynamicObject)
{
	if (dynamicObject == NULL) { return; }
	DynamicId = dynamicObject->GetObjectId()->Id;
}

void UCustomEvent::SetPosition(FVector position)
{
	Position = position;
}

FString UCustomEvent::GetDynamicId()
{
	return DynamicId;
}

void UCustomEvent::SetProperty(FString key, FString value)
{
	StringProperties.Add(key, value);
}
void UCustomEvent::SetProperty(FString key, int32 value)
{
	IntegerProperties.Add(key, value);
}
void UCustomEvent::SetProperty(FString key, float value)
{
	FloatProperties.Add(key, value);
}
void UCustomEvent::SetProperty(FString key, bool value)
{
	BoolProperties.Add(key, value);
}