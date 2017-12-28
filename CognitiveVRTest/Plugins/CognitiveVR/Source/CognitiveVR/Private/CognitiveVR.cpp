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
	GLog->Log("AnalyticsCognitiveVR::StartupModule");
	//if (Http == NULL)
		//Http = &FHttpModule::Get();
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

UWorld* currentWorld;
void FAnalyticsProviderCognitiveVR::SetWorld(UWorld* world)
{
	currentWorld = world;
}

void InitCallback(CognitiveVRResponse resp)
{
	CognitiveLog::Info("CognitiveVR InitCallback Response");

	if (!resp.IsSuccessful())
	{
		ThrowDummyResponseException("Failed to initialize CognitiveVR " + resp.GetErrorMessage());

		CognitiveLog::Info("Init callback not successful!");

		FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
		if (cog == NULL)
		{
			CognitiveLog::Error("CognitiveVR InitCallback could not GetCognitiveVRProvider!");
			return;
		}
		cog->bPendingInitRequest = false;
		return;
	}
	bHasSessionStarted = true;
	FJsonObject json = resp.GetContent();

	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL)
	{
		CognitiveLog::Error("CognitiveVR InitCallback could not GetCognitiveVRProvider!");
		return;
	}
	cog->transaction = MakeShareable(new Transaction(cog));// new Transaction(cog);
	cog->tuning = MakeShareable(new Tuning(cog, json));

	if (!cog->network.IsValid())
	{
		CognitiveLog::Warning("CognitiveVRProvider InitCallback network is null");
		return;
	}

	cog->thread_manager = MakeShareable(new BufferManager(cog));
	cog->core_utils = MakeShareable(new CoreUtilities(cog));
	cog->sensors = MakeShareable(new Sensors(cog));

	TSharedPtr<FJsonObject>deviceProperties = Util::DeviceScraper(cog->initProperties);
	if (deviceProperties.IsValid())
	{
		cog->core_utils->UpdateDeviceState(TCHAR_TO_UTF8(*cog->GetDeviceID()), deviceProperties);
	}
	else
	{
		CognitiveLog::Error("Cognitive InitCallback could not find device properties!");
	}

	//send new user / new device messages if necessary

	auto dataObject = resp.GetContent();
	if (dataObject.GetBoolField("usernew"))
	{
		cog->core_utils->NewUser(TCHAR_TO_UTF8(*cog->GetUserID()));
		//new device
	}
	if (dataObject.GetBoolField("devicenew"))
	{
		cog->core_utils->NewDevice(TCHAR_TO_UTF8(*cog->GetDeviceID()));
		//new device
	}

	cog->transaction->Begin("cvr.session");
	cog->bPendingInitRequest = false;

	//get all dynamic objects

	cog->OnInitResponse().Broadcast(resp.IsSuccessful());

	if (currentWorld != NULL)
	{
		for (TActorIterator<AStaticMeshActor> ActorItr(currentWorld); ActorItr; ++ActorItr)
		{
			// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
			AStaticMeshActor *Mesh = *ActorItr;

			UActorComponent* actorComponent = Mesh->GetComponentByClass(UDynamicObject::StaticClass());
			if (actorComponent == NULL)
			{
				continue;
			}
			UDynamicObject* dynamic = Cast<UDynamicObject>(actorComponent);
			if (dynamic == NULL)
			{
				continue;
			}
			if (dynamic->GetObjectId().IsValid())
			{
				continue;
			}
			CognitiveLog::Warning("InitCallback Dynamic Special startup!");
			dynamic->BeginPlayCallback(resp.IsSuccessful());
		}
	}
	else
	{
		CognitiveLog::Error("InitCallback current world is null");
	}

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
	CognitiveLog::Init();

	if (bPendingInitRequest)
	{
		CognitiveLog::Warning("AnalyticsProviderCognitiveVR::StartSession already pending init!");
		return false;
	}
	if (bHasSessionStarted)
	{
		//EndSession();
		//return false;
	}

	GetSessionTimestamp();
	GetSessionID();

	if (Http == NULL)
		Http = &FHttpModule::Get();

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());

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

	if (GetUserID().IsEmpty())
	{
		CognitiveLog::Info("FAnalyticsProviderCognitiveVR::StartSession user id is empty!");
		SetUserID("anonymous_"+DeviceId);
	}

	initProperties = properties;

	CustomerId = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRApiKey", false);

	OverrideHttpInterface* httpint = new OverrideHttpInterface();
	network = MakeShareable(new Network(this));
	network->Init(httpint, &InitCallback);
	bPendingInitRequest = true;

	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::StartSession");
	

	// Also include all the streaming levels in the results
	/*for (int32 LevelIndex = 0; LevelIndex < GWorld->StreamingLevels.Num(); ++LevelIndex)
	{
		ULevelStreaming* StreamingLevel = GWorld->StreamingLevels[LevelIndex];
		if (StreamingLevel != NULL)
		{
			//StreamingLevel->OnLevelLoaded.Add(this,&FAnalyticsProviderCognitiveVR::OnLevelLoaded);
			//TODO clear objectid list when persistent scene unloads
		}
	}*/

	CacheSceneData();

	return bHasSessionStarted;
}

void FAnalyticsProviderCognitiveVR::OnLevelLoaded()
{

}

void FAnalyticsProviderCognitiveVR::EndSession()
{
	if (!transaction.IsValid())
	{
		return;
	}

	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::EndSession");

	bPendingInitRequest = false;

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	properties->SetNumberField("sessionlength", Util::GetTimestamp() - GetSessionTimestamp());

	transaction->End("cvr.session", properties);

	FlushEvents();
	CognitiveLog::Info("Freeing CognitiveVR memory.");
	//delete thread_manager;

	thread_manager = NULL;

	//delete network;
	network = NULL;

	//delete transaction;
	transaction = NULL;

	//delete tuning;
	tuning = NULL;

	//delete core_utils;
	core_utils = NULL;

	//delete sensors;
	sensors = NULL;
	CognitiveLog::Info("CognitiveVR memory freed.");

	SessionTimestamp = -1;
	SessionId = "";

	bHasSessionStarted = false;
	LastSesisonTimestamp = Util::GetTimestamp() + 1;
}

void FAnalyticsProviderCognitiveVR::FlushEvents()
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents could not GetCognitiveVRProvider!");
		return;
	}

	if (!cog->thread_manager.IsValid())
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents batch manager is null!");
		return;
	}

	if (cog->HasStartedSession() == false)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents CognitiveVRProvider has not started session!");
		return;
	}

	//send to dashboard
	cog->thread_manager->SendBatch();

	//send to scene explorer
	sensors->SendData();
	UDynamicObject::SendData();

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents cannot find controller. Skip upload to scene explorer");
		return;
	}
	if (controllers[0]->GetPawn() == NULL)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents controller0 has no pawn. Skip upload to scene explorer");
		return;
	}

	UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();
	if (up == NULL)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::FlushEvents couldn't find player tracker. Skip upload to scene explorer");
		return;
	}
	else
	{
		up->SendGazeEventDataToSceneExplorer();
	}
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

double FAnalyticsProviderCognitiveVR::GetSessionTimestamp()
{
	if (SessionTimestamp < 0)
	{
		SessionTimestamp = Util::GetTimestamp();
	}
	return SessionTimestamp;
}

FString FAnalyticsProviderCognitiveVR::GetCognitiveSessionID()
{
	if (SessionId.IsEmpty())
		SessionId = FString::FromInt(GetSessionTimestamp()) + TEXT("_") + DeviceId;
	return SessionId;
}

bool FAnalyticsProviderCognitiveVR::SetSessionID(const FString& InSessionID)
{
	if (!bHasSessionStarted)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SetSessionID automatically sets session id. Ignoring");

		/*if (SessionTimestamp < 0)
		{
			SessionTimestamp = Util::GetTimestamp();
		}*/
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

void FAnalyticsProviderCognitiveVR::RecordItemPurchase(const FString& ItemId, const FString& Currency, int32 PerItemCost, int32 ItemQuantity)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("itemId", ItemId);
		properties->SetStringField("currency", Currency);
		properties->SetNumberField("PerItemCost", PerItemCost);
		properties->SetNumberField("ItemQuantity", ItemQuantity);

		transaction->BeginEnd("cvr.recorditempurchase", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordItemPurchase called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordCurrencyPurchase(const FString& GameCurrencyType, int32 GameCurrencyAmount, const FString& RealCurrencyType, float RealMoneyCost, const FString& PaymentProvider)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("GameCurrencyType", GameCurrencyType);
		properties->SetNumberField("GameCurrencyAmount", GameCurrencyAmount);
		properties->SetStringField("RealCurrencyType", RealCurrencyType);
		properties->SetNumberField("RealMoneyCost", RealMoneyCost);
		properties->SetStringField("PaymentProvider", PaymentProvider);

		transaction->BeginEnd("cvr.recordcurrencypurchase", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordCurrencyPurchase called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordCurrencyGiven(const FString& GameCurrencyType, int32 GameCurrencyAmount)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("GameCurrencyType", GameCurrencyType);
		properties->SetNumberField("GameCurrencyAmount", GameCurrencyAmount);

		transaction->BeginEnd("cvr.recordcurrencygiven", properties);
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

		transaction->BeginEnd("cvr.recorderror", properties);

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

		transaction->BeginEnd("cvr.recordprogress", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordError called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordItemPurchase(const FString& ItemId, int32 ItemQuantity, const TArray<FAnalyticsEventAttribute>& Attributes)
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

		transaction->BeginEnd("cvr.recorditempurchase", properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordItemPurchase called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::RecordCurrencyPurchase(const FString& GameCurrencyType, int32 GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& Attributes)
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

void FAnalyticsProviderCognitiveVR::RecordCurrencyGiven(const FString& GameCurrencyType, int32 GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& Attributes)
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

		transaction->BeginEnd("cvr.recordcurrencygiven", properties);
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
		//DeviceId = InDeviceID;
		//CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetDeviceID set device id");
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

TSharedPtr<FSceneData> FAnalyticsProviderCognitiveVR::GetCurrentSceneData()
{
	UWorld* myworld = currentWorld;
	//UWorld* myworld = AActor::GetWorld();
	if (myworld == NULL)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson no world - use GWorld->GetWorld");
		currentWorld = GWorld->GetWorld();
		myworld = currentWorld;
	}

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	return GetSceneData(currentSceneName);
}

TSharedPtr<FSceneData> FAnalyticsProviderCognitiveVR::GetSceneData(FString scenename)
{
	for (int i = 0; i < SceneData.Num(); i++)
	{
		if (!SceneData[i].IsValid()) { continue; }
		if (SceneData[i]->Name == scenename)
		{
			return SceneData[i];
		}
	}
	GLog->Log("FAnalyticsProviderCognitiveVR::GetSceneData couldn't find SceneData for scene " + scenename);
	return NULL;
}

void FAnalyticsProviderCognitiveVR::CacheSceneData()
{
	TArray<FString>scenstrings;
	FString TestSyncFile = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenstrings, TestSyncFile);

	for (int i = 0; i < scenstrings.Num(); i++)
	{
		TArray<FString> Array;
		scenstrings[i].ParseIntoArray(Array, TEXT(","), true);

		if (Array.Num() == 2) //scenename,sceneid
		{
			//old scene data. append versionnumber and versionid
			Array.Add("1");
			Array.Add("0");
		}

		if (Array.Num() != 4)
		{
			GLog->Log("failed to parse " + scenstrings[i]);
			continue;
		}

		FSceneData* tempscene = new FSceneData(Array[0], Array[1], FCString::Atoi(*Array[2]), FCString::Atoi(*Array[3]));
		SceneData.Add(MakeShareable(tempscene));
	}
}

bool FAnalyticsProviderCognitiveVR::SendJson(FString url, FString json)
{
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (cog == NULL)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return false;
	}

	if (Http == NULL)
	{
		Http = &FHttpModule::Get();
	}

	if (Http == NULL)
	{
		CognitiveLog::Warning("Cognitive Provider::SendJson Http module not initialized! likely hasn't started session");
		return false;
	}

	//json to scene endpoint

	TSharedRef<IHttpRequest> HttpRequest = Http->CreateRequest();
	HttpRequest->SetContentAsString(json);
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->ProcessRequest();
	return true;
}

FVector FAnalyticsProviderCognitiveVR::GetPlayerHMDPosition()
{
	//TODO cache this. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::GetPlayerHMDPosition no controllers skip");
		return FVector();
	}

	return controllers[0]->PlayerCameraManager->GetCameraLocation();
}