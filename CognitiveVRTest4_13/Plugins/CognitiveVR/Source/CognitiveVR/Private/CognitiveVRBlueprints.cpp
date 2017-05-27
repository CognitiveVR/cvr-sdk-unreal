/*
	
*/
#include "CognitiveVR.h"
#include "Private/CognitiveVRPrivatePCH.h"

#include "CognitiveVRBlueprints.h"

using namespace cognitivevrapi;

extern bool bHasSessionStarted;


void UCognitiveVRBlueprints::BeginTransaction(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	//HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	//HMDPosition = controllers[0]->GetPawn()->FindComponentByClass<UCameraComponent>()->ComponentToWorld.GetTranslation();
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();

	UCognitiveVRBlueprints::BeginTransactionPosition(Category, TransactionID, Attributes, HMDPosition);
}

void UCognitiveVRBlueprints::BeginTransactionPosition(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginTransaction could not get provider!");
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

	cog->transaction->BeginPosition(TCHAR_TO_UTF8(*Category), Position, properties, TCHAR_TO_UTF8(*TransactionID));
}

void UCognitiveVRBlueprints::UpdateTransaction(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, float Progress)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	//HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	//HMDPosition = controllers[0]->GetPawn()->FindComponentByClass<UCameraComponent>()->ComponentToWorld.GetTranslation();
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();
	UCognitiveVRBlueprints::UpdateTransactionPosition(Category, TransactionID, Attributes, HMDPosition, Progress);
}

void UCognitiveVRBlueprints::UpdateTransactionPosition(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position, float Progress)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateTransaction could not get provider!");
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

	cog->transaction->UpdatePosition(TCHAR_TO_UTF8(*Category), Position, properties, TCHAR_TO_UTF8(*TransactionID), Progress);
}

void UCognitiveVRBlueprints::EndTransaction(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	//HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	//HMDPosition = controllers[0]->GetPawn()->FindComponentByClass<UCameraComponent>()->ComponentToWorld.GetTranslation();
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();
	UCognitiveVRBlueprints::EndTransactionPosition(Category, TransactionID, Attributes, HMDPosition);
}

void UCognitiveVRBlueprints::EndTransactionPosition(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::EndTransaction could not get provider!");
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

	cog->transaction->EndPosition(TCHAR_TO_UTF8(*Category), Position, properties, TCHAR_TO_UTF8(*TransactionID));
}

void UCognitiveVRBlueprints::BeginEndTransaction(FString Category, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	//HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	//HMDPosition = controllers[0]->GetPawn()->FindComponentByClass<UCameraComponent>()->ComponentToWorld.GetTranslation();
	HMDPosition = controllers[0]->PlayerCameraManager->GetCameraLocation();
	UCognitiveVRBlueprints::BeginEndTransactionPosition(Category, Attributes, HMDPosition);
}

void UCognitiveVRBlueprints::BeginEndTransactionPosition(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
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
	
	cog->transaction->BeginEndPosition(TCHAR_TO_UTF8(*Category),Position,properties);
}

void UCognitiveVRBlueprints::UpdateCollection(FString Name, float Balance, float Change, bool IsCurrency)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateCollection could not get provider!");
		return;
	}

	cog->core_utils->UpdateCollection(TCHAR_TO_UTF8(*Name), Balance, Change, IsCurrency);
}

FString UCognitiveVRBlueprints::GetTuningValue(FString Key, ETuningValueReturn& Branches)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::GetTuningValue could not get provider!");
		return FString();
	}
	
	CognitiveVRResponse response = cog->tuning->GetValue(TCHAR_TO_UTF8(*Key),"default");

	if (!response.IsSuccessful())
	{
		CognitiveLog::Warning("FAnalyticsCognitiveVR::GetTuningValue response failed!");
		Branches = ETuningValueReturn::Failed;
		return FString();
	}

	FJsonObject json = response.GetContent();

	FString outString;
	if (json.TryGetStringField(Key, outString))
	{
		Branches = ETuningValueReturn::Success;
		return outString;
	}

	Branches = ETuningValueReturn::Failed;
	outString = FString();
	return outString;
}

void UCognitiveVRBlueprints::UpdateDevice(const TArray<FAnalyticsEventAttr>& Attributes)
{
	//SOME TRANSACTION OR SOMETHING
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
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

	
	//UPDATE DEVICE STATE
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	std::string ts = Util::GetTimestampStr();
	FString fs(ts.c_str());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, empty); //user
	Util::AppendToJsonArray(jsonArray, cog->DeviceId); //device
	Util::AppendToJsonArray(jsonArray, properties);

	//s->thread_manager->PushTask(NULL, "datacollector_updateDeviceState", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateDeviceState");
	jsonObject.Get()->SetField("args", jsonArray);

	cog->thread_manager->AddJsonToBatch(jsonObject);



	//cog->transaction->BeginEndPosition(TCHAR_TO_UTF8(*Category), Position, properties);
}

void UCognitiveVRBlueprints::UpdateUser(const TArray<FAnalyticsEventAttr>& Attributes)
{
	//SOME TRANSACTION OR SOMETHING
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
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


	//UPDATE DEVICE STATE
	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	std::string ts = Util::GetTimestampStr();
	FString fs(ts.c_str());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, cog->UserId); //user
	Util::AppendToJsonArray(jsonArray, empty); //device
	Util::AppendToJsonArray(jsonArray, properties);

	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateUserState");
	jsonObject.Get()->SetField("args", jsonArray);

	cog->thread_manager->AddJsonToBatch(jsonObject);
}

void UCognitiveVRBlueprints::RecordSensor(const FString Name, const float Value)
{
	//SOME TRANSACTION OR SOMETHING
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
		return;
	}

	cog->sensors->RecordSensor(Name, Value);
}

/*void UCognitiveVRBlueprints::GetRequestLatent(const FString Hook, struct FLatentActionInfo LatentInfo)
{
	float Duration = 5;

	//http request
	//add callback from http request

	FLatentActionManager& LatentActionManager = GWorld->GetLatentActionManager();
	if (LatentActionManager.FindExistingAction<FExitPollLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == NULL)
	{
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FExitPollLatentAction(Duration, LatentInfo));
	}

	ExitPoll::MakeQuestionSetRequest(Hook); //callback goes here?
}

void HttpExitPollRequestCallback(FExitPollQuestionSet questionSet, struct FLatentActionInfo LatentInfo)
{
	//save question set to exitpoll

	//find latent action manager. remove exitpoll latent action
	FLatentActionManager& LatentActionManager = GWorld->GetLatentActionManager();
	if (LatentActionManager.FindExistingAction<FExitPollLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) != NULL)
	{
		LatentActionManager.RemoveActionsForObject(LatentInfo.CallbackTarget);
	}
}

*/

void UCognitiveVRBlueprints::GetRequestDelegate(const FString Hook, const FCognitiveExitPollResponse response)
{
	ExitPoll::MakeQuestionSetRequest(Hook, response);
}

FExitPollQuestionSet UCognitiveVRBlueprints::GetCurrentExitPollQuestionSet(EResponseValueReturn& Out)
{
	return ExitPoll::GetCurrentQuestionSet();
}

void UCognitiveVRBlueprints::SendExitPollResponse(FExitPollResponses Responses)
{
	/*TMap<FString, FString> answers;

	for (int32 i = 0; i < Responses.answers.Num(); i++)
	{
		answers.Add(Responses.answers[i].type, Responses.answers[i].value);
	}
	ExitPoll::SendQuestionResponses(Responses.user,Responses.questionSetId,Responses.sessionId,Responses.hook,answers);*/
	ExitPoll::SendQuestionResponses(Responses);
}

