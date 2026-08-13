// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightmareItemTypes.h"
#include "NightmareItemSpawner.generated.h"

class UNightmareSpawnScheduler;
class UNightmareItemRoller;
class ANightmarePickupActor;

/**
 * P1 world bridge: ticks scheduler, rolls item def, spawns floating pickups near Origin.
 */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmareItemSpawner : public AActor
{
	GENERATED_BODY()

public:
	ANightmareItemSpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Spawn")
	int32 SpawnRolledPickups(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Spawn")
	ANightmarePickupActor* SpawnOnePickupAt(const FVector& WorldLocation, const FNightmareItemDef& Def);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Spawn")
	void ResetSpawnerSession();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Spawn")
	void SetSpawningEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Spawn")
	bool IsSpawningEnabled() const { return bSpawningEnabled; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Spawn")
	UNightmareSpawnScheduler* GetScheduler() const { return Scheduler; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Spawn")
	UNightmareItemRoller* GetRoller() const { return Roller; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Spawn")
	TObjectPtr<UNightmareSpawnScheduler> Scheduler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Spawn")
	TObjectPtr<UNightmareItemRoller> Roller;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Spawn", meta = (ClampMin = "100.0"))
	float SpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Spawn", meta = (ClampMin = "0.0"))
	float HoverHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Spawn")
	int32 MaxAlivePickups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Spawn")
	bool bSpawningEnabled;

	UPROPERTY()
	TArray<TObjectPtr<ANightmarePickupActor>> AlivePickups;

	void EnsureHelpers();
	void PruneAliveList();
	FVector RollSpawnLocation() const;
};
