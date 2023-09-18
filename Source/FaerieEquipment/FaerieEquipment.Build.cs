// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;

public class FaerieEquipment : ModuleRules
{
	public FaerieEquipment(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new []
			{
				"Core",
				"CoreUObject",
				"Engine",
				"NetCore",
				"GameplayTags",
				"StructUtils"
			});

		PublicDependencyModuleNames.AddRange(
			new []
			{
				"FaerieItemData",
				"FaerieItemMesh",
				"FaerieInventory"
			});

		PrivateDependencyModuleNames.AddRange(
			new []
			{
				"FaerieDataUtils",
				"Squirrel"
			});
	}
}