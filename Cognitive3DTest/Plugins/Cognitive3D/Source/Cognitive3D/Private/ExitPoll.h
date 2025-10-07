/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

//organizes all the question sets, responses and exit poll panel actors

#pragma once

#include "Cognitive3D/Public/C3DCommonTypes.h"

class COGNITIVE3D_API FExitPoll
{
	//class FCognitiveExitPollResponse;
	//class FExitPollQuestionSet;

private:

	double lastStartTime;
	FCognitiveExitPollResponse lastResponse;
	FExitPollQuestionSet currentQuestionSet;
	FString lastHook;

public:
	FExitPoll();
	FCognitiveExitPollResponse OnExitPollResponse;
	FCognitiveExitPollResponse ResponseDelegate;

	void MakeQuestionSetRequest(const FString Hook, FCognitiveExitPollResponse& response);
	void SendQuestionResponse(FExitPollResponse responses);
	void SendQuestionAnswers(const TArray<FExitPollAnswer>& Answers);
	void OnResponseReceived(FString ResponseContent, bool successful);
	FExitPollQuestionSet GetCurrentQuestionSet();
	
};