// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareItemInstance.h"

void UNightmareItemInstance::InitializeFromDef(const FNightmareItemDef& InDef)
{
	Def = InDef;
	Def.MaxUses = FMath::Max(1, Def.MaxUses);
	RemainingUses = Def.MaxUses;
}

bool UNightmareItemInstance::CanUse() const
{
	return RemainingUses > 0;
}

bool UNightmareItemInstance::TryUse(float& OutStaminaDelta)
{
	OutStaminaDelta = 0.0f;
	if (!CanUse())
	{
		return false;
	}

	--RemainingUses;
	OutStaminaDelta = Def.StaminaDeltaOnUse;
	return true;
}