// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CognitiveVRTest : ModuleRules
{
	public CognitiveVRTest(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "CognitiveVRAnalytics"});
		PrivateDependencyModuleNames.AddRange(new string[] {"Json","JsonUtilities","CognitiveVRAnalytics"});
		PrivateIncludePathModuleNames.AddRange(new string[] {"CognitiveVRAnalytics"});
	}
}
