/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/
#pragma once

#include "Cognitive3D/Public/Cognitive3D.h"
#include "Cognitive3D/Private/ExitPoll.h"
#include "Cognitive3D/Private/LocalCache.h"
#include "Http.h"

class FAnalyticsProviderCognitive3D;
class HttpInterface;
class FCognitiveExitPollResponse;
class FLocalCache;
struct FExitPollQuestionSet;
struct FExitPollResponse;


	class FNetwork
	{
	private:
		FHttpModule* Http;
		FString Gateway;
		TSharedPtr<FAnalyticsProviderCognitive3D> cog;
		bool hasErrorResponse;
		bool isUploadingFromCache = false;
		FHttpRequestPtr localCacheRequest;
		FString TArrayToString(const TArray<uint8> data, int32 count);

		//
		int32 VariableDelayMultiplier = 0;
		int32 VariableDelayTime = 60;
		float LocalCacheLoopDelay = 2.0f;
		bool IsServerUnreachable = false;
		FTimerHandle TimerHandle;
		FTimerHandle TimerHandleShortDelay;
	public:
		FNetwork();

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
