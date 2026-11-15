// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroSubtitleWidget.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "Characters/ToroCharacterDatabase.h"
#include "Framework/ToroDatabaseSubsystem.h"
#include "Libraries/ToroWidgetLibrary.h"
#include "Animation/WidgetAnimation.h"
#include "Framework/ToroPlayerHUD.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

UToroSubtitleWidget::UToroSubtitleWidget()
{
	AutoLayer = EToroAutoWidgetType::Gameplay;
}

bool UToroSubtitleWidget::EnqueueSubtitle(const UObject* ContextObject,
	FGameplayTag Speaker, const FToroSubtitleEntry& Entry)
{
	if (!ToroCharacterTags::IsValid(Speaker) || !Entry.IsValid())
	{
		return false;
	}

	const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(ContextObject);
	if (UToroSubtitleWidget* Widget = HUD ? HUD->GetAutoWidget<UToroSubtitleWidget>() : nullptr)
	{
		return Widget->Enqueue(Speaker, Entry);
	}

	return false;
}

bool UToroSubtitleWidget::EnqueueSubtitles(const UObject* ContextObject,
	FGameplayTag Speaker, const TArray<FToroSubtitleEntry>& Entries)
{
	if (!ToroCharacterTags::IsValid(Speaker) || Entries.IsEmpty())
	{
		return false;
	}

	const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(ContextObject);
	if (UToroSubtitleWidget* Widget = HUD ? HUD->GetAutoWidget<UToroSubtitleWidget>() : nullptr)
	{
		bool bSuccess = true;
		for (const FToroSubtitleEntry& Entry : Entries)
		{
			bSuccess &= Widget->Enqueue(Speaker, Entry);
		}
		return bSuccess;
	}

	return false;
}

void UToroSubtitleWidget::ShowNextLine()
{
	const UWorld* World = GetWorld();
	const FToroSubtitleEntry* Current = LineQueue.Peek();
	const FUserInterfaceConfig* Config = FUserInterfaceConfig::Get();
	if (IsValid(World) && Config && Current && Current->IsValid())
	{
		UpdateTextBlocks(*Current);

		// Next lines fade in, intentional
		PlayAnimation(TextAnimation);
		World->GetTimerManager().SetTimer(LineTimer,
			this, &UToroSubtitleWidget::FinishShowingLine,
			Config->CalculateTimeForText(Current->Line, Current->ExtraTime), false);
	}
	else
	{
		FinishShowingLine();
	}
}

void UToroSubtitleWidget::FinishShowingLine()
{
	FToroSubtitleEntry CurrentLine;
	if (!LineQueue.Dequeue(CurrentLine) || LineQueue.IsEmpty())
	{
		ActiveSpeaker = FGameplayTag::EmptyTag;
		PlayAnimationReverse(TextAnimation);
	}
	else
	{
		ShowNextLine();
	}
}

void UToroSubtitleWidget::UpdateTextBlocks(const FToroSubtitleEntry& Entry) const
{
	if (SpeakerBlock)
	{
		SpeakerBlock->SetVisibility(Entry.IsValid()
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
		if (Entry.IsValid())
		{
			FText NameText;
			if (const FToroCharacterEntry* CharInfo = Database.IsValid()
				? Database->FindData<FToroCharacterEntry>(ActiveSpeaker) : nullptr)
			{
				NameText = CharInfo->DisplayName;
			}
			else
			{
				const FString SpeakerStr = ActiveSpeaker.GetTagLeafName().ToString();
				NameText = FText::FromString(FName::NameToDisplayString(SpeakerStr, false));
			}
			SpeakerBlock->SetText(NameText);
		}
	}
	if (LineBlock)
	{
		LineBlock->SetText(Entry.Line);
		LineBlock->SetVisibility(Entry.IsValid()
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
	}
}

bool UToroSubtitleWidget::Enqueue(const FGameplayTag& Speaker, const FToroSubtitleEntry& Entry)
{
	const UWorld* World = GetWorld();
	if (!IsValid(World) || !ToroCharacterTags::IsValid(Speaker) || !Entry.IsValid())
	{
		return false;
	}

	FTimerManager& Timer = World->GetTimerManager();
	if (ActiveSpeaker.IsValid() && ActiveSpeaker != Speaker)
	{
		Timer.ClearTimer(LineTimer);
		LineQueue.Empty();
	}

	if (const FToroSubtitleEntry* Current = LineQueue.Peek())
	{
		if (Current->Line.ToString() == Entry.Line.ToString())
		{
			return true;
		}
	}

	ActiveSpeaker = Speaker;
	LineQueue.Enqueue(Entry);
	if (!Timer.TimerExists(LineTimer))
	{
		ShowNextLine();
	}

	return true;
}

void UToroSubtitleWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);
	if (TextAnimation == Animation && !ActiveSpeaker.IsValid())
	{
		UpdateTextBlocks({});
	}
}

void UToroSubtitleWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Database = UToroDatabaseSubsystem::Get(this);
	if (!TextAnimation || !SpeakerBlock || !LineBlock || !Database.IsValid())
	{
		RemoveFromParent();
	}

	UToroWidgetLibrary::InitWidgetAnimation(this, TextAnimation);
	SetAnimationCurrentTime(TextAnimation, 0.0f);
	PauseAnimation(TextAnimation);
	FlushAnimations();

	UpdateTextBlocks({});
}

void UToroSubtitleWidget::NativeDestruct()
{
	LineQueue.Empty();
	ActiveSpeaker = FGameplayTag::EmptyTag;
	StopAnimation(TextAnimation);
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LineTimer);
	}

	Super::NativeDestruct();
}