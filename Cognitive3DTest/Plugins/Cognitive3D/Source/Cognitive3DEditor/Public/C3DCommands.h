/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FCognitive3DCommands : public TCommands<FCognitive3DCommands>
{
public:

	FCognitive3DCommands() : TCommands<FCognitive3DCommands>(TEXT("Cognitive3D"), NSLOCTEXT("Contexts", "Cognitive3D", "Cognitive3D Plugin"), NAME_None, FName("C3D"))
	{
	}
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenProjectSetupWindow;
	TSharedPtr< FUICommandInfo > OpenSceneSetupWindow;
	TSharedPtr< FUICommandInfo > OpenDynamicObjectWindow;
	TSharedPtr< FUICommandInfo > OpenOnlineDocumentation;
	TSharedPtr< FUICommandInfo > OpenCognitiveDashboard;
};
