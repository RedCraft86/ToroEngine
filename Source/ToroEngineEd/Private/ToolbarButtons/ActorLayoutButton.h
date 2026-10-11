// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "SlateIM.h"
#include "ToroEngineEd.h"
#include "ToroEngineStyle.h"
#include "SlateIMWidgetBase.h"
#include "ScopedTransaction.h"
#include "DataTypes/CachedGetter.h"
#include "MiscEditor/ToroToolbarButton.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Subsystems/EditorActorSubsystem.h"

#define LOCTEXT_NAMESPACE "ToroEngineEd"

class FActorLayoutPopup final : public FSlateIMExposedBase, public TSharedFromThis<FActorLayoutPopup>
{
public:

	using FApplyCallback = TFunction<void(uint8, float, FVector2D)>;

	explicit FActorLayoutPopup(FApplyCallback InApply)
		: FSlateIMExposedBase(TEXT(UE_MODULE_NAME) TEXT(".ActorLayoutPopup"))
		, OnApply(MoveTemp(InApply))
	{
	}

	virtual ~FActorLayoutPopup() override
	{
		FActorLayoutPopup::DisableWidget();
		if (const TSharedPtr<IMenu> PinnedMenu = Menu.Pin())
		{
			PinnedMenu->Dismiss();
		}
	}

	void Open(const TSharedRef<SWidget>& ParentWidget)
	{
		SAssignNew(ContentHost, SBox)
		.WidthOverride(280.0f)
		.Padding(FMargin(12.0f));

		OnExposedWidgetChanged.AddSP(
			this, &FActorLayoutPopup::OnContentChanged);

		const TSharedPtr<IMenu> NewMenu =
			FSlateApplication::Get().PushMenu(
				ParentWidget,
				FWidgetPath(),
				ContentHost.ToSharedRef(),
				FSlateApplication::Get().GetCursorPos(),
				FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

		if (!NewMenu.IsValid())
		{
			return;
		}

		Menu = NewMenu;
		NewMenu->GetOnMenuDismissed().AddSP(
			this, &FActorLayoutPopup::OnMenuDismissed);

		EnableWidget();
	}

private:

	int32 MaxColumns = 10;
	float YawOffset = 0.0f;
	FVector2D XYOffset = FVector2D(100.0f);

	FApplyCallback OnApply;
	TSharedPtr<SBox> ContentHost;
	TWeakPtr<IMenu> Menu;

	virtual void DrawContent(float DeltaTime) override
	{
		SlateIM::BeginVerticalStack();

		SlateIM::Text(TEXT("Maximum columns"));
		SlateIM::SpinBox(MaxColumns, TOptional<int32>(1), TOptional<int32>(255));

		SlateIM::Text(TEXT("Yaw offset"));
		SlateIM::SpinBox(YawOffset, TOptional<float>(-360.0f), TOptional<float>(+360.0f));

		SlateIM::Text(TEXT("X spacing"));
		SlateIM::SpinBox(XYOffset.X, TOptional<double>(0.0f), TOptional<double>());

		SlateIM::Text(TEXT("Y spacing"));
		SlateIM::SpinBox(XYOffset.Y, TOptional<double>(0.0f), TOptional<double>());

		const bool bApply = SlateIM::Button(TEXT("Apply"));
		const bool bCancel = SlateIM::Button(TEXT("Cancel"));

		SlateIM::EndVerticalStack();

		if (bApply)
		{
			if (OnApply)
			{
				OnApply(MaxColumns, YawOffset, XYOffset);
			}

			Close();
		}
		else if (bCancel)
		{
			Close();
		}
	}

	void OnContentChanged(TSharedRef<SWidget> Widget) const
	{
		ContentHost->SetContent(Widget);
	}

	void OnMenuDismissed(TSharedRef<IMenu>)
	{
		DisableWidget();
		Menu.Reset();
	}

	void Close() const
	{
		if (const TSharedPtr<IMenu> PinnedMenu = Menu.Pin())
		{
			PinnedMenu->Dismiss();
		}
	}
};

class FActorLayoutButton final : public FToroToolbarButton
{
public:

	FActorLayoutButton()
		: FToroToolbarButton(
			TEXT("ActorLayout"),
			LOCTEXT("ActorLayoutLabel", "Actor Layout"),
			LOCTEXT("ActorLayoutTooltip", "Layouts the selected actors using the provided settings.")
		)
	{
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

private:

	TSharedPtr<FActorLayoutPopup> LayoutPopup;

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FToroEngineStyle::StyleName, TEXT("Toolbar.ActorLayout"));
	}

	virtual void Execute() override
	{
		if (!FToroEngineEdModule::ActorSubsystem.Get() || FToroEngineEdModule::ActorSubsystem->GetSelectedLevelActors().IsEmpty())
		{
			static const FNotificationInfo Info(LOCTEXT("NoActorsSelectedNotification", "No Actors Selected!"));
			FSlateNotificationManager::Get().AddNotification(Info);
			return;
		}

		const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
		if (!ParentWindow.IsValid())
		{
			return;
		}

		LayoutPopup.Reset();
		LayoutPopup = MakeShared<FActorLayoutPopup>(&FActorLayoutButton::LayoutActors);
		LayoutPopup->Open(ParentWindow.ToSharedRef());
	}

	static void LayoutActors(uint8 MaxColumns, float YawOffset, FVector2D XYOffset)
	{
		TArray<AActor*> SelectedActors = FToroEngineEdModule::ActorSubsystem->GetSelectedLevelActors();
		if (SelectedActors.IsEmpty())
		{
			static const FNotificationInfo Info(LOCTEXT("NoActorsSelectedNotification", "No Actors Selected!"));
			FSlateNotificationManager::Get().AddNotification(Info);
			return;
		}

		struct FGridEntry final
		{
			FVector Origin, Extent;
			TWeakObjectPtr<AActor> Actor;

			FGridEntry(AActor* InActor)
				: Actor(InActor)
			{
				InActor->GetActorBounds(false, Origin, Extent);
			}

			double GetArea() const
			{
				return Extent.X * Extent.Y;
			}
		};

		const FScopedTransaction Transaction(LOCTEXT("ActorLayoutFeature", "Layout Actors"));

		int32 Count = SelectedActors.Num();
		UE_LOG(LogToroEngineEd, Display, TEXT("[LayoutActors] Starting layout for %d actors..."), Count);

		TArray<FGridEntry> Actors;
		Actors.Reserve(Count);

		FVector2D CellSize = FVector2D::ZeroVector;
		for (AActor* SelectedActor : SelectedActors)
		{
			if (IsValid(SelectedActor))
			{
				const FGridEntry& Entry = Actors.Emplace_GetRef(SelectedActor);
				CellSize.X = FMath::Max(Entry.Extent.X * 2.0f, CellSize.X);
				CellSize.Y = FMath::Max(Entry.Extent.Y * 2.0f, CellSize.Y);
			}
		}

		if (Actors.IsEmpty())
		{
			UE_LOG(LogToroEngineEd, Display, TEXT("[LayoutActors] Layout finished as no valid actors were found."));
			return;
		}

		CellSize += XYOffset;

		UE_LOG(LogToroEngineEd, Display, TEXT("[LayoutActors] \t Cell Size: %.1f x %.1f"), CellSize.X, CellSize.Y);

		Actors.Sort([](const FGridEntry& A, const FGridEntry& B) -> bool
		{
				return A.GetArea() > B.GetArea();
		});

		Count = Actors.Num();
		for (int32 i = 0; i < Count; i++)
		{
			const FGridEntry& Entry = Actors[i];
			if (Entry.Actor.IsValid())
			{
				const FIntPoint Pos(i % MaxColumns, i / MaxColumns);
				const FVector PivotToBounds = Entry.Actor->GetActorLocation() - Entry.Origin;
				const FVector CellOffset(Pos.X * CellSize.X, Pos.Y * CellSize.Y, Entry.Extent.Z);

				FTransform Transform = Entry.Actor->GetActorTransform();
				Transform.SetRotation(FRotator(0.0f, FMath::UnwindDegrees(YawOffset), 0.0f).Quaternion());
				Transform.SetLocation(PivotToBounds + CellOffset);

				FToroEngineEdModule::ActorSubsystem->SetActorTransform(Entry.Actor.Get(), Transform);

				UE_LOG(LogToroEngineEd, Display,
					TEXT("[LayoutActors] \t Laid out [%d/%d] actor at (%d, %d)"),
					i + 1, Count, Pos.X, Pos.Y
				);
			}
		}

		UE_LOG(LogToroEngineEd, Display, TEXT("[LayoutActors] Finished laying out %d actors!"), Count);
	}
};
#undef LOCTEXT_NAMESPACE
