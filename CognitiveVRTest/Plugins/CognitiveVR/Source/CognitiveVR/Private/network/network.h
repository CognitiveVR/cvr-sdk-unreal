/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "CognitiveVR.h"
#include "Private/ExitPoll.h"
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

	public:
		Network();

		void NetworkCall(FString suburl, FString contents);

		void OnExitPollResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
		void NetworkExitPollGetQuestionSet(FString hook, FCognitiveExitPollResponse& response);
		void NetworkExitPollPostResponse(FExitPollQuestionSet currentQuestionSetName, FExitPollResponse Responses);
	};
