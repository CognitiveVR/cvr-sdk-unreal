// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/EnhancedInputTracker.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
UEnhancedInputTracker::UEnhancedInputTracker()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnhancedInputTracker::BeginPlay()
{
	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	Super::BeginPlay();

	GEngine->GetAllLocalPlayerControllers(controllers);

	if (controllers.Num() > 0)
	{
		APlayerController* PlayerController = controllers[0];
		if (PlayerController)
		{
			InitializeEnhancedInput(PlayerController);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerController not found"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No local player controllers found"));
	}
}


// Called every frame
void UEnhancedInputTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!cog->HasStartedSession()) { return; }
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

	CurrentIntervalTime += DeltaTime;
	if (CurrentIntervalTime > Interval)
	{
		if (LeftHand == NULL || RightHand == NULL)
		{
			FindControllers();
		}
		else
		{
			IntervalUpdate();
		}
		CurrentIntervalTime -= Interval;
	}
}

void UEnhancedInputTracker::FindControllers()
{
	for (TObjectIterator<UMotionControllerComponent> Itr; Itr; ++Itr)
	{
		UMotionControllerComponent* Component = *Itr;
		if (Component == NULL) { continue; }
		if (Component->GetWorld() == NULL) { continue; }
		if (!Component->GetWorld()->IsGameWorld()) { continue; }

		if (LeftHand == NULL && Component->GetTrackingSource() == EControllerHand::Left)
		{
			TArray<USceneComponent*> childComponents;
			Component->GetChildrenComponents(true, childComponents);

			//UDynamicObject* dyn = NULL;
			for (auto& Elem : childComponents)
			{
				FString elemName = Elem->GetName();
				FString elemClassName = Elem->GetClass()->GetName();
				UDynamicObject* dyn = Cast<UDynamicObject>(Elem);
				if (dyn != NULL)
				{
					if (!dyn->IsController) { continue; }

					if (dyn->ControllerInputImageName == "oculustouchleft")
					{
						ControllerType = EC3DControllerType::OculusRift;
						LeftHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "windows_mixed_reality_controller_left")
					{
						ControllerType = EC3DControllerType::WindowsMixedReality;
						LeftHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "vivecontroller")
					{
						ControllerType = EC3DControllerType::Vive;
						LeftHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "pico_neo_2_eye_controller_left")
					{
						ControllerType = EC3DControllerType::PicoNeo2;
						LeftHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "pico_neo_3_eye_controller_left")
					{
						ControllerType = EC3DControllerType::PicoNeo3;
						LeftHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "pico_neo_4_enterprise_controller_left")
					{
						ControllerType = EC3DControllerType::PicoNeo4;
						LeftHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "oculusquesttouchleft")
					{
						ControllerType = EC3DControllerType::Quest2;
						LeftHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "quest_plus_touch_left")
					{
						ControllerType = EC3DControllerType::Quest3;
						LeftHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "quest_pro_touch_left")
					{
						ControllerType = EC3DControllerType::QuestPro;
						LeftHand = dyn;
					}
				}
			}
		}
		else if (RightHand == NULL && Component->GetTrackingSource() == EControllerHand::Right)
		{
			TArray<USceneComponent*> childComponents;
			Component->GetChildrenComponents(true, childComponents);

			//UDynamicObject* dyn = NULL;
			for (auto& Elem : childComponents)
			{
				FString elemName = Elem->GetName();
				FString elemClassName = Elem->GetClass()->GetName();
				UDynamicObject* dyn = Cast<UDynamicObject>(Elem);
				if (dyn != NULL)
				{
					if (!dyn->IsController) { continue; }

					if (dyn->ControllerInputImageName == "oculustouchright")
					{
						ControllerType = EC3DControllerType::OculusRift;
						RightHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "windows_mixed_reality_controller_right")
					{
						ControllerType = EC3DControllerType::WindowsMixedReality;
						RightHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "vivecontroller")
					{
						ControllerType = EC3DControllerType::Vive;
						RightHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "pico_neo_2_eye_controller_right")
					{
						ControllerType = EC3DControllerType::PicoNeo2;
						RightHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "pico_neo_3_eye_controller_right")
					{
						ControllerType = EC3DControllerType::PicoNeo3;
						RightHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "pico_neo_4_enterprise_controller_right")
					{
						ControllerType = EC3DControllerType::PicoNeo4;
						RightHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "oculusquesttouchright")
					{
						ControllerType = EC3DControllerType::Quest2;
						RightHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "quest_plus_touch_right")
					{
						ControllerType = EC3DControllerType::Quest3;
						RightHand = dyn;
					}
					else if (dyn->ControllerInputImageName == "quest_pro_touch_right")
					{
						ControllerType = EC3DControllerType::QuestPro;
						RightHand = dyn;
					}
				}
			}
		}
	}
}

void UEnhancedInputTracker::IntervalUpdate()
{
	//check if touchpad/joystick/grip/trigger values have changed
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		//triggers
		float currentValue = LeftTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("vive_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = RightTriggerAxis;
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("vive_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//touchpads
		FVector currentLeftTouchpad = FVector(LeftTouchpadH, LeftTouchpadV, LeftTouchpadAxis.Z);
		if (FVector::Distance(LeftTouchpadAxis, currentLeftTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			LeftTouchpadAxis = currentLeftTouchpad;
			auto b = FControllerInputState("vive_touchpad", LeftTouchpadAxis);
			AppendInputState(false, b);
		}

		FVector currentRightTouchpad = FVector(RightTouchpadH, RightTouchpadV, RightTouchpadAxis.Z);
		if (FVector::Distance(RightTouchpadAxis, currentRightTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			RightTouchpadAxis = currentRightTouchpad;
			auto b = FControllerInputState("vive_touchpad", RightTouchpadAxis);
			AppendInputState(true, b);
		}
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		//triggers
		float currentValue = LeftTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = RightTriggerAxis;
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//grip
		currentValue = LeftGripAxis;
		icurrentValue = (int32)(currentValue * 100);
		if (LeftGripValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_grip", icurrentValue);
			AppendInputState(false, b);
			LeftGripValue = icurrentValue;
		}
		currentValue = RightGripAxis;
		icurrentValue = (int32)(currentValue * 100);
		if (RightGripValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_grip", icurrentValue);
			AppendInputState(true, b);
			RightGripValue = icurrentValue;
		}

		//joysticks
		FVector currentLeftJoystick = FVector(LeftJoystickH, LeftJoystickV, LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("rift_joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(RightJoystickH, RightJoystickV, RightJoystickAxis.Z);
		if (FVector::Distance(RightJoystickAxis, currentRightJoystick) > MinimumVectorChange)
		{
			//write new stuff
			RightJoystickAxis = currentRightJoystick;
			auto b = FControllerInputState("rift_joystick", RightJoystickAxis);
			AppendInputState(true, b);
		}
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		//triggers
		float currentValue = LeftTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = RightTriggerAxis;
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//grip
		currentValue = LeftGripAxis;
		icurrentValue = (int32)(currentValue * 100);
		if (LeftGripValue != icurrentValue)
		{
			auto b = FControllerInputState("grip", icurrentValue);
			AppendInputState(false, b);
			LeftGripValue = icurrentValue;
		}
		currentValue = RightGripAxis;
		icurrentValue = (int32)(currentValue * 100);
		if (RightGripValue != icurrentValue)
		{
			auto b = FControllerInputState("grip", icurrentValue);
			AppendInputState(true, b);
			RightGripValue = icurrentValue;
		}

		//joysticks
		FVector currentLeftJoystick = FVector(LeftJoystickH, LeftJoystickV, LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(RightJoystickH, RightJoystickV, RightJoystickAxis.Z);
		if (FVector::Distance(RightJoystickAxis, currentRightJoystick) > MinimumVectorChange)
		{
			//write new stuff
			RightJoystickAxis = currentRightJoystick;
			auto b = FControllerInputState("joystick", RightJoystickAxis);
			AppendInputState(true, b);
		}
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{

		//triggers
		float currentValue = LeftTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("wmr_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = RightTriggerAxis;
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("wmr_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//touchpads

		FVector currentLeftTouchpad = FVector(LeftTouchpadH, LeftTouchpadV, LeftTouchpadAxis.Z);
		if (FVector::Distance(LeftTouchpadAxis, currentLeftTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			LeftTouchpadAxis = currentLeftTouchpad;
			auto b = FControllerInputState("wmr_touchpad", LeftTouchpadAxis);
			AppendInputState(false, b);
		}

		FVector currentRightTouchpad = FVector(RightTouchpadH, RightTouchpadV, RightTouchpadAxis.Z);
		if (FVector::Distance(RightTouchpadAxis, currentRightTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			RightTouchpadAxis = currentRightTouchpad;
			auto b = FControllerInputState("wmr_touchpad", RightTouchpadAxis);
			AppendInputState(true, b);
		}

		//joysticks
		FVector currentLeftJoystick = FVector(LeftJoystickH, LeftJoystickV, LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("wmr_joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(RightJoystickH, RightJoystickV, RightJoystickAxis.Z);
		if (FVector::Distance(RightJoystickAxis, currentRightJoystick) > MinimumVectorChange)
		{
			//write new stuff
			RightJoystickAxis = currentRightJoystick;
			auto b = FControllerInputState("wmr_joystick", RightJoystickAxis);
			AppendInputState(true, b);
		}
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		//triggers
		float currentValue = LeftTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("pico_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = RightTriggerAxis;
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("pico_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//joysticks
		FVector currentLeftJoystick = FVector(LeftJoystickH, LeftJoystickV, LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("pico_joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(RightJoystickH, RightJoystickV, RightJoystickAxis.Z);
		if (FVector::Distance(RightJoystickAxis, currentRightJoystick) > MinimumVectorChange)
		{
			//write new stuff
			RightJoystickAxis = currentRightJoystick;
			auto b = FControllerInputState("pico_joystick", RightJoystickAxis);
			AppendInputState(true, b);
		}
		break;
	}
	}
}

void UEnhancedInputTracker::AppendInputState(const bool isRight, FControllerInputState& state)
{
	//append or change value
	if (isRight)
	{
		if (RightInputStates.States.Contains(state.AxisName))
		{
			RightInputStates.States[state.AxisName] = state;
		}
		else
		{
			RightInputStates.States.Add(state.AxisName, state);
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

void UEnhancedInputTracker::InitializeEnhancedInput(APlayerController* PlayerController)
{
	// Create and configure input actions

	C3D_LeftMenuButton = NewObject<UInputAction>(this);
	C3D_LeftMenuButton->ValueType = EInputActionValueType::Boolean;
	C3D_LeftMenuButton->bConsumeInput = false;
	C3D_LeftMenuButton->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_RightMenuButton = NewObject<UInputAction>(this);
	C3D_RightMenuButton->ValueType = EInputActionValueType::Boolean;
	C3D_RightMenuButton->bConsumeInput = false;
	C3D_RightMenuButton->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_RightTrackpadTouch = NewObject<UInputAction>(this);
	C3D_RightTrackpadTouch->ValueType = EInputActionValueType::Boolean;
	C3D_RightTrackpadTouch->bConsumeInput = false;
	C3D_RightTrackpadTouch->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_LeftTrackpadTouch = NewObject<UInputAction>(this);
	C3D_LeftTrackpadTouch->ValueType = EInputActionValueType::Boolean;
	C3D_LeftTrackpadTouch->bConsumeInput = false;
	C3D_LeftTrackpadTouch->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_RightTrackpadClick = NewObject<UInputAction>(this);
	C3D_RightTrackpadClick->ValueType = EInputActionValueType::Boolean;
	C3D_RightTrackpadClick->bConsumeInput = false;
	C3D_RightTrackpadClick->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_LeftTrackpadClick = NewObject<UInputAction>(this);
	C3D_LeftTrackpadClick->ValueType = EInputActionValueType::Boolean;
	C3D_LeftTrackpadClick->bConsumeInput = false;
	C3D_LeftTrackpadClick->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_LeftFaceButtonOne = NewObject<UInputAction>(this);
	C3D_LeftFaceButtonOne->ValueType = EInputActionValueType::Boolean;
	C3D_LeftFaceButtonOne->bConsumeInput = false;
	C3D_LeftFaceButtonOne->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_RightFaceButtonOne = NewObject<UInputAction>(this);
	C3D_RightFaceButtonOne->ValueType = EInputActionValueType::Boolean;
	C3D_RightFaceButtonOne->bConsumeInput = false;
	C3D_RightFaceButtonOne->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_LeftFaceButtonTwo = NewObject<UInputAction>(this);
	C3D_LeftFaceButtonTwo->ValueType = EInputActionValueType::Boolean;
	C3D_LeftFaceButtonTwo->bConsumeInput = false;
	C3D_LeftFaceButtonTwo->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_RightFaceButtonTwo = NewObject<UInputAction>(this);
	C3D_RightFaceButtonTwo->ValueType = EInputActionValueType::Boolean;
	C3D_RightFaceButtonTwo->bConsumeInput = false;
	C3D_RightFaceButtonTwo->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_LeftGrip = NewObject<UInputAction>(this);
	C3D_LeftGrip->ValueType = EInputActionValueType::Boolean;
	C3D_LeftGrip->bConsumeInput = false;
	C3D_LeftGrip->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_RightGrip = NewObject<UInputAction>(this);
	C3D_RightGrip->ValueType = EInputActionValueType::Boolean;
	C3D_RightGrip->bConsumeInput = false;
	C3D_RightGrip->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_LeftTrigger = NewObject<UInputAction>(this);
	C3D_LeftTrigger->ValueType = EInputActionValueType::Boolean;
	C3D_LeftTrigger->bConsumeInput = false;
	C3D_LeftTrigger->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_RightTrigger = NewObject<UInputAction>(this);
	C3D_RightTrigger->ValueType = EInputActionValueType::Boolean;
	C3D_RightTrigger->bConsumeInput = false;
	C3D_RightTrigger->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_LeftJoystick = NewObject<UInputAction>(this);
	C3D_LeftJoystick->ValueType = EInputActionValueType::Boolean;
	C3D_LeftJoystick->bConsumeInput = false;
	C3D_LeftJoystick->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	C3D_RightJoystick = NewObject<UInputAction>(this);
	C3D_RightJoystick->ValueType = EInputActionValueType::Boolean;
	C3D_RightJoystick->bConsumeInput = false;
	C3D_RightJoystick->Triggers.Add(NewObject<UInputTriggerPressed>(this));

	// Input Axis

	C3D_LeftJoystickAxis = NewObject<UInputAction>(this);
	C3D_LeftJoystickAxis->ValueType = EInputActionValueType::Axis2D;
	C3D_LeftJoystickAxis->bConsumeInput = false;

	C3D_LeftJoystickX = NewObject<UInputAction>(this);
	C3D_LeftJoystickX->ValueType = EInputActionValueType::Axis1D;
	C3D_LeftJoystickX->bConsumeInput = false;

	C3D_LeftJoystickY = NewObject<UInputAction>(this);
	C3D_LeftJoystickY->ValueType = EInputActionValueType::Axis1D;
	C3D_LeftJoystickY->bConsumeInput = false;

	C3D_RightJoystickAxis = NewObject<UInputAction>(this);
	C3D_RightJoystickAxis->ValueType = EInputActionValueType::Axis2D;
	C3D_RightJoystickAxis->bConsumeInput = false;

	C3D_RightJoystickX = NewObject<UInputAction>(this);
	C3D_RightJoystickX->ValueType = EInputActionValueType::Axis1D;
	C3D_RightJoystickX->bConsumeInput = false;

	C3D_RightJoystickY = NewObject<UInputAction>(this);
	C3D_RightJoystickY->ValueType = EInputActionValueType::Axis1D;
	C3D_RightJoystickY->bConsumeInput = false;

	C3D_RightTrackpadAxis = NewObject<UInputAction>(this);
	C3D_RightTrackpadAxis->ValueType = EInputActionValueType::Axis2D;
	C3D_RightTrackpadAxis->bConsumeInput = false;

	C3D_RightTrackpadX = NewObject<UInputAction>(this);
	C3D_RightTrackpadX->ValueType = EInputActionValueType::Axis1D;
	C3D_RightTrackpadX->bConsumeInput = false;

	C3D_RightTrackpadY = NewObject<UInputAction>(this);
	C3D_RightTrackpadY->ValueType = EInputActionValueType::Axis1D;
	C3D_RightTrackpadY->bConsumeInput = false;

	C3D_LeftTrackpadAxis = NewObject<UInputAction>(this);
	C3D_LeftTrackpadAxis->ValueType = EInputActionValueType::Axis2D;
	C3D_LeftTrackpadAxis->bConsumeInput = false;

	C3D_LeftTrackpadX = NewObject<UInputAction>(this);
	C3D_LeftTrackpadX->ValueType = EInputActionValueType::Axis1D;
	C3D_LeftTrackpadX->bConsumeInput = false;

	C3D_LeftTrackpadY = NewObject<UInputAction>(this);
	C3D_LeftTrackpadY->ValueType = EInputActionValueType::Axis1D;
	C3D_LeftTrackpadY->bConsumeInput = false;

	C3D_LeftGripAxis = NewObject<UInputAction>(this);
	C3D_LeftGripAxis->ValueType = EInputActionValueType::Axis1D;
	C3D_LeftGripAxis->bConsumeInput = false;

	C3D_RightGripAxis = NewObject<UInputAction>(this);
	C3D_RightGripAxis->ValueType = EInputActionValueType::Axis1D;
	C3D_RightGripAxis->bConsumeInput = false;

	C3D_LeftTriggerAxis = NewObject<UInputAction>(this);
	C3D_LeftTriggerAxis->ValueType = EInputActionValueType::Axis1D;
	C3D_LeftTriggerAxis->bConsumeInput = false;

	C3D_RightTriggerAxis = NewObject<UInputAction>(this);
	C3D_RightTriggerAxis->ValueType = EInputActionValueType::Axis1D;
	C3D_RightTriggerAxis->bConsumeInput = false;

	// Create and configure input mapping context
	C3DInputMappingContext = NewObject<UInputMappingContext>(this);

	// Add mappings to the context
	//oculus controller
	//oculus actions
	C3DInputMappingContext->MapKey(C3D_LeftMenuButton, EKeys::OculusTouch_Left_Menu_Click);
	C3DInputMappingContext->MapKey(C3D_LeftFaceButtonOne, EKeys::OculusTouch_Left_X_Click);
	C3DInputMappingContext->MapKey(C3D_RightFaceButtonOne, EKeys::OculusTouch_Right_A_Click);
	C3DInputMappingContext->MapKey(C3D_LeftFaceButtonTwo, EKeys::OculusTouch_Left_Y_Click);
	C3DInputMappingContext->MapKey(C3D_RightFaceButtonTwo, EKeys::OculusTouch_Right_B_Click);
	C3DInputMappingContext->MapKey(C3D_LeftGrip, EKeys::OculusTouch_Left_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_RightGrip, EKeys::OculusTouch_Right_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_LeftTrigger, EKeys::OculusTouch_Left_Trigger_Click);
	C3DInputMappingContext->MapKey(C3D_RightTrigger, EKeys::OculusTouch_Right_Trigger_Click);
	C3DInputMappingContext->MapKey(C3D_LeftJoystick, EKeys::OculusTouch_Left_Thumbstick_Click);
	C3DInputMappingContext->MapKey(C3D_RightJoystick, EKeys::OculusTouch_Right_Thumbstick_Click);
	//oculus axis
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	C3DInputMappingContext->MapKey(C3D_LeftJoystickAxis, EKeys::OculusTouch_Left_Thumbstick_2D);
	C3DInputMappingContext->MapKey(C3D_RightJoystickAxis, EKeys::OculusTouch_Right_Thumbstick_2D);
#endif
	//
	C3DInputMappingContext->MapKey(C3D_LeftJoystickX, EKeys::OculusTouch_Left_Thumbstick_X);
	C3DInputMappingContext->MapKey(C3D_LeftJoystickY, EKeys::OculusTouch_Left_Thumbstick_Y);
	C3DInputMappingContext->MapKey(C3D_RightJoystickY, EKeys::OculusTouch_Right_Thumbstick_Y);
	C3DInputMappingContext->MapKey(C3D_RightJoystickY, EKeys::OculusTouch_Right_Thumbstick_Y);
	//
	C3DInputMappingContext->MapKey(C3D_LeftGripAxis, EKeys::OculusTouch_Left_Grip_Axis);
	C3DInputMappingContext->MapKey(C3D_RightGripAxis, EKeys::OculusTouch_Right_Grip_Axis);
	C3DInputMappingContext->MapKey(C3D_LeftTriggerAxis, EKeys::OculusTouch_Left_Trigger_Axis);
	C3DInputMappingContext->MapKey(C3D_RightTriggerAxis, EKeys::OculusTouch_Right_Trigger_Axis);

	//vive
	//vive actions
	C3DInputMappingContext->MapKey(C3D_LeftMenuButton, EKeys::Vive_Left_Menu_Click);
	C3DInputMappingContext->MapKey(C3D_RightMenuButton, EKeys::Vive_Right_Menu_Click);
	C3DInputMappingContext->MapKey(C3D_LeftGrip, EKeys::Vive_Left_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_RightGrip, EKeys::Vive_Right_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_LeftTrigger, EKeys::Vive_Left_Trigger_Click);
	C3DInputMappingContext->MapKey(C3D_RightTrigger, EKeys::Vive_Right_Trigger_Click);
	//trackpad/touchpad for vive
	C3DInputMappingContext->MapKey(C3D_RightTrackpadTouch, EKeys::Vive_Right_Trackpad_Touch);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadTouch, EKeys::Vive_Left_Trackpad_Touch);
	C3DInputMappingContext->MapKey(C3D_RightTrackpadClick, EKeys::Vive_Right_Trackpad_Click);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadClick, EKeys::Vive_Left_Trackpad_Click);

	//vive axis
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	C3DInputMappingContext->MapKey(C3D_RightTrackpadAxis, EKeys::Vive_Right_Trackpad_2D);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadAxis, EKeys::Vive_Left_Trackpad_2D);
#endif
	//
	C3DInputMappingContext->MapKey(C3D_RightTrackpadX, EKeys::Vive_Right_Trackpad_X);
	C3DInputMappingContext->MapKey(C3D_RightTrackpadY, EKeys::Vive_Right_Trackpad_Y);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadX, EKeys::Vive_Left_Trackpad_X);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadY, EKeys::Vive_Left_Trackpad_Y);
	//

	C3DInputMappingContext->MapKey(C3D_LeftGripAxis, EKeys::Vive_Left_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_RightGripAxis, EKeys::Vive_Right_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_LeftTriggerAxis, EKeys::Vive_Left_Trigger_Axis);
	C3DInputMappingContext->MapKey(C3D_RightTriggerAxis, EKeys::Vive_Right_Trigger_Axis);

	//windows
	//windows actions
	C3DInputMappingContext->MapKey(C3D_LeftMenuButton, EKeys::MixedReality_Left_Menu_Click);
	C3DInputMappingContext->MapKey(C3D_RightMenuButton, EKeys::MixedReality_Right_Menu_Click);
	C3DInputMappingContext->MapKey(C3D_LeftFaceButtonOne, EKeys::MixedReality_Left_Trackpad_Up);
	C3DInputMappingContext->MapKey(C3D_RightFaceButtonOne, EKeys::MixedReality_Right_Trackpad_Up);
	C3DInputMappingContext->MapKey(C3D_LeftFaceButtonTwo, EKeys::MixedReality_Left_Trackpad_Right);
	C3DInputMappingContext->MapKey(C3D_RightFaceButtonTwo, EKeys::MixedReality_Right_Trackpad_Right);
	C3DInputMappingContext->MapKey(C3D_LeftGrip, EKeys::MixedReality_Left_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_RightGrip, EKeys::MixedReality_Right_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_LeftTrigger, EKeys::MixedReality_Left_Trigger_Click);
	C3DInputMappingContext->MapKey(C3D_RightTrigger, EKeys::MixedReality_Right_Trigger_Click);
	C3DInputMappingContext->MapKey(C3D_LeftJoystick, EKeys::MixedReality_Left_Thumbstick_Click);
	C3DInputMappingContext->MapKey(C3D_RightJoystick, EKeys::MixedReality_Right_Thumbstick_Click);
	//touchpad touch
	C3DInputMappingContext->MapKey(C3D_RightTrackpadTouch, EKeys::MixedReality_Right_Trackpad_Touch);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadTouch, EKeys::MixedReality_Left_Trackpad_Touch);
	C3DInputMappingContext->MapKey(C3D_RightTrackpadClick, EKeys::MixedReality_Right_Trackpad_Click);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadClick, EKeys::MixedReality_Left_Trackpad_Click);

	//trackpad axis
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	C3DInputMappingContext->MapKey(C3D_RightTrackpadAxis, EKeys::MixedReality_Right_Trackpad_2D);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadAxis, EKeys::MixedReality_Left_Trackpad_2D);
#endif
	//
	C3DInputMappingContext->MapKey(C3D_RightTrackpadX, EKeys::MixedReality_Right_Trackpad_X);
	C3DInputMappingContext->MapKey(C3D_RightTrackpadY, EKeys::MixedReality_Right_Trackpad_Y);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadX, EKeys::MixedReality_Left_Trackpad_X);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadY, EKeys::MixedReality_Left_Trackpad_Y);
	//

	//windows axis
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	C3DInputMappingContext->MapKey(C3D_LeftJoystickAxis, EKeys::MixedReality_Left_Thumbstick_2D);
	C3DInputMappingContext->MapKey(C3D_RightJoystickAxis, EKeys::MixedReality_Right_Thumbstick_2D);
#endif
	//
	C3DInputMappingContext->MapKey(C3D_LeftJoystickX, EKeys::MixedReality_Left_Thumbstick_X);
	C3DInputMappingContext->MapKey(C3D_LeftJoystickY, EKeys::MixedReality_Left_Thumbstick_Y);
	C3DInputMappingContext->MapKey(C3D_RightJoystickX, EKeys::MixedReality_Right_Thumbstick_X);
	C3DInputMappingContext->MapKey(C3D_RightJoystickY, EKeys::MixedReality_Right_Thumbstick_Y);
	//
	C3DInputMappingContext->MapKey(C3D_LeftGripAxis, EKeys::MixedReality_Left_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_RightGripAxis, EKeys::MixedReality_Right_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_LeftTriggerAxis, EKeys::MixedReality_Left_Trigger_Axis);
	C3DInputMappingContext->MapKey(C3D_RightTriggerAxis, EKeys::MixedReality_Right_Trigger_Axis);
	//touchpad axis

	//valve index
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 6)
	C3DInputMappingContext->MapKey(C3D_LeftMenuButton, EKeys::ValveIndex_Left_System_Click);
	C3DInputMappingContext->MapKey(C3D_RightMenuButton, EKeys::ValveIndex_Right_System_Click);
#endif
	C3DInputMappingContext->MapKey(C3D_LeftFaceButtonOne, EKeys::ValveIndex_Left_A_Click);
	C3DInputMappingContext->MapKey(C3D_RightFaceButtonOne, EKeys::ValveIndex_Right_A_Click);
	C3DInputMappingContext->MapKey(C3D_LeftFaceButtonTwo, EKeys::ValveIndex_Left_B_Click);
	C3DInputMappingContext->MapKey(C3D_RightFaceButtonTwo, EKeys::ValveIndex_Right_B_Click);
	C3DInputMappingContext->MapKey(C3D_LeftGrip, EKeys::ValveIndex_Left_Grip_Force);
	C3DInputMappingContext->MapKey(C3D_RightGrip, EKeys::ValveIndex_Right_Grip_Force);
	C3DInputMappingContext->MapKey(C3D_LeftTrigger, EKeys::ValveIndex_Left_Trigger_Click);
	C3DInputMappingContext->MapKey(C3D_RightTrigger, EKeys::ValveIndex_Right_Trigger_Click);
	C3DInputMappingContext->MapKey(C3D_LeftJoystick, EKeys::ValveIndex_Left_Thumbstick_Click);
	C3DInputMappingContext->MapKey(C3D_RightJoystick, EKeys::ValveIndex_Right_Thumbstick_Click);

	//touchpad touch
	C3DInputMappingContext->MapKey(C3D_RightTrackpadTouch, EKeys::ValveIndex_Right_Trackpad_Touch);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadTouch, EKeys::ValveIndex_Left_Trackpad_Touch);
	C3DInputMappingContext->MapKey(C3D_RightTrackpadClick, EKeys::ValveIndex_Right_Trackpad_Force);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadClick, EKeys::ValveIndex_Left_Trackpad_Force);

	//trackpad axis
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	C3DInputMappingContext->MapKey(C3D_RightTrackpadAxis, EKeys::ValveIndex_Right_Trackpad_2D);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadAxis, EKeys::ValveIndex_Left_Trackpad_2D);
#endif
	//
	C3DInputMappingContext->MapKey(C3D_RightTrackpadX, EKeys::ValveIndex_Right_Trackpad_X);
	C3DInputMappingContext->MapKey(C3D_RightTrackpadY, EKeys::ValveIndex_Right_Trackpad_Y);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadX, EKeys::ValveIndex_Left_Trackpad_X);
	C3DInputMappingContext->MapKey(C3D_LeftTrackpadY, EKeys::ValveIndex_Left_Trackpad_Y);
	//
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	C3DInputMappingContext->MapKey(C3D_LeftJoystickAxis, EKeys::ValveIndex_Left_Thumbstick_2D);
	C3DInputMappingContext->MapKey(C3D_RightJoystickAxis, EKeys::ValveIndex_Right_Thumbstick_2D);
#endif
	//
	C3DInputMappingContext->MapKey(C3D_LeftJoystickX, EKeys::ValveIndex_Left_Thumbstick_X);
	C3DInputMappingContext->MapKey(C3D_LeftJoystickY, EKeys::ValveIndex_Left_Thumbstick_Y);
	C3DInputMappingContext->MapKey(C3D_RightJoystickX, EKeys::ValveIndex_Right_Thumbstick_X);
	C3DInputMappingContext->MapKey(C3D_RightJoystickY, EKeys::ValveIndex_Right_Thumbstick_Y);
	//
	C3DInputMappingContext->MapKey(C3D_LeftGripAxis, EKeys::ValveIndex_Left_Grip_Axis);
	C3DInputMappingContext->MapKey(C3D_RightGripAxis, EKeys::ValveIndex_Right_Grip_Axis);
	C3DInputMappingContext->MapKey(C3D_LeftTriggerAxis, EKeys::ValveIndex_Left_Trigger_Axis);
	C3DInputMappingContext->MapKey(C3D_RightTriggerAxis, EKeys::ValveIndex_Right_Trigger_Axis);

	//pico

#ifdef INCLUDE_PICO_PLUGIN

	C3DInputMappingContext->MapKey(C3D_LeftMenuButton, FPICOTouchKey::PICOTouch_Left_Menu_Click);
	C3DInputMappingContext->MapKey(C3D_LeftFaceButtonOne, FPICOTouchKey::PICOTouch_Left_X_Click);
	C3DInputMappingContext->MapKey(C3D_RightFaceButtonOne, FPICOTouchKey::PICOTouch_Right_A_Click);
	C3DInputMappingContext->MapKey(C3D_LeftFaceButtonTwo, FPICOTouchKey::PICOTouch_Left_Y_Click);
	C3DInputMappingContext->MapKey(C3D_RightFaceButtonTwo, FPICOTouchKey::PICOTouch_Right_B_Click);
	C3DInputMappingContext->MapKey(C3D_LeftGrip, FPICOTouchKey::PICOTouch_Left_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_RightGrip, FPICOTouchKey::PICOTouch_Right_Grip_Click);
	C3DInputMappingContext->MapKey(C3D_LeftTrigger, FPICOTouchKey::PICOTouch_Left_Trigger_Click);
	C3DInputMappingContext->MapKey(C3D_RightTrigger, FPICOTouchKey::PICOTouch_Right_Trigger_Click);
	C3DInputMappingContext->MapKey(C3D_LeftJoystick, FPICOTouchKey::PICOTouch_Left_Thumbstick_Click);
	C3DInputMappingContext->MapKey(C3D_RightJoystick, FPICOTouchKey::PICOTouch_Right_Thumbstick_Click);

	C3DInputMappingContext->MapKey(C3D_LeftJoystickX, FPICOTouchKey::PICOTouch_Left_Thumbstick_X);
	C3DInputMappingContext->MapKey(C3D_LeftJoystickY, FPICOTouchKey::PICOTouch_Left_Thumbstick_Y);
	C3DInputMappingContext->MapKey(C3D_RightJoystickX, FPICOTouchKey::PICOTouch_Right_Thumbstick_X);
	C3DInputMappingContext->MapKey(C3D_RightJoystickY, FPICOTouchKey::PICOTouch_Right_Thumbstick_Y);

	C3DInputMappingContext->MapKey(C3D_LeftGripAxis, FPICOTouchKey::PICOTouch_Left_Grip_Axis);
	C3DInputMappingContext->MapKey(C3D_RightGripAxis, FPICOTouchKey::PICOTouch_Right_Grip_Axis);
	C3DInputMappingContext->MapKey(C3D_LeftTriggerAxis, FPICOTouchKey::PICOTouch_Left_Trigger_Axis);
	C3DInputMappingContext->MapKey(C3D_RightTriggerAxis, FPICOTouchKey::PICOTouch_Right_Trigger_Axis);

#endif

	// Add the input mapping context to the local player subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(C3DInputMappingContext, 99);
		//set to something high like 99 and make it not consume lower priority inputs
	}

	// Bind actions to functions
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (EnhancedInputComponent != nullptr)
	{
		//actions
		//menu buttons
		EnhancedInputComponent->BindAction(C3D_LeftMenuButton, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftMenuButtonPressed);
		EnhancedInputComponent->BindAction(C3D_LeftMenuButton, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftMenuButtonReleased);
		EnhancedInputComponent->BindAction(C3D_RightMenuButton, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightMenuButtonPressed);
		EnhancedInputComponent->BindAction(C3D_RightMenuButton, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightMenuButtonReleased);

		//trackpad touch
		EnhancedInputComponent->BindAction(C3D_RightTrackpadTouch, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTouchpadTouched);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadTouch, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightTouchpadReleased);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadTouch, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTouchpadTouched);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadTouch, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftTouchpadReleased);
		//trackpad click
		EnhancedInputComponent->BindAction(C3D_RightTrackpadClick, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTouchpadPressed);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadClick, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightTouchpadPressRelease);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadClick, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTouchpadPressed);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadClick, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftTouchpadPressRelease);

		//face buttons
		EnhancedInputComponent->BindAction(C3D_LeftFaceButtonOne, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftFaceButtonOnePressed);
		EnhancedInputComponent->BindAction(C3D_LeftFaceButtonOne, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftFaceButtonOneReleased);
		EnhancedInputComponent->BindAction(C3D_RightFaceButtonOne, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightFaceButtonOnePressed);
		EnhancedInputComponent->BindAction(C3D_RightFaceButtonOne, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightFaceButtonOneReleased);
		EnhancedInputComponent->BindAction(C3D_LeftFaceButtonTwo, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftFaceButtonTwoPressed);
		EnhancedInputComponent->BindAction(C3D_LeftFaceButtonTwo, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftFaceButtonTwoReleased);
		EnhancedInputComponent->BindAction(C3D_RightFaceButtonTwo, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightFaceButtonTwoPressed);
		EnhancedInputComponent->BindAction(C3D_RightFaceButtonTwo, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightFaceButtonTwoReleased);
		//grip
		EnhancedInputComponent->BindAction(C3D_LeftGrip, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftGripPressed);
		EnhancedInputComponent->BindAction(C3D_LeftGrip, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftGripReleased);
		EnhancedInputComponent->BindAction(C3D_RightGrip, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightGripPressed);
		EnhancedInputComponent->BindAction(C3D_RightGrip, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightGripReleased);
		//triggers
		EnhancedInputComponent->BindAction(C3D_LeftTrigger, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTriggerPressed);
		EnhancedInputComponent->BindAction(C3D_LeftTrigger, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftTriggerReleased);
		EnhancedInputComponent->BindAction(C3D_RightTrigger, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTriggerPressed);
		EnhancedInputComponent->BindAction(C3D_RightTrigger, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightTriggerReleased);
		//joystick
		EnhancedInputComponent->BindAction(C3D_LeftJoystick, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftJoystickPressed);
		EnhancedInputComponent->BindAction(C3D_LeftJoystick, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftJoystickReleased);
		EnhancedInputComponent->BindAction(C3D_RightJoystick, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightJoystickPressed);
		EnhancedInputComponent->BindAction(C3D_RightJoystick, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightJoystickReleased);

		//axis
		//must use BindAction for these under the new EnhancedInputComponent system
		//joystick axis
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		EnhancedInputComponent->BindAction(C3D_LeftJoystickAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftJoystickPressed);
		EnhancedInputComponent->BindAction(C3D_LeftJoystickAxis, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftJoystickReleased);
		EnhancedInputComponent->BindAction(C3D_RightJoystickAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightJoystickPressed);
		EnhancedInputComponent->BindAction(C3D_RightJoystickAxis, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightJoystickReleased);

		//touchpad/trackpad axis
		EnhancedInputComponent->BindAction(C3D_RightTrackpadAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTouchpadTouched);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadAxis, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightTouchpadReleased);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTouchpadTouched);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadAxis, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftTouchpadReleased);

		EnhancedInputComponent->BindAction(C3D_RightTrackpadAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTouchpadPressed);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadAxis, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightTouchpadPressRelease);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTouchpadPressed);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadAxis, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftTouchpadPressRelease);

#else
		// x and y touchpad/trackpad for versions that dont have 2d axis bindings
		EnhancedInputComponent->BindAction(C3D_RightTrackpadX, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTouchpadXTouched);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadX, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightTouchpadXReleased);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadY, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTouchpadYTouched);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadY, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightTouchpadYReleased);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadX, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTouchpadXTouched);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadX, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftTouchpadXReleased);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadY, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTouchpadYTouched);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadY, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftTouchpadYReleased);

		EnhancedInputComponent->BindAction(C3D_RightTrackpadX, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTouchpadXPressed);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadX, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightTouchpadXPressRelease);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadY, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTouchpadYPressed);
		EnhancedInputComponent->BindAction(C3D_RightTrackpadY, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightTouchpadYPressRelease);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadX, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTouchpadXPressed);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadX, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftTouchpadXPressRelease);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadY, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTouchpadYPressed);
		EnhancedInputComponent->BindAction(C3D_LeftTrackpadY, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftTouchpadYPressRelease);

		// x and y joysick movements for versions that dont have 2d axis bindings
		EnhancedInputComponent->BindAction(C3D_LeftJoystickX, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftJoystickXPressed);
		EnhancedInputComponent->BindAction(C3D_LeftJoystickY, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftJoystickYPressed);
		EnhancedInputComponent->BindAction(C3D_LeftJoystickX, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftJoystickXReleased);
		EnhancedInputComponent->BindAction(C3D_LeftJoystickY, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftJoystickYReleased);
		EnhancedInputComponent->BindAction(C3D_RightJoystickX, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightJoystickXPressed);
		EnhancedInputComponent->BindAction(C3D_RightJoystickY, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightJoystickYPressed);
		EnhancedInputComponent->BindAction(C3D_RightJoystickX, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightJoystickXReleased);
		EnhancedInputComponent->BindAction(C3D_RightJoystickY, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightJoystickYReleased);
		//
#endif
		//x and y joystick movement for pico
#ifdef INCLUDE_PICO_PLUGIN

		EnhancedInputComponent->BindAction(C3D_LeftJoystickX, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftJoystickXPressed);
		EnhancedInputComponent->BindAction(C3D_LeftJoystickY, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftJoystickYPressed);
		EnhancedInputComponent->BindAction(C3D_LeftJoystickX, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftJoystickXReleased);
		EnhancedInputComponent->BindAction(C3D_LeftJoystickY, ETriggerEvent::Completed, this, &UEnhancedInputTracker::LeftJoystickYReleased);
		EnhancedInputComponent->BindAction(C3D_RightJoystickX, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightJoystickXPressed);
		EnhancedInputComponent->BindAction(C3D_RightJoystickY, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightJoystickYPressed);
		EnhancedInputComponent->BindAction(C3D_RightJoystickX, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightJoystickXReleased);
		EnhancedInputComponent->BindAction(C3D_RightJoystickY, ETriggerEvent::Completed, this, &UEnhancedInputTracker::RightJoystickYReleased);

#endif
		//grip axis
		EnhancedInputComponent->BindAction(C3D_LeftGripAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftGripReleased);
		EnhancedInputComponent->BindAction(C3D_RightGripAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightGripReleased);
		//trigger axis
		EnhancedInputComponent->BindAction(C3D_LeftTriggerAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::LeftTriggerReleased);
		EnhancedInputComponent->BindAction(C3D_RightTriggerAxis, ETriggerEvent::Triggered, this, &UEnhancedInputTracker::RightTriggerReleased);

	}

	//listen for event or find controller immediately if session already started
	if (cog->HasStartedSession())
	{
		FindControllers();
	}
	cog->OnSessionBegin.AddDynamic(this, &UEnhancedInputTracker::FindControllers);

}

void UEnhancedInputTracker::LeftFaceButtonOnePressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_xbtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("xbtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_xbtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void UEnhancedInputTracker::LeftFaceButtonOneReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_xbtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("xbtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_xbtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void UEnhancedInputTracker::LeftFaceButtonTwoPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_ybtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("ybtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_ybtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void UEnhancedInputTracker::LeftFaceButtonTwoReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_ybtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("ybtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_ybtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void UEnhancedInputTracker::LeftMenuButtonPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_menubtn", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_start", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("menu", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_menubtn", 100);
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

void UEnhancedInputTracker::LeftMenuButtonReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_menubtn", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_start", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("menu", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_menubtn", 0);
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

void UEnhancedInputTracker::LeftJoystickPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftJoystickH = ValueVector.X;
			LeftJoystickV = ValueVector.Y;
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftJoystickH = ValueVector.X;
			LeftJoystickV = ValueVector.Y;
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftJoystickH = ValueVector.X;
			LeftJoystickV = ValueVector.Y;
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftJoystickH = ValueVector.X;
			LeftJoystickV = ValueVector.Y;
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);

		break;
	}
	}
}

void UEnhancedInputTracker::LeftJoystickReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftJoystickH = ValueVector.X;
			LeftJoystickV = ValueVector.Y;
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftJoystickH = ValueVector.X;
			LeftJoystickV = ValueVector.Y;
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftJoystickH = ValueVector.X;
			LeftJoystickV = ValueVector.Y;
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftJoystickH = ValueVector.X;
			LeftJoystickV = ValueVector.Y;
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);

		break;
	}
	}
}

void UEnhancedInputTracker::LeftJoystickXPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickH = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickH = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickH = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickH = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);

		break;
	}
	}
}

void UEnhancedInputTracker::LeftJoystickYPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickV = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickV = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickV = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickV = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);

		break;
	}
	}
}

void UEnhancedInputTracker::LeftJoystickXReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickH = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickH = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}	
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickH = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickH = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);

		break;
	}
	}
}

void UEnhancedInputTracker::LeftJoystickYReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickV = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickV = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickV = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);

		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftJoystickV = Value.Get<float>();
		}
		float x = LeftJoystickH;
		float y = LeftJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);

		break;
	}
	}
}

void UEnhancedInputTracker::LeftGripPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_grip", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_grip", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("grip", 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_grip", 100);
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

void UEnhancedInputTracker::LeftGripReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_grip", 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftGripAxis = Value.Get<float>();
		}
		float currentValue = LeftGripAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftGripValue = icurrentValue;
		auto b = FControllerInputState("rift_grip", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftGripAxis = Value.Get<float>();
		}
		float currentValue = LeftGripAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftGripValue = icurrentValue;
		auto b = FControllerInputState("grip", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_grip", 0);
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

void UEnhancedInputTracker::LeftTouchpadReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftTouchpadH = ValueVector.X;
			LeftTouchpadV = ValueVector.Y;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftTouchpadH = ValueVector.X;
			LeftTouchpadV = ValueVector.Y;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadTouched(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftTouchpadH = ValueVector.X;
			LeftTouchpadV = ValueVector.Y;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftTouchpadH = ValueVector.X;
			LeftTouchpadV = ValueVector.Y;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftTouchpadH = ValueVector.X;
			LeftTouchpadV = ValueVector.Y;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftTouchpadH = ValueVector.X;
			LeftTouchpadV = ValueVector.Y;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadPressRelease(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftTouchpadH = ValueVector.X;
			LeftTouchpadV = ValueVector.Y;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			LeftTouchpadH = ValueVector.X;
			LeftTouchpadV = ValueVector.Y;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadXReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadH = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadH = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadXTouched(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadH = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadH = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadXPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadH = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadH = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadXPressRelease(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadH = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadH = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadYReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadV = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadV = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadYTouched(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadV = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadV = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadYPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadV = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadV = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTouchpadYPressRelease(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadV = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			LeftTouchpadV = ValueVector;
		}
		float x = LeftTouchpadH;
		float y = LeftTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTriggerReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftTriggerAxis = Value.Get<float>();
		}
		float currentValue = LeftTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftTriggerValue = icurrentValue;
		auto b = FControllerInputState("vive_trigger", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftTriggerAxis = Value.Get<float>();
		}
		float currentValue = LeftTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftTriggerValue = icurrentValue;
		auto b = FControllerInputState("rift_trigger", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftTriggerAxis = Value.Get<float>();
		}
		float currentValue = LeftTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftTriggerValue = icurrentValue;
		auto b = FControllerInputState("trigger", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_trigger", 0);
		LeftTriggerValue = 0;
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			LeftTriggerAxis = Value.Get<float>();
		}
		float currentValue = LeftTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftTriggerValue = icurrentValue;
		auto b = FControllerInputState("wmr_trigger", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	}
}

void UEnhancedInputTracker::LeftTriggerPressed()
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
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_trigger", 100);
		LeftTriggerValue = 100;
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("trigger", 100);
		LeftTriggerValue = 100;
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_trigger", 100);
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

void UEnhancedInputTracker::RightFaceButtonOnePressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_abtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("abtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_abtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void UEnhancedInputTracker::RightFaceButtonOneReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_abtn", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("abtn", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_abtn", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void UEnhancedInputTracker::RightFaceButtonTwoPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_bbtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("bbtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_bbtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void UEnhancedInputTracker::RightFaceButtonTwoReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_bbtn", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("bbtn", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_bbtn", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
		break;
	}
}

void UEnhancedInputTracker::RightMenuButtonPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_menubtn", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_start", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("menu", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_menubtn", 100);
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

void UEnhancedInputTracker::RightMenuButtonReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_menubtn", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_start", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("menu", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_menubtn", 0);
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

void UEnhancedInputTracker::RightJoystickPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightJoystickH = ValueVector.X;
			RightJoystickV = ValueVector.Y;
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightJoystickH = ValueVector.X;
			RightJoystickV = ValueVector.Y;
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightJoystickH = ValueVector.X;
			RightJoystickV = ValueVector.Y;
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightJoystickH = ValueVector.X;
			RightJoystickV = ValueVector.Y;
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightJoystickReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightJoystickH = ValueVector.X;
			RightJoystickV = ValueVector.Y;
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightJoystickH = ValueVector.X;
			RightJoystickV = ValueVector.Y;
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightJoystickH = ValueVector.X;
			RightJoystickV = ValueVector.Y;
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightJoystickH = ValueVector.X;
			RightJoystickV = ValueVector.Y;
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightJoystickXPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickH = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickH = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickH = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickH = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightJoystickYPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickV = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickV = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickV = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickV = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightJoystickXReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickH = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickH = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickH = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickH = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightJoystickYReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickV = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickV = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickV = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightJoystickV = Value.Get<float>();
		}
		float x = RightJoystickH;
		float y = RightJoystickV;
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightGripPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_grip", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_grip", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("grip", 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_grip", 100);
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

void UEnhancedInputTracker::RightGripReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		auto b = FControllerInputState("vive_grip", 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightGripAxis = Value.Get<float>();
		}
		float currentValue = RightGripAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		RightGripValue = icurrentValue;
		auto b = FControllerInputState("rift_grip", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightGripAxis = Value.Get<float>();
		}
		float currentValue = RightGripAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		RightGripValue = icurrentValue;
		auto b = FControllerInputState("grip", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_grip", 0);
		AppendInputState(true, b);
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

void UEnhancedInputTracker::RightTouchpadReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightTouchpadH = ValueVector.X;
			RightTouchpadV = ValueVector.Y;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightTouchpadH = ValueVector.X;
			RightTouchpadV = ValueVector.Y;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadTouched(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightTouchpadH = ValueVector.X;
			RightTouchpadV = ValueVector.Y;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightTouchpadH = ValueVector.X;
			RightTouchpadV = ValueVector.Y;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightTouchpadH = ValueVector.X;
			RightTouchpadV = ValueVector.Y;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightTouchpadH = ValueVector.X;
			RightTouchpadV = ValueVector.Y;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadPressRelease(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightTouchpadH = ValueVector.X;
			RightTouchpadV = ValueVector.Y;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D ValueVector = Value.Get<FVector2D>();
			RightTouchpadH = ValueVector.X;
			RightTouchpadV = ValueVector.Y;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadXReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadH = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadH = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadXTouched(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadH = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadH = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadXPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadH = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadH = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadXPressRelease(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadH = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadH = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadYReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadV = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadV = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadYTouched(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadV = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadV = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadYPressed(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadV = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadV = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTouchpadYPressRelease(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadV = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			float ValueVector = Value.Get<float>();
			RightTouchpadV = ValueVector;
		}
		float x = RightTouchpadH;
		float y = RightTouchpadV;
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTriggerReleased(const FInputActionValue& Value)
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightTriggerAxis = Value.Get<float>();
		}
		float currentValue = RightTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		RightTriggerValue = icurrentValue;
		auto b = FControllerInputState("vive_trigger", icurrentValue);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightTriggerAxis = Value.Get<float>();
		}
		float currentValue = RightTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		RightTriggerValue = icurrentValue;
		auto b = FControllerInputState("rift_trigger", icurrentValue);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightTriggerAxis = Value.Get<float>();
		}
		float currentValue = RightTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		RightTriggerValue = icurrentValue;
		auto b = FControllerInputState("trigger", icurrentValue);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_trigger", 0);
		RightTriggerValue = 0;
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			RightTriggerAxis = Value.Get<float>();
		}
		float currentValue = RightTriggerAxis;
		int32 icurrentValue = (int32)(currentValue * 100);
		RightTriggerValue = icurrentValue;
		auto b = FControllerInputState("wmr_trigger", icurrentValue);
		AppendInputState(true, b);
		break;
	}
	}
}

void UEnhancedInputTracker::RightTriggerPressed()
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
	case EC3DControllerType::OculusRift:
	{
		auto b = FControllerInputState("rift_trigger", 100);
		RightTriggerValue = 100;
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		auto b = FControllerInputState("trigger", 100);
		RightTriggerValue = 100;
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		auto b = FControllerInputState("pico_trigger", 100);
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

void UEnhancedInputTracker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TSharedPtr<FAnalyticsProviderCognitive3D> cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	cogProvider->OnSessionBegin.RemoveDynamic(this, &UEnhancedInputTracker::FindControllers);
}