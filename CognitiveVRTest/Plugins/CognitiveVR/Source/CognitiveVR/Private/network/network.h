/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

//#include <stdexcept>
#include "CognitiveVR.h"
#include "Private/ExitPoll.h"
#include "Http.h"

class FAnalyticsProviderCognitiveVR;
class HttpInterface;
class FCognitiveExitPollResponse;
struct FExitPollQuestionSet;
struct FExitPollResponse;

//namespace cognitivevrapi
//{
	class Network
	{
	private:
		static FHttpModule* Http;
		static FString Gateway;
		TSharedPtr<FAnalyticsProviderCognitiveVR> cog;

	public:
		Network();

		void NetworkCall(FString suburl, FString contents);

		static void OnExitPollResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
		void NetworkExitPollGetQuestionSet(FString hook, FCognitiveExitPollResponse& response);
		void NetworkExitPollPostResponse(FExitPollQuestionSet currentQuestionSetName, FExitPollResponse Responses);
	};
//}