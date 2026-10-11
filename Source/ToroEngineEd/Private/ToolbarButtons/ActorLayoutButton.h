// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "SlateIM.h"
#include "SlateIMWidgetBase.h"
#include "ToroEngineStyle.h"
#include "Helpers/ToroActorEditorUtils.h"
#include "MiscEditor/ToroToolbarButton.h"

#define LOCTEXT_NAMESPACE "ToroEngineEd"
class FActorLayoutPopup final : public FSlateIMExposedBase, public TSharedFromThis<FActorLayoutPopup>
{
public:

	using FApplyCallback = TFunction<void(uint8, float, const FVector2D&)>;

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

	static inline int32 MaxColumns = 10;
	static inline float YawOffset = 0.0f;
	static inline FVector2D XYOffset = FVector2D(100.0f);

	FApplyCallback OnApply;
	TSharedPtr<SBox> ContentHost;
	TWeakPtr<IMenu> Menu;

	virtual void DrawContent(float DeltaTime) override
	{
		SlateIM::BeginVerticalStack();

		SlateIM::Text(TEXT("Max Columns"));
		SlateIM::SpinBox(MaxColumns, 1, 255);

		SlateIM::Text(TEXT("Yaw Offset"));
		SlateIM::SpinBox(YawOffset, -180.0f, 180.0f);

		SlateIM::Text(TEXT("X Spacing"));
		SlateIM::SpinBox(XYOffset.X, 0.0f, TOptional<double>());

		SlateIM::Text(TEXT("Y Spacing"));
		SlateIM::SpinBox(XYOffset.Y, 0.0f, TOptional<double>());

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
		if (const TArray<AActor*> Actors = FToroActorEditorUtils::GetSelectedActors(true); !Actors.IsEmpty())
		{
			const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
			if (!ParentWindow.IsValid())
			{
				return;
			}

			LayoutPopup.Reset();
			LayoutPopup = MakeShared<FActorLayoutPopup>([&Actors](uint8 MaxColumns, float YawOffset, const FVector2D& XYOffset)
			{
				FToroActorEditorUtils::LayoutActors(Actors, MaxColumns, YawOffset, XYOffset);
			});
			LayoutPopup->Open(ParentWindow.ToSharedRef());
		}
	}
};
#undef LOCTEXT_NAMESPACE
