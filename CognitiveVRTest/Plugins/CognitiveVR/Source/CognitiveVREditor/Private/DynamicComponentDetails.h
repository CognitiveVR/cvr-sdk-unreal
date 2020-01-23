#pragma once

//#include "CognitiveVREditorPrivatePCH.h"
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
#include "BaseEditorTool.h"
#include "coreminimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class UDynamicObject;

//custom details inspector for dynamic object component

class UDynamicObjectComponentDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailLayout ) override;
	FReply OnUpdateMeshAndId();
	FReply TakeScreenshot();
	FReply Export();
	FReply Upload();
	private:
	TWeakObjectPtr<UDynamicObject> SelectedDynamicObject;
	void DelayScreenshot(FLevelEditorViewportClient* perspectiveView, FVector startPos, FRotator startRot);

	bool HasOwner() const;
	bool HasOwnerAndExportDir() const;
	bool HasOwnerAndExportDirAndName() const;
};
