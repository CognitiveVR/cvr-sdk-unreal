/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_SENSORS_H_
#define COGNITIVEVR_SENSORS_H_

#include "Private/CognitiveVRPrivatePCH.h"
#include "Private/util/util.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"

class FAnalyticsProviderCognitiveVR;

namespace cognitivevrapi
{
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
		void SendData();
		
		TMap<FString, float> GetLastSensorValues();

		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonPart; }
	};
}

#endif  // COGNITIVEVR_SENSORS_H_