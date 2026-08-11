// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareInventoryComponent.h"
#include "NightmareItemInstance.h"
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

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
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
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		Slots[Index] = nullptr;
	}
}

bool UNightmareInventoryComponent::TryUseSlot(int32 SlotIndex, UNightmareStaminaComponent* Stamina)
{
	EnsureSlotArray();
	if (!Stamina || !Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex])
	{
		return false;
	}

	float Delta = 0.0f;
	if (!Slots[SlotIndex]->TryUse(Delta))
	{
		return false;
	}

	Stamina->ApplyDelta(Delta);
	if (Slots[SlotIndex]->GetRemainingUses() <= 0)
	{
		Slots[SlotIndex] = nullptr;
	}
	return true;
}