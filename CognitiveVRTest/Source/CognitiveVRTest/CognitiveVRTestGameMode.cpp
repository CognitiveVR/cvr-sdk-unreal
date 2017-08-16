// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVRTest.h"
#include "CognitiveVRTestGameMode.h"
#include "CognitiveVRTestHUD.h"
#include "CognitiveVRTestCharacter.h"

ACognitiveVRTestGameMode::ACognitiveVRTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ACognitiveVRTestHUD::StaticClass();
}
