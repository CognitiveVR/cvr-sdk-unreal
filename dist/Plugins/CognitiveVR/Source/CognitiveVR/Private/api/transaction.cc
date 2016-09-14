/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "Private/api/transaction.h"

using namespace cognitivevrapi;

Transaction::Transaction(FAnalyticsProviderCognitiveVR* sp)
{
	s = sp;
	CognitiveLog::Warning("Transaction::Transaction - INITIALIZED");
}

void Transaction::Begin(std::string category, TSharedPtr<FJsonObject> properties, std::string transaction_id)
{
	if (this == NULL)
	{
		CognitiveLog::Warning("Transaction::Begin - FAnalyticsProviderCognitiveVR is null!");
		return;
	}

	if (properties.Get() == NULL)
	{
		properties = MakeShareable(new FJsonObject);
	}

	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	std::string ts = Util::GetTimestampStr();
	FString fs(ts.c_str());
	FString trans = FString("TXN");

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	s->AppendUD(jsonArray);
	Util::AppendToJsonArray(jsonArray, category);
	Util::AppendToJsonArray(jsonArray, trans);
	Util::AppendToJsonArray(jsonArray, Config::kNetworkTimeout);
	Util::AppendToJsonArray(jsonArray, transaction_id);
	Util::AppendToJsonArray(jsonArray, properties);

	s->thread_manager->PushTask(NULL, "datacollector_beginTransaction", jsonArray);
}

void Transaction::Update(std::string category, TSharedPtr<FJsonObject> properties, std::string transaction_id, double progress)
{
	if (this == NULL)
	{
		CognitiveLog::Warning("Transaction::Update - CognitiveVR is null!");
		return;
	}

	if (properties.Get() == NULL)
	{
		properties = MakeShareable(new FJsonObject);
	}

	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	std::string ts = Util::GetTimestampStr();
	FString fs(ts.c_str());

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	s->AppendUD(jsonArray);
	Util::AppendToJsonArray(jsonArray, category);
	Util::AppendToJsonArray(jsonArray, progress);
	Util::AppendToJsonArray(jsonArray, transaction_id);
	Util::AppendToJsonArray(jsonArray, properties);

	s->thread_manager->PushTask(NULL, "datacollector_updateTransaction", jsonArray);
}

void Transaction::End(std::string category, TSharedPtr<FJsonObject> properties, std::string transaction_id, std::string result)
{
	if (this == NULL)
	{
		CognitiveLog::Warning("Transaction::End - FAnalyticsProviderCognitiveVR is null!");
		return;
	}

	if (properties.Get() == NULL)
	{
		properties = MakeShareable(new FJsonObject);
	}

	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	std::string ts = Util::GetTimestampStr();
	FString fs(ts.c_str());

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	s->AppendUD(jsonArray);
	Util::AppendToJsonArray(jsonArray, category);
	Util::AppendToJsonArray(jsonArray, result);
	Util::AppendToJsonArray(jsonArray, transaction_id);
	Util::AppendToJsonArray(jsonArray, properties);

	s->thread_manager->PushTask(NULL, "datacollector_endTransaction", jsonArray);
}

void Transaction::BeginEnd(std::string category, TSharedPtr<FJsonObject> properties, std::string transaction_id, std::string result)
{
	if (this == NULL) //does this ever happen? when called before transaction is constructed?
	{
		CognitiveLog::Warning("Transaction::BeginEnd - FAnalyticsProviderCognitiveVR is null!");
		return;
	}
	this->Begin(category, properties, transaction_id);
	this->End(category, properties, transaction_id, result);
}