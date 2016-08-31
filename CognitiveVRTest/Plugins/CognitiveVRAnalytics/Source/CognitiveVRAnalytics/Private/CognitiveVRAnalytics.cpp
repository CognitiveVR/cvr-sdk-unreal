#include "CognitiveVRAnalyticsPrivatePCH.h"
#include "CognitiveVRAnalytics.h"
#include "json.h"
//#include "IAnalyticsProvider.h"

//#define LOCTEXT_NAMESPACE "FCognitiveVRAnalytics"

void FCognitiveVRAnalytics::Init(std::string user_id, std::string device_id)
{
	if (this->cognitivevr == NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::Init this IS null"));
	}
	else if (this->cognitivevr->user_id.IsEmpty() && this->cognitivevr->device_id.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::Init found stub implementation"));
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		this->cognitivevr = cognitivevrapi::Init(user_id, device_id, properties);
	}
	UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::Init this NOT null"));
	return;
}

void FCognitiveVRAnalytics::Init(std::string user_id, std::string device_id, TSharedPtr<FJsonObject>init_device_properties)
{
	if (this->cognitivevr == NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::Init this IS null"));
	}
	else if (this->cognitivevr->user_id.IsEmpty() && this->cognitivevr->device_id.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::Init found stub implementation"));
		this->cognitivevr = cognitivevrapi::Init(user_id, device_id, init_device_properties);
	}
	UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::Init this NOT null"));
	return;
}

cognitivevrapi::CognitiveVR* FCognitiveVRAnalytics::CognitiveVR()
{
	if (this->cognitivevr == NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::OH GOD FALLBACK INIT"));

		TSharedPtr<FJsonObject> init_device_properties = MakeShareable(new FJsonObject);
		//this->cognitivevr = cognitivevrapi::Init("companyname1234-product-test", "", "", properties);
		this->cognitivevr = cognitivevrapi::Init("", "", init_device_properties);


		//cognitivevrapi::CognitiveVRResponse response(false);
		//response.SetErrorMessage("CognitiveVR has not been initialized yet.");
		//response.SetContent(Json::Value::null); //may need json here! conflict between splut jsoncpp and unreal built in
		//throw cognitivevrapi::cognitivevr_exception(response);
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
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

namespace cognitivevrapi
{
	void InitCallback(CognitiveVRResponse resp)
	{
		UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::Callback INIT----------------------------------------response"));

		//CognitiveVRResponse response = callback;

		if (!resp.IsSuccessful()) {

			//std::cout << "Network.c Init response raw: " << resp.GetContent(). << std::endl;

			//std::cout << "Netwerk.c Init response not successful: " << resp.GetContent().asString() << std::endl;

			cognitivevrapi::ThrowDummyResponseException("Failed to initialize CognitiveVR: " + resp.GetErrorMessage());
		}

		FJsonObject json = resp.GetContent();

		CognitiveVR *cog = FCognitiveVRAnalytics::Get().CognitiveVR();
		cog->transaction = new Transaction(cog);
		cog->tuning = new Tuning(cog, json);
		cog->thread_manager = new BufferManager(cog->network);
	}

    void CognitiveVR::InitNetwork(HttpInterface* httpint)
    {
		this->network = new cognitivevrapi::Network(this);
        this->network->Init(httpint, &InitCallback);
    }



    void CognitiveVR::AppendUD(TSharedPtr<FJsonValueArray>& jsonArray)
    {
        if (user_id.IsEmpty())
		{
			FString empty = FString("");
			Util::AppendToJsonArray(jsonArray, empty);
        }
		else
		{
			Util::AppendToJsonArray(jsonArray, user_id);
        }

		if (device_id.IsEmpty())
		{
			FString empty = FString("");
			Util::AppendToJsonArray(jsonArray, empty);
		}
		else
		{
			Util::AppendToJsonArray(jsonArray, device_id);
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

    void CognitiveVR::NewUser(NetworkCallback callback, std::string nuser_id)
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

        this->thread_manager->PushTask(callback, "datacollector_newUser", jsonArray);
    }

    void CognitiveVR::NewDevice(NetworkCallback callback, std::string ndevice_id)
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

        this->thread_manager->PushTask(callback, "datacollector_newDevice", jsonArray);
    }

    void CognitiveVR::NewUserChecked(NetworkCallback callback, std::string nuser_id)
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

        this->thread_manager->PushTask(callback, "datacollector_newUserChecked", jsonArray);
    }

    void CognitiveVR::NewDeviceChecked(NetworkCallback callback, std::string ndevice_id)
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

        this->thread_manager->PushTask(callback, "datacollector_newDeviceChecked", jsonArray);
    }

    void CognitiveVR::UpdateUserState(NetworkCallback callback, std::string nuser_id, TSharedPtr<FJsonObject> nproperties)
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
		Util::AppendToJsonArray(jsonArray, nproperties);

        this->thread_manager->PushTask(callback, "datacollector_updateUserState", jsonArray);
    }

    void CognitiveVR::UpdateDeviceState(NetworkCallback callback, std::string ndevice_id, TSharedPtr<FJsonObject> nproperties)
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
		Util::AppendToJsonArray(jsonArray, nproperties);

        this->thread_manager->PushTask(callback, "datacollector_updateDeviceState", jsonArray);
    }

    void CognitiveVR::UpdateCollection(NetworkCallback callback, std::string nname, double nbalance, double nbalance_delta, bool nis_currency, std::string nuser_id, std::string ndevice_id)
    {
		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;

		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		AppendUD(jsonArray);

		Util::AppendToJsonArray(jsonArray, nname);
		Util::AppendToJsonArray(jsonArray, nbalance);
		Util::AppendToJsonArray(jsonArray, nbalance_delta);
		Util::AppendToJsonArray(jsonArray, nis_currency);

        this->thread_manager->PushTask(callback, "datacollector_updateCollection", jsonArray);
    }

    void CognitiveVR::RecordPurchase(NetworkCallback callback, std::string name, double price, std::string currency_code, std::string result, std::string offer_id, std::string point_of_sale, std::string item_name, std::string nuser_id, std::string ndevice_id)
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

		this->transaction->BeginEnd("purchase", properties, name, callback, result);
    }

	CognitiveVR* Init(std::string user_id, std::string device_id, TSharedPtr<FJsonObject> init_device_properties)
	{
        OverrideHttpInterface* httpint = new OverrideHttpInterface();
		Log::Info("CognitiveVR init.");

		/*if (customer_id.empty()) {
			cognitivevrapi::ThrowDummyResponseException("A customer ID is required.");
		}*/

		CognitiveVR* s = new CognitiveVR();

		if (user_id.empty() && device_id.empty()) {
			cognitivevrapi::ThrowDummyResponseException("A user or device ID is required.");
			return s;
		}

		if (Config::kTuningCacheTtl <= 0) {
			cognitivevrapi::ThrowDummyResponseException("Invalid tuning cache TTL.");
		}

		

		
		//s->customer_id = customer_id;
		s->user_id = user_id.c_str();
		s->device_id = device_id.c_str();
		s->initProperties = init_device_properties;

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
		UE_LOG(LogTemp, Warning, TEXT("CognitiveVRAnalytics::ResponseException! %s"), UTF8_TO_TCHAR(s.c_str()));
		//UE_LOG(LogTemp, Warning, TEXT("cognitiveanalytics::dummy response exception %s"), UTF8_TO_TCHAR(response.c_str()));
        //throw cognitivevrapi::cognitivevr_exception(response);
    }
}