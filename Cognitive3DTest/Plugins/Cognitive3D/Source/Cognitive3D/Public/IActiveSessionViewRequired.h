// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "IActiveSessionViewRequired.generated.h"

class AActiveSessionView;

//this exists for unreal's reflection system
UINTERFACE(BlueprintType)
class COGNITIVE3D_API UActiveSessionViewRequired : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

//the actual interface with functions. other classes inherit from this
class COGNITIVE3D_API IActiveSessionViewRequired
{
	GENERATED_IINTERFACE_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cognitive3D Analytics")
		void Initialize(AActiveSessionView* asv);
};
