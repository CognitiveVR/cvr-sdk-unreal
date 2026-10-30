/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Network.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Containers/StringConv.h"

FNetwork::FNetwork()
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cognitive.IsValid()) { return; }
	FString C3DSettingsPath = cognitive->GetSettingsFilePathRuntime();
	GConfig->LoadFile(C3DSettingsPath);
	Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
	cog = cognitive;
	Http = &FHttpModule::Get();
}

bool FNetwork::CacheRequest(FHttpRequestPtr Request)
{
	if (!Request.IsValid() || !cog.IsValid() || !cog->localCache.IsValid() || !cog->localCache->IsEnabled()) { return false; }
	const TArray<uint8>& Content = Request->GetContent();
	return cog->localCache->WriteData(Request->GetURL(), TArrayToString(Content, Content.Num()));
}

void FNetwork::NetworkCall(FString suburl, FString contents, bool copyDataToCache)
{
	if (!cog.IsValid()) { FCognitiveLog::Warning("Network::NetworkCall Cognitive3DProvider is invalid!"); return; }
	if (cog->GetCurrentSceneId().IsEmpty()) { FCognitiveLog::Warning("Network::NetworkCall Cognitive3DProvider scene id is invalid!"); return; }
	if (cog->GetCurrentSceneVersionNumber().IsEmpty()) { FCognitiveLog::Warning("Network::NetworkCall Cognitive3DProvider scene version is invalid!"); return; }
	if (Http == nullptr) { FCognitiveLog::Warning("Network::NetworkCall Http module not initialized!"); return; }

	const FString Url = "https://" + Gateway + "/v0/" + suburl + "/" + cog->GetCurrentSceneId() + "?version=" + cog->GetCurrentSceneVersionNumber();
	auto HttpRequest = Http->CreateRequest();
	HttpRequest->SetContentAsString(contents);
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->SetHeader("Authorization", "APIKEY:DATA " + cog->ApplicationKey);
	if (IsServerUnreachable)
	{
		const bool bCached = CacheRequest(HttpRequest);
		FCognitiveLog::Info(FString::Printf(TEXT("Cognitive3D delivery deferred; cached=%s"), bCached ? TEXT("true") : TEXT("false")));
		return;
	}

	const bool bAlreadyCached = copyDataToCache && CacheRequest(HttpRequest);
	const TSharedRef<bool> CompletionHandled = MakeShared<bool>(false);
	HttpRequest->OnProcessRequestComplete().BindLambda([this, bAlreadyCached, CompletionHandled](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (*CompletionHandled) { return; }
		*CompletionHandled = true;
		OnSessionDataResponse(Request, Response, bWasSuccessful, bAlreadyCached);
	});
	if (!HttpRequest->ProcessRequest() && !*CompletionHandled)
	{
		*CompletionHandled = true;
		OnSessionDataResponse(HttpRequest, FHttpResponsePtr(), false, bAlreadyCached);
	}
	if (FCognitiveLog::DevLogEnabled()) { FCognitiveLog::DevLog("Cognitive3D recording request submitted: " + suburl); }
}

FString FNetwork::TArrayToString(const TArray<uint8> In, int32 Count)
{
	const int32 SafeCount = FMath::Clamp(Count, 0, In.Num());
	if (SafeCount == 0) { return FString(); }
	FUTF8ToTCHAR Converted(reinterpret_cast<const ANSICHAR*>(In.GetData()), SafeCount);
	return FString(Converted.Length(), Converted.Get());
}

void FNetwork::BackOffAfterFailure()
{
	hasErrorResponse = true;
	IsServerUnreachable = true;
	VariableDelayMultiplier = FMath::Min(VariableDelayMultiplier + 1, 10);
	auto World = ACognitive3DActor::GetCognitiveSessionWorld();
	if (World == nullptr) { return; }
	World->GetTimerManager().ClearTimer(TimerHandleShortDelay);
	const float Delay = static_cast<float>(VariableDelayTime * VariableDelayMultiplier);
	World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateRaw(this, &FNetwork::ResetVariableTimer), Delay, false);
}

void FNetwork::BeginCacheUpload()
{
	if (!cog.IsValid() || !cog->HasStartedSession() || !cog->localCache.IsValid() || Http == nullptr || IsServerUnreachable || localCacheRequest.IsValid()) { return; }
	auto World = ACognitive3DActor::GetCognitiveSessionWorld();
	if (World == nullptr) { return; }
	FString Url;
	FString Contents;
	if (!cog->localCache->PeekContent(Url, Contents)) { return; }
	localCacheRequest = Http->CreateRequest();
	localCacheRequest->SetContentAsString(Contents);
	localCacheRequest->SetURL(Url);
	localCacheRequest->SetVerb("POST");
	localCacheRequest->SetHeader("Content-Type", TEXT("application/json"));
	localCacheRequest->SetHeader("Authorization", "APIKEY:DATA " + cog->ApplicationKey);
	World->GetTimerManager().SetTimer(TimerHandleShortDelay, FTimerDelegate::CreateRaw(this, &FNetwork::RequestLocalCache), LocalCacheLoopDelay, false);
}

void FNetwork::OnSessionDataResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, bool bAlreadyCached)
{
	if (!cog.IsValid()) { return; }
	const int32 Status = Response.IsValid() ? Response->GetResponseCode() : 0;
	if (bWasSuccessful && Status == 200)
	{
		FCognitiveLog::Info("Cognitive3D recording delivery accepted: HTTP 200");
		if (cog->HasStartedSession())
		{
			VariableDelayMultiplier = 0;
			IsServerUnreachable = false;
			BeginCacheUpload();
		}
		return;
	}
	const bool bCached = bAlreadyCached || CacheRequest(Request);
	FCognitiveLog::Warning(FString::Printf(TEXT("Cognitive3D recording delivery failed: HTTP %d, transport_success=%s, cached=%s"), Status, bWasSuccessful ? TEXT("true") : TEXT("false"), bCached ? TEXT("true") : TEXT("false")));
	if (cog->HasStartedSession()) { BackOffAfterFailure(); }
}

void FNetwork::SessionEnd()
{
	VariableDelayMultiplier = 0;
	IsServerUnreachable = false;
	localCacheRequest.Reset();
	auto World = ACognitive3DActor::GetCognitiveSessionWorld();
	if (World != nullptr)
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
		World->GetTimerManager().ClearTimer(TimerHandleShortDelay);
	}
}

void FNetwork::RequestLocalCache()
{
	if (!localCacheRequest.IsValid() || IsServerUnreachable || !cog.IsValid() || !cog->HasStartedSession()) { return; }
	auto Request = localCacheRequest;
	Request->OnProcessRequestComplete().BindRaw(this, &FNetwork::OnLocalCacheResponse);
	if (!Request->ProcessRequest()) { OnLocalCacheResponse(Request, FHttpResponsePtr(), false); }
}

void FNetwork::ResetVariableTimer()
{
	IsServerUnreachable = false;
	BeginCacheUpload();
}

void FNetwork::OnLocalCacheResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!localCacheRequest.IsValid() || Request != localCacheRequest || !cog.IsValid() || !cog->HasStartedSession()) { return; }
	const int32 Status = Response.IsValid() ? Response->GetResponseCode() : 0;
	localCacheRequest.Reset();
	if (bWasSuccessful && Status == 200)
	{
		if (cog->localCache.IsValid()) { cog->localCache->PopContent(); }
		VariableDelayMultiplier = 0;
		IsServerUnreachable = false;
		FCognitiveLog::Info("Cognitive3D cached delivery accepted: HTTP 200");
		BeginCacheUpload();
		return;
	}
	FCognitiveLog::Warning(FString::Printf(TEXT("Cognitive3D cached delivery failed: HTTP %d; entry retained"), Status));
	BackOffAfterFailure();
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


