// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class YourProjectName : ModuleRules
{
	public YourProjectName(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore","Http", "CognitiveVR"});
		PrivateDependencyModuleNames.AddRange(new string[] {"Json","JsonUtilities","CognitiveVR"});
		PrivateIncludePathModuleNames.AddRange(new string[] {"CognitiveVR"});
	}
}
