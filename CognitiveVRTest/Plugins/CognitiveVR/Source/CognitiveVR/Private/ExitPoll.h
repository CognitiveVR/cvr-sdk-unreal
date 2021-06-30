// Fill out your copyright notice in the Description page of Project Settings.

//organizes all the question sets, responses and exit poll panel actors

#pragma once

#include "CognitiveVR/Public/C3DCommonTypes.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Public/CognitiveVRProvider.h"
#include "CognitiveVR/Public/CognitiveVRSettings.h"
#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Private/PlayerTracker.h"
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