/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Public/Cognitive3D.h"
#include "CoreMinimal.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "MotionControllerComponent.h"
#include "InputTracker.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVE3D_API UInputTracker : public UActorComponent
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

	TSharedPtr<FAnalyticsProviderCognitive3D> cog;
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
