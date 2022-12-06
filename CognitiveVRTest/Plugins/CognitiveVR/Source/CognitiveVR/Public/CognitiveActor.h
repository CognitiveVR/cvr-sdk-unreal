// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "C3DCommonTypes.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "CoreMinimal.h"
#include "CognitiveActor.generated.h"

UCLASS()
class COGNITIVEVR_API ACognitiveActor : public AActor
{
	GENERATED_BODY()

public:	
	
	//TODO confirm that these delegates need to be defined on an actor in the scene, and can't be done in the cognitive provider
	//but i also want access to these in blueprints on this class
	ACognitiveActor();
	UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
		FOnCognitiveSessionBegin OnSessionBegin;
	UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
		FOnCognitivePreSessionEnd OnPreSessionEnd;
	UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
		FOnCognitivePostSessionEnd OnPostSessionEnd;
	UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
		FOnRequestSend OnRequestSend;

	static ACognitiveActor* GetCognitiveActor();

private:

	static ACognitiveActor* instance;
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
	virtual void BeginPlay() override;
	//virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
