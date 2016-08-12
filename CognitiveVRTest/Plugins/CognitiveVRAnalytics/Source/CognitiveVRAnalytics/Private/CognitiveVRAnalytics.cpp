#include "CognitiveVRAnalyticsPrivatePCH.h"
#include "CognitiveVRAnalytics.h"
#include "json.h"
//#include "IAnalyticsProvider.h"

//#define LOCTEXT_NAMESPACE "FCognitiveVRAnalytics"

void FCognitiveVRAnalytics::Init(std::string customer_id, std::string user_id, std::string device_id)
{
	UE_LOG(LogTemp, Warning, TEXT("FCognitiveVRAnalytics::Init----------"));
	this->cognitivevr = cognitivevrapi::Init(customer_id, user_id, device_id);

	//FJsonObject json;

	//this->cognitivevr->cognitivevrapi::CognitiveVR::transaction->BeginEndAsync(NULL,"init trans", "category", "result", "context", "user id", "device id",NULL);
}

cognitivevrapi::CognitiveVR* FCognitiveVRAnalytics::CognitiveVR()
{
	if (this->cognitivevr == NULL) {
	cognitivevrapi::CognitiveVRResponse response(false);
	response.SetErrorMessage("CognitiveVR has not been initialized yet.");
	//response.SetContent(Json::Value::null); //may need json here! conflict between splut jsoncpp and unreal built in
	throw cognitivevrapi::cognitivevr_exception(response);
	}

	return this->cognitivevr;
}


void FCognitiveVRAnalytics::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
}

void FCognitiveVRAnalytics::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
//#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCognitiveVRAnalytics, "CognitiveVRAnalytics")

/*
** Copyright (c) 2015 Knetik, Inc. All rights reserved.
*/

namespace cognitivevrapi
{
    void CognitiveVR::InitNetwork(HttpInterface* httpint)
    {
		this->network = new cognitivevrapi::Network(this);
        FJsonObject json = this->network->Init(httpint);

        transaction = new Transaction(this);
        tuning = new Tuning(this, json);


        thread_manager = new ThreadManager(network);
    }

    void CognitiveVR::AppendUD(TSharedPtr<FJsonValueArray>& jsonArray, FString nuser_id, FString ndevice_id)
    {
		//TODO save user_id class variable as FString

        if (nuser_id.IsEmpty()) {
            if (user_id.empty()) {
				FString empty = FString("");
				Util::AppendToJsonArray(jsonArray, empty);
            } else {
				FString userid = user_id.c_str();
				Util::AppendToJsonArray(jsonArray, userid);
            }
        } else {
			Util::AppendToJsonArray(jsonArray, nuser_id);
        }

		if (ndevice_id.IsEmpty()) {
			if (device_id.empty()) {
				FString empty = FString("");
				Util::AppendToJsonArray(jsonArray, empty);
			}
			else {
				FString deviceid = device_id.c_str();
				Util::AppendToJsonArray(jsonArray, deviceid);
			}
		}
		else {
			Util::AppendToJsonArray(jsonArray, ndevice_id);
		}
    }

	CognitiveVRResponse CognitiveVR::HandleResponse(std::string type, CognitiveVRResponse resp)
    {
        if (!resp.IsSuccessful()) {
            resp.SetErrorMessage("CognitiveVR Error: " + resp.GetErrorMessage());
			UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::ResponseException!"));
            //throw cognitivevrapi::cognitivevr_exception(resp);
        }

        return resp;
    }

    void CognitiveVR::NewUserAsync(NetworkCallback callback, std::string nuser_id, std::string ncontext)
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

        this->thread_manager->PushTask(callback, "datacollector_newUser", jsonArray, ncontext);
    }

    void CognitiveVR::NewDeviceAsync(NetworkCallback callback, std::string ndevice_id, std::string ncontext)
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

        this->thread_manager->PushTask(callback, "datacollector_newDevice", jsonArray, ncontext);
    }

    void CognitiveVR::NewUserCheckedAsync(NetworkCallback callback, std::string nuser_id, std::string ncontext)
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

        this->thread_manager->PushTask(callback, "datacollector_newUserChecked", jsonArray, ncontext);
    }

    void CognitiveVR::NewDeviceCheckedAsync(NetworkCallback callback, std::string ndevice_id, std::string ncontext)
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

        this->thread_manager->PushTask(callback, "datacollector_newDeviceChecked", jsonArray, ncontext);
    }

    void CognitiveVR::UpdateUserStateAsync(NetworkCallback callback, std::string nuser_id, std::string ncontext, TSharedPtr<FJsonObject> nproperties)
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
		//json->SetObjectField("data", nproperties.GetObjectField("")); //TODO sharedptr properties deserialize

        this->thread_manager->PushTask(callback, "datacollector_updateUserState", jsonArray, ncontext);
    }

    void CognitiveVR::UpdateDeviceStateAsync(NetworkCallback callback, std::string ndevice_id, std::string ncontext, TSharedPtr<FJsonObject> nproperties)
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

		//json->SetObjectField("data", nproperties.GetObjectField("")); //TODO deserialize json

		network->Call("datacollector_updateDeviceState", jsonArray, ncontext, callback);
        //this->thread_manager->PushTask(callback, "datacollector_updateDeviceState", jsonArray, ncontext);
    }

    void CognitiveVR::UpdateCollectionAsync(NetworkCallback callback, std::string nname, double nbalance, double nbalance_delta, bool nis_currency, std::string ncontext, std::string nuser_id, std::string ndevice_id)
    {
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		AppendUD(jsonArray, nuser_id.c_str(), ndevice_id.c_str());

		Util::AppendToJsonArray(jsonArray, nname);
		Util::AppendToJsonArray(jsonArray, nbalance);
		Util::AppendToJsonArray(jsonArray, nbalance_delta);
		Util::AppendToJsonArray(jsonArray, nis_currency);

        this->thread_manager->PushTask(callback, "datacollector_updateCollection", jsonArray, ncontext);
    }

    void CognitiveVR::RecordPurchaseAsync(NetworkCallback callback, std::string name, double price, std::string currency_code, std::string result, std::string offer_id, std::string point_of_sale, std::string item_name, std::string context, std::string nuser_id, std::string ndevice_id)
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

        this->transaction->BeginEndAsync(callback, name, "purchase", result, context, properties, nuser_id.c_str(), ndevice_id.c_str());
    }

	CognitiveVR* Init(std::string customer_id, std::string user_id, std::string device_id) {
        OverrideHttpInterface* httpint = new OverrideHttpInterface();
		Log::Info("CognitiveVR init.");

		if (customer_id.empty()) {
			cognitivevrapi::ThrowDummyResponseException("A customer ID is required.");
		}

		if (user_id.empty() && device_id.empty()) {
			cognitivevrapi::ThrowDummyResponseException("A user or device ID is required.");
		}

		if (Config::kTuningCacheTtl <= 0) {
			cognitivevrapi::ThrowDummyResponseException("Invalid tuning cache TTL.");
		}

		CognitiveVR* s = new CognitiveVR();

		s->customer_id = customer_id;
		s->user_id = user_id;
		s->device_id = device_id;

		s->InitNetwork(httpint);

		return s;
    }

	CognitiveVR::~CognitiveVR()
    {
        Log::Info("Freeing CognitiveVR memory.");
        delete thread_manager;
        thread_manager = NULL;

        delete network;
        network = NULL;

        delete transaction;
        transaction = NULL;

        delete tuning;
        tuning = NULL;
        Log::Info("CognitiveVR memory freed.");
    }

    void ThrowDummyResponseException(std::string s)
    {
		std::cout << "CognitiveVR.c ThrowDummyResponseException: " << s << std::endl;

		CognitiveVRResponse response(false);
        response.SetErrorMessage(s);
        response.SetContent(FJsonObject());
		UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::ResponseException!"));
        //throw cognitivevrapi::cognitivevr_exception(response);
    }
}