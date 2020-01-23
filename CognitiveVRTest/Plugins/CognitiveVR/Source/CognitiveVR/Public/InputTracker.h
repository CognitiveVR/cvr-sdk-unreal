// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CognitiveVR.h"
#include "CoreMinimal.h"
#include "DynamicObject.h"
#include "MotionControllerComponent.h"
#include "InputTracker.generated.h"

UCLASS()
class COGNITIVEVR_API AInputTracker : public AActor
{
	GENERATED_BODY()
	
private:
	UDynamicObject* LeftHand;
	UDynamicObject* RightHand;

public:	
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.1))
		float Interval = 0.1;
	// Sets default values for this actor's properties
	AInputTracker();

private:
	EC3DControllerType ControllerType;
	virtual void BeginPlay() override;
	void FindControllers();
	virtual void Tick(float DeltaTime) override;
	void IntervalUpdate();

	float CurrentIntervalTime = 0;
	float MinimumVectorChange = 0.05;

	void AppendInputState(bool right, FControllerInputState state);

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
