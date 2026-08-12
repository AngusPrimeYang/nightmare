// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NightmareItemTypes.h"
#include "NightmareItemRoller.generated.h"

/**
 * Pure roll helper: at spawn time chooses interact mode, single effect type, and numeric ranges.
 * Inject FRandomStream / seed in Specs.
 */
UCLASS(BlueprintType)
class NIGHTMARE_API UNightmareItemRoller : public UObject
{
	GENERATED_BODY()

public:
	UNightmareItemRoller();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Item")
	void SetRandomSeed(int32 Seed);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Item")
	FNightmareItemDef RollItemDef();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Item")
	FNightmareItemDef RollItemDefWithStream(UPARAM(ref) FRandomStream& Stream) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Roll")
	float TouchModeChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Roll")
	float StaminaEffectChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Roll")
	float SpeedEffectChance;

	/** Jump chance is 1 - Stamina - Speed (clamped). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Stamina")
	float StaminaDeltaMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Stamina")
	float StaminaDeltaMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Speed", meta = (ClampMin = "0.0"))
	float SpeedMultiplierMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Speed", meta = (ClampMin = "0.0"))
	float SpeedMultiplierMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Jump", meta = (ClampMin = "0.0"))
	float JumpHeightMultiplierMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Jump", meta = (ClampMin = "0.0"))
	float JumpHeightMultiplierMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Jump", meta = (ClampMin = "0.0"))
	float GravityScaleMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Jump", meta = (ClampMin = "0.0"))
	float GravityScaleMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item", meta = (ClampMin = "0.0"))
	float EffectDurationMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item", meta = (ClampMin = "0.0"))
	float EffectDurationMax;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Item")
	int32 RandomSeed;

private:
	FRandomStream RandomStream;
};
