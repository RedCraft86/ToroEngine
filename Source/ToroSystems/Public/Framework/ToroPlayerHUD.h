// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/HUD.h"
#include "ToroPlayerHUD.generated.h"

class UToroMasterWidget;
class UCommonActivatableWidget;
enum class EToroMasterWidgetType : uint8;

/**
 * Player HUD that creates and owns the master UI before Blueprint BeginPlay runs.
 * Routes activatable entries to gameplay, menu, or overlay containers through its master widget.
 * Class additions use the selected container's pool; instance additions retain external ownership.
 * Forwarding calls return null or do nothing when the master UI is unavailable.
 */
UCLASS(Blueprintable, BlueprintType, meta = (ChildCanTick = true))
class TOROSYSTEMS_API AToroPlayerHUD : public AHUD
{
	GENERATED_BODY()

public:

	AToroPlayerHUD();

	/**
	 * Retrieves the active player HUD if it matches the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Player HUD",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static AHUD* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<AHUD> Class, int32 PlayerIdx = 0);

	/**
	 * Retrieves the active player HUD as the requested type.
	 * @tparam T Type derived from AHUD.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T = AToroPlayerHUD>
	[[nodiscard]] static T* Get(const UObject* ContextObject, int32 PlayerIdx = 0)
	{
		static_assert(TIsDerivedFrom<T, AHUD>::Value,
			"T must derive from AHUD");
		return Cast<T>(Get(ContextObject, T::StaticClass(), PlayerIdx));
	}

	/**
	 * Adds a pooled activatable entry, or returns the selected layer's existing registered class entry.
	 * @param Layer Destination gameplay, menu, or forced-overlay container.
	 * @param Class Concrete activatable widget class; null and abstract classes are ignored.
	 * @return Existing or newly added entry, or null if the master/layer is unavailable or addition fails.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = HUD, meta = (DeterminesOutputType = Class))
	UCommonActivatableWidget* AddByClass(EToroMasterWidgetType Layer,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> Class) const;

	/**
	 * Requests removal of the selected layer's registered entry for an exact class; removal may wait for a transition.
	 * @param Layer Container from which to remove the entry; unavailable master/layer is ignored.
	 * @param Class Concrete registered class to remove; invalid or unregistered classes are ignored.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = HUD)
	void RemoveByClass(EToroMasterWidgetType Layer,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> Class) const;

	/**
	 * Adds an externally owned, unpooled instance; its creator retains ownership responsibility.
	 * Toro user widgets have their own transitions disabled when added to the menu stack.
	 * @param Layer Destination container; unavailable master/layer is ignored.
	 * @param Instance Entry to add; invalid instances or existing members are ignored.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = HUD)
	void AddByInstance(EToroMasterWidgetType Layer, UCommonActivatableWidget* Instance) const;

	/**
	 * Requests removal of a selected layer's instance, including class-created entries; removal may wait for a transition.
	 * @param Layer Container from which to remove the entry; unavailable master/layer is ignored.
	 * @param Instance Entry to remove; invalid instances or nonmembers are ignored.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = HUD)
	void RemoveByInstance(EToroMasterWidgetType Layer, UCommonActivatableWidget* Instance) const;

protected:

	UPROPERTY(Transient)
	TObjectPtr<UToroMasterWidget> MasterWidget;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
