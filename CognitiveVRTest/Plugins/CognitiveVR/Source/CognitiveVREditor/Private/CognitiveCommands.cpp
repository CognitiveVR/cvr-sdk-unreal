#include "C3DCommands.h"

#define LOCTEXT_NAMESPACE "FCognitive3DEdtiorModule"

void FCognitive3DCommands::RegisterCommands()
{
	UI_COMMAND(OpenProjectSetupWindow, "Project Setup Window", "Opens the Project Setup Window", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenSceneSetupWindow, "Scene Setup Window", "Opens the Scene Setup Window", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenDynamicObjectWindow, "Dynamic Object Window", "Opens the Dynamic Object Window", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenOnlineDocumentation, "Open Online Documentation...", "Opens online documentation in your default browser", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
