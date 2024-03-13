/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "DynamicIdPoolAssetActions.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "DynamicIdPoolAsset.h"


#define LOCTEXT_NAMESPACE "Asset2TypeActions"

FDynamicIdPoolAssetActions::FDynamicIdPoolAssetActions(){ }

UClass* FDynamicIdPoolAssetActions::GetSupportedClass() const
{
	return UDynamicIdPoolAsset::StaticClass();
}

uint32 FDynamicIdPoolAssetActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
	return EAssetTypeCategories::Misc;
}

FText FDynamicIdPoolAssetActions::GetName() const
{
	return NSLOCTEXT("Asset2TypeActions", "Asset2TypeActions_TextAsset", "Dynamic Id Pool Asset");
}

FColor FDynamicIdPoolAssetActions::GetTypeColor() const
{
	return FColor::White;
}

#undef LOCTEXT_NAMESPACE