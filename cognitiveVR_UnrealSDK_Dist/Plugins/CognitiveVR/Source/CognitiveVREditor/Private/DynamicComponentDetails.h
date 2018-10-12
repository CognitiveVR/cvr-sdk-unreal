#pragma once

#include "CognitiveVREditorPrivatePCH.h"
#include "BaseEditorTool.h"
#include "UObject/WeakObjectPtr.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class UDynamicObject;

class UDynamicObjectComponentDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailLayout ) override;
	FReply OnUpdateMeshAndId();
	FReply TakeScreenshot();
	FReply ExportAndUpload();
	private:
	TWeakObjectPtr<UDynamicObject> SelectedDynamicObject;

	bool HasOwner() const;
	bool HasOwnerAndExportDir() const;
};