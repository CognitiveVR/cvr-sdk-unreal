/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3D/Public/Cognitive3D.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Classes/Camera/CameraComponent.h"
#ifdef INCLUDE_OCULUS_PLUGIN
#if ENGINE_MAJOR_VERSION == 4
#include "OculusFunctionLibrary.h"
#elif ENGINE_MAJOR_VERSION == 5 
#include "OculusXRFunctionLibrary.h"
#endif
#endif
#include "Interfaces/IPluginManager.h"
//IMPROVEMENT this should be in the header, but can't find ControllerType enum
#include "Cognitive3D/Private/C3DComponents/InputTracker.h"
#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif
#include "HeadMountedDisplayFunctionLibrary.h"
#include "HeadMountedDisplayTypes.h"
IMPLEMENT_MODULE(IAnalyticsCognitive3D, Cognitive3D);

bool IAnalyticsProviderCognitive3D::bHasSessionStarted = false;

void IAnalyticsCognitive3D::StartupModule()
{
	GLog->Log("AnalyticsCognitive3D::StartupModule");

	TSharedPtr<IAnalyticsProviderCognitive3D> cog = MakeShareable(new IAnalyticsProviderCognitive3D());
	AnalyticsProvider = cog;
	Cognitive3DProvider = TWeakPtr<IAnalyticsProviderCognitive3D>(cog);

	//create non-initialized data collectors and other internal stuff
	Cognitive3DProvider.Pin()->exitpoll = MakeShareable(new FExitPoll());
	Cognitive3DProvider.Pin()->customEventRecorder = new FCustomEventRecorder();
	Cognitive3DProvider.Pin()->sensors = new FSensors();
	Cognitive3DProvider.Pin()->fixationDataRecorder = new FFixationDataRecorder();
	Cognitive3DProvider.Pin()->gazeDataRecorder = new FGazeDataRecorder();
	Cognitive3DProvider.Pin()->localCache = MakeShareable(new FLocalCache(FPaths::GeneratedConfigDir()));
	Cognitive3DProvider.Pin()->network = MakeShareable(new FNetwork());
	Cognitive3DProvider.Pin()->dynamicObjectManager = new FDynamicObjectManager();
}

void IAnalyticsProviderCognitive3D::HandleSublevelLoaded(ULevel* level, UWorld* world)
{
	if (level == nullptr)
	{
		FlushAndCacheEvents();
		return;
	}

	if (!AutomaticallySetTrackingScene) { return; }

	FString levelName = level->GetFullGroupName(true);
	//GLog->Log("IAnalyticsProviderCognitive3D::HandleSublevelUnloaded Loaded sublevel: " + levelName);
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
			float duration = FUtil::GetTimestamp() - SceneStartTime;
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
		SceneStartTime = FUtil::GetTimestamp();
	}
	else //additively loading to scene without data
	{

	}
}

void IAnalyticsProviderCognitive3D::HandleSublevelUnloaded(ULevel* level, UWorld* world)
{
	if (level == nullptr)
	{
		FlushAndCacheEvents();
		return;
	}

	if (!AutomaticallySetTrackingScene) { return; }

	FString levelName = level->GetFullGroupName(true);
	//GLog->Log("IAnalyticsProviderCognitive3D::HandleSublevelUnloaded Unloaded sublevel: " + levelName);
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
			float duration = FUtil::GetTimestamp() - SceneStartTime;
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
		SceneStartTime = FUtil::GetTimestamp();
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

void IAnalyticsProviderCognitive3D::HandlePostLevelLoad(UWorld* world)
{
	auto level = world->GetCurrentLevel();
	if (level == nullptr)
	{
		FlushAndCacheEvents();
		return;
	}
	if (!AutomaticallySetTrackingScene) { return; }

	FString levelName = level->GetFullGroupName(true);
	//GLog->Log("IAnalyticsProviderCognitive3D::HandlePostLevelLoad level post load level: " + levelName);
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
		float duration = FUtil::GetTimestamp() - SceneStartTime;
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
	SceneStartTime = FUtil::GetTimestamp();
}

void IAnalyticsCognitive3D::ShutdownModule()
{

}

bool IAnalyticsProviderCognitive3D::HasStartedSession()
{
	return bHasSessionStarted;
}

TSharedPtr<IAnalyticsProvider> IAnalyticsCognitive3D::CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const
{
	return AnalyticsProvider;
}

TWeakPtr<IAnalyticsProviderCognitive3D> IAnalyticsCognitive3D::GetCognitive3DProvider() const
{
	return Cognitive3DProvider;
}

IAnalyticsProviderCognitive3D::IAnalyticsProviderCognitive3D()
{
#if PLATFORM_ANDROID
	DeviceId = FPlatformMisc::GetDeviceId();
#else
	DeviceId = FPlatformMisc::GetHashedMacAddressString();
#endif
}

IAnalyticsProviderCognitive3D::~IAnalyticsProviderCognitive3D()
{
	UE_LOG(LogTemp, Warning, TEXT("IAnalyticsProviderCognitive3D Shutdown Module"));
}

bool IAnalyticsProviderCognitive3D::StartSession()
{
	return StartSession(TArray<FAnalyticsEventAttribute>());
}

bool IAnalyticsProviderCognitive3D::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	//initialize log
	FCognitiveLog::Init();

	if (bHasSessionStarted)
	{
		FCognitiveLog::Warning("IAnalyticsProviderCognitive3D::StartSession already started");
		return false;
	}

	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == nullptr || !cognitiveActor->IsValidLowLevel())
	{
		FCognitiveLog::Error("IAnalyticsProviderCognitive3D::StartSession could not find Cognitive Actor in your level");
		return false;
	}

	auto currentWorld = cognitiveActor->GetWorld();
	if (currentWorld == nullptr)
	{
		FCognitiveLog::Error("IAnalyticsProviderCognitive3D::StartSession World not set. Are you missing a Cognitive Actor in your level?");
		return false;
	}
	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	//ApplicationKey = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "ApiKey", false);
	ApplicationKey = FAnalytics::Get().GetConfigValueFromIni(EngineIni, "Analytics", "ApiKey", false);
	AttributionKey = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "AttributionKey", false);

	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "AutomaticallySetTrackingScene", false);
	if (ValueReceived.Len() > 0)
	{
		AutomaticallySetTrackingScene = FCString::ToBool(*ValueReceived);
	}

	if (ApplicationKey.Len() == 0)
	{
#if WITH_EDITOR
		//FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("The Application Key is Invalid, this session will not be recorded on the dashboard. Please restart the editor."));
#endif
		FCognitiveLog::Error("IAnalyticsProviderCognitive3D::StartSession ApplicationKey is invalid");
		//UE_LOG(LogTemp, Warning, TEXT("App key is %s"), *ApplicationKey);
		return false;
	}

	//pre session startup
	CacheSceneData();
	SessionTimestamp = FUtil::GetTimestamp();
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
		SceneStartTime = FUtil::GetTimestamp();
	}

	//register delegates
	if (!PauseHandle.IsValid())
	{
		PauseHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddRaw(this, &IAnalyticsProviderCognitive3D::HandleApplicationWillEnterBackground);	
	}
	if (!LevelLoadHandle.IsValid())
		LevelLoadHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &IAnalyticsProviderCognitive3D::HandlePostLevelLoad);
	if (!SublevelLoadedHandle.IsValid())
		SublevelLoadedHandle = FWorldDelegates::LevelAddedToWorld.AddRaw(this, &IAnalyticsProviderCognitive3D::HandleSublevelLoaded);
	if (!SublevelUnloadedHandle.IsValid())
		SublevelUnloadedHandle = FWorldDelegates::LevelRemovedFromWorld.AddRaw(this, &IAnalyticsProviderCognitive3D::HandleSublevelUnloaded);

	//set session properties
	FUtil::SetSessionProperties();
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

	FCognitiveLog::Info("IAnalyticsProviderCognitive3D::StartSession");

	return bHasSessionStarted;
}

void IAnalyticsProviderCognitive3D::SetLobbyId(FString lobbyId)
{
	LobbyId = lobbyId;
}

void IAnalyticsProviderCognitive3D::SetSessionName(FString sessionName)
{
	bHasCustomSessionName = true;
	SetSessionProperty("c3d.sessionname", sessionName);
}

void IAnalyticsProviderCognitive3D::EndSession()
{
	if (bHasSessionStarted == false)
	{
		return;
	}

	//broadcast pre session end for pre-cleanup
	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == nullptr)
	{
		FCognitiveLog::Error("IAnalyticsProviderCognitive3D::EndSession cognitiveActor is null");
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
	FCognitiveLog::Info("Cognitive3D EndSession");
}

//broadcast to all listeners that outstanding data should be sent + written to cache as a backup
//should be used when it's unclear if the session will continue - a sudden pause event might indicate a loss of wifi connection
void IAnalyticsProviderCognitive3D::FlushAndCacheEvents()
{
	if (!bHasSessionStarted) { FCognitiveLog::Warning("Cognitive3D Flush Events, but Session has not started!"); return; }

	gazeDataRecorder->SendData(true);
	customEventRecorder->SendData(true);
	fixationDataRecorder->SendData(true);
	dynamicObjectManager->SendData(true);
	sensors->SendData(true);
	OnRequestSend.Broadcast(true);
}

//broadcast to all listeners that outstanding data should be sent. this won't immediately write all data to the cache
//should be prefered when the session will likely continue, but at a point when sending data won't be noticed (eg, win screen popup, etc)
void IAnalyticsProviderCognitive3D::FlushEvents()
{
	if (!bHasSessionStarted) { FCognitiveLog::Warning("Cognitive3D Flush Events, but Session has not started!"); return; }
	gazeDataRecorder->SendData(false);
	customEventRecorder->SendData(false);
	fixationDataRecorder->SendData(false);
	dynamicObjectManager->SendData(false);
	sensors->SendData(false);
	OnRequestSend.Broadcast(false);
}

void IAnalyticsProviderCognitive3D::SetUserID(const FString& InUserID)
{
	FString userId = InUserID;
	if (userId.Len() > 64)
	{
		FCognitiveLog::Error("IAnalyticsProviderCognitive3D::SetUserID exceeds maximum character limit. clipping to 64");
		int32 chopcount = userId.Len() - 64;
		userId = userId.LeftChop(chopcount);
	}

	ParticipantId = userId;
	SetParticipantProperty("id", userId);
	FCognitiveLog::Info("IAnalyticsProviderCognitive3D::SetUserID set user id: " + userId);
}

void IAnalyticsProviderCognitive3D::SetParticipantId(FString participantId)
{
	if (participantId.Len() > 64)
	{
		FCognitiveLog::Error("IAnalyticsProviderCognitive3D::SetParticipantId exceeds maximum character limit. clipping to 64");
		int32 chopcount = participantId.Len() - 64;
		participantId = participantId.LeftChop(chopcount);
	}

	ParticipantId = participantId;
	SetParticipantProperty("id", participantId);
	FCognitiveLog::Info("IAnalyticsProviderCognitive3D::SetParticipantId set user id: " + participantId);
}

void IAnalyticsProviderCognitive3D::SetParticipantFullName(FString participantName)
{
	ParticipantName = participantName;
	SetParticipantProperty("name", participantName);
	FCognitiveLog::Info("IAnalyticsProviderCognitive3D::SetParticipantData set user id");
	if (!bHasCustomSessionName)
		SetSessionProperty("c3d.sessionname", participantName);
}

void IAnalyticsProviderCognitive3D::SetSessionTag(FString Tag)
{
	if (Tag.Len() == 0)
	{
		FCognitiveLog::Error("IAnalyticsProviderCognitive3D::SetSessionTag must contain > 0 characters");
		return;
	}
	if (Tag.Len() > 12)
	{
		FCognitiveLog::Error("IAnalyticsProviderCognitive3D::SetSessionTag must contain <= 12 characters");
		return;
	}

	SetSessionProperty("c3d.session_tag."+Tag, "true");
}

FString IAnalyticsProviderCognitive3D::GetUserID() const
{
	return ParticipantId;
}

FString IAnalyticsProviderCognitive3D::GetCognitiveUserName() const
{
	return ParticipantName;
}

FString IAnalyticsProviderCognitive3D::GetDeviceID() const
{
	return DeviceId;
}

FString IAnalyticsProviderCognitive3D::GetSessionID() const
{
	return SessionId;
}

double IAnalyticsProviderCognitive3D::GetSessionTimestamp() const
{
	return SessionTimestamp;
}

bool IAnalyticsProviderCognitive3D::SetSessionID(const FString& InSessionID)
{
	if (!bHasSessionStarted)
	{
		SessionId = InSessionID;
	}
	else
	{
		// Log that we shouldn't switch session ids during a session
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordEvent while a session is in progress. Ignoring");
	}
	return !bHasSessionStarted;
}

//built in analytics stuff
void IAnalyticsProviderCognitive3D::RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes)
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
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordEvent called before StartSession. Ignoring");
	}
}

void IAnalyticsProviderCognitive3D::RecordItemPurchase(const FString& ItemId, const FString& Currency, int32 PerItemCost, int32 ItemQuantity)
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
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordItemPurchase called before StartSession. Ignoring");
	}
}

void IAnalyticsProviderCognitive3D::RecordCurrencyPurchase(const FString& GameCurrencyType, int32 GameCurrencyAmount, const FString& RealCurrencyType, float RealMoneyCost, const FString& PaymentProvider)
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
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordCurrencyPurchase called before StartSession. Ignoring");
	}
}

void IAnalyticsProviderCognitive3D::RecordCurrencyGiven(const FString& GameCurrencyType, int32 GameCurrencyAmount)
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
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordCurrencyGiven called before StartSession. Ignoring");
	}
}

void IAnalyticsProviderCognitive3D::SetAge(const int32 InAge)
{
	SetParticipantProperty("age", InAge);
}

void IAnalyticsProviderCognitive3D::SetLocation(const FString& InLocation)
{
	SetSessionProperty("location", InLocation);
}

void IAnalyticsProviderCognitive3D::SetGender(const FString& InGender)
{
	SetParticipantProperty("gender", InGender);
}

void IAnalyticsProviderCognitive3D::SetBuildInfo(const FString& InBuildInfo)
{
	BuildInfo = InBuildInfo;
}

void IAnalyticsProviderCognitive3D::RecordError(const FString& Error, const TArray<FAnalyticsEventAttribute>& Attributes)
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

		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordError");
	}
	else
	{
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordError called before StartSession. Ignoring");
	}
}

void IAnalyticsProviderCognitive3D::RecordProgress(const FString& ProgressType, const FString& ProgressName, const TArray<FAnalyticsEventAttribute>& Attributes)
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
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordError called before StartSession. Ignoring");
	}
}

void IAnalyticsProviderCognitive3D::RecordItemPurchase(const FString& ItemId, int32 ItemQuantity, const TArray<FAnalyticsEventAttribute>& Attributes)
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
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordItemPurchase called before StartSession. Ignoring");
	}
}

void IAnalyticsProviderCognitive3D::RecordCurrencyPurchase(const FString& GameCurrencyType, int32 GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& Attributes)
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
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordCurrencyPurchase called before StartSession. Ignoring");
	}
}

void IAnalyticsProviderCognitive3D::RecordCurrencyGiven(const FString& GameCurrencyType, int32 GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& Attributes)
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
		FCognitiveLog::Warning("FAnalyticsProvideCognitive3D::RecordCurrencyGiven called before StartSession. Ignoring");
	}
}

TSharedPtr<FSceneData> IAnalyticsProviderCognitive3D::GetCurrentSceneData()
{
	return LastSceneData;
}

TSharedPtr<FSceneData> IAnalyticsProviderCognitive3D::GetSceneData(FString scenename)
{
	for (int i = 0; i < SceneData.Num(); i++)
	{
		if (!SceneData[i].IsValid()) { continue; }
		if (SceneData[i]->Name == scenename)
		{
			return SceneData[i];
		}
	}
	//FCognitiveLog::Warning("IAnalyticsProviderCognitive3D::GetSceneData couldn't find SceneData for scene " + scenename);
	return NULL;
}

FString IAnalyticsProviderCognitive3D::GetCurrentSceneId()
{
	auto currentData = GetCurrentSceneData();
	if (!currentData.IsValid()) { return ""; }

	return currentData->Id;
}

FString IAnalyticsProviderCognitive3D::GetCurrentSceneVersionNumber()
{
	auto currentData = GetCurrentSceneData();
	if (!currentData.IsValid()) { return ""; }

	return FString::FromInt(currentData->VersionNumber);
}

void IAnalyticsProviderCognitive3D::CacheSceneData()
{
	TArray<FString>scenstrings;
	FString TestSyncFile = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->GetArray(TEXT("/Script/Cognitive3D.Cognitive3DSceneSettings"), TEXT("SceneData"), scenstrings, TestSyncFile);
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
			FCognitiveLog::Error("IAnalyticsProviderCognitive3D::CacheSceneData failed to parse " + scenstrings[i]);
			continue;
		}

		SceneData.Add(MakeShareable(new FSceneData(Array[0], Array[1], FCString::Atoi(*Array[2]), FCString::Atoi(*Array[3]))));
	}
}

bool IAnalyticsProviderCognitive3D::TryGetPlayerHMDPosition(FVector& vector)
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		FCognitiveLog::Warning("IAnalyticsProviderCognitive3D::GetPlayerHMDPosition no controllers skip");
		return false;
	}

	vector = controllers[0]->PlayerCameraManager->GetCameraLocation();
	return true;
}

 bool IAnalyticsProviderCognitive3D::TryGetPlayerHMDRotation(FRotator& rotator)
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		FCognitiveLog::Warning("IAnalyticsProviderCognitive3D::GetPlayerHMDPosition no controllers skip");
		return false;
	}

	rotator = controllers[0]->PlayerCameraManager->GetCameraRotation();
	return true;
}

bool IAnalyticsProviderCognitive3D::TryGetPlayerHMDLocalRotation(FRotator& rotator)
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		FCognitiveLog::Warning("IAnalyticsProviderCognitive3D::GetPlayerHMDPosition no controllers skip");
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

void IAnalyticsProviderCognitive3D::SetSessionProperty(FString name, int32 value)
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
void IAnalyticsProviderCognitive3D::SetSessionProperty(FString name, float value)
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
void IAnalyticsProviderCognitive3D::SetSessionProperty(FString name, FString value)
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

void IAnalyticsProviderCognitive3D::SetParticipantProperty(FString name, int32 value)
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
void IAnalyticsProviderCognitive3D::SetParticipantProperty(FString name, float value)
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
void IAnalyticsProviderCognitive3D::SetParticipantProperty(FString name, FString value)
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

FJsonObject IAnalyticsProviderCognitive3D::GetNewSessionProperties()
{
	FJsonObject returnobject = FJsonObject(NewSessionProperties);
	NewSessionProperties = FJsonObject();
	return returnobject;
}

FJsonObject IAnalyticsProviderCognitive3D::GetAllSessionProperties()
{
	FJsonObject returnobject = FJsonObject(AllSessionProperties);
	return returnobject;
}

FString IAnalyticsProviderCognitive3D::GetAttributionParameters()
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

bool IAnalyticsProviderCognitive3D::HasEyeTrackingSDK()
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

bool IAnalyticsProviderCognitive3D::TryGetRoomSize(FVector& roomsize)
{
#ifdef INCLUDE_OCULUS_PLUGIN
#if ENGINE_MAJOR_VERSION == 5
	FVector BoundaryPoints = UOculusXRFunctionLibrary::GetGuardianDimensions(EOculusXRBoundaryType::Boundary_PlayArea);
	roomsize.X = BoundaryPoints.X;
	roomsize.Y = BoundaryPoints.Y;
	return true;
#elif ENGINE_MAJOR_VERSION == 4
	FVector BoundaryPoints = UOculusFunctionLibrary::GetGuardianDimensions(EBoundaryType::Boundary_PlayArea);
	roomsize.X = BoundaryPoints.X;
	roomsize.Y = BoundaryPoints.Y;
	return true;
#endif
#elif ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION == 27 
	FVector2D areaBounds = UHeadMountedDisplayFunctionLibrary::GetPlayAreaBounds();
	roomsize.X = areaBounds.X;
	roomsize.Y = areaBounds.Y;
	return true;
#elif ENGINE_MAJOR_VERSION == 5
	FVector2D areaBounds = UHeadMountedDisplayFunctionLibrary::GetPlayAreaBounds();
	roomsize.X = areaBounds.X;
	roomsize.Y = areaBounds.Y;
	return true;
#else
	return false;
#endif
}

bool IAnalyticsProviderCognitive3D::TryGetHMDGuardianPoints(TArray<FVector>& GuardianPoints)
{
#ifdef INCLUDE_OCULUS_PLUGIN
#if ENGINE_MAJOR_VERSION == 4 
	GuardianPoints = UOculusFunctionLibrary::GetGuardianPoints(EBoundaryType::Boundary_PlayArea, false);
	return true;
#elif ENGINE_MAJOR_VERSION == 5 
	GuardianPoints = UOculusXRFunctionLibrary::GetGuardianPoints(EOculusXRBoundaryType::Boundary_PlayArea, false);
	return true;
#endif
#else
	return false;
#endif
	
}

//this function tries to get the pose of an HMD inside the pre-set boundaries
//primarily used for stationary guardian intersection
bool IAnalyticsProviderCognitive3D::TryGetHMDPose(FRotator& HMDRotation, FVector& HMDPosition, FVector& HMDNeckPos)
{

#ifdef INCLUDE_OCULUS_PLUGIN
#if ENGINE_MAJOR_VERSION == 4 
	UOculusFunctionLibrary::GetPose(HMDRotation, HMDPosition, HMDNeckPos, true, true, FVector::OneVector); //position inside bounds
	return true;
#elif ENGINE_MAJOR_VERSION == 5
	UOculusXRFunctionLibrary::GetPose(HMDRotation, HMDPosition, HMDNeckPos, true, true, FVector::OneVector);
	return true;
#endif
#else
	return false;
#endif

}

bool IAnalyticsProviderCognitive3D::TryGetHMDWornState(EHMDWornState::Type& WornState)
{
#ifdef INCLUDE_PICO_PLUGIN
	WornState = UPICOXRHMDFunctionLibrary::PXR_GetHMDWornState();
	if (WornState == EHMDWornState::Worn)
	{
		return true;
	}
	else
	{
		return false;
	}

#endif

	WornState = UHeadMountedDisplayFunctionLibrary::GetHMDWornState();

	if (WornState == EHMDWornState::Worn)
	{
		return true;
	}
	return false;
}



bool IAnalyticsProviderCognitive3D::IsPointInPolygon4(TArray<FVector> polygon, FVector testPoint)
{
	bool result = false;
	int j = polygon.Num() - 1;
	for (int i = 0; i < polygon.Num(); i++)
	{
		if (polygon[i].Y < testPoint.Y && polygon[j].Y >= testPoint.Y || polygon[j].Y < testPoint.Y && polygon[i].Y >= testPoint.Y)
		{
			if (polygon[i].X + (testPoint.Y - polygon[i].Y) / (polygon[j].Y - polygon[i].Y) * (polygon[j].X - polygon[i].X) < testPoint.X)
			{
				result = !result;
			}
		}
		j = i;
	}
	return result;
}


bool IAnalyticsProviderCognitive3D::IsPluginEnabled(const FString& PluginName)
{
	IPluginManager& PluginManager = IPluginManager::Get();
	TSharedPtr<IPlugin> Plugin = PluginManager.FindPlugin(PluginName);

	if (Plugin.IsValid())
	{
		return Plugin->IsEnabled();
	}

	return false;
}

TWeakObjectPtr<UDynamicObject> IAnalyticsProviderCognitive3D::GetControllerDynamic(bool right)
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

FString IAnalyticsProviderCognitive3D::GetRuntime()
{
	return FString();
}

void IAnalyticsProviderCognitive3D::HandleApplicationWillEnterBackground()
{
	FlushAndCacheEvents();
	if (!localCache.IsValid())
	{
		return;
	}
	localCache->SerializeToFile();
}

void IAnalyticsProviderCognitive3D::SetTrackingScene(FString levelName)
{
	FlushEvents();
	TSharedPtr<FSceneData> data = GetSceneData(levelName);
	if (data.IsValid())
	{
		ForceWriteSessionMetadata = true;
		CurrentTrackingSceneId = data->Id;
		LastSceneData = data;
		SceneStartTime = FUtil::GetTimestamp();
	}
	else
	{
		ForceWriteSessionMetadata = true;
		CurrentTrackingSceneId = FString();
		LastSceneData = data;
		SceneStartTime = FUtil::GetTimestamp();
	}
	//todo consider events for arrival/departure from scenes here
}