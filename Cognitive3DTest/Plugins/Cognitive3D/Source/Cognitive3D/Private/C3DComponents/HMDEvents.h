/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
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
