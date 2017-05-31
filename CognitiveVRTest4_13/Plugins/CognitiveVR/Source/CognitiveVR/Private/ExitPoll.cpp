// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "ExitPoll.h"
#include "CognitiveVRSettings.h"
#include "Util.h"

FCognitiveExitPollResponse r;
FExitPollQuestionSet currentSet;
FString lastHook;
TSharedPtr<FAnalyticsProviderCognitiveVR> s;

void ExitPoll::MakeQuestionSetRequest(const FString Hook, const FCognitiveExitPollResponse& response)
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	if (s.Get() == NULL)
	{
		s = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	}
	FString ValueReceived = s->CustomerId;// = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRApiKey", false);

	FString url = "https://api.cognitivevr.io/products/"+ ValueReceived +"/questionSetHooks/"+ Hook+"/questionSet";
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("GET");
	r = response;
	lastHook = Hook;
	HttpRequest->OnProcessRequestComplete().BindStatic(ExitPoll::OnResponseReceivedAsync);
	HttpRequest->ProcessRequest();
}

void ExitPoll::OnResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	currentSet = FExitPollQuestionSet();

	FString UE4Str = Response->GetContentAsString();
	std::string content(TCHAR_TO_UTF8(*UE4Str));
	CognitiveLog::Info("ExitPoll::OnResponseReceivedAsync - Response: " + content);

	//CognitiveVRResponse response = Network::ParseResponse(content);

	FExitPollQuestionSet set = FExitPollQuestionSet();
	TSharedPtr<FJsonObject> jobject;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(UE4Str);
	if (FJsonSerializer::Deserialize(Reader, jobject))
	{
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
			if (qobject->TryGetStringField(TEXT("maxLabel"), minLabel))
			{
				q.maxLabel = maxLabel;
			}
			const TSharedPtr<FJsonObject>* range;
			if (qobject->TryGetObjectField(TEXT("range"), range))
			{
				int32 start = 0;
				if (range->Get()->TryGetNumberField(TEXT("maxLabel"), start))
				{
					
				}
				int32 end = 10;
				if (range->Get()->TryGetNumberField(TEXT("maxLabel"), end))
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
					choice.answer = (*answers)[i]->AsObject()->GetStringField(TEXT("answer"));
					choice.icon = (*answers)[i]->AsObject()->GetBoolField(TEXT("icon"));
					q.answers.Add(choice);
				}
			}

			set.questions.Add(q);
		}

		currentSet = set;

		r.Execute(currentSet);
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

void ExitPoll::SendQuestionResponses(FExitPollResponses Responses)
{
	if (s.Get() == NULL)
	{
		s = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	}

	TSharedPtr<FJsonObject> ResponseObject = MakeShareable(new FJsonObject);

	//ResponseObject->SetStringField("user", s->GetUserID());
	//ResponseObject->SetStringField("questionSetId", currentSet.id);
	//ResponseObject->SetStringField("sessionId", s->GetSessionID());
	//ResponseObject->SetStringField("hook", lastHook);

	ResponseObject->SetStringField("user", Responses.user);
	ResponseObject->SetStringField("questionSetId", Responses.questionSetId);
	ResponseObject->SetStringField("sessionId", Responses.sessionId);
	ResponseObject->SetStringField("hook", Responses.hook);

	TArray<TSharedPtr<FJsonValue>> answerValues;

	for (int32 i = 0; i < Responses.answers.Num(); i++)
	{
		TSharedPtr<FJsonObject> answerObject = MakeShareable(new FJsonObject);
		//answerObject->SetStringField("type",Responses.answers[i].type);
		if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Number)
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
		else //skipped. Null
		{
			answerObject->SetField("null", MakeShareable(new FJsonValueNull()));
		}
		TSharedPtr<FJsonValueObject> vo = MakeShareable(new FJsonValueObject(answerObject));
		answerValues.Add(vo);
	}
	ResponseObject->SetArrayField("answers", answerValues);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(ResponseObject.ToSharedRef(), Writer);
	
	//serialize and send this json
	
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	FString ValueReceived = s->CustomerId;// = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRApiKey", false);
	FString url = "https://api.cognitivevr.io/products/" + ValueReceived + "/questionSets/" + currentSet.name + "/" + FString::FromInt(currentSet.version) + "/responses";

	HttpRequest->SetURL(url);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->ProcessRequest();


	//send this as a transaction too

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	//properties->SetStringField("userId", s->GetUserID());
	//properties->SetStringField("questionSetId", currentSet.id);
	//properties->SetStringField("hook", lastHook);

	properties->SetStringField("userId", Responses.user);
	properties->SetStringField("questionSetId", Responses.questionSetId);
	properties->SetStringField("hook", Responses.hook);

	for (int32 i = 0; i < Responses.answers.Num(); i++)
	{
		if (Responses.answers[i].AnswerValueType == EAnswerValueTypeReturn::Number)
		{
			properties->SetNumberField("Answer" + FString::FromInt(i), Responses.answers[i].numberValue);
		}
		else //bool as number
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
	}

	s.Get()->transaction->BeginEnd("cvr.exitpoll", properties);

	//then flush transactions
	s.Get()->FlushEvents();
}

/*FExitPollQuestionSet FExitPoll::GetExitPollQuestionSet(EResponseValueReturn& Out, FString Hook)
{
	return FExitPollQuestionSet();
}

void FExitPoll::SendExitPollResponse(FExitPollResponses FExitPoll)
{

}*/