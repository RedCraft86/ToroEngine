// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "ToroCoreEd.h"

#include "MiscEditor/ToroSlateStyle.h"
#include "MiscEditor/ToroToolbarButton.h"
#include "ComponentVis/ToroComponentVisualizer.h"

#include "DetailsPanel/ToroIdentityCustomization.h"
#include "DetailsPanel/SimpleCooldownCustomization.h"
#include "DetailsPanel/ToroWrapperCustomization.h"
#include "DetailsPanel/ToroStructCustomization.h"
#include "DataTypes/WrappedTypes.h"
#include "DataTypes/InlineCurves.h"

#include "DetailsPanel/ToroActorCustomizations.h"
#include "DetailsPanel/ToroClassCustomization.h"
#include "Actors/ToroCharacter.h"
#include "Actors/ToroVolume.h"
#include "Actors/ToroActor.h"

DEFINE_LOG_CATEGORY(LogToroCoreEd);

#define LOCTEXT_NAMESPACE "FToroCoreEdModule"

void FToroCoreEdModule::StartupModule()
{
	FToroStructCustomization::Register<FWrappedBool, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FWrappedFloat, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FWrappedByte, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FWrappedInt32, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FWrappedInt64, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FWrappedName, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FWrappedString, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FWrappedGameplayTag, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FInlineFloatCurve, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FInlineVectorCurve, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FInlineColorCurve, FToroWrapperCustomization>();
	FToroStructCustomization::Register<FSimpleCooldown, FSimpleCooldownCustomization>();
	FToroStructCustomization::Register<FToroIdentity, FToroIdentityCustomization>();

	FToroClassCustomization::Register<AToroActor, FToroActorCustomization>();
	FToroClassCustomization::Register<AToroVolume, FToroVolumeCustomization>();
	FToroClassCustomization::Register<AToroCharacter, FToroCharacterCustomization>();
}

void FToroCoreEdModule::ShutdownModule()
{
	FToroToolbarButton::UnregisterAll();
	FToroClassCustomization::UnregisterAll();
	FToroStructCustomization::UnregisterAll();
	FToroComponentVisualizer::UnregisterAll();
	FToroSlateStyle::UnregisterAll();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToroCoreEdModule, ToroCoreEd)