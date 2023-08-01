

#include "CognitiveVREditorModule.h"

IMPLEMENT_MODULE(FCognitiveVREditorModule, CognitiveVREditor);

//sets up customization for settings
//adds scene setup window
//creates editortools

#define LOCTEXT_NAMESPACE "DemoTools"

TSharedPtr<FExtensibilityManager> FCognitiveVREditorModule::GetMenuExtensibilityManager()
{
	return MenuExtensibilityManager;
}

TSharedPtr<FExtensibilityManager> FCognitiveVREditorModule::GetToolBarExtensibilityManager()
{
	return ToolBarExtensibilityManager;
}

void FCognitiveVREditorModule::StartupModule()
{
#if WITH_EDITOR
	// Create the Extender that will add content to the menu
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));

	FString tempGateway;
	GConfig->GetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("Gateway"), tempGateway, EngineIni);

	if (tempGateway.IsEmpty())
	{
		GLog->Log("CognitiveVRModule::StartupModule write defaults to ini");
		FString defaultgateway = "data.cognitive3d.com";
		FString defaultsessionviewer = "viewer.cognitive3d.com/scene/";
		FString defaultdashboard = "app.cognitive3d.com";
		FString trueString = "True";
		GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("Gateway"), *defaultgateway, EngineIni);
		GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SessionViewer"), *defaultsessionviewer, EngineIni);
		GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("Dashboard"), *defaultdashboard, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("GazeBatchSize"), 64, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("CustomEventBatchSize"), 64, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("CustomEventExtremeLimit"), 128, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("CustomEventMinTimer"), 2, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("CustomEventAutoTimer"), 30, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicDataLimit"), 64, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicExtremeLimit"), 128, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicMinTimer"), 2, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicAutoTimer"), 30, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SensorDataLimit"), 64, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SensorExtremeLimit"), 128, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SensorMinTimer"), 2, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SensorAutoTimer"), 30, EngineIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsDebug"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsTest"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsDevelopment"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsDebug"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsTest"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->SetString(TEXT("AnalyticsDevelopment"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);

		GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("EnableLocalCache"), *trueString, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("LocalCacheSize"), 100, EngineIni);
	}

	FCognitiveEditorTools::Initialize();
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedPtr< FDynamicIdPoolAssetActions> action = MakeShared<FDynamicIdPoolAssetActions>();
	AssetTools.RegisterAssetTypeActions(action.ToSharedRef());
	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, EditorIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("BlenderPath"), FCognitiveEditorTools::GetInstance()->BlenderPath, EditorIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, EditorIni);

	// register standalone UI
	LevelEditorTabManagerChangedHandle = LevelEditorModule.OnTabManagerChanged().AddLambda([]()
		{
			FLevelEditorModule& LocalLevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
			LocalLevelEditorModule.GetLevelEditorTabManager()->RegisterTabSpawner(FName("CognitiveSceneSetup"), FOnSpawnTab::CreateStatic(&FCognitiveVREditorModule::SpawnCognitiveSceneSetupTab))
				.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
				.SetDisplayName(LOCTEXT("SceneSetupTabTitle", "Cognitive Scene Setup"))
				.SetTooltipText(LOCTEXT("SceneSetupTooltipText", "Open the Cognitive Scene Setup Wizard"));

			LocalLevelEditorModule.GetLevelEditorTabManager()->RegisterTabSpawner(FName("CognitiveProjectSetup"), FOnSpawnTab::CreateStatic(&FCognitiveVREditorModule::SpawnCognitiveProjectSetupTab))
				.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
				.SetDisplayName(LOCTEXT("ProjectSetupTabTitle", "Cognitive Project Setup"))
				.SetTooltipText(LOCTEXT("ProjectSetupTooltipText", "Open the Cognitive Project Setup Wizard"));

			LocalLevelEditorModule.GetLevelEditorTabManager()->RegisterTabSpawner(FName("CognitiveDynamicObjectManager"), FOnSpawnTab::CreateStatic(&FCognitiveVREditorModule::SpawnCognitiveDynamicTab))
				.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
				.SetDisplayName(LOCTEXT("DynamicObjectManagerTabTitle", "Cognitive Dynamic Object Manager"))
				.SetTooltipText(LOCTEXT("DynamicObjectManagerTooltipText", "Open the Cognitive Dynamic Object Manager"));
		});

	// Register the details customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	PropertyModule.RegisterCustomClassLayout(TEXT("CognitiveVRSettings"), FOnGetDetailCustomizationInstance::CreateStatic(&FCognitiveSettingsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(TEXT("DynamicObject"), FOnGetDetailCustomizationInstance::CreateStatic(&UDynamicObjectComponentDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(TEXT("DynamicIdPoolAsset"), FOnGetDetailCustomizationInstance::CreateStatic(&UDynamicIdPoolAssetDetails::MakeInstance));
#endif
}

void FCognitiveVREditorModule::ShutdownModule()
{
	//TODO why is this wrapped in 'WITH_EDITOR'? this module should only ever exist in the editor
#if WITH_EDITOR

	if (GEditor)
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("LevelEditor")))
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
			LevelEditorModule.OnTabManagerChanged().Remove(LevelEditorTabManagerChangedHandle);
		}
	}
#endif
}

bool FCognitiveVREditorModule::SupportsDynamicReloading()
{
	return true;
}

TSharedRef<SDockTab> FCognitiveVREditorModule::SpawnCognitiveDynamicTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FLevelEditorModule& LocalLevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	FTabId tabId = FTabId(FName("CognitiveDynamicObjectManager"));
	auto foundTab = LocalLevelEditorModule.GetLevelEditorTabManager()->FindExistingLiveTab(tabId);
	if (foundTab.IsValid())
	{
		FGlobalTabmanager::Get()->SetActiveTab(foundTab);
		return foundTab.ToSharedRef();
	}
	else
	{
		const TSharedRef<SDockTab> MajorTab =
			SNew(SDockTab)
			.TabRole(ETabRole::NomadTab);

		MajorTab->SetContent(SNew(SDynamicObjectManagerWidget));

		return MajorTab;
	}
}

void FCognitiveVREditorModule::SpawnCognitiveDynamicTab()
{
	CloseProjectSetupWindow();

	FLevelEditorModule& LocalLevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	FTabId tabId = FTabId(FName("CognitiveDynamicObjectManager"));
	auto foundTab = LocalLevelEditorModule.GetLevelEditorTabManager()->FindExistingLiveTab(tabId);
	if (foundTab.IsValid())
	{
		FGlobalTabmanager::Get()->SetActiveTab(foundTab);
	}
	else
	{
		TSharedPtr<SDockTab> MajorTab = LocalLevelEditorModule.GetLevelEditorTabManager()->TryInvokeTab(tabId);
		MajorTab->SetContent(SNew(SDynamicObjectManagerWidget));
	}
}

TSharedRef<SDockTab> FCognitiveVREditorModule::SpawnCognitiveSceneSetupTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FLevelEditorModule& LocalLevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	FTabId tabId = FTabId(FName("CognitiveSceneSetup"));
	auto foundTab = LocalLevelEditorModule.GetLevelEditorTabManager()->FindExistingLiveTab(tabId);
	if (foundTab.IsValid())
	{
		FGlobalTabmanager::Get()->SetActiveTab(foundTab);
		return foundTab.ToSharedRef();
	}
	else
	{
		const TSharedRef<SDockTab> MajorTab =
			SNew(SDockTab)
			.TabRole(ETabRole::NomadTab);

		MajorTab->SetContent(SNew(SSceneSetupWidget));

		return MajorTab;
	}
}

void FCognitiveVREditorModule::SpawnCognitiveSceneSetupTab()
{
	CloseProjectSetupWindow();

	FLevelEditorModule& LocalLevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	FTabId tabId = FTabId(FName("CognitiveSceneSetup"));
	auto foundTab = LocalLevelEditorModule.GetLevelEditorTabManager()->FindExistingLiveTab(tabId);
	if (foundTab.IsValid())
	{
		FGlobalTabmanager::Get()->SetActiveTab(foundTab);
	}
	else
	{
		TSharedPtr<SDockTab> MajorTab = LocalLevelEditorModule.GetLevelEditorTabManager()->TryInvokeTab(tabId);
		MajorTab->SetContent(SNew(SSceneSetupWidget));
	}
}

TSharedRef<SDockTab> FCognitiveVREditorModule::SpawnCognitiveProjectSetupTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> MajorTab =
		SNew(SDockTab)
		.Tag(FName("CognitiveProjectSetup"))
		.TabRole(ETabRole::NomadTab);

	MajorTab->SetContent(SNew(SProjectSetupWidget));

	return MajorTab;
}

void FCognitiveVREditorModule::SpawnCognitiveProjectSetupTab()
{
	FLevelEditorModule& LocalLevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	FTabId tabId = FTabId(FName("CognitiveProjectSetup"));
	auto foundTab = LocalLevelEditorModule.GetLevelEditorTabManager()->FindExistingLiveTab(tabId);
	if (foundTab.IsValid())
	{
		FGlobalTabmanager::Get()->SetActiveTab(foundTab);
	}
	else
	{
		TSharedPtr<SDockTab> MajorTab = LocalLevelEditorModule.GetLevelEditorTabManager()->TryInvokeTab(tabId);
		MajorTab->SetContent(SNew(SProjectSetupWidget));
	}
}

void FCognitiveVREditorModule::CloseProjectSetupWindow()
{
	FLevelEditorModule& LocalLevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	FTabId projectTabId = FTabId(FName("CognitiveProjectSetup"));
	auto projectTab = LocalLevelEditorModule.GetLevelEditorTabManager()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
}

/** Unregisters asset tool actions. */
void FCognitiveVREditorModule::UnregisterAssetTools()
{
	FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

	if (AssetToolsModule != nullptr)
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();

		for (auto Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
}

/** Registers main menu and tool bar menu extensions. */
void FCognitiveVREditorModule::RegisterMenuExtensions()
{
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
}

/** Unregisters main menu and tool bar menu extensions. */
void FCognitiveVREditorModule::UnregisterMenuExtensions()
{
	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();
}

#undef LOCTEXT_NAMESPACE
