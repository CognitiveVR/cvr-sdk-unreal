// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "BoundaryEvent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UBoundaryEvent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UBoundaryEvent();

private:	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
		void OnSessionBegin();
	UFUNCTION()
		void OnSessionEnd();


	FVector RoomSize;
	FVector HMDWorldPos;

	FRotator HMDRotation;
	FVector HMDPosition;
	FVector HMDNeckPos;
	TArray<FVector> StationaryPoints;
	TArray<FVector> GuardianPoints;

	float Interval = 0.1f; //change
	FTimerHandle IntervalHandle;
	void EndInterval();

	bool BoundaryCrossed;
	bool StillOutsideBoundary;

	bool PicoCheckBoundary;
};
