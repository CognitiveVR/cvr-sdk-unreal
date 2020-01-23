#pragma once

// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVREditorPrivatePCH.h"
#include "DynamicComponentDetails.h"
#include "Components/SceneComponent.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "DynamicObject.h"
#include "CognitiveEditorTools.h"

//#define LOCTEXT_NAMESPACE "SkyLightComponentDetails"

TSharedRef<IDetailCustomization> UDynamicObjectComponentDetails::MakeInstance()
{
	return MakeShareable( new UDynamicObjectComponentDetails);
}

void UDynamicObjectComponentDetails::CustomizeDetails( IDetailLayoutBuilder& DetailLayout )
{
	//const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetSelectedObjects(); 4.18
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
		.IsEnabled_Raw(this, &UDynamicObjectComponentDetails::HasOwner)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &UDynamicObjectComponentDetails::OnUpdateMeshAndId)
		[
			SNew( STextBlock )
			.Font( IDetailLayoutBuilder::GetDetailFont() )
			.Text(FText::FromString("Generate Mesh Name and Unique ID") )
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
		.IsEnabled_Raw(this, &UDynamicObjectComponentDetails::HasOwnerAndExportDirAndName)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(FText::FromString("Export Directory must be set. See CognitiveVR Settings"))
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
		.IsEnabled_Raw(this, &UDynamicObjectComponentDetails::HasOwnerAndExportDirAndName)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(FText::FromString("Export Directory must be set. See CognitiveVR Settings"))
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
		.IsEnabled_Raw(this, &UDynamicObjectComponentDetails::HasOwnerAndExportDirAndName)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.ToolTipText(FText::FromString("Export Directory must be set. See CognitiveVR Settings"))
		.OnClicked(this, &UDynamicObjectComponentDetails::Upload)
		[
			SNew( STextBlock )
			.Font( IDetailLayoutBuilder::GetDetailFont() )
			.Text(FText::FromString("Upload Mesh") )
		]
	];
}

bool UDynamicObjectComponentDetails::HasOwner() const
{
	return SelectedDynamicObject.Get() != NULL && SelectedDynamicObject.Get()->GetOwner() != NULL;
}

bool UDynamicObjectComponentDetails::HasOwnerAndExportDir() const
{
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return false; }
	return HasOwner();
}
bool UDynamicObjectComponentDetails::HasOwnerAndExportDirAndName() const
{
	if (!HasOwner()) { return false; }
	if (FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory().IsEmpty()) { return false; }
	if (SelectedDynamicObject.Get()->MeshName.IsEmpty()) { return false; }
	return true;
}

FReply UDynamicObjectComponentDetails::OnUpdateMeshAndId()
{
	if (SelectedDynamicObject.IsValid())
	{
		SelectedDynamicObject->TryGenerateCustomIdAndMesh();
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
