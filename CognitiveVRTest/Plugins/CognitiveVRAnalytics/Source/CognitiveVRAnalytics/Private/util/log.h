/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_LOG_H_
#define COGNITIVEVR_LOG_H_

#include <iostream>
#include <ctime>
#include <string>

#include "Private/util/config.h"

namespace cognitivevrapi
{
    class Log
    {
        private:
            static void LogString(std::string s, bool newline);
			#pragma warning(push)
			#pragma warning(disable:4996) //Disable warning regarding use of localtime_s, this function is platform specific.
            static std::string CurrentDateTime() {
                time_t     now = time(0);
                struct tm  tstruct;
                char       buf[80];
                tstruct = *localtime(&now);
                strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

                return buf;
            }
			#pragma warning(pop)

        public:
            static void Info(std::string s, bool newline = true);
            static void Warn(std::string s, bool newline = true);
            static void Error(std::string s, bool newline = true);
    };
}
#endif  // COGNITIVEVR_LOG_H_