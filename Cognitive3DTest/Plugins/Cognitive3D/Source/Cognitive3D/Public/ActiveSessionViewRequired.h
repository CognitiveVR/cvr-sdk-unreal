/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "ActiveSessionViewRequired.generated.h"

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
