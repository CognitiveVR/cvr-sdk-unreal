// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Cognitive3DTest : ModuleRules
{
	public Cognitive3DTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
