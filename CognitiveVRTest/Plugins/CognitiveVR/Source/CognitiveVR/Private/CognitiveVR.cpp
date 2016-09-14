// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"
//#if WITH_EDITOR
//#include "ISettingsModule.h"
//#include "ISettingsSection.h"
//#endif
#include "AnalyticsSettings.h"
#include "CognitiveVRSettings.h"

using namespace cognitivevrapi;

IMPLEMENT_MODULE(FAnalyticsCognitiveVR, CognitiveVR);


void FAnalyticsCognitiveVR::StartupModule()
{
	//UE_LOG(CognitiveVR_Log, Log, TEXT("---------------------------------------------------FANALYTICS COGNITIVEVR STARTUP"));
	CognitiveVRProvider = MakeShareable(new FAnalyticsProviderCognitiveVR());
}

void FAnalyticsCognitiveVR::ShutdownModule()
{
	//UE_LOG(CognitiveVR_Log, Log, TEXT("---------------------------------------------------FANALYTICS COGNITIVEVR SHUTDOWN"));
	/*if (CognitiveVRProvider.IsValid())
	{
		CognitiveVRProvider->EndSession();
	}*/
}

TSharedPtr<IAnalyticsProvider> FAnalyticsCognitiveVR::CreateAnalyticsProvider(const FAnalytics::FProviderConfigurationDelegate& GetConfigValue) const
{
	//UE_LOG(CognitiveVR_Log, Log, TEXT("---------------------------------------------------FANALYTICS COGNITIVEVR CREATE PROVIDER"));
	return CognitiveVRProvider;
}

TSharedPtr<FAnalyticsProviderCognitiveVR> FAnalyticsCognitiveVR::GetCognitiveVRProvider() const
{
	//UE_LOG(CognitiveVR_Log, Log, TEXT("---------------------------------------------------FANALYTICS COGNITIVEVR GET COGNITIVE PROVIDER"));
	TSharedPtr<FAnalyticsProviderCognitiveVR> prov = StaticCastSharedPtr<FAnalyticsProviderCognitiveVR>(CognitiveVRProvider);
	return prov;
}

// Provider


FAnalyticsProviderCognitiveVR::FAnalyticsProviderCognitiveVR() :
	bHasSessionStarted(false),
	Age(0)
{
	//UE_LOG(CognitiveVR_Log, Log, TEXT("---------------------------------------------------COGNITITVE PROVIDER CONSTUCTOR"));
	DeviceId = FPlatformMisc::GetUniqueDeviceId();
}

/*FAnalyticsProviderCognitiveVR::~FAnalyticsProviderCognitiveVR()
{
	if (bHasSessionStarted)
	{
		EndSession();
	}
}*/

void InitCallback(CognitiveVRResponse resp)
{
	CognitiveLog::Info("CognitiveVR InitCallback Response");
	if (!resp.IsSuccessful())
	{
		ThrowDummyResponseException("Failed to initialize CognitiveVR " + resp.GetErrorMessage());
	}
	FJsonObject json = resp.GetContent();

	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	cog->transaction = new Transaction(cog);
	cog->tuning = new Tuning(cog, json);
	cog->thread_manager = new BufferManager(cog->network);
	cog->core_utils = new CoreUtilities(cog);

	cog->transaction->Begin("Session");
	cog->bHasSessionStarted = true;
	cog->bPendingInitRequest = false;
}

bool FAnalyticsProviderCognitiveVR::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bPendingInitRequest) { return false; }
	if (bHasSessionStarted)
	{
		EndSession();
	}

	SessionId = UserId + TEXT("-") + FDateTime::Now().ToString();

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	
	

	//get attributes
	//userid
	//deviceid
	//initProperties
	if (Age != 0)
	{
		properties->SetNumberField("Age", Age);
	}
	if (Gender.Len() > 0)
	{
		properties->SetStringField("Gender", Gender);
	}
	if (Location.Len() > 0)
	{
		properties->SetStringField("Location", Location);
	}

	initProperties = properties;


	OverrideHttpInterface* httpint = new OverrideHttpInterface();
	network = new Network(this);
	network->Init(httpint, &InitCallback);
	bPendingInitRequest = true;

	return bHasSessionStarted;
}

void FAnalyticsProviderCognitiveVR::EndSession()
{
	transaction->End("Session");

	/*
	FlushEvents();
	CognitiveLog::Info("Freeing CognitiveVR memory.");
	delete thread_manager;
	thread_manager = NULL;

	delete network;
	network = NULL;

	delete transaction;
	transaction = NULL;

	delete tuning;
	tuning = NULL;

	delete core_utils;
	core_utils = NULL;
	CognitiveLog::Info("CognitiveVR memory freed.");
	*/

	bHasSessionStarted = false;
}

void FAnalyticsProviderCognitiveVR::FlushEvents()
{
	//TODO flush batched calls

	/*if (FileArchive != nullptr)
	{
		FileArchive->Flush();
		UE_LOG(CognitiveVR_Log, Display, TEXT("Analytics file flushed"));
	}*/
}

void FAnalyticsProviderCognitiveVR::SetUserID(const FString& InUserID)
{
	if (!bHasSessionStarted)
	{
		UserId = InUserID;
		CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetUserID set user id");
		//UE_LOG(CognitiveVR_Log, Display, TEXT("User is now (%s)"), *UserId);
	}
	else
	{
		// Log that we shouldn't switch users during a session
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SetUserID called while session is in progress. Ignoring");
		//UE_LOG(CognitiveVR_Log, Warning, TEXT("FAnalyticsProviderFileLogging::SetUserID called while a session is in progress. Ignoring."));
	}
}

FString FAnalyticsProviderCognitiveVR::GetUserID() const
{
	return UserId;
}

FString FAnalyticsProviderCognitiveVR::GetDeviceID() const
{
	return DeviceId;
}

FString FAnalyticsProviderCognitiveVR::GetSessionID() const
{
	return SessionId;
}

bool FAnalyticsProviderCognitiveVR::SetSessionID(const FString& InSessionID)
{
	if (!bHasSessionStarted)
	{
		SessionId = InSessionID;
		CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetSessionID set new session id");
		
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("id", SessionId);
		
		transaction->Update("Session", properties);
	}
	else
	{
		// Log that we shouldn't switch session ids during a session
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordEvent while a session is in progress. Ignoring");
	}
	return !bHasSessionStarted;
}

void FAnalyticsProviderCognitiveVR::RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);


		for (auto Attr : Attributes)
		{
			properties->SetStringField(Attr.AttrName, Attr.AttrValue);
		}

		std::string temp(TCHAR_TO_UTF8(*EventName));

		transaction->BeginEnd(temp, properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordEvent called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordItemPurchase(const FString& ItemId, const FString& Currency, int PerItemCost, int ItemQuantity)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("itemId", ItemId);
		properties->SetStringField("currency", Currency);
		properties->SetNumberField("PerItemCost", PerItemCost);
		properties->SetNumberField("ItemQuantity", ItemQuantity);

		transaction->BeginEnd("RecordItemPurchase", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordItemPurchase called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordCurrencyPurchase(const FString& GameCurrencyType, int GameCurrencyAmount, const FString& RealCurrencyType, float RealMoneyCost, const FString& PaymentProvider)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("GameCurrencyType", GameCurrencyType);
		properties->SetNumberField("GameCurrencyAmount", GameCurrencyAmount);
		properties->SetStringField("RealCurrencyType", RealCurrencyType);
		properties->SetNumberField("RealMoneyCost", RealMoneyCost);
		properties->SetStringField("PaymentProvider", PaymentProvider);

		transaction->BeginEnd("RecordCurrencyPurchase", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordCurrencyPurchase called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordCurrencyGiven(const FString& GameCurrencyType, int GameCurrencyAmount)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("GameCurrencyType", GameCurrencyType);
		properties->SetNumberField("GameCurrencyAmount", GameCurrencyAmount);

		transaction->BeginEnd("RecordCurrencyGiven", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordCurrencyGiven called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::SetAge(const int32 InAge)
{
	Age = InAge;
}

void FAnalyticsProviderCognitiveVR::SetLocation(const FString& InLocation)
{
	Location = InLocation;
}

void FAnalyticsProviderCognitiveVR::SetGender(const FString& InGender)
{
	Gender = InGender;
}

void FAnalyticsProviderCognitiveVR::SetBuildInfo(const FString& InBuildInfo)
{
	BuildInfo = InBuildInfo;
}

void FAnalyticsProviderCognitiveVR::RecordError(const FString& Error, const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("Error", Error);

		for (auto Attr : Attributes)
		{
			properties->SetStringField(Attr.AttrName, Attr.AttrValue);
		}

		transaction->BeginEnd("RecordError", properties);

		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordError");
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordError called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordProgress(const FString& ProgressType, const FString& ProgressName, const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("ProgressType", ProgressType);
		properties->SetStringField("ProgressName", ProgressName);

		for (auto Attr : Attributes)
		{
			properties->SetStringField(Attr.AttrName, Attr.AttrValue);
		}

		transaction->BeginEnd("RecordProgress", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordError called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordItemPurchase(const FString& ItemId, int ItemQuantity, const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("ItemId", ItemId);
		properties->SetNumberField("ItemQuantity", ItemQuantity);

		for (auto Attr : Attributes)
		{
			properties->SetStringField(Attr.AttrName, Attr.AttrValue);
		}

		transaction->BeginEnd("RecordItemPurchase", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordItemPurchase called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordCurrencyPurchase(const FString& GameCurrencyType, int GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("GameCurrencyType", GameCurrencyType);
		properties->SetNumberField("GameCurrencyAmount", GameCurrencyAmount);

		for (auto Attr : Attributes)
		{
			properties->SetStringField(Attr.AttrName, Attr.AttrValue);
		}

		transaction->BeginEnd("RecordCurrencyPurchase", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordCurrencyPurchase called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordCurrencyGiven(const FString& GameCurrencyType, int GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("GameCurrencyType", GameCurrencyType);
		properties->SetNumberField("GameCurrencyAmount", GameCurrencyAmount);

		for (auto Attr : Attributes)
		{
			properties->SetStringField(Attr.AttrName, Attr.AttrValue);
		}

		transaction->BeginEnd("RecordCurrencyGiven", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordCurrencyGiven called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::AppendUD(TSharedPtr<FJsonValueArray>& jsonArray)
{
	if (UserId.IsEmpty())
	{
		FString empty = FString("");
		Util::AppendToJsonArray(jsonArray, empty);
	}
	else
	{
		Util::AppendToJsonArray(jsonArray, UserId);
	}

	if (DeviceId.IsEmpty())
	{
		FString empty = FString("");
		Util::AppendToJsonArray(jsonArray, empty);
	}
	else
	{
		Util::AppendToJsonArray(jsonArray, DeviceId);
	}
}

void FAnalyticsProviderCognitiveVR::SetDeviceID(const FString& InDeviceID)
{
	if (!bHasSessionStarted)
	{
		DeviceId = InDeviceID;
		CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetDeviceID set device id");
	}
	else
	{
		// Log that we shouldn't switch users during a session
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SetDeviceID called while session is in progress. Ignoring");
	}
}

void ThrowDummyResponseException(std::string s)
{
	CognitiveVRResponse response(false);
	response.SetErrorMessage(s);
	response.SetContent(FJsonObject());
	CognitiveLog::Error("CognitiveVRAnalytics::ResponseException! " + s);
}