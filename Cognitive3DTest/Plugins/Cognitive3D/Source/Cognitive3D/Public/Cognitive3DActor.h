/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "TimerManager.h"
#include "Cognitive3DActor.generated.h"

//this class basically only exists to send EndPlay to the CognitiveProvider
//and as a simple way of getting the right UWorld

class UActorComponent;
class FAnalyticsProviderCognitive3D;
class UMotionControllerComponent;
class USceneComponent;
class UDynamicObject;
class APawn;

UCLASS()
class COGNITIVE3D_API ACognitive3DActor : public AActor
{
	GENERATED_BODY()

public:	
	
	ACognitive3DActor();

	static ACognitive3DActor* GetCognitive3DActor(const UObject *WorldContextObject = nullptr);
	static UWorld* GetCognitiveSessionWorld();
	UDynamicObject* GetControllerDynamic(bool bRight) const;

	//used only for blueprint events
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Begin Session"))
		void ReceiveBeginSession();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Pre End Session"))
		void ReceivePreEndSession();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Post End Session"))
		void ReceivePostEndSession();

private:

	static TMap<uint32, TWeakObjectPtr<ACognitive3DActor>> PerWorldInstanceMap;
	TSharedPtr<FAnalyticsProviderCognitive3D> cog;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void OnEndPIE(bool bIsSimulating);

	bool bIsEditorStoppingPIE = false;

	//Find and automatically assign dynamic objects to controllers
	UPROPERTY()
	class UMotionControllerComponent* LeftController;

	UPROPERTY()
	class UMotionControllerComponent* RightController;

	UPROPERTY()
	USceneComponent* LeftHandComponent;

	UPROPERTY()
	USceneComponent* RightHandComponent;

	UPROPERTY(Transient)
	UDynamicObject* LeftDynamic;

	UPROPERTY(Transient)
	UDynamicObject* RightDynamic;

	UPROPERTY(Transient)
	UDynamicObject* AutomaticLeftDynamic;

	UPROPERTY(Transient)
	UDynamicObject* AutomaticRightDynamic;

	FTimerHandle ControllerDiscoveryHandle;
	TWeakObjectPtr<APawn> ControllerPawn;

	UFUNCTION()
	void InitializeControllers();

	USceneComponent* FindHandComponent(USceneComponent* Parent);

	UDynamicObject* FindControllerDynamic(USceneComponent* Parent, bool bRight) const;
};
