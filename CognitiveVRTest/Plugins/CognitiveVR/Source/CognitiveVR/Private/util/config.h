/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_CONFIG_H_
#define COGNITIVEVR_CONFIG_H_

#include <string>
#include "CognitiveVR.h"

namespace cognitivevrapi
{
    class Config {
        public:
			#pragma warning(push)
			#pragma warning(disable:4251) //Disable DLL warning that does not apply in this context.

            //CognitiveVR API host. -- Only enterprise customers need this. Ex: https://data.cognitivevr.com
			static FString kNetworkHost;
			static int32 kNetworkVersion;

            //SSF app. Ex: isos-personalization
            //static std::string kSsfApp;

            //SSF Version. Ex: 4
            //static std::string kSsfVersion;

            //SSF Output. Ex: json
            //static std::string kSsfOutput;

            //Tuning variable cache time to live, measured in milliseconds. Ex. 900000 - 15 Minutes
            //static long kTuningCacheTtl;

            //Network call timeout, measured in seconds. Ex. 5
            //static long kNetworkTimeout;
			#pragma warning(pop)

			
			/*
			//POST dynamics json data to scene explorer             https://data.sceneexplorer.com/dynamics/:sceneId?version=:versionNumber
			FORCEINLINE static FString PostDynamicData(FString sceneid, int32 versionnumber)
			{
				return "https://data.sceneexplorer.com/dynamics/" + sceneid + "?version=" + FString::FromInt(versionnumber);
			}

			//POST gaze json data to scene explorer                 https://data.sceneexplorer.com/gaze/:sceneId?version=:versionNumber
			FORCEINLINE static FString PostGazeData(FString sceneid, int32 versionnumber)
			{
				return "https://data.sceneexplorer.com/gaze/" + sceneid + "?version=" + FString::FromInt(versionnumber);
			}

			//POST event json data to scene explorer                https://data.sceneexplorer.com/events/:sceneId?version=:versionNumber
			FORCEINLINE static FString PostEventData(FString sceneid, int32 versionnumber)
			{
				return "https://data.sceneexplorer.com/events/" + sceneid + "?version=" + FString::FromInt(versionnumber);
			}

			//POST sensor json data to scene explorer               https://data.sceneexplorer.com/sensors/:sceneId?version=:versionNumber
			FORCEINLINE static FString PostSensorData(FString sceneid, int32 versionnumber)
			{
				return "https://data.sceneexplorer.com/sensors/" + sceneid + "?version=" + FString::FromInt(versionnumber);
			}

			//POST used in core initialization, personalization (tuning), data collector https://data.cognitivevr.io
			FORCEINLINE static FString DataHost()
			{
				return "http://data.cognitivevr.io";
			}

			//GET request question set                              https ://api.cognitivevr.io/products :customerID / questionSetHooks / :hookName / questionSet
			FORCEINLINE static FString GetExitPollQuestionSet(FString customerid, FString hookname)
			{
				return "https://api.cognitivevr.io/products/" + customerid + "/questionSetHooks/" + hookname + "/questionSet";
			}

			//POST question set responses                           https ://api.cognitivevr.io/products :customerID / questionSets / :questionSetName / :questionSetVersion / responses
			FORCEINLINE static FString PostExitPollResponses(FString customerid, FString questionsetname, int32 questionsetversion)
			{
				return "https://api.cognitivevr.io/products/" + customerid + "/questionSets/" + questionsetname + "/" + FString::FromInt(questionsetversion) + "/responses";
			}*/
    };
}
#endif  // COGNITIVEVR_CONFIG_H_