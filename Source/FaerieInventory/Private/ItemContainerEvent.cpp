// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemContainerEvent.h"

namespace Faerie::Inventory::Tags
{
	UE_DEFINE_GAMEPLAY_TAG_TYPED_COMMENT(FFaerieInventoryTag, Addition,
		"Fae.Inventory.Addition", "Inventory item data added event")
	UE_DEFINE_GAMEPLAY_TAG_TYPED_COMMENT(FFaerieInventoryTag, RemovalBase,
		"Fae.Inventory.Removal", "Inventory item data removed event")
	UE_DEFINE_GAMEPLAY_TAG_TYPED_COMMENT(FFaerieInventoryTag, RemovalDeletion,
		"Fae.Inventory.Removal.Deletion", "Remove an item by deleting it entirely")
	UE_DEFINE_GAMEPLAY_TAG_TYPED_COMMENT(FFaerieInventoryTag, RemovalMoving,
		"Fae.Inventory.Removal.Moving", "Remove an item for the purpose of moving it elsewhere")

	const TSet<FFaerieInventoryTag>& RemovalTagsAllowedByDefault()
	{
		static const TSet<FFaerieInventoryTag> StaticTags
		{
			RemovalDeletion,
			RemovalMoving
		};

		return StaticTags;
	}
}