// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Templates/Casts.h"
#include "Templates/SubclassOf.h"
#include "Templates/UnrealTypeTraits.h"
#include "Misc/CoreMiscDefines.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/StructOpsTypeTraits.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroBehaviorRef.generated.h"

class AActor;
class UToroBehaviorComponent;

/**
 * Resolves a sibling behavior component by class and caches it weakly.
 * UToroBehaviorComponent binds directly declared reference properties, including inherited
 * properties, to its owner on registration and clears their bindings on unregistration.
 * References used elsewhere require explicit owner binding. Neither the owner nor the
 * cached component is kept alive; loading clears both runtime references.
 * Lookup and compatibility errors use Kismet reporting with an active script frame,
 * or a native ensureAlwaysMsgf otherwise.
 */
USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FToroBehaviorRef final
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class FToroBehaviorRefCustomization;
#endif
private:

	/** Whether Get() emits an ensure diagnostic when no valid component can be resolved. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = true))
	bool bRequired;

	/** Component class to search for; instances of derived classes also match. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Reference, NoClear, meta = (AllowAbstract = false, AllowPrivateAccess = true))
	TSubclassOf<UToroBehaviorComponent> Class;

	/** Weak actor binding used for component lookup; never serialized. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<const AActor> Owner;

	/** Weak cache populated by Get(); invalid cached results are retried on subsequent calls. */
	UPROPERTY(Transient)
	TWeakObjectPtr<UToroBehaviorComponent> Reference;

public:

	FToroBehaviorRef()
		: bRequired(false), Class(nullptr), Owner(nullptr), Reference(nullptr)
	{}

	/**
	 * Creates an unbound reference configured to search for T or a derived component class.
	 * @tparam T A complete UToroBehaviorComponent-derived type.
	 * @param bIsRequired Whether Get() should emit an ensure diagnostic on failed resolution.
	 */
	template<typename T>
	[[nodiscard]] static FToroBehaviorRef MakeFor(bool bIsRequired = false)
	{
		static_assert(TIsDerivedFrom<T, UToroBehaviorComponent>::Value,
			"T must derive from UToroBehaviorComponent");

		FToroBehaviorRef Ref;
		Ref.Class = T::StaticClass();
		Ref.bRequired = bIsRequired;
		return Ref;
	}

	/** Returns if this reference is required. */
	[[nodiscard]] FORCEINLINE bool IsRequired() const { return bRequired; }

	/** Returns the configured lookup class, which may be null. */
	[[nodiscard]] FORCEINLINE TSubclassOf<UToroBehaviorComponent> GetClass() const { return Class; }

	/** Returns the bound actor, or nullptr if the weak owner binding is invalid. */
	[[nodiscard]] FORCEINLINE const AActor* GetOwner() const { return Owner.Get(); }

	/** Returns true if the configured class or bound owner is invalid; does not check component availability. */
	[[nodiscard]] bool IsInvalid() const;

	/**
	 * Checks whether the configured lookup class can safely be returned as InClass.
	 * Does not inspect the owner or search for a component.
	 * @param InClass Expected result type; both classes must be valid and the configured
	 *        class must equal or derive from this type.
	 * @param bErrorIfInvalid Whether a failed compatibility check reports a Toro error.
	 */
	[[nodiscard]] bool CanResolveAs(TSubclassOf<UToroBehaviorComponent> InClass, bool bErrorIfInvalid) const;

	/** Clears the cached component without changing the lookup class, required flag, or owner binding. */
	void Reset();

	/**
	 * Replaces the owner binding and clears the component cache without performing a lookup.
	 * @param InActor Actor to search, or nullptr to clear the owner binding.
	 */
	void BindOwner(const AActor* InActor);

	/**
	 * Searches the bound actor for the first component matching the configured class or a subclass.
	 * Bypasses the cache and required-reference check. An invalid owner or class reports a Toro error.
	 * @return The matching component, or nullptr if lookup fails.
	 */
	UToroBehaviorComponent* Find() const;

	/**
	 * Returns the valid cached component, searching and updating the weak cache when necessary.
	 * Failed lookups are retried on subsequent calls and emit an ensure diagnostic if required.
	 * An invalid owner or class also reports a Toro error during lookup.
	 * @return The cached or newly resolved component, or nullptr if resolution fails.
	 */
	UToroBehaviorComponent* Get();

	/**
	 * Checks compatibility with T, then resolves through the cached Get() path.
	 * An incompatible configured class reports a Toro error without attempting lookup.
	 * @tparam T A complete UToroBehaviorComponent-derived type that equals or is a base
	 *         of the configured lookup class.
	 * @return The resolved component cast to T, or nullptr on incompatibility or failed resolution.
	 */
	template<typename T>
	[[nodiscard]] T* Get()
	{
		static_assert(TIsDerivedFrom<T, UToroBehaviorComponent>::Value,
			"T must derive from UToroBehaviorComponent");

		if (CanResolveAs(T::StaticClass(), true))
		{
			return Cast<T>(Get());
		}

		return nullptr;
	}

	void PostSerialize(const FArchive& Ar);
};

template<>
struct TStructOpsTypeTraits<FToroBehaviorRef> : TStructOpsTypeTraitsBase2<FToroBehaviorRef>
{
	enum
	{
		WithPostSerialize = true
	};
};

/**
 * Blueprint compatibility-checked lookup and in-place cache and owner control for behavior references.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROSYSTEMS_API UToroBehaviorRefLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Returns true if the reference has an invalid lookup class or owner; does not search for a component. */
	UFUNCTION(BlueprintPure, Category = "ActorBehaviors|Reference", DisplayName = "Is Invalid")
	static bool IsBehaviorRefInvalid(const FToroBehaviorRef& BehaviorRef);

	/**
	 * Searches for a matching component without reading or updating the cache or enforcing required status.
	 * Incompatible types and invalid lookup classes or owners report a Toro error.
	 * @param InClass Expected result type; must equal or be a base of the configured lookup class.
	 * @return The matching component, or nullptr if compatibility or lookup fails.
	 */
	UFUNCTION(BlueprintPure, Category = "ActorBehaviors|Reference", DisplayName = "Find Reference",
		meta = (DeterminesOutputType = InClass, CompactNodeTitle = "FIND"))
	static UToroBehaviorComponent* FindBehaviorRef(const FToroBehaviorRef& BehaviorRef,
		TSubclassOf<UToroBehaviorComponent> InClass);

	/**
	 * Resolves a compatible component through the reference's weak cache, updating it when invalid.
	 * Incompatible types report a Toro error without lookup. Failed compatible lookups emit an
	 * ensure diagnostic if required; invalid lookup classes or owners also report a Toro error.
	 * @param InClass Expected result type; must equal or be a base of the configured lookup class.
	 * @return The cached or newly resolved component, or nullptr if compatibility or resolution fails.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorBehaviors|Reference", DisplayName = "Get Reference",
		meta = (DeterminesOutputType = InClass, CompactNodeTitle = "RESOLVE"))
	static UToroBehaviorComponent* GetBehaviorRef(UPARAM(ref) FToroBehaviorRef& BehaviorRef,
		TSubclassOf<UToroBehaviorComponent> InClass);

	/** Clears the cached component while preserving the lookup class, required flag, and owner binding. */
	UFUNCTION(BlueprintCallable, Category = "ActorBehaviors|Reference", DisplayName = "Reset")
	static void ResetBehaviorRef(UPARAM(ref) FToroBehaviorRef& BehaviorRef);

	/**
	 * Replaces the reference's owner binding and clears its cached component without searching.
	 * @param InActor Actor to search, or nullptr to clear the owner binding.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorBehaviors|Reference", DisplayName = "Bind Owner", meta = (DefaultToSelf = InActor))
	static void BindBehaviorRefOwner(UPARAM(ref) FToroBehaviorRef& BehaviorRef, const AActor* InActor);
};
