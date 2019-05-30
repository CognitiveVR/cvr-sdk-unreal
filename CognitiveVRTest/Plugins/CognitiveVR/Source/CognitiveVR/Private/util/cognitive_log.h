/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_LOG_H_
#define COGNITIVEVR_LOG_H_

#include <iostream>
#include <ctime>

#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"
//#include "Private/util/config.h"

#include "CognitiveVR.h"

namespace cognitivevrapi
{
    class CognitiveLog
    {
        public:
			static void Init();
			static void DevLog(FString s);
			static void Info(FString s);
            static void Warning(FString s);
            static void Error(FString s);

			static bool DevLogEnabled();
    };
}
#endif  // COGNITIVEVR_LOG_H_