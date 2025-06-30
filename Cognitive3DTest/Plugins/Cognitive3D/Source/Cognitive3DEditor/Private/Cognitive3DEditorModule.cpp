/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3DEditorModule.h"
#include "C3DCommands.h"
#include "CognitiveEditorStyle.h"

IMPLEMENT_MODULE(FCognitive3DEditorModule, Cognitive3DEditor);

//sets up customization for settings
//adds scene setup window
//creates editortools

#define LOCTEXT_NAMESPACE "DemoTools"

TSharedPtr<FExtensibilityManager> FCognitive3DEditorModule::GetMenuExtensibilityManager()
{
	return MenuExtensibilityManager;
}

TSharedPtr<FExtensibilityManager> FCognitive3DEditorModule::GetToolBarExtensibilityManager()
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

TSharedRef<SDockTab> CreateCognitiveFullSetupTabArgs(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SProjectManagerWidget)
		];
}

TSharedRef<SDockTab> CreateCognitiveFeatureBuilderTabArgs(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SFeatureBuilderWidget)
		];
}

void FCognitive3DEditorModule::StartupModule()
{
#if WITH_EDITOR
	// Create the Extender that will add content to the menu
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	FCognitiveEditorTools::Initialize();

	FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
	FString C3DKeysPath = FCognitiveEditorTools::GetInstance()->GetKeysFilePath();
	GConfig->LoadFile(C3DSettingsPath);
	GConfig->LoadFile(C3DKeysPath);
	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, C3DSettingsPath);
	GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, C3DSettingsPath);
	GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, C3DKeysPath);
	GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, C3DSettingsPath);
	GConfig->Flush(false, C3DSettingsPath);

	FCognitiveEditorStyle::Initialize();

	// Grab the singleton packaging-settings object (its Config=Game, defaultconfig)
	UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();

	// Build a directory entry pointing at your folder (relative to /Game or absolute as needed)
	FDirectoryPath NeverCookDir;
	FString FolderToExclude = C3DKeysPath;
	NeverCookDir.Path = FPaths::ConvertRelativePathToFull(C3DKeysPath);

	// 1) Remove it from "Copy as loose files"
	PackagingSettings->DirectoriesToAlwaysStageAsNonUFS.RemoveAll(
		[FolderToExclude](const FDirectoryPath& Entry)
		{
			return Entry.Path.Equals(FolderToExclude, ESearchCase::IgnoreCase);
		}
	);

	// 2) (Just in case) Remove from "Package inside the .pak"
	PackagingSettings->DirectoriesToAlwaysStageAsUFS.RemoveAll(
		[FolderToExclude](const FDirectoryPath& Entry)
		{
			return Entry.Path.Equals(FolderToExclude, ESearchCase::IgnoreCase);
		}
	);

	// Persist the change back to DefaultGame.ini (or DefaultGame.ini -> [YourProject]/Config)
	PackagingSettings->SaveConfig();

	GConfig->Flush(false, C3DKeysPath);

	if (FCognitiveEditorTools::GetInstance()->DeveloperKey.IsEmpty() || FCognitiveEditorTools::GetInstance()->ApplicationKey.IsEmpty()
		|| FCognitiveEditorTools::GetInstance()->BaseExportDirectory.IsEmpty())
	{
		FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
		FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
		const FString NormalizedEngineIni = GConfig->NormalizeConfigIniPath(EngineIni);
		const FString NormalizedEditorIni = GConfig->NormalizeConfigIniPath(EditorIni);

		GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, NormalizedEngineIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, NormalizedEngineIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, NormalizedEditorIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, NormalizedEditorIni);

		//
		GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, GGameIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, GGameIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, GGameIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, GGameIni);

		GConfig->Flush(false, GGameIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ApiKey"), *FCognitiveEditorTools::GetInstance()->ApplicationKey, C3DSettingsPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("AttributionKey"), *FCognitiveEditorTools::GetInstance()->AttributionKey, C3DSettingsPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("DeveloperKey"), *FCognitiveEditorTools::GetInstance()->DeveloperKey, C3DKeysPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("ExportPath"), *FCognitiveEditorTools::GetInstance()->BaseExportDirectory, C3DSettingsPath);
		GConfig->Flush(false, C3DSettingsPath);
		GConfig->Flush(false, C3DKeysPath);
#else
		GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, EngineIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, EngineIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, EditorIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, EditorIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ApiKey"), *FCognitiveEditorTools::GetInstance()->ApplicationKey, C3DSettingsPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("AttributionKey"), *FCognitiveEditorTools::GetInstance()->AttributionKey, C3DSettingsPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("DeveloperKey"), *FCognitiveEditorTools::GetInstance()->DeveloperKey, C3DKeysPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("ExportPath"), *FCognitiveEditorTools::GetInstance()->BaseExportDirectory, C3DSettingsPath);
		GConfig->Flush(false, C3DSettingsPath);
		GConfig->Flush(false, C3DKeysPath);
#endif // ENGINE_MAJOR_VERSION == 4
	}

	//add actions for the dynamic object id pool asset
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedPtr< FDynamicIdPoolAssetActions> action = MakeShared<FDynamicIdPoolAssetActions>();
	AssetTools.RegisterAssetTypeActions(action.ToSharedRef());

	//register window spawning
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("CognitiveSceneSetup"), FOnSpawnTab::CreateStatic(&CreateCognitiveSceneSetupTabArgs)).SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("CognitiveProjectSetup"), FOnSpawnTab::CreateStatic(&CreateCognitiveProjectSetupTabArgs)).SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("CognitiveDynamicObjectManager"), FOnSpawnTab::CreateStatic(&CreateCognitiveDynamicObjectTabArgs)).SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("CognitiveFullProjectSetup"), FOnSpawnTab::CreateStatic(&CreateCognitiveFullSetupTabArgs)).SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("CognitiveFeatureBuilder"), FOnSpawnTab::CreateStatic(&CreateCognitiveFeatureBuilderTabArgs)).SetMenuType(ETabSpawnerMenuType::Hidden);

	FCognitive3DCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);

	//map all the menu items to functions
	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenProjectSetupWindow,
		FExecuteAction::CreateStatic(&FCognitive3DEditorModule::SpawnCognitiveProjectSetupTab)
	);

	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenSceneSetupWindow,
		FExecuteAction::CreateStatic(&FCognitive3DEditorModule::SpawnCognitiveSceneSetupTab)
	);

	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenDynamicObjectWindow,
		FExecuteAction::CreateStatic(&FCognitive3DEditorModule::SpawnCognitiveDynamicTab)
	);

	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenOnlineDocumentation,
		FExecuteAction::CreateStatic(&FCognitive3DEditorModule::OpenOnlineDocumentation)
	);

	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenCognitiveDashboard,
		FExecuteAction::CreateStatic(&FCognitive3DEditorModule::OpenCognitiveDashboard)
	);
	
	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenFullC3DSetup,
		FExecuteAction::CreateStatic(&FCognitive3DEditorModule::SpawnFullC3DSetup)
	);

	PluginCommands->MapAction(
		FCognitive3DCommands::Get().OpenFeatureBuilder,
		FExecuteAction::CreateStatic(&FCognitive3DEditorModule::SpawnFeatureBuilder)
	);
	

	//append the menu after help
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuBarExtension(
		"Help",
		EExtensionHook::After,
		PluginCommands,
		FMenuBarExtensionDelegate::CreateStatic(&FCognitive3DEditorModule::AddMenu)
	);
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	// Register the details customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	PropertyModule.RegisterCustomClassLayout(TEXT("Cognitive3DSettings"), FOnGetDetailCustomizationInstance::CreateStatic(&ICognitiveSettingsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(TEXT("DynamicObject"), FOnGetDetailCustomizationInstance::CreateStatic(&IDynamicObjectComponentDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(TEXT("DynamicIdPoolAsset"), FOnGetDetailCustomizationInstance::CreateStatic(&IDynamicIdPoolAssetDetails::MakeInstance));

	if (FCognitiveEditorTools::GetInstance()->bIsRestartEditorAfterSetup)
	{
		FCognitiveEditorTools::GetInstance()->bIsRestartEditorAfterSetup = false;
		//turn off build on startup once the editor is back up
		//we only turn it on when we restart after a project setup (third party sdk change)
		// Check if config file is already set to auto recompile on startup.
		bool bAutoRecompile = false;
		GConfig->GetBool(TEXT("/Script/UnrealEd.EditorLoadingSavingSettings"), TEXT("bForceCompilationAtStartup"), bAutoRecompile, GEditorPerProjectIni);

		if (bAutoRecompile)
		{
			GConfig->SetBool(TEXT("/Script/UnrealEd.EditorLoadingSavingSettings"), TEXT("bForceCompilationAtStartup"), false, GEditorPerProjectIni);
			GConfig->Flush(false, GEditorPerProjectIni);
			UE_LOG(LogTemp, Log, TEXT("Enabling Editor Auto Recompile at Startup"));
		}
	}

#endif
}

	//defines the menu to add the contents of the menu
	void FCognitive3DEditorModule::AddMenu(FMenuBarBuilder& MenuBuilder)
	{
		MenuBuilder.AddPullDownMenu(
			LOCTEXT("MenuLocKey", "Cognitive3D"),
			LOCTEXT("MenuTooltipKey", "Open Cognitive3D menu"),
			FNewMenuDelegate::CreateStatic(&FCognitive3DEditorModule::FillMenu),
			FName(TEXT("Cognitive3D")),
			FName(TEXT("Cognitive3DName")));
	}

	void FCognitive3DEditorModule::FillMenu(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenFullC3DSetup);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenFeatureBuilder);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenOnlineDocumentation);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenCognitiveDashboard);

		MenuBuilder.AddSubMenu(
			LOCTEXT("Cognitive3DSettingsMenu", "Legacy"),
			LOCTEXT("Cognitive3DSettingsMenuTooltip", "Open Legacy Cognitive3D Windows"),
			FNewMenuDelegate::CreateStatic(&FCognitive3DEditorModule::FillLegacySubMenu)
		);
	}

	void FCognitive3DEditorModule::FillLegacySubMenu(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenProjectSetupWindow);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenSceneSetupWindow);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenDynamicObjectWindow);
	}

void FCognitive3DEditorModule::ShutdownModule()
{
	FCognitiveEditorStyle::Shutdown();
	FCognitive3DCommands::Unregister();
}

bool FCognitive3DEditorModule::SupportsDynamicReloading()
{
	return true;
}

void FCognitive3DEditorModule::SpawnCognitiveDynamicTab()
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

void FCognitive3DEditorModule::SpawnCognitiveSceneSetupTab()
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

void FCognitive3DEditorModule::SpawnCognitiveProjectSetupTab()
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

void FCognitive3DEditorModule::OpenOnlineDocumentation()
{
	FString url = "https://docs.cognitive3d.com/unreal/get-started/";
	FPlatformProcess::LaunchURL(*url, nullptr, nullptr);
}

void FCognitive3DEditorModule::OpenCognitiveDashboard()
{
	FString url = "https://app.cognitive3d.com";
	FPlatformProcess::LaunchURL(*url, nullptr, nullptr);
}

void FCognitive3DEditorModule::CloseProjectSetupWindow()
{
	FTabId projectTabId = FTabId(FName("CognitiveProjectSetup"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
}

void FCognitive3DEditorModule::CloseSceneSetupWindow()
{
	FTabId projectTabId = FTabId(FName("CognitiveSceneSetup"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
}

void FCognitive3DEditorModule::CloseDynamicObjectWindow()
{
	FTabId projectTabId = FTabId(FName("CognitiveDynamicObjectManager"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
}

void FCognitive3DEditorModule::SpawnFullC3DSetup()
{
	FTabId projectTabId = FTabId(FName("CognitiveFullProjectSetup"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
	else
	{
		TSharedPtr<SDockTab> MajorTab = FGlobalTabmanager::Get()->TryInvokeTab(projectTabId);
		//MajorTab->SetContent(SNew(SProjectManagerWidget));
	}
	//FGlobalTabmanager::Get()->TryInvokeTab(FTabId(FName("CognitiveFullProjectSetup")));
}

void FCognitive3DEditorModule::CloseFullC3DSetup()
{
	FTabId projectTabId = FTabId(FName("CognitiveFullProjectSetup"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
}

void FCognitive3DEditorModule::SpawnFeatureBuilder()
{
	FTabId projectTabId = FTabId(FName("CognitiveFeatureBuilder"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
	else
	{
		TSharedPtr<SDockTab> MajorTab = FGlobalTabmanager::Get()->TryInvokeTab(projectTabId);
		//MajorTab->SetContent(SNew(SProjectManagerWidget));
	}
}

void FCognitive3DEditorModule::CloseFeatureBuilder()
{
	FTabId projectTabId = FTabId(FName("CognitiveFeatureBuilder"));
	auto projectTab = FGlobalTabmanager::Get()->FindExistingLiveTab(projectTabId);
	if (projectTab.IsValid())
	{
		projectTab->RequestCloseTab();
	}
}


#undef LOCTEXT_NAMESPACE
