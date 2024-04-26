// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#ifdef INCLUDE_OCULUS_PASSTHROUGH
#include "OculusXRFunctionLibrary.h"
#include "OculusXRPassthroughLayerComponent.h"
#endif
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "OculusPassthrough.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UOculusPassthrough : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOculusPassthrough();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


#ifdef INCLUDE_OCULUS_PASSTHROUGH
	bool IsPassthroughVisible;
	UOculusXRPassthroughLayerComponent* PassthroughLayer;
	void FindOculusXRPassthroughLayer(UOculusXRPassthroughLayerComponent*& OculusPassthroughLayer);
	float lastEventTime;

	float PassthroughSendInterval = 1.0f;
	float currentTime;
#endif
		
};
