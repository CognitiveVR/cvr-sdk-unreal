// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "C3DPlatformGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COGNITIVE3D_API UC3DPlatformGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
};
