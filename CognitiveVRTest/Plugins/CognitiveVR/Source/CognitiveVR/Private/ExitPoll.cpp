// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "ExitPoll.h"
#include "CognitiveVRSettings.h"
#include "Util.h"

FCognitiveExitPollResponse ExitPoll::lastResponse;
FExitPollQuestionSet ExitPoll::currentQuestionSet;
FString ExitPoll::lastHook;
double ExitPoll::lastStartTime;

void ExitPoll::MakeQuestionSetRequest(const FString Hook, FCognitiveExitPollResponse& response)
{
	auto cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cogProvider.IsValid() || !cogProvider.Pin()->HasStartedSession())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::MakeQuestionSetRequest could not get provider!");
		return;
	}
	lastResponse = response;
	lastHook = Hook;
	cogProvider.Pin()->network->NetworkExitPollGetQuestionSet(Hook,response);
}

void ExitPoll::OnResponseReceived(FString ResponseContent, bool successful)
{
	lastStartTime = cognitivevrapi::Util::GetTimestamp();
	if (!successful)
	{
		if (lastResponse.IsBound())
		{
			lastResponse.Execute(FExitPollQuestionSet());
		}
		return;
	}


	currentQuestionSet = FExitPollQuestionSet();

	//FString UE4Str = Response->GetContentAsString();
	cognitivevrapi::CognitiveLog::Info("ExitPoll::OnResponseReceived - Response: " + ResponseContent);

	//CognitiveVRResponse response = Network::ParseResponse(content);

	//FExitPollQuestionSet set = FExitPollQuestionSet();
	TSharedPtr<FJsonObject> jobject;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(ResponseContent);
	if (FJsonSerializer::Deserialize(Reader, jobject))
	{
		if (!jobject->HasField(TEXT("customerId")))
		{
			cognitivevrapi::CognitiveLog::Info("ExitPoll::OnResponseReceivedAsync - no customerId in response - fail");
			if (lastResponse.IsBound())
			{
				lastResponse.Execute(FExitPollQuestionSet());
			}
			return;
		}

		currentQuestionSet.customerId = jobject->GetStringField(TEXT("customerId"));
		currentQuestionSet.id = jobject->GetStringField(TEXT("id"));
		currentQuestionSet.name = jobject->GetStringField(TEXT("name"));
		currentQuestionSet.version = jobject->GetNumberField(TEXT("version"));
		currentQuestionSet.title = jobject->GetStringField(TEXT("title"));
		currentQuestionSet.status = jobject->GetStringField(TEXT("status"));

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

			currentQuestionSet.questions.Add(q);
		}

		if (lastResponse.IsBound())
		{
			lastResponse.Execute(currentQuestionSet);
		}
	}
	else
	{
		lastResponse.Execute(FExitPollQuestionSet());
	}
}

FExitPollQuestionSet ExitPoll::GetCurrentQuestionSet()
{
	return currentQuestionSet;
}

void ExitPoll::SendQuestionResponse(FExitPollResponse Responses)
{
	auto cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cogProvider.IsValid() || !cogProvider.Pin()->HasStartedSession())
	{
		cognitivevrapi::CognitiveLog::Error("ExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	cogProvider.Pin()->network->NetworkExitPollPostResponse(currentQuestionSet, Responses);
	currentQuestionSet = FExitPollQuestionSet();
}

void ExitPoll::SendQuestionAnswers(const TArray<FExitPollAnswer>& answers)
{
	auto provider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	auto questionSet = GetCurrentQuestionSet();
	FExitPollResponse responses = FExitPollResponse();
	responses.duration = cognitivevrapi::Util::GetTimestamp() - lastStartTime;
	responses.hook = lastHook;
	responses.user = provider.Pin()->GetUserID();
	responses.questionSetId = questionSet.id;
	responses.sessionId = provider.Pin()->GetSessionID();
	responses.answers = answers;
	SendQuestionResponse(responses);
}