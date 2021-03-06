/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Private/ExitPoll.h"
#include "Http.h"

class FAnalyticsProviderCognitiveVR;
class HttpInterface;
class FCognitiveExitPollResponse;
struct FExitPollQuestionSet;
struct FExitPollResponse;


	class Network
	{
	private:
		FHttpModule* Http;
		FString Gateway;
		TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
		bool hasErrorResponse;

	public:
		Network();

		void NetworkCall(FString suburl, FString contents);
		void OnCallReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
		bool HasErrorResponse();

		void OnExitPollResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
		void NetworkExitPollGetQuestionSet(FString hook, FCognitiveExitPollResponse& response);
		void NetworkExitPollPostResponse(FExitPollQuestionSet currentQuestionSetName, FExitPollResponse Responses);
	};
