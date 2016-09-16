/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_TUNING_H_
#define COGNITIVEVR_TUNING_H_

#include "Private/CognitiveVRPrivatePCH.h"
#include "Private/network/network.h"

#include "AllowWindowsPlatformTypes.h" 
#include <windows.h>
#include <map>
#include "HideWindowsPlatformTypes.h"
#include "Private/util/cognitive_log.h"
#include "Private/util/util.h"
#include "Private/unreal/buffer_manager.h"
//using namespace cognitivevrapi;

class FAnalyticsProviderCognitiveVR;

namespace cognitivevrapi
{
	enum EntityType {
		kEntityTypeUser,
		kEntityTypeDevice
	};

	class TuningValue
	{
	private:
		std::string value;
		long ttl;

	public:
		TuningValue(std::string val, long time)
		{
			value = val;
			ttl = time;
		}

		long GetTtl()
		{
			return ttl;
		}

		std::string GetValue()
		{
			return value;
		}
	};
}

class COGNITIVEVR_API Tuning
{
	friend class Network;

    private:
		FAnalyticsProviderCognitiveVR* s;

        std::map< std::string, std::map<std::string, cognitivevrapi::TuningValue*> > users_value_cache;
        std::map< std::string, std::map<std::string, cognitivevrapi::TuningValue*> > devices_value_cache;
        long getallval_cache_ttl;

        std::string GetEntityTypeString(cognitivevrapi::EntityType entity_type);
        void CacheValues(std::string entity_id, FJsonObject object, cognitivevrapi::EntityType entity_type, bool getallc = false);

    public:
        Tuning(FAnalyticsProviderCognitiveVR* sp, FJsonObject json);
        //~Tuning();

        /** Get all tuning values for an entity. These values are cached according to the kTuningCacheTtl variable.

            @param std::string entity_id
            @param EntityType entity_type

            @return CognitiveVRResponse
            @throws cognitivevr_exception
        */
        void GetAllValues(std::string entity_id, cognitivevrapi::EntityType entity_type);

        /** Get a single tuning value. These values are cached according to the kTuningCacheTtl variable.

            @param std::string name
            @param std::string default_value - Value returned if the CognitiveVR network is not available.
            @param std::string entity_id
            @param EntityType entity_type

            @return CognitiveVRResponse
            @throws cognitivevr_exception
        */
		CognitiveVRResponse GetValue(std::string name, std::string default_value, std::string entity_id, cognitivevrapi::EntityType entity_type);


		CognitiveVRResponse GetValue(std::string name, std::string default_value);

        /** Record the use of a tuning value.

            @param std::string name
            @param std::string default_value
            @param std::string user_id - Optional.
            @param std::string device_id - Optional.
            @param std::string context -Optional.

            @return CognitiveVRResponse
            @throws cognitivevr_exception
        */
        void RecordValueAsync(NetworkCallback callback, std::string name, std::string default_value, std::string user_id = "", std::string device_id = "");
};
//}
#endif  // COGNITIVEVR_TUNING_H_