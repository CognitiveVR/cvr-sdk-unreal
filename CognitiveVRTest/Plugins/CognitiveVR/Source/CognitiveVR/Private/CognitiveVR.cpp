// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Public/CognitiveVRProvider.h"
#include "Classes/Camera/CameraComponent.h"
//IMPROVEMENT this should be in the header, but can't find ControllerType enum
#include "CognitiveVR/Private/InputTracker.h"

IMPLEMENT_MODULE(FAnalyticsCognitiveVR, CognitiveVR);

bool FAnalyticsProviderCognitiveVR::bHasSessionStarted = false;

void FAnalyticsCognitiveVR::StartupModule()
{
	GLog->Log("AnalyticsCognitiveVR::StartupModule");

	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = MakeShareable(new FAnalyticsProviderCognitiveVR());
	AnalyticsProvider = cog;
	CognitiveVRProvider = TWeakPtr<FAnalyticsProviderCognitiveVR>(cog);

	//create non-initialized data collectors and other internal stuff
	CognitiveVRProvider.Pin()->exitpoll = MakeShareable(new ExitPoll());
	CognitiveVRProvider.Pin()->customEventRecorder = new UCustomEventRecorder();
	CognitiveVRProvider.Pin()->sensors = new USensors();
	CognitiveVRProvider.Pin()->fixationDataRecorder = new UFixationDataRecorder();
	CognitiveVRProvider.Pin()->gazeDataRecorder = new UGazeDataRecorder();
	CognitiveVRProvider.Pin()->localCache = MakeShareable(new LocalCache(FPaths::GeneratedConfigDir()));
	CognitiveVRProvider.Pin()->network = MakeShareable(new Network());
	CognitiveVRProvider.Pin()->dynamicObjectManager = new UDynamicObjectManager();
}

void FAnalyticsProviderCognitiveVR::HandleSublevelLoaded(ULevel* level, UWorld* world)
{
	if (level == nullptr)
	{
		FlushAndCacheEvents();
		return;
	}

	if (!AutomaticallySetTrackingScene) { return; }

	FString levelName = level->GetFullGroupName(true);
	//GLog->Log("FAnalyticsProviderCognitiveVR::HandleSublevelUnloaded Loaded sublevel: " + levelName);
	auto currentSceneData = GetCurrentSceneData();

	//lookup scenedata and if valid, push to the stack
	TSharedPtr<FSceneData> data = GetSceneData(levelName);
	if (currentSceneData.IsValid()) //currently has valid scene data
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
		if (data.IsValid())
		{
			properties->SetStringField("Scene Name", FString(data->Name));
			properties->SetStringField("Scene Id", FString(data->Id));
			float duration = Util::GetTimestamp() - SceneStartTime;
			properties->SetNumberField("Duration", duration);
			customEventRecorder->Send("c3d.SceneChange", properties);
		}
		else
		{
			properties->SetStringField("Sublevel Name", FString(levelName));
			customEventRecorder->Send("c3d.Level Streaming Load", properties);
		}
		FlushAndCacheEvents();
	}

	if (data.IsValid())
	{
		LoadedSceneDataStack.Push(data);
		ForceWriteSessionMetadata = true;
		CurrentTrackingSceneId = data->Id;
		LastSceneData = data;
		SceneStartTime = Util::GetTimestamp();
	}
	else //additively loading to scene without data
	{

	}
}

void FAnalyticsProviderCognitiveVR::HandleSublevelUnloaded(ULevel* level, UWorld* world)
{
	if (level == nullptr)
	{
		FlushAndCacheEvents();
		return;
	}

	if (!AutomaticallySetTrackingScene) { return; }

	FString levelName = level->GetFullGroupName(true);
	//GLog->Log("FAnalyticsProviderCognitiveVR::HandleSublevelUnloaded Unloaded sublevel: " + levelName);
	auto currentSceneData = GetCurrentSceneData();
	if (LoadedSceneDataStack.Num() == 0)
	{
		//no scene data to unload
		return;
	}
	auto stackTop = LoadedSceneDataStack.Top();
	TSharedPtr<FSceneData> data = GetSceneData(levelName);


	if (stackTop == currentSceneData && data == currentSceneData) //changing current scene to something further down the stack (or no scene)
	{		
		//remove the scene data
		LoadedSceneDataStack.Remove(data);
		auto stackNewTop = LoadedSceneDataStack.Top();

		//send scene change event (with destination if valid)
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
		if (stackNewTop.IsValid())
		{
			properties->SetStringField("Scene Name", FString(stackNewTop->Name));
			properties->SetStringField("Scene Id", FString(stackNewTop->Id));
			float duration = Util::GetTimestamp() - SceneStartTime;
			properties->SetNumberField("Duration", duration);
			customEventRecorder->Send("c3d.SceneChange", properties);
		}
		FlushAndCacheEvents();

		//set new current scene
		if (stackNewTop.IsValid())
		{
			ForceWriteSessionMetadata = true;
			CurrentTrackingSceneId = stackNewTop->Id;
			LastSceneData = stackNewTop;
		}
		else
		{
			CurrentTrackingSceneId = "";
			LastSceneData = nullptr;
		}
		SceneStartTime = Util::GetTimestamp();
	}
	else
	{
		//remove scene data from stack and do nothing else
		LoadedSceneDataStack.Remove(data);
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
		properties->SetStringField("Sublevel Name", FString(levelName));
		customEventRecorder->Send("c3d.Level Streaming Unload", properties);
	}
}

void FAnalyticsProviderCognitiveVR::HandlePostLevelLoad(UWorld* world)
{
	auto level = world->GetCurrentLevel();
	if (level == nullptr)
	{
		FlushAndCacheEvents();
		return;
	}
	if (!AutomaticallySetTrackingScene) { return; }

	FString levelName = level->GetFullGroupName(true);
	//GLog->Log("FAnalyticsProviderCognitiveVR::HandlePostLevelLoad level post load level: " + levelName);
	auto currentSceneData = GetCurrentSceneData();

	//lookup scenedata and if valid, add it to the stack
	LoadedSceneDataStack.Empty();
	TSharedPtr<FSceneData> data = GetSceneData(levelName);

	//if the new scene is the same as the current scene, return
	if (LastSceneData.IsValid() && data.IsValid() && LastSceneData->Id == data->Id)
	{
		return;
	}

	if (currentSceneData.IsValid()) //currently has valid scene data
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
		if (data.IsValid())
		{
			properties->SetStringField("Scene Name", FString(data->Name));
			properties->SetStringField("Scene Id", FString(data->Id));
		}
		float duration = Util::GetTimestamp() - SceneStartTime;
		properties->SetNumberField("Duration", duration);
		customEventRecorder->Send("c3d.SceneChange", properties);
		FlushAndCacheEvents();
	}

	if (data.IsValid())
	{
		LoadedSceneDataStack.Push(data);
		ForceWriteSessionMetadata = true;
		CurrentTrackingSceneId = data->Id;
		LastSceneData = data;
	}
	else //loading to scene without data
	{
		ForceWriteSessionMetadata = true;
		CurrentTrackingSceneId = "";
		LastSceneData = data;
	}
	SceneStartTime = Util::GetTimestamp();
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

bool FAnalyticsProviderCognitiveVR::StartSession()
{
	return StartSession(TArray<FAnalyticsEventAttribute>());
}

bool FAnalyticsProviderCognitiveVR::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	//initialize log
	CognitiveLog::Init();

	if (bHasSessionStarted)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::StartSession already started");
		return false;
	}

	auto cognitiveActor = ACognitiveVRActor::GetCognitiveVRActor();
	if (cognitiveActor == nullptr || !cognitiveActor->IsValidLowLevel())
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::StartSession could not find Cognitive Actor in your level");
		return false;
	}

	auto currentWorld = cognitiveActor->GetWorld();
	if (currentWorld == nullptr)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::StartSession World not set. Are you missing a Cognitive Actor in your level?");
		return false;
	}

	ApplicationKey = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "ApiKey", false);
	AttributionKey = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "AttributionKey", false);

	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "AutomaticallySetTrackingScene", false);
	if (ValueReceived.Len() > 0)
	{
		AutomaticallySetTrackingScene = FCString::ToBool(*ValueReceived);
	}

	if (ApplicationKey.Len() == 0)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::StartSession ApplicationKey is invalid");
		return false;
	}

	//pre session startup
	CacheSceneData();
	SessionTimestamp = Util::GetTimestamp();
	if (SessionId.IsEmpty())
	{
		SessionId = FString::FromInt(GetSessionTimestamp()) + TEXT("_") + DeviceId;
	}
	bHasSessionStarted = true;

	//set initial scene data
	if (AutomaticallySetTrackingScene)
	{
		auto level = currentWorld->GetCurrentLevel();
		if (level != nullptr)
		{
			FString levelName = level->GetFullGroupName(true);
			TSharedPtr<FSceneData> data = GetSceneData(levelName);
			if (data.IsValid())
			{
				LoadedSceneDataStack.Push(data);
				ForceWriteSessionMetadata = true;
				CurrentTrackingSceneId = data->Id;
				LastSceneData = data;
			}
		}
		SceneStartTime = Util::GetTimestamp();
	}

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


	customEventRecorder->StartSession();
	fixationDataRecorder->StartSession();
	dynamicObjectManager->OnSessionBegin();
	sensors->StartSession();
	OnSessionBegin.Broadcast();

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
	auto cognitiveActor = ACognitiveVRActor::GetCognitiveVRActor();
	if (cognitiveActor == nullptr)
	{
		CognitiveLog::Error("FAnalyticsProviderCognitiveVR::EndSession cognitiveActor is null");
		return;
	}
	gazeDataRecorder->PreSessionEnd();
	customEventRecorder->PreSessionEnd();
	fixationDataRecorder->PreSessionEnd();
	dynamicObjectManager->OnPreSessionEnd();
	sensors->PreSessionEnd();

	OnPreSessionEnd.Broadcast();
	FlushAndCacheEvents();

	//OnPostSessionEnd broadcast. used by components to clean up anything, including delegates
	gazeDataRecorder->PostSessionEnd();
	customEventRecorder->PostSessionEnd();
	fixationDataRecorder->PostSessionEnd();
	dynamicObjectManager->OnPostSessionEnd();
	sensors->PostSessionEnd();
	OnPostSessionEnd.Broadcast();
	localCache->Close();

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
	CurrentTrackingSceneId.Empty();
	LastSceneData.Reset();

	//broadcast end session
	CognitiveLog::Info("CognitiveVR EndSession");
}

//broadcast to all listeners that outstanding data should be sent + written to cache as a backup
//should be used when it's unclear if the session will continue - a sudden pause event might indicate a loss of wifi connection
void FAnalyticsProviderCognitiveVR::FlushAndCacheEvents()
{
	if (!bHasSessionStarted) { CognitiveLog::Warning("CognitiveVR Flush Events, but Session has not started!"); return; }

	gazeDataRecorder->SendData(true);
	customEventRecorder->SendData(true);
	fixationDataRecorder->SendData(true);
	dynamicObjectManager->SendData(true);
	sensors->SendData(true);
	OnRequestSend.Broadcast(true);
}

//broadcast to all listeners that outstanding data should be sent. this won't immediately write all data to the cache
//should be prefered when the session will likely continue, but at a point when sending data won't be noticed (eg, win screen popup, etc)
void FAnalyticsProviderCognitiveVR::FlushEvents()
{
	if (!bHasSessionStarted) { CognitiveLog::Warning("CognitiveVR Flush Events, but Session has not started!"); return; }
	gazeDataRecorder->SendData(false);
	customEventRecorder->SendData(false);
	fixationDataRecorder->SendData(false);
	dynamicObjectManager->SendData(false);
	sensors->SendData(false);
	OnRequestSend.Broadcast(false);
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
	return LastSceneData;
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

bool FAnalyticsProviderCognitiveVR::TryGetPlayerHMDPosition(FVector& vector)
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::GetPlayerHMDPosition no controllers skip");
		return false;
	}

	vector = controllers[0]->PlayerCameraManager->GetCameraLocation();
	return true;
}

 bool FAnalyticsProviderCognitiveVR::TryGetPlayerHMDRotation(FRotator& rotator)
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::GetPlayerHMDPosition no controllers skip");
		return false;
	}

	rotator = controllers[0]->PlayerCameraManager->GetCameraRotation();
	return true;
}

bool FAnalyticsProviderCognitiveVR::TryGetPlayerHMDLocalRotation(FRotator& rotator)
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::GetPlayerHMDPosition no controllers skip");
		return false;
	}

	APawn* pawn = controllers[0]->GetPawn();
	if (pawn == NULL) { return false; }
	UActorComponent* pawnComponent = pawn->GetComponentByClass(UCameraComponent::StaticClass());
	if (pawnComponent == NULL) { return false; }
	UCameraComponent* pawnCamera = Cast<UCameraComponent>(pawnComponent);
	if (pawnCamera == NULL) { return false; }

	rotator = pawnCamera->GetRelativeRotation();
	return true;
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

bool FAnalyticsProviderCognitiveVR::HasEyeTrackingSDK()
{
#if defined TOBII_EYETRACKING_ACTIVE
	return true;
#elif defined WAVEVR_EYETRACKING
	return true;
#elif defined OPENXR_EYETRACKING
	return true;
#elif defined HPGLIA_API
	return true;
#elif defined PICOMOBILE_API
	return true;
#elif defined VARJOEYETRACKER_API
	return true;
#elif defined SRANIPAL_1_3_API
	return true;
#elif defined SRANIPAL_1_2_API
	return true;
#else
	return false;
#endif
}

bool FAnalyticsProviderCognitiveVR::TryGetRoomSize(FVector& roomsize)
{
#if ENGINE_MAJOR_VERSION == 5
	FVector2D areaBounds = UHeadMountedDisplayFunctionLibrary::GetPlayAreaBounds();
	roomsize.X = areaBounds.X;
	roomsize.Y = areaBounds.Y;
	return true;
#elif ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION == 27

	FVector2D areaBounds = UHeadMountedDisplayFunctionLibrary::GetPlayAreaBounds();
	roomsize.X = areaBounds.X;
	roomsize.Y = areaBounds.Y;
	return true;
#else

	//oculus room size
	//TODO including this causes the editor to fail launching. can't find the OculusHMD internal module
	//roomsize = UOculusFunctionLibrary::GetGuardianDimensions(EBoundaryType::Boundary_PlayArea);
	return false;
#endif
}

TWeakObjectPtr<UDynamicObject> FAnalyticsProviderCognitiveVR::GetControllerDynamic(bool right)
{
	if (right)
	{
		if (dynamicObjectManager != nullptr && dynamicObjectManager->RightHandController.IsValid())
		{
			return dynamicObjectManager->RightHandController;
		}
	}
	else
	{
		if (dynamicObjectManager != nullptr && dynamicObjectManager->LeftHandController.IsValid())
		{
			return dynamicObjectManager->LeftHandController;
		}
	}
	return nullptr;
}

FString FAnalyticsProviderCognitiveVR::GetRuntime()
{
	return FString();
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

void FAnalyticsProviderCognitiveVR::SetTrackingScene(FString levelName)
{
	FlushEvents();
	TSharedPtr<FSceneData> data = GetSceneData(levelName);
	if (data.IsValid())
	{
		ForceWriteSessionMetadata = true;
		CurrentTrackingSceneId = data->Id;
		LastSceneData = data;
		SceneStartTime = Util::GetTimestamp();
	}
	else
	{
		ForceWriteSessionMetadata = true;
		CurrentTrackingSceneId = FString();
		LastSceneData = data;
		SceneStartTime = Util::GetTimestamp();
	}
	//todo consider events for arrival/departure from scenes here
}