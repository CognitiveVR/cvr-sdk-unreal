// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Public/CognitiveVRProvider.h"
//IMPROVEMENT this should be in the header, but can't find ControllerType enum
#include "CognitiveVR/Public/InputTracker.h"

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
#if PLATFORM_ANDROID
	DeviceId = FPlatformMisc::GetDeviceId();
#else
	DeviceId = FPlatformMisc::GetHashedMacAddressString();
#endif
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

bool FAnalyticsProviderCognitiveVR::StartSession()
{
	return StartSession(TArray<FAnalyticsEventAttribute>());
}

bool FAnalyticsProviderCognitiveVR::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	CognitiveLog::Init();

	if (bHasSessionStarted)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::StartSession already started");
		return false;
	}

	UPlayerTracker* pt = UPlayerTracker::GetPlayerTracker();

	if (pt == NULL)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::StartSession could not find PlayerTracker component in level");
		return false;
	}

	if (currentWorld == NULL)
		currentWorld = pt->GetWorld();

	if (currentWorld == NULL)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::StartSession World not set. Are you missing a Cognitive3D::Player Tracker component on your camera?");
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
		properties->SetStringField(Attr.GetName(), Attr.GetValue());
	}

	ApplicationKey = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "ApiKey", false);
	AttributionKey = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "AttributionKey", false);

	exitpoll = MakeShareable(new ExitPoll());
	customEventRecorder = MakeShareable(new CustomEventRecorder());
	sensors = MakeShareable(new Sensors());
	localCache = MakeShareable(new LocalCache(FPaths::GeneratedConfigDir()));
	network = MakeShareable(new Network());

	customEventRecorder->StartSession();
	sensors->StartSession();

	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::StartSession");
	
	CacheSceneData();

	CognitiveLog::Info("CognitiveVR InitCallback Response");

	//-----------------------'response'

	bHasSessionStarted = true;

	if (!network.IsValid())
	{
		CognitiveLog::Warning("CognitiveVRProvider Network is null");
		pt->OnSessionBegin.Broadcast(false);
		return false;
	}

	Util::SetSessionProperties();
	if (!GetUserName().IsEmpty())
		SetParticipantFullName(GetUserName());
	if (currentWorld->WorldType == EWorldType::Game)
	{
		SetSessionProperty("c3d.app.inEditor", "false");
	}
	else
	{
		SetSessionProperty("c3d.app.inEditor", "true");
	}

	customEventRecorder->Send(FString("c3d.sessionStart"));
	
	UDynamicObject::OnSessionBegin();

	pt->OnSessionBegin.Broadcast(true);

	for (TObjectIterator<AInputTracker> Itr; Itr; ++Itr)
	{
		if ((*Itr)->GetWorld() == NULL) { continue; }
		if (!(*Itr)->GetWorld()->IsGameWorld()) { continue; }
		(*Itr)->FindControllers(false);
		break;
	}

	auto fixationRecorder = UFixationRecorder::GetFixationRecorder();
	if (fixationRecorder != NULL)
	{
		fixationRecorder->BeginSession();
	}

	return bHasSessionStarted;
}

void FAnalyticsProviderCognitiveVR::SetLobbyId(FString lobbyId)
{
	LobbyId = lobbyId;
}

void FAnalyticsProviderCognitiveVR::SetSessionName(FString sessionName)
{
	bHasCustomSessionName = true;
	SetSessionProperty("c3d.sessionname", sessionName);
}

void FAnalyticsProviderCognitiveVR::EndSession()
{
	if (!customEventRecorder.IsValid())
	{
		return;
	}
	if (bHasSessionStarted == false)
	{
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	properties->SetNumberField("sessionlength", Util::GetTimestamp() - GetSessionTimestamp());
	customEventRecorder->Send(FString("c3d.sessionEnd"), properties);

	FlushAndCacheEvents();

	network.Reset();
	customEventRecorder.Reset();
	sensors.Reset();
	UCognitiveVRBlueprints::cog.Reset();
	UCustomEvent::cog.Reset();

	for (TObjectIterator<UFixationRecorder> Itr; Itr; ++Itr)
	{
		Itr->EndSession();
	}

	SessionTimestamp = -1;
	SessionId = "";

	bHasCustomSessionName = false;
	bHasSessionStarted = false;

	UDynamicObject::OnSessionEnd();

	CognitiveLog::Info("CognitiveVR EndSession");
	currentWorld = NULL;
	if (localCache != nullptr)
	{
		localCache->Close();
		localCache = nullptr;
	}
}

void FAnalyticsProviderCognitiveVR::FlushAndCacheEvents()
{
	if (!bHasSessionStarted) { CognitiveLog::Warning("CognitiveVR Flush Events, but Session has not started!"); return; }

	this->customEventRecorder->SendData(true);
	sensors->SendData(true);
	UDynamicObject::SendData(true);

	auto pt = UPlayerTracker::GetPlayerTracker();
	if (pt != NULL)
		pt->SendData(true);

	auto fix = UFixationRecorder::GetFixationRecorder();
	if (fix != NULL)
		fix->SendData(true);
}

void FAnalyticsProviderCognitiveVR::FlushEvents()
{
	if (!bHasSessionStarted) { CognitiveLog::Warning("CognitiveVR Flush Events, but Session has not started!"); return; }

	this->customEventRecorder->SendData();
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
	FString userId = InUserID;
	if (userId.Len() > 64)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::SetUserID exceeds maximum character limit. clipping to 64");
		int32 chopcount = userId.Len() - 64;
		userId = userId.LeftChop(chopcount);
	}

	ParticipantId = userId;
	SetParticipantProperty("id", userId);
	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetUserID set user id: " + userId);
}

void FAnalyticsProviderCognitiveVR::SetParticipantId(FString participantId)
{
	if (participantId.Len() > 64)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::SetParticipantId exceeds maximum character limit. clipping to 64");
		int32 chopcount = participantId.Len() - 64;
		participantId = participantId.LeftChop(chopcount);
	}

	ParticipantId = participantId;
	SetParticipantProperty("id", participantId);
	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetParticipantId set user id: " + participantId);
}

void FAnalyticsProviderCognitiveVR::SetParticipantFullName(FString participantName)
{
	ParticipantName = participantName;
	SetParticipantProperty("name", participantName);
	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::SetParticipantData set user id");
	if (!bHasCustomSessionName)
		SetSessionProperty("c3d.sessionname", participantName);
}

void FAnalyticsProviderCognitiveVR::SetSessionTag(FString Tag)
{
	if (Tag.Len() == 0)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::SetSessionTag must contain > 0 characters");
		return;
	}
	if (Tag.Len() > 12)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::SetSessionTag must contain <= 12 characters");
		return;
	}

	SetSessionProperty("c3d.session_tag."+Tag, "true");
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
			properties->SetStringField(Attr.GetName(), Attr.GetValue());
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
			properties->SetStringField(Attr.GetName(), Attr.GetValue());
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
			properties->SetStringField(Attr.GetName(), Attr.GetValue());
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
			properties->SetStringField(Attr.GetName(), Attr.GetValue());
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
			properties->SetStringField(Attr.GetName(), Attr.GetValue());
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
			properties->SetStringField(Attr.GetName(), Attr.GetValue());
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
	if (currentWorld == NULL)
	{
		currentWorld = GWorld->GetWorld();
	}

	//what is the current scenename
	FString sceneName = currentWorld->GetMapName();
	sceneName.RemoveFromStart(currentWorld->StreamingLevelsPrefix);

	//if names match, return cached scene data
	if (LastSceneData.IsValid() && LastSceneData->Name == sceneName)
	{
		return LastSceneData;
	}

	//check the sublevel scene names
 const TArray<ULevelStreaming*> streamedLevels = GetWorld()->GetStreamingLevels();
	for (ULevelStreaming* streamingLevel : streamedLevels)
	{
		FString sublevelName = FPackageName::GetShortFName(streamingLevel->GetWorldAssetPackageFName()).ToString();
		sublevelName.RemoveFromStart(currentWorld->StreamingLevelsPrefix);
		//if names match, return cached scene data
		if (LastSceneData.IsValid() && LastSceneData->Name == sublevelName)
		{
			return LastSceneData;
		}
	}

	//last scene data is invalid OR last scene is not loaded
	//need to change LastSceneData
	
	//find a scene data (prefering main level)
	TSharedPtr<FSceneData> mainScenePtr = GetSceneData(sceneName);
	if (mainScenePtr.IsValid())
	{
		ForceWriteSessionMetadata = true;
		CurrentTrackingSceneId = mainScenePtr->Id;
		LastSceneData = mainScenePtr;
		return LastSceneData;
	}

	//loop through all subscenes to find one
	for (ULevelStreaming* streamingLevel : streamedLevels)
	{
		FString sublevelName = FPackageName::GetShortFName(streamingLevel->GetWorldAssetPackageFName()).ToString();
		sublevelName.RemoveFromStart(currentWorld->StreamingLevelsPrefix);
		TSharedPtr<FSceneData> subScenePtr = GetSceneData(sublevelName);

		if (subScenePtr.IsValid())
		{
			ForceWriteSessionMetadata = true;
			CurrentTrackingSceneId = subScenePtr->Id;
			LastSceneData = subScenePtr;
			return LastSceneData;
		}
	}

	return NULL;
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
	//CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::GetSceneData couldn't find SceneData for scene " + scenename);
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
	SceneData.Empty();

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
			CognitiveLog::Error("FAnalyticsProviderCognitiveVR::CacheSceneData failed to parse " + scenstrings[i]);
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
	if (NewSessionProperties.HasField(name))
		NewSessionProperties.Values[name] = MakeShareable(new FJsonValueNumber(value));
	else
		NewSessionProperties.SetNumberField(name, (int32)value);
	if (AllSessionProperties.HasField(name))
		AllSessionProperties.Values[name] = MakeShareable(new FJsonValueNumber(value));
	else
		AllSessionProperties.SetNumberField(name, (int32)value);
}
void FAnalyticsProviderCognitiveVR::SetSessionProperty(FString name, float value)
{
	if (NewSessionProperties.HasField(name))
		NewSessionProperties.Values[name] = MakeShareable(new FJsonValueNumber(value));
	else
		NewSessionProperties.SetNumberField(name, (float)value);
	if (AllSessionProperties.HasField(name))
		AllSessionProperties.Values[name] = MakeShareable(new FJsonValueNumber(value));
	else
		AllSessionProperties.SetNumberField(name, (float)value);
}
void FAnalyticsProviderCognitiveVR::SetSessionProperty(FString name, FString value)
{
	if (NewSessionProperties.HasField(name))
		NewSessionProperties.Values[name] = MakeShareable(new FJsonValueString(value));
	else
		NewSessionProperties.SetStringField(name, value);
	if (AllSessionProperties.HasField(name))
		AllSessionProperties.Values[name] = MakeShareable(new FJsonValueString(value));
	else
		AllSessionProperties.SetStringField(name, value);
}

void FAnalyticsProviderCognitiveVR::SetParticipantProperty(FString name, int32 value)
{
	FString completeName = "c3d.participant." + name;
	if (NewSessionProperties.HasField(completeName))
		NewSessionProperties.Values[completeName] = MakeShareable(new FJsonValueNumber(value));
	else
		NewSessionProperties.SetNumberField(completeName, (int32)value);
	if (AllSessionProperties.HasField(completeName))
		AllSessionProperties.Values[completeName] = MakeShareable(new FJsonValueNumber(value));
	else
		AllSessionProperties.SetNumberField(completeName, (int32)value);
}
void FAnalyticsProviderCognitiveVR::SetParticipantProperty(FString name, float value)
{
	FString completeName = "c3d.participant." + name;
	if (NewSessionProperties.HasField(completeName))
		NewSessionProperties.Values[completeName] = MakeShareable(new FJsonValueNumber(value));
	else
		NewSessionProperties.SetNumberField(completeName, (float)value);
	if (AllSessionProperties.HasField(completeName))
		AllSessionProperties.Values[completeName] = MakeShareable(new FJsonValueNumber(value));
	else
		AllSessionProperties.SetNumberField(completeName, (float)value);
}
void FAnalyticsProviderCognitiveVR::SetParticipantProperty(FString name, FString value)
{
	FString completeName = "c3d.participant." + name;
	if (NewSessionProperties.HasField(completeName))
		NewSessionProperties.Values[completeName] = MakeShareable(new FJsonValueString(value));
	else
		NewSessionProperties.SetStringField(completeName, value);
	if (AllSessionProperties.HasField(completeName))
		AllSessionProperties.Values[completeName] = MakeShareable(new FJsonValueString(value));
	else
		AllSessionProperties.SetStringField(completeName, value);
}

FJsonObject FAnalyticsProviderCognitiveVR::GetNewSessionProperties()
{
	FJsonObject returnobject = FJsonObject(NewSessionProperties);
	NewSessionProperties = FJsonObject();
	return returnobject;
}

FJsonObject FAnalyticsProviderCognitiveVR::GetAllSessionProperties()
{
	FJsonObject returnobject = FJsonObject(AllSessionProperties);
	return returnobject;
}

FString FAnalyticsProviderCognitiveVR::GetAttributionParameters()
{
	TSharedPtr<FJsonObject> parameters = MakeShareable(new FJsonObject);

	int32 versionId = 0;
	auto scene = GetCurrentSceneData();
	if (scene.IsValid())
		versionId = scene->VersionId;

	parameters->SetNumberField("sceneVersionId", versionId);
	parameters->SetStringField("sessionId", SessionId);
	parameters->SetStringField("attributionKey", AttributionKey);
	
	//json to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(parameters.ToSharedRef(), Writer);

	//string to base64
	FString baseString = FBase64::Encode(OutputString);

	return FString("?c3dAtkd=AK-"+ baseString);
}
