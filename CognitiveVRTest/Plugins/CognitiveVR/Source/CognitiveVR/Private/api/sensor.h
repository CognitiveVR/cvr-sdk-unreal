/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#pragma once

#include "CognitiveVR/Public/CognitiveVR.h"
#include "TimerManager.h"
#include "CognitiveVR/Private/util/util.h"
#include "CoreMinimal.h"

class FAnalyticsProviderCognitiveVR;
class UCognitiveVRBlueprints;

	class COGNITIVEVR_API Sensors
	{
	private:
		TSharedPtr<FAnalyticsProviderCognitiveVR> cog;

		//Q: why is a map of string values instead of floats? A: because formatting it in json
		TMap<FString, FString> SensorDataPoints;

		int32 jsonPart = 1;
		int32 sensorDataCount = 0;
		int32 SensorThreshold = 16;

		int32 AutoTimer = 2;
		int32 MinTimer = 2;
		int32 ExtremeBatchSize = 64;
		float LastSendTime = -60;
		FTimerHandle AutoSendHandle;

		//checks minimum send timer before sending recorded data to dashboard
		void TrySendData();

		TMap<FString, float> LastSensorValues;

	public:
		Sensors();
		void StartSession();
		void RecordSensor(FString Name, float value);
		void RecordSensor(FString Name, double value);
		void SendData();
		
		TMap<FString, float> GetLastSensorValues();

		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonPart; }
		int32 GetDataPoints() { return sensorDataCount; }
	};
