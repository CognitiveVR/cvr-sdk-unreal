/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/config.h"

using namespace cognitivevrapi;

//Configuration defaults.
std::string Config::kNetworkHost = "https://data2.cognitivevr.io";
std::string Config::kSsfApp = "isos-personalization";
std::string Config::kSsfVersion = "4";
std::string Config::kSsfOutput = "json";
long Config::kTuningCacheTtl = 900000;
long Config::kNetworkTimeout = 5;
