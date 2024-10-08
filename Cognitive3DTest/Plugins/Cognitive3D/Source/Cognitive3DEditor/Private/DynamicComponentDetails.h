/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

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
#include "coreminimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class UDynamicObject;

//custom details inspector for dynamic object component

class IDynamicObjectComponentDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailLayout ) override;
	FReply TakeScreenshot();
	FReply Export();
	FReply Upload();
	FReply SetLeftHand();
	FReply SetRightHand();
	TWeakObjectPtr<UDynamicObject> SelectedDynamicObject;

	bool HasOwner() const;
	bool HasOwnerAndExportDirAndName() const;
	bool HasOwnerAndMeshExportDirAndName() const;
	bool HasExportAndValidSceneData() const;
	FText HandSetupText() const;

	FText GetExportTooltip() const;
	FText GetScreenshotTooltip() const;
	FText GetUploadTooltip() const;
};
