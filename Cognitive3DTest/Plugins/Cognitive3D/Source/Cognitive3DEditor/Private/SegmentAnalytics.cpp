// Fill out your copyright notice in the Description page of Project Settings.


#include "SegmentAnalytics.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "Misc/Base64.h"

USegmentAnalytics* USegmentAnalytics::SingletonInstance = nullptr;

USegmentAnalytics* USegmentAnalytics::Get()
{
    if (!SingletonInstance)
    {
        SingletonInstance = NewObject<USegmentAnalytics>();
        SingletonInstance->AddToRoot(); // Prevent GC
    }
    return SingletonInstance;
}

void USegmentAnalytics::Initialize()
{
    GetKeyFromServer();
 
    // Generate temporary user ID if not yet set
    if (SingletonInstance->UserId == 0)
    {
        SingletonInstance->UserId = static_cast<int32>(GetTypeHash(FGuid::NewGuid()));
    }
	// Generate anonymous ID if not yet set
	if (SingletonInstance->AnonymousId == 0)
	{
		SingletonInstance->AnonymousId = static_cast<int32>(GetTypeHash(FGuid::NewGuid()));
	}
	UE_LOG(LogTemp, Log, TEXT("SegmentAnalytics initialized with UserId: %d, AnonymousId: %d"), UserId, AnonymousId);
	// Optionally, you can also set a GroupId if needed
	SingletonInstance->GroupId = static_cast<int32>(GetTypeHash(FGuid::NewGuid()));

	UE_LOG(LogTemp, Log, TEXT("SegmentAnalytics initialized with WriteKey: %s"), *WriteKey);

	FetchUserData();
}

void USegmentAnalytics::GetKeyFromServer()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(KEY_URL);
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            if (bSuccess && Resp->GetResponseCode() == 200)
            {
                FString RetrievedKey = Resp->GetContentAsString().TrimStartAndEnd();

                if (!RetrievedKey.IsEmpty())
                {
                    WriteKey = RetrievedKey;
                    UE_LOG(LogTemp, Log, TEXT("Segment write key retrieved and stored."));
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Empty Segment write key received."));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to fetch Segment write key: %s"), *Resp->GetContentAsString());
            }
        });

    Request->ProcessRequest();
}

void USegmentAnalytics::TrackEvent(const FString& EventName, const FString& ButtonName)
{
    if (WriteKey.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("WriteKey is not set. Cannot track event."));
        return;
    }

    TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetNumberField("userId", UserId);
	JsonObject->SetNumberField("anonymousId", AnonymousId);
    JsonObject->SetStringField("event", EventName);

    TSharedPtr<FJsonObject> PropertiesObject = MakeShared<FJsonObject>();
    PropertiesObject->SetStringField("buttonName", ButtonName);

    JsonObject->SetObjectField("properties", PropertiesObject);

    SendToSegment(TEXT("track"), JsonObject);
}

void USegmentAnalytics::TrackEvent(const FString& EventName, TSharedPtr<FJsonObject> Properties)
{
    if (WriteKey.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("WriteKey is not set. Cannot track event."));
        return;
    }

    TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetNumberField("userId", UserId);
    JsonObject->SetNumberField("anonymousId", AnonymousId);
    JsonObject->SetStringField("event", EventName);
    JsonObject->SetObjectField("properties", Properties);

    SendToSegment(TEXT("track"), JsonObject);
}

void USegmentAnalytics::FetchUserData()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
    GConfig->LoadFile(C3DSettingsPath);

    FString Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
    if (Gateway.Len() == 0)
    {
        Gateway = "data.cognitive3d.com";
    }
    FString userURL = "https://" + Gateway + "/v0/user";
    Request->SetURL(userURL);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + FCognitiveEditorTools::GetInstance()->DeveloperKey);

    Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            if (!bSuccess || Resp->GetResponseCode() != 200)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to fetch user data: %s"), *Resp->GetContentAsString());
                return;
            }

            TSharedPtr<FJsonObject> UserData;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());

            if (!FJsonSerializer::Deserialize(Reader, UserData) || !UserData.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid JSON in user data response."));
                return;
            }

            // Log the raw parsed object
            FString PrettyJson;
            TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PrettyJson);
            FJsonSerializer::Serialize(UserData.ToSharedRef(), Writer);

            UE_LOG(LogTemp, Log, TEXT("UserData Response:\n%s"), *PrettyJson);

            int32 ParsedUserId = 0;
            int32 ParsedGroupId = 0;

            if (UserData->TryGetNumberField(TEXT("userId"), ParsedUserId))
            {
                UserId = ParsedUserId;
            }

            if (UserData->TryGetNumberField(TEXT("organizationId"), ParsedGroupId))
            {
                GroupId = ParsedGroupId;
            }

            Identify(UserData);
            Group(UserData);
            TrackEvent(TEXT("EditorStarted"), TEXT("Startup"));
        });

    Request->ProcessRequest();
}

void USegmentAnalytics::Identify(TSharedPtr<FJsonObject> UserData)
{
    if (!UserData.IsValid()) return;

    TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetNumberField(TEXT("userId"), UserId);
    Payload->SetNumberField(TEXT("anonymousId"), AnonymousId);

    TSharedPtr<FJsonObject> Traits = MakeShared<FJsonObject>();

    FString Email, FirstName, LastName;
    if (UserData->TryGetStringField(TEXT("email"), Email)) Traits->SetStringField(TEXT("email"), Email);
    UserData->TryGetStringField(TEXT("firstName"), FirstName);
    UserData->TryGetStringField(TEXT("lastName"), LastName);
	FString fullName = FirstName + TEXT(" ") + LastName;
	if (!fullName.IsEmpty())
	{
		Traits->SetStringField(TEXT("name"), fullName);
	}
	int32 ProjectId = 0;
    FString ProjectName;
	if (UserData->TryGetNumberField(TEXT("projectId"), ProjectId)) Traits->SetNumberField(TEXT("projectId"), ProjectId);
	if (UserData->TryGetStringField(TEXT("projectName"), ProjectName))
	{
		Traits->SetStringField(TEXT("projectName"), ProjectName);
	}

    Traits->SetStringField(TEXT("engine"), TEXT("UnrealEditor"));

    Payload->SetObjectField(TEXT("traits"), Traits);
    SendToSegment(TEXT("identify"), Payload);
}

void USegmentAnalytics::Group(TSharedPtr<FJsonObject> UserData)
{
    if (!UserData.IsValid()) return;

    TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetNumberField(TEXT("userId"), UserId);
    Payload->SetNumberField(TEXT("anonymousId"), AnonymousId);
	Payload->SetNumberField(TEXT("groupId"), GroupId);

    TSharedPtr<FJsonObject> Traits = MakeShared<FJsonObject>();

    FString OrgName;
    if (UserData->TryGetStringField(TEXT("organizationName"), OrgName))
    {
        Traits->SetStringField(TEXT("organizationName"), OrgName);
    }

    Payload->SetObjectField(TEXT("traits"), Traits);

    SendToSegment(TEXT("group"), Payload);
}

void USegmentAnalytics::SendToSegment(const FString& Endpoint, const TSharedRef<FJsonObject>& JsonObject)
{
    if (WriteKey.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Segment WriteKey not set"));
        return;
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject, Writer);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(FString::Printf(TEXT("https://api.segment.io/v1/%s"), *Endpoint));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    const FString AuthHeader = TEXT("Basic ") + FBase64::Encode(WriteKey + TEXT(":"));
    Request->SetHeader(TEXT("Authorization"), AuthHeader);
    Request->SetContentAsString(OutputString);

    Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            if (bSuccess && Resp->GetResponseCode() == 200)
            {
                UE_LOG(LogTemp, Log, TEXT("Segment event sent successfully."));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Segment event failed: %s"), *Resp->GetContentAsString());
            }
        });

    Request->ProcessRequest();
}
