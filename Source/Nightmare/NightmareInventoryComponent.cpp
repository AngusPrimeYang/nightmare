// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareInventoryComponent.h"
#include "NightmareItemEffectApply.h"
#include "NightmareItemInstance.h"
#include "NightmarePlayerEffectComponent.h"
#include "NightmareStaminaComponent.h"

UNightmareInventoryComponent::UNightmareInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SlotCount = 3;
	EnsureSlotArray();
}

void UNightmareInventoryComponent::EnsureSlotArray()
{
	SlotCount = FMath::Clamp(SlotCount, 1, 16);
	if (Slots.Num() != SlotCount)
	{
		Slots.SetNum(SlotCount);
	}
}

int32 UNightmareInventoryComponent::CountOccupied() const
{
	int32 Count = 0;
	for (const TObjectPtr<UNightmareItemInstance>& Slot : Slots)
	{
		if (Slot)
		{
			++Count;
		}
	}
	return Count;
}

bool UNightmareInventoryComponent::IsFull() const
{
	return CountOccupied() >= Slots.Num();
}

UNightmareItemInstance* UNightmareInventoryComponent::GetSlotItem(int32 SlotIndex) const
{
	if (!Slots.IsValidIndex(SlotIndex))
	{
		return nullptr;
	}
	return Slots[SlotIndex];
}

int32 UNightmareInventoryComponent::TryAddItem(UNightmareItemInstance* Item)
{
	EnsureSlotArray();
	if (!Item)
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index <Slots.Num(); ++Index)
	{
		if (!Slots[Index])
		{
			Slots[Index] = Item;
			return Index;
		}
	}
	return INDEX_NONE;
}

void UNightmareInventoryComponent::ClearSlot(int32 SlotIndex)
{
	EnsureSlotArray();
	if (Slots.IsValidIndex(SlotIndex))
	{
		Slots[SlotIndex] = nullptr;
	}
}

void UNightmareInventoryComponent::ClearAll()
{
	EnsureSlotArray();
	for (int32 Index = 0; Index <Slots.Num(); ++Index)
	{
		Slots[Index] = nullptr;
	}
}

bool UNightmareInventoryComponent::TryUseSlot(
	int32 SlotIndex,
	UNightmareStaminaComponent* Stamina,
	UNightmarePlayerEffectComponent* Effects)
{
	EnsureSlotArray();
	if (!Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex] || !Slots[SlotIndex]->CanUse())
	{
		return false;
	}

	// Peek effect type before consume to validate targets.
	const ENightmareItemEffectType EffectType = Slots[SlotIndex]->GetEffectType();
	if (EffectType == ENightmareItemEffectType::Stamina && !Stamina)
	{
		return false;
	}
	if ((EffectType == ENightmareItemEffectType::Speed || EffectType == ENightmareItemEffectType::Jump) && !Effects)
	{
		return false;
	}

	FNightmareItemUseResult Result;
	if (!Slots[SlotIndex]->TryUse(Result))
	{
		return false;
	}

	if (!NightmareItemEffectApply::ApplyUseResult(Result, Stamina, Effects))
	{
		// Should not happen after pre-check; treat as failure without restoring use (rare).
		return false;
	}

	if (Slots[SlotIndex]->GetRemainingUses() <= 0)
	{
		Slots[SlotIndex] = nullptr;
	}
	return true;
}
