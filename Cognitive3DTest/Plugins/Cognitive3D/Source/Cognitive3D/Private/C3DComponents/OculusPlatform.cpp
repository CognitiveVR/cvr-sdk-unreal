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
#ifdef INCLUDE_OCULUS_PLATFORM
#ifdef __ANDROID__
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jobject Activity = FAndroidApplication::GetGameActivityThis();

	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString OculusAppId = FAnalytics::Get().GetConfigValueFromIni(EngineIni, "OnlineSubsystemOculus", "OculusAppId", false);
	auto tempAnsi = StringCast<ANSICHAR>(*OculusAppId);
	const char* AppID = tempAnsi.Get();

	if (Env != nullptr && Activity != nullptr)
	{
		ovrPlatformInitializeResult result = ovr_PlatformInitializeAndroid(AppID, Activity, Env);
		if (result != ovrPlatformInitialize_Success)
		{
			// Handle initialization failure
			UE_LOG(LogTemp, Error, TEXT("Failed to initialize Oculus Platform, Error code: %d"), result);
		}
		else
		{
			// Initialization successful
			UE_LOG(LogTemp, Log, TEXT("Oculus Platform initialized successfully"));
		}
	}
	else
	{
		// Failed to obtain JNI Environment or Activity
		UE_LOG(LogTemp, Error, TEXT("Failed to obtain JNI Environment or Android Activity"));
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
				UE_LOG(LogTemp, Error, TEXT("User is not entitled: %s"), *FString(ErrorMessage));
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
				UE_LOG(LogTemp, Error, TEXT("Failed to retrieve user: %s"), *FString(ErrorMessage));
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
				UE_LOG(LogTemp, Error, TEXT("Failed to retrieve access token: %s"), *FString(ErrorMessage));
			}
			else
			{
				//handle access token
				HandleAccessToekenRetrieved(Message);
			}
		}
	}
	if (gotAccessToken == false)
	{
		ovr_User_GetAccessToken();
	}
}

void UOculusPlatform::HandleUserRetrieved(const ovrMessageHandle Message)
{
	ovrUserHandle User = ovr_Message_GetUser(Message);
	const char* UserName = ovr_User_GetDisplayName(User);
	UE_LOG(LogTemp, Log, TEXT("User retrieved: %s"), *FString(UserName));
	FString propertyStr = FString::Printf(TEXT("%s"), *FString(UserName));
	//cog->SetSessionProperty("Oculus Username", propertyStr);
	ovrID oculusID = ovr_User_GetID(User);
	char* idString = new char[256];
	ovrID_ToString(idString, 256, oculusID);
	cog->SetParticipantProperty("oculusId", idString);
	cog->SetParticipantProperty("oculusUsername", propertyStr);

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
	//cog->SetSessionProperty("acesstoken test", AccessTokenStr);
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
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &ACognitive3DActor::OnHttpResponseReceived);
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
			UE_LOG(LogTemp, Warning, TEXT("Successfully retrieved data: %s"), *Response->GetContentAsString());

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

							if (DataObject->TryGetStringField(TEXT("sku"), Sku) &&
								DataObject->TryGetBoolField(TEXT("is_trial"), IsTrial) &&
								DataObject->TryGetBoolField(TEXT("is_active"), IsActive))
							{
								cog->SetSessionProperty("c3d.user.meta.subscription1.sku", Sku);
								cog->SetSessionProperty("c3d.user.meta.subscription1.is_trial", IsTrial);
								cog->SetSessionProperty("c3d.user.meta.subscription1.is_active", IsActive);

								UE_LOG(LogTemp, Log, TEXT("SKU: %s, Is Trial: %s, Is Active: %s"),
									*Sku,
									IsTrial ? TEXT("true") : TEXT("false"),
									IsActive ? TEXT("true") : TEXT("false"));
							}
						}
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("DataArray is null."));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to access 'data' field in JSON."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON."));
		}
	}
	else
	{
		// Handle errors
		UE_LOG(LogTemp, Error, TEXT("HTTP Request Failed: %s"), *Response->GetContentAsString());
	}
}
#endif