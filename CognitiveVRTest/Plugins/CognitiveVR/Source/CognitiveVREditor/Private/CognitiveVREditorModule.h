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

class FCognitiveVREditorModule : public IModuleInterface, IHasMenuExtensibility, IHasToolBarExtensibility
{
public:

	static TSharedRef<SDockTab> SpawnCognitiveSceneSetupTab(const FSpawnTabArgs& SpawnTabArgs);
	static void SpawnCognitiveSceneSetupTab();
	static TSharedRef<SDockTab> SpawnCognitiveDynamicTab(const FSpawnTabArgs& SpawnTabArgs);
	static void SpawnCognitiveDynamicTab();
	static TSharedRef<SDockTab> SpawnCognitiveProjectSetupTab(const FSpawnTabArgs& SpawnTabArgs);

	static inline FCognitiveVREditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked< FCognitiveVREditorModule >("CognitiveVREditor");
	}

private:

	/** Holds the menu extensibility manager. */
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;

	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;

	/** Holds the plug-ins style set. */
	//TSharedPtr<ISlateStyle> Style;

	/** Holds the tool bar extensibility manager. */
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;

	FDelegateHandle LevelEditorTabManagerChangedHandle;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override;
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override;
	virtual bool SupportsDynamicReloading() override;

	void RegisterMenuExtensions();
	void UnregisterMenuExtensions();

	void UnregisterAssetTools();
	static void CloseProjectSetupWindow();
};