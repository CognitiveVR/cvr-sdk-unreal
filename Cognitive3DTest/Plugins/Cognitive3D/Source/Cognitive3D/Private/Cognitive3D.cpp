/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3D/Public/Cognitive3D.h"
//#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Classes/Camera/CameraComponent.h"
#ifdef INCLUDE_OCULUS_PLUGIN
#if ENGINE_MAJOR_VERSION == 4
#include "OculusFunctionLibrary.h"
#elif ENGINE_MAJOR_VERSION == 5 
#include "OculusXRFunctionLibrary.h"
#endif
#endif
#include "Interfaces/IPluginManager.h"
#ifdef INCLUDE_PICO_PLUGIN
#include "PXR_HMDFunctionLibrary.h"
#endif
//IMPROVEMENT this should be in the header, but can't find ControllerType enum
#include "Cognitive3D/Private/C3DComponents/InputTracker.h"
#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif
#include "HeadMountedDisplayFunctionLibrary.h"
#include "HeadMountedDisplayTypes.h"

//
#include "Cognitive3D/Public/C3DCommonTypes.h"
#include "Analytics.h"
#include "TimerManager.h"
#include "AnalyticsEventAttribute.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Cognitive3D/Public/Cognitive3DBlueprints.h"
#include "HeadMountedDisplay.h"
#include "Cognitive3D/Public/Cognitive3DSettings.h"
#include "Cognitive3D/Private/ExitPoll.h"
#include "Cognitive3D/Private/C3DComponents/PlayerTracker.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "Cognitive3D/Private/C3DComponents/FixationRecorder.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Private/C3DUtil/CognitiveLog.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"
#include "Cognitive3D/Public/CustomEvent.h"
#include "Cognitive3D/Private/C3DApi/SensorRecorder.h"
#include "Cognitive3D/Private/LocalCache.h"
#include "Cognitive3D/Private/C3DApi/GazeDataRecorder.h"
#include "Cognitive3D/Private/C3DUtil/CognitiveLog.h"
#include "Cognitive3D/Private/C3DApi/FixationDataRecorder.h"
#include "Cognitive3D/Private/C3DComponents/RemoteControls.h"
#include "Cognitive3D/Private/C3DApi/RemoteControlsRecorder.h"
#include "Cognitive3D/Private/C3DApi/BoundaryRecorder.h"
#include "LandscapeStreamingProxy.h"

IMPLEMENT_MODULE(FAnalyticsCognitive3D, Cognitive3D);

bool FAnalyticsProviderCognitive3D::bHasSessionStarted = false;

void FAnalyticsCognitive3D::StartupModule()
{
	GLog->Log("AnalyticsCognitive3D::StartupModule");

	TSharedPtr<FAnalyticsProviderCognitive3D> cog = MakeShareable(new FAnalyticsProviderCognitive3D());
	AnalyticsProvider = cog;
	Cognitive3DProvider = TWeakPtr<FAnalyticsProviderCognitive3D>(cog);

	//create non-initialized data collectors and other internal stuff
	Cognitive3DProvider.Pin()->exitpoll = MakeShareable(new FExitPoll());
	Cognitive3DProvider.Pin()->customEventRecorder = new FCustomEventRecorder();
	Cognitive3DProvider.Pin()->sensors = new FSensors();
	Cognitive3DProvider.Pin()->fixationDataRecorder = new FFixationDataRecorder();
	Cognitive3DProvider.Pin()->gazeDataRecorder = new FGazeDataRecorder();
	Cognitive3DProvider.Pin()->boundaryRecorder = new BoundaryRecorder();
	Cognitive3DProvider.Pin()->localCache = MakeShareable(new FLocalCache(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("c3dlocal/"))));
	Cognitive3DProvider.Pin()->network = MakeShareable(new FNetwork());
	Cognitive3DProvider.Pin()->dynamicObjectManager = new FDynamicObjectManager();
	FRemoteControlsRecorder::Initialize();
}

void FAnalyticsProviderCognitive3D::HandleSublevelLoaded(ULevel* level, UWorld* world)
{
	if (!bHasSessionStarted)
	{
		return;
	}
	
	if (level == nullptr)
	{
		FlushAndCacheEvents();
		return;
	}

	if (!AutomaticallySetTrackingScene) { return; }

	FString levelName = level->GetFullGroupName(true);
	TArray<FString> PathParts;
	level->GetPathName().ParseIntoArray(PathParts, TEXT("/"), true);
	FString levelPath = "";
	if (PathParts.Num() > 0)
	{
		PathParts.RemoveAt(PathParts.Num() - 1); //remove the last part, which is the level name
		levelPath = FString::Join(PathParts, TEXT("/"));
		levelPath = "/" + levelPath; //add a leading slash
	}

	//GLog->Log("FAnalyticsProviderCognitive3D::HandleSublevelUnloaded Loaded sublevel: " + levelName);
	auto currentSceneData = GetCurrentSceneData();

	bool bIsLandscapeStreamingProxy = false;

	// Check if the level has any actors and if the first valid actor is a Landscape Streaming Proxy
	for (AActor* Actor : level->Actors)
	{
		if (Actor) // Make sure the actor is valid
		{
			if (Actor->IsA(ALandscapeStreamingProxy::StaticClass()))
			{
				UE_LOG(LogTemp, Log, TEXT("HandleSublevelLoaded: Detected Landscape Streaming Proxy: %s in sublevel %s"), *Actor->GetName(), *level->GetName());
				//customEventRecorder->Send("c3d.LandscapeLoaded LOADED LANDSCAPE PROXY");
				bIsLandscapeStreamingProxy = true;
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("HandleSublevelLoaded: Non-landscape actor detected: %s in sublevel %s"), *Actor->GetName(), *level->GetName());
			}
		}
	}

	if (bIsLandscapeStreamingProxy)
	{
		return;
	}

	//lookup scenedata and if valid, push to the stack
	TSharedPtr<FSceneData> data = GetSceneData(levelPath + "/" + levelName);
	if (currentSceneData.IsValid()) //currently has valid scene data
	{
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
		if (data.IsValid())
		{
			properties->SetStringField("Scene Name", FString(data->Name));
			properties->SetStringField("Scene Id", FString(data->Id));
			properties->SetStringField("Previous Scene Name", FString(currentSceneData->Name));
			float duration = FUtil::GetTimestamp() - SceneStartTime;
			properties->SetNumberField("Duration", duration);
			customEventRecorder->Send("c3d.SceneLoaded", properties);
		}
		else
		{
			properties->SetStringField("Sublevel Name", FString(levelName));
			customEventRecorder->Send("c3d.Level Streaming Load", properties);
		}
		gazeDataRecorder->SendData(true);
		customEventRecorder->SendData(true);
		fixationDataRecorder->SendData(true);
		sensors->SendData(true);
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

	//empty current dynamics manifest
	dynamicObjectManager->manifest.Empty();
	dynamicObjectManager->newManifest.Empty();
	//register dynamics in new level
	// Iterate over all actors in the loaded level
	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		for (UActorComponent* actorComponent : ActorItr->GetComponents())
		{
			if (actorComponent->IsA(UDynamicObject::StaticClass()))
			{
				UDynamicObject* dynamicComponent = Cast<UDynamicObject>(actorComponent);
				if (dynamicComponent && !dynamicComponent->IsRegistered())
				{
					// Register the component
					dynamicComponent->RegisterComponent();
				}
				if (dynamicComponent && !dynamicComponent->IsController)
				{
					dynamicComponent->HasInitialized = false;
					dynamicComponent->Initialize();
				}
			}
		}
	}

	//we send the dynamic data stream after the new scene is loaded and set as the current scene
	//that way we can send the new, correct manifest to the desired level
	dynamicObjectManager->SendData(true);
	//
	boundaryRecorder->SendData(true);
	OnCognitiveLevelChange.Broadcast();
}

void FAnalyticsProviderCognitive3D::HandleSublevelUnloaded(ULevel* level, UWorld* world)
{
	if (!bHasSessionStarted)
	{
		return;
	}

	if (level == nullptr)
	{
		FlushAndCacheEvents();
		return;
	}

	if (!AutomaticallySetTrackingScene) { return; }

	FString levelName = level->GetFullGroupName(true);
	TArray<FString> PathParts;
	level->GetPathName().ParseIntoArray(PathParts, TEXT("/"), true);
	FString levelPath = "";
	if (PathParts.Num() > 0)
	{
		PathParts.RemoveAt(PathParts.Num() - 1); //remove the last part, which is the level name
		levelPath = FString::Join(PathParts, TEXT("/"));
		levelPath = "/" + levelPath; //add a leading slash
	}

	//GLog->Log("FAnalyticsProviderCognitive3D::HandleSublevelUnloaded Unloaded sublevel: " + levelName);
	auto currentSceneData = GetCurrentSceneData();
	if (LoadedSceneDataStack.Num() == 0)
	{
		//no scene data to unload
		return;
	}
	auto stackTop = LoadedSceneDataStack.Top();
	TSharedPtr<FSceneData> data = GetSceneData(levelPath + "/" + levelName);


	if (stackTop == currentSceneData && data == currentSceneData) //changing current scene to something further down the stack (or no scene)
	{		
		//remove the scene data
		LoadedSceneDataStack.Remove(data);

		TSharedPtr<FSceneData> stackNewTop;
		if (LoadedSceneDataStack.Num() > 0)
		{
			stackNewTop = LoadedSceneDataStack.Top();
		}

		//send scene change event (with destination if valid)
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
		if (stackNewTop.IsValid())
		{
			properties->SetStringField("Scene Name", FString(data->Name));
			properties->SetStringField("Scene Id", FString(data->Id));
			properties->SetStringField("Destination Scene Name", FString(stackNewTop->Name));
			float duration = FUtil::GetTimestamp() - SceneStartTime;
			properties->SetNumberField("Duration", duration);
			customEventRecorder->Send("c3d.SceneUnloaded", properties);
		}
		gazeDataRecorder->SendData(true);
		customEventRecorder->SendData(true);
		fixationDataRecorder->SendData(true);
		sensors->SendData(true);

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

		//empty current dynamics manifest
		dynamicObjectManager->manifest.Empty();
		dynamicObjectManager->newManifest.Empty();
		//register dynamics in new level
		// Iterate over all actors in the loaded level
		for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
		{
			for (UActorComponent* actorComponent : ActorItr->GetComponents())
			{
				if (actorComponent->IsA(UDynamicObject::StaticClass()))
				{
					UDynamicObject* dynamicComponent = Cast<UDynamicObject>(actorComponent);
					if (dynamicComponent && !dynamicComponent->IsRegistered())
					{
						// Register the component
						dynamicComponent->RegisterComponent();
					}
					if (dynamicComponent && !dynamicComponent->IsController)
					{
						dynamicComponent->HasInitialized = false;
						dynamicComponent->Initialize();
					}
				}
			}
		}

		dynamicObjectManager->SendData(true);
		//
		boundaryRecorder->SendData(true);

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
	OnCognitiveLevelChange.Broadcast();
}

void FAnalyticsProviderCognitive3D::HandlePostLevelLoad(UWorld* world)
{
	auto level = world->GetCurrentLevel();
	if (level == nullptr)
	{
		FlushAndCacheEvents();
		return;
	}
	if (!AutomaticallySetTrackingScene) { return; }

	FString levelName = level->GetFullGroupName(true);
	TArray<FString> PathParts;
	level->GetPathName().ParseIntoArray(PathParts, TEXT("/"), true);
	FString levelPath = "";
	if (PathParts.Num() > 0)
	{
		PathParts.RemoveAt(PathParts.Num() - 1); //remove the last part, which is the level name
		levelPath = FString::Join(PathParts, TEXT("/"));
		levelPath = "/" + levelPath; //add a leading slash
	}

	//GLog->Log("FAnalyticsProviderCognitive3D::HandlePostLevelLoad level post load level: " + levelName);
	auto currentSceneData = GetCurrentSceneData();

	//lookup scenedata and if valid, add it to the stack
	LoadedSceneDataStack.Empty();
	TSharedPtr<FSceneData> data = GetSceneData(levelPath + "/" + levelName);

	//if the new scene is the same as the current scene, return
	if (LastSceneData.IsValid() && data.IsValid() && LastSceneData->Id == data->Id)
	{
		return;
	}

	if (LastSceneData.IsValid())
	{
		//send event for unloading LastSceneData before loading current scene
		TSharedPtr<FJsonObject> lastsceneproperties = MakeShareable(new FJsonObject());
		lastsceneproperties->SetStringField("Scene Name", FString(LastSceneData->Name));
		lastsceneproperties->SetStringField("Scene Id", FString(LastSceneData->Id));
		customEventRecorder->Send("c3d.SceneUnload", lastsceneproperties);
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
		customEventRecorder->Send("c3d.SceneLoad", properties);
	}

	//will send events to the unloaded level before changing tracking scene id
	//this is done instead of a full flush of all streams so that dynamics from the loaded scene are not written to the unloaded level data
	customEventRecorder->SendData(true);

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
	boundaryRecorder->SendData(true);
	OnCognitiveLevelChange.Broadcast();
}

void FAnalyticsCognitive3D::ShutdownModule()
{

}

bool FAnalyticsProviderCognitive3D::HasStartedSession()
{
	return bHasSessionStarted;
}

TSharedPtr<IAnalyticsProvider> FAnalyticsCognitive3D::CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const
{
	return AnalyticsProvider;
}

TWeakPtr<FAnalyticsProviderCognitive3D> FAnalyticsCognitive3D::GetCognitive3DProvider() const
{
	return Cognitive3DProvider;
}

FAnalyticsProviderCognitive3D::FAnalyticsProviderCognitive3D()
{
#if PLATFORM_ANDROID
	DeviceId = FPlatformMisc::GetDeviceId();
#else
	DeviceId = FPlatformMisc::GetHashedMacAddressString();
#endif
}

FAnalyticsProviderCognitive3D::~FAnalyticsProviderCognitive3D()
{
	UE_LOG(LogTemp, Warning, TEXT("FAnalyticsProviderCognitive3D Shutdown Module"));
}

bool FAnalyticsProviderCognitive3D::StartSession()
{
	return StartSession(TArray<FAnalyticsEventAttribute>());
}

bool FAnalyticsProviderCognitive3D::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
	//initialize log
	FCognitiveLog::Init();

	if (bHasSessionStarted)
	{
		FCognitiveLog::Warning("FAnalyticsProviderCognitive3D::StartSession already started");
		return false;
	}

	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == nullptr || !cognitiveActor->IsValidLowLevel())
	{
		FCognitiveLog::Error("FAnalyticsProviderCognitive3D::StartSession could not find Cognitive Actor in your level");
		return false;
	}

	auto currentWorld = cognitiveActor->GetWorld();
	if (currentWorld == nullptr)
	{
		FCognitiveLog::Error("FAnalyticsProviderCognitive3D::StartSession World not set. Are you missing a Cognitive Actor in your level?");
		return false;
	}

	FString C3DSettingsPath = GetSettingsFilePathRuntime();
	FString C3DKeysPath = GetKeysFilePathRuntime();

	ApplicationKey = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "Analytics", "ApiKey", false);
	AttributionKey = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "Analytics", "AttributionKey", false);

	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "AutomaticallySetTrackingScene", false);
	if (ValueReceived.Len() > 0)
	{
		AutomaticallySetTrackingScene = FCString::ToBool(*ValueReceived);
	}

	if (ApplicationKey.Len() == 0)
	{
#if WITH_EDITOR
		//FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("The Application Key is Invalid, this session will not be recorded on the dashboard. Please restart the editor."));
#endif
		FCognitiveLog::Error("FAnalyticsProviderCognitive3D::StartSession ApplicationKey is invalid");
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
		UE_LOG(LogTemp, Log, TEXT("FAnalyticsProviderCognitive3D::StartSession level post load level: %s"), *level->GetPathName());
		TArray<FString> PathParts;
		level->GetPathName().ParseIntoArray(PathParts, TEXT("/"), true);
		FString levelPath = "";
		if (PathParts.Num() > 0)
		{
			PathParts.RemoveAt(PathParts.Num() - 1); //remove the last part, which is the level name
			levelPath = FString::Join(PathParts, TEXT("/"));
			levelPath = "/" + levelPath; //add a leading slash
		}

		if (level != nullptr)
		{
			FString levelName = level->GetFullGroupName(true);
			TSharedPtr<FSceneData> data = GetSceneData(levelPath + "/" + levelName);
			if (data.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("FAnalyticsProviderCognitive3D::StartSession Scene Data is valid: %s"), *data->Name);
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
		PauseHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddRaw(this, &FAnalyticsProviderCognitive3D::HandleApplicationWillEnterBackground);	
	}
	if (!LevelLoadHandle.IsValid())
		LevelLoadHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FAnalyticsProviderCognitive3D::HandlePostLevelLoad);
	if (!SublevelLoadedHandle.IsValid())
		SublevelLoadedHandle = FWorldDelegates::LevelAddedToWorld.AddRaw(this, &FAnalyticsProviderCognitive3D::HandleSublevelLoaded);
	if (!SublevelUnloadedHandle.IsValid())
		SublevelUnloadedHandle = FWorldDelegates::LevelRemovedFromWorld.AddRaw(this, &FAnalyticsProviderCognitive3D::HandleSublevelUnloaded);

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
		SetSessionProperty("c3d.app.inEditor", false);
	}
	else
	{
		SetSessionProperty("c3d.app.inEditor", true);
	}


	customEventRecorder->StartSession();
	fixationDataRecorder->StartSession();
	dynamicObjectManager->OnSessionBegin();
	boundaryRecorder->StartSession();
	sensors->StartSession();
	OnSessionBegin.Broadcast();

	FCognitiveLog::Info("FAnalyticsProviderCognitive3D::StartSession");

	return bHasSessionStarted;
}

void FAnalyticsProviderCognitive3D::SetLobbyId(FString lobbyId)
{
	LobbyId = lobbyId;
}

void FAnalyticsProviderCognitive3D::SetSessionName(FString sessionName)
{
	bHasCustomSessionName = true;
	SetSessionProperty("c3d.sessionname", sessionName);
}

void FAnalyticsProviderCognitive3D::EndSession()
{
	if (bHasSessionStarted == false)
	{
		return;
	}

	//broadcast pre session end for pre-cleanup
	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == nullptr)
	{
		FCognitiveLog::Error("FAnalyticsProviderCognitive3D::EndSession cognitiveActor is null");
		return;
	}
	gazeDataRecorder->PreSessionEnd();
	customEventRecorder->PreSessionEnd();
	fixationDataRecorder->PreSessionEnd();
	dynamicObjectManager->OnPreSessionEnd();
	boundaryRecorder->PreSessionEnd();
	sensors->PreSessionEnd();

	OnPreSessionEnd.Broadcast();
	FlushAndCacheEvents();

	//OnPostSessionEnd broadcast. used by components to clean up anything, including delegates
	gazeDataRecorder->PostSessionEnd();
	customEventRecorder->PostSessionEnd();
	fixationDataRecorder->PostSessionEnd();
	dynamicObjectManager->OnPostSessionEnd();
	boundaryRecorder->PostSessionEnd();
	sensors->PostSessionEnd();
	OnPostSessionEnd.Broadcast();
	localCache->Close();

	//cleanup pause and level load delegates
	if (!PauseHandle.IsValid())
	{
		FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(PauseHandle);
		PauseHandle.Reset();
	}
	if (!LevelLoadHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(LevelLoadHandle);
		LevelLoadHandle.Reset();
	}
	if (!SublevelLoadedHandle.IsValid())
	{
		FWorldDelegates::LevelAddedToWorld.Remove(SublevelLoadedHandle);
		SublevelLoadedHandle.Reset();
	}
	if (!SublevelUnloadedHandle.IsValid())
	{
		FWorldDelegates::LevelRemovedFromWorld.Remove(SublevelUnloadedHandle);
		SublevelUnloadedHandle.Reset();
	}

	//reset variables
	SessionTimestamp = -1;
	SessionId = "";
	bHasCustomSessionName = false;
	bHasSessionStarted = false;
	FRemoteControlsRecorder::GetInstance()->bHasRemoteControlVariables = false;
	FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesString.Empty();
	FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesInt.Empty();
	FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesFloat.Empty();
	FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesBool.Empty();
	CurrentTrackingSceneId.Empty();
	LastSceneData.Reset();

	//broadcast end session
	FCognitiveLog::Info("Cognitive3D EndSession");
}

//broadcast to all listeners that outstanding data should be sent + written to cache as a backup
//should be used when it's unclear if the session will continue - a sudden pause event might indicate a loss of wifi connection
void FAnalyticsProviderCognitive3D::FlushAndCacheEvents()
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
void FAnalyticsProviderCognitive3D::FlushEvents()
{
	if (!bHasSessionStarted) { FCognitiveLog::Warning("Cognitive3D Flush Events, but Session has not started!"); return; }
	gazeDataRecorder->SendData(false);
	customEventRecorder->SendData(false);
	fixationDataRecorder->SendData(false);
	dynamicObjectManager->SendData(false);
	sensors->SendData(false);
	OnRequestSend.Broadcast(false);
}

void FAnalyticsProviderCognitive3D::SetUserID(const FString& InUserID)
{
	FString userId = InUserID;
	if (userId.Len() > 64)
	{
		FCognitiveLog::Error("FAnalyticsProviderCognitive3D::SetUserID exceeds maximum character limit. clipping to 64");
		int32 chopcount = userId.Len() - 64;
		userId = userId.LeftChop(chopcount);
	}

	ParticipantId = userId;
	OnParticipantIdSet.Broadcast(ParticipantId);
	SetParticipantProperty("id", userId);
	FCognitiveLog::Info("FAnalyticsProviderCognitive3D::SetUserID set user id: " + userId);
}

void FAnalyticsProviderCognitive3D::SetParticipantId(FString participantId)
{
	if (participantId.Len() > 64)
	{
		FCognitiveLog::Error("FAnalyticsProviderCognitive3D::SetParticipantId exceeds maximum character limit. clipping to 64");
		int32 chopcount = participantId.Len() - 64;
		participantId = participantId.LeftChop(chopcount);
	}

	ParticipantId = participantId;
	OnParticipantIdSet.Broadcast(ParticipantId);
	SetParticipantProperty("id", participantId);
	FCognitiveLog::Info("FAnalyticsProviderCognitive3D::SetParticipantId set user id: " + participantId);
}

void FAnalyticsProviderCognitive3D::SetParticipantFullName(FString participantName)
{
	ParticipantName = participantName;
	SetParticipantProperty("name", participantName);
	FCognitiveLog::Info("FAnalyticsProviderCognitive3D::SetParticipantData set user id");
	if (!bHasCustomSessionName)
		SetSessionProperty("c3d.sessionname", participantName);
}

void FAnalyticsProviderCognitive3D::SetSessionTag(FString Tag)
{
	SetSessionTag(Tag, true);
}

void FAnalyticsProviderCognitive3D::SetSessionTag(FString Tag, bool value)
{
	if (Tag.Len() == 0)
	{
		FCognitiveLog::Error("FAnalyticsProviderCognitive3D::SetSessionTag must contain > 0 characters");
		return;
	}
	if (Tag.Len() > 12)
	{
		FCognitiveLog::Error("FAnalyticsProviderCognitive3D::SetSessionTag must contain <= 12 characters");
		return;
	}

	if (value == true)
	{
		SetSessionProperty("c3d.session_tag." + Tag, true);
	}
	else
	{
		SetSessionProperty("c3d.session_tag." + Tag, false);
	}
}

FString FAnalyticsProviderCognitive3D::GetUserID() const
{
	return ParticipantId;
}

FString FAnalyticsProviderCognitive3D::GetCognitiveUserName() const
{
	return ParticipantName;
}

FString FAnalyticsProviderCognitive3D::GetDeviceID() const
{
	return DeviceId;
}

FString FAnalyticsProviderCognitive3D::GetSessionID() const
{
	return SessionId;
}

double FAnalyticsProviderCognitive3D::GetSessionTimestamp() const
{
	return SessionTimestamp;
}

bool FAnalyticsProviderCognitive3D::SetSessionID(const FString& InSessionID)
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
void FAnalyticsProviderCognitive3D::RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes)
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

void FAnalyticsProviderCognitive3D::RecordItemPurchase(const FString& ItemId, const FString& Currency, int32 PerItemCost, int32 ItemQuantity)
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

void FAnalyticsProviderCognitive3D::RecordCurrencyPurchase(const FString& GameCurrencyType, int32 GameCurrencyAmount, const FString& RealCurrencyType, float RealMoneyCost, const FString& PaymentProvider)
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

void FAnalyticsProviderCognitive3D::RecordCurrencyGiven(const FString& GameCurrencyType, int32 GameCurrencyAmount)
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

void FAnalyticsProviderCognitive3D::SetAge(const int32 InAge)
{
	SetParticipantProperty("age", InAge);
}

void FAnalyticsProviderCognitive3D::SetLocation(const FString& InLocation)
{
	SetSessionProperty("location", InLocation);
}

void FAnalyticsProviderCognitive3D::SetGender(const FString& InGender)
{
	SetParticipantProperty("gender", InGender);
}

void FAnalyticsProviderCognitive3D::SetBuildInfo(const FString& InBuildInfo)
{
	BuildInfo = InBuildInfo;
}

void FAnalyticsProviderCognitive3D::RecordError(const FString& Error, const TArray<FAnalyticsEventAttribute>& Attributes)
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

void FAnalyticsProviderCognitive3D::RecordProgress(const FString& ProgressType, const FString& ProgressName, const TArray<FAnalyticsEventAttribute>& Attributes)
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

void FAnalyticsProviderCognitive3D::RecordItemPurchase(const FString& ItemId, int32 ItemQuantity, const TArray<FAnalyticsEventAttribute>& Attributes)
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

void FAnalyticsProviderCognitive3D::RecordCurrencyPurchase(const FString& GameCurrencyType, int32 GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& Attributes)
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

void FAnalyticsProviderCognitive3D::RecordCurrencyGiven(const FString& GameCurrencyType, int32 GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& Attributes)
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

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4

TArray<FAnalyticsEventAttribute> DefaultEventAttributes;

void FAnalyticsProviderCognitive3D::SetDefaultEventAttributes(TArray<FAnalyticsEventAttribute>&& Attributes)
{
	DefaultEventAttributes = Attributes;
}

TArray<FAnalyticsEventAttribute> FAnalyticsProviderCognitive3D::GetDefaultEventAttributesSafe() const
{
	return DefaultEventAttributes;
}

int32 FAnalyticsProviderCognitive3D::GetDefaultEventAttributeCount() const
{
	return DefaultEventAttributes.Num();
}

FAnalyticsEventAttribute FAnalyticsProviderCognitive3D::GetDefaultEventAttribute(int AttributeIndex) const
{
	return DefaultEventAttributes[AttributeIndex];
}
#endif

TSharedPtr<FSceneData> FAnalyticsProviderCognitive3D::GetCurrentSceneData()
{
	return LastSceneData;
}

TSharedPtr<FSceneData> FAnalyticsProviderCognitive3D::GetSceneData(FString scenename)
{
	UE_LOG(LogTemp, Warning, TEXT("FAnalyticsProviderCognitive3D::GetSceneData %s"), *scenename);

	FString ShortName = scenename;
	FString PathName = "";

	if (scenename.Contains("/"))
	{
		TArray<FString> PathParts;
		scenename.ParseIntoArray(PathParts, TEXT("/"), true);
		if (PathParts.Num() > 0)
		{
			ShortName = PathParts.Last();
			PathParts.RemoveAt(PathParts.Num() - 1); //remove the last part, which is the level name
			PathName = FString::Join(PathParts, TEXT("/"));
			PathName = "/" + PathName; //add a leading slash
		}
	}

	for (int i = 0; i < SceneData.Num(); i++)
	{
		if (!SceneData[i].IsValid()) { continue; }
		if (SceneData[i]->Name == ShortName)
		{
			if (!PathName.IsEmpty() && SceneData[i]->Path != PathName)
			{
				//scene name matches, but path does not
				continue;
			}
			return SceneData[i];
		}
	}
	FCognitiveLog::Warning("FAnalyticsProviderCognitive3D::GetSceneData couldn't find SceneData for scene " + scenename);
	return NULL;
}

FString FAnalyticsProviderCognitive3D::GetSettingsFilePathRuntime() const
{
	// Get the project's Config directory.
	FString BaseConfigDir = FPaths::ProjectConfigDir();

	// Define the subfolder and ensure it exists.
	FString CustomFolder = FPaths::Combine(BaseConfigDir, TEXT("c3dlocal"));
	if (!FPaths::DirectoryExists(CustomFolder))
	{
		// Create the directory if it doesn't exist.
		IFileManager::Get().MakeDirectory(*CustomFolder);
	}

	// Combine the subfolder path with your INI file name.
	FString ConfigFilePath = FPaths::Combine(CustomFolder, TEXT("Cognitive3DSettings.ini"));

	// If the file doesn't exist, create it with some default content.
	if (!FPaths::FileExists(ConfigFilePath))
	{
		FString DefaultContent = TEXT("; Cognitive3D Plugin Settings\n[General]\n");
		if (!FFileHelper::SaveStringToFile(DefaultContent, *ConfigFilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create config file: %s"), *ConfigFilePath);
			return FString();
		}
	}
	return ConfigFilePath;
}

FString FAnalyticsProviderCognitive3D::GetKeysFilePathRuntime() const
{
	// Get the project's Config directory.
	FString BaseConfigDir = FPaths::ProjectConfigDir();
	FString BaseProjectDir = FPaths::ProjectDir();

	// Define the subfolder and ensure it exists.
	FString CustomFolder = FPaths::Combine(BaseProjectDir, TEXT("c3dlocal"));
	if (!FPaths::DirectoryExists(CustomFolder))
	{
		// Create the directory if it doesn't exist.
		IFileManager::Get().MakeDirectory(*CustomFolder);
	}

	// Combine the subfolder path with your INI file name.
	FString ConfigFilePath = FPaths::Combine(CustomFolder, TEXT("Cognitive3DKeys.ini"));

	// If the file doesn't exist, create it with some default content.
	if (!FPaths::FileExists(ConfigFilePath))
	{
		FString DefaultContent = TEXT("; Cognitive3D Plugin Keys\n[General]\n");
		if (!FFileHelper::SaveStringToFile(DefaultContent, *ConfigFilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create config file: %s"), *ConfigFilePath);
			return FString();
		}
	}
	return ConfigFilePath;
}

FString FAnalyticsProviderCognitive3D::GetCurrentSceneId()
{
	auto currentData = GetCurrentSceneData();
	if (!currentData.IsValid()) { return ""; }

	return currentData->Id;
}

FString FAnalyticsProviderCognitive3D::GetCurrentSceneVersionNumber()
{
	auto currentData = GetCurrentSceneData();
	if (!currentData.IsValid()) { return ""; }

	return FString::FromInt(currentData->VersionNumber);
}

void FAnalyticsProviderCognitive3D::CacheSceneData()
{
	TArray<FString>scenstrings;
	FString C3DSettingsPath = GetSettingsFilePathRuntime();
	GConfig->LoadFile(C3DSettingsPath);
	GConfig->GetArray(TEXT("/Script/Cognitive3D.Cognitive3DSceneSettings"), TEXT("SceneData"), scenstrings, C3DSettingsPath);
	GConfig->Flush(false, C3DSettingsPath);

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

		if (Array.Num() != 5)
		{
			FCognitiveLog::Error("FAnalyticsProviderCognitive3D::CacheSceneData failed to parse " + scenstrings[i]);
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("FAnalyticsProviderCognitive3D::CacheSceneData %s %s %s %s %s"), *Array[0], *Array[1], *Array[2], *Array[3], *Array[4]);
		SceneData.Add(MakeShareable(new FSceneData(Array[0], Array[1], Array[2], FCString::Atoi(*Array[3]), FCString::Atoi(*Array[4]))));
	}
}

bool FAnalyticsProviderCognitive3D::TryGetPlayerHMDPosition(FVector& vector)
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		FCognitiveLog::Warning("FAnalyticsProviderCognitive3D::GetPlayerHMDPosition no controllers skip");
		return false;
	}

	vector = controllers[0]->PlayerCameraManager->GetCameraLocation();
	return true;
}

 bool FAnalyticsProviderCognitive3D::TryGetPlayerHMDRotation(FRotator& rotator)
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		FCognitiveLog::Warning("FAnalyticsProviderCognitive3D::GetPlayerHMDPosition no controllers skip");
		return false;
	}

	rotator = controllers[0]->PlayerCameraManager->GetCameraRotation();
	return true;
}

bool FAnalyticsProviderCognitive3D::TryGetPlayerHMDLocalRotation(FRotator& rotator)
{
	//IMPROVEMENT cache this and check for null. playercontrollers DO NOT persist across level changes

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		FCognitiveLog::Warning("FAnalyticsProviderCognitive3D::GetPlayerHMDPosition no controllers skip");
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

void FAnalyticsProviderCognitive3D::SetSessionProperty(FString name, int32 value)
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
void FAnalyticsProviderCognitive3D::SetSessionProperty(FString name, float value)
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
void FAnalyticsProviderCognitive3D::SetSessionProperty(FString name, FString value)
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
void FAnalyticsProviderCognitive3D::SetSessionProperty(FString name, bool value)
{
	if (NewSessionProperties.HasField(name))
		NewSessionProperties.Values[name] = MakeShareable(new FJsonValueBoolean(value));
	else
		NewSessionProperties.SetBoolField(name, value);
	if (AllSessionProperties.HasField(name))
		AllSessionProperties.Values[name] = MakeShareable(new FJsonValueBoolean(value));
	else
		AllSessionProperties.SetBoolField(name, value);
}

void FAnalyticsProviderCognitive3D::SetParticipantProperty(FString name, int32 value)
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
void FAnalyticsProviderCognitive3D::SetParticipantProperty(FString name, float value)
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
void FAnalyticsProviderCognitive3D::SetParticipantProperty(FString name, FString value)
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
void FAnalyticsProviderCognitive3D::SetParticipantProperty(FString name, bool value)
{
	FString completeName = "c3d.participant." + name;
	if (NewSessionProperties.HasField(completeName))
		NewSessionProperties.Values[completeName] = MakeShareable(new FJsonValueBoolean(value));
	else
		NewSessionProperties.SetBoolField(completeName, value);
	if (AllSessionProperties.HasField(completeName))
		AllSessionProperties.Values[completeName] = MakeShareable(new FJsonValueBoolean(value));
	else
		AllSessionProperties.SetBoolField(completeName, value);
}

FJsonObject FAnalyticsProviderCognitive3D::GetNewSessionProperties()
{
	FJsonObject returnobject = FJsonObject(NewSessionProperties);
	NewSessionProperties = FJsonObject();
	return returnobject;
}

FJsonObject FAnalyticsProviderCognitive3D::GetAllSessionProperties()
{
	FJsonObject returnobject = FJsonObject(AllSessionProperties);
	return returnobject;
}

FString FAnalyticsProviderCognitive3D::GetAttributionParameters()
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

bool FAnalyticsProviderCognitive3D::HasEyeTrackingSDK()
{
#if defined INCLUDE_TOBII_PLUGIN
	return true;
#elif defined WAVEVR_EYETRACKING
	return true;
#elif defined OPENXR_EYETRACKING
	return true;
#elif defined INCLUDE_HPGLIA_PLUGIN
	return true;
#elif defined INCLUDE_PICOMOBILE_PLUGIN
	return true;
#elif defined INCLUDE_VARJO_PLUGIN
	return true;
#elif defined SRANIPAL_1_3_API
	return true;
#elif defined SRANIPAL_1_2_API
	return true;
#else
	return false;
#endif
}

bool FAnalyticsProviderCognitive3D::TryGetRoomSize(FVector& roomsize)
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
	//pico
#elif defined INCLUDE_PICO_PLUGIN
	FVector BoundaryDimensions = UPICOXRHMDFunctionLibrary::PXR_GetBoundaryDimensions(EPICOXRBoundaryType::PlayArea);
	roomsize.X = FMath::Abs(BoundaryDimensions.X);
	roomsize.Y = FMath::Abs(BoundaryDimensions.Y);
	return true;
	//crossplatform
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

bool FAnalyticsProviderCognitive3D::TryGetHMDGuardianPoints(TArray<FVector>& GuardianPoints, bool usePawnSpace)
{
#ifdef INCLUDE_OCULUS_PLUGIN
#if ENGINE_MAJOR_VERSION == 4 
	GuardianPoints = UOculusFunctionLibrary::GetGuardianPoints(EBoundaryType::Boundary_PlayArea, usePawnSpace);
	return true;
#elif ENGINE_MAJOR_VERSION == 5 
	GuardianPoints = UOculusXRFunctionLibrary::GetGuardianPoints(EOculusXRBoundaryType::Boundary_PlayArea, usePawnSpace);
	return true;
#endif
#elif (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION == 27) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 0)
	//OpenXR/SteamVR
	// 1) Query the full rectangle size
	//    This returns the width (X) and depth (Y) in Unreal units of the largest rectangle
	//    that fits inside your guardian/chaperone area in the Floor (stage) origin. 
	FVector2D Bounds = UHeadMountedDisplayFunctionLibrary::GetPlayAreaBounds();

	if (Bounds.Size() > 0)
	{
		GuardianPoints.Empty();
		// 2) Compute half-extents
		const float HalfX = Bounds.X * 0.5f;
		const float HalfY = Bounds.Y * 0.5f;

		// 3) Build the four corner points in the X/Y plane (Z = 0)
		//    These are centered on (0,0), which in tracking space is your floor-level origin.
		GuardianPoints.Add(FVector(HalfX, HalfY, 0.f));  // front-right
		GuardianPoints.Add(FVector(HalfX, -HalfY, 0.f));  // back-right
		GuardianPoints.Add(FVector(-HalfX, -HalfY, 0.f));  // back-left
		GuardianPoints.Add(FVector(-HalfX, HalfY, 0.f));  // front-left

		return true;
	}
	return false;
#else
	return false;
#endif
	
}

//this function tries to get the pose of an HMD inside the pre-set boundaries
//primarily used for stationary guardian intersection
bool FAnalyticsProviderCognitive3D::TryGetHMDPose(FRotator& HMDRotation, FVector& HMDPosition, FVector& HMDNeckPos)
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

bool FAnalyticsProviderCognitive3D::TryGetHMDWornState(EHMDWornState::Type& WornState)
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



bool FAnalyticsProviderCognitive3D::IsPointInPolygon4(TArray<FVector> polygon, FVector testPoint)
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


bool FAnalyticsProviderCognitive3D::IsPluginEnabled(const FString& PluginName)
{
	IPluginManager& PluginManager = IPluginManager::Get();
	TSharedPtr<IPlugin> Plugin = PluginManager.FindPlugin(PluginName);

	if (Plugin.IsValid())
	{
		return Plugin->IsEnabled();
	}

	return false;
}

TWeakObjectPtr<UDynamicObject> FAnalyticsProviderCognitive3D::GetControllerDynamic(bool right)
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

FString FAnalyticsProviderCognitive3D::GetRuntime()
{
	return FString();
}

void FAnalyticsProviderCognitive3D::HandleApplicationWillEnterBackground()
{
	FlushAndCacheEvents();
	if (!localCache.IsValid())
	{
		return;
	}
	localCache->SerializeToFile();
}

void FAnalyticsProviderCognitive3D::SetTrackingScene(FString levelName)
{
	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
	FString previousSceneName = LastSceneData->Name;
	properties->SetStringField("Scene Name", FString(LastSceneData->Name));
	properties->SetStringField("Scene Id", FString(LastSceneData->Id));
	properties->SetStringField("Destination Scene Name", levelName);
	float duration = FUtil::GetTimestamp() - SceneStartTime;
	properties->SetNumberField("Duration", duration);
	customEventRecorder->Send("c3d.SceneUnloaded", properties);

	FlushEvents();
	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	auto currentWorld = cognitiveActor -> GetWorld();
	auto level = currentWorld->GetCurrentLevel();
	TArray<FString> PathParts;
	level->GetPathName().ParseIntoArray(PathParts, TEXT("/"), true);
	FString levelPath = "";
	if (PathParts.Num() > 0)
	{
		PathParts.RemoveAt(PathParts.Num() - 1); //remove the last part, which is the level name
		levelPath = FString::Join(PathParts, TEXT("/"));
		levelPath = "/" + levelPath; //add a leading slash
	}

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

	properties->SetStringField("Scene Name", FString(data->Name));
	properties->SetStringField("Scene Id", FString(data->Id));
	properties->SetStringField("Previous Scene Name", FString(previousSceneName));
	properties->SetNumberField("Duration", duration);
	customEventRecorder->Send("c3d.SceneLoaded", properties);
}