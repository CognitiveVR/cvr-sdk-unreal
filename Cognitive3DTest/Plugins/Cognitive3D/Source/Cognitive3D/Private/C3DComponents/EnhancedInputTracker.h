// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputTriggers.h"
#ifdef INCLUDE_OCULUS_PLUGIN
#include "OculusXRInputFunctionLibrary.h"
#endif
#ifdef INCLUDE_PICO_PLUGIN
#include "PXR_InputFunctionLibrary.h"
#include "PICOXRInput/Private/PXR_InputState.h"
#endif 
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "MotionControllerComponent.h"
#include "Cognitive3D/Private/C3DComponents/InputTracker.h"
#include "EnhancedInputTracker.generated.h"



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEnhancedInputTracker : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEnhancedInputTracker();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
		void FindControllers();

private:

	UDynamicObject* LeftHand;
	UDynamicObject* RightHand;

	float Interval = 0.1;

	TSharedPtr<FAnalyticsProviderCognitive3D> cog;
	UEnhancedInputComponent* EnhancedInputComponent;

	EC3DControllerType ControllerType;
	void IntervalUpdate();

	float CurrentIntervalTime = 0;
	float MinimumVectorChange = 0.05;

	void AppendInputState(const bool isRight, FControllerInputState& state);

	//button states
	FControllerInputStateCollection LeftInputStates;
	FControllerInputStateCollection RightInputStates;

	int32 LeftTriggerValue;
	int32 RightTriggerValue;

	int32 LeftGripValue;
	int32 RightGripValue;

	FVector LeftJoystickAxis;
	FVector RightJoystickAxis;

	FVector LeftTouchpadAxis;
	FVector RightTouchpadAxis;

	//to get local player controller for enhanced input
	TArray<APlayerController*, FDefaultAllocator> controllers;

	// Input actions

	UPROPERTY()
		UInputAction* C3D_LeftMenuButton;

	UPROPERTY()
		UInputAction* C3D_RightMenuButton;

	UPROPERTY()
		UInputAction* C3D_RightTrackpadTouch;

	UPROPERTY()
		UInputAction* C3D_LeftTrackpadTouch;

	UPROPERTY()
		UInputAction* C3D_RightTrackpadClick;

	UPROPERTY()
		UInputAction* C3D_LeftTrackpadClick;

	UPROPERTY()
		UInputAction* C3D_LeftFaceButtonOne;

	UPROPERTY()
		UInputAction* C3D_RightFaceButtonOne;

	UPROPERTY()
		UInputAction* C3D_LeftFaceButtonTwo;

	UPROPERTY()
		UInputAction* C3D_RightFaceButtonTwo;

	UPROPERTY()
		UInputAction* C3D_LeftGrip;

	UPROPERTY()
		UInputAction* C3D_RightGrip;

	UPROPERTY()
		UInputAction* C3D_LeftTrigger;

	UPROPERTY()
		UInputAction* C3D_RightTrigger;

	UPROPERTY()
		UInputAction* C3D_LeftJoystick;

	UPROPERTY()
		UInputAction* C3D_RightJoystick;

	// Input Axis
	UPROPERTY()
		UInputAction* C3D_LeftJoystickAxis;

	UPROPERTY()
		UInputAction* C3D_LeftJoystickX;

	UPROPERTY()
		UInputAction* C3D_LeftJoystickY;

	UPROPERTY()
		UInputAction* C3D_RightJoystickAxis;

	UPROPERTY()
		UInputAction* C3D_RightJoystickX;

	UPROPERTY()
		UInputAction* C3D_RightJoystickY;

	UPROPERTY()
		UInputAction* C3D_RightTrackpadAxis;

	UPROPERTY()
		UInputAction* C3D_RightTrackpadX;

	UPROPERTY()
		UInputAction* C3D_RightTrackpadY;

	UPROPERTY()
		UInputAction* C3D_LeftTrackpadAxis;

	UPROPERTY()
		UInputAction* C3D_LeftTrackpadX;

	UPROPERTY()
		UInputAction* C3D_LeftTrackpadY;

	UPROPERTY()
		UInputAction* C3D_LeftGripAxis;

	UPROPERTY()
		UInputAction* C3D_RightGripAxis;

	UPROPERTY()
		UInputAction* C3D_LeftTriggerAxis;

	UPROPERTY()
		UInputAction* C3D_RightTriggerAxis;

	// Input mapping context
	UPROPERTY()
		UInputMappingContext* C3DInputMappingContext;

	void InitializeEnhancedInput(APlayerController* PlayerController);

	//input callbacks
public:

	void LeftFaceButtonOnePressed();
	void LeftFaceButtonOneReleased();

	void LeftFaceButtonTwoPressed();
	void LeftFaceButtonTwoReleased();

	void LeftMenuButtonPressed();
	void LeftMenuButtonReleased();

	void LeftJoystickPressed(const FInputActionValue& Value);
	void LeftJoystickReleased(const FInputActionValue& Value);
	void LeftJoystickXPressed(const FInputActionValue& Value);
	void LeftJoystickYPressed(const FInputActionValue& Value);
	void LeftJoystickXReleased(const FInputActionValue& Value);
	void LeftJoystickYReleased(const FInputActionValue& Value);
	float LeftJoystickH;
	float LeftJoystickV;

	void LeftGripPressed();
	void LeftGripReleased(const FInputActionValue& Value);
	float LeftGripAxis;

	void LeftTouchpadReleased(const FInputActionValue& Value);
	void LeftTouchpadTouched(const FInputActionValue& Value);
	void LeftTouchpadPressed(const FInputActionValue& Value);
	void LeftTouchpadPressRelease(const FInputActionValue& Value);
	void LeftTouchpadXReleased(const FInputActionValue& Value);
	void LeftTouchpadXTouched(const FInputActionValue& Value);
	void LeftTouchpadXPressed(const FInputActionValue& Value);
	void LeftTouchpadXPressRelease(const FInputActionValue& Value);
	void LeftTouchpadYReleased(const FInputActionValue& Value);
	void LeftTouchpadYTouched(const FInputActionValue& Value);
	void LeftTouchpadYPressed(const FInputActionValue& Value);
	void LeftTouchpadYPressRelease(const FInputActionValue& Value);
	float LeftTouchpadH;
	float LeftTouchpadV;

	void LeftTriggerReleased(const FInputActionValue& Value);
	void LeftTriggerPressed();
	float LeftTriggerAxis;


	void RightFaceButtonOnePressed();
	void RightFaceButtonOneReleased();

	void RightFaceButtonTwoPressed();
	void RightFaceButtonTwoReleased();

	void RightMenuButtonPressed();
	void RightMenuButtonReleased();

	void RightJoystickPressed(const FInputActionValue& Value);
	void RightJoystickReleased(const FInputActionValue& Value);
	void RightJoystickXPressed(const FInputActionValue& Value);
	void RightJoystickYPressed(const FInputActionValue& Value);
	void RightJoystickXReleased(const FInputActionValue& Value);
	void RightJoystickYReleased(const FInputActionValue& Value);
	float RightJoystickH;
	float RightJoystickV;

	void RightGripPressed();
	void RightGripReleased(const FInputActionValue& Value);
	float RightGripAxis;

	void RightTouchpadReleased(const FInputActionValue& Value);
	void RightTouchpadTouched(const FInputActionValue& Value);
	void RightTouchpadPressed(const FInputActionValue& Value);
	void RightTouchpadPressRelease(const FInputActionValue& Value);
	void RightTouchpadXReleased(const FInputActionValue& Value);
	void RightTouchpadXTouched(const FInputActionValue& Value);
	void RightTouchpadXPressed(const FInputActionValue& Value);
	void RightTouchpadXPressRelease(const FInputActionValue& Value);
	void RightTouchpadYReleased(const FInputActionValue& Value);
	void RightTouchpadYTouched(const FInputActionValue& Value);
	void RightTouchpadYPressed(const FInputActionValue& Value);
	void RightTouchpadYPressRelease(const FInputActionValue& Value);
	float RightTouchpadH;
	float RightTouchpadV;

	void RightTriggerReleased(const FInputActionValue& Value);
	void RightTriggerPressed();
	float RightTriggerAxis;
};
