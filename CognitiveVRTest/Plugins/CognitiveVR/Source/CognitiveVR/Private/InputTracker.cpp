// Fill out your copyright notice in the Description page of Project Settings.

#include "InputTracker.h"

// Sets default values
AInputTracker::AInputTracker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AInputTracker::BeginPlay()
{
	Super::BeginPlay();

	//get player controller 0
	APlayerController* p0 = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	EnableInput(p0);
	InputComponent->bBlockInput = 0;

	InputComponent->BindAction("RightMenuButton", EInputEvent::IE_Pressed, this, &AInputTracker::RightMenuButtonPressed).bConsumeInput = 0;
	InputComponent->BindAction("RightMenuButton", EInputEvent::IE_Released, this, &AInputTracker::RightMenuButtonReleased).bConsumeInput = 0;
	InputComponent->BindAction("LeftMenuButton", EInputEvent::IE_Pressed, this, &AInputTracker::LeftMenuButtonPressed).bConsumeInput = 0;
	InputComponent->BindAction("LeftMenuButton", EInputEvent::IE_Released, this, &AInputTracker::LeftMenuButtonReleased).bConsumeInput = 0;

	InputComponent->BindAction("RightTrigger", EInputEvent::IE_Pressed, this, &AInputTracker::RightTriggerPressed).bConsumeInput = 0;
	InputComponent->BindAction("RightTrigger", EInputEvent::IE_Released, this, &AInputTracker::RightTriggerReleased).bConsumeInput = 0;
	InputComponent->BindAction("LeftTrigger", EInputEvent::IE_Pressed, this, &AInputTracker::LeftTriggerPressed).bConsumeInput = 0;
	InputComponent->BindAction("LeftTrigger", EInputEvent::IE_Released, this, &AInputTracker::LeftTriggerReleased).bConsumeInput = 0;

	InputComponent->BindAction("RightGrip", EInputEvent::IE_Pressed, this, &AInputTracker::RightGripPressed).bConsumeInput = 0;
	InputComponent->BindAction("RightGrip", EInputEvent::IE_Released, this, &AInputTracker::RightGripReleased).bConsumeInput = 0;
	InputComponent->BindAction("LeftGrip", EInputEvent::IE_Pressed, this, &AInputTracker::LeftGripPressed).bConsumeInput = 0;
	InputComponent->BindAction("LeftGrip", EInputEvent::IE_Released, this, &AInputTracker::LeftGripReleased).bConsumeInput = 0;

	InputComponent->BindAction("RightTouchpadPress", EInputEvent::IE_Pressed, this, &AInputTracker::RightTouchpadPressed).bConsumeInput = 0;
	InputComponent->BindAction("RightTouchpadPress", EInputEvent::IE_Released, this, &AInputTracker::RightTouchpadPressRelease).bConsumeInput = 0;
	InputComponent->BindAction("RightTouchpadTouch", EInputEvent::IE_Pressed, this, &AInputTracker::RightTouchpadTouched).bConsumeInput = 0;
	InputComponent->BindAction("RightTouchpadTouch", EInputEvent::IE_Released, this, &AInputTracker::RightTouchpadReleased).bConsumeInput = 0;

	InputComponent->BindAction("LeftTouchpadPress", EInputEvent::IE_Pressed, this, &AInputTracker::LeftTouchpadPressed).bConsumeInput = 0;
	InputComponent->BindAction("LeftTouchpadPress", EInputEvent::IE_Released, this, &AInputTracker::LeftTouchpadPressRelease).bConsumeInput = 0;
	InputComponent->BindAction("LeftTouchpadTouch", EInputEvent::IE_Pressed, this, &AInputTracker::LeftTouchpadTouched).bConsumeInput = 0;
	InputComponent->BindAction("LeftTouchpadTouch", EInputEvent::IE_Released, this, &AInputTracker::LeftTouchpadReleased).bConsumeInput = 0;

	InputComponent->BindAction("RightJoystick", EInputEvent::IE_Pressed, this, &AInputTracker::RightJoystickPressed).bConsumeInput = 0;
	InputComponent->BindAction("RightJoystick", EInputEvent::IE_Released, this, &AInputTracker::RightJoystickReleased).bConsumeInput = 0;
	InputComponent->BindAction("LeftJoystick", EInputEvent::IE_Pressed, this, &AInputTracker::LeftJoystickPressed).bConsumeInput = 0;
	InputComponent->BindAction("LeftJoystick", EInputEvent::IE_Released, this, &AInputTracker::LeftJoystickReleased).bConsumeInput = 0;

	InputComponent->BindAction("LeftFaceButtonOne", EInputEvent::IE_Pressed, this, &AInputTracker::LeftFaceButtonOnePressed).bConsumeInput = 0;
	InputComponent->BindAction("LeftFaceButtonOne", EInputEvent::IE_Released, this, &AInputTracker::LeftFaceButtonOneReleased).bConsumeInput = 0;
	InputComponent->BindAction("LeftFaceButtonTwo", EInputEvent::IE_Pressed, this, &AInputTracker::LeftFaceButtonTwoPressed).bConsumeInput = 0;
	InputComponent->BindAction("LeftFaceButtonTwo", EInputEvent::IE_Released, this, &AInputTracker::LeftFaceButtonTwoReleased).bConsumeInput = 0;

	InputComponent->BindAction("RightFaceButtonOne", EInputEvent::IE_Pressed, this, &AInputTracker::RightFaceButtonOnePressed).bConsumeInput = 0;
	InputComponent->BindAction("RightFaceButtonOne", EInputEvent::IE_Released, this, &AInputTracker::RightFaceButtonOneReleased).bConsumeInput = 0;
	InputComponent->BindAction("RightFaceButtonTwo", EInputEvent::IE_Pressed, this, &AInputTracker::RightFaceButtonTwoPressed).bConsumeInput = 0;
	InputComponent->BindAction("RightFaceButtonTwo", EInputEvent::IE_Released, this, &AInputTracker::RightFaceButtonTwoReleased).bConsumeInput = 0;

	InputComponent->BindAxis(TEXT("LeftTriggerAxis")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("RightTriggerAxis")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("LeftTouchpadV")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("LeftTouchpadH")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("RightTouchpadV")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("RightTouchpadH")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("LeftJoystickH")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("LeftJoystickV")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("RightJoystickH")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("RightJoystickV")).bConsumeInput = 0;

	InputComponent->BindAxis(TEXT("LeftGripAxis")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("RightGripAxis")).bConsumeInput = 0;

	FindControllers();
}

void AInputTracker::FindControllers()
{

	for (TObjectIterator<UMotionControllerComponent> Itr; Itr; ++Itr)
	{
		UMotionControllerComponent *Component = *Itr;
		if (Component->GetOwner() == NULL) { continue; } //likely motion controller from wrong world
		if (LeftHand == NULL && Component->GetTrackingSource() == EControllerHand::Left)
		{
			auto left = Component->GetOwner()->GetComponentByClass(UDynamicObject::StaticClass());
			if (left != NULL)
			{
				LeftHand = Cast<UDynamicObject>(left);
				if (!LeftHand->IsController) { continue; }

				if (LeftHand->ControllerType == "oculustouchleft")
				{
					ControllerType = EC3DControllerType::Oculus;
				}
				else if (LeftHand->ControllerType == "windows_mixed_reality_controller_left")
				{
					ControllerType = EC3DControllerType::WindowsMixedReality;
				}
				else if (LeftHand->ControllerType == "vivecontroller")
				{
					ControllerType = EC3DControllerType::Vive;
				}
			}
		}
		else if (RightHand == NULL && Component->GetTrackingSource() == EControllerHand::Right)
		{
			auto right = Component->GetOwner()->GetComponentByClass(UDynamicObject::StaticClass());
			if (right != NULL)
			{
				RightHand = Cast<UDynamicObject>(right);
				if (!RightHand->IsController) { continue; }

				if (RightHand->ControllerType == "oculustouchright")
				{
					ControllerType = EC3DControllerType::Oculus;
				}
				else if (RightHand->ControllerType == "windows_mixed_reality_controller_right")
				{
					ControllerType = EC3DControllerType::WindowsMixedReality;
				}
				else if (RightHand->ControllerType == "vivecontroller")
				{
					ControllerType = EC3DControllerType::Vive;
				}
			}
		}
	}
}

void AInputTracker::Tick(float DeltaTime)
{
	if (LeftInputStates.States.Num() > 0)
	{
		if (LeftHand != NULL)
			LeftHand->FlushButtons(LeftInputStates);
		else
			LeftInputStates.States.Empty();
		//flush left controller states
	}
	if (RightInputStates.States.Num() > 0)
	{
		if (RightHand != NULL)
			RightHand->FlushButtons(RightInputStates);
		else
			RightInputStates.States.Empty();
		//flush right controller states
	}

	Super::Tick(DeltaTime);
	CurrentIntervalTime += DeltaTime;
	if (CurrentIntervalTime > Interval)
	{
		if (LeftHand == NULL || RightHand == NULL)
			FindControllers();

		CurrentIntervalTime -= Interval;
		IntervalUpdate();
	}
}

void AInputTracker::IntervalUpdate()
{
	//check if touchpad/joystick/grip/trigger values have changed
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		//triggers
		float currentValue = InputComponent->GetAxisValue("LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("vive_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = InputComponent->GetAxisValue("RightTriggerAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("vive_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//touchpads
		FVector currentLeftTouchpad = FVector(InputComponent->GetAxisValue("LeftTouchpadH"), InputComponent->GetAxisValue("LeftTouchpadV"), LeftTouchpadAxis.Z);
		if (FVector::Distance(LeftTouchpadAxis, currentLeftTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			LeftTouchpadAxis = currentLeftTouchpad;
			auto b = FControllerInputState("vive_touchpad", LeftTouchpadAxis);
			AppendInputState(false, b);
		}

		FVector currentRightTouchpad = FVector(InputComponent->GetAxisValue("RightTouchpadH"), InputComponent->GetAxisValue("RightTouchpadV"), RightTouchpadAxis.Z);
		if (FVector::Distance(RightTouchpadAxis, currentRightTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			RightTouchpadAxis = currentRightTouchpad;
			auto b = FControllerInputState("vive_touchpad", RightTouchpadAxis);
			AppendInputState(true, b);
		}
		break;
	}
	case EC3DControllerType::Oculus:
	{
		//triggers
		float currentValue = InputComponent->GetAxisValue("LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = InputComponent->GetAxisValue("RightTriggerAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//grip
		currentValue = InputComponent->GetAxisValue("LeftGripAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (LeftGripValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_grip", icurrentValue);
			AppendInputState(false, b);
			LeftGripValue = icurrentValue;
		}
		currentValue = InputComponent->GetAxisValue("RightGripAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightGripValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_grip", icurrentValue);
			AppendInputState(true, b);
			RightGripValue = icurrentValue;
		}

		//joysticks
		FVector currentLeftJoystick = FVector(InputComponent->GetAxisValue("LeftJoystickH"), InputComponent->GetAxisValue("LeftJoystickV"), LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("rift_joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(InputComponent->GetAxisValue("RightJoystickH"), InputComponent->GetAxisValue("RightJoystickV"), RightJoystickAxis.Z);
		if (FVector::Distance(RightJoystickAxis, currentRightJoystick) > MinimumVectorChange)
		{
			//write new stuff
			RightJoystickAxis = currentRightJoystick;
			auto b = FControllerInputState("rift_joystick", RightJoystickAxis);
			AppendInputState(true, b);
		}
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{

		//triggers
		float currentValue = InputComponent->GetAxisValue("LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("wmr_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = InputComponent->GetAxisValue("RightTriggerAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("wmr_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//touchpads

		FVector currentLeftTouchpad = FVector(InputComponent->GetAxisValue("LeftTouchpadH"), InputComponent->GetAxisValue("LeftTouchpadV"), LeftTouchpadAxis.Z);
		if (FVector::Distance(LeftTouchpadAxis, currentLeftTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			LeftTouchpadAxis = currentLeftTouchpad;
			auto b = FControllerInputState("wmr_touchpad", LeftTouchpadAxis);
			AppendInputState(false, b);
		}

		FVector currentRightTouchpad = FVector(InputComponent->GetAxisValue("RightTouchpadH"), InputComponent->GetAxisValue("RightTouchpadV"), RightTouchpadAxis.Z);
		if (FVector::Distance(RightTouchpadAxis, currentRightTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			RightTouchpadAxis = currentRightTouchpad;
			auto b = FControllerInputState("wmr_touchpad", RightTouchpadAxis);
			AppendInputState(true, b);
		}

		//joysticks
		FVector currentLeftJoystick = FVector(InputComponent->GetAxisValue("LeftJoystickH"), InputComponent->GetAxisValue("LeftJoystickV"), LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("wmr_joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(InputComponent->GetAxisValue("RightJoystickH"), InputComponent->GetAxisValue("RightJoystickV"), RightJoystickAxis.Z);
		if (FVector::Distance(RightJoystickAxis, currentRightJoystick) > MinimumVectorChange)
		{
			//write new stuff
			RightJoystickAxis = currentRightJoystick;
			auto b = FControllerInputState("wmr_joystick", RightJoystickAxis);
			AppendInputState(true, b);
		}
		break;
	}
	}
}

void AInputTracker::AppendInputState(bool right, FControllerInputState state)
{
	//append or change value
	if (right)
	{
		if (RightInputStates.States.Contains(state.AxisName))
		{
			RightInputStates.States[state.AxisName] = state;
		}
		else
		{
			RightInputStates.States.Add(state.AxisName,state);
		}
	}
	else
	{
		if (LeftInputStates.States.Contains(state.AxisName))
		{
			LeftInputStates.States[state.AxisName] = state;
		}
		else
		{
			LeftInputStates.States.Add(state.AxisName, state);
		}
	}
}

void AInputTracker::LeftFaceButtonOnePressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_xbtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void AInputTracker::LeftFaceButtonOneReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_xbtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void AInputTracker::LeftFaceButtonTwoPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_ybtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}
void AInputTracker::LeftFaceButtonTwoReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_ybtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void AInputTracker::LeftMenuButtonPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_menubtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_start", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_menubtn", 100);
		AppendInputState(false, b);
		break;
	}
	}
}
void AInputTracker::LeftMenuButtonReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_menubtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_start", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_menubtn", 0);
		AppendInputState(false, b);
		break;
	}
	}
}

void AInputTracker::LeftJoystickPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		float x = InputComponent->GetAxisValue("LeftJoystickH");
		float y = InputComponent->GetAxisValue("LeftJoystickV");
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("LeftJoystickH");
		float y = InputComponent->GetAxisValue("LeftJoystickV");
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	}
}
void AInputTracker::LeftJoystickReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		float x = InputComponent->GetAxisValue("LeftJoystickH");
		float y = InputComponent->GetAxisValue("LeftJoystickV");
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("LeftJoystickH");
		float y = InputComponent->GetAxisValue("LeftJoystickV");
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	}
}

void AInputTracker::LeftTouchpadReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = InputComponent->GetAxisValue("LeftTouchpadH");
		float y = InputComponent->GetAxisValue("LeftTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("LeftTouchpadH");
		float y = InputComponent->GetAxisValue("LeftTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	}
}
void AInputTracker::LeftTouchpadTouched()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = InputComponent->GetAxisValue("LeftTouchpadH");
		float y = InputComponent->GetAxisValue("LeftTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("LeftTouchpadH");
		float y = InputComponent->GetAxisValue("LeftTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}
void AInputTracker::LeftTouchpadPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = InputComponent->GetAxisValue("LeftTouchpadH");
		float y = InputComponent->GetAxisValue("LeftTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("LeftTouchpadH");
		float y = InputComponent->GetAxisValue("LeftTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	}
}

void AInputTracker::LeftTouchpadPressRelease()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = InputComponent->GetAxisValue("LeftTouchpadH");
		float y = InputComponent->GetAxisValue("LeftTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("LeftTouchpadH");
		float y = InputComponent->GetAxisValue("LeftTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}


void AInputTracker::RightFaceButtonOnePressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_abtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}
void AInputTracker::RightFaceButtonOneReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_abtn", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void AInputTracker::RightFaceButtonTwoPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_bbtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}
void AInputTracker::RightFaceButtonTwoReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_bbtn", 0);
		AppendInputState(true, b);
		break;
	}
		break;
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void AInputTracker::RightMenuButtonPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_menubtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_start", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_menubtn", 100);
		AppendInputState(true, b);
		break;
	}
	}
}
void AInputTracker::RightMenuButtonReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_menubtn", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_start", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_menubtn", 0);
		AppendInputState(true, b);
		break;
	}
	}
}

void AInputTracker::RightJoystickPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		float x = InputComponent->GetAxisValue("RightJoystickH");
		float y = InputComponent->GetAxisValue("RightJoystickV");
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("RightJoystickH");
		float y = InputComponent->GetAxisValue("RightJoystickV");
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}
void AInputTracker::RightJoystickReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::Oculus:
	{
		float x = InputComponent->GetAxisValue("RightJoystickH");
		float y = InputComponent->GetAxisValue("RightJoystickV");
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("RightJoystickH");
		float y = InputComponent->GetAxisValue("RightJoystickV");
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	}
}

void AInputTracker::RightTouchpadReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = InputComponent->GetAxisValue("RightTouchpadH");
		float y = InputComponent->GetAxisValue("RightTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("RightTouchpadH");
		float y = InputComponent->GetAxisValue("RightTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	}
}
void AInputTracker::RightTouchpadTouched()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = InputComponent->GetAxisValue("RightTouchpadH");
		float y = InputComponent->GetAxisValue("RightTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("RightTouchpadH");
		float y = InputComponent->GetAxisValue("RightTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}
void AInputTracker::RightTouchpadPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = InputComponent->GetAxisValue("RightTouchpadH");
		float y = InputComponent->GetAxisValue("RightTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("RightTouchpadH");
		float y = InputComponent->GetAxisValue("RightTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void AInputTracker::RightTouchpadPressRelease()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = InputComponent->GetAxisValue("RightTouchpadH");
		float y = InputComponent->GetAxisValue("RightTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = InputComponent->GetAxisValue("RightTouchpadH");
		float y = InputComponent->GetAxisValue("RightTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}

void AInputTracker::RightGripPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_grip", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_grip", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_grip", 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void AInputTracker::RightGripReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_grip", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		float currentValue = InputComponent->GetAxisValue("RightGripAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		RightGripValue = icurrentValue;
		auto b = FControllerInputState("rift_grip", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_grip", 0);
		AppendInputState(true, b);
		break;
	}
	}
}

void AInputTracker::LeftGripPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_grip", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_grip", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_grip", 100);
		AppendInputState(false, b);
		break;
	}
	}
}

void AInputTracker::LeftGripReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_grip", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		float currentValue = InputComponent->GetAxisValue("LeftGripAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftGripValue = icurrentValue;
		auto b = FControllerInputState("rift_grip", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_grip", 0);
		AppendInputState(false, b);
		break;
	}
	}
}


void AInputTracker::LeftTriggerPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_trigger", 100);
		LeftTriggerValue = 100;
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_trigger", 100);
		LeftTriggerValue = 100;
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_trigger", 100);
		LeftTriggerValue = 100;
		AppendInputState(false, b);
		break;
	}
	}
}

void AInputTracker::LeftTriggerReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float currentValue = InputComponent->GetAxisValue("LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftTriggerValue = icurrentValue;
		auto b = FControllerInputState("vive_trigger", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		float currentValue = InputComponent->GetAxisValue("LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftTriggerValue = icurrentValue;
		auto b = FControllerInputState("rift_trigger", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float currentValue = InputComponent->GetAxisValue("LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftTriggerValue = icurrentValue;
		auto b = FControllerInputState("wmr_trigger", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	}
}

void AInputTracker::RightTriggerPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_trigger", 100);
		RightTriggerValue = 100;
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		auto b = FControllerInputState("rift_trigger", 100);
		RightTriggerValue = 100;
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		auto b = FControllerInputState("wmr_trigger", 100);
		RightTriggerValue = 100;
		AppendInputState(true, b);
		break;
	}
	}
}

void AInputTracker::RightTriggerReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float currentValue = InputComponent->GetAxisValue("RightTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		RightTriggerValue = icurrentValue;
		auto b = FControllerInputState("vive_trigger", icurrentValue);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Oculus:
	{
		float currentValue = InputComponent->GetAxisValue("RightTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		RightTriggerValue = icurrentValue;
		auto b = FControllerInputState("rift_trigger", icurrentValue);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float currentValue = InputComponent->GetAxisValue("RightTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		RightTriggerValue = icurrentValue;
		auto b = FControllerInputState("wmr_trigger", icurrentValue);
		AppendInputState(true, b);
		break;
	}
	}
}