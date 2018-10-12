#pragma once

#include "CognitiveEditorTools.h"
#include "BaseEditorTool.h"
#include "SceneSetupWindow.generated.h"

//this is a container class to be customized. all functionality will stay in cognitivetools

UCLASS(Blueprintable)
class USceneSetupWindow : public UBaseEditorTool
{
	GENERATED_BODY()

public:

	//UFUNCTION(Exec, Category = "Export")
		//void ExportScene();

	//virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	//static TSharedRef<IDetailCustomization> MakeInstance();

};