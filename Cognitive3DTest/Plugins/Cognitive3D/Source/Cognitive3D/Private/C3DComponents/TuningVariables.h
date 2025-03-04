// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C3DCommonTypes.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "TuningVariables.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UTuningVariables : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTuningVariables();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TSharedPtr<FAnalyticsProviderCognitive3D> cog;

	UFUNCTION()
	void OnSessionBegin();
	UFUNCTION()
	void OnSessionEnd();

	UFUNCTION()
	void QueryTuningVariable(FString UserID);

	void OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void ParseJsonResponse(const FString& JsonResponse);
};
