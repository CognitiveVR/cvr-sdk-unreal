// Fill out your copyright notice in the Description page of Project Settings.


#include "CognitiveEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Brushes/SlateImageBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Styling/SlateTypes.h"
#include "Misc/Paths.h"

TSharedPtr<FSlateStyleSet> FCognitiveEditorStyle::StyleInstance = nullptr;

FCognitiveEditorStyle::FCognitiveEditorStyle()
{
}

FCognitiveEditorStyle::~FCognitiveEditorStyle()
{
}

void FCognitiveEditorStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
        // Pass the FName directly
        StyleInstance = MakeShared<FSlateStyleSet>(GetStyleSetName());

        // Locate plugins Resources folder
        const FString PluginBaseDir = IPluginManager::Get()
            .FindPlugin(TEXT("Cognitive3D"))
            ->GetBaseDir();
        const FString ResourceDir = PluginBaseDir / TEXT("Resources");

        // Register the banner brush (size should match PNGs dimensions)
        StyleInstance->Set(
            TEXT("CognitiveEditor.Banner"),
            new FSlateImageBrush(
                ResourceDir / TEXT("C3D-Primary-Logo-WhiteText-1k.png"),
                FVector2D(720, 144)
            )
        );

		StyleInstance->Set(
			TEXT("CognitiveEditor.CustomEventsSimple"),
			new FSlateImageBrush(
				ResourceDir / TEXT("event-simple.png"),
				FVector2D(400, 400)
			)
		);

        StyleInstance->Set(
            TEXT("CognitiveEditor.DynamicObjectsIcon"),
            new FSlateImageBrush(
                ResourceDir / TEXT("dynamicObjects.png"),
                FVector2D(100, 100)
            )
        );
        StyleInstance->Set(
            TEXT("CognitiveEditor.ExitPollIcon"),
            new FSlateImageBrush(
                ResourceDir / TEXT("exitPoll.png"),
                FVector2D(100, 100)
            )
        );
        StyleInstance->Set(
            TEXT("CognitiveEditor.RemoteControlsIcon"),
            new FSlateImageBrush(
                ResourceDir / TEXT("remoteControls.png"),
                FVector2D(100, 100)
            )
        );
        StyleInstance->Set(
            TEXT("CognitiveEditor.CustomEvents"),
            new FSlateImageBrush(
                ResourceDir / TEXT("customEvents.png"),
                FVector2D(100, 100)
            )
        );

        // Code box styling - consistent across UE versions
        StyleInstance->Set(
            TEXT("CognitiveEditor.CodeBox"),
            new FSlateColorBrush(FLinearColor(0.07f, 0.07f, 0.07f, 1.0f)) // Dark gray background
        );

        // Feature list button styling - dark theme with light text
        StyleInstance->Set(
            TEXT("CognitiveEditor.FeatureButton"),
            FButtonStyle()
                .SetNormal(FSlateColorBrush(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f)))      // Very dark background
                .SetHovered(FSlateColorBrush(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f)))     // Slightly lighter on hover
                .SetPressed(FSlateColorBrush(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f)))     // Even darker when pressed
                .SetDisabled(FSlateColorBrush(FLinearColor(0.03f, 0.03f, 0.03f, 1.0f)))    // Muted when disabled
        );

        // Light text colors for dark buttons
        StyleInstance->Set(
            TEXT("CognitiveEditor.FeatureButtonText"),
            FTextBlockStyle()
                .SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f)))     // Light gray text
        );

        // Large title text for feature buttons
        StyleInstance->Set(
            TEXT("CognitiveEditor.FeatureButtonTitle"),
            FTextBlockStyle()
                .SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 18))
                .SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f)))     // Light gray text with large font
        );

        // Finally, register with Slate
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FCognitiveEditorStyle::Shutdown()
{
    if (StyleInstance.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
        ensure(StyleInstance.IsUnique());
        StyleInstance.Reset();
    }
}

FName FCognitiveEditorStyle::GetStyleSetName()
{
	static FName StyleName(TEXT("CognitiveEditorStyle"));
	return StyleName;
}

TSharedPtr<FSlateStyleSet> FCognitiveEditorStyle::GetStyleSet()
{
	return StyleInstance;
}

const FSlateBrush* FCognitiveEditorStyle::GetBrush(const FName PropertyName)
{
    return StyleInstance.IsValid()
        ? StyleInstance->GetBrush(PropertyName)
        : nullptr;
}
