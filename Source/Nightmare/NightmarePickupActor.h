// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightmareItemTypes.h"
#include "NightmarePickupActor.generated.h"

class UNightmareInventoryComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Placeable world pickup with graybox cube (G3). Collection via TryCollectInto.
 */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmarePickupActor : public AActor
{
	GENERATED_BODY()

public:
	ANightmarePickupActor();

	virtual void Tick(float DeltaSeconds) override;

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

	/** G3 graybox — engine cube, no custom material. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GrayboxMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Pickup")
	FNightmareItemDef ItemDef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Pickup")
	bool bCollected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Graybox", meta = (ClampMin = "0.0"))
	float YawRotateSpeedDegrees;
};
