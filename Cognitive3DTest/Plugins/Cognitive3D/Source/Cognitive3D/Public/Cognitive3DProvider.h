/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once


#include "CoreMinimal.h"
#include "Cognitive3D/Public/C3DCommonTypes.h"
#include "Interfaces/IAnalyticsProvider.h"
#include "HeadMountedDisplayTypes.h"
#include "Json.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManagerGeneric.h"
#include "Engine/Engine.h"
#include "Misc/Base64.h"
#include "Misc/PackageName.h"//to get friendly name of streaming levels

	//included here so the class can be saved as a variable without a circular reference (since these often need to reference the provider)

	class FNetwork;
	class FCustomEventRecorder;
	class Cognitive3DResponse;
	class FSensors;
	class FExitPoll;
	class FLocalCache;
	class UDynamicObject;
	class FGazeDataRecorder;
	class FFixationDataRecorder;
	//
	class FAnalyticsCognitive3D;
	class UCognitive3DBlueprints;
	class UFixationRecorder;
	class FDynamicObjectManager;
	class FLocalCache;
	class FCognitiveLog;

	class COGNITIVE3D_API FAnalyticsProviderCognitive3D : public IAnalyticsProvider
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
		FAnalyticsProviderCognitive3D();
		virtual ~FAnalyticsProviderCognitive3D();

		UPROPERTY(BlueprintAssignable, Category = "Cognitive3D Analytics")
			FOnCognitiveSessionBegin OnSessionBegin;
		UPROPERTY(BlueprintAssignable, Category = "Cognitive3D Analytics")
			FOnCognitivePreSessionEnd OnPreSessionEnd;
		UPROPERTY(BlueprintAssignable, Category = "Cognitive3D Analytics")
			FOnCognitivePostSessionEnd OnPostSessionEnd;
		UPROPERTY(BlueprintAssignable, Category = "Cognitive3D Analytics")
			FOnRequestSend OnRequestSend;
		UPROPERTY(BlueprintAssignable, Category = "Cognitive3D Analytics")
			FOnCognitiveInterval OnCognitiveInterval;

		UPROPERTY(BlueprintAssignable, Category = "Cognitive3D Analytics")
		FOnCognitiveParticipantIdSet OnParticipantIdSet;

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
		
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
		virtual void SetDefaultEventAttributes(TArray<FAnalyticsEventAttribute>&& Attributes) override;
		virtual TArray<FAnalyticsEventAttribute> GetDefaultEventAttributesSafe() const override;
		virtual int32 GetDefaultEventAttributeCount() const override;
		virtual FAnalyticsEventAttribute GetDefaultEventAttribute(int AttributeIndex) const override;
#endif

		//consider making these TSharedPtr
		FCustomEventRecorder* customEventRecorder = nullptr;
		FSensors* sensors = nullptr;
		FDynamicObjectManager* dynamicObjectManager = nullptr;
		FGazeDataRecorder* gazeDataRecorder = nullptr;
		FFixationDataRecorder* fixationDataRecorder = nullptr;
		TSharedPtr<FNetwork> network;
		TSharedPtr<FExitPoll> exitpoll;
		TSharedPtr<FLocalCache> localCache;

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

		FString GetSettingsFilePathRuntime() const;

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
		void SetParticipantProperty(FString name, bool value);
		void SetSessionTag(FString tag);
		void SetSessionTag(FString tag, bool value);

		void SetSessionProperty(FString name, int32 value);
		void SetSessionProperty(FString name, float value);
		void SetSessionProperty(FString name, FString value);
		void SetSessionProperty(FString name, bool value);

		FString GetAttributionParameters();

		//gameplay references
		bool HasEyeTrackingSDK();
		void SetTrackingScene(FString levelName);
		bool TryGetRoomSize(FVector& roomsize);
		bool TryGetHMDGuardianPoints(TArray<FVector>& GuardianPoints);
		bool TryGetHMDPose(FRotator& HMDRotation, FVector& HMDPosition, FVector& HMDNeckPos);
		bool TryGetHMDWornState(EHMDWornState::Type& WornState);
		bool IsPointInPolygon4(TArray<FVector> polygon, FVector testPoint);
		bool IsPluginEnabled(const FString& PluginName);
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
		bool AutomaticallySetTrackingScene = true;
		
	};