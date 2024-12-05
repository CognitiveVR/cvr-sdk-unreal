// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WifiSignal.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UWifiSignal : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWifiSignal();

protected:
	// Called when the application starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void OnSessionBegin();
	UFUNCTION()
	void OnSessionEnd();
	FTimerHandle IntervalHandle;
	void EndInterval();
	float IntervalDuration = 1.0f;
	int currentSignalStrength = 0;
	int previousSignalStrength = 0;

};
