/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Network.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"

FNetwork::FNetwork()
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cognitive.IsValid()) {
		return;
	}

	FString C3DSettingsPath = cognitive->GetSettingsFilePathRuntime();
	GConfig->LoadFile(C3DSettingsPath);

	Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	Http = &FHttpModule::Get();
	hasErrorResponse = false;
}

void FNetwork::NetworkCall(FString suburl, FString contents, bool copyDataToCache)
{
	if (!cog.IsValid())
	{
		FCognitiveLog::Warning("Network::NetworkCall Cognitive3DProvider is invalid!");
		return;
	}

	if (cog->GetCurrentSceneId().Len() == 0)
	{
		FCognitiveLog::Warning("Network::NetworkCall Cognitive3DProvider scene id is invalid!");
		return;
	}
	if (cog->GetCurrentSceneVersionNumber().Len() == 0)
	{
		FCognitiveLog::Warning("Network::NetworkCall Cognitive3DProvider scene version is invalid!");
		return;
	}

	if (Http == NULL)
	{
		FCognitiveLog::Warning("Network::NetworkCall Http module not initialized! possibly hasn't started session");
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
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FNetwork::OnSessionDataResponse);

	if (IsServerUnreachable)
	{
		//
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
		//
		
	
		return;	
	}
	else
	{
		HttpRequest->ProcessRequest();
	}

	if (FCognitiveLog::DevLogEnabled())
		FCognitiveLog::DevLog(url + "\n" + contents);
	
	//this section is only for writing requests to the cache when the app is closing and we dont know what the response will be
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

inline FString FNetwork::TArrayToString(const TArray<uint8> In, int32 Count)
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

void FNetwork::OnSessionDataResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!cog.IsValid()) { GLog->Log("Network::OnCallReceivedAsync response while cognitive providor is invalid"); return; }
	if (!cog->HasStartedSession()) { GLog->Log("Network::OnCallReceivedAsync response while session not started"); return; }
	if (Response.IsValid())
	{
		int32 responseCode = Response.Get()->GetResponseCode();
		//during the wait time for the 500 error code, skip this part and only do the "else" block
		if (responseCode == 200) 
		{
			VariableDelayMultiplier = 0;
			if (!cog->localCache.IsValid()) { GLog->Log("Network::OnCallReceivedAsync response while local cache is invalid"); return; }
			if (localCacheRequest == NULL)
			{
				//start uploading data
				FString contents;
				FString url;
				auto world = ACognitive3DActor::GetCognitiveSessionWorld();
				if (world != nullptr)
				{
					if (world->GetTimerManager().IsTimerActive(TimerHandleShortDelay))
					{
						return;
					}
					else
					{
					}
					if (cog->localCache->PeekContent(url, contents))
					{
						localCacheRequest = Http->CreateRequest();
						localCacheRequest->SetContentAsString(*contents);
						localCacheRequest->SetURL(*url);
						localCacheRequest->SetVerb("POST");
						FString AuthValue = "APIKEY:DATA " + cog->ApplicationKey;
						localCacheRequest->SetHeader("Content-Type", TEXT("application/json"));
						localCacheRequest->SetHeader("Authorization", AuthValue);

						if (!world->GetTimerManager().IsTimerActive(TimerHandleShortDelay))
						{
							ResetVariableTimer();
							world->GetTimerManager().SetTimer(TimerHandleShortDelay, FTimerDelegate::CreateRaw(this, &FNetwork::RequestLocalCache), LocalCacheLoopDelay, false);
						}
						VariableDelayMultiplier = 0;
					}
				}
			}
			else
			{
			}
		}
		//when we get here, we start the variable delay, 60 seconds each time, and just let the rest of the block happen
		else 
		{
			if (responseCode < 500 && responseCode != 0) {return;}

			if (Request == nullptr) { return; }

			//hold a pointer to a request specifically for uploading cached data
			//if request exist, cancel it
			auto world = ACognitive3DActor::GetCognitiveSessionWorld();
			if (world != nullptr)
			{
				world->GetTimerManager().ClearTimer(TimerHandleShortDelay);
				if (!IsServerUnreachable)
				{
					IsServerUnreachable = true;
					++VariableDelayMultiplier;

					if (!world->GetTimerManager().IsTimerActive(TimerHandle))
					{
						float VariableDelay = VariableDelayTime * VariableDelayMultiplier;
						world->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateRaw(this, &FNetwork::ResetVariableTimer), VariableDelay, false, VariableDelay);
					}
				}
			}
			
			

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
		//FCognitiveLog::DevLog("Network::OnCallReceivedAsync Response Invalid!");
	}
}

void FNetwork::SessionEnd()
{
	VariableDelayMultiplier = 0;
	localCacheRequest = NULL;
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world != nullptr)
	{
		world->GetTimerManager().ClearTimer(TimerHandle);
		world->GetTimerManager().ClearTimer(TimerHandleShortDelay);
	}
	
}

void FNetwork::RequestLocalCache()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world != nullptr)
	{
		if (!world->GetTimerManager().IsTimerActive(TimerHandleShortDelay))
		{
			if (IsServerUnreachable)
			{
				world->GetTimerManager().ClearTimer(TimerHandleShortDelay);
				localCacheRequest = NULL;
				return;
			}
			localCacheRequest->OnProcessRequestComplete().BindRaw(this, &FNetwork::OnLocalCacheResponse);
			localCacheRequest->ProcessRequest();
		}
	}
	
}

void FNetwork::ResetVariableTimer()
{
	IsServerUnreachable = false;
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world != nullptr)
	{
		if (!world->GetTimerManager().IsTimerActive(TimerHandle))
		{
			world->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
}

void FNetwork::OnLocalCacheResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!cog.IsValid()) { return; }
	if (!cog->HasStartedSession()) { FCognitiveLog::Info("Network::OnLocalCacheCallReceivedAsync get response while session not started"); return; }
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	
	if (world == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ACognitive3DActor::GetCognitiveSessionWorld() returns NULLPTR"));
		return;
	}

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

				if (!world->GetTimerManager().IsTimerActive(TimerHandle))
				{
					world->GetTimerManager().ClearTimer(TimerHandleShortDelay);
					world->GetTimerManager().SetTimer(TimerHandleShortDelay, FTimerDelegate::CreateRaw(this, &FNetwork::RequestLocalCache), LocalCacheLoopDelay, false);
				}
			}
			else
			{
				localCacheRequest = NULL;
				world->GetTimerManager().ClearTimer(TimerHandleShortDelay);
			}
		}
		else
		{
			if (responseCode < 500 && responseCode != 0)
			{
				cog->localCache->PopContent();
				world->GetTimerManager().ClearTimer(TimerHandleShortDelay);
				localCacheRequest = NULL;
				return;
			}


			//if local cache upload request gets 500 response start variable delay timer
			if (!IsServerUnreachable)
			{
				IsServerUnreachable = true;
				if (VariableDelayMultiplier < 10)
				{
					VariableDelayMultiplier++;
				}
				world->GetTimerManager().ClearTimer(TimerHandleShortDelay);
				localCacheRequest = NULL;

				if (!world->GetTimerManager().IsTimerActive(TimerHandle))
				{
					float VariableDelay = VariableDelayTime * VariableDelayMultiplier;
					world->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateRaw(this, &FNetwork::ResetVariableTimer), VariableDelay, false, VariableDelay);
				}
			}

			world->GetTimerManager().ClearTimer(TimerHandleShortDelay);
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
		FCognitiveLog::Error("Network::OnLocalCacheCallReceivedAsync Response Invalid");
		localCacheRequest = NULL;
	}
}

bool FNetwork::HasErrorResponse()
{
	return hasErrorResponse;
}

void FNetwork::NetworkExitPollGetQuestionSet(FString hook, FCognitiveExitPollResponse& response)
{
	auto HttpRequest = Http->CreateRequest();
	
	FString AuthValue = "APIKEY:DATA " + cog->ApplicationKey;
	
	FString url = "https://" + Gateway + "/v" + FString::FromInt(0) + "/questionSetHooks/" + hook + "/questionSet";

	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FNetwork::OnExitPollResponseReceivedAsync);
	HttpRequest->ProcessRequest();
}

void FNetwork::OnExitPollResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!Response.IsValid() || !bWasSuccessful)
	{
		FCognitiveLog::Error("Network::OnExitPollResponseReceivedAsync - No valid Response. Check internet connection");
		cog->exitpoll->OnResponseReceived("", false);
		return;
	}
	cog->exitpoll->OnResponseReceived(Response->GetContentAsString(), true);
}

void FNetwork::NetworkExitPollPostResponse(FExitPollQuestionSet currentQuestionSet, FExitPollResponse Responses)
{
	if (!cog.IsValid() || !cog->HasStartedSession())
	{
		FCognitiveLog::Error("Network::NetworkExitPollPostResponse could not get provider!");
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


