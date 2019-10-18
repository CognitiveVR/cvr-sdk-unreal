/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_TRANSACTION_H_
#define COGNITIVEVR_TRANSACTION_H_

#include "Private/CognitiveVRPrivatePCH.h"
#include "Private/util/util.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"

class FAnalyticsProviderCognitiveVR;
struct FCustomEvent;

namespace cognitivevrapi
{
	class COGNITIVEVR_API CustomEventRecorder
	{
	private:

		static uint64 lastFrameCount;
		static int32 consecutiveFrame;

		
		TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
		int32 jsonEventPart = 1;
		int32 CustomEventBatchSize = 16;

		int32 AutoTimer = 2;
		int32 MinTimer = 2;
		int32 ExtremeBatchSize = 64;
		float LastSendTime = -60;
		FTimerHandle AutoSendHandle;

		TArray<TSharedPtr<FJsonObject>> events;

		//checks minimum send timer before sending recorded data to dashboard
		void TrySendData();

	public:
		CustomEventRecorder();

		void StartSession(); //IMPROVEMENT should be private and friend class cognitive core. namespace issues

		//record event with name linked to a dynamic object
		void Send(FString category, FString dynamicObjectId);
		//record event with name and properties linked to a dynamic object
		void Send(FString category, TSharedPtr<FJsonObject> properties, FString dynamicObjectId);
		//record event with name at a position linked to a dynamic object
		void Send(FString category, FVector Position, FString dynamicObjectId);
		//record event with name at a position with properties linked to a dynamic object
		void Send(FString category, FVector Position, TSharedPtr<FJsonObject> properties, FString dynamicObjectId);

		//record event with name
		void Send(FString category);
		//record event with name and properties
		void Send(FString category, TSharedPtr<FJsonObject> properties);
		//record event with name at a position
		void Send(FString category, FVector Position);
		//record event with name at a position with properties
		void Send(FString category, FVector Position, TSharedPtr<FJsonObject> properties);

		void Send(FCustomEvent* customEvent);

		//send all outstanding custom events to Cognitive dashboard
		void SendData();

		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonEventPart; }
	};
}
#endif  // COGNITIVEVR_TRANSACTION_H_