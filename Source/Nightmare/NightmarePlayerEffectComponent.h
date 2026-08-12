// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NightmarePlayerEffectComponent.generated.h"

class UCharacterMovementComponent;

/**
 * Temporary Speed / Jump modifiers (P8 / P9). Capture baseline from movement, restore on expire.
 */
UCLASS(Blueprintable, ClassGroup = (Nightmare), meta = (BlueprintSpawnableComponent))
class NIGHTMARE_API UNightmarePlayerEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNightmarePlayerEffectComponent();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Effect")
	void CaptureBaselineFromMovement(UCharacterMovementComponent* Movement);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Effect")
	void ApplySpeedEffect(float Multiplier, float DurationSeconds);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Effect")
	void ApplyJumpEffect(float JumpHeightMultiplier, float InGravityScale, float DurationSeconds);

	/** Applies timed effects to Movement; call from owner Tick. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Effect")
	void TickEffects(float DeltaSeconds, UCharacterMovementComponent* Movement);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Effect")
	bool HasActiveSpeedEffect() const { return SpeedTimeRemaining > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Effect")
	bool HasActiveJumpEffect() const { return JumpTimeRemaining > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Effect")
	float GetSpeedTimeRemaining() const { return SpeedTimeRemaining; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Effect")
	float GetJumpTimeRemaining() const { return JumpTimeRemaining; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Effect")
	float BaselineMaxWalkSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Effect")
	float BaselineJumpZVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Effect")
	float BaselineGravityScale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Effect")
	float SpeedMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Effect")
	float SpeedTimeRemaining;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Effect")
	float JumpHeightMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Effect")
	float ActiveGravityScale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Effect")
	float JumpTimeRemaining;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Effect")
	bool bBaselineCaptured;

	void ApplyToMovement(UCharacterMovementComponent* Movement) const;
};
