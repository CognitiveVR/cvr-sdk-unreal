/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GeneralProjectSettings.h" //used to get project version
#include "Misc/App.h" //used to get project name
#include <ctime>
#include "Json.h"
#if PLATFORM_ANDROID
#include "Android/AndroidPlatformMisc.h"
#else
#include "Windows/WindowsPlatformMisc.h"
#endif


    class FUtil
    {
        public:
            static double GetTimestamp();

			static FString GetDeviceName(FString DeviceName);

			//record several default hardware values to session properties
			static void SetSessionProperties();

            static FString GetSpecificHMDFromHardware(FString GPUBrand, FString HMDDeviceName, FString CPUChipset, int32 DeviceMemory);

            static FString GetHMDDeviceName();

            static UWorld* GetCurrentActiveWorld();

    private:
        static FString HMDSpecificDeviceName;
    };
