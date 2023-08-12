// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieItemGeneratorEditor : ModuleRules
{
    public FaerieItemGeneratorEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new []
            {
                "Core",
                "StructUtils",
                "FaerieItemGenerator",
                "FaerieDataSystemEditor"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new []
            {
                "AssetDefinition",
                "CoreUObject",
                "Engine",
                "PropertyEditor",
                "Slate",
                "SlateCore",
                "UnrealEd"
            }
        );
    }
}