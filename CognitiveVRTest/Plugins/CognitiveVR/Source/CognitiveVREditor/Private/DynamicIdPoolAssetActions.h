// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AssetTypeActions_Base.h"
#include "Templates/SharedPointer.h"

class ISlateStyle;


//implements some actions for dynamic id pool
class FDynamicIdPoolAssetActions
	: public FAssetTypeActions_Base
{

	
	

public:
	FDynamicIdPoolAssetActions();

	//~ FAssetTypeActions_Base overrides

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
};
