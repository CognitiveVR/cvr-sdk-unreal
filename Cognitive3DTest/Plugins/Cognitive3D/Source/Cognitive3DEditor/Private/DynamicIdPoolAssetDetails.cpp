#pragma once

/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "DynamicIdPoolAssetDetails.h"

TSharedRef<IDetailCustomization> IDynamicIdPoolAssetDetails::MakeInstance()
{
	FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
	return MakeShareable( new IDynamicIdPoolAssetDetails);
}

void IDynamicIdPoolAssetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
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
		.BorderImage(FCognitiveEditorTools::GetBrush(FName("ToolPanel.LightGroupBorder")))
		.Visibility_Raw(this, &IDynamicIdPoolAssetDetails::HasMeshBeenExported)
		.Padding(8.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Visibility_Raw(this, &IDynamicIdPoolAssetDetails::HasMeshBeenExported)
				.Image(FCognitiveEditorTools::GetBrush(FName("SettingsEditor.WarningIcon")))
			]
				// Notice
			+SHorizontalBox::Slot()
			.Padding(16.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::Black)
				.Visibility_Raw(this, &IDynamicIdPoolAssetDetails::HasMeshBeenExported)
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
		.IsEnabled_Raw(this, &IDynamicIdPoolAssetDetails::CanUploadIds)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &IDynamicIdPoolAssetDetails::Upload)
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
		.OnClicked(this, &IDynamicIdPoolAssetDetails::GenerateId)
		[
			SNew( STextBlock )
			.Font( IDetailLayoutBuilder::GetDetailFont() )
			.Text(FText::FromString("Add Id") )
		]
	];
}

FReply IDynamicIdPoolAssetDetails::Upload()
{
	UWorld* myworld = GWorld->GetWorld();

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

	FCognitiveEditorTools::GetInstance()->UploadDynamicsManifestIds(currentSceneName, SelectedTextAsset->Ids, SelectedTextAsset->MeshName, SelectedTextAsset->PrefabName);
	return FReply::Handled();
}

FReply IDynamicIdPoolAssetDetails::GenerateId()
{
	SelectedTextAsset->GenerateNewId();
	return FReply::Handled();
}

bool IDynamicIdPoolAssetDetails::CanUploadIds() const
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

EVisibility IDynamicIdPoolAssetDetails::HasMeshBeenExported() const
{
	if (SelectedTextAsset->MeshName.IsEmpty()) { return EVisibility::Visible; }
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedTextAsset->MeshName))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Hidden;
}