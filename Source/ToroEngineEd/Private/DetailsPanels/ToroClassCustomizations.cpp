// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "EditorCategoryUtils.h"
#include "DetailCategoryBuilder.h"
#include "DetailsPanels/ToroClassCustomization.h"

IDetailCategoryBuilder& FToroClassCustomization::FindOrAddCategory(FName CategoryName)
{
	checkf(WeakBuilder.IsValid(), TEXT("Detail Builder is null and cannot access category info."))
	const TSharedPtr<IDetailLayoutBuilder> Builder = WeakBuilder.Pin();
	if (const FToroCategoryInfo* Info = CategoryMap.Find(CategoryName))
	{
		IDetailCategoryBuilder& Category = Builder->EditCategory(CategoryName, Info->DisplayName, Info->Priority);
		return Category;
	}

	return Builder->EditCategory(CategoryName);
}

IDetailCategoryBuilder& FToroClassCustomization::SetCategoryDisplayName(FName CategoryName, const FText& DisplayName)
{
	CategoryMap.FindOrAdd(CategoryName).DisplayName = DisplayName;
	return FindOrAddCategory(CategoryName);
}

IDetailCategoryBuilder& FToroClassCustomization::SetCategoryPriority(FName CategoryName, ECategoryPriority::Type Priority)
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

bool FToroClassCustomization::ShouldManageCategories() const
{
	for (const UClass* Class = CustomizingClass; Class; Class = Class->GetSuperClass())
	{
		const FName ClassName = Class->GetFName();
		if (RegisteredEntries.Contains(ClassName))
		{
			return RegisteredClassName == ClassName;
		}
	}

	return false;
}

void FToroClassCustomization::HandleCategoryRenames()
{
	static const FName META_RenameCategories(TEXT("RenameCategories"));

	if (!CustomizingClass)
	{
		return;
	}

	TArray<FString> MetadataEntries;
	CustomizingClass->GetMetaData(META_RenameCategories)
	                .ParseIntoArray(MetadataEntries, TEXT(","));

	for (const FString& Entry : MetadataEntries)
	{
		FString From, To;
		if (Entry.Split(TEXT("="), &From, &To, ESearchCase::IgnoreCase))
		{
			To.TrimStartAndEndInline();
			From.TrimStartAndEndInline();
			if (!From.IsEmpty() && !To.IsEmpty())
			{
				CategoryMap.FindOrAdd(*From).DisplayName = FText::FromString(To);
			}
		}
	}
}

void FToroClassCustomization::HandleCategoryPriority()
{
	if (!CustomizingClass)
	{
		return;
	}

	TArray<FString> CategoryEntries;
	CustomizingClass->GetPrioritizeCategories(CategoryEntries);
	for (FString& Entry : CategoryEntries)
	{
		Entry.TrimStartAndEndInline();
		if (!Entry.IsEmpty())
		{
			CategoryMap.FindOrAdd(*Entry).Priority = ECategoryPriority::Important;
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
			// Forced or shown, keep.
			return true;
		}

		if (MatchesAny(HideCategories, Category))
		{
			// Hidden, beats Default.
			return false;
		}

		// Untouched by Force/Show/Hide: allow list keeps only Default, otherwise keep everything.
		return !bAllowList || MatchesAny(DefaultCategories, Category);
	});

	for (const FName& Category : CategoriesToHide)
	{
		CategoryMap.Remove(Category);
		if (!Builder->EditCategory(Category).IsEmpty())
		{
			Builder->HideCategory(Category);
		}
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
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder->GetObjectsBeingCustomized(Objects);
	for (const TWeakObjectPtr<UObject>& Object : Objects)
	{
		// Check validity and whether template customization is enabled
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
			// Disallow customizing different classes
			return;
		}
	}

	if (!SelectedClass)
	{
		return;
	}

	WeakBuilder = DetailBuilder;
	CustomizingClass = SelectedClass;

	if (ShouldManageCategories())
	{
		HandleCategoryRenames();
		HandleCategoryPriority();
		HandleCategoryVisibility();
	}

	// Apply changes made by the handler functions
	for (const TPair<FName, FToroCategoryInfo>& CategoryInfo : CategoryMap)
	{
		FindOrAddCategory(CategoryInfo.Key);
	}

	IDetailCustomization::CustomizeDetails(DetailBuilder);
}
