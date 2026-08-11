// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightmareItemTypes.h"
#include "NightmarePickupActor.generated.h"

class UNightmareInventoryComponent;
class USceneComponent;

/**
 * Placeable world pickup (C++ shell). Collection rules are Spec-tested via TryCollectInto.
 * Humans wire meshes / levels in Editor — this loop does not edit .uasset maps.
 */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmarePickupActor : public AActor
{
	GENERATED_BODY()

public:
	ANightmarePickupActor();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Pickup")
	bool TryCollectInto(UNightmareInventoryComponent* Inventory);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Pickup")
	bool IsCollected() const { return bCollected; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Pickup")
	void SetItemDef(const FNightmareItemDef& InDef);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Pickup")
	FNightmareItemDef GetItemDef() const { return ItemDef; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Pickup")
	FNightmareItemDef ItemDef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Pickup")
	bool bCollected;
};