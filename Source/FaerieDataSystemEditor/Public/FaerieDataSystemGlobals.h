// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Misc/AssetCategoryPath.h"

namespace Faerie::Editor
{
	// Asset category used for faerie assets.
	inline const FAssetCategoryPath DefaultAssetCategory = NSLOCTEXT("FaerieEditor", "DefaultAssetCategory", "Faerie Data");
	inline const TArray<FAssetCategoryPath> DefaultCategories = { DefaultAssetCategory };
}