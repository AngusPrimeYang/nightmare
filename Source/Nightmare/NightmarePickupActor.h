// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightmareItemTypes.h"
#include "NightmarePickupActor.generated.h"

class UNightmareInventoryComponent;
class UNightmareStaminaComponent;
class UNightmarePlayerEffectComponent;
class USceneComponent;
class UStaticMeshComponent;
class USphereComponent;
class ACharacter;

/**
 * World pickup (G3). HoldToUse → inventory; TouchInstant → apply on overlap / TryTouchApply.
 */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmarePickupActor : public AActor
{
	GENERATED_BODY()

public:
	ANightmarePickupActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Pickup")
	bool TryCollectInto(UNightmareInventoryComponent* Inventory);

	/** P2-A: apply rolled effect immediately (Spec-friendly; also used by overlap). */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Pickup")
	bool TryTouchApply(
		UNightmareStaminaComponent* Stamina,
		UNightmarePlayerEffectComponent* Effects,
		ACharacter* KnockbackCharacter);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Pickup")
	bool IsCollected() const { return bCollected; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Pickup")
	void SetItemDef(const FNightmareItemDef& InDef);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Pickup")
	FNightmareItemDef GetItemDef() const { return ItemDef; }

protected:
	void MarkCollectedAndHide();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GrayboxMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> TouchSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Pickup")
	FNightmareItemDef ItemDef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Pickup")
	bool bCollected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Graybox", meta = (ClampMin = "0.0"))
	float YawRotateSpeedDegrees;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Pickup", meta = (ClampMin = "0.0"))
	float KnockbackHorizontalSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Pickup", meta = (ClampMin = "0.0"))
	float KnockbackUpSpeed;

	UFUNCTION()
	void OnTouchBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
