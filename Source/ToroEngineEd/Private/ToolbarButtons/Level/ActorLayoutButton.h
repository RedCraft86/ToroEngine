// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SlateIM.h"
#include "SlateIMWidgetBase.h"
#include "SlateStyles/ToroEngineStyle.h"
#include "ToolbarButtons/ToroToolbarButton.h"
#include "Utilities/ToroActorEdUtils.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
class FActorLayoutButton final : public FToroToolbarButton
{
	class FConfigPopup final : public FSlateIMExposedBase, public TSharedFromThis<FConfigPopup>
	{
		friend class FActorLayoutButton;

	public:

		FConfigPopup(const TSharedRef<FActorLayoutButton>& InButton)
			: FSlateIMExposedBase(InButton->GetName())
			, WeakButton(InButton)
		{}

		virtual ~FConfigPopup() override
		{
			FConfigPopup::DisableWidget();
			Close();
		}

	private:

		static inline int32 MaxColumns = 10;
		static inline float YawOffset = 0.0f;
		static inline FVector2D XYOffset = FVector2D(100.0f);

		TWeakPtr<IMenu> Menu;
		TSharedPtr<SBox> ContentHost;
		TWeakPtr<FActorLayoutButton> WeakButton;

		void Open(const TSharedRef<SWidget>& Parent)
		{
			SAssignNew(ContentHost, SBox)
			.WidthOverride(280.0f)
			.Padding(FMargin(12.0f));

			OnExposedWidgetChanged.AddSP(this, &FConfigPopup::OnContentChanged);
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

			Menu.Pin()->GetOnMenuDismissed().AddSP(this, &FConfigPopup::OnMenuDismissed);
			EnableWidget();
		}

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
				if (WeakButton.IsValid())
				{
					ToroEngine::ActorEdUtils::LayoutActors(
						WeakButton.Pin()->GetSelectedActors(),
						MaxColumns, YawOffset, XYOffset
					);
				}

				Close();
			}
			else if (bCancel)
			{
				Close();
			}
		}
	};

public:

	FActorLayoutButton()
		: FToroToolbarButton(TEXT("ActorLayout"))
	{
		Label = LOCTEXT("ActorLayoutLabel", "Layout Actors");
		Tooltip = LOCTEXT("ActorLayoutTooltip", "Lays out the selected actors using the configured settings.");
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

	TArray<AActor*> GetSelectedActors()
	{
		TArray<AActor*> Temp = SelectedActors;
		SelectedActors.Empty();
		return Temp;
	}

private:

	TArray<AActor*> SelectedActors;
	TSharedPtr<FConfigPopup> LayoutPopup;

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

		SelectedActors = ToroEngine::ActorEdUtils::GetSelectedActors(true);
		if (SelectedActors.IsEmpty())
		{
			return;
		}

		LayoutPopup.Reset();
		LayoutPopup = MakeShared<FConfigPopup>(StaticCastSharedRef<FActorLayoutButton>(AsShared()));
		LayoutPopup->Open(ParentWindow.ToSharedRef());
	}
};
#undef LOCTEXT_NAMESPACE