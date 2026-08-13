// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NightmareEnemyTypes.h"
#include "NightmareEnemyRoller.generated.h"

/**
 * P5: at spawn time rolls MoveSpeed / AttackPower and initial move mode.
 * P12: also rolls Hover vs Walk locomotion (once per spawn; not retargeted).
 * P18: rolls enemy MaxHealth (>= 1, separate from player stamina drain).
 * Spec-friendly — inject seed / FRandomStream; no World required.
 */
UCLASS(BlueprintType)
class NIGHTMARE_API UNightmareEnemyRoller : public UObject
{
	GENERATED_BODY()

public:
	UNightmareEnemyRoller();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetRandomSeed(int32 Seed);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	FNightmareEnemyStats RollStats();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	FNightmareEnemyStats RollStatsWithStream(UPARAM(ref) FRandomStream& Stream) const;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	ENightmareEnemyMoveMode RollMoveMode();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	ENightmareEnemyMoveMode RollMoveModeWithStream(UPARAM(ref) FRandomStream& Stream) const;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	ENightmareEnemyLocomotionType RollLocomotionType();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	ENightmareEnemyLocomotionType RollLocomotionTypeWithStream(UPARAM(ref) FRandomStream& Stream) const;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	float RollHealth();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	float RollHealthWithStream(UPARAM(ref) FRandomStream& Stream) const;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	float RollRetargetInterval();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	float RollRetargetIntervalWithStream(UPARAM(ref) FRandomStream& Stream) const;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	FVector RollWanderDirection();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	FVector RollWanderDirectionWithStream(UPARAM(ref) FRandomStream& Stream) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "0.0"))
	float MoveSpeedMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "0.0"))
	float MoveSpeedMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "0.0"))
	float AttackPowerMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "0.0"))
	float AttackPowerMax;

	/** P18: rolled MaxHealth range (clamped to >= 1). Default centers on 25 (= 100/4). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "1.0"))
	float HealthMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "1.0"))
	float HealthMax;

	/** Chance to enter Chase when retargeting (remainder = Wander). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChaseModeChance;

	/** Chance to roll Hover at spawn (remainder = Walk). Sticky for the enemy lifetime. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HoverLocomotionChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "0.0"))
	float RetargetIntervalMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy|Roll", meta = (ClampMin = "0.0"))
	float RetargetIntervalMax;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Enemy")
	int32 RandomSeed;

private:
	FRandomStream RandomStream;
};
