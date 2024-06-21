// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#ifdef INCLUDE_OCULUS_PLATFORM
#include "OVR_Platform.h"
#endif
#include "OculusPlatform.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UOculusPlatform : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOculusPlatform();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float currentTime = 0;
	float maxWaitTime = 60.f;

	TSharedPtr<FAnalyticsProviderCognitive3D> cog;

	UFUNCTION()
		void OnSessionBegin();
	UFUNCTION()
		void OnSessionEnd();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#ifdef INCLUDE_OCULUS_PLATFORM

	// Processes incoming messages
	void ProcessOculusMessages();

	// Handles successful user data retrieval
	void HandleUserRetrieved(const ovrMessageHandle Message);

	//Handles successful access token data retrieval
	void HandleAccessToekenRetrieved(const ovrMessageHandle Message);

	UPROPERTY(EditAnywhere, Category = "Oculus Platform")
		bool RecordOculusData = false;

	// User ID to retrieve
	ovrID CurrentUserID;
	bool gotAccessToken;
	void SubscriptionStatusQuery(FString AccessToken);
	void OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SendSubscriptionData();

	// Subscriptions JSON object
	TSharedPtr<FJsonObject> SubscriptionsJsonObject = MakeShareable(new FJsonObject);
#endif

};
