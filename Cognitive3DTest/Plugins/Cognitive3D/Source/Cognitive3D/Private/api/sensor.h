/*
** Copyright (c) 2016 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Public/Cognitive3D.h"
#include "TimerManager.h"
#include "Cognitive3D/Private/util/util.h"
#include "CoreMinimal.h"

class FAnalyticsProviderCognitive3D;
class UCognitive3DBlueprints;


	class COGNITIVE3D_API USensors
	{
		friend class FAnalyticsProviderCognitive3D;
		friend class FAnalyticsCognitive3D;

	private:
		TSharedPtr<FAnalyticsProviderCognitive3D> cog;

		//TODO merge these maps
		//Q: why is a map of string values instead of floats? A: format immediately to "[time,value]" instead of holding the 2 values
		//CONSIDER saving float and serializing on another thread just before sending
		TMap<FString, TArray<FString>> SensorDataPoints;
		TMap<FString, float> LastSensorValues;
		TMap<FString, SensorData*> sensorData;

		int32 jsonPart = 1;
		int32 sensorDataCount = 0;
		int32 SensorThreshold = 64;

		int32 AutoTimer = 2;
		float LastSendTime = -60;
		FTimerHandle AutoSendHandle;

		UFUNCTION()
		void StartSession();
		UFUNCTION()
		void PreSessionEnd();
		UFUNCTION()
		void PostSessionEnd();

		USensors();

	public:
		
		void InitializeSensor(FString sensorName, float hzRate = 10, float initialValue = 0);
		void RecordSensor(FString Name, float value);
		void RecordSensor(FString Name, double value);
		UFUNCTION()
		void SendData(bool copyDataToCache);
		TMap<FString, float> GetLastSensorValues();
		float GetLastSendTime() { return LastSendTime; }
		int32 GetPartNumber() { return jsonPart; }
		int32 GetDataPoints() { return sensorDataCount; }
	};
