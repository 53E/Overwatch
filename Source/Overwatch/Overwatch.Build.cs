// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Overwatch : ModuleRules
{
	public Overwatch(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" , "UMG", "Slate" , "SlateCore", "EnhancedInput", "OnlineSubsystem", "OnlineSubsystemUtils" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "Overwatch",
            }
        );

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // 기본 Online Subsystem 사용
DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");

// Steam을 사용할 경우 아래 주석 해제
// DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
