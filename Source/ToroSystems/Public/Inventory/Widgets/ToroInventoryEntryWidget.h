// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonUserWidget.h"
#include "ToroInventoryEntryWidget.generated.h"

class UCommonButtonBase;
class UToroInventoryWidget;
class UInventoryBehavior;
class UItemDefinition;

UCLASS(Abstract, NotBlueprintable, BlueprintType, NotPlaceable)
class TOROSYSTEMS_API UToroInventoryEntryWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelected, const UToroInventoryEntryWidget*)

	UToroInventoryEntryWidget();

	void Init(UInventoryBehavior* InInventory, const UItemDefinition* InItem);

	void SetIsSelected(bool bSelected) const;
	
	const UItemDefinition* GetItem() const
	{
		return Item;
	}

	[[nodiscard]] FORCEINLINE FOnSelected::RegistrationType& GetOnSelected()
	{
		return OnSelectedEvent;
	}

protected:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> SelectButton;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget> NewMarker;

	UPROPERTY(Transient)
	TWeakObjectPtr<UInventoryBehavior> Inventory;

	UPROPERTY(Transient)
	TObjectPtr<const UItemDefinition> Item;

	FOnSelected OnSelectedEvent;

	void OnButtonSelected(bool bSelected) const;

	virtual bool InitEntry();
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
