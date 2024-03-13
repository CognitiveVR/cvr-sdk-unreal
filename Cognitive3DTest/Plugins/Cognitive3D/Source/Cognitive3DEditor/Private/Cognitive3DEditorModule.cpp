/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3DEditorModule.h"
#include "C3DCommands.h"

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

void FCognitive3DEditorModule::StartupModule()
{
#if WITH_EDITOR
	// Create the Extender that will add content to the menu
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));

	FCognitiveEditorTools::Initialize();	
	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, EditorIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, EditorIni);

	//add actions for the dynamic object id pool asset
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedPtr< FDynamicIdPoolAssetActions> action = MakeShared<FDynamicIdPoolAssetActions>();
	AssetTools.RegisterAssetTypeActions(action.ToSharedRef());

	//register window spawning
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("CognitiveSceneSetup"), FOnSpawnTab::CreateStatic(&CreateCognitiveSceneSetupTabArgs)).SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("CognitiveProjectSetup"), FOnSpawnTab::CreateStatic(&CreateCognitiveProjectSetupTabArgs)).SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("CognitiveDynamicObjectManager"), FOnSpawnTab::CreateStatic(&CreateCognitiveDynamicObjectTabArgs)).SetMenuType(ETabSpawnerMenuType::Hidden);

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
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenProjectSetupWindow);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenSceneSetupWindow);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenDynamicObjectWindow);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenOnlineDocumentation);
		MenuBuilder.AddMenuEntry(FCognitive3DCommands::Get().OpenCognitiveDashboard);
	}

void FCognitive3DEditorModule::ShutdownModule()
{
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

#undef LOCTEXT_NAMESPACE
