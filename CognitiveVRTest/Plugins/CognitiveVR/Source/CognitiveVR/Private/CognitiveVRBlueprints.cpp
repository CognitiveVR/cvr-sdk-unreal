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

void UCognitiveVRBlueprints::SetParticipantPropertyInt(const FString name, const int32 value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetParticipantPropertyInt could not get provider!");
		return;
	}
	cog->SetParticipantProperty(name, value);
}

void UCognitiveVRBlueprints::SetParticipantPropertyFloat(const FString name, const float value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetParticipantPropertyFloat could not get provider!");
		return;
	}
	cog->SetParticipantProperty(name, value);
}

void UCognitiveVRBlueprints::SetParticipantPropertyString(const FString name, const FString value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetParticipantPropertyString could not get provider!");
		return;
	}
	cog->SetParticipantProperty(name, value);
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
		CognitiveLog::Error("UCognitiveVRBlueprints::FlushEvents could not get provider!");
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
		CognitiveLog::Error("UCognitiveVRBlueprints::StartSession could not get provider!");
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
		CognitiveLog::Error("UCognitiveVRBlueprints::EndSession could not get provider!");
		return;
	}
	cog->EndSession();
}

void UCognitiveVRBlueprints::SetParticipantFullName(const FString Name)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetParticipantFullName could not get provider!");
		return;
	}
	cog->SetParticipantFullName(Name);
}
void UCognitiveVRBlueprints::SetParticipantId(const FString Id)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid())
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::SetParticipantId could not get provider!");
		return;
	}
	cog->SetParticipantId(Id);
}

UCustomEvent* UCognitiveVRBlueprints::MakeCustomEvent(FString eventName)
{
	UPlayerTracker* pt = UPlayerTracker::GetPlayerTracker();
	if (pt != NULL)
	{
		UCustomEvent* ce = NewObject<UCustomEvent>(pt);
		ce->SetCategory(eventName);
		return ce;
	}
	return NULL;
}

UCustomEvent* UCognitiveVRBlueprints::SetIntegerProperty(UCustomEvent* target, FString key, int32 intValue)
{
	target->SetProperty(key, intValue);
	return target;
}
UCustomEvent* UCognitiveVRBlueprints::SetStringProperty(UCustomEvent* target, FString key, FString stringValue)
{
	target->SetProperty(key, stringValue);
	return target;
}
UCustomEvent* UCognitiveVRBlueprints::SetBoolProperty(UCustomEvent* target, FString key, bool boolValue)
{
	target->SetProperty(key, boolValue);
	return target;
}
UCustomEvent* UCognitiveVRBlueprints::SetFloatProperty(UCustomEvent* target, FString key, float floatValue)
{
	target->SetProperty(key, floatValue);
	return target;
}
UCustomEvent* UCognitiveVRBlueprints::SetDynamicObject(UCustomEvent* target, UDynamicObject* dynamicObject)
{
	if (dynamicObject == NULL)
	{
		return target;
	}
	target->SetDynamicObject(dynamicObject);
	return target;
}
UCustomEvent* UCognitiveVRBlueprints::SetPosition(UCustomEvent* target, FVector position)
{
	target->SetPosition(position);
	return target;
}


void UCognitiveVRBlueprints::Send(UCustomEvent* target)
{
	target->Send();
}

UCustomEvent* UCognitiveVRBlueprints::AppendAllSensors(UCustomEvent* target)
{
	target->AppendAllSensors();
	return target;
}

UCustomEvent* UCognitiveVRBlueprints::AppendSensors(UCustomEvent* target, TArray<FString> keys)
{
	target->AppendSensors(keys);
	return target;
}

UCustomEvent* UCognitiveVRBlueprints::AppendSensor(UCustomEvent* target, FString key)
{
	target->AppendSensor(key);
	return target;
}



//debug stuff
FString UCognitiveVRBlueprints::GetSessionName()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return ""; }
	return cog->GetSessionID();
}

float UCognitiveVRBlueprints::GetSessionDuration()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!cog->HasStartedSession()) { return 0; }

	float dur = Util::GetTimestamp() - cog->GetSessionTimestamp();
	return dur;
}

FString UCognitiveVRBlueprints::GetSceneName()
{
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
}

FString UCognitiveVRBlueprints::GetSceneId()
{
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
}


float UCognitiveVRBlueprints::GetLastEventSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }
	return cog->customEventRecorder->GetLastSendTime();
}

float UCognitiveVRBlueprints::GetLastGazeSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }

	if (UPlayerTracker::GetPlayerTracker() == nullptr)
	{
		return 0;
	}

	return UPlayerTracker::GetPlayerTracker()->GetLastSendTime();
}

float UCognitiveVRBlueprints::GetLastDynamicSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }
	return UDynamicObject::GetLastSendTime();
}

float UCognitiveVRBlueprints::GetLastSensorSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }
	return cog->sensors->GetLastSendTime();
}

float UCognitiveVRBlueprints::GetLastFixationSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }

	if (UFixationRecorder::GetFixationRecorder() == NULL)
	{
		return -1;
	}
	return UFixationRecorder::GetFixationRecorder()->GetLastSendTime();
}


TArray<FString> UCognitiveVRBlueprints::GetDebugQuestionSet()
{
	auto qs = ExitPoll::GetCurrentQuestionSet();
	TArray<FString> questions;

	for (auto& Elem : qs.questions)
	{
		questions.Add(Elem.title);
	}
	return questions;

}

TArray<FString> UCognitiveVRBlueprints::GetSensorKeys()
{
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
}

TArray<FString> UCognitiveVRBlueprints::GetSensorValues()
{
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
}

bool UCognitiveVRBlueprints::IsFixating()
{
	auto fixptr = UFixationRecorder::GetFixationRecorder();
	if (fixptr == NULL) { return false; }
	return fixptr->IsFixating();
}
			 
int32 UCognitiveVRBlueprints::GetEventPartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->customEventRecorder->GetPartNumber();
}
			 
int32 UCognitiveVRBlueprints::GetGazePartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }

	auto gazeptr = UPlayerTracker::GetPlayerTracker();
	if (gazeptr == NULL) { return 0; }
	return gazeptr->GetPartNumber();
}
			 
int32 UCognitiveVRBlueprints::GetDynamicPartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return UDynamicObject::GetPartNumber();
}

int32 UCognitiveVRBlueprints::GetSensorPartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->sensors->GetPartNumber();
}

int32 UCognitiveVRBlueprints::GetFixationPartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	
	auto fixptr = UFixationRecorder::GetFixationRecorder();
	if (fixptr == NULL) { return 0; }
	return fixptr->GetPartNumber();
}


int32 UCognitiveVRBlueprints::GetEventDataPointCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->customEventRecorder->GetDataPoints();
}

int32 UCognitiveVRBlueprints::GetGazePointCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	auto gazeptr = UPlayerTracker::GetPlayerTracker();
	if (gazeptr == NULL) { return 0; }
	return gazeptr->GetDataPoints();
}

int32 UCognitiveVRBlueprints::GetDynamicDataCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }

	return UDynamicObject::GetDataPoints();
}

int32 UCognitiveVRBlueprints::GetDynamicObjectCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	
	return UDynamicObject::GetDynamicObjectCount();
}

int32 UCognitiveVRBlueprints::GetFixationPointCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	
	if (UFixationRecorder::GetFixationRecorder() == nullptr)
	{
		return 0;
	}
	return UFixationRecorder::GetFixationRecorder()->GetDataPoints();
}

int32 UCognitiveVRBlueprints::GetSensorDataPointCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->sensors->GetDataPoints();
}

FString UCognitiveVRBlueprints::GetAttributionParameters()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cog.IsValid()) { return FString(""); }
	return cog->GetAttributionParameters();
}