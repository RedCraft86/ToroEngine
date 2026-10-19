// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "GameEntityComponent.h"
#include "GameEntityComponentRef.generated.h"

class AGameEntityActor;

/**
 * Weak, lazily resolved reference to a component class on a bound AGameEntityActor.
 * Direct scalar reflected fields on UGameEntityComponent are bound during registration and cleared
 * on unregistration. Standalone values, nested structs, and containers are not automatically bound.
 * Access requires a live owner, configured class, and matching component; missing required components
 * retain checked-failure behavior. Neither the owner nor the cached component is kept alive.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FGameEntityComponentRef final
{
	GENERATED_BODY()

private:

	friend class UGameEntityComponent;
#if WITH_EDITOR
	friend class FGameEntityComponentRefCustomization;
#endif

	/**
	 * Configured lookup class, editable on defaults; typed access requires an exact class match.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Reference)
	TSubclassOf<UGameEntityComponent> ComponentClass;

	/**
	 * Transient weak owner used for lookup; cleared on loading and component unregistration.
	 */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> OwnerActor;

	/**
	 * Transient weak component cache, reused while valid and resolved again after expiry or reset.
	 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UGameEntityComponent> Reference;

	/**
	 * Sets the lookup owner and clears the cached component without changing the configured class.
	 * @param InOwner Actor used for subsequent lookup; nullptr removes the binding.
	 */
	void BindOwner(AActor* InOwner);
	/**
	 * Looks up the configured component on the bound owner without updating the cache.
	 * Logs an error when the owner or configured class is unavailable.
	 * @return First matching component or subclass instance, or nullptr when lookup cannot resolve it.
	 */
	UGameEntityComponent* FindComponent() const;

public:

	/**
	 * Creates an unconfigured reference with no bound owner or cached component.
	 */
	FGameEntityComponentRef() = default;

	/**
	 * Creates a reference configured for a reflected entity-component type, without binding an owner.
	 * @tparam T Reflected type derived from UGameEntityComponent.
	 * @return Configured reference that must be bound before component access.
	 */
	template<typename T>
	static FGameEntityComponentRef For()
	{
		static_assert(TIsDerivedFrom<T, UGameEntityComponent>::Value,
			"T must derive from UGameEntityComponent");

		FGameEntityComponentRef Result;
		Result.ComponentClass = T::StaticClass();
		return Result;
	}

	/**
	 * Gets the bound owner, or nullptr when the binding is absent or its weak pointer has expired.
	 */
	const AActor* GetOwner() const { return OwnerActor.Get(); }
	/**
	 * Gets the configured lookup class, which may be unset.
	 */
	TSubclassOf<UGameEntityComponent> GetClass() const { return ComponentClass; }

	/**
	 * Resolves the required component after verifying the configured class exactly matches T.
	 * Matching a base or derived lookup class is not sufficient for this type check, although the
	 * resolved instance may derive from the configured class.
	 * @tparam T Reflected type derived from UGameEntityComponent, equal to the configured class.
	 * @return Typed component, or nullptr after an ensure diagnostic for a class mismatch.
	 * For a matching class, unresolved required references retain the non-template Get() checkf behavior.
	 */
	template<typename T>
	T* Get()
	{
		static_assert(TIsDerivedFrom<T, UGameEntityComponent>::Value,
			"T must derive from UGameEntityComponent");

		const UClass* OutClass = T::StaticClass();
		if (!ensureMsgf(OutClass == ComponentClass,
			TEXT("FGameEntityComponentRef::Get<...>() expects %s but reference is for %s."),
			*GetNameSafe(OutClass), *GetNameSafe(ComponentClass)))
		{
			// Fix: A failed typed pointer lookup must return a null pointer.
			return nullptr;
		}

		return Cast<T>(Get());
	}

	/**
	 * Clears only the cached component; retains the owner binding and configured class for the next lookup.
	 */
	void Reset();
	/**
	 * Returns the live cached component or resolves and caches the first matching component on the owner.
	 * The owner, configured class, and matching component must be available before access.
	 * An unresolved lookup triggers checkf when checks are enabled; it is not an optional-reference query.
	 * @return Resolved component, potentially nullptr when lookup fails in a build with checks disabled.
	 */
	UGameEntityComponent* Get();

	/**
	 * Clears the owner binding and component cache when loading, preserving the configured class.
	 * Registration must bind the owner again before access. Other archive operations leave binding unchanged.
	 * @param Ar Archive used to determine whether this operation is loading.
	 */
	void PostSerialize(const FArchive& Ar);
};

/**
 * Enables the post-serialization hook that invalidates owner binding and cache after loading.
 */
template<>
struct TStructOpsTypeTraits<FGameEntityComponentRef> : TStructOpsTypeTraitsBase2<FGameEntityComponentRef>
{
	enum { WithPostSerialize = true };
};

//// BELOW THIS LINE ARE TESTS, DO NOT COMMENT ON THEM

UCLASS(Blueprintable, ClassGroup = (Modular), meta = (BlueprintSpawnableComponent, ChildCanTick = true))
class TOROENGINE_API UGameEntityComponentA : public UGameEntityComponent
{
	GENERATED_BODY()

public:

	UGameEntityComponentA(){}
};


UCLASS(Blueprintable, ClassGroup = (Modular), meta = (BlueprintSpawnableComponent, ChildCanTick = true))
class TOROENGINE_API UGameEntityComponentB : public UGameEntityComponent
{
	GENERATED_BODY()

public:

	UGameEntityComponentB()
		: ReferenceToA(FGameEntityComponentRef::For<UGameEntityComponentA>())
	{}
	
	UPROPERTY(EditAnywhere, Category = Settings)
	FGameEntityComponentRef ReferenceToA;
};