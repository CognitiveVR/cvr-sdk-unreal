// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/OculusPlatform.h"
#ifdef INCLUDE_OCULUS_PLATFORM
#include "OVRPlatform.h"
#include "OVR_Platform.h"
#include "OVRPlatformSubsystem.h"
#include "OVRPlatformSDK.cpp"
#endif
#ifdef __ANDROID__
#include "Android/AndroidApplication.h"
#endif
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Private/C3DApi/GazeDataRecorder.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"

// Sets default values for this component's properties
UOculusPlatform::UOculusPlatform()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UOculusPlatform::BeginPlay()
{
	Super::BeginPlay();

	// ...
	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString OculusAppId = FAnalytics::Get().GetConfigValueFromIni(EngineIni, "OnlineSubsystemOculus", "OculusAppId", false);
	if (OculusAppId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Oculus App ID is not set in DefaultEngine.ini under OnlineSubsystemOculus"));
		return;
	}
#ifdef INCLUDE_OCULUS_PLATFORM
#ifdef __ANDROID__
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jobject Activity = FAndroidApplication::GetGameActivityThis();

	
	auto tempAnsi = StringCast<ANSICHAR>(*OculusAppId);
	const char* AppID = tempAnsi.Get();

	if (Env != nullptr && Activity != nullptr)
	{
		ovrPlatformInitializeResult result = ovr_PlatformInitializeAndroid(AppID, Activity, Env);
		if (result != ovrPlatformInitialize_Success)
		{
			// Handle initialization failure
		}
		else
		{
			// Initialization successful
		}
	}
	else
	{
		// Failed to obtain JNI Environment or Activity
	}
#endif
#endif

	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cog.IsValid())
	{
		cog->OnSessionBegin.AddDynamic(this, &UOculusPlatform::OnSessionBegin);
		cog->OnPreSessionEnd.AddDynamic(this, &UOculusPlatform::OnSessionEnd);
		if (cog->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}


// Called every frame
void UOculusPlatform::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
#ifdef INCLUDE_OCULUS_PLATFORM
	ProcessOculusMessages();
#endif
}

void UOculusPlatform::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }

#ifdef INCLUDE_OCULUS_PLATFORM
	//initialize the subsystem and start the message pump
	//platform initialization for the subsystem is initialized in the game instance
	UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	UOvrPlatformSubsystem* ovrSubsystem = GameInstance->GetSubsystem<UOvrPlatformSubsystem>();

	ovrSubsystem->StartMessagePump();

	//request to get the entitlement status
	ovr_Entitlement_GetIsViewerEntitled();

	// get the logged in user's id
	// this is not a request, so it returns the data directly
	ovrID loggedInUserID = ovr_GetLoggedInUserID();

	//initiate a request to get the logged in users's information
	// this is a request, so the response is returned on the message queue
	ovr_User_Get(loggedInUserID);

	//request to get access token
	gotAccessToken = false;
	ovr_User_GetAccessToken();
#endif
}

void UOculusPlatform::OnSessionEnd()
{
	//
}

void UOculusPlatform::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cog.IsValid())
	{
		cog->OnSessionBegin.RemoveDynamic(this, &UOculusPlatform::OnSessionBegin);
		cog->OnPreSessionEnd.RemoveDynamic(this, &UOculusPlatform::OnSessionEnd);
		if (cog->HasStartedSession())
		{
			OnSessionEnd();
		}
	}
}

#ifdef INCLUDE_OCULUS_PLATFORM
void UOculusPlatform::ProcessOculusMessages()
{
	ovrMessageHandle Message;
	//ovrMessageHandle UserMessage;
	//ovrMessageHandle AccessTokenMessage;
	while ((Message = ovr_PopMessage()) != nullptr)
	{
		if (ovr_Message_GetType(Message) == ovrMessage_Entitlement_GetIsViewerEntitled)
		{
			if (ovr_Message_IsError(Message))
			{
				ovrErrorHandle Error = ovr_Message_GetError(Message);
				const char* ErrorMessage = ovr_Error_GetMessage(Error);
			}
			else //user is entitled
			{

			}
		}

		if (ovr_Message_GetType(Message) == ovrMessage_User_Get)
		{
			if (ovr_Message_IsError(Message))
			{
				// Error handling
				ovrErrorHandle Error = ovr_Message_GetError(Message);
				const char* ErrorMessage = ovr_Error_GetMessage(Error);
			}
			else
			{
				// Handle the user data
				HandleUserRetrieved(Message);
			}
		}

		if (ovr_Message_GetType(Message) == ovrMessage_User_GetAccessToken)
		{
			if (ovr_Message_IsError(Message))
			{
				//error handling
				ovrErrorHandle Error = ovr_Message_GetError(Message);
				const char* ErrorMessage = ovr_Error_GetMessage(Error);
			}
			else
			{
				//handle access token
				HandleAccessToekenRetrieved(Message);
			}
		}
	}
}

void UOculusPlatform::HandleUserRetrieved(const ovrMessageHandle Message)
{
	ovrUserHandle User = ovr_Message_GetUser(Message);

	const char* DisplayName = ovr_User_GetDisplayName(User); 
	const char* UserName = ovr_User_GetOculusID(User); 

	FString displayNameStr = FString::Printf(TEXT("%s"), *FString(DisplayName));
	FString usernameStr = FString::Printf(TEXT("%s"), *FString(UserName));

	ovrID oculusID = ovr_User_GetID(User);
	char* idString = new char[256];
	ovrID_ToString(idString, 256, oculusID);

	cog->SetParticipantFullName(usernameStr);

	if (UseOculusIdAsParticipant)
	{
		cog->SetParticipantId(idString);
	}

	cog->SetParticipantProperty("oculusId", FString(idString));
	cog->SetParticipantProperty("oculusDisplayName", displayNameStr);
	cog->SetParticipantProperty("oculusUsername", usernameStr);

	OnOculusNameHandled.Broadcast(displayNameStr);

	if (gotAccessToken == false)
	{
		ovr_User_GetAccessToken();
	}
}

void UOculusPlatform::HandleAccessToekenRetrieved(const ovrMessageHandle Message)
{
	const char* AccessToken = ovr_Message_GetString(Message);

	//make GET request for subscription context

	FString AccessTokenStr = FString(AccessToken);
	if (AccessTokenStr.Len() > 0)
	{
		gotAccessToken = true;
		SubscriptionStatusQuery(AccessTokenStr);
	}

}

void UOculusPlatform::SubscriptionStatusQuery(FString AccessToken)
{
	FString Url = FString::Printf(TEXT("https://graph.oculus.com/application/subscriptions?access_token=%s&fields=sku,period_start_time,period_end_time,is_trial,is_active,next_renewal_time"), *AccessToken);

	// Create HTTP Request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Handle the response
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UOculusPlatform::OnHttpResponseReceived);
	HttpRequest->ProcessRequest();
}

void UOculusPlatform::OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		// Parse the JSON response if needed
		FString JsonResponse = Response->GetContentAsString();
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			// Handle your data
			// Access the data array
			const TArray<TSharedPtr<FJsonValue>>* DataArray;
			if (JsonObject->TryGetArrayField(TEXT("data"), DataArray))
			{
				// Ensure DataArray is not null
				if (DataArray != nullptr)
				{
					// Iterate through the array using a pointer dereference
					for (const auto& Element : *DataArray)
					{
						TSharedPtr<FJsonObject> DataObject = Element->AsObject();
						if (DataObject.IsValid())
						{
							FString Sku;
							bool IsTrial;
							bool IsActive;
							FString period_start_time;
							FString period_end_time;
							FString next_renewal_time;

							if (DataObject->TryGetStringField(TEXT("sku"), Sku) &&
								DataObject->TryGetBoolField(TEXT("is_trial"), IsTrial) &&
								DataObject->TryGetBoolField(TEXT("is_active"), IsActive) &&
								DataObject->TryGetStringField(TEXT("period_start_time"), period_start_time) &&
								DataObject->TryGetStringField(TEXT("period_end_time"), period_end_time) &&
								DataObject->TryGetStringField(TEXT("next_renewal_time"), next_renewal_time))
							{
								
								// Convert date-time strings to Unix timestamps
								FDateTime StartTime, EndTime, NextRenewalTime;
								if (FDateTime::ParseIso8601(*period_start_time, StartTime) &&
									FDateTime::ParseIso8601(*period_end_time, EndTime) &&
									FDateTime::ParseIso8601(*next_renewal_time, NextRenewalTime))
								{
									int64 StartTimestamp = StartTime.ToUnixTimestamp();
									int64 EndTimestamp = EndTime.ToUnixTimestamp();
									int64 NextRenewalTimestamp = NextRenewalTime.ToUnixTimestamp();

									//add to json for gaze stream
									TSharedPtr<FJsonObject> oculusSub = MakeShareable(new FJsonObject);
									oculusSub->SetStringField(TEXT("sku"), Sku);
									oculusSub->SetBoolField(TEXT("is_active"), IsActive);
									oculusSub->SetBoolField(TEXT("is_trial"), IsTrial);
									oculusSub->SetNumberField(TEXT("period_start_date"), StartTimestamp);
									oculusSub->SetNumberField(TEXT("period_end_date"), EndTimestamp);
									oculusSub->SetNumberField(TEXT("next_renewal_date"), NextRenewalTimestamp);

									SubscriptionsJsonObject->SetObjectField(Sku, oculusSub);

								}
								else
								{
									// Handle parsing error for date strings
								}
							}
						}
					}
					SendSubscriptionData();
				}
				else
				{
					//DataArray is null
				}
			}
			else
			{
				//Failed to access data field in JSON
			}
		}
		else
		{
			//Failed to parse JSON
		}
	}
	else
	{
		// HTTP Request Failed
	}
}

void UOculusPlatform::SendSubscriptionData()
{
	int partNum = cog->gazeDataRecorder->GetPartNumber();
	cog->gazeDataRecorder->IncrementPartNumber();

	//construct the rest of the gaze stream, then add SubscriptionsJsonObject to it
	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;

	wholeObj->SetStringField("userid", cog->GetDeviceID());
	if (!cog->LobbyId.IsEmpty())
	{
		wholeObj->SetStringField("lobbyId", cog->LobbyId);
	}
	wholeObj->SetNumberField("timestamp", cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetNumberField("part", partNum);
	wholeObj->SetStringField("formatversion", "1.0");
	

	FName DeviceName(NAME_None);
	FString DeviceNameString = "unknown";

	//get HMDdevice name
	if (GEngine->XRSystem.IsValid())
	{
		DeviceName = GEngine->XRSystem->GetSystemName();
		DeviceNameString = FUtil::GetDeviceName(DeviceName.ToString());
	}

	wholeObj->SetStringField("formatversion", "1.0");
	wholeObj->SetStringField("hmdtype", DeviceNameString);

	wholeObj->SetNumberField("interval", 0.1f);

	wholeObj->SetArrayField("data", dataArray);

	wholeObj->SetObjectField(TEXT("subscriptions"), SubscriptionsJsonObject);

	FString OutputString;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);

	// Send serialized JSON
	cog->network->NetworkCall("gaze", OutputString, false);
}

#endif