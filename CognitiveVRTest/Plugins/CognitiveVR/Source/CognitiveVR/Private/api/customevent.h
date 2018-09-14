/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_TRANSACTION_H_
#define COGNITIVEVR_TRANSACTION_H_

#include "Private/CognitiveVRPrivatePCH.h"
#include "Private/util/util.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"

class FAnalyticsProviderCognitiveVR;

namespace cognitivevrapi
{
	class COGNITIVEVR_API CustomEvent
	{
	private:
		FAnalyticsProviderCognitiveVR* cog;
		TArray<TSharedPtr<FJsonObject>> batchedJson;
		int32 jsonEventPart = 1;
		int32 CustomEventBatchSize = 16;

		int32 AutoTimer = 2;
		int32 MinTimer = 2;
		int32 ExtremeBatchSize = 64;
		int32 LastSendTime = 0;
		FTimerHandle AutoSendHandle;

		TArray<TSharedPtr<FJsonObject>> events;

	public:
		CustomEvent(FAnalyticsProviderCognitiveVR* cvr);

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

		//send all outstanding custom events to Cognitive dashboard
		void SendData();
	};
}
#endif  // COGNITIVEVR_TRANSACTION_H_