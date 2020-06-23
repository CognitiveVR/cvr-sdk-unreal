// Fill out your copyright notice in the Description page of Project Settings.

//organizes all the question sets, responses and exit poll panel actors

#pragma once

#include "CommonTypes.h"
#include "CognitiveVR.h"
#include "CognitiveVRProvider.h"
#include "CognitiveVRSettings.h"
#include "Private/util/util.h"
#include "PlayerTracker.h"
//#include "ExitPoll.generated.h"



class COGNITIVEVR_API ExitPoll
{
	//class FCognitiveExitPollResponse;
	//class FExitPollQuestionSet;

private:

	double lastStartTime;
	FCognitiveExitPollResponse lastResponse;
	FExitPollQuestionSet currentQuestionSet;
	FString lastHook;

public:
	ExitPoll();
	FCognitiveExitPollResponse OnExitPollResponse;
	FCognitiveExitPollResponse ResponseDelegate;

	void MakeQuestionSetRequest(const FString Hook, FCognitiveExitPollResponse& response);
	void SendQuestionResponse(FExitPollResponse responses);
	void SendQuestionAnswers(const TArray<FExitPollAnswer>& Answers);
	void OnResponseReceived(FString ResponseContent, bool successful);
	FExitPollQuestionSet GetCurrentQuestionSet();
	
};