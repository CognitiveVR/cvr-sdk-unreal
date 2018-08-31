/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_SENSORS_H_
#define COGNITIVEVR_SENSORS_H_

#include "Private/CognitiveVRPrivatePCH.h"
#include "Private/util/util.h"

class FAnalyticsProviderCognitiveVR;

namespace cognitivevrapi
{
	class COGNITIVEVR_API Sensors
	{
	private:
		FAnalyticsProviderCognitiveVR* s;

		TMap<FString, FString> somedatapoints;

		int32 jsonPart = 1;
		int32 sensorDataCount = 0;
		int32 SensorThreshold = 16;

	public:
		Sensors(FAnalyticsProviderCognitiveVR* sp);
		void RecordSensor(FString Name, float value);
		void SendData();

	};
}

#endif  // COGNITIVEVR_SENSORS_H_