#pragma once

// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DynamicIdPoolAssetDetails.h"

TSharedRef<IDetailCustomization> UDynamicIdPoolAssetDetails::MakeInstance()
{
	return MakeShareable( new UDynamicIdPoolAssetDetails);
}

void UDynamicIdPoolAssetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	if (SelectedTextAsset == NULL)
	{
		const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetSelectedObjects();
		for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
		{
			const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
			if (CurrentObject.IsValid())
			{
				UDynamicIdPoolAsset* CurrentCaptureActor = Cast<UDynamicIdPoolAsset>(CurrentObject.Get());
				if (CurrentCaptureActor != NULL)
				{
					SelectedTextAsset = CurrentCaptureActor;
					break;
				}
			}
		}
	}

	//IMPROVEMENT add export directory field
	DetailLayout.EditCategory( "Scene Explorer" )
	.AddCustomRow( NSLOCTEXT("SkyLightDetails", "UpdateSkyLight", "Recapture Scene") )
	.ValueContent()
	.MaxDesiredWidth(200.f)
	.MinDesiredWidth(200.f)
	[
		SNew(STextBlock)
		.Visibility_Raw(this, &UDynamicIdPoolAssetDetails::HasMeshBeenExported)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString("Mesh Name not found. Has this Mesh been exported?"))
	];

	DetailLayout.EditCategory( "Scene Explorer" )
	.AddCustomRow( NSLOCTEXT("SkyLightDetails", "UpdateSkyLight", "Recapture Scene") )
	.ValueContent()
	.MaxDesiredWidth(200.f)
	.MinDesiredWidth(200.f)
	[
		SNew(SButton)
		.ContentPadding(1)
		.IsEnabled_Raw(this, &UDynamicIdPoolAssetDetails::CanUploadIds)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &UDynamicIdPoolAssetDetails::Upload)
		[
			SNew( STextBlock )
			.Font( IDetailLayoutBuilder::GetDetailFont() )
			.Text(FText::FromString("Upload Ids for Aggregation") )
		]
	];

	DetailLayout.EditCategory( "Scene Explorer" )
	.AddCustomRow( NSLOCTEXT("SkyLightDetails", "UpdateSkyLight", "Recapture Scene") )
	.ValueContent()
	.MaxDesiredWidth(200.f)
	.MinDesiredWidth(200.f)
	[
		SNew(SButton)
		.ContentPadding(1)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &UDynamicIdPoolAssetDetails::GenerateId)
		[
			SNew( STextBlock )
			.Font( IDetailLayoutBuilder::GetDetailFont() )
			.Text(FText::FromString("Generate Id") )
		]
	];
}

FReply UDynamicIdPoolAssetDetails::Upload()
{
	FCognitiveEditorTools::GetInstance()->UploadDynamicsManifestIds(SelectedTextAsset->Ids, SelectedTextAsset->MeshName, SelectedTextAsset->PrefabName);
	return FReply::Handled();
}

FReply UDynamicIdPoolAssetDetails::GenerateId()
{
	SelectedTextAsset->GenerateNewId();
	return FReply::Handled();
}

bool UDynamicIdPoolAssetDetails::CanUploadIds() const
{
	//has ids
	if (SelectedTextAsset->Ids.Num() == 0) { return false; }
	//prefab name is set
	if (SelectedTextAsset->PrefabName.IsEmpty()) { return false; }
	//mesh name is set
	if (SelectedTextAsset->MeshName.IsEmpty()) { return false; }
	//scene has id
	TSharedPtr<FEditorSceneData> data = FCognitiveEditorTools::GetInstance()->GetCurrentSceneData();
	
	return data.IsValid();
}

EVisibility UDynamicIdPoolAssetDetails::HasMeshBeenExported() const
{
	if (SelectedTextAsset->MeshName.IsEmpty()) { return EVisibility::Visible; }
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedTextAsset->MeshName))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Hidden;
}