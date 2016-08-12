/*
** Copyright (c) 2015 Knetik, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_TUNING_H_
#define COGNITIVEVR_TUNING_H_

#include "CognitiveVRAnalytics.h"
#include "Private/network/network.h"

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

    class Tuning
    {
        friend class Network;
        private:
			CognitiveVR* s;

			//TODO why is this flipping out? somethign about header order?
            //std::map< std::string, std::map<std::string, TuningValue*> > users_value_cache;
            //std::map< std::string, std::map<std::string, TuningValue*> > devices_value_cache;
            long getallval_cache_ttl;

            std::string GetEntityTypeString(EntityType entity_type);
            void CacheValues(std::string entity_id, FJsonObject object, EntityType entity_type, bool getallc = false);

        public:
            Tuning(CognitiveVR* sp, FJsonObject json);
            ~Tuning();

            /** Get all tuning values for an entity. These values are cached according to the kTuningCacheTtl variable.

                @param std::string entity_id
                @param EntityType entity_type

                @return CognitiveVRResponse
                @throws cognitivevr_exception
            */
            void GetAllValues(std::string entity_id, EntityType entity_type);

            /** Get a single tuning value. These values are cached according to the kTuningCacheTtl variable.

                @param std::string name
                @param std::string default_value - Value returned if the CognitiveVR network is not available.
                @param std::string entity_id
                @param EntityType entity_type

                @return CognitiveVRResponse
                @throws cognitivevr_exception
            */
			CognitiveVRResponse GetValue(std::string name, std::string default_value, std::string entity_id, EntityType entity_type);

            /** Record the use of a tuning value.

                @param std::string name
                @param std::string default_value
                @param std::string user_id - Optional.
                @param std::string device_id - Optional.
                @param std::string context -Optional.

                @return CognitiveVRResponse
                @throws cognitivevr_exception
            */
            void RecordValueAsync(NetworkCallback callback, std::string name, std::string default_value, std::string user_id = "", std::string device_id = "", std::string context = "defaultContext");
    };
}
#endif  // COGNITIVEVR_TUNING_H_