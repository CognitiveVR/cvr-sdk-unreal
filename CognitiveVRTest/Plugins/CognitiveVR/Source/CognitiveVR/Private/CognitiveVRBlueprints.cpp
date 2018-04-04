/*
	
*/
#include "CognitiveVR.h"
#include "Private/CognitiveVRPrivatePCH.h"

#include "CognitiveVRBlueprints.h"

using namespace cognitivevrapi;

extern bool bHasSessionStarted;

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
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::EndTransaction could not get provider!");
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

void UCognitiveVRBlueprints::UpdateDevice(const TArray<FAnalyticsEventAttr>& Attributes)
{
	//SOME TRANSACTION OR SOMETHING
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
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
	//Util::AppendToJsonArray(jsonArray, empty); //user
	//Util::AppendToJsonArray(jsonArray, cog->DeviceId); //device
	//Util::AppendToJsonArray(jsonArray, properties);

	//s->thread_manager->PushTask(NULL, "datacollector_updateDeviceState", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateDeviceState");
	jsonObject.Get()->SetField("args", jsonArray);

	//cog->thread_manager->AddJsonToBatch(jsonObject);
	//TODO blueprint update device data


	//cog->transaction->BeginEndPosition(TCHAR_TO_UTF8(*Category), Position, properties);
}

void UCognitiveVRBlueprints::UpdateUser(const TArray<FAnalyticsEventAttr>& Attributes)
{
	//SOME TRANSACTION OR SOMETHING
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
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
}

void UCognitiveVRBlueprints::RecordSensor(const FString Name, const float Value)
{
	//SOME TRANSACTION OR SOMETHING
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
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

//void UCognitiveVRBlueprints::SendExitPollResponse(FExitPollResponses Responses)
//{
	/*TMap<FString, FString> answers;

	for (int32 i = 0; i < Responses.answers.Num(); i++)
	{
		answers.Add(Responses.answers[i].type, Responses.answers[i].value);
	}
	ExitPoll::SendQuestionResponses(Responses.user,Responses.questionSetId,Responses.sessionId,Responses.hook,answers);*/
//	ExitPoll::SendQuestionResponses(Responses);
//}

void UCognitiveVRBlueprints::SendExitPollAnswers(const TArray<FExitPollAnswer>& Answers)
{
	ExitPoll::SendQuestionAnswers(Answers);
}

