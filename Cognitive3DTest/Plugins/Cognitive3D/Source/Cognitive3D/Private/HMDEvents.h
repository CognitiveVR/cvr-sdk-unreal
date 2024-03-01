// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cognitive3D/Private/util/util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "HMDEvents.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UHMDEvents : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHMDEvents();

private:
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnSessionBegin();
	UFUNCTION()
		void OnSessionEnd();
	
		void EndInterval();

	FTimerHandle IntervalHandle;
	float IntervalDuration = 1;
	EHMDWornState::Type WornState = EHMDWornState::NotWorn;
};
