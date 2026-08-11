// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NightmareStaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNightmareStaminaDepletedSignature);

/**
 * Rule-layer stamina for the first gameplay slice.
 * Drain is advanced via TickStamina (injectable); no World required for Specs.
 */
UCLASS(Blueprintable, ClassGroup = (Nightmare), meta = (BlueprintSpawnableComponent))
class NIGHTMARE_API UNightmareStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNightmareStaminaComponent();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Stamina")
	void ResetStamina();

	/** Applies a signed delta, clamps to [0, MaxStamina], and updates depleted state. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Stamina")
	float ApplyDelta(float Delta);

	/** Advances global drain: ApplyDelta(-StaminaDrainPerSecond * DeltaSeconds). */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Stamina")
	void TickStamina(float DeltaSeconds);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Stamina")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Stamina")
	float GetStaminaDrainPerSecond() const { return StaminaDrainPerSecond; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Stamina")
	bool IsDepleted() const { return bIsDepleted; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Stamina")
	void SetMaxStamina(float NewMaxStamina);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Stamina")
	void SetStaminaDrainPerSecond(float NewDrainPerSecond);

	UPROPERTY(BlueprintAssignable, Category = "Nightmare|Stamina")
	FNightmareStaminaDepletedSignature OnStaminaDepleted;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|Stamina", meta = (ClampMin = "0.0"))
	float MaxStamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|Stamina", meta = (ClampMin = "0.0"))
	float StaminaDrainPerSecond;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Stamina")
	bool bIsDepleted;

private:
	void RefreshDepletedState();
};