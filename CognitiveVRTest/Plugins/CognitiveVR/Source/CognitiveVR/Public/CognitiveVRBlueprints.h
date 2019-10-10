/*
	
*/
#pragma once


#include "CognitiveVR.h"
#include "CognitiveVRProvider.h"
#include "Private/CognitiveVRPrivatePCH.h"
#include "Engine.h"
#include "Classes/AnalyticsBlueprintLibrary.h"
//#include "EngineAnalytics.h"
#include "Runtime/Analytics/Analytics/Public/AnalyticsEventAttribute.h"
//#include "IAnalyticsProvider.h"
#include "DynamicObject.h"
#include "PlayerTracker.h"
#include "ExitPoll.h"
//#include "LatentActions.h"
#include "DynamicObject.h"
#include "CognitiveVRBlueprints.generated.h"

class CognitiveVRResponse;

UCLASS()
class COGNITIVEVR_API UCognitiveVRBlueprints : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		static void SendCustomEventToCore(FString Name, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position, UDynamicObject* dynamic);
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
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties")
		static void UpdateSessionInt(const FString name, const int32 value);
	//add a float value to the session properties
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties")
		static void UpdateSessionFloat(const FString name, const float value);
	//add a string value to the session properties
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties")
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
	//set a human-friendly name to label the user on the Cognitive dashboard
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static void SetUserId(const FString Name);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent MakeCustomEvent(FString eventName);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent SetFloatProperty(UPARAM(ref) FCustomEvent& target, FString key, float floatValue);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent SetIntegerProperty(UPARAM(ref) FCustomEvent& target, FString key, int32 intValue);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent SetStringProperty(UPARAM(ref) FCustomEvent& target, FString key, FString stringValue);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent SetBoolProperty(UPARAM(ref) FCustomEvent& target, FString key, bool boolValue);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent SetDynamicObject(UPARAM(ref) FCustomEvent& target, UDynamicObject* dynamicObject);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent SetPosition(UPARAM(ref) FCustomEvent& target, FVector position);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static void Send(UPARAM(ref) FCustomEvent& target);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent AppendSensor(UPARAM(ref) FCustomEvent& target, FString sensorName);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent AppendSensors(UPARAM(ref) FCustomEvent& target, TArray<FString> sensorNames);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
		static FCustomEvent AppendAllSensors(UPARAM(ref) FCustomEvent& target);

	UFUNCTION(BlueprintPure, Category = "CognitiveVR Analytics")
		static bool HasSessionStarted();
};
