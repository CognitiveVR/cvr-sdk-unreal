// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CognitiveVR/Public/C3DCommonTypes.h"
#include "CoreMinimal.h"
#include "Analytics.h"
#include "TimerManager.h"
#include "AnalyticsEventAttribute.h"
#include "Interfaces/IAnalyticsProvider.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Public/CognitiveVRBlueprints.h"

#include "HeadMountedDisplay.h"
#include "CognitiveVR/Public/CognitiveVRSettings.h"
#include "CognitiveVR/Private/ExitPoll.h"
#include "CognitiveVR/Private/PlayerTracker.h"
#include "CognitiveVR/Public/DynamicObject.h"
#include "CognitiveVR/Private/FixationRecorder.h"
#include "CognitiveVR/Public/CognitiveVRActor.h"
#include "Classes/Camera/CameraComponent.h"

#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Private/util/cognitive_log.h"
#include "CognitiveVR/Private/network/network.h"
#include "CognitiveVR/Private/api/customeventrecorder.h"
#include "CognitiveVR/Public/CustomEvent.h"
#include "CognitiveVR/Private/api/sensor.h"
#include "CognitiveVR/Private/LocalCache.h"
#include "Engine/Engine.h"
#include "Misc/Base64.h"
#include "CognitiveVR/Private/api/FixationDataRecorder.h"
#include "CognitiveVR/Private/api/GazeDataRecorder.h"
#include "Misc/PackageName.h"//to get friendly name of streaming levels

//OCULUS
#include "OculusFunctionLibrary.h"

	//included here so the class can be saved as a variable without a circular reference (since these often need to reference the provider)
	//everything here is referenced from headers. why is this being forward declared?
	class Network;
	class UCustomEventRecorder;
	class CognitiveVRResponse;
	class USensors;
	class ExitPoll;
	class LocalCache;
	class UDynamicObject;
	class UGazeDataRecorder;
	class UFixationDataRecorder;

	class COGNITIVEVR_API FAnalyticsProviderCognitiveVR : public IAnalyticsProvider
	{
		/** Unique Id representing the session the analytics are recording for */
		FString SessionId = "";
		/** Holds the Age if set */
		/** Holds the build info if set */
		FString BuildInfo = "";
		FString ParticipantName = "";
		/** Id representing the user the analytics are recording for */
		FString ParticipantId = "";
		FString DeviceId = "";
		double SessionTimestamp = -1;
		FJsonObject NewSessionProperties;
		FJsonObject AllSessionProperties;

	private:
		//reads all scene data from engine ini
		void CacheSceneData();
		static bool bHasSessionStarted;
		double SceneStartTime = 0;

	public:
		FAnalyticsProviderCognitiveVR();
		virtual ~FAnalyticsProviderCognitiveVR();

		UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
			FOnCognitiveSessionBegin OnSessionBegin;
		UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
			FOnCognitivePreSessionEnd OnPreSessionEnd;
		UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
			FOnCognitivePostSessionEnd OnPostSessionEnd;
		UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
			FOnRequestSend OnRequestSend;
		UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
			FOnCognitiveInterval OnCognitiveInterval;
		

		bool StartSession();
		virtual bool StartSession(const TArray<FAnalyticsEventAttribute>& Attributes) override;
		virtual void EndSession() override;
		virtual void FlushEvents() override;
		void FlushAndCacheEvents();

		virtual void SetUserID(const FString& InUserID) override;
		//returns unique participant id
		virtual FString GetUserID() const override;
		//returns participant name
		FString GetCognitiveUserName() const;

		virtual FString GetSessionID() const override;
		//allows developers to manually set a sesison id instead of generating one
		virtual bool SetSessionID(const FString& InSessionID) override;

		virtual void RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes) override;

		virtual void RecordItemPurchase(const FString& ItemId, const FString& Currency, int32 PerItemCost, int32 ItemQuantity) override;

		virtual void RecordCurrencyPurchase(const FString& GameCurrencyType, int32 GameCurrencyAmount, const FString& RealCurrencyType, float RealMoneyCost, const FString& PaymentProvider) override;

		virtual void RecordCurrencyGiven(const FString& GameCurrencyType, int32 GameCurrencyAmount) override;

		virtual void SetBuildInfo(const FString& InBuildInfo) override;
		virtual void SetGender(const FString& InGender) override;
		virtual void SetLocation(const FString& InLocation) override;
		virtual void SetAge(const int32 InAge) override;

		virtual void RecordItemPurchase(const FString& ItemId, int32 ItemQuantity, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;
		virtual void RecordCurrencyPurchase(const FString& GameCurrencyType, int32 GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;
		virtual void RecordCurrencyGiven(const FString& GameCurrencyType, int32 GameCurrencyAmount, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;
		virtual void RecordError(const FString& Error, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;
		virtual void RecordProgress(const FString& ProgressType, const FString& ProgressHierarchy, const TArray<FAnalyticsEventAttribute>& EventAttrs) override;
		
		//consider making these TSharedPtr
		UCustomEventRecorder* customEventRecorder = nullptr;
		USensors* sensors = nullptr;
		UDynamicObjectManager* dynamicObjectManager = nullptr;
		UGazeDataRecorder* gazeDataRecorder = nullptr;
		UFixationDataRecorder* fixationDataRecorder = nullptr;
		TSharedPtr<Network> network;
		TSharedPtr<ExitPoll> exitpoll;
		TSharedPtr<LocalCache> localCache;

		FString GetDeviceID() const;

		void SetLobbyId(FString lobbyId);
		FString LobbyId;

		double GetSessionTimestamp() const;

		bool TryGetPlayerHMDPosition(FVector& vector);
		bool TryGetPlayerHMDRotation(FRotator& rotator);
		bool TryGetPlayerHMDLocalRotation(FRotator& rotator);
		bool HasStartedSession();

		FString ApplicationKey = "";
		FString AttributionKey = "";

		FString GetCurrentSceneId();
		FString GetCurrentSceneVersionNumber();
		//if a session name has been explicitly set. otherwise will use participant name when that is set
		bool bHasCustomSessionName = false;
		void SetSessionName(FString sessionName);
		
		//used to identify when a scene changes and session properties need to be resent
		FString CurrentTrackingSceneId;
		//used to see id the current scene has changed and needs to search for new sceneId
		TSharedPtr<FSceneData> LastSceneData;
		//holds scenedata for all open scenes (including sublevels). used to get the latest data after a sublevel is loaded
		TArray<TSharedPtr<FSceneData>> LoadedSceneDataStack;

		TArray<TSharedPtr<FSceneData>> SceneData;
		TSharedPtr<FSceneData> GetSceneData(FString scenename);
		TSharedPtr<FSceneData> GetCurrentSceneData();
		FJsonObject GetNewSessionProperties();
		FJsonObject GetAllSessionProperties();

		bool ForceWriteSessionMetadata = false;

		void SetParticipantFullName(FString participantName);
		void SetParticipantId(FString participantId);
		void SetParticipantProperty(FString name, int32 value);
		void SetParticipantProperty(FString name, float value);
		void SetParticipantProperty(FString name, FString value);
		void SetSessionTag(FString tag);

		void SetSessionProperty(FString name, int32 value);
		void SetSessionProperty(FString name, float value);
		void SetSessionProperty(FString name, FString value);

		FString GetAttributionParameters();

		//gameplay references
		bool HasEyeTrackingSDK();
		bool TryGetRoomSize(FVector& roomsize);
		TWeakObjectPtr<UDynamicObject> GetControllerDynamic(const bool right);
		FString GetRuntime();

		private:
		FDelegateHandle PauseHandle;
		FDelegateHandle LevelLoadHandle;
		FDelegateHandle SublevelLoadedHandle;
		FDelegateHandle SublevelUnloadedHandle;

		void HandlePostLevelLoad(UWorld* world);
		void HandleSublevelLoaded(ULevel* level, UWorld* world);
		void HandleSublevelUnloaded(ULevel* level, UWorld* world);
		void HandleApplicationWillEnterBackground();
	};