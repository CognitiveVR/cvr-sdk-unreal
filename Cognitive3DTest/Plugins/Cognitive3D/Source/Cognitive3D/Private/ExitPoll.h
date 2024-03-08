// Fill out your copyright notice in the Description page of Project Settings.

//organizes all the question sets, responses and exit poll panel actors

#pragma once

#include "Cognitive3D/Public/C3DCommonTypes.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Cognitive3D/Public/Cognitive3DSettings.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Private/C3DComponents/PlayerTracker.h"
//#include "ExitPoll.generated.h"



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