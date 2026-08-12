// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NightmareInventoryComponent.generated.h"

class UNightmareItemInstance;
class UNightmareStaminaComponent;
class UNightmarePlayerEffectComponent;

/** Fixed-capacity inventory (v1: 3 slots). */
UCLASS(Blueprintable, ClassGroup = (Nightmare), meta = (BlueprintSpawnableComponent))
class NIGHTMARE_API UNightmareInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNightmareInventoryComponent();

	UFUNCTION(BlueprintPure, Category = "Nightmare|Inventory")
	int32 GetSlotCount() const { return SlotCount; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Inventory")
	int32 CountOccupied() const;

	UFUNCTION(BlueprintPure, Category = "Nightmare|Inventory")
	bool IsFull() const;

	UFUNCTION(BlueprintPure, Category = "Nightmare|Inventory")
	UNightmareItemInstance* GetSlotItem(int32 SlotIndex) const;

	/** Adds item to first empty slot. Returns slot index or INDEX_NONE. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Inventory")
	int32 TryAddItem(UNightmareItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Inventory")
	void ClearSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Inventory")
	void ClearAll();

	/**
	 * Uses item in slot. Stamina and/or Effects required depending on item EffectType.
	 * Clears slot when RemainingUses hits 0.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Inventory")
	bool TryUseSlot(
		int32 SlotIndex,
		UNightmareStaminaComponent* Stamina,
		UNightmarePlayerEffectComponent* Effects);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|Inventory", meta = (ClampMin = "1", ClampMax = "16"))
	int32 SlotCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Inventory")
	TArray<TObjectPtr<UNightmareItemInstance>> Slots;

	void EnsureSlotArray();
};
