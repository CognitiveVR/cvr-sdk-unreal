// Fill out your copyright notice in the Description page of Project Settings.


#include "CognitiveEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Brushes/SlateImageBrush.h"
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
				FVector2D(514, 216)
			)
		);

		StyleInstance->Set(
			TEXT("CognitiveEditor.CustomEventsProperties"),
			new FSlateImageBrush(
				ResourceDir / TEXT("event-properties.png"),
				FVector2D(712, 182)
			)
		);

        StyleInstance->Set(
            TEXT("CognitiveEditor.CustomEventsDynamicObjects"),
            new FSlateImageBrush(
                ResourceDir / TEXT("event-dynamic.png"),
                FVector2D(456, 250)
            )
        );

        StyleInstance->Set(
            TEXT("CognitiveEditor.CustomEventsMakeObject"),
            new FSlateImageBrush(
                ResourceDir / TEXT("event-object.png"),
                FVector2D(1084, 186)
            )
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
