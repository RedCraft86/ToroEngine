// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DetailsPanels/Class/ToroActorBaseCustomization.h"
#include "Utilities/ToroValidation.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Input/Reply.h"
#include "GameFramework/Actor.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
AActor* FToroActorBaseCustomization::GetPrimaryActor() const
{
	if (WeakBuilder.IsValid())
	{
		const TSharedPtr<IDetailLayoutBuilder> Builder = WeakBuilder.Pin();

		TArray<TWeakObjectPtr<UObject>> Objects;
		Builder->GetObjectsBeingCustomized(Objects);
		if (Objects.Num() != 1 || !Objects[0].IsValid() || !Objects[0]->IsA<AActor>())
		{
			return nullptr;
		}

		return Cast<AActor>(Objects[0].Get());
	}

	return nullptr;
}

bool FToroActorBaseCustomization::GetIsEnabled() const
{
	return IsValid(GetPrimaryActor());
}

FReply FToroActorBaseCustomization::OnValidateActor() const
{
	if (const AActor* Actor = GetPrimaryActor())
	{
		ToroEngine::Validation::ValidateActor(Actor);
	}

	return FReply::Handled();
}

void FToroActorBaseCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& ValidateCategory = DetailBuilder.EditCategory(
		TEXT("Validate"), LOCTEXT("ActorValidateLabel", "Validate"), ECategoryPriority::Transform
	);

	ValidateCategory.SetIsEmpty(true);
	ValidateCategory.SetSortOrder(-100);
	ValidateCategory.HeaderContent(
		SNew(SBox)
		.Padding(10.0f, 1.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Content()
		[
			SNew(SButton)
			.Text(LOCTEXT("ActorValidateLabel", "Validate"))
			.IsEnabled(TAttribute<bool>::CreateSP(this, &FToroActorBaseCustomization::GetIsEnabled))
			.OnClicked(FOnClicked::CreateSP(this, &FToroActorBaseCustomization::OnValidateActor))
			.ContentPadding(FMargin(50.0f, 2.0f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
		]
	, true);

	// Dummy row to show the category
	ValidateCategory.AddCustomRow(LOCTEXT("ActorValidateLabel", "Validate"))
	[
		SNullWidget::NullWidget
	];
}
#undef LOCTEXT_NAMESPACE