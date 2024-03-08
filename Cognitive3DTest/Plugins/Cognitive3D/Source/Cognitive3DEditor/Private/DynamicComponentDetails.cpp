#pragma once

/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "DynamicComponentDetails.h"

TSharedRef<IDetailCustomization> IDynamicObjectComponentDetails::MakeInstance()
{
	return MakeShareable( new IDynamicObjectComponentDetails);
}

void IDynamicObjectComponentDetails::CustomizeDetails( IDetailLayoutBuilder& DetailLayout )
{
	TArray< TWeakObjectPtr<UObject> > SelectedObjects; //= DetailLayout.GetSelectedObjects();
	DetailLayout.GetObjectsBeingCustomized(SelectedObjects);

	for( int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex )
	{
		TWeakObjectPtr<UObject> CurrentObject = SelectedObjects[ObjectIndex];
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
		.IsEnabled_Raw(this, &IDynamicObjectComponentDetails::HasOwnerAndExportDirAndName)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(this, &IDynamicObjectComponentDetails::GetExportTooltip)
		.OnClicked(this, &IDynamicObjectComponentDetails::Export)
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
		.IsEnabled_Raw(this, &IDynamicObjectComponentDetails::HasOwnerAndMeshExportDirAndName)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(this, &IDynamicObjectComponentDetails::GetScreenshotTooltip)
		.OnClicked(this, &IDynamicObjectComponentDetails::TakeScreenshot)
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
		.IsEnabled_Raw(this, &IDynamicObjectComponentDetails::HasExportAndValidSceneData)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(this,&IDynamicObjectComponentDetails::GetUploadTooltip)
		.OnClicked(this, &IDynamicObjectComponentDetails::Upload)
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
			.ToolTipText(this, &IDynamicObjectComponentDetails::HandSetupText)
			.OnClicked(this, &IDynamicObjectComponentDetails::SetLeftHand)
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
			.ToolTipText(this, &IDynamicObjectComponentDetails::HandSetupText)
			.OnClicked(this, &IDynamicObjectComponentDetails::SetRightHand)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(FText::FromString("Set Right Hand"))
			]
		]		
	];
}

bool IDynamicObjectComponentDetails::HasOwner() const
{
	return SelectedDynamicObject.Get() != NULL && (SelectedDynamicObject.Get()->GetAttachParent() != NULL);
}

bool IDynamicObjectComponentDetails::HasOwnerAndExportDirAndName() const
{
	if (!HasOwner()) { UE_LOG(LogTemp, Warning, TEXT("NO OWNER FOUND!")); return false; }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { UE_LOG(LogTemp, Warning, TEXT("NO EXPORT DIRECTORY FOUND!")); return false; }
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { UE_LOG(LogTemp, Warning, TEXT("NO MESH NAME FOUND!")); return false; }
	return true;
}

bool IDynamicObjectComponentDetails::HasOwnerAndMeshExportDirAndName() const
{
	if (!HasOwner()) { return false; }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return false; }
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return false; }
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedDynamicObject.Get()->MeshName)) { return false; } //mesh directory exists
	return true;
}

bool IDynamicObjectComponentDetails::HasExportAndValidSceneData() const
{
	if (!HasOwner()) { return false; }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return false; } //base export directory exists
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return false; } //mesh name is valid
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedDynamicObject.Get()->MeshName)) { return false; } //mesh directory exists
	if (!FCognitiveEditorTools::GetInstance()->GetCurrentSceneData().IsValid()) { return false; } //scene is valid
	return true;
}

FText IDynamicObjectComponentDetails::GetUploadTooltip() const
{
	if (!HasOwner()) { return FText::FromString(""); }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return FText::FromString("Export Directory doesn't exist"); }
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return FText::FromString("MeshName should not be empty"); }
	if (!FCognitiveEditorTools::GetInstance()->GetCurrentSceneData().IsValid()) { return FText::FromString("Scene Data is invalid"); }
	return FText::FromString("Upload the Dynamic Object Mesh to the current scene");
}

FText IDynamicObjectComponentDetails::HandSetupText() const
{
	return FText::FromString("This will configure the Dynamic Object for a specific hand. Make sure to select the Controller Type!");
}

FText IDynamicObjectComponentDetails::GetExportTooltip() const
{
	if (!HasOwner()) { return FText::FromString(""); }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return FText::FromString("Export Directory doesn't exist"); } //base export directory exists
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return FText::FromString("MeshName should not be empty"); } //mesh name is valid
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedDynamicObject.Get()->MeshName)) { return FText::FromString("Export Directory doesn't exist"); } //mesh directory exists
	return FText::FromString("Exports the Dynamic Object Mesh to a temporary folder");
}

FText IDynamicObjectComponentDetails::GetScreenshotTooltip() const
{
	if (!HasOwner()) { return FText::FromString(""); }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return FText::FromString("Export Directory doesn't exist"); } //base export directory exists
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return FText::FromString("MeshName should not be empty"); } //mesh name is valid
	if (!FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(SelectedDynamicObject.Get()->MeshName)) { return FText::FromString("Export Directory doesn't exist"); } //mesh directory exists
	return FText::FromString("Saves a screenshot of this Dynamic Object Mesh from the level viewport to a temporary folder");
}

FReply IDynamicObjectComponentDetails::SetRightHand()
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

FReply IDynamicObjectComponentDetails::SetLeftHand()
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

FReply IDynamicObjectComponentDetails::TakeScreenshot()
{
	FCognitiveEditorTools::GetInstance()->TakeDynamicScreenshot(SelectedDynamicObject->MeshName);
	return FReply::Handled();
}

FReply IDynamicObjectComponentDetails::Export()
{
	GEditor->SelectNone(false, true, false);

	GEditor->SelectActor(SelectedDynamicObject->GetOwner(), true, false, true);

	FCognitiveEditorTools::GetInstance()->ExportSelectedDynamics();

	return FReply::Handled();
}

FReply IDynamicObjectComponentDetails::Upload()
{
	FCognitiveEditorTools::GetInstance()->UploadDynamic(SelectedDynamicObject->MeshName);

	return FReply::Handled();
}
