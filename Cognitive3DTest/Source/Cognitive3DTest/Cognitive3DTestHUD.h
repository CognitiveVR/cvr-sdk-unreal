// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Cognitive3DTestHUD.generated.h"

UCLASS()
class ACognitive3DTestHUD : public AHUD
{
	GENERATED_BODY()

public:
	ACognitive3DTestHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

