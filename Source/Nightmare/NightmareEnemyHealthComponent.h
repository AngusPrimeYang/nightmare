// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NightmareEnemyHealthComponent.generated.h"

/**
 * P18: enemy-only health pool. No global drain — separate from player UNightmareStaminaComponent.
 */
UCLASS(Blueprintable, ClassGroup = (Nightmare), meta = (BlueprintSpawnableComponent))
class NIGHTMARE_API UNightmareEnemyHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNightmareEnemyHealthComponent();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemyHealth")
	void ResetHealth();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemyHealth")
	void SetMaxHealth(float NewMaxHealth);

	/** Applies positive damage, clamps to 0, updates depleted state. Returns remaining health. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|EnemyHealth")
	float ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemyHealth")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemyHealth")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemyHealth")
	bool IsDepleted() const { return bIsDepleted; }

	/** P18 default baseline = PlayerMaxStamina / 4 (100 -> 25). */
	UFUNCTION(BlueprintPure, Category = "Nightmare|EnemyHealth")
	static float ComputeDefaultBaseline(float PlayerMaxStamina);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemyHealth", meta = (ClampMin = "1.0"))
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemyHealth")
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|EnemyHealth")
	bool bIsDepleted;

private:
	void RefreshDepletedState();
};
