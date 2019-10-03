/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "CognitiveVR.h"
#include "network.h"
//#include "AnalyticsSettings.h"

//using namespace cognitivevrapi;
FHttpModule* cognitivevrapi::Network::Http;
FString cognitivevrapi::Network::Gateway;

cognitivevrapi::Network::Network(FAnalyticsProviderCognitiveVR* sp)
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
    s = sp;
	if (Http == NULL)
		Http = &FHttpModule::Get();
}

void cognitivevrapi::Network::NetworkCall(FString suburl, FString contents)
{
	if (s == NULL)
	{
		cognitivevrapi::CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}

	if (s->GetCurrentSceneId().Len() == 0)
	{
		cognitivevrapi::CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}
	if (s->GetCurrentSceneVersionNumber().Len() == 0)
	{
		cognitivevrapi::CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}

	if (Http == NULL)
	{
		cognitivevrapi::CognitiveLog::Warning("Cognitive Provider::SendJson Http module not initialized! likely hasn't started session");
		return;
	}

	//json to scene endpoint
	FString url = "https://"+ Gateway +"/v"+FString::FromInt(0)+"/"+suburl+"/"+s->GetCurrentSceneId() + "?version=" + s->GetCurrentSceneVersionNumber();

	FString AuthValue = "APIKEY:DATA " + s->APIKey;

	TSharedRef<IHttpRequest> HttpRequest = Http->CreateRequest();
	HttpRequest->SetContentAsString(contents);
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->ProcessRequest();

	if (cognitivevrapi::CognitiveLog::DevLogEnabled())
		cognitivevrapi::CognitiveLog::DevLog(url + "\n" + contents);

	return;
}

void cognitivevrapi::Network::NetworkExitPollGetQuestionSet(FString hook, FCognitiveExitPollResponse& response)
{

	TSharedRef<IHttpRequest> HttpRequest = Http->CreateRequest();
	
	auto provider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	
	FString AuthValue = "APIKEY:DATA " + provider->APIKey;
	//TODO move exitpoll get request to network class. use config networkhost and config networkversion
	
	FString url = "https://" + Gateway + "/v" + FString::FromInt(0) + "/questionSetHooks/" + hook + "/questionSet";
	//FString url = "https://data.cognitive3d.com/v0/questionSetHooks/"+ hook +"/questionSet";

	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", AuthValue);
	//lastResponse = response;
	//lastHook = Hook;
	HttpRequest->OnProcessRequestComplete().BindStatic(Network::OnExitPollResponseReceivedAsync);
	HttpRequest->ProcessRequest();
}

void cognitivevrapi::Network::OnExitPollResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!Response.IsValid() || !bWasSuccessful)
	{
		cognitivevrapi::CognitiveLog::Error("Network::OnExitPollResponseReceivedAsync - No valid Response. Check internet connection");
	
		ExitPoll::OnResponseReceived("", false);
		return;
	}
	ExitPoll::OnResponseReceived(Response->GetContentAsString(), true);
}

void cognitivevrapi::Network::NetworkExitPollPostResponse(FExitPollQuestionSet currentQuestionSet, FExitPollResponse Responses)
{
	auto cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

	if (!cogProvider.IsValid() || !cogProvider->HasStartedSession())
	{
		cognitivevrapi::CognitiveLog::Error("Network::NetworkExitPollPostResponse could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> ResponseObject = MakeShareable(new FJsonObject);
	ResponseObject->SetStringField("userId", Responses.user);
	ResponseObject->SetStringField("questionSetId", Responses.questionSetId);
	ResponseObject->SetStringField("sessionId", Responses.sessionId);
	ResponseObject->SetStringField("hook", Responses.hook);

	auto scenedata = cogProvider->GetCurrentSceneData();
	if (scenedata.IsValid())
	{
		ResponseObject->SetStringField("sceneId", scenedata->Id);
		ResponseObject->SetNumberField("versionNumber", scenedata->VersionNumber);
		ResponseObject->SetNumberField("versionId", scenedata->VersionId);
	}

	TArray<TSharedPtr<FJsonValue>> answerValues;
	

	for (int32 i = 0; i < Responses.answers.Num(); i++)
	{
		TSharedPtr<FJsonObject> answerObject = MakeShareable(new FJsonObject);
		answerObject->SetStringField("type", Responses.answers[i].type);
		if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::String)
		{
			answerObject->SetStringField("value", Responses.answers[i].stringValue);
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Number)
		{
			answerObject->SetNumberField("value", Responses.answers[i].numberValue);
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Bool)
		{
			if (Responses.answers[i].boolValue == true)
			{
				answerObject->SetNumberField("value", 1);
			}
			else
			{
				answerObject->SetNumberField("value", 0);
			}
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Null)
		{
			answerObject->SetNumberField("value", -32768);
		}
		TSharedPtr<FJsonValueObject> ao = MakeShareable(new FJsonValueObject(answerObject));
		answerValues.Add(ao);
	}
	ResponseObject->SetArrayField("answers", answerValues);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(ResponseObject.ToSharedRef(), Writer);

	//serialize and send this json

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	//TODO move exitpoll send request to network class. use config networkhost and config networkversion
	
	FString url = "https://" + Gateway + "/v" + FString::FromInt(0) + "/questionSets/" + currentQuestionSet.name + "/" + FString::FromInt(currentQuestionSet.version) + "/responses";
	//FString url = "https://data.cognitive3d.com/v0/questionSets/" + currentQuestionSet.name + "/" + FString::FromInt(currentQuestionSet.version) + "/responses";
	FString AuthValue = "APIKEY:DATA " + cogProvider->APIKey;

	HttpRequest->SetURL(url);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetContentAsString(OutputString);
	//HttpRequest->OnProcessRequestComplete().BindStatic(ExitPoll::OnQuestionResponse);
	HttpRequest->ProcessRequest();



	//send this as a transaction too
	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	properties->SetStringField("userId", Responses.user);
	properties->SetStringField("questionSetId", Responses.questionSetId);
	properties->SetStringField("hook", Responses.hook);
	properties->SetNumberField("duration", Responses.duration);

	for (int32 i = 0; i < Responses.answers.Num(); i++)
	{
		if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Number)
		{
			properties->SetNumberField("Answer" + FString::FromInt(i), Responses.answers[i].numberValue);
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Bool) //bool as number
		{
			if (Responses.answers[i].boolValue == true)
			{
				properties->SetNumberField("Answer" + FString::FromInt(i), 1);
			}
			else
			{
				properties->SetNumberField("Answer" + FString::FromInt(i), 0);
			}
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Null)
		{
			//skipped answer
			properties->SetNumberField("Answer" + FString::FromInt(i), -32768);
		}
		else if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::String)
		{
			//voice answer. don't display on dashboard, but not skipped
			properties->SetNumberField("Answer" + FString::FromInt(i), 0);
		}
	}

	//IMPROVEMENT custom event position should be exitpoll panel position. how to get panel position?
	cogProvider.Get()->customeventrecorder->Send(FString("c3d.exitpoll"), FVector(0, 0, 0), properties);

	//then flush transactions
	cogProvider.Get()->FlushEvents();
}