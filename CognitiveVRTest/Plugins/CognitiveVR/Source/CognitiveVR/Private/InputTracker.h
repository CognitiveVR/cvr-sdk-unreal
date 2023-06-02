// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CognitiveVR/Public/CognitiveVR.h"
#include "CoreMinimal.h"
#include "CognitiveVR/Public/DynamicObject.h"
#include "MotionControllerComponent.h"
#include "InputTracker.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVEVR_API UInputTracker : public UActorComponent
{
	GENERATED_BODY()
	
private:
	UDynamicObject* LeftHand;
	UDynamicObject* RightHand;

	float Interval = 0.1;
	UInputTracker();

public:	
	UFUNCTION()
	void FindControllers();

private:

	UInputComponent* InputComponent;

	EC3DControllerType ControllerType;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	//events
public:
	void LeftFaceButtonOnePressed();
	void LeftFaceButtonOneReleased();
	
	void LeftFaceButtonTwoPressed();
	void LeftFaceButtonTwoReleased();
	
	void LeftMenuButtonPressed();
	void LeftMenuButtonReleased();
	
	void LeftJoystickPressed();
	void LeftJoystickReleased();

	void LeftGripPressed();
	void LeftGripReleased();
	
	void LeftTouchpadReleased();
	void LeftTouchpadTouched();
	void LeftTouchpadPressed();
	void LeftTouchpadPressRelease();

	void LeftTriggerReleased();
	void LeftTriggerPressed();


	void RightFaceButtonOnePressed();
	void RightFaceButtonOneReleased();

	void RightFaceButtonTwoPressed();
	void RightFaceButtonTwoReleased();

	void RightMenuButtonPressed();
	void RightMenuButtonReleased();

	void RightJoystickPressed();
	void RightJoystickReleased();

	void RightGripPressed();
	void RightGripReleased();

	void RightTouchpadReleased();
	void RightTouchpadTouched();
	void RightTouchpadPressed();
	void RightTouchpadPressRelease();

	void RightTriggerReleased();
	void RightTriggerPressed();
};
