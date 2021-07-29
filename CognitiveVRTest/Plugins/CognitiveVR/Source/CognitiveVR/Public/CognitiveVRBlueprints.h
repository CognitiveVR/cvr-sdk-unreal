/*
	
*/
#pragma once

#include "CognitiveVR/Public/C3DCommonTypes.h"
#include "CoreMinimal.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Public/CognitiveVRProvider.h"
#include "AnalyticsBlueprintLibrary.h"
#include "Runtime/Analytics/Analytics/Public/AnalyticsEventAttribute.h"
#include "CognitiveVR/Public/DynamicObject.h"
#include "CognitiveVR/Private/PlayerTracker.h"
#include "CognitiveVR/Private/ExitPoll.h"
#include "CognitiveVR/Private/FixationRecorder.h"
#include "CognitiveVR/Public/CustomEvent.h"
#include "CognitiveVRBlueprints.generated.h"

class CognitiveVRResponse;

UCLASS()
class COGNITIVEVR_API UCognitiveVRBlueprints : public UBlueprintFunctionLibrary
{
	friend class FAnalyticsProviderCognitiveVR;

	GENERATED_BODY()

		static void SendCustomEventToCore(FString Name, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position, UDynamicObject* dynamic);
		static TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
public:

	//record an event with attributes at a position associated to a dynamic object
	//attributes and dynamic are both optional
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "Attributes"), Category = "CognitiveVR Analytics|Custom Events")
	static void SendCustomEventDynamic(FString Name, const TArray<FAnalyticsEventAttr>& Attributes, class UDynamicObject* dynamic, FVector Position);
	
	//record an event with attributes
	//records the event at the position of the HMD
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "Attributes"), Category = "CognitiveVR Analytics|Custom Events")
	static void SendCustomEvent(FString Name, const TArray<FAnalyticsEventAttr>& Attributes);

	//record an event with attributes at a position
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "Attributes"), Category = "CognitiveVR Analytics|Custom Events")
	static void SendCustomEventPosition(FString Name, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position);

	//add an integer value to the session properties
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties", DisplayName = "Set Session Property Int")
		static void UpdateSessionInt(const FString name, const int32 value);
	//add a float value to the session properties
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties", DisplayName = "Set Session Property Float")
		static void UpdateSessionFloat(const FString name, const float value);
	//add a string value to the session properties
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties", DisplayName = "Set Session Property String")
		static void UpdateSessionString(const FString name, const FString value);

	//label this session to a human-friendly on the Cognitive dashboard
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
		static void SetSessionName(const FString name);
	//set a lobby id to connect multiple sessions together as a single multiplayer experience
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
		static void SetLobbyId(const FString lobbyId);

	//record a point of sensor data
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static void RecordSensor(const FString Name, const float Value);

	//request a question set by a hook name from the Cognitive dashboard
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Exit Poll")
	static void GetQuestionSet(const FString Hook, FCognitiveExitPollResponse response);

	//return the values of the current question set
	UFUNCTION(BlueprintPure, Category = "CognitiveVR Analytics|Exit Poll")
	static FExitPollQuestionSet GetCurrentExitPollQuestionSet();

	//record all responses to a questionset
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Exit Poll")
	static void SendExitPollAnswers(const TArray<FExitPollAnswer>& Answers);


	//send all outstanding data to the Cognitive dashboard
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static void FlushEvents();
	//enable recording data
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static bool StartSession();
	//sends all outstanding data and stops recording data
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static void EndSession();

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static void SetParticipantFullName(const FString Name);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static void SetParticipantId(const FString Id);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static void SetSessionTag(const FString Tag);

	//add an integer value to the session properties
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties")
		static void SetParticipantPropertyInt(const FString name, const int32 value);
	//add a float value to the session properties
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties")
		static void SetParticipantPropertyFloat(const FString name, const float value);
	//add a string value to the session properties
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties")
		static void SetParticipantPropertyString(const FString name, const FString value);


	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* MakeCustomEvent(FString eventName);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* SetFloatProperty(UCustomEvent* target, FString key, float floatValue);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* SetIntegerProperty(UCustomEvent* target, FString key, int32 intValue);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* SetStringProperty(UCustomEvent* target, FString key, FString stringValue);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* SetBoolProperty(UCustomEvent* target, FString key, bool boolValue);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* SetDynamicObject(UCustomEvent* target, UDynamicObject* dynamicObject);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* SetPosition(UCustomEvent* target, FVector position);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static void Send(UCustomEvent* target);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events", DisplayName = "Send At HMD Position")
		static void SendAtHMDPosition(UCustomEvent* target);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* AppendSensor(UCustomEvent* target, FString sensorName);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* AppendSensors(UCustomEvent* target, TArray<FString> sensorNames);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static UCustomEvent* AppendAllSensors(UCustomEvent* target);

	UFUNCTION(BlueprintPure, Category = "CognitiveVR Analytics")
		static bool HasSessionStarted();


	//active session view
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static FString GetSessionName();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static float GetSessionDuration();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static FString GetSceneName();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static FString GetSceneId();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static float GetLastEventSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static float GetLastGazeSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static float GetLastDynamicSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static float GetLastSensorSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static float GetLastFixationSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static bool IsFixating();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetEventPartNumber();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetGazePartNumber();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetDynamicPartNumber();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetSensorPartNumber();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetFixationPartNumber();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static TArray<FString> GetDebugQuestionSet();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static TArray<FString> GetSensorKeys();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static TArray<FString> GetSensorValues();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetEventDataPointCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetGazePointCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetDynamicDataCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetDynamicObjectCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetFixationPointCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static int32 GetSensorDataPointCount();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "CognitiveVR Analytics|Debug")
		static bool HasNetworkError();

	UFUNCTION(BlueprintPure, Category = "CognitiveVR Analytics")
		static FString GetAttributionParameters();

};
