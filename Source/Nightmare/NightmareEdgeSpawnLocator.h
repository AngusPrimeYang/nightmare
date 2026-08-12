// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NightmareEdgeSpawnLocator.generated.h"

/**
 * P4: rolls a world location on the perimeter band of a rectangular play area.
 * Spec-friendly — no World required; inject seed / stream.
 */
UCLASS(BlueprintType)
class NIGHTMARE_API UNightmareEdgeSpawnLocator : public UObject
{
	GENERATED_BODY()

public:
	UNightmareEdgeSpawnLocator();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemySpawn")
	void SetOrigin(const FVector& InOrigin);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemySpawn")
	void SetPlayAreaHalfExtent(float InHalfExtentX, float InHalfExtentY);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemySpawn")
	void SetEdgeBandWidth(float InEdgeBandWidth);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemySpawn")
	void SetSpawnHeight(float InSpawnHeight);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemySpawn")
	void SetRandomSeed(int32 Seed);

	/** Rolls next edge location using the internal seeded stream. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemySpawn")
	FVector RollLocation();

	/** Deterministic roll with an injected stream (Specs / callers). */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemySpawn")
	FVector RollLocationWithStream(UPARAM(ref) FRandomStream& Stream) const;

	/**
	 * True if XY is inside the outer rectangle and outside the shrunk inner rectangle
	 * (i.e. within EdgeBandWidth of the perimeter). Z is ignored.
	 */
	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	bool IsOnEdgeBand(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	FVector GetOrigin() const { return Origin; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	float GetHalfExtentX() const { return HalfExtentX; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	float GetHalfExtentY() const { return HalfExtentY; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	float GetEdgeBandWidth() const { return EdgeBandWidth; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemySpawn")
	float GetSpawnHeight() const { return SpawnHeight; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemySpawn")
	FVector Origin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemySpawn", meta = (ClampMin = "1.0"))
	float HalfExtentX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemySpawn", meta = (ClampMin = "1.0"))
	float HalfExtentY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemySpawn", meta = (ClampMin = "0.0"))
	float EdgeBandWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemySpawn")
	float SpawnHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemySpawn")
	int32 RandomSeed;

private:
	FRandomStream RandomStream;

	void ClampExtents();
};
