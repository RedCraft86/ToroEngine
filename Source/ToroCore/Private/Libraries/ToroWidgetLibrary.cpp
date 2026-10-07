// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroWidgetLibrary.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Utilities/RobAccess.h"

namespace
{
	UE5Coro::TCoroutine<> WaitForWidgetAnimEnded(const TSharedPtr<FWidgetAnimationState>& Player)
	{
		co_await Player->GetOnWidgetAnimationFinished();
	}
}

ROB_DEFINE_FUNC(
	UUserWidget,
	GetOrAddAnimationState,
	TSharedPtr<FWidgetAnimationState>,
	UWidgetAnimation*
);

void UToroWidgetLibrary::InitWidgetAnimations(UUserWidget* Target)
{
	if (!IsValid(Target))
	{
		return;
	}

	for (UClass* C = Target->GetClass(); C; C = C->GetSuperClass())
	{
		if (const UWidgetBlueprintGeneratedClass* Class = Cast<UWidgetBlueprintGeneratedClass>(C))
		{
			for (UWidgetAnimation* Animation : Class->Animations)
			{
				if (IsValid(Animation))
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
	if (!IsValid(Target) || !IsValid(Animation) || !FMath::IsFinite(PlayRate))
	{
		return false;
	}

	FWidgetAnimationHandle AnimHandle;
	const TSharedPtr<FWidgetAnimationState> AnimState = (Target->*RobAccess(UUserWidget, GetOrAddAnimationState))(Animation);
	if (!bResumePlay)
	{
		// Forward and Reverse should directly map since they're 0 and 1.
		// PingPong (2) is irrelevant since the Toro enum don't have it defined.
		const EUMGSequencePlayMode::Type NativeMode = static_cast<EUMGSequencePlayMode::Type>(PlayMode);
		const float StartTime = (PlayMode == EToroAnimationPlayMode::Forward) ? Animation->GetStartTime() : Animation->GetEndTime();
		AnimHandle = Target->PlayAnimation(Animation, StartTime, 1, NativeMode, FMath::Max(PlayRate, 0.1f));
	}
	else
	{
		switch (PlayMode)
		{
		case EToroAnimationPlayMode::Forward:
			AnimHandle = Target->PlayAnimationForward(Animation, FMath::Max(PlayRate, 0.1f));
			break;

		case EToroAnimationPlayMode::Reverse:
			AnimHandle = Target->PlayAnimationReverse(Animation, FMath::Max(PlayRate, 0.1f));
			break;
		}
	}

	return AnimHandle.IsValid() && AnimHandle.GetAnimationState()->GetPlaybackStatus() == EMovieScenePlayerStatus::Playing;
}

FVoidCoroutine UToroWidgetLibrary::PlayWidgetAnimationAsync(FLatentActionInfo LatentInfo, bool& bSuccess,
	UUserWidget* Target, UWidgetAnimation* Animation, EToroAnimationPlayMode PlayMode, float PlayRate, bool bResumePlay)
{
	bSuccess = false;
	if (!IsValid(Target) || !IsValid(Animation) || !FMath::IsFinite(PlayRate))
	{
		co_return;
	}

	const TSharedPtr<FWidgetAnimationState> AnimState = (Target->*RobAccess(UUserWidget, GetOrAddAnimationState))(Animation);
	auto OnAnimEnded = WaitForWidgetAnimEnded(AnimState);

	const float ExpectedTime = PlayMode == EToroAnimationPlayMode::Forward ? Animation->GetEndTime() : Animation->GetStartTime();
	if (PlayWidgetAnimation(Target, Animation, PlayMode, PlayRate, bResumePlay))
	{
		co_await OnAnimEnded;
		bSuccess = AnimState.IsValid() && FMath::IsNearlyEqual(AnimState->GetCurrentTime().AsSeconds(), ExpectedTime, 0.1f);
	}

	co_return;
}
