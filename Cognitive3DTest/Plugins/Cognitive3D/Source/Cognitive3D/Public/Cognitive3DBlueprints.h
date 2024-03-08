/*
	
*/
#pragma once

#include "Cognitive3D/Public/C3DCommonTypes.h"
#include "CoreMinimal.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "AnalyticsBlueprintLibrary.h"
#include "Runtime/Analytics/Analytics/Public/AnalyticsEventAttribute.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "Cognitive3D/Private/C3DComponents/PlayerTracker.h"
#include "Cognitive3D/Private/ExitPoll.h"
#include "Cognitive3D/Private/C3DComponents/FixationRecorder.h"
#include "Cognitive3D/Public/DynamicObjectManager.h"
#include "Cognitive3D/Public/CustomEvent.h"
#include "Cognitive3DBlueprints.generated.h"

class Cognitive3DResponse;

UCLASS()
class COGNITIVE3D_API UCognitive3DBlueprints : public UBlueprintFunctionLibrary
{
	friend class IAnalyticsProviderCognitive3D;

	GENERATED_BODY()

		static void SendCustomEventToCore(FString Name, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position, UDynamicObject* dynamic);
		static TSharedPtr<IAnalyticsProviderCognitive3D> cog;
public:

	//record an event with attributes at a position associated to a dynamic object
	//attributes and dynamic are both optional
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "Attributes"), Category = "Cognitive3D Analytics|Custom Events")
	static void SendCustomEventDynamic(FString Name, const TArray<FAnalyticsEventAttr>& Attributes, class UDynamicObject* dynamic, FVector Position);
	
	//record an event with attributes
	//records the event at the position of the HMD
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "Attributes"), Category = "Cognitive3D Analytics|Custom Events")
	static void SendCustomEvent(FString Name, const TArray<FAnalyticsEventAttr>& Attributes);

	//record an event with attributes at a position
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "Attributes"), Category = "Cognitive3D Analytics|Custom Events")
	static void SendCustomEventPosition(FString Name, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position);

	//add an integer value to the session properties
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Properties", DisplayName = "Set Session Property Int")
		static void UpdateSessionInt(const FString name, const int32 value);
	//add a float value to the session properties
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Properties", DisplayName = "Set Session Property Float")
		static void UpdateSessionFloat(const FString name, const float value);
	//add a string value to the session properties
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Properties", DisplayName = "Set Session Property String")
		static void UpdateSessionString(const FString name, const FString value);

	//label this session to a human-friendly on the Cognitive dashboard
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
		static void SetSessionName(const FString name);
	//set a lobby id to connect multiple sessions together as a single multiplayer experience
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
		static void SetLobbyId(const FString lobbyId);

	//record a point of sensor data
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
	static void InitializeSensor(const FString Name, const float HzRate = 10, const float InitialValue = 0);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
	static void RecordSensor(const FString Name, const float Value);

	//request a question set by a hook name from the Cognitive dashboard
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Exit Poll")
	static void GetQuestionSet(const FString Hook, FCognitiveExitPollResponse response);

	//return the values of the current question set
	UFUNCTION(BlueprintPure, Category = "Cognitive3D Analytics|Exit Poll")
	static FExitPollQuestionSet GetCurrentExitPollQuestionSet();

	//record all responses to a questionset
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Exit Poll")
	static void SendExitPollAnswers(const TArray<FExitPollAnswer>& Answers);


	//send all outstanding data to the Cognitive dashboard
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
	static void FlushEvents();
	//enable recording data
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
	static bool StartSession();
	//sends all outstanding data and stops recording data
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
	static void EndSession();

	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
	static void SetParticipantFullName(const FString Name);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
	static void SetParticipantId(const FString Id);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
	static void SetSessionTag(const FString Tag);

	//add an integer value to the session properties
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Properties")
		static void SetParticipantPropertyInt(const FString name, const int32 value);
	//add a float value to the session properties
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Properties")
		static void SetParticipantPropertyFloat(const FString name, const float value);
	//add a string value to the session properties
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Properties")
		static void SetParticipantPropertyString(const FString name, const FString value);


	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* MakeCustomEvent(FString eventName);

	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* SetFloatProperty(UCustomEvent* target, FString key, float floatValue);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* SetIntegerProperty(UCustomEvent* target, FString key, int32 intValue);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* SetStringProperty(UCustomEvent* target, FString key, FString stringValue);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* SetBoolProperty(UCustomEvent* target, FString key, bool boolValue);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* SetDynamicObject(UCustomEvent* target, UDynamicObject* dynamicObject);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* SetPosition(UCustomEvent* target, FVector position);

	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static void Send(UCustomEvent* target);

	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events", DisplayName = "Send At HMD Position")
		static void SendAtHMDPosition(UCustomEvent* target);

	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* AppendSensor(UCustomEvent* target, FString sensorName);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* AppendSensors(UCustomEvent* target, TArray<FString> sensorNames);
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics|Custom Events")
		static UCustomEvent* AppendAllSensors(UCustomEvent* target);

	UFUNCTION(BlueprintPure, Category = "Cognitive3D Analytics")
		static bool HasSessionStarted();


	//active session view
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static FString GetSessionName();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static float GetSessionDuration();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static FString GetSceneName();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static FString GetSceneId();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static float GetLastEventSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static float GetLastGazeSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static float GetLastDynamicSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static float GetLastSensorSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static float GetLastFixationSendTime();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static bool IsFixating();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetEventPartNumber();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetGazePartNumber();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetDynamicPartNumber();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetSensorPartNumber();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetFixationPartNumber();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static TArray<FString> GetDebugQuestionSet();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static TArray<FString> GetSensorKeys();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static TArray<FString> GetSensorValues();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetEventDataPointCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetGazePointCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetDynamicDataCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetDynamicObjectCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetFixationPointCount();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static int32 GetSensorDataPointCount();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2"), Category = "Cognitive3D Analytics|Debug")
		static bool HasNetworkError();

	UFUNCTION(BlueprintPure, Category = "Cognitive3D Analytics")
		static FString GetAttributionParameters();

	UFUNCTION(BlueprintPure, Category = "Cognitive3D Analytics")
		static UFixationRecorder* GetFixationRecorder();

	UFUNCTION(BlueprintPure, Category = "Cognitive3D Analytics")
		static UPlayerTracker* GetPlayerTracker();

	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
		static void SetTrackingScene(const FString SceneName);
};
