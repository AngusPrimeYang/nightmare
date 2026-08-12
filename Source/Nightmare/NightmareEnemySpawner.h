// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightmareEnemySpawner.generated.h"

class UNightmareSpawnScheduler;
class UNightmareEdgeSpawnLocator;
class UNightmareEnemyRoller;
class ANightmareEnemyActor;

/**
 * World bridge: ticks scheduler, rolls edge location + enemy stats, spawns enemies.
 */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmareEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	ANightmareEnemySpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemySpawn")
	int32 SpawnEnemiesAtEdge(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemySpawn")
	ANightmareEnemyActor* SpawnOneEnemyAt(const FVector& WorldLocation);

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	UNightmareSpawnScheduler* GetScheduler() const { return Scheduler; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	UNightmareEdgeSpawnLocator* GetEdgeLocator() const { return EdgeLocator; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	UNightmareEnemyRoller* GetEnemyRoller() const { return EnemyRoller; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	int32 GetAliveEnemyCount() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemySpawn")
	TObjectPtr<UNightmareSpawnScheduler> Scheduler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemySpawn")
	TObjectPtr<UNightmareEdgeSpawnLocator> EdgeLocator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemySpawn")
	TObjectPtr<UNightmareEnemyRoller> EnemyRoller;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|EnemySpawn", meta = (ClampMin = "1.0"))
	float PlayAreaHalfExtentX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|EnemySpawn", meta = (ClampMin = "1.0"))
	float PlayAreaHalfExtentY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|EnemySpawn", meta = (ClampMin = "0.0"))
	float EdgeBandWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|EnemySpawn")
	float SpawnHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|EnemySpawn", meta = (ClampMin = "0.0"))
	float MinSpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|EnemySpawn", meta = (ClampMin = "0.0"))
	float MaxSpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|EnemySpawn")
	int32 MaxAliveEnemies;

	/** PIE smoke: spawn this many on BeginPlay so edges are immediately testable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|EnemySpawn", meta = (ClampMin = "0"))
	int32 InitialSpawnCount;

	UPROPERTY()
	TArray<TObjectPtr<ANightmareEnemyActor>> AliveEnemies;

	void EnsureHelpers();
	void SyncLocatorFromProperties();
	void PruneAliveList();
};
