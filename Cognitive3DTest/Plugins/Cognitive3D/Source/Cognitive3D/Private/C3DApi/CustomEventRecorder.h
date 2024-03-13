/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/
#pragma once

//#include "TimerManager.h"
#include "Analytics.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
//#include "Cognitive3D/Private/C3DUtil/Util.h"
//#include "Cognitive3D/Private/C3DComponents/PlayerTracker.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"

class FAnalyticsCognitive3D;
class FAnalyticsProviderCognitive3D;
class UCustomEvent;
class UCognitive3DBlueprints;


	class COGNITIVE3D_API FCustomEventRecorder
	{
		friend class FAnalyticsProviderCognitive3D;
		friend class FAnalyticsCognitive3D;

	private:

		uint64 lastFrameCount = 0;
		int32 consecutiveFrame = 0;
		
		TSharedPtr<FAnalyticsProviderCognitive3D> cog;
		int32 jsonEventPart = 1;
		int32 CustomEventBatchSize = 64;
		int32 AutoTimer = 2;
		float LastSendTime = -60;
		FTimerHandle AutoSendHandle;

		TArray<TSharedPtr<FJsonObject>> events;

		UFUNCTION()
		void StartSession();
		UFUNCTION()
		void PreSessionEnd();
		UFUNCTION()
		void PostSessionEnd();

		bool HasInitialized = false;
		FCustomEventRecorder();

	public:
		

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

		void Send(UCustomEvent* customEvent);

		//send all outstanding custom events to Cognitive dashboard
		UFUNCTION()
		void SendData(bool copyDataToCache);

		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonEventPart; }
		int32 GetDataPoints() { return events.Num(); }
	};