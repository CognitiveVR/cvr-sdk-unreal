#include "api/coreutilities.h"

namespace cognitivevrapi
{
	CoreUtilities::CoreUtilities(CognitiveVR* sp)
	{
		s = sp;
	}

	void CoreUtilities::NewUser(std::string nuser_id)
	{
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, nuser_id);
		Util::AppendToJsonArray(jsonArray, empty);

		s->thread_manager->PushTask(NULL, "datacollector_newUser", jsonArray);
	}

	void CoreUtilities::NewDevice(std::string ndevice_id)
	{
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, empty);
		Util::AppendToJsonArray(jsonArray, ndevice_id);

		s->thread_manager->PushTask(NULL, "datacollector_newDevice", jsonArray);
	}

	void CoreUtilities::NewUserChecked(std::string nuser_id)
	{
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, nuser_id);
		Util::AppendToJsonArray(jsonArray, empty);

		s->thread_manager->PushTask(NULL, "datacollector_newUserChecked", jsonArray);
	}

	void CoreUtilities::NewDeviceChecked(std::string ndevice_id)
	{
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, empty);
		Util::AppendToJsonArray(jsonArray, ndevice_id);

		s->thread_manager->PushTask(NULL, "datacollector_newDeviceChecked", jsonArray);
	}

	void CoreUtilities::UpdateUserState(std::string nuser_id, TSharedPtr<FJsonObject> nproperties)
	{
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
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

		s->thread_manager->PushTask(NULL, "datacollector_updateUserState", jsonArray);
	}

	void CoreUtilities::UpdateDeviceState(std::string ndevice_id, TSharedPtr<FJsonObject> nproperties)
	{
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;

		if (nproperties.Get() == NULL)
		{
			nproperties = MakeShareable(new FJsonObject);
		}
		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, empty);
		Util::AppendToJsonArray(jsonArray, ndevice_id);
		Util::AppendToJsonArray(jsonArray, nproperties);

		s->thread_manager->PushTask(NULL, "datacollector_updateDeviceState", jsonArray);
	}

	void CoreUtilities::UpdateCollection(std::string nname, double nbalance, double nbalance_delta, bool nis_currency)
	{
		//TODO this null check explodes everything - why??
		return;
		if (s == NULL)
		{
			Log::Warning("CoreUtilities::UpdateCollection- CognitiveVR is null!");
			return;
		}

		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		s->AppendUD(jsonArray);

		Util::AppendToJsonArray(jsonArray, nname);
		Util::AppendToJsonArray(jsonArray, nbalance);
		Util::AppendToJsonArray(jsonArray, nbalance_delta);
		Util::AppendToJsonArray(jsonArray, nis_currency);

		//timestamp, timestamp, userid, deviceid, name, balance, balancemod, currency
		s->thread_manager->PushTask(NULL, "datacollector_updateCollection", jsonArray);
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

		s->transaction->BeginEnd("purchase", properties, transaction_id, result);
	}
}