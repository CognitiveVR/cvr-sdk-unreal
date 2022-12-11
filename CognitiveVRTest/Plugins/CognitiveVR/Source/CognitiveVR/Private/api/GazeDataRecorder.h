/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

//#include "TimerManager.h"
#include "Analytics.h"
#include "CognitiveVR/Public/CognitiveVRProvider.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "CognitiveVR/Private/Fixations.h"
#include "GazeDataRecorder.generated.h"

class FAnalyticsCognitiveVR;
class FAnalyticsProviderCognitiveVR;
class UCognitiveVRBlueprints;

UCLASS()
	class COGNITIVEVR_API UGazeDataRecorder : public UObject
	{
		GENERATED_BODY()

	private:

		TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
		int32 jsonPart = 1;

		UFUNCTION()
		void StartSession();
		UFUNCTION()
		void PreSessionEnd();
		UFUNCTION()
		void PostSessionEnd();

		//TODO CRASH possibly getting garbage collected between scenes? can't use UPROPERTY because jsonobject isn't a UCLASS USTRUCT or UENUM
		//TArray<TSharedPtr<FJsonObject>> snapshots;

		UPROPERTY()
			TArray< FGazeData> snapshots;
		float LastSendTime = -60;
		int32 GazeBatchSize = 100;

	public:
		UGazeDataRecorder();
		//call this immediately after creation - sets callbacks and reference to CognitiveVRProvider
		void Initialize();

		void BuildSnapshot(FVector position, FVector gaze, FRotator rotation, double time, bool didHitFloor, FVector floorHitPos, FString objectId = "");
		void BuildSnapshot(FVector position, FRotator rotation, double time, bool didHitFloor, FVector floorHitPos);

		//send all outstanding gaze data to Cognitive dashboard
		UFUNCTION()
		void SendData(bool copyDataToCache);

		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonPart; }
		int32 GetDataPoints() { return snapshots.Num(); }
	};