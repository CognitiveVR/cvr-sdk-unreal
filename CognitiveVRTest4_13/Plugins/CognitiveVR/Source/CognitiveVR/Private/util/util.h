/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_UTIL_H_
#define COGNITIVEVR_UTIL_H_

#include "CognitiveVR.h"
#include <sstream>
#include <ctime>
#include <string>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include "Json.h"
#include "Windows/WindowsPlatformMisc.h"

namespace cognitivevrapi
{
    class Util
    {
        public:
            static double GetTimestamp();
			static long GetTimestampLong();
            static std::string GetTimestampStr(long t = 0);
            template < typename T >
            static std::string ToString( const T& n );

            static inline std::string &LTrim(std::string &s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
                return s;
            }

            static inline std::string &RTrim(std::string &s) {
                s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
                return s;
            }

            static inline std::string &Trim(std::string &s) {
                return LTrim(RTrim(s));
            }

			static FString GetDeviceName(FString DeviceName);

			static TSharedPtr<FJsonObject> DeviceScraper(TSharedPtr<FJsonObject> properties);

			static void AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, FString &fstring);
			static void AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, std::string &string);
			static void AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, long &longValue);
			static void AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, bool &mybool);
			static void AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, int &intValue);
			static void AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, double &doubleValue);
			static void AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, TSharedPtr<FJsonObject> &object);

			static void AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, std::string &stdstring);
			static void AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, FString &fstring);
			static void AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, bool &mybool);
			static void AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, int &intValue);
			static void AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, double &doubleValue);
    };
}
#endif  // COGNITIVEVR_UTIL_H_