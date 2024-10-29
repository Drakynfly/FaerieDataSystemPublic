// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieInventoryContentEditor : ModuleRules
{
    public FaerieInventoryContentEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        FaerieDataUtils.ApplySharedModuleSetup(this, Target);

        PublicDependencyModuleNames.AddRange(
            new []
            {
                "Core",
                "FaerieEquipmentEditor",
                "FaerieInventoryContent",
                "FaerieDataSystemEditor",
                "StructUtils"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new []
            {
                "CoreUObject",
                "Engine",
                "GameplayTags",
                "GameplayTagsEditor",
                "Slate",
                "SlateCore",
            }
        );
    }
}