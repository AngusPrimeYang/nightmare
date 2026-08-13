// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NightmareFailPopupState.generated.h"

/**
 * Pure fail-popup countdown state (U5–U8). Spec-friendly; no World required.
 */
UCLASS(BlueprintType)
class NIGHTMARE_API UNightmareFailPopupState : public UObject
{
	GENERATED_BODY()

public:
	UNightmareFailPopupState();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void Open();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void Close();

	/** Returns true when countdown reached zero (auto return-to-menu). */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	bool TickCountdown(float DeltaSeconds);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Flow")
	bool IsOpen() const { return bIsOpen; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Flow")
	float GetRemainingSeconds() const { return RemainingSeconds; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Flow")
	int32 GetDisplayCountdownSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Nightmare|Flow")
	float GetCountdownDurationSeconds() const { return CountdownDurationSeconds; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void SetCountdownDurationSeconds(float NewDuration);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nightmare|Flow", meta = (ClampMin = "0.0"))
	float CountdownDurationSeconds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Flow")
	float RemainingSeconds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Flow")
	bool bIsOpen;
};
