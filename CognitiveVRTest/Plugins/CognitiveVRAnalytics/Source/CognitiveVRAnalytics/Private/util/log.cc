/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/log.h"

namespace cognitivevrapi
{
    void Log::LogString(std::string s, bool newline)
    {
        std::string nstring = "[" + Log::CurrentDateTime() + "] " + s;
        std::cout << nstring;
        if (newline) {
            std::cout << "\n";
        }
    }

    void Log::Info(std::string s, bool newline)
    {
        if (Config::kDebugLog) {
            LogString("INFO: " + s, newline);
        }
    }

    void Log::Warn(std::string s, bool newline)
    {
        if (Config::kDebugLog) {
            LogString("WARN: " + s, newline);
        }
    }

    void Log::Error(std::string s, bool newline)
    {
        if (Config::kDebugLog) {
            LogString("ERROR: " + s, newline);
        }
    }
}