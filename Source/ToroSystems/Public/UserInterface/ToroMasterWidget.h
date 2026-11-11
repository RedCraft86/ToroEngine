// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonUserWidget.h"
#include "ToroMasterWidget.generated.h"

class UToroMasterMenuStack;
class UToroMasterWidgetOverlay;
class UCommonActivatableWidget;

/** Selects the master UI container used for an entry. */
UENUM(BlueprintType)
enum class EToroMasterWidgetType : uint8
{
	/** Gameplay information displayed beneath menus and hidden while a menu is displayed. */
	Gameplay,

	/** Menu stack whose transitions replace the gameplay presentation. */
	Menu,

	/** Independent forced overlays displayed above both gameplay content and menus. */
	Overlay
};

/**
 * Composes the gameplay/menu stack and an independent forced-overlay container for the player UI.
 * Menus hide the gameplay layer, while forced overlays remain above the stack. Camera fading reduces
 * stack opacity and blocks its pointer input without fading the forced-overlay layer.
 * Toro user widgets supply their own overlay transitions; menu-stack entries use the stack's transitions.
 */
UCLASS(MinimalAPI, NotBlueprintable, NotBlueprintType)
class UToroMasterWidget final : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Adds a pooled activatable entry to the selected container, or returns its existing registered class entry.
	 * @param Layer Destination container; gameplay and forced overlays support simultaneous entries.
	 * @param Class Concrete activatable widget class; null and abstract classes are ignored.
	 * @return Existing or newly added entry, or null if the layer is unavailable or addition fails.
	 */
	TOROSYSTEMS_API UCommonActivatableWidget* AddByClass(EToroMasterWidgetType Layer,
		TSubclassOf<UCommonActivatableWidget> Class) const;

	/**
	 * Requests removal of the selected container's registered entry for an exact class.
	 * Removal may wait for the stack transition or a Toro overlay entry's exit animation.
	 * @param Layer Container from which to remove the entry; unavailable layers are ignored.
	 * @param Class Registered class to remove; invalid or unregistered classes are ignored.
	 */
	TOROSYSTEMS_API void RemoveByClass(EToroMasterWidgetType Layer,
		TSubclassOf<UCommonActivatableWidget> Class) const;

	/**
	 * Adds an externally owned, unpooled instance; its creator retains ownership responsibility.
	 * ToroUserWidgets have their own transitions disabled when added to the menu stack.
	 * @param Layer Destination container; unavailable layers are ignored.
	 * @param Instance Activatable entry to add; invalid instances or existing members are ignored.
	 */
	TOROSYSTEMS_API void AddByInstance(EToroMasterWidgetType Layer,
		UCommonActivatableWidget* Instance) const;

	/**
	 * Requests removal of an instance belonging to the selected container, including class-created entries.
	 * Removal may wait for the stack transition or a Toro overlay entry's exit animation.
	 * @param Layer Container from which to remove the entry; unavailable layers are ignored.
	 * @param Instance Entry to remove; invalid instances or instances outside the selected container are ignored.
	 */
	TOROSYSTEMS_API void RemoveByInstance(EToroMasterWidgetType Layer,
		UCommonActivatableWidget* Instance) const;

private:

	UPROPERTY(Transient)
	TObjectPtr<UToroMasterWidgetOverlay> GameplayLayer;

	UPROPERTY(Transient)
	TObjectPtr<UToroMasterMenuStack> MenuLayer;

	UPROPERTY(Transient)
	TObjectPtr<UToroMasterWidgetOverlay> OverlayLayer;

	bool bUpdateMenuOpacity = false;

	virtual bool Initialize() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
