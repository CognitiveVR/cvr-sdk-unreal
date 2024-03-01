#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "LevelEditor.h"
#include "CognitiveEditorTools.h"
#include "SDockTab.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "Editor/EditorStyle/Public/EditorStyleSet.h"
#include "SSceneSetupWidget.h"
#include "SProjectSetupWidget.h"
#include "DynamicObjectManagerWidget.h"
#include "WorkspaceMenuStructure.h"
#include "FCognitiveSettingsCustomization.h"
#include "Containers/Ticker.h"
#include "DynamicComponentDetails.h"
#include "DynamicIdPoolAssetDetails.h"
#include "DynamicIdPoolAssetActions.h"

class FCognitive3DEditorModule : public IModuleInterface, IHasMenuExtensibility, IHasToolBarExtensibility
{
public:
	static void SpawnCognitiveSceneSetupTab();
	static void SpawnCognitiveDynamicTab();
	static void SpawnCognitiveProjectSetupTab();
	static void OpenOnlineDocumentation();
	static void OpenCognitiveDashboard();
	static void CloseProjectSetupWindow();
	static void CloseSceneSetupWindow();
	static void CloseDynamicObjectWindow();

	static inline FCognitive3DEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked< FCognitive3DEditorModule >("Cognitive3DEditor");
	}

private:

	/** Holds the menu extensibility manager. */
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;

	/** Holds the tool bar extensibility manager. */
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;

	FDelegateHandle LevelEditorTabManagerChangedHandle;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override;
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override;
	virtual bool SupportsDynamicReloading() override;

	TSharedPtr<FUICommandList> PluginCommands;
	static void AddMenu(FMenuBarBuilder& MenuBuilder);
	static void FillMenu(FMenuBuilder& MenuBuilder);
};