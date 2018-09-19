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

//#define LOCTEXT_NAMESPACE "SkyLightComponentDetails"

TSharedRef<IDetailCustomization> UDynamicObjectComponentDetails::MakeInstance()
{
	return MakeShareable( new UDynamicObjectComponentDetails);
}

void UDynamicObjectComponentDetails::CustomizeDetails( IDetailLayoutBuilder& DetailLayout )
{
	//const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetSelectedObjects(); 4.18
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

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
			.Text(FText::FromString("Generate Meshname and Unique ID") )
		]
	];
}

bool UDynamicObjectComponentDetails::HasOwner() const
{
	return SelectedDynamicObject.Get() != NULL && SelectedDynamicObject.Get()->GetOwner() != NULL;
}

FReply UDynamicObjectComponentDetails::OnUpdateMeshAndId()
{
	if (SelectedDynamicObject.IsValid())
	{
		SelectedDynamicObject->TryGenerateCustomIdAndMesh();
	}

	return FReply::Handled();
}