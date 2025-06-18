// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
 * 
 */
class FCognitiveEditorStyle
{
public:
    FCognitiveEditorStyle();
	~FCognitiveEditorStyle();

    /** Registers the style set (call in StartupModule). */
    static void Initialize();

    /** Unregisters and releases the style set (call in ShutdownModule). */
    static void Shutdown();

    /** @return the FName under which this style is registered. */
    static FName GetStyleSetName();

    /** @return the singleton style instance. */
    static TSharedPtr<FSlateStyleSet> GetStyleSet();

    /** @return a brush from the style by property name. */
    static const FSlateBrush* GetBrush(const FName PropertyName);

private:
    /** Singleton instance of the style set. */
    static TSharedPtr<FSlateStyleSet> StyleInstance;
};
