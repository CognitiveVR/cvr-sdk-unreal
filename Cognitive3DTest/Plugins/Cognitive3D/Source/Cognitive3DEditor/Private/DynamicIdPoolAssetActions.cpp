/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "DynamicIdPoolAssetActions.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "DynamicIdPoolAsset.h"


#define LOCTEXT_NAMESPACE "Cognitive3DEditor"

FDynamicIdPoolAssetActions::FDynamicIdPoolAssetActions(){ }

UClass* FDynamicIdPoolAssetActions::GetSupportedClass() const
{
	return UDynamicIdPoolAsset::StaticClass();
}

uint32 FDynamicIdPoolAssetActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

FText FDynamicIdPoolAssetActions::GetName() const
{
	return NSLOCTEXT("Cognitive3DEditor", "DynamicIdPoolAsset_Name", "Dynamic Id Pool Asset");
}

FColor FDynamicIdPoolAssetActions::GetTypeColor() const
{
	return FColor::White;
}

#undef LOCTEXT_NAMESPACE