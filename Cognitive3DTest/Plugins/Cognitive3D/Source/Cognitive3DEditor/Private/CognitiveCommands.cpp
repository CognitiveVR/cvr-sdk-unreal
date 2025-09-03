/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "C3DCommands.h"

#define LOCTEXT_NAMESPACE "FCognitive3DEditorModule"

void FCognitive3DCommands::RegisterCommands()
{
	UI_COMMAND(OpenProjectSetupWindow, "Project Setup Window", "Opens the Project Setup Window", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenSceneSetupWindow, "Scene Setup Window", "Opens the Scene Setup Window", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenDynamicObjectWindow, "Dynamic Object Window", "Opens the Dynamic Object Window", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenOnlineDocumentation, "Open Online Documentation...", "Opens online documentation in your default browser", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenCognitiveDashboard, "Open Dashboard...", "Opens the Dashboard in your default browser", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenFullC3DSetup, "Open Full Setup", "Opens the full setup window", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenFeatureBuilder, "Open Feature Builder", "Opens the Feature Builder Window for Dynamic Object Management and more", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
