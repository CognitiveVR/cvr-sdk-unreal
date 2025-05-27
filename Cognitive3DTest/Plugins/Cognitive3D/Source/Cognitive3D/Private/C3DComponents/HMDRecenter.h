// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "VRNotificationsComponent.h"
#include "Components/ActorComponent.h"
#include "HMDRecenter.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UHMDRecenter : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHMDRecenter();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnSessionBegin();
	UFUNCTION()
	void OnSessionEnd();


	UPROPERTY()
	class UVRNotificationsComponent* VRNotifications;

	UFUNCTION()
	void HandleRecenter();

	UFUNCTION()
	void HandleControllerRecenter();

	FVector HMDWorldPos;
};
