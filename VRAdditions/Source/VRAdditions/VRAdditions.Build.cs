using UnrealBuildTool;

public class VRAdditions : ModuleRules
{
    public VRAdditions(ReadOnlyTargetRules Target) : base(Target)
    {
	    PrivateDependencyModuleNames.AddRange(new string[] {"HeadMountedDisplay"});
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject",
            "Engine",
            "InputCore",
            "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemNULL",
            "SignificanceManager",
            "PhysX", "APEX", "PhysXVehicles", "ApexDestruction",
            "AkAudio",
            "ReplicationGraph",
            "UMG",
            "AIModule",
            "NavigationSystem",
            "AssetRegistry",
            "GameplayTasks",
            "AnimGraphRuntime",
            "Slate", "SlateCore",
            "Json",
			"AbstractInstance", "HeadMountedDisplay"
		});


        if (Target.Type == TargetRules.TargetType.Editor) {
			PublicDependencyModuleNames.AddRange(new string[] {"OnlineBlueprintSupport", "AnimGraph"});
		}
        PublicDependencyModuleNames.AddRange(new string[] {"FactoryGame", "SML"});
    }
}