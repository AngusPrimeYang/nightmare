// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NightmareItemTypes.generated.h"

UENUM(BlueprintType)
enum class ENightmareItemEffectType : uint8
{
	Stamina UMETA(DisplayName = "Stamina"),
	Speed UMETA(DisplayName = "Speed"),
	Jump UMETA(DisplayName = "Jump")
};

UENUM(BlueprintType)
enum class ENightmareItemInteractMode : uint8
{
	/** P2-A: overlap applies effect immediately and despawns. */
	TouchInstant UMETA(DisplayName = "TouchInstant"),
	/** P2-B: collect into inventory, then use from a slot. */
	HoldToUse UMETA(DisplayName = "HoldToUse")
};

USTRUCT(BlueprintType)
struct NIGHTMARE_API FNightmareItemDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item", meta = (ClampMin = "1"))
	int32 MaxUses;

	/** Exactly one effect per item (P2/P8/P9 mutual exclusion). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item")
	ENightmareItemEffectType EffectType;

	/** Exactly one interact mode (P2 A/B mutual exclusion). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item")
	ENightmareItemInteractMode InteractMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Stamina")
	float StaminaDeltaOnUse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Speed", meta = (ClampMin = "0.0"))
	float SpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Jump", meta = (ClampMin = "0.0"))
	float JumpHeightMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item|Jump", meta = (ClampMin = "0.0"))
	float GravityScale;

	/** Duration for Speed/Jump effects (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item", meta = (ClampMin = "0.0"))
	float EffectDuration;

	FNightmareItemDef()
		: ItemId(NAME_None)
		, DisplayName(TEXT("Item"))
		, MaxUses(1)
		, EffectType(ENightmareItemEffectType::Stamina)
		, InteractMode(ENightmareItemInteractMode::HoldToUse)
		, StaminaDeltaOnUse(10.0f)
		, SpeedMultiplier(1.0f)
		, JumpHeightMultiplier(1.0f)
		, GravityScale(1.0f)
		, EffectDuration(0.0f)
	{
	}
};

/** Payload produced when an item use is consumed (inventory or touch). */
USTRUCT(BlueprintType)
struct NIGHTMARE_API FNightmareItemUseResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Nightmare|Item")
	ENightmareItemEffectType EffectType;

	UPROPERTY(BlueprintReadOnly, Category = "Nightmare|Item")
	float StaminaDelta;

	UPROPERTY(BlueprintReadOnly, Category = "Nightmare|Item")
	float SpeedMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "Nightmare|Item")
	float JumpHeightMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "Nightmare|Item")
	float GravityScale;

	UPROPERTY(BlueprintReadOnly, Category = "Nightmare|Item")
	float EffectDuration;

	FNightmareItemUseResult()
		: EffectType(ENightmareItemEffectType::Stamina)
		, StaminaDelta(0.0f)
		, SpeedMultiplier(1.0f)
		, JumpHeightMultiplier(1.0f)
		, GravityScale(1.0f)
		, EffectDuration(0.0f)
	{
	}
};

UENUM(BlueprintType)
enum class ENightmareMatchState : uint8
{
	Playing UMETA(DisplayName = "Playing"),
	Won UMETA(DisplayName = "Won"),
	Failed UMETA(DisplayName = "Failed")
};
