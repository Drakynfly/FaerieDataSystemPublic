// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

// A wrapper around FNativeGameplayTag that enforces the use of a FGameplayTag child.
template <typename TagT>
class TTypedNativeGameplayTag
{
public:
	TTypedNativeGameplayTag(const FName PluginName, const FName ModuleName, const FName TagName, const FString& TagDevComment, ENativeGameplayTagToken)
	  : InnerNativeTag(PluginName, ModuleName, TagName, TagDevComment, ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD) {}

	operator TagT() const { return TagT::ConvertChecked(InnerNativeTag); }
	TagT GetTag() const { return TagT::ConvertChecked(InnerNativeTag); }

private:
	FNativeGameplayTag InnerNativeTag;
};

/**
 * Declares a native gameplay tag that is defined in a cpp with UE_DEFINE_GAMEPLAY_TAG_TYPED to allow other modules or code to use the created tag variable.
 */
#define UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(Type, TagName) extern TTypedNativeGameplayTag<Type> TagName;

/**
 * Defines a native gameplay tag with a comment that is externally declared in a header to allow other modules or code to use the created tag variable.
 */
#define UE_DEFINE_GAMEPLAY_TAG_TYPED_COMMENT(Type, TagName, Tag, Comment) TTypedNativeGameplayTag<Type> TagName(UE_PLUGIN_NAME, UE_MODULE_NAME, Tag, TEXT(Comment), ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD); static_assert(UE::GameplayTags::Private::HasFileExtension(__FILE__), "UE_DEFINE_GAMEPLAY_TAG_TYPED_COMMENT can only be used in .cpp files, if you're trying to share tags across modules, use UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN in the public header, and UE_DEFINE_GAMEPLAY_TAG_TYPED_COMMENT in the private .cpp");

/**
 * Defines a native gameplay tag with no comment that is externally declared in a header to allow other modules or code to use the created tag variable.
 */
#define UE_DEFINE_GAMEPLAY_TAG_TYPED(Type, TagName, Tag) TTypedNativeGameplayTag<Type> TagName(UE_PLUGIN_NAME, UE_MODULE_NAME, Tag, TEXT(""), ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD); static_assert(UE::GameplayTags::Private::HasFileExtension(__FILE__), "UE_DEFINE_GAMEPLAY_TAG_TYPED can only be used in .cpp files, if you're trying to share tags across modules, use UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN in the public header, and UE_DEFINE_GAMEPLAY_TAG_TYPED in the private .cpp");

/**
 * Defines a native gameplay tag such that it's only available to the cpp file you define it in.
 */
#define UE_DEFINE_GAMEPLAY_TAG_TYPED_STATIC(Type, TagName, Tag) static TTypedNativeGameplayTag<Type> TagName(UE_PLUGIN_NAME, UE_MODULE_NAME, Tag, TEXT(""), ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD); static_assert(UE::GameplayTags::Private::HasFileExtension(__FILE__), "UE_DEFINE_GAMEPLAY_TAG_TYPED_STATIC can only be used in .cpp files, if you're trying to share tags across modules, use UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN in the public header, and UE_DEFINE_GAMEPLAY_TAG_TYPED in the private .cpp");