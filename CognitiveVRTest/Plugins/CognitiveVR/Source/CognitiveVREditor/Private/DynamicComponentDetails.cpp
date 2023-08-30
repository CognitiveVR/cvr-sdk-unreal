#pragma once

// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DynamicComponentDetails.h"

TSharedRef<IDetailCustomization> UDynamicObjectComponentDetails::MakeInstance()
{
	return MakeShareable( new UDynamicObjectComponentDetails);
}

void UDynamicObjectComponentDetails::CustomizeDetails( IDetailLayoutBuilder& DetailLayout )
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetSelectedObjects();

	for( int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex )
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if ( CurrentObject.IsValid() )
		{
			UDynamicObject* CurrentCaptureActor = Cast<UDynamicObject>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				SelectedDynamicObject = CurrentCaptureActor;
				break;
			}
		}
	}

	DetailLayout.EditCategory( "DynamicObject" )
	.AddCustomRow( NSLOCTEXT("SkyLightDetails", "UpdateSkyLight", "Recapture Scene") )
	.ValueContent()
	.MaxDesiredWidth(200.f)
	.MinDesiredWidth(200.f)
	[
		SNew(SButton)
		.ContentPadding(1)
		.IsEnabled_Raw(this, &UDynamicObjectComponentDetails::HasOwnerAndExportDirAndName)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(this, &UDynamicObjectComponentDetails::GetExportTooltip)
		.OnClicked(this, &UDynamicObjectComponentDetails::Export)
		[
			SNew( STextBlock )
			.Font( IDetailLayoutBuilder::GetDetailFont() )
			.Text(FText::FromString("Export Mesh") )
		]
	];
	DetailLayout.EditCategory( "DynamicObject" )
	.AddCustomRow( NSLOCTEXT("SkyLightDetails", "UpdateSkyLight", "Recapture Scene") )
	.ValueContent()
	.MaxDesiredWidth(200.f)
	.MinDesiredWidth(200.f)
	[
		SNew(SButton)
		.ContentPadding(1)
		.IsEnabled_Raw(this, &UDynamicObjectComponentDetails::HasOwnerAndMeshExportDirAndName)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(this, &UDynamicObjectComponentDetails::GetScreenshotTooltip)
		.OnClicked(this, &UDynamicObjectComponentDetails::TakeScreenshot)
		[
			SNew( STextBlock )
			.Font( IDetailLayoutBuilder::GetDetailFont() )
			.Text(FText::FromString("Take Screenshot") )
		]
	];
	DetailLayout.EditCategory( "DynamicObject" )
	.AddCustomRow( NSLOCTEXT("SkyLightDetails", "UpdateSkyLight", "Recapture Scene") )
	.ValueContent()
	.MaxDesiredWidth(200.f)
	.MinDesiredWidth(200.f)
	[
		SNew(SButton)
		.ContentPadding(1)
		.IsEnabled_Raw(this, &UDynamicObjectComponentDetails::HasExportAndValidSceneData)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(this,&UDynamicObjectComponentDetails::GetUploadTooltip)
		.OnClicked(this, &UDynamicObjectComponentDetails::Upload)
		[
			SNew( STextBlock )
			.Font( IDetailLayoutBuilder::GetDetailFont() )
			.Text(FText::FromString("Upload Mesh") )
		]
	];
	DetailLayout.EditCategory( "DynamicObject" )
	.AddCustomRow( NSLOCTEXT("SkyLightDetails", "UpdateSkyLight", "Recapture Scene") )
	.ValueContent()
	.MaxDesiredWidth(200.f)
	.MinDesiredWidth(200.f)
	[
		SNew(SHorizontalBox)		
		+SHorizontalBox::Slot()
		[
			SNew(SButton)
			.ContentPadding(1)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.ToolTipText(this, &UDynamicObjectComponentDetails::HandSetupText)
			.OnClicked(this, &UDynamicObjectComponentDetails::SetLeftHand)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(FText::FromString("Set Left Hand"))
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SButton)
			.ContentPadding(1)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.ToolTipText(this, &UDynamicObjectComponentDetails::HandSetupText)
			.OnClicked(this, &UDynamicObjectComponentDetails::SetRightHand)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(FText::FromString("Set Right Hand"))
			]
		]		
	];
}

bool UDynamicObjectComponentDetails::HasOwner() const
{
	return SelectedDynamicObject.Get() != NULL && SelectedDynamicObject.Get()->GetOwner() != NULL;
}

bool UDynamicObjectComponentDetails::HasOwnerAndExportDirAndName() const
{
	if (!HasOwner()) { return false; }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return false; }
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return false; }
	return true;
}

bool UDynamicObjectComponentDetails::HasOwnerAndMeshExportDirAndName() const
{
	if (!HasOwner()) { return false; }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return false; }
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedDynamicObject.Get()->MeshName)) { return false; } //mesh directory exists
	return true;
}

bool UDynamicObjectComponentDetails::HasExportAndValidSceneData() const
{
	if (!HasOwner()) { return false; }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return false; } //base export directory exists
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return false; } //mesh name is valid
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedDynamicObject.Get()->MeshName)) { return false; } //mesh directory exists
	if (!FCognitiveEditorTools::GetInstance()->GetCurrentSceneData().IsValid()) { return false; } //scene is valid
	return true;
}

FText UDynamicObjectComponentDetails::GetUploadTooltip() const
{
	if (!HasOwner()) { return FText::FromString(""); }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return FText::FromString("Export Directory doesn't exist"); }
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return FText::FromString("MeshName should not be empty"); }
	if (!FCognitiveEditorTools::GetInstance()->GetCurrentSceneData().IsValid()) { return FText::FromString("Scene Data is invalid"); }
	return FText::FromString("Upload the Dynamic Object Mesh to the current scene");
}

FText UDynamicObjectComponentDetails::HandSetupText() const
{
	return FText::FromString("This will configure the Dynamic Object for a specific hand. Make sure to select the Controller Type!");
}

FText UDynamicObjectComponentDetails::GetExportTooltip() const
{
	if (!HasOwner()) { return FText::FromString(""); }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return FText::FromString("Export Directory doesn't exist"); } //base export directory exists
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return FText::FromString("MeshName should not be empty"); } //mesh name is valid
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedDynamicObject.Get()->MeshName)) { return FText::FromString("Export Directory doesn't exist"); } //mesh directory exists
	return FText::FromString("Exports the Dynamic Object Mesh to a temporary folder");
}

FText UDynamicObjectComponentDetails::GetScreenshotTooltip() const
{
	if (!HasOwner()) { return FText::FromString(""); }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return FText::FromString("Export Directory doesn't exist"); } //base export directory exists
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return FText::FromString("MeshName should not be empty"); } //mesh name is valid
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedDynamicObject.Get()->MeshName)) { return FText::FromString("Export Directory doesn't exist"); } //mesh directory exists
	return FText::FromString("Saves a screenshot of this Dynamic Object Mesh from the level viewport to a temporary folder");
}

FReply UDynamicObjectComponentDetails::SetRightHand()
{
	SelectedDynamicObject.Get()->IsController = true;
	SelectedDynamicObject.Get()->IsRightController = true;
	SelectedDynamicObject.Get()->SyncUpdateWithPlayer = true;
	SelectedDynamicObject.Get()->IdSourceType = EIdSourceType::GeneratedId;
	SelectedDynamicObject.Get()->MeshName = "RightHandMesh";

	//mark package to be saved
	UWorld* world = SelectedDynamicObject.Get()->GetWorld();
	if (world != NULL)
	{
		world->MarkPackageDirty();
	}
	else
	{
		SelectedDynamicObject.Get()->MarkPackageDirty();
	}
	return FReply::Handled();
}

FReply UDynamicObjectComponentDetails::SetLeftHand()
{
	SelectedDynamicObject.Get()->IsController = true;
	SelectedDynamicObject.Get()->IsRightController = false;
	SelectedDynamicObject.Get()->SyncUpdateWithPlayer = true;
	SelectedDynamicObject.Get()->IdSourceType = EIdSourceType::GeneratedId;
	SelectedDynamicObject.Get()->MeshName = "LeftHandMesh";

	//mark package to be saved
	UWorld* world = SelectedDynamicObject.Get()->GetWorld();
	if (world != NULL)
	{
		world->MarkPackageDirty();
	}
	else
	{
		SelectedDynamicObject.Get()->MarkPackageDirty();
	}
	return FReply::Handled();
}

FReply UDynamicObjectComponentDetails::TakeScreenshot()
{
	FCognitiveEditorTools::GetInstance()->TakeDynamicScreenshot(SelectedDynamicObject->MeshName);
	return FReply::Handled();
}

FReply UDynamicObjectComponentDetails::Export()
{
	GEditor->SelectNone(false, true, false);

	GEditor->SelectActor(SelectedDynamicObject->GetOwner(), true, false, true);

	if (!FCognitiveEditorTools::GetInstance()->HasFoundBlender())
	{
		UE_LOG(CognitiveVR_Log, Error, TEXT("Could not complete Dynamic Export - Must have Blender installed to convert images"));
		return FReply::Handled();
	}

	FCognitiveEditorTools::GetInstance()->ExportSelectedDynamics();

	return FReply::Handled();
}

FReply UDynamicObjectComponentDetails::Upload()
{
	FCognitiveEditorTools::GetInstance()->UploadDynamic(SelectedDynamicObject->MeshName);

	return FReply::Handled();
}
