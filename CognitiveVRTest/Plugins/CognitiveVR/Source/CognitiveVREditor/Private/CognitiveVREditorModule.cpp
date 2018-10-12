#include "CognitiveVREditorPrivatePCH.h"
#include "BaseEditorTool.h"
#include "PropertyEditorModule.h"
#include "LevelEditor.h"
#include "CognitiveEditorTools.h"
#include "SDockTab.h"
#include "SceneSetupWindow.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "Editor/EditorStyle/Public/EditorStyleSet.h"
#include "SSceneSetupWidget.h"
#include "FCognitiveSettingsCustomization.h"
#include "Containers/Ticker.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "DynamicComponentDetails.h"

//sets up customization for settings
//adds scene setup window
//creates editortools

#define LOCTEXT_NAMESPACE "DemoTools"

class FCognitiveVREditorModule : public IModuleInterface
{
public:

	//	FName SequenceRecorderTabName = FName("SequenceRecorder");
		// IMoudleInterface interface
		//virtual void StartupModule() override;
		//virtual void ShutdownModule() override;
		// End of IModuleInterface interface

		//static void OnToolWindowClosed(const TSharedRef<SWindow>& Window, UBaseEditorTool* Instance);

		//void AddMenuEntry(FMenuBuilder& MenuBuilder);
		//void DisplayPopup();
		//void SpawnSequenceRecorderTab(const FSpawnTabArgs& SpawnTabArgs);

		/*static void HandleTestCommandExcute();

		static bool HandleTestCommandCanExcute();*/

	//TSharedPtr<FUICommandList> CommandList;

	//FCognitiveTools* CognitiveEditorTools;

	FTickerDelegate TickDelegate;
	FDelegateHandle TickDelegateHandle;

	TSharedPtr<IImageWrapper> ImageWrapper;

	bool Tick(float deltatime)
	{
		FCognitiveEditorTools::GetInstance()->Tick(deltatime);
		return true;
	}

	virtual void StartupModule() override
	{
#if WITH_EDITOR
		// Create the Extender that will add content to the menu
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		


		//FAnalyticsCognitiveVR::Get().DeveloperKey = "read from config";

		FString EngineIni = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
		FString EditorIni = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEditor.ini"));
		//GLog->Log("FCognitiveTools::SaveAPIDeveloperKeysToFile save: " + CustomerId);

		GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), EngineIni);
		GConfig->Flush(false, EngineIni);
		
		FCognitiveEditorTools::Initialize();
		GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->APIKey, EngineIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FAnalyticsCognitiveVR::Get().DeveloperKey, EditorIni);
		//ConfigFileHasChanged = true;

		//TickDelegate = FTickerDelegate::CreateRaw(this, &FCognitiveVREditorModule::Tick);
		//TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);

		/*
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension(
			"EditMain",
			EExtensionHook::After,
			NULL,
			FMenuExtensionDelegate::CreateRaw(this, &FCognitiveVREditorModule::AddMenuEntry)
		);

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);*/


		// register 'keep simulation changes' recorder
		//FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		//LevelEditorModule.OnCaptureSingleFrameAnimSequence().BindStatic(&FCognitiveVREditorModule::HandleCaptureSingleFrameAnimSequence);

		// register standalone UI
		LevelEditorTabManagerChangedHandle = LevelEditorModule.OnTabManagerChanged().AddLambda([]()
		{
			TSharedPtr<FSlateStyleSet> StyleSet = MakeShareable(new FSlateStyleSet("CognitiveEditor"));
			StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));
			
			FString iconpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("CognitiveSceneWizardTabIcon.png");
			//FName BrushName = FName(*iconpath);

			//const TCHAR* charPath = *iconpath;
			StyleSet->Set(FName(*iconpath),new FSlateImageBrush(iconpath,FVector2D(128,128),FSlateColor()));

			//FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());

			FLevelEditorModule& LocalLevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
			LocalLevelEditorModule.GetLevelEditorTabManager()->RegisterTabSpawner(FName("CognitiveSceneSetup"), FOnSpawnTab::CreateStatic(&FCognitiveVREditorModule::SpawnCognitiveSceneSetupTab))
				.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
				.SetDisplayName(LOCTEXT("SceneSetupTabTitle", "Cognitive Scene Setup"))
				.SetTooltipText(LOCTEXT("SceneSetupTooltipText", "Open the Cognitive Scene Setup Wizard"));
				//.SetIcon(FSlateIcon(StyleSet, "CognitiveSceneWizardTabIcon"));
		});

		// Register the details customizations
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

		PropertyModule.RegisterCustomClassLayout(TEXT("CognitiveVRSettings"), FOnGetDetailCustomizationInstance::CreateStatic(&FCognitiveSettingsCustomization::MakeInstance));
		//PropertyModule.RegisterCustomClassLayout(TEXT("BaseEditorTool"), FOnGetDetailCustomizationInstance::CreateStatic(&FSetupCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(TEXT("DynamicObject"), FOnGetDetailCustomizationInstance::CreateStatic(&UDynamicObjectComponentDetails::MakeInstance));
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
#endif
	}

	virtual void ShutdownModule() override
	{
#if WITH_EDITOR

		//FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);

		if (GEditor)
		{
			//FDemoCommands::Unregister();
			//FDemoStyle::Shutdown();

			if (FModuleManager::Get().IsModuleLoaded(TEXT("LevelEditor")))
			{
				FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
				//LevelEditorModule.OnCaptureSingleFrameAnimSequence().Unbind();
				LevelEditorModule.OnTabManagerChanged().Remove(LevelEditorTabManagerChangedHandle);
			}
		}
#endif
	}

	void OnToolWindowClosed(const TSharedRef<SWindow>& Window, UBaseEditorTool* Instance)
	{
		Instance->RemoveFromRoot();
	}

	static TSharedRef<SDockTab> SpawnCognitiveSceneSetupTab(const FSpawnTabArgs& SpawnTabArgs)
	{
		const TSharedRef<SDockTab> MajorTab =
			SNew(SDockTab)
			//.Icon(FEditorStyle::Get().GetBrush("SequenceRecorder.TabIcon"))
			.TabRole(ETabRole::MajorTab);

		MajorTab->SetContent(SNew(SSceneSetupWidget));

		return MajorTab;
	}

	FDelegateHandle LevelEditorTabManagerChangedHandle;
};
IMPLEMENT_MODULE(FCognitiveVREditorModule, CognitiveVREditor);

#undef LOCTEXT_NAMESPACE