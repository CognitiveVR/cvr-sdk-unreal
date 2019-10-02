/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_NETWORK_H_
#define COGNITIVEVR_NETWORK_H_

#include <stdexcept>
#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"
#include "Http.h"
#include "ExitPoll.h"
#include "network.h"

class FAnalyticsProviderCognitiveVR;
class HttpInterface;

namespace cognitivevrapi
{
	class Network
	{
	private:
		FAnalyticsProviderCognitiveVR* s;

		static FHttpModule* Http;
		static FString Gateway;

	public:
		Network(FAnalyticsProviderCognitiveVR* sp);

		void NetworkCall(FString suburl, FString contents);

		static void OnExitPollResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
		void NetworkExitPollGetQuestionSet(FString hook, FCognitiveExitPollResponse& response);
		void NetworkExitPollPostResponse(FExitPollQuestionSet currentQuestionSetName, FExitPollResponse Responses);
	};
}
#endif  // COGNITIVEVR_NETWORK_H_