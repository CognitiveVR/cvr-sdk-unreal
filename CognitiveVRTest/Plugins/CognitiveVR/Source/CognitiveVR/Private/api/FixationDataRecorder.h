/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#pragma once

#include "Analytics.h"
#include "CognitiveVR/Public/CognitiveVRProvider.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "CognitiveVR/Private/Fixations.h"

class FAnalyticsCognitiveVR;
class FAnalyticsProviderCognitiveVR;
class UCognitiveVRBlueprints;

	class COGNITIVEVR_API UFixationDataRecorder
	{
		friend class FAnalyticsProviderCognitiveVR;

	private:

		int32 FixationBatchSize = 64;
		int32 AutoTimer = 2;
		int32 MinTimer = 2;
		int32 ExtremeBatchSize = 64;
		float LastSendTime = -60;
		FTimerHandle AutoSendHandle;
		
		TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
		int32 jsonPart = 1;
		int32 CustomEventBatchSize = 16;

		UPROPERTY()
			TArray<FFixation> Fixations;

		UFUNCTION()
		void StartSession();
		UFUNCTION()
		void PreSessionEnd();
		UFUNCTION()
		void PostSessionEnd();

	public:
		UFixationDataRecorder();
		void RecordFixationEnd(const FFixation& data);

		//send all outstanding fixations to Cognitive dashboard
		UFUNCTION()
		void SendData(bool copyDataToCache);

		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonPart; }
		int32 GetDataPoints() { return Fixations.Num(); }
	};