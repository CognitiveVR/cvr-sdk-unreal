// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"
#include "HeadMountedDisplay.h"
#include "AnalyticsSettings.h"
#include "CognitiveVRSettings.h"
#include "PlayerTracker.h"
#include "DynamicObject.h"

using namespace cognitivevrapi;

IMPLEMENT_MODULE(FAnalyticsCognitiveVR, CognitiveVR);

bool bHasSessionStarted = false;
FHttpModule* Http;

void FAnalyticsCognitiveVR::StartupModule()
{
	CognitiveVRProvider = MakeShareable(new FAnalyticsProviderCognitiveVR());
}

void FAnalyticsCognitiveVR::ShutdownModule()
{
	/*if (CognitiveVRProvider.IsValid())
	{
	CognitiveVRProvider->EndSession();
	}*/
}

bool FAnalyticsProviderCognitiveVR::HasStartedSession()
{
	return bHasSessionStarted;
}

TSharedPtr<IAnalyticsProvider> FAnalyticsCognitiveVR::CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const
{
	return CognitiveVRProvider;
}

TSharedPtr<FAnalyticsProviderCognitiveVR> FAnalyticsCognitiveVR::GetCognitiveVRProvider() const
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	if (Provider.IsValid())
	{
		TSharedPtr<FAnalyticsProviderCognitiveVR> prov = StaticCastSharedPtr<FAnalyticsProviderCognitiveVR>(CognitiveVRProvider);
		return prov;
	}
	cognitivevrapi::CognitiveLog::Warning("FAnalyticsCognitiveVR::GetCognitiveVRProvider could not get provider!");
	return NULL;
}

// Provider


FAnalyticsProviderCognitiveVR::FAnalyticsProviderCognitiveVR() :
	Age(0)
{
	DeviceId = FPlatformMisc::GetUniqueDeviceId();
}

FAnalyticsProviderCognitiveVR::~FAnalyticsProviderCognitiveVR()
{
	UE_LOG(LogTemp, Warning, TEXT("shutdown cognitivevr module"));
}

void InitCallback(CognitiveVRResponse resp)
{
	CognitiveLog::Info("CognitiveVR InitCallback Response");
	if (!resp.IsSuccessful())
	{
		ThrowDummyResponseException("Failed to initialize CognitiveVR " + resp.GetErrorMessage());
	}
	bHasSessionStarted = true;
	FJsonObject json = resp.GetContent();

	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL)
	{
		CognitiveLog::Error("CognitiveVR InitCallback could not GetCognitiveVRProvider!");
		return;
	}
	cog->transaction = new Transaction(cog);
	cog->tuning = new Tuning(cog, json);

	if (cog->network == NULL)
	{
		GLog->Log("cognitivevr init netowork == null");
	}

	Http = &FHttpModule::Get();

	cog->thread_manager = new BufferManager(cog->network);
	cog->core_utils = new CoreUtilities(cog);
	cog->sensors = new Sensors(cog);

	CognitiveLog::Info("------------------CognitiveVR InitCallback send session start transaction!");
	cog->transaction->Begin("Session");
	cog->bPendingInitRequest = false;

	cog->SendDeviceInfo();
}

void FAnalyticsProviderCognitiveVR::SendDeviceInfo()
{
	//add a bunch of properties
	if (GEngine->HMDDevice.IsValid())
	{
		auto hmd = GEngine->HMDDevice.Get();

		EHMDDeviceType::Type devicetype = hmd->GetHMDDeviceType();

		if (devicetype == EHMDDeviceType::DT_SteamVR)
		{
			TArray<FAnalyticsEventAttribute> EventAttributes;
			FName deviceName = hmd->GetDeviceName();
			EventAttributes.Add(FAnalyticsEventAttribute(TEXT("DeviceName"), deviceName.ToString()));
			FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->RecordEvent("HMDDevice", EventAttributes);
		}
		else
		{
			hmd->RecordAnalytics();
		}
	}
}

bool FAnalyticsProviderCognitiveVR::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bPendingInitRequest)
	{
		CognitiveLog::Info("------------------>>>>>>pending init");
		return false;
	}
	if (bHasSessionStarted)
	{
		//EndSession();
		//return false;
	}

	SessionTimestamp = Util::GetTimestamp();
	SessionId = FString::FromInt(Util::GetTimestampLong()) + TEXT("_") + DeviceId;


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

	CognitiveLog::Info("------------------>>>>>>CognitiveVR FAnalyticsProviderCognitiveVR::StartSession!");

	return bHasSessionStarted;
}

void FAnalyticsProviderCognitiveVR::EndSession()
{
	if (transaction == NULL)
	{
		return;
	}


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

	delete sensors;
	sensors = NULL;
	CognitiveLog::Info("CognitiveVR memory freed.");


	bHasSessionStarted = false;
}

void FAnalyticsProviderCognitiveVR::FlushEvents()
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents could not GetCognitiveVRProvider!");
		return;
	}

	if (cog->thread_manager == NULL)
	{
		GLog->Log("thread manager is null");
		return;
	}

	if (cog->HasStartedSession() == false)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents CognitiveVRProvider has not started session!");
		return;
	}

	cog->thread_manager->SendBatch();

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents number of controllers is zero!");
		return;
	}
	UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();
	if (up == NULL)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents couldn't find player tracker!");
		return;
	}
	else
	{
		up->SendGazeEventDataToSceneExplorer();
	}

	sensors->SendData();

	UDynamicObject::SendData();

	//OnSendData.Broadcast();

	//delegate calls 'SendData'. dynamic objects connected to this
}

void FAnalyticsProviderCognitiveVR::SetUserID(const FString& InUserID)
{
	if (!bHasSessionStarted)
	{
		UserId = InUserID;
		CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetUserID set user id");
	}
	else
	{
		// Log that we shouldn't switch users during a session
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SetUserID called while session is in progress. Ignoring");
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

double FAnalyticsProviderCognitiveVR::GetSessionTimestamp() const
{
	return SessionTimestamp;
}

bool FAnalyticsProviderCognitiveVR::SetSessionID(const FString& InSessionID)
{
	if (!bHasSessionStarted)
	{
		//SessionId = InSessionID;
		SessionTimestamp = Util::GetTimestamp();
		CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetSessionID set new session id");

		//TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		//properties->SetStringField("id", SessionId);

		//transaction->Update("Session", properties);
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

		transaction->BeginEnd(TCHAR_TO_UTF8(*EventName), properties);
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

FString FAnalyticsProviderCognitiveVR::GetSceneKey(FString sceneName)
{

	//GConfig->GetArray()
	FConfigSection* Section = GConfig->GetSectionPrivate(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), false, true, GEngineIni);
	if (Section == NULL)
	{
		return "";
	}
	for (FConfigSection::TIterator It(*Section); It; ++It)
	{
		if (It.Key() == TEXT("SceneData"))
		{
			FString name;
			FString key;
			It.Value().GetValue().Split(TEXT(","), &name, &key);
			if (*name == sceneName)
			{
				GLog->Log("-----> UPlayerTracker::GetSceneKey found key for scene " + name);
				return key;
			}
			else
			{
				GLog->Log("UPlayerTracker::GetSceneKey found key for scene " + name);
			}
		}
	}

	//no matches anywhere
	CognitiveLog::Warning("UPlayerTracker::GetSceneKey ------- no matches in ini");
	return "";
}

void FAnalyticsProviderCognitiveVR::SendJson(FString endpoint, FString json)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL)
	{
		CognitiveLog::Warning("UPlayerTRacker::SendJson CognitiveVRProvider has not started a session!");
		GLog->Log("UPlayerTRacker::SendJson CognitiveVRProvider has not started a session!");
		return;
	}

	if (Http == NULL)
	{
		CognitiveLog::Warning("Cognitive Provider::SendJson Http module not initialized! likely hasn't started session");
		GLog->Log("Cognitive Provider::SendJson Http module not initialized! likely hasn't started session");
		return;
	}

	UWorld* myworld = GWorld->GetWorld();
	if (myworld == NULL)
	{
		CognitiveLog::Warning("PlayerTracker::SendJson no world");
		GLog->Log("PlayerTracker::SendJson no world");
		return;
	}

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

	FString sceneKey = cog->GetSceneKey(currentSceneName);
	if (sceneKey == "")
	{
		CognitiveLog::Warning("UPlayerTracker::SendJson does not have scenekey. fail!");
		GLog->Log("UPlayerTracker::SendJson does not have scenekey. fail!");
		return;
	}
	GLog->Log("Scene Key " + sceneKey);

	std::string stdjson(TCHAR_TO_UTF8(*json));
	std::string ep(TCHAR_TO_UTF8(*endpoint));
	//CognitiveLog::Info(stdjson);

	GLog->Log(json);

	FString url = "https://sceneexplorer.com/api/" + endpoint + "/" + sceneKey;


	//json to scene endpoint

	TSharedRef<IHttpRequest> HttpRequest = Http->CreateRequest();
	HttpRequest->SetContentAsString(json);
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->ProcessRequest();
}

FVector FAnalyticsProviderCognitiveVR::GetPlayerHMDPosition()
{
	//TODO cache this. are playercontrollers persisted across level changes?

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Info("--------------------------no controllers");
		return FVector();
	}

	return controllers[0]->PlayerCameraManager->GetCameraLocation();
	/*
	UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();
	if (up == NULL)
	{
	if (controllers[0] == NULL) { CognitiveLog::Info("--------------------------controller0 is null"); return FVector(); }

	if (controllers[0]->PlayerCameraManager->HasActiveCameraComponent())
	{
	CognitiveLog::Info("--------------------------controllers[0]->PlayerCameraManager->HasActiveCameraComponent() TRUE TRUE TRUE TRUE");
	return FVector();
	}

	//there is no camera component! never has been, never will be. playertracker also isn't a component that needs to be put into the scene

	//FVector loc;
	//FRotator rot;
	//controllers[0]->GetPlayerViewPoint(loc, rot);




	//if (controllers[0]->GetPawn() == NULL) { CognitiveLog::Info("-------------------------------------controller0 pawn is null"); return FVector(); }

	//if (controllers[0]->player FindComponentByClass<UCameraComponent>() == NULL) { CognitiveLog::Info("----------------------------------controller0 cameracomponent is null"); return FVector(); }

	if (controllers[0]->FindComponentByClass<UCameraComponent>() == NULL) { CognitiveLog::Info("----------------------------------controller0 cameracomponent is null"); return FVector(); }

	CognitiveLog::Info("-------------------------------FAnalyticsProviderCognitiveVR::GetPlayerHMDPosition couldn't find UPlayerTracker. using player camera instead");
	return controllers[0]->FindComponentByClass<UCameraComponent>()->ComponentToWorld.GetTranslation();
	}
	else
	{
	CognitiveLog::Info("--------------------------use playertracker component");
	return up->ComponentToWorld.GetTranslation();
	}*/

	//return controllers[0]->GetPawn()->GetActorTransform().GetLocation();
}