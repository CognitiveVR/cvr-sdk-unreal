/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "DynamicIdPoolAssetFactoryNew.generated.h"


/**
 * Implements a factory for UTextAsset objects.
 */
UCLASS(hidecategories=Object)
class UDynamicIdPoolAssetFactoryNew
	: public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	//~ UFactory Interface

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};
