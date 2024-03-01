#pragma once

// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DynamicIdPoolAssetDetails.h"

TSharedRef<IDetailCustomization> UDynamicIdPoolAssetDetails::MakeInstance()
{
	FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
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

	//IMPROVEMENT add export directory field?

	DetailLayout.EditCategory("Scene Explorer")
	.AddCustomRow(NSLOCTEXT("SkyLightDetails", "UpdateSkyLight", "Recapture Scene"))
	.ValueContent()
	.MaxDesiredWidth(500.f)
	.MinDesiredWidth(400.f)
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.LightGroupBorder"))
		.Visibility_Raw(this, &UDynamicIdPoolAssetDetails::HasMeshBeenExported)
		.Padding(8.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Visibility_Raw(this, &UDynamicIdPoolAssetDetails::HasMeshBeenExported)
				.Image(FEditorStyle::GetBrush("SettingsEditor.WarningIcon"))
			]
				// Notice
			+SHorizontalBox::Slot()
			.Padding(16.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::Black)
				.Visibility_Raw(this, &UDynamicIdPoolAssetDetails::HasMeshBeenExported)
				.AutoWrapText(true)
				.Text(FText::FromString("Check that the Mesh has been exported.\nThis window will only upload Ids for objects for aggregation.\nA mesh should already by uploaded to visualize these objects on the dashboard."))
			]
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
			.Text(FText::FromString("Add Id") )
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