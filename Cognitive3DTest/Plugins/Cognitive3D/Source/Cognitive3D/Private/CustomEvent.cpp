/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "CustomEvent.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "Cognitive3D/Private/C3DUtil/CognitiveLog.h"

TSharedPtr<FAnalyticsProviderCognitive3D> UCustomEvent::cog;

UCustomEvent::UCustomEvent()
{
	StartTime = FUtil::GetTimestamp();
}

void UCustomEvent::SetCategory(FString category)
{
	Category = category;
}

void UCustomEvent::Send()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();

	if (!cog.IsValid())
	{
		FCognitiveLog::Error("FExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	if (!cog->HasStartedSession())
	{
		FCognitiveLog::Error("UCustomEvent::Send Session not started");
		return;
	}

	float duration = FUtil::GetTimestamp() - StartTime;
	FloatProperties.Add("duration", duration);
	cog->customEventRecorder->Send(this);
}

void UCustomEvent::SendAtHMDPosition()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();

	if (!cog.IsValid())
	{
		FCognitiveLog::Error("FExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	if (!cog->HasStartedSession())
	{
		FCognitiveLog::Error("UCustomEvent::Send Session not started");
		return;
	}
	cog->TryGetPlayerHMDPosition(Position);
	float duration = FUtil::GetTimestamp() - StartTime;
	FloatProperties.Add("duration", duration);
	cog->customEventRecorder->Send(this);
}

void UCustomEvent::AppendAllSensors()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("FExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	if (!cog->HasStartedSession())
	{
		FCognitiveLog::Error("UCustomEvent::AppendAllSensors Session not started");
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
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();

	if (!cog.IsValid())
	{
		FCognitiveLog::Error("FExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	if (!cog->HasStartedSession())
	{
		FCognitiveLog::Error("UCustomEvent::AppendSensors Session not started");
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
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();

	if (!cog.IsValid())
	{
		FCognitiveLog::Error("FExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	if (!cog->HasStartedSession())
	{
		FCognitiveLog::Error("UCustomEvent::AppendSensor Session not started");
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