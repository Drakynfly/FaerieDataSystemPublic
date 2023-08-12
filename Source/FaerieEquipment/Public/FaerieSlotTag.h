// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "TTypedTagStaticImpl2.h"
#include "FaerieSlotTag.generated.h"

/**
 * The key used to flag entries with custom data.
 */
USTRUCT(BlueprintType, meta = (Categories = "Fae.Slot"))
struct FFaerieSlotTag : public FGameplayTag
{
	GENERATED_BODY()

	FFaerieSlotTag() {}
	static FFaerieSlotTag GetRootTag() { return TTypedTagStaticImpl2<FFaerieSlotTag>::StaticImpl.RootTag; }
	static FFaerieSlotTag TryConvert(const FGameplayTag FromTag) { return TTypedTagStaticImpl2<FFaerieSlotTag>::TryConvert(FromTag, false); }
	static FFaerieSlotTag ConvertChecked(const FGameplayTag FromTag) { return TTypedTagStaticImpl2<FFaerieSlotTag>::TryConvert(FromTag, true); }
	static FFaerieSlotTag AddNativeTag(const FString& TagBody, const FString& DevComment) { return TTypedTagStaticImpl2<FFaerieSlotTag>::AddNativeTag(TagBody, DevComment); }
	bool ExportTextItem(FString& ValueStr, const FFaerieSlotTag& DefaultValue, UObject* Parent, const int32 PortFlags, UObject* ExportRootScope) const
	{
		return TTypedTagStaticImpl2<FFaerieSlotTag>::ExportTextItem(*this, ValueStr, PortFlags);
	}

protected:
	FFaerieSlotTag(const FGameplayTag& Tag) { TagName = Tag.GetTagName(); }
	static const TCHAR* GetRootTagStr() { return TEXT("Fae.Slot"); }
	friend class TTypedTagStaticImpl2<FFaerieSlotTag>;
};

template<> struct TNameOf<FFaerieSlotTag> { FORCEINLINE static TCHAR const* GetName() { return TEXT("FFaerieSlotTag"); } };

template<>
struct TStructOpsTypeTraits<FFaerieSlotTag> : public TStructOpsTypeTraitsBase2<FFaerieSlotTag>
{
	enum
	{
		WithExportTextItem = true,
		WithImportTextItem = true
	};
};

struct FAERIEEQUIPMENT_API FFaerieSlotTags : public FGameplayTagNativeAdder
{
	FORCEINLINE static const FFaerieSlotTags& Get() { return FaerieSlotTags; }

	FFaerieSlotTag Body;
	FFaerieSlotTag Hand_Right;
	FFaerieSlotTag Hand_Left;

protected:
	virtual void AddTags() override
	{
		Body = FFaerieSlotTag::AddNativeTag(TEXT("Body"), "");

		Hand_Right = FFaerieSlotTag::AddNativeTag(TEXT("Hand.Right"), "");

		Hand_Left = FFaerieSlotTag::AddNativeTag(TEXT("Hand.Left"), "");
	}

private:
	// Private static object for the global tags. Use the Get() function to access externally.
	static FFaerieSlotTags FaerieSlotTags;
};