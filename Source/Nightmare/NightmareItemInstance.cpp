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

bool UNightmareItemInstance::TryUse(FNightmareItemUseResult& OutResult)
{
	OutResult = FNightmareItemUseResult();
	if (!CanUse())
	{
		return false;
	}

	--RemainingUses;
	OutResult.EffectType = Def.EffectType;
	OutResult.StaminaDelta = Def.StaminaDeltaOnUse;
	OutResult.SpeedMultiplier = Def.SpeedMultiplier;
	OutResult.JumpHeightMultiplier = Def.JumpHeightMultiplier;
	OutResult.GravityScale = Def.GravityScale;
	OutResult.EffectDuration = Def.EffectDuration;
	return true;
}
