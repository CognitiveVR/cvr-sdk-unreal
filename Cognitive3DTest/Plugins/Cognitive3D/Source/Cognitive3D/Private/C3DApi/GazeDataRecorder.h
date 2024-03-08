/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/
#pragma once

//#include "TimerManager.h"
#include "Analytics.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "Cognitive3D/Private/Fixations.h"

class IAnalyticsCognitive3D;
class IAnalyticsProviderCognitive3D;
class UCognitive3DBlueprints;

	class COGNITIVE3D_API FGazeDataRecorder
	{
		friend class IAnalyticsProviderCognitive3D;
		friend class IAnalyticsCognitive3D;		

	private:

		TSharedPtr<IAnalyticsProviderCognitive3D> cog;
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

		FGazeDataRecorder();

	public:
		

		void BuildSnapshot(FVector position, FVector gaze, FRotator rotation, double timestamp, bool didHitFloor, FVector floorHitPos, FString objectId = "");
		void BuildSnapshot(FVector position, FRotator rotation, double timestamp, bool didHitFloor, FVector floorHitPos);

		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonPart; }
		int32 GetDataPoints() { return snapshots.Num(); }
	};