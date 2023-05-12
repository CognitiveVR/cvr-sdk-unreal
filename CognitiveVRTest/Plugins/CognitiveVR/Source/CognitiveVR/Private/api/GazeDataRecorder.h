/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

//#include "TimerManager.h"
#include "Analytics.h"
#include "CognitiveVR/Public/CognitiveVRProvider.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "CognitiveVR/Private/Fixations.h"

class FAnalyticsCognitiveVR;
class FAnalyticsProviderCognitiveVR;
class UCognitiveVRBlueprints;

	class COGNITIVEVR_API UGazeDataRecorder
	{
		friend class FAnalyticsProviderCognitiveVR;

	private:

		TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
		int32 jsonPart = 1;

		UFUNCTION()
		void StartSession();
		UFUNCTION()
		void PreSessionEnd();
		UFUNCTION()
		void PostSessionEnd();

		TArray< FGazeData> snapshots;
		float LastSendTime = -60;
		int32 GazeBatchSize = 100;

		//send all outstanding gaze data to Cognitive dashboard
		UFUNCTION()
			void SendData(bool copyDataToCache);

	public:
		UGazeDataRecorder();

		void BuildSnapshot(FVector position, FVector gaze, FRotator rotation, double timestamp, bool didHitFloor, FVector floorHitPos, FString objectId = "");
		void BuildSnapshot(FVector position, FRotator rotation, double timestamp, bool didHitFloor, FVector floorHitPos);

		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonPart; }
		int32 GetDataPoints() { return snapshots.Num(); }
	};