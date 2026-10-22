// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SlateIM.h"
#include "SlateIMWidgetBase.h"
#include "SlateStyles/ToroEngineStyle.h"
#include "ToolbarButtons/ToroToolbarButton.h"
#include "Utilities/ToroActorEdUtils.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
class FActorLayoutPopup final : public FSlateIMExposedBase, public TSharedFromThis<FActorLayoutPopup>
{
public:

	using FApplyCallback = TFunction<void(uint8, float, const FVector2D&)>;

	FActorLayoutPopup(FApplyCallback InApply)
		: FSlateIMExposedBase(TEXT(UE_MODULE_NAME) TEXT(".ActorLayoutPopup"))
		, OnApply(MoveTemp(InApply))
	{}

	virtual ~FActorLayoutPopup() override
	{
		FActorLayoutPopup::DisableWidget();
		Close();
	}

	void Open(const TSharedRef<SWidget>& Parent)
	{
		SAssignNew(ContentHost, SBox).WidthOverride(280.0f).Padding(FMargin(12.0f));

		OnExposedWidgetChanged.AddSP(this, &FActorLayoutPopup::OnContentChanged);

		Menu = FSlateApplication::Get().PushMenu(
			Parent,
			FWidgetPath(),
			ContentHost.ToSharedRef(),
			FSlateApplication::Get().GetCursorPos(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
		);

		if (!Menu.IsValid())
		{
			return;
		}

		Menu.Pin()->GetOnMenuDismissed().AddSP(this, &FActorLayoutPopup::OnMenuDismissed);
		EnableWidget();
	}

private:

	static inline int32 MaxColumns = 10;
	static inline float YawOffset = 0.0f;
	static inline FVector2D XYOffset = FVector2D(100.0f);

	TWeakPtr<IMenu> Menu;
	TSharedPtr<SBox> ContentHost;
	FApplyCallback OnApply;

	void Close() const
	{
		if (const TSharedPtr<IMenu> PinnedMenu = Menu.Pin())
		{
			PinnedMenu->Dismiss();
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

	virtual void DrawContent(float DeltaTime) override
	{
		SlateIM::BeginVerticalStack();

		SlateIM::Text(TEXT("Max Columns"));
		SlateIM::SpinBox(MaxColumns, 1, 50);

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
};

class FActorLayoutButton final : public FToroToolbarButton
{
	friend class FActorLayoutPopup;

public:

	FActorLayoutButton()
		: FToroToolbarButton(TEXT("ActorLayout"))
	{
		Label = LOCTEXT("ActorLayoutLabel", "Layout Actors");
		Tooltip = LOCTEXT("ActorLayoutTooltip", "Lays out the selected actors using the configured settings.");
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

private:

	TSharedPtr<FActorLayoutPopup> LayoutPopup;

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FToroEngineStyle::StyleName, TEXT("Toolbar.ActorLayout"));
	}

	virtual void Execute(const FToolMenuContext& Context) override
	{
		const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
		if (!ParentWindow.IsValid())
		{
			return;
		}

		const TArray<AActor*> Actors = ToroEngine::ActorEdUtils::GetSelectedActors(true);
		if (Actors.IsEmpty())
		{
			return;
		}

		LayoutPopup.Reset();
		LayoutPopup = MakeShared<FActorLayoutPopup>([Actors](uint8 MaxColumns, float YawOffset, const FVector2D& XYOffset)
		{
			ToroEngine::ActorEdUtils::LayoutActors(Actors, MaxColumns, YawOffset, XYOffset);
		});

		LayoutPopup->Open(ParentWindow.ToSharedRef());
	}
};
#undef LOCTEXT_NAMESPACE