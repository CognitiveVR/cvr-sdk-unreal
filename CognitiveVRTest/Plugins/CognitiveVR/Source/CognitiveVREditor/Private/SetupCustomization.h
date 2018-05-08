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
	
	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute);

	int32 CurrentPage;
	EVisibility ShouldDisplay(int32 page);

	FReply ToolsDebugButton();
};