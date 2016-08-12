/*
** Copyright (c) 2015 Knetik, Inc. All rights reserved.
*/
#include "api/tuning.h"

namespace cognitivevrapi
{
    Tuning::Tuning(CognitiveVR* sp, FJsonObject json)
    {
        s = sp;

		//TODO cache values
        //CacheValues(sp->user_id, json.GetObjectField("usertuning"), kEntityTypeUser, true);
        //CacheValues(sp->device_id, json.GetObjectField("devicetuning"), kEntityTypeDevice, true);
    }

    Tuning::~Tuning()
    {
        cognitivevrapi::Log::Info("Freeing tuning memory.");
    }

    void Tuning::CacheValues(std::string entity_id, FJsonObject values, EntityType entity_type, bool getallc)
    {
        if (Config::kTuningCacheTtl <= 0) {
            cognitivevrapi::ThrowDummyResponseException("Invalid tuning cache TTL.");
        }

        long cache_time = Util::GetTimestamp() + Config::kTuningCacheTtl;

        if (getallc) {
            Tuning::getallval_cache_ttl = cache_time;
        }

		Log::Info("CACHING TUNING VALUES: " + entity_id + ":" + Tuning::GetEntityTypeString(entity_type));// +" " + values.toStyledString());

		/*for (TObjectIterator<TMap<FString, TSharedPtr<FJsonValue>>> Itr; Itr; ++Itr)
		{
			//TODO iterate through values;
		}*/
		/*
        for(FJsonObject::iterator it = values.begin(); it != values.end(); ++it)
        {
			FJsonObject value = (*it);
            std::string skey = it.key().asString();

            TuningValue* tval = new TuningValue(value.asString(), cache_time);
            if(entity_type == kEntityTypeUser) {
                users_value_cache[entity_id][skey] = tval;
            } else {
                devices_value_cache[entity_id][skey] = tval;
            }
        }*/
    }

    void Tuning::GetAllValues(std::string entity_id, EntityType entity_type)
    {
		/*
        long curtime = Util::GetTimestamp();

        if (curtime <= Tuning::getallval_cache_ttl) {

            bool entvalid = false;
            CognitiveVRResponse resp(true);
			FJsonObject root;
            if (entity_type == kEntityTypeUser) {
                if (users_value_cache.count(entity_id)) {
                    entvalid = true;
                    std::map<std::string, TuningValue*> entcache = users_value_cache.find(entity_id)->second;
                    for(std::map<std::string, TuningValue*>::iterator it = entcache.begin(); it != entcache.end(); it++) {
                        root[it->first] = it->second->GetValue();
                    }
                }
            } else {
                if (devices_value_cache.count(entity_id)) {
                    entvalid = true;
                    std::map<std::string, TuningValue*> entcache = devices_value_cache.find(entity_id)->second;
                    for(std::map<std::string, TuningValue*>::iterator it = entcache.begin(); it != entcache.end(); it++) {
                        root[it->first] = it->second->GetValue();
                    }
                }
            }

            if (entvalid) {
                Log::Info("Get All Value cache has NOT expired and entity already exists.");
                resp.SetContent(root);

                Log::Info("GET: " + Tuning::GetEntityTypeString(entity_type) + " " + resp.GetContent().toStyledString());
                return resp;
            }
        }
		*/

		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));
		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;
		std::string entityType = Tuning::GetEntityTypeString(entity_type);

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, entity_id);
		Util::AppendToJsonArray(jsonArray, entityType);

        Log::Info("Get All Value cache has expired or the entity does not exist.");
		
		CognitiveVRResponse resp = s->GetNetwork()->Call("tuner_getAllValues", jsonArray);
		//resp.SetContent(resp.GetContent().GetObjectField("value").Get);// ["value"]);
        CacheValues(entity_id, resp.GetContent(), entity_type, true);
    }

	CognitiveVRResponse Tuning::GetValue(std::string name, std::string default_value, std::string entity_id, EntityType entity_type)
    {
		
        long curtime = Util::GetTimestamp();
        TuningValue* v = NULL;
        /*
		if (entity_type == kEntityTypeUser) {
            if (users_value_cache.count(entity_id)) {
                std::map<std::string, TuningValue*> entcache = users_value_cache.find(entity_id)->second;
                if (entcache.count(name)) {
                    v = entcache.find(name)->second;
                }
            }
        } else {
            if (devices_value_cache.count(entity_id)) {
                std::map<std::string, TuningValue*> entcache = devices_value_cache.find(entity_id)->second;
                if (entcache.count(name)) {
                    v = entcache.find(name)->second;
                }
            }
        }
        long ttl = 0L;

        if (v != NULL) {
            ttl = v->GetTtl();
        }

        if (curtime >= ttl) {
            Log::Info("Value cache has expired.");

			FJsonObject json;

            std::string ts = Util::GetTimestampStr();
            json.append(ts);
            json.append(ts);
            json.append(name);
            json.append(entity_id);
            std::string entity_type_string = Tuning::GetEntityTypeString(entity_type);
            json.append(entity_type_string);

            CognitiveVRResponse resp(false);
            try {
                resp = s->GetNetwork()->Call("tuner_getValue", json);
            } catch(cognitivevrapi::cognitivevr_exception e) {
                Log::Error("Get Tuning Value Fail(Returning Default): " + e.GetResponse().GetErrorMessage());

                CognitiveVRResponse dresp(true);
				FJsonObject content;
                content[name] = default_value;
                dresp.SetContent(content);
                return dresp;
            }

            resp.SetContent(resp.GetContent()["value"]);
            if (resp.GetContent() == FJsonObject::null) {
                cognitivevrapi::ThrowDummyResponseException("Tuning variable '" + name + "' for " + entity_type_string + " \"" + entity_id + "\" does not exist.");
                return resp;
            }

            CacheValues(entity_id, resp.GetContent(), entity_type);

            return resp;
        }
		*/
        Log::Info("Value cache has NOT expired.");
		CognitiveVRResponse resp(true);

		FJsonObject root;
		root.SetStringField(name.c_str(), v->GetValue().c_str());
        //root[name] = v->GetValue();
        resp.SetContent(root);

		//Log::Info("GET: " + Tuning::GetEntityTypeString(entity_type) + " " + resp.GetContent().Values.);
        //Log::Info("GET: " + Tuning::GetEntityTypeString(entity_type) + " " + resp.GetContent().toStyledString());


        return resp;
    }

    void Tuning::RecordValueAsync(NetworkCallback callback, std::string name, std::string default_value, std::string user_id, std::string device_id, std::string ncontext)
    {
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		s->AppendUD(jsonArray, user_id.c_str(), device_id.c_str());
		Util::AppendToJsonArray(jsonArray, name);
		Util::AppendToJsonArray(jsonArray, default_value);

        s->thread_manager->PushTask(callback, "tuner_recordUsed", jsonArray, ncontext);
    }

    std::string Tuning::GetEntityTypeString(EntityType entity_type)
    {
        switch (entity_type) {
            case kEntityTypeUser:
                return "USER";
            case kEntityTypeDevice:
                return "DEVICE";
        }
        return "UNKNOWN";
    }
}