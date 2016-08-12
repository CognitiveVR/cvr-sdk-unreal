/*
** Copyright (c) 2015 Knetik, Inc. All rights reserved.
*/
#include "api/transaction.h"
#include "util/util.h"

namespace cognitivevrapi
{
    Transaction::Transaction(CognitiveVR* sp)
    {
        s = sp;
    }

    void Transaction::BeginAsync(NetworkCallback callback, std::string transaction_id, std::string category, int timeout, std::string context, TSharedPtr<FJsonObject> properties, FString user_id, FString device_id)
    {
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString trans = FString("TXN");

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		s->AppendUD(jsonArray, user_id, device_id);
		Util::AppendToJsonArray(jsonArray, category);
		Util::AppendToJsonArray(jsonArray, trans);
		Util::AppendToJsonArray(jsonArray, timeout);
		Util::AppendToJsonArray(jsonArray, transaction_id);
		Util::AppendToJsonArray(jsonArray, properties);
		
		//TODO proper deserialize this. atm, this doesn't support nested objects - pretty sure?
		TArray<FString> keys;
		/*for (int i = 0; i < properties.Values.GetKeys(keys); i++)
		{
			json.SetStringField(keys[i],properties.GetStringField(keys[i]));
		}*/

		//json.SetObjectField("data", properties.GetObjectField("data"));

        s->thread_manager->PushTask(callback, "datacollector_beginTransaction", jsonArray, context);
    }

    void Transaction::UpdateAsync(NetworkCallback callback, std::string transaction_id, std::string category, double progress, std::string context, TSharedPtr<FJsonObject> properties, FString user_id, FString device_id)
    {
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		s->AppendUD(jsonArray, user_id, device_id);
		Util::AppendToJsonArray(jsonArray, category);
		Util::AppendToJsonArray(jsonArray, progress);
		Util::AppendToJsonArray(jsonArray, transaction_id);
		Util::AppendToJsonArray(jsonArray, properties);

        s->thread_manager->PushTask(callback, "datacollector_updateTransaction", jsonArray, context);
    }

    void Transaction::EndAsync(NetworkCallback callback, std::string transaction_id, std::string category, std::string result, std::string context, TSharedPtr<FJsonObject> properties, FString user_id, FString device_id)
    {
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		s->AppendUD(jsonArray, user_id, device_id);
		Util::AppendToJsonArray(jsonArray, category);
		Util::AppendToJsonArray(jsonArray, result);
		Util::AppendToJsonArray(jsonArray, transaction_id);
		Util::AppendToJsonArray(jsonArray, properties);

		FString OutputString;
		TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(properties.ToSharedRef(), Writer);

        s->thread_manager->PushTask(callback, "datacollector_endTransaction", jsonArray, context);
    }

    void Transaction::BeginEndAsync(NetworkCallback callback, std::string transaction_id, std::string category, std::string result, std::string context, TSharedPtr<FJsonObject> properties, FString user_id, FString device_id)
    {
		//isntead of NULL, was FJsonObject::null. previously Value::null
        this->BeginAsync(NULL, transaction_id, category, 1000, context, properties, user_id, device_id);
        this->EndAsync(callback, transaction_id, category, result, context, properties, user_id, device_id);
    }
}