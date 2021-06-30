/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include <ctime>
#include "Json.h"
#if !PLATFORM_ANDROID
#include "Windows/WindowsPlatformMisc.h"
#endif


    class Util
    {
        public:
            static double GetTimestamp();

			static FString GetDeviceName(FString DeviceName);

			//record several default hardware values to session properties
			static void SetHardwareSessionProperties();
    };
