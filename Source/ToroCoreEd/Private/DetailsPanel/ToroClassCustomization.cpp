// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DetailsPanel/ToroClassCustomization.h"
#include "DetailCategoryBuilder.h"
#include "EditorCategoryUtils.h"

IDetailCategoryBuilder* FToroClassCustomization::FindOrAddCategory(FName CategoryName)
{
	const TSharedPtr<IDetailLayoutBuilder> Builder = WeakBuilder.Pin();
	if (Builder.IsValid())
	{
		if (const FToroCategoryInfo* Info = CategoryMap.Find(CategoryName))
		{
			constexpr int32 PriorityGroupSize = 1000;
			IDetailCategoryBuilder& Category = Builder->EditCategory(CategoryName, Info->DisplayName, Info->Priority);
			// NOT USED: EditCategory only assigns priority on first use; preserve the engine's within-group order.
			// Category.SetSortOrder(Info->Priority * PriorityGroupSize + Category.GetSortOrder() % PriorityGroupSize);
			return &Category;
		}

		return &Builder->EditCategory(CategoryName);
	}

	return nullptr;
}

IDetailCategoryBuilder* FToroClassCustomization::SetCategoryDisplayName(FName CategoryName, const FText& DisplayName)
{
	CategoryMap.FindOrAdd(CategoryName).DisplayName = DisplayName;
	return FindOrAddCategory(CategoryName);
}

IDetailCategoryBuilder* FToroClassCustomization::SetCategoryPriority(FName CategoryName, ECategoryPriority::Type Priority)
{
	CategoryMap.FindOrAdd(CategoryName).Priority = Priority;
	return FindOrAddCategory(CategoryName);
}

TArray<FString> FToroClassCustomization::GetDefaultCategories()
{
	return TArray<FString>();
}

TArray<FString> FToroClassCustomization::GetForceShowCategories()
{
	return TArray<FString>{
		TEXT("Transform"),
		TEXT("TransformCommon")
	};
}

void FToroClassCustomization::HandleCategoryRenames()
{
	static const FName META_RenameCategories("RenameCategories");

	if (!CustomizingClass)
	{
		return;
	}

	TArray<FString> MetadataEntries;
	CustomizingClass->GetMetaData(META_RenameCategories).ParseIntoArray(MetadataEntries, TEXT(","));

	for (const FString& Entry : MetadataEntries)
	{
		FString From, To;
		if (!Entry.Split(TEXT("="), &From, &To, ESearchCase::IgnoreCase))
		{
			continue;
		}

		To.TrimStartAndEndInline();
		From.TrimStartAndEndInline();
		if (!From.IsEmpty() && !To.IsEmpty())
		{
			CategoryMap.FindOrAdd(*From).DisplayName = FText::FromString(To);
		}
	}
}

void FToroClassCustomization::HandleCategoryPriority()
{
	if (!CustomizingClass)
	{
		return;
	}

	TArray<FString> PrioritizeCategories;
	CustomizingClass->GetPrioritizeCategories(PrioritizeCategories);
	for (FString& Category : PrioritizeCategories)
	{
		Category.TrimStartAndEndInline();
		if (!Category.IsEmpty())
		{
			CategoryMap.FindOrAdd(*Category).Priority = ECategoryPriority::Important;
		}
	}
}

void FToroClassCustomization::HandleCategoryVisibility()
{
    const TSharedPtr<IDetailLayoutBuilder> Builder = WeakBuilder.Pin();
    if (!Builder.IsValid())
    {
       return;
    }

    // True if Category equals an entry or is a child of one ("Entry|Sub").
    auto MatchesAny = [](const TArray<FString>& Array, const FString& Category) -> bool
    {
       for (const FString& Entry : Array)
       {
          if (Category.Len() < Entry.Len() || !Category.StartsWith(Entry))
          {
             continue;
          }

          if (Category.Len() == Entry.Len() || Category[Entry.Len()] == TEXT('|'))
          {
             return true;
          }
       }

       return false;
    };

    TArray<FString> HideCategories, ShowCategories;
    FEditorCategoryUtils::GetClassHideCategories(CustomizingClass, HideCategories);
    FEditorCategoryUtils::GetClassShowCategories(CustomizingClass, ShowCategories);

    const TArray<FString> ForceShowCategories = GetForceShowCategories();
    const TArray<FString> DefaultCategories = GetDefaultCategories();

    // "*" in Hide or a non-empty Default list turns this into an allow list.
    const bool bAllowList = HideCategories.Remove(TEXT("*")) > 0 || !DefaultCategories.IsEmpty();

    TArray<FName> CategoriesToHide;
    Builder->GetCategoryNames(CategoriesToHide);

    // Priority: ForceShow > Show > Hide > Default.
    // Reduce the list to the categories that actually get hidden.
    CategoriesToHide.RemoveAll([&](const FName& CategoryName) -> bool
    {
       const FString Category = CategoryName.ToString();

       if (MatchesAny(ForceShowCategories, Category) || MatchesAny(ShowCategories, Category))
       {
          return true; // Forced or shown, keep.
       }

       if (MatchesAny(HideCategories, Category))
       {
          return false; // Hidden, beats Default.
       }

       // Untouched by Force/Show/Hide: allow list keeps only Default, otherwise keep everything.
       return !bAllowList || MatchesAny(DefaultCategories, Category);
    });

    for (const FName& Category : CategoriesToHide)
    {
       Builder->HideCategory(Category);
       CategoryMap.Remove(Category);
    }
}

void FToroClassCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder)
{
	WeakBuilder.Reset();
	CategoryMap.Empty();
	CustomizingClass = nullptr;
	if (!DetailBuilder.IsValid())
	{
		return;
	}

	UClass* SelectedClass = nullptr;
	TArray<TWeakObjectPtr<>> Objects;
	DetailBuilder->GetObjectsBeingCustomized(Objects);
	for (const TWeakObjectPtr<>& Object : Objects)
	{
		// Check validity and whether template customization is enabled.
		if (!Object.IsValid() || (!bCustomizeTemplate && Object->IsTemplate()))
		{
			return;
		}

		if (!SelectedClass)
		{
			SelectedClass = Object->GetClass();
		}
		else if (SelectedClass != Object->GetClass())
		{
			// Disallow customizing different classes.
			return;
		}
	}

	if (!SelectedClass)
	{
		return;
	}

	WeakBuilder = DetailBuilder;
	CustomizingClass = SelectedClass;

	HandleCategoryRenames();
	HandleCategoryPriority();
	HandleCategoryVisibility();

	IDetailCustomization::CustomizeDetails(DetailBuilder);
}
