/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Private/ExitPoll.h"
#include "CognitiveVR/Private/LocalCache.h"
#include "Http.h"

class FAnalyticsProviderCognitiveVR;
class HttpInterface;
class FCognitiveExitPollResponse;
class LocalCache;
struct FExitPollQuestionSet;
struct FExitPollResponse;


	class Network
	{
	private:
		FHttpModule* Http;
		FString Gateway;
		TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
		bool hasErrorResponse;
		bool isUploadingFromCache = false;
		FHttpRequestPtr localCacheRequest;
		FString TArrayToString(const TArray<uint8> data, int32 count);

		//
		int32 VariableDelayMultiplier = 0;
		float LocalCacheLoopDelay = 2.0f;
		bool IsServerUnreachable = false;
		FTimerHandle TimerHandle;
		FTimerHandle TimerHandleShortDelay;
	public:
		Network();

		void NetworkCall(FString suburl, FString contents, bool copyDataToCache);
		void OnSessionDataResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
		void OnLocalCacheResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
		bool HasErrorResponse();

		void OnExitPollResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
		void NetworkExitPollGetQuestionSet(FString hook, FCognitiveExitPollResponse& response);
		void NetworkExitPollPostResponse(FExitPollQuestionSet currentQuestionSetName, FExitPollResponse Responses);


		void SessionEnd();
		void RequestLocalCache();
		void ResetVariableTimer();
	};
