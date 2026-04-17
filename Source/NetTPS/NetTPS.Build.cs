// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetTPS : ModuleRules
{
	public NetTPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"NetTPS",
			"NetTPS/Variant_Platforming",
			"NetTPS/Variant_Platforming/Animation",
			"NetTPS/Variant_Combat",
			"NetTPS/Variant_Combat/AI",
			"NetTPS/Variant_Combat/Animation",
			"NetTPS/Variant_Combat/Gameplay",
			"NetTPS/Variant_Combat/Interfaces",
			"NetTPS/Variant_Combat/UI",
			"NetTPS/Variant_SideScrolling",
			"NetTPS/Variant_SideScrolling/AI",
			"NetTPS/Variant_SideScrolling/Gameplay",
			"NetTPS/Variant_SideScrolling/Interfaces",
			"NetTPS/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
