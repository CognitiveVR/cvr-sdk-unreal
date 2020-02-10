/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#pragma once

#include "CognitiveVR.h"
#include "GenericPlatformDriver.h"
//#include <sstream>
#include <ctime>
//#include <string>
//#include <algorithm>
//#include <functional>
//#include <cctype>
//#include <locale>
#include "Json.h"
#include "Windows/WindowsPlatformMisc.h"

//namespace cognitivevrapi
//{
    class Util
    {
        public:
            static double GetTimestamp();

			static FString GetDeviceName(FString DeviceName);

			//record several default hardware values to session properties
			static void SetHardwareSessionProperties();
    };
//}