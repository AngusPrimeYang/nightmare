// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NightmareSpawnScheduler.generated.h"

/**
 * Random-interval spawn clock. Inject delays / seed for Specs — no World required.
 */
UCLASS(BlueprintType)
class NIGHTMARE_API UNightmareSpawnScheduler : public UObject
{
	GENERATED_BODY()

public:
	UNightmareSpawnScheduler();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Spawn")
	void SetIntervalRange(float InMinSeconds, float InMaxSeconds);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Spawn")
	void SetRandomSeed(int32 Seed);

	/** Forces the next countdown (test injection). */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Spawn")
	void SetTimeUntilNextSpawn(float Seconds);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Spawn")
	void ResetScheduler();

	/** Advances clock. Returns how many spawn events fired this call (0 or more if dt is large). */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Spawn")
	int32 TickSpawn(float DeltaSeconds);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Spawn")
	float GetTimeUntilNextSpawn() const { return TimeUntilNextSpawn; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Spawn")
	float GetMinSpawnInterval() const { return MinSpawnInterval; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Spawn")
	float GetMaxSpawnInterval() const { return MaxSpawnInterval; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|Spawn", meta = (ClampMin = "0.0"))
	float MinSpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|Spawn", meta = (ClampMin = "0.0"))
	float MaxSpawnInterval;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Spawn")
	float TimeUntilNextSpawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Spawn")
	int32 RandomSeed;

private:
	FRandomStream RandomStream;
	float RollNextInterval();
};