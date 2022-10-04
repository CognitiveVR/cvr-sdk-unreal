/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#include "network.h"

Network::Network()
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	Http = &FHttpModule::Get();
	hasErrorResponse = false;
}

void Network::NetworkCall(FString suburl, FString contents, bool copyDataToCache)
{
	if (!cog.IsValid())
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}

	if (cog->GetCurrentSceneId().Len() == 0)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}
	if (cog->GetCurrentSceneVersionNumber().Len() == 0)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}

	if (Http == NULL)
	{
		CognitiveLog::Warning("Cognitive Provider::SendJson Http module not initialized! likely hasn't started session");
		return;
	}

	//json to scene endpoint
	FString url = "https://"+ Gateway +"/v"+FString::FromInt(0)+"/"+suburl+"/"+cog->GetCurrentSceneId() + "?version=" + cog->GetCurrentSceneVersionNumber();

	FString AuthValue = "APIKEY:DATA " + cog->ApplicationKey;

	auto HttpRequest = Http->CreateRequest();
	HttpRequest->SetContentAsString(contents);
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->SetHeader("Authorization", AuthValue);
	
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &Network::OnCallReceivedAsync);
	HttpRequest->ProcessRequest();

	if (CognitiveLog::DevLogEnabled())
		CognitiveLog::DevLog(url + "\n" + contents);

	if (!copyDataToCache) { return; }
	if (!cog.IsValid()) { return; }
	if (!cog->HasStartedSession()) { return; }

	if (!cog->localCache.IsValid()) { return; }
	if (cog->localCache->IsEnabled())
	{
		TArray<uint8> contentArray = HttpRequest->GetContent();
		if (cog->localCache->CanWrite(contentArray.Num()))
		{
			FString contentString = TArrayToString(contentArray, HttpRequest->GetContent().Num());
			cog->localCache->WriteData(HttpRequest->GetURL(), contentString);
		}
	}
	
}

inline FString Network::TArrayToString(const TArray<uint8> In, int32 Count)
{
	FString Result;
	Result.Empty(Count);

	for (int32 i = 0; i < Count; i++)
	{
		int16 Value = In[i];
		Result += TCHAR(Value);
	}
	return Result;
}

void Network::OnCallReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!cog.IsValid()) { return; }
	if (!cog->HasStartedSession()) { GLog->Log("Network::OnCallReceivedAsync response while session not started"); return; }
	if (Response.IsValid())
	{
		int32 responseCode = Response.Get()->GetResponseCode();
		if (responseCode == 200)
		{
			if (!cog->localCache.IsValid()) { return; }
			if (localCacheRequest == NULL)
			{
				//start uploading data
				FString contents;
				FString url;
				if (cog->localCache->PeekContent(url, contents))
				{
					localCacheRequest = Http->CreateRequest();
					localCacheRequest->SetContentAsString(*contents);
					localCacheRequest->SetURL(*url);
					localCacheRequest->SetVerb("POST");
					FString AuthValue = "APIKEY:DATA " + cog->ApplicationKey;
					localCacheRequest->SetHeader("Content-Type", TEXT("application/json"));
					localCacheRequest->SetHeader("Authorization", AuthValue);

					localCacheRequest->OnProcessRequestComplete().BindRaw(this, &Network::OnLocalCacheCallReceivedAsync);
					localCacheRequest->ProcessRequest();
				}
			}
		}
		else
		{
			if (responseCode == 401) { return; }
			if (responseCode == 404) { return; }
			if (responseCode == -1) { return; }

			if (Request == nullptr) { return; }

			//hold a pointer to a request specifically for uploading cached data
			//if request exist, cancel it
			
			if (cog->localCache == nullptr) { return; } //not set to null on session end
			//isUploadingFromCache = false;
			if (cog->localCache->IsEnabled())
			{
				TArray<uint8> contentArray = Request->GetContent();
				if (cog->localCache->CanWrite(contentArray.Num()))
				{
					FString contentString = TArrayToString(contentArray, Request->GetContent().Num());
					cog->localCache->WriteData(Request->GetURL(), contentString);
				}
			}

			hasErrorResponse = true;
		}
	}
	else
	{
		//CognitiveLog::DevLog("Network::OnCallReceivedAsync Response Invalid!");
	}
}

void Network::OnLocalCacheCallReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!cog.IsValid()) { return; }
	if (!cog->HasStartedSession()) { CognitiveLog::Info("Network::OnLocalCacheCallReceivedAsync get response while session not started"); return; }
	if (Response.IsValid())
	{
		int32 responseCode = Response.Get()->GetResponseCode();
		if (responseCode == 200)
		{
			if (!cog->localCache.IsValid()) { return; }
			cog->localCache->PopContent();
			localCacheRequest = NULL;
			FString contents;
			FString url;
			if (cog->localCache->PeekContent(url, contents))
			{
				localCacheRequest = Http->CreateRequest();
				localCacheRequest->SetContentAsString(*contents);
				localCacheRequest->SetURL(*url);
				localCacheRequest->SetVerb("POST");
				FString AuthValue = "APIKEY:DATA " + cog->ApplicationKey;
				localCacheRequest->SetHeader("Content-Type", TEXT("application/json"));
				localCacheRequest->SetHeader("Authorization", AuthValue);

				localCacheRequest->OnProcessRequestComplete().BindRaw(this, &Network::OnLocalCacheCallReceivedAsync);
				localCacheRequest->ProcessRequest();
			}
			else
			{
				localCacheRequest = NULL;
			}
		}
		else
		{
			//TEST should pop content from the read file anyway and write it to the write file
			if (cog->localCache->IsEnabled())
			{
				TArray<uint8> contentArray = Request->GetContent();
				if (cog->localCache->CanWrite(contentArray.Num()))
				{
					FString contentString = TArrayToString(contentArray, Request->GetContent().Num());
					cog->localCache->WriteData(Request->GetURL(), contentString);
					cog->localCache->PopContent();
				}
			}
			localCacheRequest = NULL;
		}
	}
	else
	{
		CognitiveLog::Error("Network::OnLocalCacheCallReceivedAsync Response Invalid");
		localCacheRequest = NULL;
	}
}

bool Network::HasErrorResponse()
{
	return hasErrorResponse;
}

void Network::NetworkExitPollGetQuestionSet(FString hook, FCognitiveExitPollResponse& response)
{
	auto HttpRequest = Http->CreateRequest();
	
	FString AuthValue = "APIKEY:DATA " + cog->ApplicationKey;
	
	FString url = "https://" + Gateway + "/v" + FString::FromInt(0) + "/questionSetHooks/" + hook + "/questionSet";

	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &Network::OnExitPollResponseReceivedAsync);
	HttpRequest->ProcessRequest();
}

void Network::OnExitPollResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!Response.IsValid() || !bWasSuccessful)
	{
		CognitiveLog::Error("Network::OnExitPollResponseReceivedAsync - No valid Response. Check internet connection");
		cog->exitpoll->OnResponseReceived("", false);
		return;
	}
	cog->exitpoll->OnResponseReceived(Response->GetContentAsString(), true);
}

void Network::NetworkExitPollPostResponse(FExitPollQuestionSet currentQuestionSet, FExitPollResponse Responses)
{
	if (!cog.IsValid() || !cog->HasStartedSession())
	{
		CognitiveLog::Error("Network::NetworkExitPollPostResponse could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> ResponseObject = MakeShareable(new FJsonObject);
	ResponseObject->SetStringField("userId", Responses.userId);
	ResponseObject->SetStringField("participantId", Responses.participantId);
	ResponseObject->SetStringField("questionSetId", Responses.questionSetId);
	ResponseObject->SetStringField("sessionId", Responses.sessionId);
	ResponseObject->SetStringField("hook", Responses.hook);

	auto scenedata = cog->GetCurrentSceneData();
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

	auto HttpRequest = FHttpModule::Get().CreateRequest();
	
	FString url = "https://" + Gateway + "/v" + FString::FromInt(0) + "/questionSets/" + currentQuestionSet.name + "/" + FString::FromInt(currentQuestionSet.version) + "/responses";
	FString AuthValue = "APIKEY:DATA " + cog->ApplicationKey;

	HttpRequest->SetURL(url);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->ProcessRequest();

	//send this as a transaction too
	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	properties->SetStringField("userId", Responses.userId);
	properties->SetStringField("participantId", Responses.participantId);
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
	cog->customEventRecorder->Send(FString("c3d.exitpoll"), FVector(0, 0, 0), properties);

	//then flush transactions
	cog->FlushEvents();
}