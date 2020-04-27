// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Public/CognitiveVR.h"
#include "Public/CognitiveVRProvider.h"
//IMPROVEMENT this should be in the header, but can't find ControllerType enum
#include "Public/InputTracker.h"

IMPLEMENT_MODULE(FAnalyticsCognitiveVR, CognitiveVR);

bool FAnalyticsProviderCognitiveVR::bHasSessionStarted = false;
UWorld* FAnalyticsProviderCognitiveVR::currentWorld;

void FAnalyticsCognitiveVR::StartupModule()
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = MakeShareable(new FAnalyticsProviderCognitiveVR());
	AnalyticsProvider = cog;
	CognitiveVRProvider = TWeakPtr<FAnalyticsProviderCognitiveVR>(cog);
	GLog->Log("AnalyticsCognitiveVR::StartupModule");
}

void FAnalyticsCognitiveVR::ShutdownModule()
{

}

bool FAnalyticsProviderCognitiveVR::HasStartedSession()
{
	return bHasSessionStarted;
}

TSharedPtr<IAnalyticsProvider> FAnalyticsCognitiveVR::CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const
{
	return AnalyticsProvider;
}

TWeakPtr<FAnalyticsProviderCognitiveVR> FAnalyticsCognitiveVR::GetCognitiveVRProvider() const
{
	return CognitiveVRProvider;
}

FAnalyticsProviderCognitiveVR::FAnalyticsProviderCognitiveVR()
{
	DeviceId = FPlatformMisc::GetDeviceId();
}

FAnalyticsProviderCognitiveVR::~FAnalyticsProviderCognitiveVR()
{
	UE_LOG(LogTemp, Warning, TEXT("FAnalyticsProviderCognitiveVR Shutdown Module"));
}


void FAnalyticsProviderCognitiveVR::SetWorld(UWorld* world)
{
	currentWorld = world;
}

UWorld* FAnalyticsProviderCognitiveVR::GetWorld()
{
	return currentWorld;
}

UWorld* FAnalyticsProviderCognitiveVR::EnsureGetWorld()
{
	if (currentWorld == NULL)
	{
		for (TObjectIterator<UPlayerTracker> Itr; Itr; ++Itr)
		{
			Itr->BeginPlay();
		}
	}

	return currentWorld;
}

bool FAnalyticsProviderCognitiveVR::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	CognitiveLog::Init();

	if (bHasSessionStarted)
	{
		return false;
		//EndSession();
		//return false;
	}

	UPlayerTracker* pt = UPlayerTracker::GetPlayerTracker();

	if (pt == NULL) { return false; }

	if (currentWorld == NULL)
	{
		GLog->Log("FAnalyticsProviderCognitiveVR::StartSession World not set. Are you missing a Cognitive3D::Player Tracker component on your camera?");
		pt->OnSessionBegin.Broadcast(false);
		return false;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());

	SessionTimestamp = Util::GetTimestamp();
	if (SessionId.IsEmpty())
	{
		SessionId = FString::FromInt(GetSessionTimestamp()) + TEXT("_") + DeviceId;
	}

	for (auto Attr : Attributes)
	{
		properties->SetStringField(Attr.AttrName, Attr.AttrValueString);
	}

	APIKey = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "ApiKey", false);

	network = MakeShareable(new Network());
	customEventRecorder = MakeShareable(new CustomEventRecorder());
	sensors = MakeShareable(new Sensors());

	customEventRecorder->StartSession();
	sensors->StartSession();

	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::StartSession");
	
	CacheSceneData();

	CognitiveLog::Info("CognitiveVR InitCallback Response");

	//-----------------------'response'

	bHasSessionStarted = true;

	if (!network.IsValid())
	{
		CognitiveLog::Warning("CognitiveVRProvider InitCallback network is null");
		pt->OnSessionBegin.Broadcast(false);
		return false;
	}

	Util::SetHardwareSessionProperties();

	FString HMDDeviceName = UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName().ToString();
	SetSessionProperty("c3d.device.hmd.type", HMDDeviceName);

	if (HMDDeviceName.Contains("vive"))
	{
		SetSessionProperty("c3d.device.manufacturer", "HTC");
	}
	else if (HMDDeviceName.Contains("oculus"))
	{
		SetSessionProperty("c3d.device.manufacturer", "Oculus");
	}
	else if (HMDDeviceName.Contains("microsoft"))
	{
		SetSessionProperty("c3d.device.manufacturer", "Microsoft");
	}
	else
	{
		SetSessionProperty("c3d.device.manufacturer", "Unknown");
	}

#if defined TOBII_EYETRACKING_ACTIVE
	SetSessionProperty("c3d.device.eyetracking.enabled", true);
	SetSessionProperty("c3d.device.eyetracking.type", "Tobii");
#else
	SetSessionProperty("c3d.device.eyetracking.enabled", false);
	SetSessionProperty("c3d.device.eyetracking.type", "None");
#endif



	if (currentWorld->WorldType == EWorldType::Game)
	{
		SetSessionProperty("c3d.app.inEditor", "false");
	}
	else
	{
		SetSessionProperty("c3d.app.inEditor", "true");
	}

	SetSessionProperty("c3d.app.sdktype", "Default");

	SetSessionProperty("c3d.app.engine", "Unreal");

	SetParticipantFullName(GetUserName());
	SetParticipantId(GetUserID());
	SetSessionProperty("c3d.deviceid", GetDeviceID());
	if (!SessionProperties.HasField("c3d.sessionname"))
	{
		SetSessionProperty("c3d.sessionname", SessionId);
	}

	customEventRecorder->Send(FString("c3d.sessionStart"));
	
	UDynamicObject::OnSessionBegin();

	pt->OnSessionBegin.Broadcast(true);

	AInputTracker* it = AInputTracker::GetInputTracker();
	if (it != NULL)
	{
		it->FindControllers(false);
	}

	return bHasSessionStarted;
}

void FAnalyticsProviderCognitiveVR::SetLobbyId(FString lobbyId)
{
	LobbyId = lobbyId;
}

void FAnalyticsProviderCognitiveVR::SetSessionName(FString sessionName)
{
	SetSessionProperty("c3d.sessionname", sessionName);
}

void FAnalyticsProviderCognitiveVR::EndSession()
{
	if (!customEventRecorder.IsValid())
	{
		return;
	}

	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::EndSession");

	//bPendingInitRequest = false;

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	properties->SetNumberField("sessionlength", Util::GetTimestamp() - GetSessionTimestamp());

	customEventRecorder->Send(FString("c3d.sessionEnd"), properties);

	FlushEvents();
	CognitiveLog::Info("Freeing CognitiveVR memory.");

	//delete network;
	network.Reset();

	//delete transaction;
	customEventRecorder.Reset();

	//delete sensors;
	sensors.Reset();

	UCognitiveVRBlueprints::cog.Reset();

	UCustomEvent::cog.Reset();


	for (TObjectIterator<UFixationRecorder> Itr; Itr; ++Itr)
	{
		Itr->EndSession();
	}

	SessionTimestamp = -1;
	SessionId = "";

	bHasSessionStarted = false;

	UDynamicObject::OnSessionEnd();

	CognitiveLog::Info("CognitiveVR EndSession");
	currentWorld = NULL;
}

void FAnalyticsProviderCognitiveVR::FlushEvents()
{
	//send to dashboard
	this->customEventRecorder->SendData();

	//send to scene explorer
	sensors->SendData();
	UDynamicObject::SendData();

	auto pt = UPlayerTracker::GetPlayerTracker();
	if (pt != NULL)
		pt->SendData();

	auto fix = UFixationRecorder::GetFixationRecorder();
	if (fix != NULL)
		fix->SendData();
}

void FAnalyticsProviderCognitiveVR::SetUserID(const FString& InUserID)
{
	ParticipantId = InUserID;
	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetUserID set user id");
}

void FAnalyticsProviderCognitiveVR::SetParticipantId(FString participantId)
{
	ParticipantId = participantId;
	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetParticipantData set user id");
}

void FAnalyticsProviderCognitiveVR::SetParticipantFullName(FString participantName)
{
	ParticipantName = participantName;
	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetParticipantData set user id");
}

FString FAnalyticsProviderCognitiveVR::GetUserID() const
{
	return ParticipantId;
}

FString FAnalyticsProviderCognitiveVR::GetUserName() const
{
	return ParticipantName;
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
		SessionId = InSessionID;
	}
	else
	{
		// Log that we shouldn't switch session ids during a session
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordEvent while a session is in progress. Ignoring");
	}
	return !bHasSessionStarted;
}

//built in analytics stuff
void FAnalyticsProviderCognitiveVR::RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bHasSessionStarted)
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);

		for (auto Attr : Attributes)
		{
			properties->SetStringField(Attr.AttrName, Attr.AttrValueString);
		}

		customEventRecorder->Send(EventName, properties);
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

		customEventRecorder->Send("c3d.recorditempurchase", properties);
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

		customEventRecorder->Send("c3d.recordcurrencypurchase", properties);
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

		customEventRecorder->Send(FString("c3d.recordcurrencygiven"), properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordCurrencyGiven called before StartSession. Ignoring");
	}
}

void FAnalyticsProviderCognitiveVR::SetAge(const int32 InAge)
{
	SetParticipantProperty("age", InAge);
}

void FAnalyticsProviderCognitiveVR::SetLocation(const FString& InLocation)
{
	SetSessionProperty("location", InLocation);
}

void FAnalyticsProviderCognitiveVR::SetGender(const FString& InGender)
{
	SetParticipantProperty("gender", InGender);
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
			properties->SetStringField(Attr.AttrName, Attr.AttrValueString);
		}

		customEventRecorder->Send(FString("c3d.recorderror"), properties);

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
			properties->SetStringField(Attr.AttrName, Attr.AttrValueString);
		}

		customEventRecorder->Send(FString("c3d.recordprogress"), properties);
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
			properties->SetStringField(Attr.AttrName, Attr.AttrValueString);
		}

		customEventRecorder->Send(FString("c3d.recorditempurchase"), properties);
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
			properties->SetStringField(Attr.AttrName, Attr.AttrValueString);
		}

		customEventRecorder->Send(FString("RecordCurrencyPurchase"), properties);
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
			properties->SetStringField(Attr.AttrName, Attr.AttrValueString);
		}

		customEventRecorder->Send(FString("c3d.recordcurrencygiven"), properties);
	}
	else
	{
		CognitiveLog::Warning("FAnalyticsProvideCognitiveVR::RecordCurrencyGiven called before StartSession. Ignoring");
	}
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
	CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::GetSceneData couldn't find SceneData for scene " + scenename);
	return NULL;
}

FString FAnalyticsProviderCognitiveVR::GetCurrentSceneId()
{
	auto currentData = GetCurrentSceneData();
	if (!currentData.IsValid()) { return ""; }

	return currentData->Id;
}

FString FAnalyticsProviderCognitiveVR::GetCurrentSceneVersionNumber()
{
	auto currentData = GetCurrentSceneData();
	if (!currentData.IsValid()) { return ""; }

	return FString::FromInt(currentData->VersionNumber);
}

void FAnalyticsProviderCognitiveVR::CacheSceneData()
{
	TArray<FString>scenstrings;
	FString TestSyncFile = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
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

		SceneData.Add(MakeShareable(new FSceneData(Array[0], Array[1], FCString::Atoi(*Array[2]), FCString::Atoi(*Array[3]))));
	}
}

FVector FAnalyticsProviderCognitiveVR::GetPlayerHMDPosition()
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::GetPlayerHMDPosition no controllers skip");
		return FVector();
	}

	return controllers[0]->PlayerCameraManager->GetCameraLocation();
}

void FAnalyticsProviderCognitiveVR::SetSessionProperty(FString name, int32 value)
{
	if (SessionProperties.HasField(name))
		SessionProperties.Values[name] = MakeShareable(new FJsonValueNumber(value));
	else
		SessionProperties.SetNumberField(name, (int32)value);
}
void FAnalyticsProviderCognitiveVR::SetSessionProperty(FString name, float value)
{
	if (SessionProperties.HasField(name))
		SessionProperties.Values[name] = MakeShareable(new FJsonValueNumber(value));
	else
		SessionProperties.SetNumberField(name, (float)value);
}
void FAnalyticsProviderCognitiveVR::SetSessionProperty(FString name, FString value)
{
	if (SessionProperties.HasField(name))
		SessionProperties.Values[name] = MakeShareable(new FJsonValueString(value));
	else
		SessionProperties.SetStringField(name, value);
}

void FAnalyticsProviderCognitiveVR::SetParticipantProperty(FString name, int32 value)
{
	FString completeName = "c3d.participant." + name;
	if (SessionProperties.HasField(completeName))
		SessionProperties.Values[completeName] = MakeShareable(new FJsonValueNumber(value));
	else
		SessionProperties.SetNumberField(completeName, (int32)value);
}
void FAnalyticsProviderCognitiveVR::SetParticipantProperty(FString name, float value)
{
	FString completeName = "c3d.participant." + name;
	if (SessionProperties.HasField(completeName))
		SessionProperties.Values[completeName] = MakeShareable(new FJsonValueNumber(value));
	else
		SessionProperties.SetNumberField(completeName, (float)value);
}
void FAnalyticsProviderCognitiveVR::SetParticipantProperty(FString name, FString value)
{
	FString completeName = "c3d.participant." + name;
	if (SessionProperties.HasField(completeName))
		SessionProperties.Values[completeName] = MakeShareable(new FJsonValueString(value));
	else
		SessionProperties.SetStringField(completeName, value);
}

FJsonObject FAnalyticsProviderCognitiveVR::GetSessionProperties()
{
	FJsonObject returnobject = FJsonObject(SessionProperties);
	SessionProperties = FJsonObject();
	return returnobject;
}
