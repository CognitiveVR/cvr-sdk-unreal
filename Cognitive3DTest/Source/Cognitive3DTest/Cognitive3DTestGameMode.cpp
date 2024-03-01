// Copyright Epic Games, Inc. All Rights Reserved.

#include "Cognitive3DTestGameMode.h"
#include "Cognitive3DTestHUD.h"
#include "Cognitive3DTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACognitive3DTestGameMode::ACognitive3DTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ACognitive3DTestHUD::StaticClass();
}
