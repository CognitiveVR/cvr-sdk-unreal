

#include "CognitiveVREditorModule.h"
#include "C3DCommands.h"

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

TSharedRef<SDockTab> CreateCognitiveSceneSetupTabArgs(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		SNew(SSceneSetupWidget)
	];
}

TSharedRef<SDockTab> CreateCognitiveProjectSetupTabArgs(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SProjectSetupWidget)
		];
}

TSharedRef<SDockTab> CreateCognitiveDynamicObjectTabArgs(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SDynamicObjectManagerWidget)
		];
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
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("CustomEventAutoTimer"), 30, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicDataLimit"), 64, EngineIni);
		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("DynamicAutoTimer"), 30, EngineIni);

		GConfig->SetInt(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SensorDataLimit"), 64, EngineIni);
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

	//do i need to unregister this dynamic object pool action?
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedPtr< FDynamicIdPoolAssetActions> action = MakeShared<FDynamicIdPoolAssetActions>();
	AssetTools.RegisterAssetTypeActions(action.ToSharedRef());
	
	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, EditorIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("BlenderPath"), FCognitiveEditorTools::GetInstance()->BlenderPath, EditorIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, EditorIni);

	FCognitive3DCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);

	//map all the menu items to functions
	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenProjectSetupWindow,
		FExecuteAction::CreateStatic(&FCognitiveVREditorModule::SpawnCognitiveProjectSetupTab)
	);

	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenSceneSetupWindow,
		FExecuteAction::CreateStatic(&FCognitiveVREditorModule::SpawnCognitiveSceneSetupTab)
	);

	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenDynamicObjectWindow,
		FExecuteAction::CreateStatic(&FCognitiveVREditorModule::SpawnCognitiveDynamicTab)
	);

	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenOnlineDocumentation,
		FExecuteAction::CreateStatic(&FCognitiveVREditorModule::OpenOnlineDocumentation)
	);

	//append the menu after help
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuBarExtension(
		"Help",
		EExtensionHook::After,
		PluginCommands,
		FMenuBarExtensionDelegate::CreateStatic(&FCognitiveVREditorModule::AddMenu)
	);
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	// Register the details customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	PropertyModule.RegisterCustomClassLayout(TEXT("CognitiveVRSettings"), FOnGetDetailCustomizationInstance::CreateStatic(&FCognitiveSettingsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(TEXT("DynamicObject"), FOnGetDetailCustomizationInstance::CreateStatic(&UDynamicObjectComponentDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(TEXT("DynamicIdPoolAsset"), FOnGetDetailCustomizationInstance::CreateStatic(&UDynamicIdPoolAssetDetails::MakeInstance));
#endif
}

	//defines the menu to add the contents of the menu
	void FCognitiveVREditorModule::AddMenu(FMenuBarBuilder& MenuBuilder)
	{
		MenuBuilder.AddPullDownMenu(
			LOCTEXT("MenuLocKey", "Cognitive3D"),
			LOCTEXT("MenuTooltipKey", "Open Cognitive3D menu"),
			FNewMenuDelegate::CreateStatic(&FCognitiveVREditorModule::FillMenu),
			FName(TEXT("Cognitive3D")),
			FName(TEXT("Cognitive3DName")));
	}

	void FCognitiveVREditorModule::FillMenu(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenDynamicObjectWindow);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenSceneSetupWindow);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenProjectSetupWindow);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenOnlineDocumentation);
	}

void FCognitiveVREditorModule::ShutdownModule()
{
	FCognitive3DCommands::Unregister();
}

bool FCognitiveVREditorModule::SupportsDynamicReloading()
{
	return true;
}

void FCognitiveVREditorModule::SpawnCognitiveDynamicTab()
{
	FTabId tabId = FTabId(FName("CognitiveDynamicObjectManager"));
	auto foundTab = FGlobalTabmanager::Get()->FindExistingLiveTab(tabId);
	if (foundTab.IsValid())
	{
		FGlobalTabmanager::Get()->SetActiveTab(foundTab);
	}
	else
	{
		TSharedPtr<SDockTab> MajorTab = FGlobalTabmanager::Get()->TryInvokeTab(tabId);
		MajorTab->SetContent(SNew(SDynamicObjectManagerWidget));
	}
}

void FCognitiveVREditorModule::SpawnCognitiveSceneSetupTab()
{
	FTabId tabId = FTabId(FName("CognitiveSceneSetup"));
	auto foundTab = FGlobalTabmanager::Get()->FindExistingLiveTab(tabId);
	if (foundTab.IsValid())
	{
		FGlobalTabmanager::Get()->SetActiveTab(foundTab);
	}
	else
	{
		TSharedPtr<SDockTab> MajorTab = FGlobalTabmanager::Get()->TryInvokeTab(tabId);
		MajorTab->SetContent(SNew(SSceneSetupWidget));
	}
}

void FCognitiveVREditorModule::SpawnCognitiveProjectSetupTab()
{
	FTabId tabId = FTabId(FName("CognitiveProjectSetup"));
	auto foundTab = FGlobalTabmanager::Get()->FindExistingLiveTab(tabId);
	if (foundTab.IsValid())
	{
		FGlobalTabmanager::Get()->SetActiveTab(foundTab);
	}
	else
	{
		TSharedPtr<SDockTab> MajorTab = FGlobalTabmanager::Get()->TryInvokeTab(tabId);
		MajorTab->SetContent(SNew(SProjectSetupWidget));
	}
}

void FCognitiveVREditorModule::OpenOnlineDocumentation()
{
	FString url = "https://docs.cognitive3d.com/unreal/get-started/";
	FPlatformProcess::LaunchURL(*url, nullptr, nullptr);
}

void FCognitiveVREditorModule::CloseProjectSetupWindow()
{
	FTabId projectTabId = FTabId(FName("CognitiveProjectSetup"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
}

void FCognitiveVREditorModule::CloseSceneSetupWindow()
{
	FTabId projectTabId = FTabId(FName("CognitiveSceneSetup"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
}

void FCognitiveVREditorModule::CloseDynamicObjectWindow()
{
	FTabId projectTabId = FTabId(FName("CognitiveDynamicObjectManager"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
}

#undef LOCTEXT_NAMESPACE
