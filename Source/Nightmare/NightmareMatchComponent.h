// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NightmareItemTypes.h"
#include "NightmareMatchComponent.generated.h"

class UNightmareStaminaComponent;

/**
 * Survive-duration match rules. Failed (stamina depleted) wins over Won on the same evaluation.
 */
UCLASS(Blueprintable, ClassGroup = (Nightmare), meta = (BlueprintSpawnableComponent))
class NIGHTMARE_API UNightmareMatchComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNightmareMatchComponent();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Match")
	void ResetMatch();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Match")
	void TickMatch(float DeltaSeconds, const UNightmareStaminaComponent* Stamina);

	/** Pure evaluation helper for Specs / same-frame priority checks. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Match")
	ENightmareMatchState Evaluate(bool bStaminaDepleted, float InElapsedSeconds) const;

	UFUNCTION(BlueprintPure, Category = "Nightmare|Match")
	ENightmareMatchState GetMatchState() const { return MatchState; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Match")
	float GetElapsedSeconds() const { return ElapsedSeconds; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Match")
	float GetSurviveDurationSeconds() const { return SurviveDurationSeconds; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Match")
	void SetSurviveDurationSeconds(float NewDuration);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|Match", meta = (ClampMin = "0.0"))
	float SurviveDurationSeconds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Match")
	float ElapsedSeconds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Match")
	ENightmareMatchState MatchState;
};