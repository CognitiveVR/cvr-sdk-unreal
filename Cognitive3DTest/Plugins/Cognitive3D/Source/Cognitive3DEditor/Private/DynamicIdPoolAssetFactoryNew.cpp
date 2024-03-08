/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "DynamicIdPoolAssetFactoryNew.h"

#include "DynamicIdPoolAsset.h"


/* UTextAssetFactoryNew structors
 *****************************************************************************/

UDynamicIdPoolAssetFactoryNew::UDynamicIdPoolAssetFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UDynamicIdPoolAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UDynamicIdPoolAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UDynamicIdPoolAsset>(InParent, InClass, InName, Flags);
}


bool UDynamicIdPoolAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
