/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "ExitPoll.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"

//FCognitiveExitPollResponse FExitPoll::lastResponse;
//FExitPollQuestionSet FExitPoll::currentQuestionSet;
//FString FExitPoll::lastHook;
//double FExitPoll::lastStartTime;

FExitPoll::FExitPoll()
{

}

void FExitPoll::MakeQuestionSetRequest(const FString Hook, FCognitiveExitPollResponse& response)
{
	auto cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();
	if (!cogProvider.IsValid() || !cogProvider.Pin()->HasStartedSession())
	{
		FCognitiveLog::Error("FExitPoll::MakeQuestionSetRequest could not get provider!");
		return;
	}
	lastResponse = response;
	lastHook = Hook;
	cogProvider.Pin()->network->NetworkExitPollGetQuestionSet(Hook,response);
}

void FExitPoll::OnResponseReceived(FString ResponseContent, bool successful)
{
	lastStartTime = FUtil::GetTimestamp();
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
	FCognitiveLog::Info("FExitPoll::OnResponseReceived - Response: " + ResponseContent);

	//Cognitive3DResponse response = Network::ParseResponse(content);

	//FExitPollQuestionSet set = FExitPollQuestionSet();
	TSharedPtr<FJsonObject> jobject;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(ResponseContent);
	if (FJsonSerializer::Deserialize(Reader, jobject))
	{
		if (!jobject->HasField(TEXT("customerId")))
		{
			FCognitiveLog::Info("FExitPoll::OnResponseReceivedAsync - no customerId in response - fail");
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

FExitPollQuestionSet FExitPoll::GetCurrentQuestionSet()
{
	return currentQuestionSet;
}

void FExitPoll::SendQuestionResponse(FExitPollResponse Responses)
{
	auto cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();
	if (!cogProvider.IsValid() || !cogProvider.Pin()->HasStartedSession())
	{
		FCognitiveLog::Error("FExitPoll::SendQuestionResponse could not get provider!");
		return;
	}
	cogProvider.Pin()->network->NetworkExitPollPostResponse(currentQuestionSet, Responses);
	currentQuestionSet = FExitPollQuestionSet();
}

void FExitPoll::SendQuestionAnswers(const TArray<FExitPollAnswer>& answers)
{
	auto provider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();
	if (!provider.IsValid())
	{
		return;
	}

	auto cog = provider.Pin();
	auto questionSet = GetCurrentQuestionSet();
	FExitPollResponse responses = FExitPollResponse();
	responses.duration = FUtil::GetTimestamp() - lastStartTime;
	responses.hook = lastHook;
	responses.userId = cog->GetDeviceID();
	responses.participantId = cog->GetUserID();
	responses.questionSetId = questionSet.id;
	responses.sessionId = cog->GetSessionID();
	responses.answers = answers;
	SendQuestionResponse(responses);
}