// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NightmareDevGameMode.generated.h"

class ANightmareItemSpawner;
class AController;
class APawn;

/** Dev smoke GameMode: default pawn is ANightmareDevCharacter; spawns item spawner. */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmareDevGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANightmareDevGameMode();

	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

protected:
	/** Repositions PlayerStart actors and pawns so Open World landscape cannot bury spawn. */
	void ApplySafeSpawnTransforms();

	void EnsurePawnSafeSpawn(APawn* Pawn) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Spawn")
	FVector ItemSpawnerLocation;

	/** Preferred XY + minimum Z for PlayerStart / pawn (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Spawn")
	FVector SafeSpawnLocation;

	/** If true, force SafeSpawnLocation even when PlayerStart Z looks OK. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Spawn")
	bool bForceSafeSpawnLocation;

	/** Line-trace from this height down to find landscape surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Spawn", meta = (ClampMin = "100.0"))
	float SpawnTraceHeight;

	/** Capsule half-height padding above hit surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Spawn", meta = (ClampMin = "0.0"))
	float SpawnSurfacePadding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Spawn")
	TObjectPtr<ANightmareItemSpawner> ItemSpawner;
};
