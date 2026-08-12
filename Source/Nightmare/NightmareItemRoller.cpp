// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareItemRoller.h"

UNightmareItemRoller::UNightmareItemRoller()
{
	TouchModeChance = 0.45f;
	StaminaEffectChance = 0.50f;
	SpeedEffectChance = 0.25f;
	StaminaDeltaMin = -25.0f;
	StaminaDeltaMax = 30.0f;
	SpeedMultiplierMin = 0.4f;
	SpeedMultiplierMax = 1.8f;
	JumpHeightMultiplierMin = 0.05f;
	JumpHeightMultiplierMax = 2.5f;
	GravityScaleMin = 0.2f;
	GravityScaleMax = 2.5f;
	EffectDurationMin = 2.0f;
	EffectDurationMax = 8.0f;
	RandomSeed = 1;
	RandomStream.Initialize(RandomSeed);
}

void UNightmareItemRoller::SetRandomSeed(int32 Seed)
{
	RandomSeed = Seed;
	RandomStream.Initialize(RandomSeed);
}

FNightmareItemDef UNightmareItemRoller::RollItemDef()
{
	return RollItemDefWithStream(RandomStream);
}

FNightmareItemDef UNightmareItemRoller::RollItemDefWithStream(FRandomStream& Stream) const
{
	FNightmareItemDef Def;
	Def.InteractMode = (Stream.FRand() < TouchModeChance)
		? ENightmareItemInteractMode::TouchInstant
		: ENightmareItemInteractMode::HoldToUse;

	const float EffectRoll = Stream.FRand();
	const float StaminaCut = FMath::Clamp(StaminaEffectChance, 0.0f, 1.0f);
	const float SpeedCut = StaminaCut + FMath::Clamp(SpeedEffectChance, 0.0f, 1.0f - StaminaCut);
	if (EffectRoll < StaminaCut)
	{
		Def.EffectType = ENightmareItemEffectType::Stamina;
	}
	else if (EffectRoll < SpeedCut)
	{
		Def.EffectType = ENightmareItemEffectType::Speed;
	}
	else
	{
		Def.EffectType = ENightmareItemEffectType::Jump;
	}

	Def.MaxUses = 1;
	Def.EffectDuration = Stream.FRandRange(EffectDurationMin, EffectDurationMax);
	Def.StaminaDeltaOnUse = 0.0f;
	Def.SpeedMultiplier = 1.0f;
	Def.JumpHeightMultiplier = 1.0f;
	Def.GravityScale = 1.0f;

	switch (Def.EffectType)
	{
	case ENightmareItemEffectType::Stamina:
		Def.StaminaDeltaOnUse = Stream.FRandRange(StaminaDeltaMin, StaminaDeltaMax);
		Def.ItemId = TEXT("StaminaPickup");
		Def.DisplayName = TEXT("Stamina");
		Def.EffectDuration = 0.0f;
		break;
	case ENightmareItemEffectType::Speed:
		Def.SpeedMultiplier = Stream.FRandRange(SpeedMultiplierMin, SpeedMultiplierMax);
		Def.ItemId = TEXT("SpeedPickup");
		Def.DisplayName = TEXT("Speed");
		break;
	case ENightmareItemEffectType::Jump:
		Def.JumpHeightMultiplier = Stream.FRandRange(JumpHeightMultiplierMin, JumpHeightMultiplierMax);
		Def.GravityScale = Stream.FRandRange(GravityScaleMin, GravityScaleMax);
		Def.ItemId = TEXT("JumpPickup");
		Def.DisplayName = TEXT("Jump");
		break;
	}

	return Def;
}
