/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "CognitiveVR.h"
#include "CustomEvent.h"

//original cognitive provider in exitpoll header
//TSharedPtr<FAnalyticsProviderCognitiveVR> cogProvider2;

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
	auto cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

	if (!cogProvider.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}

	float duration = cognitivevrapi::Util::GetTimestamp() - StartTime;
	FloatProperties.Add("duration", duration);
	cogProvider->customeventrecorder->Send(this);
}
void FCustomEvent::Send(FVector position)
{
	Position = position;
	auto cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

	if (!cogProvider.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	float duration = cognitivevrapi::Util::GetTimestamp() - StartTime;
	FloatProperties.Add("duration", duration);
	cogProvider->customeventrecorder->Send(this);
}
void FCustomEvent::AppendSensors()
{
	auto cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	
	if (!cogProvider.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	auto sensorValues = cogProvider->sensors->GetLastSensorValues();
	for (auto& Elem : sensorValues)
	{
		FloatProperties.Add(Elem.Key, Elem.Value);
	}
}
void FCustomEvent::AppendSensors(TArray<FString> sensorNames)
{
	auto cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

	if (!cogProvider.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	auto sensorValues = cogProvider->sensors->GetLastSensorValues();
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
	auto cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

	if (!cogProvider.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	auto sensorValues = cogProvider->sensors->GetLastSensorValues();
	
	if (sensorValues.Contains(sensorName))
		FloatProperties.Add(sensorName, sensorValues[sensorName]);
}

void FCustomEvent::SetDynamicObject(FString dynamicObjectId)
{
	DynamicId = dynamicObjectId;
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