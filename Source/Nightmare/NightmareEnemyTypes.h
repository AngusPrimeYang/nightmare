// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NightmareEnemyTypes.generated.h"

UENUM(BlueprintType)
enum class ENightmareEnemyMoveMode : uint8
{
	Chase UMETA(DisplayName = "Chase"),
	Wander UMETA(DisplayName = "Wander")
};

/** P12: spawn-time locomotion identity. Chase/Wander may retarget; this type does not. */
UENUM(BlueprintType)
enum class ENightmareEnemyLocomotionType : uint8
{
	Hover UMETA(DisplayName = "Hover"),
	Walk UMETA(DisplayName = "Walk")
};

/** Per-enemy stats assigned at spawn (P5). AttackPower is consumed by P6. */
USTRUCT(BlueprintType)
struct NIGHTMARE_API FNightmareEnemyStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy", meta = (ClampMin = "0.0"))
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy", meta = (ClampMin = "0.0"))
	float AttackPower;

	FNightmareEnemyStats()
		: MoveSpeed(300.0f)
		, AttackPower(10.0f)
	{
	}
};
