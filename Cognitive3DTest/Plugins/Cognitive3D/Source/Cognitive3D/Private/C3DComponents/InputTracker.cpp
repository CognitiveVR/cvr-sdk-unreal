/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "InputTracker.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectIterator.h"
#include "Components/InputComponent.h"
#include "Cognitive3DActor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"

// Sets default values
UInputTracker::UInputTracker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

}

void UInputTracker::BeginPlay()
{
	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	Super::BeginPlay();

	InitializeInputBindings();

	TSharedPtr<FAnalyticsProviderCognitive3D> cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cogProvider.IsValid() && cogProvider->HasStartedSession())
	{
		HandleSessionBegin();
	}
	if (cogProvider.IsValid()) cogProvider->OnSessionBegin.AddUniqueDynamic(this, &UInputTracker::HandleSessionBegin);
}

bool UInputTracker::InitializeInputBindings()
{
	APlayerController* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!IsValid(Player) || !IsValid(GetOwner())) return false;
	if (IsValid(InputComponent) && BoundPlayerController.Get() == Player) return true;
	UInputComponent* PreviousComponent = InputComponent;
	if (BoundPlayerController.IsValid()) GetOwner()->DisableInput(BoundPlayerController.Get());
	GetOwner()->EnableInput(Player);
	InputComponent = GetOwner()->InputComponent;
	if (!IsValid(InputComponent)) return false;
	BoundPlayerController = Player;
	if (InputComponent == PreviousComponent && bHasInputBindings) return true;
	InputComponent->bBlockInput = 0;

	InputComponent->BindAction("C3D_RightMenuButton", EInputEvent::IE_Pressed, this, &UInputTracker::RightMenuButtonPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightMenuButton", EInputEvent::IE_Released, this, &UInputTracker::RightMenuButtonReleased).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftMenuButton", EInputEvent::IE_Pressed, this, &UInputTracker::LeftMenuButtonPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftMenuButton", EInputEvent::IE_Released, this, &UInputTracker::LeftMenuButtonReleased).bConsumeInput = 0;

	InputComponent->BindAction("C3D_RightTrigger", EInputEvent::IE_Pressed, this, &UInputTracker::RightTriggerPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightTrigger", EInputEvent::IE_Released, this, &UInputTracker::RightTriggerReleased).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftTrigger", EInputEvent::IE_Pressed, this, &UInputTracker::LeftTriggerPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftTrigger", EInputEvent::IE_Released, this, &UInputTracker::LeftTriggerReleased).bConsumeInput = 0;

	InputComponent->BindAction("C3D_RightGrip", EInputEvent::IE_Pressed, this, &UInputTracker::RightGripPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightGrip", EInputEvent::IE_Released, this, &UInputTracker::RightGripReleased).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftGrip", EInputEvent::IE_Pressed, this, &UInputTracker::LeftGripPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftGrip", EInputEvent::IE_Released, this, &UInputTracker::LeftGripReleased).bConsumeInput = 0;

	InputComponent->BindAction("C3D_RightTouchpadPress", EInputEvent::IE_Pressed, this, &UInputTracker::RightTouchpadPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightTouchpadPress", EInputEvent::IE_Released, this, &UInputTracker::RightTouchpadPressRelease).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightTouchpadTouch", EInputEvent::IE_Pressed, this, &UInputTracker::RightTouchpadTouched).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightTouchpadTouch", EInputEvent::IE_Released, this, &UInputTracker::RightTouchpadReleased).bConsumeInput = 0;

	InputComponent->BindAction("C3D_LeftTouchpadPress", EInputEvent::IE_Pressed, this, &UInputTracker::LeftTouchpadPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftTouchpadPress", EInputEvent::IE_Released, this, &UInputTracker::LeftTouchpadPressRelease).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftTouchpadTouch", EInputEvent::IE_Pressed, this, &UInputTracker::LeftTouchpadTouched).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftTouchpadTouch", EInputEvent::IE_Released, this, &UInputTracker::LeftTouchpadReleased).bConsumeInput = 0;

	InputComponent->BindAction("C3D_RightJoystick", EInputEvent::IE_Pressed, this, &UInputTracker::RightJoystickPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightJoystick", EInputEvent::IE_Released, this, &UInputTracker::RightJoystickReleased).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftJoystick", EInputEvent::IE_Pressed, this, &UInputTracker::LeftJoystickPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftJoystick", EInputEvent::IE_Released, this, &UInputTracker::LeftJoystickReleased).bConsumeInput = 0;

	InputComponent->BindAction("C3D_LeftFaceButtonOne", EInputEvent::IE_Pressed, this, &UInputTracker::LeftFaceButtonOnePressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftFaceButtonOne", EInputEvent::IE_Released, this, &UInputTracker::LeftFaceButtonOneReleased).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftFaceButtonTwo", EInputEvent::IE_Pressed, this, &UInputTracker::LeftFaceButtonTwoPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_LeftFaceButtonTwo", EInputEvent::IE_Released, this, &UInputTracker::LeftFaceButtonTwoReleased).bConsumeInput = 0;

	InputComponent->BindAction("C3D_RightFaceButtonOne", EInputEvent::IE_Pressed, this, &UInputTracker::RightFaceButtonOnePressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightFaceButtonOne", EInputEvent::IE_Released, this, &UInputTracker::RightFaceButtonOneReleased).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightFaceButtonTwo", EInputEvent::IE_Pressed, this, &UInputTracker::RightFaceButtonTwoPressed).bConsumeInput = 0;
	InputComponent->BindAction("C3D_RightFaceButtonTwo", EInputEvent::IE_Released, this, &UInputTracker::RightFaceButtonTwoReleased).bConsumeInput = 0;

	InputComponent->BindAxis(TEXT("C3D_LeftTriggerAxis")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_RightTriggerAxis")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_LeftTouchpadV")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_LeftTouchpadH")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_RightTouchpadV")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_RightTouchpadH")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_LeftJoystickH")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_LeftJoystickV")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_RightJoystickH")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_RightJoystickV")).bConsumeInput = 0;

	InputComponent->BindAxis(TEXT("C3D_LeftGripAxis")).bConsumeInput = 0;
	InputComponent->BindAxis(TEXT("C3D_RightGripAxis")).bConsumeInput = 0;


	const UInputSettings* Settings = GetDefault<UInputSettings>();
	auto BindPico = [this, Settings](FName Action, const TCHAR* KeyName,
		void (UInputTracker::*Pressed)(), void (UInputTracker::*Released)())
	{
		for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
			if (Mapping.ActionName == Action) return;
		const FKey Key(KeyName);
		if (!Key.IsValid()) return;
		FInputKeyBinding& PressBinding = InputComponent->BindKey(Key, IE_Pressed, this, Pressed);
		PressBinding.bConsumeInput = false;
		PressBinding.bExecuteWhenPaused = true;
		FInputKeyBinding& ReleaseBinding = InputComponent->BindKey(Key, IE_Released, this, Released);
		ReleaseBinding.bConsumeInput = false;
		ReleaseBinding.bExecuteWhenPaused = true;
	};
	BindPico(TEXT("C3D_LeftMenuButton"), TEXT("PICOTouch_Left_Menu_Click"), &UInputTracker::LeftMenuButtonPressed, &UInputTracker::LeftMenuButtonReleased);
	BindPico(TEXT("C3D_LeftTrigger"), TEXT("PICOTouch_Left_Trigger_Click"), &UInputTracker::LeftTriggerPressed, &UInputTracker::LeftTriggerReleased);
	BindPico(TEXT("C3D_LeftGrip"), TEXT("PICOTouch_Left_Grip_Click"), &UInputTracker::LeftGripPressed, &UInputTracker::LeftGripReleased);
	BindPico(TEXT("C3D_LeftJoystick"), TEXT("PICOTouch_Left_Thumbstick_Click"), &UInputTracker::LeftJoystickPressed, &UInputTracker::LeftJoystickReleased);
	BindPico(TEXT("C3D_LeftFaceButtonOne"), TEXT("PICOTouch_Left_X_Click"), &UInputTracker::LeftFaceButtonOnePressed, &UInputTracker::LeftFaceButtonOneReleased);
	BindPico(TEXT("C3D_LeftFaceButtonTwo"), TEXT("PICOTouch_Left_Y_Click"), &UInputTracker::LeftFaceButtonTwoPressed, &UInputTracker::LeftFaceButtonTwoReleased);
	BindPico(TEXT("C3D_RightMenuButton"), TEXT("PICOTouch_Right_System_Click"), &UInputTracker::RightMenuButtonPressed, &UInputTracker::RightMenuButtonReleased);
	BindPico(TEXT("C3D_RightTrigger"), TEXT("PICOTouch_Right_Trigger_Click"), &UInputTracker::RightTriggerPressed, &UInputTracker::RightTriggerReleased);
	BindPico(TEXT("C3D_RightGrip"), TEXT("PICOTouch_Right_Grip_Click"), &UInputTracker::RightGripPressed, &UInputTracker::RightGripReleased);
	BindPico(TEXT("C3D_RightJoystick"), TEXT("PICOTouch_Right_Thumbstick_Click"), &UInputTracker::RightJoystickPressed, &UInputTracker::RightJoystickReleased);
	BindPico(TEXT("C3D_RightFaceButtonOne"), TEXT("PICOTouch_Right_A_Click"), &UInputTracker::RightFaceButtonOnePressed, &UInputTracker::RightFaceButtonOneReleased);
	BindPico(TEXT("C3D_RightFaceButtonTwo"), TEXT("PICOTouch_Right_B_Click"), &UInputTracker::RightFaceButtonTwoPressed, &UInputTracker::RightFaceButtonTwoReleased);
	PicoAxisFallbacks.Empty();
	auto AddPicoAxis = [this, Settings](FName Axis, const TCHAR* KeyName)
	{
		for (const FInputAxisKeyMapping& Mapping : Settings->GetAxisMappings())
			if (Mapping.AxisName == Axis) return;
		const FKey Key(KeyName);
		if (Key.IsValid()) PicoAxisFallbacks.Add(Axis, Key);
	};
	AddPicoAxis(TEXT("C3D_LeftTriggerAxis"), TEXT("PICOTouch_Left_Trigger_Axis"));
	AddPicoAxis(TEXT("C3D_LeftGripAxis"), TEXT("PICOTouch_Left_Grip_Axis"));
	AddPicoAxis(TEXT("C3D_LeftJoystickH"), TEXT("PICOTouch_Left_Thumbstick_X"));
	AddPicoAxis(TEXT("C3D_LeftJoystickV"), TEXT("PICOTouch_Left_Thumbstick_Y"));
	AddPicoAxis(TEXT("C3D_RightTriggerAxis"), TEXT("PICOTouch_Right_Trigger_Axis"));
	AddPicoAxis(TEXT("C3D_RightGripAxis"), TEXT("PICOTouch_Right_Grip_Axis"));
	AddPicoAxis(TEXT("C3D_RightJoystickH"), TEXT("PICOTouch_Right_Thumbstick_X"));
	AddPicoAxis(TEXT("C3D_RightJoystickV"), TEXT("PICOTouch_Right_Thumbstick_Y"));
	bHasInputBindings = true;
	return true;
}

float UInputTracker::ReadAxis(FName AxisName) const
{
	if (ControllerType == EC3DControllerType::PicoNeo2 || ControllerType == EC3DControllerType::PicoNeo3 ||
		ControllerType == EC3DControllerType::PicoNeo4)
	{
		const FKey* Key = PicoAxisFallbacks.Find(AxisName);
		if (Key && BoundPlayerController.IsValid()) return BoundPlayerController->GetInputAnalogKeyState(*Key);
	}
	return IsValid(InputComponent) ? InputComponent->GetAxisValue(AxisName) : 0.0f;
}

void UInputTracker::FindControllers()
{
	ACognitive3DActor* Recorder = ACognitive3DActor::GetCognitive3DActor(this);
	UDynamicObject* NewLeft = IsValid(Recorder) ? Recorder->GetControllerDynamic(false) : nullptr;
	UDynamicObject* NewRight = IsValid(Recorder) ? Recorder->GetControllerDynamic(true) : nullptr;
	if (LeftHand != NewLeft) { LeftHand = NewLeft; ResetInputState(false); }
	if (RightHand != NewRight) { RightHand = NewRight; ResetInputState(true); }
	if (IsValid(LeftHand)) ControllerType = LeftHand->ControllerType;
	else if (IsValid(RightHand)) ControllerType = RightHand->ControllerType;
}

void UInputTracker::ResetInputState(bool bRight)
{
	if (bRight)
	{
		RightInputStates.States.Empty();
		RightTriggerValue = RightGripValue = -1;
		RightJoystickAxis = RightTouchpadAxis = FVector(BIG_NUMBER, BIG_NUMBER, 0);
		bSeedRightButtons = true;
	}
	else
	{
		LeftInputStates.States.Empty();
		LeftTriggerValue = LeftGripValue = -1;
		LeftJoystickAxis = LeftTouchpadAxis = FVector(BIG_NUMBER, BIG_NUMBER, 0);
		bSeedLeftButtons = true;
	}
}

void UInputTracker::HandleSessionBegin()
{
	ResetInputState(false);
	ResetInputState(true);
	CurrentIntervalTime = 0;
	FindControllers();
}

void UInputTracker::SeedButtonStates(bool bRight)
{
	APlayerController* Player = BoundPlayerController.Get();
	if (!IsValid(Player)) return;
	const UInputSettings* Settings = GetDefault<UInputSettings>();
	auto Seed = [this, Player, Settings](FName Action, const TCHAR* PicoKey,
		void (UInputTracker::*Pressed)(), void (UInputTracker::*Released)())
	{
		bool bDown = false, bMapped = false;
		for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
		{
			if (Mapping.ActionName != Action) continue;
			bMapped = true;
			bDown |= Player->IsInputKeyDown(Mapping.Key);
		}
		if (!bMapped && (ControllerType == EC3DControllerType::PicoNeo2 ||
			ControllerType == EC3DControllerType::PicoNeo3 || ControllerType == EC3DControllerType::PicoNeo4))
			bDown = Player->IsInputKeyDown(FKey(PicoKey));
		(this->*(bDown ? Pressed : Released))();
	};
	if (bRight)
	{
		Seed(TEXT("C3D_RightMenuButton"), TEXT("PICOTouch_Right_System_Click"), &UInputTracker::RightMenuButtonPressed, &UInputTracker::RightMenuButtonReleased);
		Seed(TEXT("C3D_RightTrigger"), TEXT("PICOTouch_Right_Trigger_Click"), &UInputTracker::RightTriggerPressed, &UInputTracker::RightTriggerReleased);
		Seed(TEXT("C3D_RightGrip"), TEXT("PICOTouch_Right_Grip_Click"), &UInputTracker::RightGripPressed, &UInputTracker::RightGripReleased);
		Seed(TEXT("C3D_RightJoystick"), TEXT("PICOTouch_Right_Thumbstick_Click"), &UInputTracker::RightJoystickPressed, &UInputTracker::RightJoystickReleased);
		Seed(TEXT("C3D_RightFaceButtonOne"), TEXT("PICOTouch_Right_A_Click"), &UInputTracker::RightFaceButtonOnePressed, &UInputTracker::RightFaceButtonOneReleased);
		Seed(TEXT("C3D_RightFaceButtonTwo"), TEXT("PICOTouch_Right_B_Click"), &UInputTracker::RightFaceButtonTwoPressed, &UInputTracker::RightFaceButtonTwoReleased);
	}
	else
	{
		Seed(TEXT("C3D_LeftMenuButton"), TEXT("PICOTouch_Left_Menu_Click"), &UInputTracker::LeftMenuButtonPressed, &UInputTracker::LeftMenuButtonReleased);
		Seed(TEXT("C3D_LeftTrigger"), TEXT("PICOTouch_Left_Trigger_Click"), &UInputTracker::LeftTriggerPressed, &UInputTracker::LeftTriggerReleased);
		Seed(TEXT("C3D_LeftGrip"), TEXT("PICOTouch_Left_Grip_Click"), &UInputTracker::LeftGripPressed, &UInputTracker::LeftGripReleased);
		Seed(TEXT("C3D_LeftJoystick"), TEXT("PICOTouch_Left_Thumbstick_Click"), &UInputTracker::LeftJoystickPressed, &UInputTracker::LeftJoystickReleased);
		Seed(TEXT("C3D_LeftFaceButtonOne"), TEXT("PICOTouch_Left_X_Click"), &UInputTracker::LeftFaceButtonOnePressed, &UInputTracker::LeftFaceButtonOneReleased);
		Seed(TEXT("C3D_LeftFaceButtonTwo"), TEXT("PICOTouch_Left_Y_Click"), &UInputTracker::LeftFaceButtonTwoPressed, &UInputTracker::LeftFaceButtonTwoReleased);
	}
}

void UInputTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!InitializeInputBindings() || !cog.IsValid() || !cog->HasStartedSession()) return;
	FindControllers();

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
		if (IsValid(LeftHand) && bSeedLeftButtons) { SeedButtonStates(false); bSeedLeftButtons = false; }
		if (IsValid(RightHand) && bSeedRightButtons) { SeedButtonStates(true); bSeedRightButtons = false; }
		if (IsValid(LeftHand) || IsValid(RightHand)) IntervalUpdate();
		CurrentIntervalTime -= Interval;
	}
}

void UInputTracker::IntervalUpdate()
{
	//check if touchpad/joystick/grip/trigger values have changed
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		//triggers
		float currentValue = ReadAxis("C3D_LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("vive_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = ReadAxis("C3D_RightTriggerAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("vive_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//touchpads
		FVector currentLeftTouchpad = FVector(ReadAxis("C3D_LeftTouchpadH"), ReadAxis("C3D_LeftTouchpadV"), LeftTouchpadAxis.Z);
		if (FVector::Distance(LeftTouchpadAxis, currentLeftTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			LeftTouchpadAxis = currentLeftTouchpad;
			auto b = FControllerInputState("vive_touchpad", LeftTouchpadAxis);
			AppendInputState(false, b);
		}

		FVector currentRightTouchpad = FVector(ReadAxis("C3D_RightTouchpadH"), ReadAxis("C3D_RightTouchpadV"), RightTouchpadAxis.Z);
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
		float currentValue = ReadAxis("C3D_LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = ReadAxis("C3D_RightTriggerAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//grip
		currentValue = ReadAxis("C3D_LeftGripAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (LeftGripValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_grip", icurrentValue);
			AppendInputState(false, b);
			LeftGripValue = icurrentValue;
		}
		currentValue = ReadAxis("C3D_RightGripAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightGripValue != icurrentValue)
		{
			auto b = FControllerInputState("rift_grip", icurrentValue);
			AppendInputState(true, b);
			RightGripValue = icurrentValue;
		}

		//joysticks
		FVector currentLeftJoystick = FVector(ReadAxis("C3D_LeftJoystickH"), ReadAxis("C3D_LeftJoystickV"), LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("rift_joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(ReadAxis("C3D_RightJoystickH"), ReadAxis("C3D_RightJoystickV"), RightJoystickAxis.Z);
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
		float currentValue = ReadAxis("C3D_LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = ReadAxis("C3D_RightTriggerAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//grip
		currentValue = ReadAxis("C3D_LeftGripAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (LeftGripValue != icurrentValue)
		{
			auto b = FControllerInputState("grip", icurrentValue);
			AppendInputState(false, b);
			LeftGripValue = icurrentValue;
		}
		currentValue = ReadAxis("C3D_RightGripAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightGripValue != icurrentValue)
		{
			auto b = FControllerInputState("grip", icurrentValue);
			AppendInputState(true, b);
			RightGripValue = icurrentValue;
		}

		//joysticks
		FVector currentLeftJoystick = FVector(ReadAxis("C3D_LeftJoystickH"), ReadAxis("C3D_LeftJoystickV"), LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(ReadAxis("C3D_RightJoystickH"), ReadAxis("C3D_RightJoystickV"), RightJoystickAxis.Z);
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
		float currentValue = ReadAxis("C3D_LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("wmr_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = ReadAxis("C3D_RightTriggerAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("wmr_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		//touchpads

		FVector currentLeftTouchpad = FVector(ReadAxis("C3D_LeftTouchpadH"), ReadAxis("C3D_LeftTouchpadV"), LeftTouchpadAxis.Z);
		if (FVector::Distance(LeftTouchpadAxis, currentLeftTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			LeftTouchpadAxis = currentLeftTouchpad;
			auto b = FControllerInputState("wmr_touchpad", LeftTouchpadAxis);
			AppendInputState(false, b);
		}

		FVector currentRightTouchpad = FVector(ReadAxis("C3D_RightTouchpadH"), ReadAxis("C3D_RightTouchpadV"), RightTouchpadAxis.Z);
		if (FVector::Distance(RightTouchpadAxis, currentRightTouchpad) > MinimumVectorChange)
		{
			//write new stuff
			RightTouchpadAxis = currentRightTouchpad;
			auto b = FControllerInputState("wmr_touchpad", RightTouchpadAxis);
			AppendInputState(true, b);
		}

		//joysticks
		FVector currentLeftJoystick = FVector(ReadAxis("C3D_LeftJoystickH"), ReadAxis("C3D_LeftJoystickV"), LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("wmr_joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(ReadAxis("C3D_RightJoystickH"), ReadAxis("C3D_RightJoystickV"), RightJoystickAxis.Z);
		if (FVector::Distance(RightJoystickAxis, currentRightJoystick) > MinimumVectorChange)
		{
			//write new stuff
			RightJoystickAxis = currentRightJoystick;
			auto b = FControllerInputState("wmr_joystick", RightJoystickAxis);
			AppendInputState(true, b);
		}
		break;
	}
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	case EC3DControllerType::PicoNeo2:
	{
		//triggers
		float currentValue = ReadAxis("C3D_LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		if (LeftTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("pico_trigger", icurrentValue);
			AppendInputState(false, b);
			LeftTriggerValue = icurrentValue;
		}
		currentValue = ReadAxis("C3D_RightTriggerAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightTriggerValue != icurrentValue)
		{
			auto b = FControllerInputState("pico_trigger", icurrentValue);
			AppendInputState(true, b);
			RightTriggerValue = icurrentValue;
		}

		currentValue = ReadAxis("C3D_LeftGripAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (LeftGripValue != icurrentValue)
		{
			auto State = FControllerInputState("pico_grip", icurrentValue);
			AppendInputState(false, State);
			LeftGripValue = icurrentValue;
		}
		currentValue = ReadAxis("C3D_RightGripAxis");
		icurrentValue = (int32)(currentValue * 100);
		if (RightGripValue != icurrentValue)
		{
			auto State = FControllerInputState("pico_grip", icurrentValue);
			AppendInputState(true, State);
			RightGripValue = icurrentValue;
		}

		//joysticks
		FVector currentLeftJoystick = FVector(ReadAxis("C3D_LeftJoystickH"), ReadAxis("C3D_LeftJoystickV"), LeftJoystickAxis.Z);
		if (FVector::Distance(LeftJoystickAxis, currentLeftJoystick) > MinimumVectorChange)
		{
			//write new stuff
			LeftJoystickAxis = currentLeftJoystick;
			auto b = FControllerInputState("pico_joystick", LeftJoystickAxis);
			AppendInputState(false, b);
		}

		FVector currentRightJoystick = FVector(ReadAxis("C3D_RightJoystickH"), ReadAxis("C3D_RightJoystickV"), RightJoystickAxis.Z);
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

void UInputTracker::AppendInputState(const bool isRight, FControllerInputState& state)
{
	if (!cog.IsValid() || !cog->HasStartedSession()) return;
	if (isRight ? !IsValid(RightHand) : !IsValid(LeftHand)) return;
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

void UInputTracker::LeftFaceButtonOnePressed()
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

void UInputTracker::LeftFaceButtonOneReleased()
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

void UInputTracker::LeftFaceButtonTwoPressed()
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
void UInputTracker::LeftFaceButtonTwoReleased()
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

void UInputTracker::LeftMenuButtonPressed()
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
void UInputTracker::LeftMenuButtonReleased()
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

void UInputTracker::LeftJoystickPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		float x = ReadAxis("C3D_LeftJoystickH");
		float y = ReadAxis("C3D_LeftJoystickV");
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		float x = ReadAxis("C3D_LeftJoystickH");
		float y = ReadAxis("C3D_LeftJoystickV");
		auto b = FControllerInputState("joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		float x = ReadAxis("C3D_LeftJoystickH");
		float y = ReadAxis("C3D_LeftJoystickV");
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_LeftJoystickH");
		float y = ReadAxis("C3D_LeftJoystickV");
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		LeftJoystickAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	}
}
void UInputTracker::LeftJoystickReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		float x = ReadAxis("C3D_LeftJoystickH");
		float y = ReadAxis("C3D_LeftJoystickV");
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		float x = ReadAxis("C3D_LeftJoystickH");
		float y = ReadAxis("C3D_LeftJoystickV");
		auto b = FControllerInputState("joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		float x = ReadAxis("C3D_LeftJoystickH");
		float y = ReadAxis("C3D_LeftJoystickV");
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_LeftJoystickH");
		float y = ReadAxis("C3D_LeftJoystickV");
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		LeftJoystickAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	}
}

void UInputTracker::LeftTouchpadReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = ReadAxis("C3D_LeftTouchpadH");
		float y = ReadAxis("C3D_LeftTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_LeftTouchpadH");
		float y = ReadAxis("C3D_LeftTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		LeftTouchpadAxis = FVector(x, y, 0);
		AppendInputState(false, b);
		break;
	}
	}
}
void UInputTracker::LeftTouchpadTouched()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = ReadAxis("C3D_LeftTouchpadH");
		float y = ReadAxis("C3D_LeftTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_LeftTouchpadH");
		float y = ReadAxis("C3D_LeftTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}
void UInputTracker::LeftTouchpadPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = ReadAxis("C3D_LeftTouchpadH");
		float y = ReadAxis("C3D_LeftTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_LeftTouchpadH");
		float y = ReadAxis("C3D_LeftTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		LeftTouchpadAxis = FVector(x, y, 100);
		AppendInputState(false, b);
		break;
	}
	}
}

void UInputTracker::LeftTouchpadPressRelease()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = ReadAxis("C3D_LeftTouchpadH");
		float y = ReadAxis("C3D_LeftTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_LeftTouchpadH");
		float y = ReadAxis("C3D_LeftTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		LeftTouchpadAxis = FVector(x, y, 50);
		AppendInputState(false, b);
		break;
	}
	}
}


void UInputTracker::RightFaceButtonOnePressed()
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
void UInputTracker::RightFaceButtonOneReleased()
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

void UInputTracker::RightFaceButtonTwoPressed()
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
void UInputTracker::RightFaceButtonTwoReleased()
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

void UInputTracker::RightMenuButtonPressed()
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
void UInputTracker::RightMenuButtonReleased()
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

void UInputTracker::RightJoystickPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		float x = ReadAxis("C3D_RightJoystickH");
		float y = ReadAxis("C3D_RightJoystickV");
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		float x = ReadAxis("C3D_RightJoystickH");
		float y = ReadAxis("C3D_RightJoystickV");
		auto b = FControllerInputState("joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		float x = ReadAxis("C3D_RightJoystickH");
		float y = ReadAxis("C3D_RightJoystickV");
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_RightJoystickH");
		float y = ReadAxis("C3D_RightJoystickV");
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 100));
		RightJoystickAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}
void UInputTracker::RightJoystickReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
		break;
	case EC3DControllerType::OculusRift:
	{
		float x = ReadAxis("C3D_RightJoystickH");
		float y = ReadAxis("C3D_RightJoystickV");
		auto b = FControllerInputState("rift_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::Quest2:
	case EC3DControllerType::Quest3:
	case EC3DControllerType::QuestPro:
	{
		float x = ReadAxis("C3D_RightJoystickH");
		float y = ReadAxis("C3D_RightJoystickV");
		auto b = FControllerInputState("joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::PicoNeo2:
	case EC3DControllerType::PicoNeo3:
	case EC3DControllerType::PicoNeo4:
	{
		float x = ReadAxis("C3D_RightJoystickH");
		float y = ReadAxis("C3D_RightJoystickV");
		auto b = FControllerInputState("pico_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_RightJoystickH");
		float y = ReadAxis("C3D_RightJoystickV");
		auto b = FControllerInputState("wmr_joystick", FVector(x, y, 0));
		RightJoystickAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	}
}

void UInputTracker::RightTouchpadReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = ReadAxis("C3D_RightTouchpadH");
		float y = ReadAxis("C3D_RightTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_RightTouchpadH");
		float y = ReadAxis("C3D_RightTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 0));
		RightTouchpadAxis = FVector(x, y, 0);
		AppendInputState(true, b);
		break;
	}
	}
}
void UInputTracker::RightTouchpadTouched()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = ReadAxis("C3D_RightTouchpadH");
		float y = ReadAxis("C3D_RightTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_RightTouchpadH");
		float y = ReadAxis("C3D_RightTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}
void UInputTracker::RightTouchpadPressed()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = ReadAxis("C3D_RightTouchpadH");
		float y = ReadAxis("C3D_RightTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_RightTouchpadH");
		float y = ReadAxis("C3D_RightTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 100));
		RightTouchpadAxis = FVector(x, y, 100);
		AppendInputState(true, b);
		break;
	}
	}
}

void UInputTracker::RightTouchpadPressRelease()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float x = ReadAxis("C3D_RightTouchpadH");
		float y = ReadAxis("C3D_RightTouchpadV");
		auto b = FControllerInputState("vive_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
		break;
	case EC3DControllerType::WindowsMixedReality:
	{
		float x = ReadAxis("C3D_RightTouchpadH");
		float y = ReadAxis("C3D_RightTouchpadV");
		auto b = FControllerInputState("wmr_touchpad", FVector(x, y, 50));
		RightTouchpadAxis = FVector(x, y, 50);
		AppendInputState(true, b);
		break;
	}
	}
}

void UInputTracker::RightGripPressed()
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

void UInputTracker::RightGripReleased()
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
		float currentValue = ReadAxis("C3D_RightGripAxis");
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
		float currentValue = ReadAxis("C3D_RightGripAxis");
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

void UInputTracker::LeftGripPressed()
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

void UInputTracker::LeftGripReleased()
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
		float currentValue = ReadAxis("C3D_LeftGripAxis");
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
		float currentValue = ReadAxis("C3D_LeftGripAxis");
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


void UInputTracker::LeftTriggerPressed()
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

void UInputTracker::LeftTriggerReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float currentValue = ReadAxis("C3D_LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftTriggerValue = icurrentValue;
		auto b = FControllerInputState("vive_trigger", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		float currentValue = ReadAxis("C3D_LeftTriggerAxis");
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
		float currentValue = ReadAxis("C3D_LeftTriggerAxis");
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
		float currentValue = ReadAxis("C3D_LeftTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		LeftTriggerValue = icurrentValue;
		auto b = FControllerInputState("wmr_trigger", icurrentValue);
		AppendInputState(false, b);
		break;
	}
	}
}

void UInputTracker::RightTriggerPressed()
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

void UInputTracker::RightTriggerReleased()
{
	switch (ControllerType)
	{
	case EC3DControllerType::Vive:
	{
		float currentValue = ReadAxis("C3D_RightTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		RightTriggerValue = icurrentValue;
		auto b = FControllerInputState("vive_trigger", icurrentValue);
		AppendInputState(true, b);
		break;
	}
	case EC3DControllerType::OculusRift:
	{
		float currentValue = ReadAxis("C3D_RightTriggerAxis");
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
		float currentValue = ReadAxis("C3D_RightTriggerAxis");
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
		float currentValue = ReadAxis("C3D_RightTriggerAxis");
		int32 icurrentValue = (int32)(currentValue * 100);
		RightTriggerValue = icurrentValue;
		auto b = FControllerInputState("wmr_trigger", icurrentValue);
		AppendInputState(true, b);
		break;
	}
	}
}

void UInputTracker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	TSharedPtr<FAnalyticsProviderCognitive3D> cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cogProvider.IsValid()) cogProvider->OnSessionBegin.RemoveDynamic(this, &UInputTracker::HandleSessionBegin);
	if (IsValid(GetOwner()) && BoundPlayerController.IsValid()) GetOwner()->DisableInput(BoundPlayerController.Get());
	LeftHand = nullptr;
	RightHand = nullptr;
	BoundPlayerController.Reset();
}
