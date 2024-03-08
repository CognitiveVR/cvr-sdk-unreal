/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/
#pragma once

#include "Analytics.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "Cognitive3D/Private/Fixations.h"

class IAnalyticsCognitive3D;
class IAnalyticsProviderCognitive3D;
class UCognitive3DBlueprints;

	class COGNITIVE3D_API FFixationDataRecorder
	{
		friend class IAnalyticsProviderCognitive3D;
		friend class IAnalyticsCognitive3D;

	private:

		int32 FixationBatchSize = 64;
		int32 AutoTimer = 2;
		float LastSendTime = -60;
		FTimerHandle AutoSendHandle;
		
		TSharedPtr<IAnalyticsProviderCognitive3D> cog;
		int32 jsonPart = 1;

		UPROPERTY()
			TArray<FFixation> Fixations;

		UFUNCTION()
		void StartSession();
		UFUNCTION()
		void PreSessionEnd();
		UFUNCTION()
		void PostSessionEnd();

	public:
		FFixationDataRecorder();
		void RecordFixationEnd(const FFixation& data);

		//send all outstanding fixations to Cognitive dashboard
		UFUNCTION()
		void SendData(bool copyDataToCache);

		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonPart; }
		int32 GetDataPoints() { return Fixations.Num(); }
	};