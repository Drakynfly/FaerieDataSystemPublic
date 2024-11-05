// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "SGameplayTagGraphPin_FIXED.h"
#include "EdGraph/EdGraphSchema.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "K2Node_CallFunction.h"
#include "K2Node_FunctionTerminator.h"
#include "K2Node_VariableSet.h"
#include "SGameplayTagCombo.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "TypedGameplayTagGraphPin"

FString GameplayTagExportText(const FGameplayTag Tag)
{
	FString ExportString;
	FGameplayTag::StaticStruct()->ExportText(ExportString, &Tag, &Tag, /*OwnerObject*/nullptr, /*PortFlags*/0, /*ExportRootScope*/nullptr);
	return ExportString;
}

static FName NAME_Categories = FName("Categories");
static FName NAME_GameplayTagFilter = FName("GameplayTagFilter");

// Fixed version of UE::GameplayTags::EditorUtilities::ExtractTagFilterStringFromGraphPin
FString ExtractTagFilterStringFromGraphPin(UEdGraphPin* InTagPin)
{
	FString FilterString;

	if (ensure(InTagPin))
	{
		UEdGraphNode* OwningNode = InTagPin->GetOwningNode();

		if (FilterString.IsEmpty())
		{
			FilterString = OwningNode->GetPinMetaData(InTagPin->PinName, NAME_Categories);
		}
		if (FilterString.IsEmpty())
		{
			FilterString = OwningNode->GetPinMetaData(InTagPin->PinName, NAME_GameplayTagFilter);
		}

		const UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();

		if (FilterString.IsEmpty())
		{
			if (UScriptStruct* PinStructType = Cast<UScriptStruct>(InTagPin->PinType.PinSubCategoryObject.Get()))
			{
				FilterString = TagManager.GetCategoriesMetaFromField(PinStructType);
			}
		}

		if (FilterString.IsEmpty())
		{
			if (const UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(OwningNode))
			{
				if (const UFunction* TargetFunction = CallFuncNode->GetTargetFunction())
				{
					FilterString = TagManager.GetCategoriesMetaFromFunction(TargetFunction, InTagPin->PinName);
				}
			}
			else if (const UK2Node_VariableSet* VariableSetNode = Cast<UK2Node_VariableSet>(OwningNode))
			{
				if (FProperty* SetVariable = VariableSetNode->GetPropertyForVariable())
				{
					FilterString = TagManager.GetCategoriesMetaFromField(SetVariable);
				}
			}
			else if (const UK2Node_FunctionTerminator* FuncTermNode = Cast<UK2Node_FunctionTerminator>(OwningNode))
			{
				if (const UFunction* SignatureFunction = FuncTermNode->FindSignatureFunction())
				{
					FilterString = TagManager.GetCategoriesMetaFromFunction(SignatureFunction, InTagPin->PinName);
				}
			}
		}
	}

	return FilterString;
}

void SGameplayTagGraphPin_FIXED::Construct( const FArguments& InArgs, UEdGraphPin* InGraphPinObj )
{
	SGraphPin::Construct( SGraphPin::FArguments(), InGraphPinObj );
}

void SGameplayTagGraphPin_FIXED::ParseDefaultValueData()
{
	// Read using import text, but with serialize flag set so it doesn't always throw away invalid ones
	GameplayTag.FromExportString(GraphPinObj->GetDefaultAsString(), PPF_SerializedAsImportText);
}

void SGameplayTagGraphPin_FIXED::OnTagChanged(const FGameplayTag NewTag)
{
	GameplayTag = NewTag;

	const FString TagString = GameplayTagExportText(GameplayTag);

	FString CurrentDefaultValue = GraphPinObj->GetDefaultAsString();
	if (CurrentDefaultValue.IsEmpty())
	{
		CurrentDefaultValue = GameplayTagExportText(FGameplayTag());
	}

	if (!CurrentDefaultValue.Equals(TagString))
	{
		const FScopedTransaction Transaction(LOCTEXT("ChangeDefaultValue", "Change Pin Default Value"));
		GraphPinObj->Modify();
		GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, TagString);
	}
}

TSharedRef<SWidget> SGameplayTagGraphPin_FIXED::GetDefaultValueWidget()
{
	if (GraphPinObj == nullptr)
	{
		return SNullWidget::NullWidget;
	}

	ParseDefaultValueData();

	// Note: this call here is the reason for this class. The engine's version has a bug.
	const FString FilterString = ExtractTagFilterStringFromGraphPin(GraphPinObj);

	return SNew(SGameplayTagCombo)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		.Filter(FilterString)
		.Tag(this, &SGameplayTagGraphPin_FIXED::GetGameplayTag)
		.OnTagChanged(this, &SGameplayTagGraphPin_FIXED::OnTagChanged);
}

FGameplayTag SGameplayTagGraphPin_FIXED::GetGameplayTag() const
{
	return GameplayTag;
}

#undef LOCTEXT_NAMESPACE