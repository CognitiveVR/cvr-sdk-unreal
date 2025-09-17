// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CognitiveEditorTools.h"
#include "Analytics.h"
#include "SegmentAnalytics.generated.h"

/**
 * 
 */
UCLASS()
class USegmentAnalytics : public UObject
{
	GENERATED_BODY()
	
public:
    static USegmentAnalytics* Get(); // Singleton accessor
    void Initialize();
    void GetKeyFromServer();

    void TrackEvent(const FString& EventName, const FString& ButtonName);
    void TrackEvent(const FString& EventName, TSharedPtr<FJsonObject> Properties);

    //getters
    int32 GetUserId() const { return UserId; }
    int32 GetProjectId() const { return ProjectId; }

private:
    
    FString KEY_URL = "https://data.cognitive3d.com/segmentWriteKey?engine=unreal";

    FString WriteKey;
    int32 UserId = 0;
    int32 AnonymousId = 0;
    int32 GroupId = 0;
    int32 ProjectId = 0;

    void FetchUserData();
    void Identify(TSharedPtr<FJsonObject> UserData);
    void Group(TSharedPtr<FJsonObject> UserData);

    void SendToSegment(const FString& Endpoint, const TSharedRef<FJsonObject>& JsonObject);
    static USegmentAnalytics* SingletonInstance;
};
