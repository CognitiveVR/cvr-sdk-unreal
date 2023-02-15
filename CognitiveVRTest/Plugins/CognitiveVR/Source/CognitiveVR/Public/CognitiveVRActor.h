// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "C3DCommonTypes.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "CoreMinimal.h"
#include "CognitiveVRActor.generated.h"

//this class basically only exists to send EndPlay to the CognitiveProvider
//and as a simple way of getting the right UWorld

UCLASS()
class COGNITIVEVR_API ACognitiveVRActor : public AActor
{
	GENERATED_BODY()

public:	
	
	ACognitiveVRActor();

	static ACognitiveVRActor* GetCognitiveVRActor();
	static UWorld* GetCognitiveSessionWorld();

	//used only for blueprint events
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Begin Session"))
		void ReceiveBeginSession();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Pre End Session"))
		void ReceivePreEndSession();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Post End Session"))
		void ReceivePostEndSession();

private:

	static ACognitiveVRActor* instance;
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};