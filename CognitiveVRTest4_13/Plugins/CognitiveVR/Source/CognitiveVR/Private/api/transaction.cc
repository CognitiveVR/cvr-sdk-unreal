/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "Private/api/transaction.h"
#include "PlayerTracker.h"

using namespace cognitivevrapi;

Transaction::Transaction(FAnalyticsProviderCognitiveVR* sp)
{
	s = sp;
}

void Transaction::Begin(std::string category, TSharedPtr<FJsonObject> properties, std::string transaction_id)
{
	Transaction::BeginPosition(category, s->GetPlayerHMDPosition(), properties, transaction_id);
}

void Transaction::BeginPosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties, std::string transaction_id)
{
	if (!bHasSessionStarted)
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

	double ts = Util::GetTimestamp();

	FString trans = FString("TXN");

	Util::AppendToJsonArray(jsonArray, ts);
	Util::AppendToJsonArray(jsonArray, ts);
	s->AppendUD(jsonArray);
	Util::AppendToJsonArray(jsonArray, category);
	Util::AppendToJsonArray(jsonArray, trans);
	Util::AppendToJsonArray(jsonArray, Config::kNetworkTimeout);
	Util::AppendToJsonArray(jsonArray, transaction_id);
	Util::AppendToJsonArray(jsonArray, properties);

	//dashboard
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_beginTransaction");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);

	//scene explorer
	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0) { return; }
	if (controllers[0]->GetPawn() == NULL) { return; }
	UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();
	if (up == NULL) { return; }
	
	TArray< TSharedPtr<FJsonValue> > pos;
	pos.Add(MakeShareable(new FJsonValueNumber((int32)-Position.X)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Z)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Y)));

	FJsonObject* eventObject = new FJsonObject;
	eventObject->SetStringField("name", category.c_str());
	eventObject->SetNumberField("time", ts);
	eventObject->SetArrayField("point", pos);

	if (properties.Get()->Values.Num() > 0)
	{
		eventObject->SetObjectField("properties", properties);
	}
	
	if (up != nullptr)
	{
		up->AddJsonEvent(eventObject);
	}
}

void Transaction::Update(std::string category, TSharedPtr<FJsonObject> properties, std::string transaction_id, double progress)
{
	Transaction::UpdatePosition(category, s->GetPlayerHMDPosition(), properties, transaction_id, progress);
}

void Transaction::UpdatePosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties, std::string transaction_id, double progress)
{
	if (!bHasSessionStarted)
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

	double ts = Util::GetTimestamp();

	Util::AppendToJsonArray(jsonArray, ts);
	Util::AppendToJsonArray(jsonArray, ts);
	s->AppendUD(jsonArray);
	Util::AppendToJsonArray(jsonArray, category);
	Util::AppendToJsonArray(jsonArray, progress);
	Util::AppendToJsonArray(jsonArray, transaction_id);
	Util::AppendToJsonArray(jsonArray, properties);

	//dashboard
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateTransaction");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);

	//scene explorer
	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0) { return; }
	if (controllers[0]->GetPawn() == NULL) { return; }
	UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();
	if (up == NULL) { return; }

	TArray< TSharedPtr<FJsonValue> > pos;
	pos.Add(MakeShareable(new FJsonValueNumber((int32)-Position.X)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Z)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Y)));

	FJsonObject* eventObject = new FJsonObject;
	eventObject->SetStringField("name", category.c_str());
	eventObject->SetNumberField("time", ts);
	eventObject->SetArrayField("point", pos);
	if (properties.Get()->Values.Num() > 0)
	{
		eventObject->SetObjectField("properties", properties);
	}

	if (up != nullptr)
	{
		up->AddJsonEvent(eventObject);
	}
}

void Transaction::End(std::string category, TSharedPtr<FJsonObject> properties, std::string transaction_id, std::string result)
{
	if (s == NULL)
	{
		CognitiveLog::Warning("Transaction::End - FAnalyticsProviderCognitiveVR is null!");
		return;
	}
	Transaction::EndPosition(category, s->GetPlayerHMDPosition(), properties, transaction_id, result);
}

void Transaction::EndPosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties, std::string transaction_id, std::string result)
{
	if (!bHasSessionStarted)
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

	double ts = Util::GetTimestamp();

	Util::AppendToJsonArray(jsonArray, ts);
	Util::AppendToJsonArray(jsonArray, ts);
	s->AppendUD(jsonArray);
	Util::AppendToJsonArray(jsonArray, category);
	Util::AppendToJsonArray(jsonArray, result);
	Util::AppendToJsonArray(jsonArray, transaction_id);
	Util::AppendToJsonArray(jsonArray, properties);

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);

	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_endTransaction");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);

	if (controllers.Num() == 0)
	{
		CognitiveLog::Warning("Transaction. local player controller count is 0. skip transaction on scene explorer");
		return;
	}

	if (controllers[0]->GetPawn() == NULL)
	{
		CognitiveLog::Warning("Transaction. local player controller does not have pawn. skip transaction on scene explorer");
		return;
	}

	UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();

	TArray< TSharedPtr<FJsonValue> > pos;
	pos.Add(MakeShareable(new FJsonValueNumber((int32)-Position.X)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Z)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Y)));

	FJsonObject* eventObject = new FJsonObject;
	eventObject->SetStringField("name", category.c_str());
	eventObject->SetNumberField("time", ts);
	eventObject->SetArrayField("point", pos);
	if (properties.Get()->Values.Num() > 0)
	{
		eventObject->SetObjectField("properties", properties);
	}

	if (up != nullptr)
	{
		up->AddJsonEvent(eventObject);
	}
}

//add to some list of json transactions

void Transaction::BeginEnd(std::string category, TSharedPtr<FJsonObject> properties, std::string transaction_id, std::string result)
{
	if (s == NULL)
	{
		CognitiveLog::Warning("Transaction::BeginEnd - FAnalyticsProviderCognitiveVR is null!");
		return;
	}
	Transaction::BeginEndPosition(category, s->GetPlayerHMDPosition(), properties, transaction_id, result);
}

void Transaction::BeginEndPosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties, std::string transaction_id, std::string result)
{
	if (!bHasSessionStarted)
	{
		CognitiveLog::Warning("Transaction::BeginEnd - FAnalyticsProviderCognitiveVR is null!");
		return;
	}
	this->EndPosition(category, Position, properties, transaction_id, result);
}
