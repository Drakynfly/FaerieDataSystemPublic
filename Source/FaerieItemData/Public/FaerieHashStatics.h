// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.
#pragma once

#include "FaerieHash.h"

class UFaerieItem;
class UFaerieItemContainerBase;

using FFaerieItemHashFunction = TFunctionRef<uint32(const UFaerieItem*)>;

namespace Faerie::Hash
{
	FAERIEITEMDATA_API [[nodiscard]] uint32 Combine(const uint32 A, const uint32 B);

	FAERIEITEMDATA_API [[nodiscard]] FFaerieHash CombineHashes(TArray<uint32>& Hashes);

	// Get the hash of a FProperty's value on a specific object
	FAERIEITEMDATA_API [[nodiscard]] uint32 HashFProperty(const void* Ptr, const FProperty* Property);

	FAERIEITEMDATA_API [[nodiscard]] uint32 HashStructByProps(const void* Ptr, const UScriptStruct* Struct, bool IncludeSuper);
	FAERIEITEMDATA_API [[nodiscard]] uint32 HashObjectByProps(const UObject* Obj, bool IncludeSuper);

	FAERIEITEMDATA_API [[nodiscard]] FFaerieHash HashItemSet(const TSet<const UFaerieItem*>& Items, const FFaerieItemHashFunction& Function);

	FAERIEITEMDATA_API [[nodiscard]] uint32 HashItemByName(const UFaerieItem* Item);
}