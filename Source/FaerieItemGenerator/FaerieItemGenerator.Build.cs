// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieItemGenerator : ModuleRules
{
    public FaerieItemGenerator(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new []
            {
                "Core",
                "GameplayTags",
                "StructUtils"
            }
        );

        PublicDependencyModuleNames.AddRange(
            new []
            {
                "FaerieItemData"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new []
            {
                "CoreUObject",
                "Engine",
                "NetCore",
                "Slate",
                "SlateCore",
                "Squirrel"
            }
        );
    }
}