#pragma once

#include "CognitiveTools.h"
#include "BaseEditorTool.h"
#include "Editor/DetailCustomizations/Private/DetailCustomizationsPrivatePCH.h"

//customizes SceneSetupWindow. Displays buttons to limit number of calls on 

class FSetupCustomization : public IDetailCustomization
{

public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> MakeInstance();
};