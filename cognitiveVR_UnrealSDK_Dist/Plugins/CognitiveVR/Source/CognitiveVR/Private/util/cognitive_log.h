/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_LOG_H_
#define COGNITIVEVR_LOG_H_

#include <iostream>
#include <ctime>

#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"
#include "Private/util/config.h"

#include "CognitiveVR.h"

namespace cognitivevrapi
{
    class CognitiveLog
    {
        public:
            static void Info(std::string s, bool newline = true);
            static void Warning(std::string s, bool newline = true);
            static void Error(std::string s, bool newline = true);
    };
}
#endif  // COGNITIVEVR_LOG_H_