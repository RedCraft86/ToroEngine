// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroWidgetLibrary.h"
#include "Animation/WidgetAnimation.h"
#include "Animation/WidgetAnimationState.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "Utilities/RobAccess.h"
#include "MovieScene.h"

ROB_DEFINE_FUNC(
	UUserWidget,
	GetOrAddAnimationState,
	TSharedPtr<FWidgetAnimationState>,
	UWidgetAnimation*
);

namespace
{
	UE5Coro::TCoroutine<> AwaitWidgetAnimEnded(const TSharedPtr<FWidgetAnimationState>& Player)
	{
		co_await Player->GetOnWidgetAnimationFinished();
	}
}

void UToroWidgetLibrary::InitWidgetAnimations(UUserWidget* Target)
{
	if (!IsValid(Target))
	{
		return;
	}

	for (UClass* Class = Target->GetClass(); Class; Class = Class->GetSuperClass())
	{
		if (const UWidgetBlueprintGeneratedClass* BPClass = Cast<UWidgetBlueprintGeneratedClass>(Class))
		{
			for (UWidgetAnimation* Animation : BPClass->Animations)
			{
				if (IsValid(Animation) && IsValid(Animation->GetMovieScene()))
				{
					(Target->*RobAccess(UUserWidget, GetOrAddAnimationState))(Animation);
				}
			}
		}
	}
}

bool UToroWidgetLibrary::PlayWidgetAnimation(UUserWidget* Target, UWidgetAnimation* Animation,
	EToroAnimationPlayMode PlayMode, float PlayRate, bool bResumePlay)
{
	if (!FMath::IsFinite(PlayRate))
	{
		FFrame::KismetExecutionMessage(TEXT("PlayRate must be finite."), ELogVerbosity::Error);
		return false;
	}

	if (!IsValid(Target)
		|| !IsValid(Animation)
		|| !IsValid(Animation->GetMovieScene())
		|| (PlayMode != EToroAnimationPlayMode::Forward && PlayMode != EToroAnimationPlayMode::Reverse))
	{
		return false;
	}

	FWidgetAnimationHandle AnimHandle;
	const TSharedPtr<FWidgetAnimationState> AnimState = (Target->*RobAccess(UUserWidget, GetOrAddAnimationState))(Animation);
	if (!AnimState.IsValid() || !AnimState->IsValid())
	{
		return false;
	}

	const float AdjustedRate = FMath::Max(PlayRate, 0.1f);
	if (!bResumePlay)
	{
		// Forward and Reverse should directly map since they're 0 and 1.
		// PingPong (2) is irrelevant since EToroAnimationPlayMode doesn't have it.
		const EUMGSequencePlayMode::Type NativeMode = static_cast<EUMGSequencePlayMode::Type>(PlayMode);
		AnimHandle = Target->PlayAnimation(Animation, 0.0f, 1, NativeMode, AdjustedRate);
	}
	else
	{
		AnimState->SetPlaybackSpeed(AdjustedRate);
		switch (PlayMode)
		{
		case EToroAnimationPlayMode::Forward:
			AnimHandle = Target->PlayAnimationForward(Animation, AdjustedRate);
			break;

		case EToroAnimationPlayMode::Reverse:
			AnimHandle = Target->PlayAnimationReverse(Animation, AdjustedRate);
			break;
		}
	}

	return AnimHandle.IsValid();
}

FVoidCoroutine UToroWidgetLibrary::PlayWidgetAnimationAsync(FLatentActionInfo LatentInfo, bool& bSuccess, UUserWidget* Target,
	UWidgetAnimation* Animation, EToroAnimationPlayMode PlayMode, float PlayRate, bool bResumePlay)
{
	bSuccess = false;

	if (!FMath::IsFinite(PlayRate))
	{
		FFrame::KismetExecutionMessage(TEXT("PlayRate must be finite."), ELogVerbosity::Error);
		co_return;
	}

	if (!IsValid(Target)
		|| !IsValid(Animation)
		|| !IsValid(Animation->GetMovieScene())
		|| (PlayMode != EToroAnimationPlayMode::Forward && PlayMode != EToroAnimationPlayMode::Reverse))
	{
		co_return;
	}

	const TSharedPtr<FWidgetAnimationState> AnimState = (Target->*RobAccess(UUserWidget, GetOrAddAnimationState))(Animation);
	if (!AnimState.IsValid() || !AnimState->IsValid())
	{
		co_return;
	}

	auto OnAnimEnded = AwaitWidgetAnimEnded(AnimState);

	const float ExpectedTime = PlayMode == EToroAnimationPlayMode::Forward ? Animation->GetEndTime() : Animation->GetStartTime();
	if (PlayWidgetAnimation(Target, Animation, PlayMode, PlayRate, bResumePlay))
	{
		co_await OnAnimEnded;
		bSuccess = AnimState.IsValid() && FMath::IsNearlyEqual(AnimState->GetCurrentTime().AsSeconds(), ExpectedTime, 0.1f);
	}
}

UWidget* UToroWidgetLibrary::GetWidgetRoot(UUserWidget* Target)
{
	return IsValid(Target) ? Target->GetRootWidget() : nullptr;
}

UPanelWidget* UToroWidgetLibrary::GetWidgetParent(UWidget* Target)
{
	if (!IsValid(Target))
	{
		return nullptr;
	}

	UPanelWidget* Parent = Target->GetParent();
	if (IsValid(Parent))
	{
		return Parent;
	}

	const UWidgetTree* WidgetTree = Cast<UWidgetTree>(Target->GetOuter());
	if (!IsValid(WidgetTree))
	{
		return nullptr;
	}

	const UUserWidget* UserWidget = Cast<UUserWidget>(WidgetTree->GetOuter());
	if (IsValid(UserWidget))
	{
		return UserWidget->GetParent();
	}

	return nullptr;
}

void UToroWidgetLibrary::ButtonClick(UButton* Target)
{
	if (IsValid(Target))
	{
		Target->OnClicked.Broadcast();
	}
}

void UToroWidgetLibrary::ButtonPress(UButton* Target)
{
	if (IsValid(Target))
	{
		Target->OnPressed.Broadcast();
	}
}

void UToroWidgetLibrary::ButtonRelease(UButton* Target)
{
	if (IsValid(Target))
	{
		Target->OnReleased.Broadcast();
	}
}

void UToroWidgetLibrary::ButtonHover(UButton* Target)
{
	if (IsValid(Target))
	{
		Target->OnHovered.Broadcast();
	}
}

void UToroWidgetLibrary::ButtonUnhover(UButton* Target)
{
	if (IsValid(Target))
	{
		Target->OnUnhovered.Broadcast();
	}
}
