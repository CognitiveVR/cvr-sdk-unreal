/*
	
*/
#include "CognitiveVR.h"
#include "Private/CognitiveVRPrivatePCH.h"

#include "CognitiveVRBlueprints.h"

void UCognitiveVRBlueprints::SendCustomEvent(FString Category, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();
	UCognitiveVRBlueprints::SendCustomEventToCore(Category, Attributes, HMDPosition,NULL);
}

void UCognitiveVRBlueprints::SendCustomEventDynamic(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, UDynamicObject* dynamic, FVector Position)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();

	UCognitiveVRBlueprints::SendCustomEventToCore(Category, Attributes, HMDPosition, dynamic);
}

void UCognitiveVRBlueprints::SendCustomEventToCore(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position, UDynamicObject* dynamic)
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::SendCustomEventToCore could not get provider!");
		return;
	}
	if (!cog->customeventrecorder.IsValid())
	{
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	for (int32 i = 0; i < Attributes.Num(); i++)
	{
		if (Attributes[i].Name.Len() > 0)
		{
			properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
		}
	}

	FString dynamicId = "";
	if (dynamic != NULL)
	{
		dynamicId = dynamic->GetObjectId()->Id;
	}

	cog->customeventrecorder->Send(Category, Position, properties, dynamicId);
}

void UCognitiveVRBlueprints::SendCustomEventPosition(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	SendCustomEventToCore(Category, Attributes, Position, NULL);
}

void UCognitiveVRBlueprints::UpdateSessionInt(const FString name, const int32 value)
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::UpdateSessionInt could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitiveVRBlueprints::UpdateSessionFloat(const FString name, const float value)
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::UpdateSessionFloat could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitiveVRBlueprints::UpdateSessionString(const FString name, const FString value)
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::UpdateSessionString could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitiveVRBlueprints::SetSessionName(const FString name)
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::SetSessionName could not get provider!");
		return;
	}
	cog->SetSessionName(name);
}

void UCognitiveVRBlueprints::SetLobbyId(const FString lobbyId)
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return;
	}
	cog->SetLobbyId(lobbyId);
}

void UCognitiveVRBlueprints::RecordSensor(const FString Name, const float Value)
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::RecordSensor could not get provider!");
		return;
	}
	if (!cog->sensors.IsValid())
	{
		return;
	}
	cog->sensors->RecordSensor(Name, Value);
}

void UCognitiveVRBlueprints::GetQuestionSet(const FString Hook, FCognitiveExitPollResponse response)
{
	ExitPoll::MakeQuestionSetRequest(Hook, response);
}

bool UCognitiveVRBlueprints::HasSessionStarted()
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		return false;
	}

	return cog->HasStartedSession();
}

FExitPollQuestionSet UCognitiveVRBlueprints::GetCurrentExitPollQuestionSet()
{
	return ExitPoll::GetCurrentQuestionSet();
}

void UCognitiveVRBlueprints::SendExitPollAnswers(const TArray<FExitPollAnswer>& Answers)
{
	ExitPoll::SendQuestionAnswers(Answers);
}

void UCognitiveVRBlueprints::FlushEvents()
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return;
	}
	cog->FlushEvents();
}

bool UCognitiveVRBlueprints::StartSession()
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return false;
	}
	return cog->StartSession(TArray<FAnalyticsEventAttribute>());
}

void UCognitiveVRBlueprints::EndSession()
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return;
	}
	cog->EndSession();
}

void UCognitiveVRBlueprints::SetUserId(const FString Name)
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return;
	}
	cog->SetUserID(Name);
}

FCustomEvent UCognitiveVRBlueprints::MakeCustomEvent(FString eventName)
{
	return FCustomEvent(eventName);
}

FCustomEvent UCognitiveVRBlueprints::SetIntegerProperty(UPARAM(ref) FCustomEvent& target, FString key, int32 intValue)
{
	target.SetProperty(key, intValue);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetStringProperty(UPARAM(ref) FCustomEvent& target, FString key, FString stringValue)
{
	target.SetProperty(key, stringValue);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetBoolProperty(UPARAM(ref) FCustomEvent& target, FString key, bool boolValue)
{
	target.SetProperty(key, boolValue);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetFloatProperty(UPARAM(ref) FCustomEvent& target, FString key, float floatValue)
{
	target.SetProperty(key, floatValue);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetDynamicObject(UPARAM(ref) FCustomEvent& target, UDynamicObject* dynamicObject)
{
	if (dynamicObject == NULL)
	{
		return target;
	}
	target.SetDynamicObject(dynamicObject);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetPosition(UPARAM(ref) FCustomEvent& target, FVector position)
{
	target.SetPosition(position);
	return target;
}


void UCognitiveVRBlueprints::Send(UPARAM(ref) FCustomEvent& target)
{
	target.Send();
}

FCustomEvent UCognitiveVRBlueprints::AppendAllSensors(UPARAM(ref) FCustomEvent& target)
{
	target.AppendSensors();
	return target;
}

FCustomEvent UCognitiveVRBlueprints::AppendSensors(UPARAM(ref) FCustomEvent& target, TArray<FString> keys)
{
	target.AppendSensors(keys);
	return target;
}

FCustomEvent UCognitiveVRBlueprints::AppendSensor(UPARAM(ref) FCustomEvent& target, FString key)
{
	target.AppendSensor(key);
	return target;
}