// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieInventoryContent : ModuleRules
{
    public FaerieInventoryContent(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Engine dependencies
        PublicDependencyModuleNames.AddRange(
            new []
            {
                "Core",
                "CoreUObject",
                "Engine",
                "NetCore",
                "GameplayTags",
                "UMG",
                "Slate",
                "StructUtils"
            });

        // Plugin dependencies
        PublicDependencyModuleNames.AddRange(
            new []
            {
                "FaerieEquipment",
                "FaerieInventory",
                "FaerieItemGenerator",
                "FaerieItemData"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "GeometryScriptingCore",
                "GeometryFramework"
            });
    }
}