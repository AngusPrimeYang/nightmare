// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NightmareItemTypes.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Item")
	float StaminaDeltaOnUse;

	FNightmareItemDef()
		: ItemId(NAME_None)
		, DisplayName(TEXT("Item"))
		, MaxUses(1)
		, StaminaDeltaOnUse(10.0f)
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