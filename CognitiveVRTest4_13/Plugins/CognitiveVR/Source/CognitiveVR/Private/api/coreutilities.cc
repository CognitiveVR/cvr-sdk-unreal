#include "Private/api/coreutilities.h"

using namespace cognitivevrapi;

CoreUtilities::CoreUtilities(FAnalyticsProviderCognitiveVR* sp)
{
	s = sp;
}

void CoreUtilities::NewUser(std::string nuser_id)
{
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, nuser_id);
	Util::AppendToJsonArray(jsonArray, empty);

	//s->thread_manager->PushTask(NULL, "datacollector_newUser", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_newUser");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);
}

void CoreUtilities::NewDevice(std::string ndevice_id)
{
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, empty);
	Util::AppendToJsonArray(jsonArray, ndevice_id);

	//s->thread_manager->PushTask(NULL, "datacollector_newDevice", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_newDevice");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);
}

void CoreUtilities::NewUserChecked(std::string nuser_id)
{
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, nuser_id);
	Util::AppendToJsonArray(jsonArray, empty);

	//s->thread_manager->PushTask(NULL, "datacollector_newUserChecked", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_newUserChecked");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);
}

void CoreUtilities::NewDeviceChecked(std::string ndevice_id)
{
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, empty);
	Util::AppendToJsonArray(jsonArray, ndevice_id);

	//s->thread_manager->PushTask(NULL, "datacollector_newDeviceChecked", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_newDeviceChecked");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);
}

void CoreUtilities::UpdateUserState(std::string nuser_id, TSharedPtr<FJsonObject> nproperties)
{
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	if (nproperties.Get() == NULL)
	{
		nproperties = MakeShareable(new FJsonObject);
	}

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, nuser_id);
	Util::AppendToJsonArray(jsonArray, empty);
	Util::AppendToJsonArray(jsonArray, nproperties);

	//s->thread_manager->PushTask(NULL, "datacollector_updateUserState", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateUserState");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);
}

void CoreUtilities::UpdateDeviceState(std::string ndevice_id, TSharedPtr<FJsonObject> nproperties)
{
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	if (nproperties.Get() == NULL)
	{
		nproperties = MakeShareable(new FJsonObject);
	}
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, empty); //user
	Util::AppendToJsonArray(jsonArray, ndevice_id); //device
	Util::AppendToJsonArray(jsonArray, nproperties);

	//s->thread_manager->PushTask(NULL, "datacollector_updateDeviceState", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateDeviceState");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);
}

void CoreUtilities::UpdateCollection(std::string nname, double nbalance, double nbalance_delta, bool nis_currency)
{
	if (s == NULL)
	{
		CognitiveLog::Warning("CoreUtilities::UpdateCollection- FAnalyticsProviderCognitiveVR is null!");
		return;
	}

	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	s->AppendUD(jsonArray);

	Util::AppendToJsonArray(jsonArray, nname);
	Util::AppendToJsonArray(jsonArray, nbalance);
	Util::AppendToJsonArray(jsonArray, nbalance_delta);
	Util::AppendToJsonArray(jsonArray, nis_currency);

	//s->thread_manager->PushTask(NULL, "datacollector_updateCollection", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateCollection");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);
}

void CoreUtilities::RecordPurchase(std::string transaction_id, double price, std::string currency_code, std::string result, std::string offer_id, std::string point_of_sale, std::string item_name)//, std::string nuser_id, std::string ndevice_id)
{
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));
	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);

	TSharedPtr<FJsonObject> currency_json(new FJsonObject());
	currency_json.Get()->SetNumberField(currency_code.c_str(), price);
	properties->SetObjectField("price", currency_json);

	properties->SetStringField("offerId", offer_id.c_str());
	properties->SetStringField("pointOfSale", point_of_sale.c_str());
	properties->SetStringField("itemName", item_name.c_str());

	s->transaction->BeginEnd("cvr.purchase", properties, transaction_id, result);
}
