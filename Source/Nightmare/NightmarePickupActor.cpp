// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmarePickupActor.h"
#include "NightmareInventoryComponent.h"
#include "NightmareItemInstance.h"
#include "Components/SceneComponent.h"

ANightmarePickupActor::ANightmarePickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	bCollected = false;
	ItemDef.ItemId = TEXT("Pickup");
	ItemDef.DisplayName = TEXT("Pickup");
	ItemDef.MaxUses = 1;
	ItemDef.StaminaDeltaOnUse = 10.0f;
}

void ANightmarePickupActor::SetItemDef(const FNightmareItemDef& InDef)
{
	ItemDef = InDef;
}

bool ANightmarePickupActor::TryCollectInto(UNightmareInventoryComponent* Inventory)
{
	if (bCollected || !Inventory)
	{
		return false;
	}

	UNightmareItemInstance* Instance = NewObject<UNightmareItemInstance>(Inventory);
	Instance->InitializeFromDef(ItemDef);
	const int32 Slot = Inventory->TryAddItem(Instance);
	if (Slot == INDEX_NONE)
	{
		return false;
	}

	bCollected = true;
	return true;
}