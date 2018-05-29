/*
	
*/
#include "CognitiveVR.h"
#include "Private/CognitiveVRPrivatePCH.h"

#include "CognitiveVRBlueprints.h"

using namespace cognitivevrapi;

//extern bool bHasSessionStarted;

void UCognitiveVRBlueprints::SendCustomEvent(FString Category, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();
	UCognitiveVRBlueprints::SendCustomEventPosition(Category, Attributes, HMDPosition);
}

void UCognitiveVRBlueprints::SendCustomEventPosition(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SendCustomEventPosition could not get provider!");
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

	cog->customevent->Send(Category, Position, properties);
}

/*void UCognitiveVRBlueprints::UpdateDevice(const TArray<FAnalyticsEventAttr>& Attributes)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
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

	
	//UPDATE DEVICE STATE
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	//s->thread_manager->PushTask(NULL, "datacollector_updateDeviceState", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateDeviceState");
	jsonObject.Get()->SetField("args", jsonArray);

	//cog->thread_manager->AddJsonToBatch(jsonObject);
	//TODO blueprint update device data


	//cog->transaction->BeginEndPosition(TCHAR_TO_UTF8(*Category), Position, properties);
}*/

/*
void UCognitiveVRBlueprints::UpdateDeviceInt(const FString name, const int32 value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}
	 
void UCognitiveVRBlueprints::UpdateDeviceFloat(const FString name, const float value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}
	 
void UCognitiveVRBlueprints::UpdateDeviceString(const FString name, const FString value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}
	 
	 
void UCognitiveVRBlueprints::UpdateUserInt(const FString name, const int32 value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}
	 
void UCognitiveVRBlueprints::UpdateUserFloat(const FString name, const float value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}
	 
void UCognitiveVRBlueprints::UpdateUserString(const FString name, const FString value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}
*/


void UCognitiveVRBlueprints::UpdateSessionInt(const FString name, const int32 value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitiveVRBlueprints::UpdateSessionFloat(const FString name, const float value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitiveVRBlueprints::UpdateSessionString(const FString name, const FString value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitiveVRBlueprints::SetSessionName(const FString name)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateDevice could not get provider!");
		return;
	}
	cog->SetSessionProperty("cvr.sessionname", name);
}

/*void UCognitiveVRBlueprints::UpdateUser(const TArray<FAnalyticsEventAttr>& Attributes)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateUser could not get provider!");
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


	//UPDATE DEVICE STATE
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	//Util::AppendToJsonArray(jsonArray, fs);
	//Util::AppendToJsonArray(jsonArray, fs);
	//Util::AppendToJsonArray(jsonArray, cog->UserId); //user
	//Util::AppendToJsonArray(jsonArray, empty); //device
	//Util::AppendToJsonArray(jsonArray, properties);

	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateUserState");
	jsonObject.Get()->SetField("args", jsonArray);

	//cog->thread_manager->AddJsonToBatch(jsonObject);
	//TODO blueprint update user data
}*/

void UCognitiveVRBlueprints::RecordSensor(const FString Name, const float Value)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::RecordSensor could not get provider!");
		return;
	}

	cog->sensors->RecordSensor(Name, Value);
}

void UCognitiveVRBlueprints::GetQuestionSet(const FString Hook, const FCognitiveExitPollResponse response)
{
	ExitPoll::MakeQuestionSetRequest(Hook, response);
}

FExitPollQuestionSet UCognitiveVRBlueprints::GetCurrentExitPollQuestionSet()
{
	return ExitPoll::GetCurrentQuestionSet();
}

void UCognitiveVRBlueprints::SendExitPollAnswers(const TArray<FExitPollAnswer>& Answers)
{
	ExitPoll::SendQuestionAnswers(Answers);
}

