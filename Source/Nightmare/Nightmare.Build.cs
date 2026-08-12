// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Nightmare : ModuleRules
{
	public Nightmare(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// Ensure Source/Nightmare is visible to files under Tests/.
		PrivateIncludePaths.Add(ModuleDirectory);
	}
}
