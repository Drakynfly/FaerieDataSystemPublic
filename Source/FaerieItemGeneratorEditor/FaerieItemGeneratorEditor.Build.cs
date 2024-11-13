// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieItemGeneratorEditor : ModuleRules
{
    public FaerieItemGeneratorEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        FaerieDataUtils.ApplySharedModuleSetup(this, Target);

        PublicDependencyModuleNames.AddRange(
            new []
            {
                "Core",
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