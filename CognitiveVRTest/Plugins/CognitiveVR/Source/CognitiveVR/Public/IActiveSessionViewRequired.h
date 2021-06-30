// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "IActiveSessionViewRequired.generated.h"

class AActiveSessionView;

//this exists for unreal's reflection system
UINTERFACE(BlueprintType)
class COGNITIVEVR_API UActiveSessionViewRequired : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

//the actual interface with functions. other classes inherit from this
class COGNITIVEVR_API IActiveSessionViewRequired
{
	GENERATED_IINTERFACE_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CognitiveVR Analytics")
		void Initialize(AActiveSessionView* asv);
};
