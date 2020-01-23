/*
	
*/
#include "CognitiveVRBlueprints.h"
//#include "Private/CognitiveVRPrivatePCH.h"

TSharedPtr<FAnalyticsProviderCognitiveVR> UCognitiveVRBlueprints::cog;

void UCognitiveVRBlueprints::SendCustomEvent(FString Category, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();
	UCognitiveVRBlueprints::SendCustomEventToCore(Category, Attributes, HMDPosition,NULL);
}

void UCognitiveVRBlueprints::SendCustomEventDynamic(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, UDynamicObject* dynamic, FVector Position)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();

	UCognitiveVRBlueprints::SendCustomEventToCore(Category, Attributes, HMDPosition, dynamic);
}

void UCognitiveVRBlueprints::SendCustomEventToCore(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position, UDynamicObject* dynamic)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SendCustomEventToCore could not get provider!");
		return;
	}
	if (!cog->customEventRecorder.IsValid())
	{
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	for (int32 i = 0; i < Attributes.Num(); i++)
	{
		if (Attributes[i].Name.Len() > 0)
		{
			properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
		}
	}

	FString dynamicId = "";
	if (dynamic != NULL)
	{
		dynamicId = dynamic->GetObjectId()->Id;
	}

	cog->customEventRecorder->Send(Category, Position, properties, dynamicId);
}

void UCognitiveVRBlueprints::SendCustomEventPosition(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	SendCustomEventToCore(Category, Attributes, Position, NULL);
}

void UCognitiveVRBlueprints::UpdateSessionInt(const FString name, const int32 value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateSessionInt could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitiveVRBlueprints::UpdateSessionFloat(const FString name, const float value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateSessionFloat could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitiveVRBlueprints::UpdateSessionString(const FString name, const FString value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateSessionString could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitiveVRBlueprints::SetSessionName(const FString name)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetSessionName could not get provider!");
		return;
	}
	cog->SetSessionName(name);
}

void UCognitiveVRBlueprints::SetLobbyId(const FString lobbyId)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return;
	}
	cog->SetLobbyId(lobbyId);
}

void UCognitiveVRBlueprints::RecordSensor(const FString Name, const float Value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::RecordSensor could not get provider!");
		return;
	}
	if (!cog->sensors.IsValid())
	{
		return;
	}
	cog->sensors->RecordSensor(Name, Value);
}

void UCognitiveVRBlueprints::GetQuestionSet(const FString Hook, FCognitiveExitPollResponse response)
{
	ExitPoll::MakeQuestionSetRequest(Hook, response);
}

bool UCognitiveVRBlueprints::HasSessionStarted()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		return false;
	}

	return cog->HasStartedSession();
}

FExitPollQuestionSet UCognitiveVRBlueprints::GetCurrentExitPollQuestionSet()
{
	return FExitPollQuestionSet();
}

void UCognitiveVRBlueprints::SendExitPollAnswers(const TArray<FExitPollAnswer>& Answers)
{
	ExitPoll::SendQuestionAnswers(Answers);
}

void UCognitiveVRBlueprints::FlushEvents()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return;
	}
	cog->FlushEvents();
}

bool UCognitiveVRBlueprints::StartSession()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return false;
	}
	return cog->StartSession(TArray<FAnalyticsEventAttribute>());
}

void UCognitiveVRBlueprints::EndSession()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return;
	}
	cog->EndSession();
}

void UCognitiveVRBlueprints::SetUserId(const FString Name)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetLobbyId could not get provider!");
		return;
	}
	cog->SetUserID(Name);
}

FCustomEvent UCognitiveVRBlueprints::MakeCustomEvent(FString eventName)
{
	return FCustomEvent(eventName);
}

FCustomEvent UCognitiveVRBlueprints::SetIntegerProperty(UPARAM(ref) FCustomEvent& target, FString key, int32 intValue)
{
	target.SetProperty(key, intValue);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetStringProperty(UPARAM(ref) FCustomEvent& target, FString key, FString stringValue)
{
	target.SetProperty(key, stringValue);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetBoolProperty(UPARAM(ref) FCustomEvent& target, FString key, bool boolValue)
{
	target.SetProperty(key, boolValue);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetFloatProperty(UPARAM(ref) FCustomEvent& target, FString key, float floatValue)
{
	target.SetProperty(key, floatValue);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetDynamicObject(UPARAM(ref) FCustomEvent& target, UDynamicObject* dynamicObject)
{
	if (dynamicObject == NULL)
	{
		return target;
	}
	target.SetDynamicObject(dynamicObject);
	return target;
}
FCustomEvent UCognitiveVRBlueprints::SetPosition(UPARAM(ref) FCustomEvent& target, FVector position)
{
	target.SetPosition(position);
	return target;
}


void UCognitiveVRBlueprints::Send(UPARAM(ref) FCustomEvent& target)
{
	target.Send();
}

FCustomEvent UCognitiveVRBlueprints::AppendAllSensors(UPARAM(ref) FCustomEvent& target)
{
	target.AppendSensors();
	return target;
}

FCustomEvent UCognitiveVRBlueprints::AppendSensors(UPARAM(ref) FCustomEvent& target, TArray<FString> keys)
{
	target.AppendSensors(keys);
	return target;
}

FCustomEvent UCognitiveVRBlueprints::AppendSensor(UPARAM(ref) FCustomEvent& target, FString key)
{
	target.AppendSensor(key);
	return target;
}



//debug stuff
FString UCognitiveVRBlueprints::GetSessionName()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return ""; }
	return cog->GetSessionID();
#else
	return "";
#endif
}

float UCognitiveVRBlueprints::GetSessionDuration()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!cog->HasStartedSession()) { return 0; }

	float dur = Util::GetTimestamp() - cog->GetSessionTimestamp();
	return dur;
#else
	return 0;
#endif
}

FString UCognitiveVRBlueprints::GetSceneName()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return ""; }
	if (!cog->HasStartedSession()){ return ""; }

	auto data = cog->GetCurrentSceneData();

	if (data.IsValid())
	{
		return data->Name;
	}
	return "invalid";
#else
	return "";
#endif
}

FString UCognitiveVRBlueprints::GetSceneId()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return ""; }
	if (!cog->HasStartedSession()) { return ""; }

	auto data = cog->GetCurrentSceneData();

	if (data.IsValid())
	{
		return data->Id;
	}
	return "invalid";
#else
	return "";
#endif
}


float UCognitiveVRBlueprints::GetLastEventSendTime()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->customEventRecorder->GetLastSendTime();
#else
	return 0;
#endif
}

float UCognitiveVRBlueprints::GetLastGazeSendTime()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }

	if (UPlayerTracker::GetPlayerTracker() == nullptr)
	{
		return 0;
	}

	return UPlayerTracker::GetPlayerTracker()->GetLastSendTime();
#else
	return 0;
#endif
}

float UCognitiveVRBlueprints::GetLastDynamicSendTime()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return UDynamicObject::GetLastSendTime();
#else
	return 0;
#endif
}

float UCognitiveVRBlueprints::GetLastSensorSendTime()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->sensors->GetLastSendTime();
#else
	return 0;
#endif
}

float UCognitiveVRBlueprints::GetLastFixationSendTime()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }

	if (UFixationRecorder::GetFixationRecorder() == nullptr)
	{
		return 0;
	}
	return UFixationRecorder::GetFixationRecorder()->GetLastSendTime();
#else
	return 0;
#endif
}


TArray<FString> UCognitiveVRBlueprints::GetDebugQuestionSet()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	auto qs = ExitPoll::GetCurrentQuestionSet();
	TArray<FString> questions;

	for (auto& Elem : qs.questions)
	{
		questions.Add(Elem.title);
	}
	return questions;
#else
	return TArray<FString>();
#endif
}

TArray<FString> UCognitiveVRBlueprints::GetSensorKeys()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return TArray<FString>(); }
	if (!HasSessionStarted()) { return TArray<FString>(); }

	auto map = cog->sensors->GetLastSensorValues();
	
	TArray<FString> keys;

	for (auto& Elem : map)
	{
		keys.Add(Elem.Key);
	}
	return keys;
#else
	return TArray<FString>();
#endif
}

TArray<FString> UCognitiveVRBlueprints::GetSensorValues()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return TArray<FString>(); }
	if (!HasSessionStarted()) { return TArray<FString>(); }

	auto map = cog->sensors->GetLastSensorValues();

	TArray<FString> values;
	for (auto& Elem : map)
	{
		values.Add(FString::SanitizeFloat(Elem.Value));
	}
	return values;
#else
	return TArray<FString>();
#endif
}

bool UCognitiveVRBlueprints::IsFixating()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	auto fixptr = UFixationRecorder::GetFixationRecorder();
	if (fixptr == NULL) { return false; }
	return fixptr->IsFixating();
#else
	return false;
#endif
}
			 
int32 UCognitiveVRBlueprints::GetEventPartNumber()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->customEventRecorder->GetPartNumber();
#else
	return 0;
#endif
}
			 
int32 UCognitiveVRBlueprints::GetGazePartNumber()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }

	auto gazeptr = UPlayerTracker::GetPlayerTracker();
	if (gazeptr == NULL) { return 0; }
	return gazeptr->GetPartNumber();
#else
	return 0;
#endif
}
			 
int32 UCognitiveVRBlueprints::GetDynamicPartNumber()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return UDynamicObject::GetPartNumber();
#else
	return 0;
#endif
}

int32 UCognitiveVRBlueprints::GetSensorPartNumber()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->sensors->GetPartNumber();
#else
	return 0;
#endif
}

int32 UCognitiveVRBlueprints::GetFixationPartNumber()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	
	auto fixptr = UFixationRecorder::GetFixationRecorder();
	if (fixptr == NULL) { return 0; }
	return fixptr->GetPartNumber();
#else
	return 0;
#endif
}


int32 UCognitiveVRBlueprints::GetEventDataPointCount()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->customEventRecorder->GetDataPoints();
#else
return 0;
#endif
}

int32 UCognitiveVRBlueprints::GetGazePointCount()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	auto gazeptr = UPlayerTracker::GetPlayerTracker();
	if (gazeptr == NULL) { return 0; }
	return gazeptr->GetDataPoints();
#else
	return 0;
#endif
}

int32 UCognitiveVRBlueprints::GetDynamicDataCount()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }

	return UDynamicObject::GetDataPoints();
#else
	return 0;
#endif
}

int32 UCognitiveVRBlueprints::GetDynamicObjectCount()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	
	return UDynamicObject::GetDynamicObjectCount();
#else
	return 0;
#endif
}

int32 UCognitiveVRBlueprints::GetFixationPointCount()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	
	if (UFixationRecorder::GetFixationRecorder() == nullptr)
	{
		return 0;
	}
	return UFixationRecorder::GetFixationRecorder()->GetDataPoints();

#else
	return 0;
#endif
}

int32 UCognitiveVRBlueprints::GetSensorDataPointCount()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->sensors->GetDataPoints();
#else
	return 0;
#endif
}