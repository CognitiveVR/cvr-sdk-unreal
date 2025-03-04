// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/TuningVariables.h"
#include "C3DComponents/OculusPlatform.h"
#include "Cognitive3DActor.h"
#include "Components/ActorComponent.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"

// Sets default values for this component's properties
UTuningVariables::UTuningVariables()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTuningVariables::BeginPlay()
{
	Super::BeginPlay();

	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cog.IsValid())
	{
		cog->OnSessionBegin.AddDynamic(this, &UTuningVariables::OnSessionBegin);
		cog->OnPreSessionEnd.AddDynamic(this, &UTuningVariables::OnSessionEnd);
		if (cog->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
	
}


// Called every frame
void UTuningVariables::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTuningVariables::OnSessionBegin()
{
	// Get the owner and cast it to ACognitive3DActor
	ACognitive3DActor* CognitiveActor = Cast<ACognitive3DActor>(GetOwner());

#ifdef INCLUDE_OCULUS_PLATFORM
	// Get the OculusPlatform component from the owner
	UOculusPlatform* OculusPlatformComp = CognitiveActor->FindComponentByClass<UOculusPlatform>();
	OculusPlatformComp->OnOculusNameHandled.AddDynamic(this, &UTuningVariables::QueryTuningVariable);
#else
	QueryTuningVariable(cog->GetUserID());
#endif

}

void UTuningVariables::OnSessionEnd()
{
#ifdef INCLUDE_OCULUS_PLATFORM
	ACognitive3DActor* CognitiveActor = Cast<ACognitive3DActor>(GetOwner());
	UOculusPlatform* OculusPlatformComp = CognitiveActor->FindComponentByClass<UOculusPlatform>();
	OculusPlatformComp->OnOculusNameHandled.RemoveDynamic(this, &UTuningVariables::QueryTuningVariable);
#endif
}

void UTuningVariables::QueryTuningVariable(FString UserID)
{
	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString Gateway = FAnalytics::Get().GetConfigValueFromIni(EngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
	FString Url = "https://" + Gateway + "/v" + FString::FromInt(0) + "/remotevariables?identifier=" + UserID;

	if (cog->ApplicationKey.IsEmpty())
	{
		cog->ApplicationKey = FAnalytics::Get().GetConfigValueFromIni(EngineIni, "Analytics", "ApiKey", false);
	}
	FString AuthValue = "APIKEY:DATA " + cog->ApplicationKey;
	// Create HTTP Request
	auto HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetHeader(TEXT("Authorization"), AuthValue);

	// Handle the response
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UTuningVariables::OnHttpResponseReceived);
	HttpRequest->ProcessRequest();
}

void UTuningVariables::OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		// Retrieve the response as a string
		FString ResponseString = Response->GetContentAsString();
		UE_LOG(LogTemp, Log, TEXT("TUNING VARIABLE Responsecode %d: %s"), Response->GetResponseCode(), *ResponseString);
		// You can now parse the JSON or process the data as needed.
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
		properties->SetStringField("TuningVariable", ResponseString);
		cog->customEventRecorder->Send("TuningVariable", properties);
		ParseJsonResponse(ResponseString);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TUNING VARIABLE HTTP Request failed"));
		// Handle the error accordingly.
		UE_LOG(LogTemp, Error, TEXT("TUNING VARIABLE Error Code: %d"), Response->GetResponseCode());
		UE_LOG(LogTemp, Error, TEXT("TUNING VARIABLE Error Message: %s"), *Response->GetContentAsString());
		cog->customEventRecorder->Send("TuningVariable", "Error");
	}
}

void UTuningVariables::ParseJsonResponse(const FString& JsonResponse)
{
	// Create a JSON reader from the response string.
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonResponse);

	// Deserialize the JSON string into a JSON object.
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// --- Parse the "abTests" array ---
		const TArray<TSharedPtr<FJsonValue>>* AbTestsArray;
		if (JsonObject->TryGetArrayField(TEXT("abTests"), AbTestsArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *AbTestsArray)
			{
				TSharedPtr<FJsonObject> TestObj = Value->AsObject();
				if (TestObj.IsValid())
				{
					FString Name = TestObj->GetStringField(TEXT("name"));
					FString Description = TestObj->GetStringField(TEXT("description"));
					FString RemoteVariableName = TestObj->GetStringField(TEXT("remoteVariableName"));
					FString Type = TestObj->GetStringField(TEXT("type"));
					int32 ValueInt = TestObj->GetIntegerField(TEXT("valueInt"));

					// Log or use the extracted values.
					UE_LOG(LogTemp, Log, TEXT("AB Test - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %d"),
						*Name, *Description, *RemoteVariableName, *Type, ValueInt);
					TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
					properties->SetStringField("Name", Name);
					properties->SetStringField("Description", Description);
					properties->SetStringField("RemoteVariableName", RemoteVariableName);
					properties->SetStringField("Type", Type);
					properties->SetNumberField("ValueInt", ValueInt);
					cog->customEventRecorder->Send("AB Test", properties);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No 'abTests' array found in JSON."));
		}

		// --- Parse the "remoteConfigurations" array ---
		const TArray<TSharedPtr<FJsonValue>>* RemoteConfigsArray;
		if (JsonObject->TryGetArrayField(TEXT("remoteConfigurations"), RemoteConfigsArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *RemoteConfigsArray)
			{
				TSharedPtr<FJsonObject> ConfigObj = Value->AsObject();
				if (ConfigObj.IsValid())
				{
					FString Name = ConfigObj->GetStringField(TEXT("name"));
					FString Description = ConfigObj->GetStringField(TEXT("description"));
					FString RemoteVariableName = ConfigObj->GetStringField(TEXT("remoteVariableName"));
					FString Type = ConfigObj->GetStringField(TEXT("type"));
					int32 ValueInt = ConfigObj->GetIntegerField(TEXT("valueInt"));

					// Log or use the extracted values.
					UE_LOG(LogTemp, Log, TEXT("Remote Config - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %d"),
						*Name, *Description, *RemoteVariableName, *Type, ValueInt);
					TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
					properties->SetStringField("Name", Name);
					properties->SetStringField("Description", Description);
					properties->SetStringField("RemoteVariableName", RemoteVariableName);
					properties->SetStringField("Type", Type);
					properties->SetNumberField("ValueInt", ValueInt);
					cog->customEventRecorder->Send("Remote Config", properties);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No 'remoteConfigurations' array found in JSON."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response."));
	}
}