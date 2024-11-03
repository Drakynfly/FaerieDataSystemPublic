// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieInventoryContent : ModuleRules
{
    public FaerieInventoryContent(ReadOnlyTargetRules Target) : base(Target)
    {
        FaerieDataUtils.ApplySharedModuleSetup(this, Target);

        // Engine dependencies
        PublicDependencyModuleNames.AddRange(
            new []
            {
                "Core",
                "CoreUObject",
                "Engine",
                "NetCore",
                "GameplayTags",
                "StructUtils",
                "UMG",
                "Slate",
                "SlateCore"
            });

        // Plugin dependencies
        PublicDependencyModuleNames.AddRange(
            new []
            {
                "FaerieEquipment",
                "FaerieInventory",
                "FaerieItemGenerator",
                "FaerieItemData",
                "FaerieItemMesh"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "FaerieDataUtils",
                "GeometryScriptingCore",
                "GeometryFramework",
                "Squirrel"
            });
    }
}