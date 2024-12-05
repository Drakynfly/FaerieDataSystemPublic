// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieEquipment : ModuleRules
{
	public FaerieEquipment(ReadOnlyTargetRules Target) : base(Target)
	{
		FaerieDataUtils.ApplySharedModuleSetup(this, Target);

		PublicDependencyModuleNames.AddRange(
			new []
			{
				"Core",
				"CoreUObject",
				"Engine",
				"NetCore",
				"GameplayTags"
			});

		// @TODO temporary
		PublicDependencyModuleNames.AddRange(
			new []
			{
				"FaerieInventoryContent"
			});

		PublicDependencyModuleNames.AddRange(
			new []
			{
				"FaerieItemData",
				"FaerieItemMesh",
				"FaerieInventory",
				"Flakes"
			});

		PrivateDependencyModuleNames.AddRange(
			new []
			{
				"FaerieDataUtils",
				"Squirrel"
			});
	}
}