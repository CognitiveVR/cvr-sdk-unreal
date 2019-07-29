#pragma once

#include "CognitiveVREditorPrivatePCH.h"
#include "BaseEditorTool.h"
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

	bool HasOwner() const;
	bool HasOwnerAndExportDir() const;
};