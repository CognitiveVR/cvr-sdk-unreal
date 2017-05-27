// Fill out your copyright notice in the Description page of Project Settings.

//organizes all the question sets, responses and exit poll panel actors

#pragma once

//#include "Engine.h"
//#include "Kismet/BlueprintFunctionLibrary.h"
//#include "ExitPoll.generated.h"
//#include "CognitiveVRBlueprints.h"
#include "PlayerTracker.h"
#include "Http.h"

class COGNITIVEVR_API ExitPoll
{
private:

public:
	
	//static FExitPollQuestionSet GetQuestionSet(FString hookName);
	
	static FCognitiveExitPollResponse& ResponseDelegate;

	static void MakeQuestionSetRequest(const FString Hook, const FCognitiveExitPollResponse& response);

	static void SendQuestionResponses(FExitPollResponses responses);

	static void OnResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	static FExitPollQuestionSet GetCurrentQuestionSet();
	
};