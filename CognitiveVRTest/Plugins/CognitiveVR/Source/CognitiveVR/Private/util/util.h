/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#pragma once

#include "CognitiveVR.h"
#include "GenericPlatformDriver.h"
#include <ctime>
#include "Json.h"
#include "Windows/WindowsPlatformMisc.h"


    class Util
    {
        public:
            static double GetTimestamp();

			static FString GetDeviceName(FString DeviceName);

			//record several default hardware values to session properties
			static void SetHardwareSessionProperties();
    };
