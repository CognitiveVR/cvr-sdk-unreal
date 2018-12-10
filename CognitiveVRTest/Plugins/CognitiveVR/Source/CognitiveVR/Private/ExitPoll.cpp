// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "ExitPoll.h"
#include "CognitiveVRSettings.h"
#include "Util.h"

FCognitiveExitPollResponse r;
FExitPollQuestionSet currentSet;
FString lastHook;
TSharedPtr<FAnalyticsProviderCognitiveVR> cogProvider;

void ExitPoll::MakeQuestionSetRequest(const FString Hook, const FCognitiveExitPollResponse& response)
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	if (cogProvider.Get() == NULL)
	{
		cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	}

	FString AuthValue = "APIKEY:DATA " + cogProvider->APIKey;
	//TODO move exitpoll get request to network class. use config networkhost and config networkversion
	FString url = "https://data.cognitive3d.com/v0/questionSetHooks/"+Hook+"/questionSet";
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", AuthValue);
	r = response;
	lastHook = Hook;
	HttpRequest->OnProcessRequestComplete().BindStatic(ExitPoll::OnResponseReceivedAsync);
	HttpRequest->ProcessRequest();
}

void ExitPoll::OnResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!Response.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::OnResponseReceivedAsync - No valid Response. Check internet connection");

		if (r.IsBound())
		{
			r.Execute(FExitPollQuestionSet());
		}
		return;
	}

	currentSet = FExitPollQuestionSet();

	FString UE4Str = Response->GetContentAsString();
	cognitivevrapi::CognitiveLog::Info("ExitPoll::OnResponseReceivedAsync - Response: " + UE4Str);

	//CognitiveVRResponse response = Network::ParseResponse(content);

	FExitPollQuestionSet set = FExitPollQuestionSet();
	TSharedPtr<FJsonObject> jobject;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(UE4Str);
	if (FJsonSerializer::Deserialize(Reader, jobject))
	{
		if (!jobject->HasField(TEXT("customerId")))
		{
			cognitivevrapi::CognitiveLog::Info("ExitPoll::OnResponseReceivedAsync - no customerId in response - fail");
			if (r.IsBound())
			{
				r.Execute(FExitPollQuestionSet());
			}
			return;
		}

		set.customerId = jobject->GetStringField(TEXT("customerId"));
		set.id = jobject->GetStringField(TEXT("id"));
		set.name = jobject->GetStringField(TEXT("name"));
		set.version = jobject->GetNumberField(TEXT("version"));
		set.title = jobject->GetStringField(TEXT("title"));
		set.status = jobject->GetStringField(TEXT("status"));

		TArray<TSharedPtr<FJsonValue>> questions = jobject->GetArrayField(TEXT("questions"));
		for (int32 i = 0; i < questions.Num(); i++)
		{
			//replace with trygets
			auto qobject = questions[i]->AsObject();
			FExitPollQuestion q = FExitPollQuestion();
			q.title = qobject->GetStringField(TEXT("title"));
			q.type = qobject->GetStringField(TEXT("type"));

			//voice
			int32 maxResponseLength;
			if (qobject->TryGetNumberField(TEXT("maxResponseLength"), maxResponseLength))
			{
				q.maxResponseLength = maxResponseLength;
			}

			//scale
			FString minLabel;
			if (qobject->TryGetStringField(TEXT("minLabel"), minLabel))
			{
				q.minLabel = minLabel;
			}
			FString maxLabel;
			if (qobject->TryGetStringField(TEXT("maxLabel"), maxLabel))
			{
				q.maxLabel = maxLabel;
			}
			const TSharedPtr<FJsonObject>* range;
			if (qobject->TryGetObjectField(TEXT("range"), range))
			{
				int32 start = 0;
				if (range->Get()->TryGetNumberField(TEXT("start"), start))
				{
					
				}
				int32 end = 10;
				if (range->Get()->TryGetNumberField(TEXT("end"), end))
				{
					
				}
				q.range = FExitPollScaleRange();
				q.range.start = start;
				q.range.end = end;
			}

			
			//multiple choice
			const TArray<TSharedPtr<FJsonValue>>* answers;
			if (qobject->TryGetArrayField(TEXT("answers"), answers))
			{
				for (int32 j = 0; j < answers->Num(); j++)
				{
					FExitPollMultipleChoice choice = FExitPollMultipleChoice();
					choice.answer = (*answers)[j]->AsObject()->GetStringField(TEXT("answer"));
					//choice.icon = (*answers)[j]->AsObject()->GetBoolField(TEXT("icon"));
					q.answers.Add(choice);
				}
			}

			set.questions.Add(q);
		}

		currentSet = set;
		if (r.IsBound())
		{
			r.Execute(currentSet);
		}
	}
	else
	{
		r.Execute(FExitPollQuestionSet());
	}
}

FExitPollQuestionSet ExitPoll::GetCurrentQuestionSet()
{
	return currentSet;
}

void ExitPoll::SendQuestionResponse(FExitPollResponse Responses)
{
	if (cogProvider.Get() == NULL)
	{
		cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	}

	if (!cogProvider.IsValid() || !cogProvider->HasStartedSession())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
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
		answerObject->SetStringField("type",Responses.answers[i].type);
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
	FString url = "https://data.cognitive3d.com/v0/questionSets/"+currentSet.name+"/"+FString::FromInt(currentSet.version)+"/responses";
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
	
	cogProvider.Get()->customevent->Send(FString("c3d.exitpoll"), FVector(0,0,0), properties); //TODO custom event position should be exitpoll panel position

	//then flush transactions
	cogProvider.Get()->FlushEvents();
}

//for debugging responses from exitpoll microservice
void ExitPoll::OnQuestionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!Response.IsValid())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::OnQuestionResponse - No valid Response. Check internet connection");
		return;
	}

	FString UE4Str = "ExitPoll::OnQuestionResponse: " + Response->GetContentAsString();
	cognitivevrapi::CognitiveLog::Info(TCHAR_TO_UTF8(*UE4Str));
}

void ExitPoll::SendQuestionAnswers(const TArray<FExitPollAnswer>& answers)
{
	auto questionSet = GetCurrentQuestionSet();
	FExitPollResponse responses = FExitPollResponse();
	responses.hook = lastHook;
	responses.user = cogProvider->GetUserID();
	responses.questionSetId = questionSet.id;
	responses.sessionId = cogProvider->GetSessionID();
	responses.answers = answers;
	SendQuestionResponse(responses);
}