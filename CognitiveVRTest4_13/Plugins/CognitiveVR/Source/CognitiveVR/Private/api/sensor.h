/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_SENSORS_H_
#define COGNITIVEVR_SENSORS_H_

#include "Private/CognitiveVRPrivatePCH.h"
#include "Private/util/util.h"

class FAnalyticsProviderCognitiveVR;

class COGNITIVEVR_API Sensors
{
    private:
		FAnalyticsProviderCognitiveVR* s;

		TMap<FString, TArray<TArray<FJsonValueNumber>>> sensorDataJson;
		TMap<FString, FString> somedatapoints;
		
		FString SensorDataToString();
		int jsonPart = 0;
		int sensorDataCount = 0;
		int SensorThreshold = 16;

    public:
		Sensors(FAnalyticsProviderCognitiveVR* sp);
		void RecordSensor(FString Name, float value);
		void SendData();
        
};
#endif  // COGNITIVEVR_SENSORS_H_