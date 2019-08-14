// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVRTestGameMode.h"
#include "CognitiveVRTestHUD.h"
#include "CognitiveVRTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACognitiveVRTestGameMode::ACognitiveVRTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ACognitiveVRTestHUD::StaticClass();
}
