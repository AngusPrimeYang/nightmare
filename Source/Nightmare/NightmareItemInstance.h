// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NightmareItemTypes.h"
#include "NightmareItemInstance.generated.h"

/**
 * Runtime item with remaining uses. Pure rule object — safe for Automation Specs.
 */
UCLASS(BlueprintType)
class NIGHTMARE_API UNightmareItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Item")
	void InitializeFromDef(const FNightmareItemDef& InDef);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Item")
	bool CanUse() const;

	/** Consumes one use when available. OutStaminaDelta receives Def.StaminaDeltaOnUse. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Item")
	bool TryUse(float& OutStaminaDelta);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Item")
	int32 GetRemainingUses() const { return RemainingUses; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Item")
	int32 GetMaxUses() const { return Def.MaxUses; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Item")
	float GetStaminaDeltaOnUse() const { return Def.StaminaDeltaOnUse; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Item")
	FName GetItemId() const { return Def.ItemId; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Item")
	FNightmareItemDef GetDef() const { return Def; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Item")
	FNightmareItemDef Def;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Item")
	int32 RemainingUses;
};