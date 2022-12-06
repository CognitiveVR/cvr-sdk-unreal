// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Public/CognitiveVRProvider.h"
//IMPROVEMENT this should be in the header, but can't find ControllerType enum
#include "CognitiveVR/Public/InputTracker.h"

IMPLEMENT_MODULE(FAnalyticsCognitiveVR, CognitiveVR);

void FAnalyticsCognitiveVR::StartupModule()
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = MakeShareable(new FAnalyticsProviderCognitiveVR());
	AnalyticsProvider = cog;
	CognitiveVRProvider = TWeakPtr<FAnalyticsProviderCognitiveVR>(cog);
	GLog->Log("AnalyticsCognitiveVR::StartupModule");
}

void FAnalyticsProviderCognitiveVR::HandleSublevelLoaded(ULevel* level, UWorld* world)
{
	GLog->Log("Loaded level: " + level->GetFullGroupName(true)); //Additive
}

void FAnalyticsProviderCognitiveVR::HandleSublevelUnloaded(ULevel* level, UWorld* world)
{
	GLog->Log("----------------C3D HandleSublevelUnloaded");

	GLog->Log("POST LEVEL LOAD");

	if (level != nullptr)
	{
		GLog->Log("Unloaded level: " + level->GetFullGroupName(true)); //Additive??
	}
	else
		GLog->Log("level is null");
	if (world != nullptr)
	{
		auto worldlevel = world->GetCurrentLevel();
		if (worldlevel != nullptr)
			GLog->Log("unloaded world level " + worldlevel->GetFullGroupName(true));
		else
			GLog->Log("unloaded " + world->GetFName().ToString());
	}
	else
		GLog->Log("world is null");
}


void FAnalyticsProviderCognitiveVR::HandlePostLevelLoad(UWorld* world)
{
	GLog->Log("----------------C3D HandlePostLevelLoad");
	auto level = world->GetCurrentLevel();

	if (level != nullptr)
	{
		GLog->Log("level post load level: " + level->GetFullGroupName(true));
	}
	else
		GLog->Log("level post load level is null");
	if (world != nullptr)
	{
		auto worldlevel = world->GetCurrentLevel();
		if (worldlevel != nullptr)
			GLog->Log("level post load world level " + worldlevel->GetFullGroupName(true));
		else
			GLog->Log("level post load unloaded " + world->GetFName().ToString());
	}
	else
		GLog->Log("level post load world is null");
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

UWorld* FAnalyticsProviderCognitiveVR::GetWorld()
{
	return currentWorld;
}

bool FAnalyticsProviderCognitiveVR::StartSession()
{
	return StartSession(TArray<FAnalyticsEventAttribute>());
}

bool FAnalyticsProviderCognitiveVR::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	if (bHasSessionStarted)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::StartSession already started");
		return false;
	}

	auto cognitiveActor = ACognitiveActor::GetCognitiveActor();
	if (cognitiveActor == nullptr)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::StartSession could not find Cognitive Actor in your level");
		return false;
	}
	currentWorld = cognitiveActor->GetWorld();
	if (currentWorld == NULL)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::StartSession World not set. Are you missing a Cognitive Actor in your level?");
		return false;
	}


	//initialize log
	CognitiveLog::Init();

	//construct data recording streams
	exitpoll = MakeShareable(new ExitPoll());
	customEventRecorder = NewObject<UCustomEventRecorder>(); //constructor. beginplay?
	sensors = NewObject<USensors>();
	localCache = MakeShareable(new LocalCache(FPaths::GeneratedConfigDir()));
	network = MakeShareable(new Network());
	dynamicObjectManager = NewObject<UDynamicObjectManager>();

	//pre session startup
	CacheSceneData();
	SessionTimestamp = Util::GetTimestamp();
	if (SessionId.IsEmpty())
	{
		SessionId = FString::FromInt(GetSessionTimestamp()) + TEXT("_") + DeviceId;
	}
	ApplicationKey = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "ApiKey", false);
	AttributionKey = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "AttributionKey", false);
	bHasSessionStarted = true;

	//register delegates
	if (!PauseHandle.IsValid())
	{
		PauseHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddRaw(this, &FAnalyticsProviderCognitiveVR::HandleApplicationWillEnterBackground);
	}
	if (!LevelLoadHandle.IsValid())
		LevelLoadHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FAnalyticsProviderCognitiveVR::HandlePostLevelLoad);
	if (!SublevelLoadedHandle.IsValid())
		SublevelLoadedHandle = FWorldDelegates::LevelAddedToWorld.AddRaw(this, &FAnalyticsProviderCognitiveVR::HandleSublevelLoaded);
	if (!SublevelUnloadedHandle.IsValid())
		SublevelUnloadedHandle = FWorldDelegates::LevelRemovedFromWorld.AddRaw(this, &FAnalyticsProviderCognitiveVR::HandleSublevelUnloaded);

	//set session properties
	Util::SetSessionProperties();
	for (auto Attr : Attributes)
	{
		SetSessionProperty(Attr.GetName(), Attr.GetValue());
	}
	if (!GetCognitiveUserName().IsEmpty())
	{
		SetParticipantFullName(GetCognitiveUserName());
	}
	if (currentWorld->WorldType == EWorldType::Game)
	{
		SetSessionProperty("c3d.app.inEditor", "false");
	}
	else
	{
		SetSessionProperty("c3d.app.inEditor", "true");
	}

	cognitiveActor->OnSessionBegin.Broadcast();

	CognitiveLog::Info("FAnalyticsProviderCognitiveVR::StartSession");

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
	if (bHasSessionStarted == false)
	{
		return;
	}

	//broadcast pre session end for pre-cleanup
	auto cognitiveActor = ACognitiveActor::GetCognitiveActor();
	if (cognitiveActor == nullptr)
	{
		return;
	}
	cognitiveActor->OnPreSessionEnd.Broadcast();
	FlushAndCacheEvents();

	//OnPostSessionEnd broadcast. used by components to clean up anything, including delegates
	cognitiveActor->OnPostSessionEnd.Broadcast();

	//reset components and uobjects
	network.Reset();
	customEventRecorder = nullptr;
	sensors = nullptr;
	UCognitiveVRBlueprints::cog.Reset();
	UCustomEvent::cog.Reset();
	dynamicObjectManager = nullptr;
	if (localCache.IsValid())
	{
		localCache->Close();
		localCache.Reset();
	}

	//cleanup pause and level load delegates
	if (!PauseHandle.IsValid())
		FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(PauseHandle);
	if (!LevelLoadHandle.IsValid())
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(LevelLoadHandle);
	if (!SublevelLoadedHandle.IsValid())
		FWorldDelegates::LevelAddedToWorld.Remove(SublevelLoadedHandle);
	if (!SublevelUnloadedHandle.IsValid())
		FWorldDelegates::LevelRemovedFromWorld.Remove(SublevelUnloadedHandle);

	//reset variables
	SessionTimestamp = -1;
	SessionId = "";
	bHasCustomSessionName = false;
	bHasSessionStarted = false;
	currentWorld = NULL;

	//broadcast end session
	CognitiveLog::Info("CognitiveVR EndSession");
}

//broadcast to all listeners that outstanding data should be sent + written to cache as a backup
//should be used when it's unclear if the session will continue - a sudden pause event might indicate a loss of wifi connection
void FAnalyticsProviderCognitiveVR::FlushAndCacheEvents()
{
	if (!bHasSessionStarted) { CognitiveLog::Warning("CognitiveVR Flush Events, but Session has not started!"); return; }

	auto cognitiveActor = ACognitiveActor::GetCognitiveActor();
	if (cognitiveActor == nullptr)
	{
		return;
	}
	cognitiveActor->OnRequestSend.Broadcast(true);
}

//broadcast to all listeners that outstanding data should be sent. this won't immediately write all data to the cache
//should be prefered when the session will likely continue, but at a point when sending data won't be noticed (eg, win screen popup, etc)
void FAnalyticsProviderCognitiveVR::FlushEvents()
{
	if (!bHasSessionStarted) { CognitiveLog::Warning("CognitiveVR Flush Events, but Session has not started!"); return; }

	auto cognitiveActor = ACognitiveActor::GetCognitiveActor();
	if (cognitiveActor == nullptr)
	{
		return;
	}
	cognitiveActor->OnRequestSend.Broadcast(false);
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

FString FAnalyticsProviderCognitiveVR::GetCognitiveUserName() const
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

void FAnalyticsProviderCognitiveVR::HandleApplicationWillEnterBackground()
{
	FlushAndCacheEvents();
	if (!localCache.IsValid())
	{
		return;
	}
	localCache->SerializeToFile();
}