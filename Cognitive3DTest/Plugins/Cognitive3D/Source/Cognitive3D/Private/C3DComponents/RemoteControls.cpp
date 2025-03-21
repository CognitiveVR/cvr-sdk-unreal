// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/RemoteControls.h"
#include "C3DComponents/OculusPlatform.h"
#include "Cognitive3DActor.h"
#include "Components/ActorComponent.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"
#include "TimerManager.h"

// Sets default values for this component's properties
URemoteControls::URemoteControls()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URemoteControls::BeginPlay()
{
	Super::BeginPlay();

	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cog.IsValid())
	{
		cog->remoteControls = this;
		cog->OnSessionBegin.AddDynamic(this, &URemoteControls::OnSessionBegin);
		cog->OnPreSessionEnd.AddDynamic(this, &URemoteControls::OnSessionEnd);
		if (cog->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
	
}


// Called every frame
void URemoteControls::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void URemoteControls::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }

	if (bFetchVariablesAutomatically)
	{
		if (bUseParticipantID)
		{
			if (!cog->GetUserID().IsEmpty())
			{
				//UE_LOG(LogTemp, Log, TEXT("REMOTE CONTROL VARIABLE Fetching: FOUND USER ID"));
				QueryRemoteControlVariable(cog->GetUserID());
			}
			else
			{
				//UE_LOG(LogTemp, Log, TEXT("REMOTE CONTROL VARIABLE Fetching: WAITING FOR USER ID"));

				cog->OnParticipantIdSet.AddDynamic(this, &URemoteControls::QueryRemoteControlVariable);

				world->GetTimerManager().SetTimer(TimerHandle, this, &URemoteControls::CallTimerEndFunction, WaitForParticipantIdTimeout, false);
			}
		}
		else
		{
			//UE_LOG(LogTemp, Log, TEXT("REMOTE CONTROL VARIABLE Fetching: DEVICE ID"));
			QueryRemoteControlVariable(cog->GetDeviceID());
		}
	}

	
}

void URemoteControls::OnSessionEnd()
{
	bHasRemoteControlVariables = false;
	if (cog->OnParticipantIdSet.IsBound())
	{
		cog->OnParticipantIdSet.RemoveDynamic(this, &URemoteControls::QueryRemoteControlVariable);
	}
}

void URemoteControls::QueryRemoteControlVariable(FString ParticipantId)
{
	if (bHasRemoteControlVariables)
	{
		UE_LOG(LogTemp, Log, TEXT("REMOTE CONTROL VARIABLE Already received"));
		return;
	}
	//UE_LOG(LogTemp, Log, TEXT("REMOTE CONTROL VARIABLE Fetching: QUERY REMOTE CONTROL VARIABLE: %s"), *ParticipantId);

	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString Gateway = FAnalytics::Get().GetConfigValueFromIni(EngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
	FString Url = "https://" + Gateway + "/v" + FString::FromInt(0) + "/remotevariables?identifier=" + ParticipantId;

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
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &URemoteControls::OnHttpResponseReceived);
	HttpRequest->ProcessRequest();
}

void URemoteControls::OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bHasRemoteControlVariables)
	{
		UE_LOG(LogTemp, Log, TEXT("REMOTE CONTROL VARIABLE Already received"));
		return;
	}

	if (bWasSuccessful && Response.IsValid())
	{
		// Retrieve the response as a string
		FString ResponseString = Response->GetContentAsString();
		//UE_LOG(LogTemp, Log, TEXT("REMOTE CONTROL VARIABLE Responsecode %d: %s"), Response->GetResponseCode(), *ResponseString);
		// You can now parse the JSON or process the data as needed.
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
		properties->SetStringField("RemoteControlVariable", ResponseString);
		//cog->customEventRecorder->Send("RemoteControlVariable", properties);
		ParseJsonResponse(ResponseString);
		//cache response string
		CacheRemoteControlVariables(ResponseString);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("REMOTE CONTROL VARIABLE HTTP Request failed"));
		//cog->customEventRecorder->Send("RemoteControlVariable", "Error");
		//attempt to read response from cache if available
		ReadFromCache();
	}
}

void URemoteControls::ParseJsonResponse(const FString& JsonResponse)
{
	//Reset variable arrays
	RemoteControlVariablesBool.Empty();
	RemoteControlVariablesFloat.Empty();
	RemoteControlVariablesInt.Empty();
	RemoteControlVariablesString.Empty();

	bool bSuccess = false;
	
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
					bSuccess = true;

					FString Name = TestObj->GetStringField(TEXT("name"));
					FString Description = TestObj->GetStringField(TEXT("description"));
					FString RemoteVariableName = TestObj->GetStringField(TEXT("remoteVariableName"));
					FString Type = TestObj->GetStringField(TEXT("type"));

					TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
					if (Type == "int")
					{
						int32 ValueInt = TestObj->GetIntegerField(TEXT("valueInt"));
						properties->SetNumberField("ValueInt", ValueInt);
						FString PropertyName = "c3d.remote_variable." + RemoteVariableName;
						cog->SetSessionProperty(PropertyName, ValueInt);
						RemoteControlVariablesInt.Add(RemoteVariableName, ValueInt);
						//UE_LOG(LogTemp, Log, TEXT("AB Test - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %d"),
						//	*Name, *Description, *RemoteVariableName, *Type, ValueInt);
					}
					else if (Type == "float")
					{
						float ValueFloat = TestObj->GetNumberField(TEXT("valueFloat"));
						properties->SetNumberField("ValueFloat", ValueFloat);
						FString PropertyName = "c3d.remote_variable." + RemoteVariableName;
						cog->SetSessionProperty(PropertyName, ValueFloat);
						RemoteControlVariablesFloat.Add(RemoteVariableName, ValueFloat);
						//UE_LOG(LogTemp, Log, TEXT("AB Test - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %f"),
						//	*Name, *Description, *RemoteVariableName, *Type, ValueFloat);
					}
					else if (Type == "string")
					{
						FString ValueString = TestObj->GetStringField(TEXT("valueString"));
						properties->SetStringField("ValueString", ValueString);
						FString PropertyName = "c3d.remote_variable." + RemoteVariableName;
						cog->SetSessionProperty(PropertyName, ValueString);
						RemoteControlVariablesString.Add(RemoteVariableName, ValueString);
						//UE_LOG(LogTemp, Log, TEXT("AB Test - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %s"),
						//	*Name, *Description, *RemoteVariableName, *Type, *ValueString);
					}
					else if (Type == "bool")
					{
						bool ValueBool = TestObj->GetBoolField(TEXT("valueBool"));
						properties->SetBoolField("ValueBool", ValueBool);
						FString PropertyName = "c3d.remote_variable." + RemoteVariableName;
						cog->SetSessionProperty(PropertyName, ValueBool);
						RemoteControlVariablesBool.Add(RemoteVariableName, ValueBool);
						//UE_LOG(LogTemp, Log, TEXT("AB Test - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %d"),
						//	*Name, *Description, *RemoteVariableName, *Type, ValueBool);
					}

					properties->SetStringField("Name", Name);
					properties->SetStringField("Description", Description);
					properties->SetStringField("RemoteVariableName", RemoteVariableName);
					properties->SetStringField("Type", Type);
					//cog->customEventRecorder->Send("AB Test", properties);
				}
			}
		}
		else
		{
			bSuccess = false;
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
					bSuccess = true;

					FString Name = ConfigObj->GetStringField(TEXT("name"));
					FString Description = ConfigObj->GetStringField(TEXT("description"));
					FString RemoteVariableName = ConfigObj->GetStringField(TEXT("remoteVariableName"));
					FString Type = ConfigObj->GetStringField(TEXT("type"));
					
					TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
					if (Type == "int")
					{
						int32 ValueInt = ConfigObj->GetIntegerField(TEXT("valueInt"));
						if (!RemoteControlVariablesInt.Contains(RemoteVariableName))
						{
							properties->SetNumberField("ValueInt", ValueInt);
							FString PropertyName = "c3d.remote_variable." + RemoteVariableName;
							cog->SetSessionProperty(PropertyName, ValueInt);
							RemoteControlVariablesInt.Add(RemoteVariableName, ValueInt);
						}

						//UE_LOG(LogTemp, Log, TEXT("AB Test - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %d"),
						//	*Name, *Description, *RemoteVariableName, *Type, ValueInt);
					}
					else if (Type == "float")
					{
						float ValueFloat = ConfigObj->GetNumberField(TEXT("valueFloat"));
						if (!RemoteControlVariablesFloat.Contains(RemoteVariableName))
						{
							properties->SetNumberField("ValueFloat", ValueFloat);
							FString PropertyName = "c3d.remote_variable." + RemoteVariableName;
							cog->SetSessionProperty(PropertyName, ValueFloat);
							RemoteControlVariablesFloat.Add(RemoteVariableName, ValueFloat);
						}
						//UE_LOG(LogTemp, Log, TEXT("AB Test - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %f"),
						//	*Name, *Description, *RemoteVariableName, *Type, ValueFloat);
					}
					else if (Type == "string")
					{
						FString ValueString = ConfigObj->GetStringField(TEXT("valueString"));
						if (!RemoteControlVariablesString.Contains(RemoteVariableName))
						{
							properties->SetStringField("ValueString", ValueString);
							FString PropertyName = "c3d.remote_variable." + RemoteVariableName;
							cog->SetSessionProperty(PropertyName, ValueString);
							RemoteControlVariablesString.Add(RemoteVariableName, ValueString);
						}
						//UE_LOG(LogTemp, Log, TEXT("AB Test - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %s"),
						//	*Name, *Description, *RemoteVariableName, *Type, *ValueString);
					}
					else if (Type == "bool")
					{
						bool ValueBool = ConfigObj->GetBoolField(TEXT("valueBool"));
						if (!RemoteControlVariablesBool.Contains(RemoteVariableName))
						{
							properties->SetBoolField("ValueBool", ValueBool);
							FString PropertyName = "c3d.remote_variable." + RemoteVariableName;
							cog->SetSessionProperty(PropertyName, ValueBool);
							RemoteControlVariablesBool.Add(RemoteVariableName, ValueBool);
						}
						//UE_LOG(LogTemp, Log, TEXT("AB Test - Name: %s, Description: %s, RemoteVariableName: %s, Type: %s, ValueInt: %d"),
						//	*Name, *Description, *RemoteVariableName, *Type, ValueBool);
					}
					properties->SetStringField("Name", Name);
					properties->SetStringField("Description", Description);
					properties->SetStringField("RemoteVariableName", RemoteVariableName);
					properties->SetStringField("Type", Type);
					//cog->customEventRecorder->Send("Remote Config", properties);
				}
			}
		}
		else
		{
			bSuccess = false;
			UE_LOG(LogTemp, Warning, TEXT("No 'remoteConfigurations' array found in JSON."));
		}
	}
	else
	{
		bSuccess = false;
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response."));
	}

	if (bSuccess)
	{
		bHasRemoteControlVariables = true;
		OnRemoteControlVariableReceived.Broadcast();
	}
}

int32 URemoteControls::GetRemoteControlVariableInt(const FString& Key, int32 DefaultValue)
{
	if (RemoteControlVariablesInt.Contains(Key))
	{
		return RemoteControlVariablesInt[Key];
	}
	else
	{
		return DefaultValue;
	}
}

float URemoteControls::GetRemoteControlVariableFloat(const FString& Key, float DefaultValue)
{
	if (RemoteControlVariablesFloat.Contains(Key))
	{
		return RemoteControlVariablesFloat[Key];
	}
	else
	{
		return DefaultValue;
	}
}

FString URemoteControls::GetRemoteControlVariableString(const FString& Key, const FString& DefaultValue)
{
	if (RemoteControlVariablesString.Contains(Key))
	{
		return RemoteControlVariablesString[Key];
	}
	else
	{
		return DefaultValue;
	}
}

bool URemoteControls::GetRemoteControlVariableBool(const FString& Key, bool DefaultValue)
{
	if (RemoteControlVariablesBool.Contains(Key))
	{
		return RemoteControlVariablesBool[Key];
	}
	else
	{
		return DefaultValue;
	}
}

void URemoteControls::CacheRemoteControlVariables(const FString& JsonResponse)
{
	//make folder remotecontrols
	//store it in a RemoteControls file inside that with just the response
	if (!cog.IsValid()) { return; }
	if (!cog->HasStartedSession()) { return; }

	// Get the project's Config directory.
	FString BaseConfigDir = FPaths::ProjectConfigDir();

	// Define the subfolder and ensure it exists.
	FString CustomFolder = FPaths::Combine(BaseConfigDir, TEXT("c3dlocal/remotecontrols"));
	if (!FPaths::DirectoryExists(CustomFolder))
	{
		// Create the directory if it doesn't exist.
		IFileManager::Get().MakeDirectory(*CustomFolder);
	}

	// Combine the subfolder path with your INI file name.
	FString RemoteControlsFilePath = FPaths::Combine(CustomFolder, TEXT("RemoteControls"));

	// If the file doesn't exist, create it with some default content.
	if (!FPaths::FileExists(RemoteControlsFilePath))
	{
		if (!FFileHelper::SaveStringToFile(JsonResponse, *RemoteControlsFilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create config file: %s"), *RemoteControlsFilePath);
		}
	}
}

void URemoteControls::ReadFromCache()
{
	if (!cog.IsValid()) { return; }
	if (!cog->HasStartedSession()) { return; }

	// Get the project's Config directory.
	FString BaseConfigDir = FPaths::ProjectConfigDir();

	// Define the subfolder and ensure it exists.
	FString CustomFolder = FPaths::Combine(BaseConfigDir, TEXT("c3dlocal/remotecontrols"));
	if (!FPaths::DirectoryExists(CustomFolder))
	{
		// Create the directory if it doesn't exist.
		IFileManager::Get().MakeDirectory(*CustomFolder);
	}

	// Combine the subfolder path with your INI file name.
	FString RemoteControlsFilePath = FPaths::Combine(CustomFolder, TEXT("RemoteControls"));

	// If the file doesn't exist, create it with some default content.
	if (!FPaths::FileExists(RemoteControlsFilePath))
	{
		FString JsonResponse;
		if (!FFileHelper::LoadFileToString(JsonResponse, *RemoteControlsFilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to read config file: %s"), *RemoteControlsFilePath);
		}
		else
		{
			// Parse the content
			ParseJsonResponse(JsonResponse);
		}
	}
}

void URemoteControls::CallTimerEndFunction()
{
	UE_LOG(LogTemp, Log, TEXT("REMOTE CONTROL VARIABLE Fetching: TIMER END"));
	QueryRemoteControlVariable(cog->GetDeviceID());
}
