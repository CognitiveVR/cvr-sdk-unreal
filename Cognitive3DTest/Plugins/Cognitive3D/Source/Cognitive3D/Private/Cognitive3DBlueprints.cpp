/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/
#include "Cognitive3DBlueprints.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"
#include "Cognitive3D/Private/C3DComponents/RemoteControls.h"
//#include "Private/Cognitive3DPrivatePCH.h"

TSharedPtr<FAnalyticsProviderCognitive3D> UCognitive3DBlueprints::cog;

void UCognitive3DBlueprints::SendCustomEvent(FString Category, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();
	UCognitive3DBlueprints::SendCustomEventToCore(Category, Attributes, HMDPosition,NULL);
}

void UCognitive3DBlueprints::SendCustomEventDynamic(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, UDynamicObject* dynamic, FVector Position)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();

	UCognitive3DBlueprints::SendCustomEventToCore(Category, Attributes, HMDPosition, dynamic);
}

void UCognitive3DBlueprints::SendCustomEventToCore(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position, UDynamicObject* dynamic)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SendCustomEventToCore could not get provider!");
		return;
	}
	if (!HasSessionStarted()) { return; }

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

void UCognitive3DBlueprints::SendCustomEventPosition(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	SendCustomEventToCore(Category, Attributes, Position, NULL);
}

void UCognitive3DBlueprints::UpdateSessionInt(const FString name, const int32 value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::UpdateSessionInt could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitive3DBlueprints::UpdateSessionFloat(const FString name, const float value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::UpdateSessionFloat could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitive3DBlueprints::UpdateSessionString(const FString name, const FString value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::UpdateSessionString could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitive3DBlueprints::UpdateSessionBool(const FString name, const bool value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::UpdateSessionBool could not get provider!");
		return;
	}
	cog->SetSessionProperty(name, value);
}

void UCognitive3DBlueprints::SetParticipantPropertyInt(const FString name, const int32 value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SetParticipantPropertyInt could not get provider!");
		return;
	}
	cog->SetParticipantProperty(name, value);
}

void UCognitive3DBlueprints::SetParticipantPropertyFloat(const FString name, const float value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SetParticipantPropertyFloat could not get provider!");
		return;
	}
	cog->SetParticipantProperty(name, value);
}

void UCognitive3DBlueprints::SetParticipantPropertyString(const FString name, const FString value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SetParticipantPropertyString could not get provider!");
		return;
	}
	cog->SetParticipantProperty(name, value);
}

void UCognitive3DBlueprints::SetParticipantPropertyBool(const FString name, const bool value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SetParticipantPropertyBool could not get provider!");
		return;
	}
	cog->SetParticipantProperty(name, value);
}

void UCognitive3DBlueprints::SetSessionName(const FString name)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SetSessionName could not get provider!");
		return;
	}
	cog->SetSessionName(name);
}

void UCognitive3DBlueprints::SetLobbyId(const FString lobbyId)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SetLobbyId could not get provider!");
		return;
	}
	cog->SetLobbyId(lobbyId);
}

void UCognitive3DBlueprints::InitializeSensor(const FString Name, const float HzRate, const float InitialValue)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::RecordSensor could not get provider!");
		return;
	}
	if (!HasSessionStarted()) { return; }
	cog->sensors->InitializeSensor(Name, HzRate, InitialValue);
}

void UCognitive3DBlueprints::RecordSensor(const FString Name, const float Value)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::RecordSensor could not get provider!");
		return;
	}
	if (!HasSessionStarted()) { return; }
	cog->sensors->RecordSensor(Name, Value);
}

void UCognitive3DBlueprints::FetchRemoteControlVariableNoParticipantId()
{
	if(!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return; }
	if (!cog->remoteControls->IsActive()) { return; }
	cog->remoteControls->FetchRemoteControlVariable();
}

void UCognitive3DBlueprints::FetchRemoteControlVariableWithParticipantId(const FString ParticipantId)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return; }
	if (!cog->remoteControls->IsActive()) { return; }
	cog->remoteControls->FetchRemoteControlVariable(ParticipantId);
}

FString UCognitive3DBlueprints::GetRemoteControlVariableString(const FString Key, const FString DefaultValue)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return FString(); }
	if (!cog->remoteControls->IsActive()) { return FString(); }
	return cog->remoteControls->GetRemoteControlVariableString(Key, DefaultValue);
}

int32 UCognitive3DBlueprints::GetRemoteControlVariableInt(const FString Key, const int32 DefaultValue)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!cog->remoteControls->IsActive()) { return 0; }
	return cog->remoteControls->GetRemoteControlVariableInt(Key, DefaultValue);
}

float UCognitive3DBlueprints::GetRemoteControlVariableFloat(const FString Key, const float DefaultValue)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0.0f; }
	if (!cog->remoteControls->IsActive()) { return 0.0f; }
	return cog->remoteControls->GetRemoteControlVariableFloat(Key, DefaultValue);
}

bool UCognitive3DBlueprints::GetRemoteControlVariableBool(const FString Key, const bool DefaultValue)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return false; }
	if (!cog->remoteControls->IsActive()) { return false; }
	return cog->remoteControls->GetRemoteControlVariableBool(Key, DefaultValue);
}

void UCognitive3DBlueprints::GetQuestionSet(const FString Hook, FCognitiveExitPollResponse response)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return; }
	if (!cog->exitpoll.IsValid()) { return; }
	if (!HasSessionStarted()) {return;}
	cog->exitpoll->MakeQuestionSetRequest(Hook, response);
}

bool UCognitive3DBlueprints::HasSessionStarted()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		return false;
	}

	return cog->HasStartedSession();
}

FExitPollQuestionSet UCognitive3DBlueprints::GetCurrentExitPollQuestionSet()
{
	return FExitPollQuestionSet();
}

void UCognitive3DBlueprints::SendExitPollAnswers(const TArray<FExitPollAnswer>& Answers)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return; }
	if (!cog->exitpoll.IsValid()) { return; }
	cog->exitpoll->SendQuestionAnswers(Answers);
}

void UCognitive3DBlueprints::FlushEvents()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::FlushEvents could not get provider!");
		return;
	}
	cog->FlushEvents();
}

bool UCognitive3DBlueprints::StartSession()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::StartSession could not get provider!");
		return false;
	}
	return cog->StartSession(TArray<FAnalyticsEventAttribute>());
}

void UCognitive3DBlueprints::EndSession()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::EndSession could not get provider!");
		return;
	}
	cog->EndSession();
}

void UCognitive3DBlueprints::SetParticipantFullName(const FString Name)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SetParticipantFullName could not get provider!");
		return;
	}
	cog->SetParticipantFullName(Name);
}
void UCognitive3DBlueprints::SetParticipantId(const FString Id)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SetParticipantId could not get provider!");
		return;
	}
	cog->SetParticipantId(Id);
}
void UCognitive3DBlueprints::SetSessionTag(const FString Tag)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid())
	{
		FCognitiveLog::Error("UCognitive3DBlueprints::SetParticipantId could not get provider!");
		return;
	}
	cog->SetSessionTag(Tag);
}

UCustomEvent* UCognitive3DBlueprints::MakeCustomEvent(FString eventName)
{
	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor != NULL)
	{
		UCustomEvent* ce = NewObject<UCustomEvent>(cognitiveActor);
		ce->SetCategory(eventName);
		return ce;
	}
	return NULL;
}

UCustomEvent* UCognitive3DBlueprints::SetIntegerProperty(UCustomEvent* target, FString key, int32 intValue)
{
	target->SetProperty(key, intValue);
	return target;
}
UCustomEvent* UCognitive3DBlueprints::SetStringProperty(UCustomEvent* target, FString key, FString stringValue)
{
	target->SetProperty(key, stringValue);
	return target;
}
UCustomEvent* UCognitive3DBlueprints::SetBoolProperty(UCustomEvent* target, FString key, bool boolValue)
{
	target->SetProperty(key, boolValue);
	return target;
}
UCustomEvent* UCognitive3DBlueprints::SetFloatProperty(UCustomEvent* target, FString key, float floatValue)
{
	target->SetProperty(key, floatValue);
	return target;
}
UCustomEvent* UCognitive3DBlueprints::SetDynamicObject(UCustomEvent* target, UDynamicObject* dynamicObject)
{
	if (dynamicObject == NULL)
	{
		return target;
	}
	target->SetDynamicObject(dynamicObject);
	return target;
}
UCustomEvent* UCognitive3DBlueprints::SetPosition(UCustomEvent* target, FVector position)
{
	target->SetPosition(position);
	return target;
}


void UCognitive3DBlueprints::Send(UCustomEvent* target)
{
	target->Send();
}
void UCognitive3DBlueprints::SendAtHMDPosition(UCustomEvent* target)
{
	target->SendAtHMDPosition();
}

UCustomEvent* UCognitive3DBlueprints::AppendAllSensors(UCustomEvent* target)
{
	target->AppendAllSensors();
	return target;
}

UCustomEvent* UCognitive3DBlueprints::AppendSensors(UCustomEvent* target, TArray<FString> keys)
{
	target->AppendSensors(keys);
	return target;
}

UCustomEvent* UCognitive3DBlueprints::AppendSensor(UCustomEvent* target, FString key)
{
	target->AppendSensor(key);
	return target;
}



//debug stuff
FString UCognitive3DBlueprints::GetSessionName()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return ""; }
	return cog->GetSessionID();
}

float UCognitive3DBlueprints::GetSessionDuration()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!cog->HasStartedSession()) { return 0; }

	float dur = FUtil::GetTimestamp() - cog->GetSessionTimestamp();
	return dur;
}

FString UCognitive3DBlueprints::GetSceneName()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return ""; }
	if (!cog->HasStartedSession()){ return ""; }

	auto data = cog->GetCurrentSceneData();

	if (data.IsValid())
	{
		return data->Name;
	}
	return "invalid";
}

FString UCognitive3DBlueprints::GetSceneId()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return ""; }
	if (!cog->HasStartedSession()) { return ""; }

	auto data = cog->GetCurrentSceneData();

	if (data.IsValid())
	{
		return data->Id;
	}
	return "invalid";
}


float UCognitive3DBlueprints::GetLastEventSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }
	return cog->customEventRecorder->GetLastSendTime();
}

float UCognitive3DBlueprints::GetLastGazeSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }

	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == NULL) { return 0; }
	auto gazeptr = Cast<UPlayerTracker>(cognitiveActor->GetComponentByClass(UPlayerTracker::StaticClass()));

	if (gazeptr == nullptr)
	{
		return 0;
	}

	return gazeptr->GetLastSendTime();
}

float UCognitive3DBlueprints::GetLastDynamicSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }

	TSharedPtr<FAnalyticsProviderCognitive3D> cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	auto dynamicObjectManager = cogProvider->dynamicObjectManager;
	if (dynamicObjectManager == nullptr) { return -1; }
	return dynamicObjectManager->GetLastSendTime();
}

float UCognitive3DBlueprints::GetLastSensorSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }
	return cog->sensors->GetLastSendTime();
}

float UCognitive3DBlueprints::GetLastFixationSendTime()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return -1; }
	if (!HasSessionStarted()) { return -1; }

	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == NULL) { return 0; }
	auto fixptr = Cast<UFixationRecorder>(cognitiveActor->GetComponentByClass(UFixationRecorder::StaticClass()));

	if (fixptr == NULL)
	{
		return -1;
	}
	return fixptr->GetLastSendTime();
}


TArray<FString> UCognitive3DBlueprints::GetDebugQuestionSet()
{
	TArray<FString> questions;

	if (!cog.IsValid()) { return questions; }
	if (!cog->exitpoll.IsValid()) { return questions; }
	auto qs = cog->exitpoll->GetCurrentQuestionSet();
	for (auto& Elem : qs.questions)
	{
		questions.Add(Elem.title);
	}
	return questions;
}

TArray<FString> UCognitive3DBlueprints::GetSensorKeys()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
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

TArray<FString> UCognitive3DBlueprints::GetSensorValues()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
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

bool UCognitive3DBlueprints::IsFixating()
{
	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == NULL) { return false; }
	auto fixptr = Cast<UFixationRecorder>(cognitiveActor->GetComponentByClass(UFixationRecorder::StaticClass()));
	if (fixptr == NULL) { return false; }
	return fixptr->IsFixating();
}
			 
int32 UCognitive3DBlueprints::GetEventPartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->customEventRecorder->GetPartNumber();
}
			 
int32 UCognitive3DBlueprints::GetGazePartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }

	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == NULL) { return 0; }
	auto gazeptr = Cast<UPlayerTracker>(cognitiveActor->GetComponentByClass(UPlayerTracker::StaticClass()));
	if (gazeptr == NULL) { return 0; }
	return gazeptr->GetPartNumber();
}
			 
int32 UCognitive3DBlueprints::GetDynamicPartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	TSharedPtr<FAnalyticsProviderCognitive3D> cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	auto dynamicObjectManager = cogProvider->dynamicObjectManager;
	if (dynamicObjectManager == nullptr) { return 0; }
	return dynamicObjectManager->GetPartNumber();
}

int32 UCognitive3DBlueprints::GetSensorPartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->sensors->GetPartNumber();
}

int32 UCognitive3DBlueprints::GetFixationPartNumber()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	
	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == NULL) { return 0; }
	auto fixptr = Cast<UFixationRecorder>(cognitiveActor->GetComponentByClass(UFixationRecorder::StaticClass()));
	if (fixptr == NULL) { return 0; }
	return fixptr->GetPartNumber();
}


int32 UCognitive3DBlueprints::GetEventDataPointCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->customEventRecorder->GetDataPoints();
}

int32 UCognitive3DBlueprints::GetGazePointCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }

	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == NULL) { return 0; }
	auto gazeptr = Cast<UPlayerTracker>(cognitiveActor->GetComponentByClass(UPlayerTracker::StaticClass()));
	if (gazeptr == NULL) { return 0; }
	return gazeptr->GetDataPoints();
}

int32 UCognitive3DBlueprints::GetDynamicDataCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }

	TSharedPtr<FAnalyticsProviderCognitive3D> cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	auto dynamicObjectManager = cogProvider->dynamicObjectManager;
	if (dynamicObjectManager == nullptr) { return 0; }
	return dynamicObjectManager->GetDataPoints();
}

int32 UCognitive3DBlueprints::GetDynamicObjectCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	
	TSharedPtr<FAnalyticsProviderCognitive3D> cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	auto dynamicObjectManager = cogProvider->dynamicObjectManager;
	if (dynamicObjectManager == nullptr) { return 0; }
	return dynamicObjectManager->GetDynamicObjectCount();
}

int32 UCognitive3DBlueprints::GetFixationPointCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	
	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == NULL) { return 0; }
	auto fixptr = Cast<UFixationRecorder>(cognitiveActor->GetComponentByClass(UFixationRecorder::StaticClass()));

	if (fixptr == nullptr)
	{
		return 0;
	}
	return fixptr->GetDataPoints();
}

int32 UCognitive3DBlueprints::GetSensorDataPointCount()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return 0; }
	if (!HasSessionStarted()) { return 0; }
	return cog->sensors->GetDataPoints();
}

bool UCognitive3DBlueprints::HasNetworkError()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return false; }
	if (!HasSessionStarted()) { return false; }
	return cog->network->HasErrorResponse();
}

FString UCognitive3DBlueprints::GetAttributionParameters()
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cog.IsValid()) { return FString(""); }
	return cog->GetAttributionParameters();
}

UFixationRecorder* UCognitive3DBlueprints::GetFixationRecorder()
{
	ACognitive3DActor* cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == nullptr) { return nullptr; }

	auto actorComponent = cognitiveActor->GetComponentByClass(UFixationRecorder::StaticClass());
	if (actorComponent == nullptr) { return nullptr; }
	return Cast<UFixationRecorder>(actorComponent);
}

UPlayerTracker* UCognitive3DBlueprints::GetPlayerTracker()
{
	ACognitive3DActor* cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == nullptr) { return nullptr; }

	auto actorComponent = cognitiveActor->GetComponentByClass(UPlayerTracker::StaticClass());
	if (actorComponent == nullptr) { return nullptr; }
	return Cast<UPlayerTracker>(actorComponent);
}

void UCognitive3DBlueprints::SetTrackingScene(FString SceneName)
{
	if (!cog.IsValid())
		cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	cog->SetTrackingScene(SceneName);
}