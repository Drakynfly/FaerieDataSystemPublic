// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieItemMesh : ModuleRules
{
    public FaerieItemMesh(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new []
            {
                "Core",
                "GameplayTags",
                "StructUtils",
            });

        PublicDependencyModuleNames.AddRange(
            new []
            {
                "FaerieItemData"
            });

        PrivateDependencyModuleNames.AddRange(
            new []
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "SkeletalMerging"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "GeometryScriptingCore",
                "GeometryFramework"
            });
    }
}