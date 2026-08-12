// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareEnemyRoller.h"

UNightmareEnemyRoller::UNightmareEnemyRoller()
{
	// Wide speed band so PIE can tell individuals apart.
	MoveSpeedMin = 90.0f;
	MoveSpeedMax = 520.0f;
	AttackPowerMin = 5.0f;
	AttackPowerMax = 20.0f;
	// More Wander so motion isn't "always chase at similar feel".
	ChaseModeChance = 0.50f;
	RetargetIntervalMin = 2.5f;
	RetargetIntervalMax = 6.0f;
	RandomSeed = 1;
	RandomStream.Initialize(RandomSeed);
}

void UNightmareEnemyRoller::SetRandomSeed(int32 Seed)
{
	RandomSeed = Seed;
	RandomStream.Initialize(RandomSeed);
}

FNightmareEnemyStats UNightmareEnemyRoller::RollStats()
{
	return RollStatsWithStream(RandomStream);
}

FNightmareEnemyStats UNightmareEnemyRoller::RollStatsWithStream(FRandomStream& Stream) const
{
	const float SpeedLo = FMath::Min(MoveSpeedMin, MoveSpeedMax);
	const float SpeedHi = FMath::Max(MoveSpeedMin, MoveSpeedMax);
	const float PowerLo = FMath::Min(AttackPowerMin, AttackPowerMax);
	const float PowerHi = FMath::Max(AttackPowerMin, AttackPowerMax);

	FNightmareEnemyStats Stats;
	Stats.MoveSpeed = Stream.FRandRange(SpeedLo, SpeedHi);
	Stats.AttackPower = Stream.FRandRange(PowerLo, PowerHi);
	return Stats;
}

ENightmareEnemyMoveMode UNightmareEnemyRoller::RollMoveMode()
{
	return RollMoveModeWithStream(RandomStream);
}

ENightmareEnemyMoveMode UNightmareEnemyRoller::RollMoveModeWithStream(FRandomStream& Stream) const
{
	const float Chance = FMath::Clamp(ChaseModeChance, 0.0f, 1.0f);
	return (Stream.FRand() < Chance)
		? ENightmareEnemyMoveMode::Chase
		: ENightmareEnemyMoveMode::Wander;
}

float UNightmareEnemyRoller::RollRetargetInterval()
{
	return RollRetargetIntervalWithStream(RandomStream);
}

float UNightmareEnemyRoller::RollRetargetIntervalWithStream(FRandomStream& Stream) const
{
	const float Lo = FMath::Min(RetargetIntervalMin, RetargetIntervalMax);
	const float Hi = FMath::Max(RetargetIntervalMin, RetargetIntervalMax);
	if (FMath::IsNearlyEqual(Lo, Hi))
	{
		return Lo;
	}
	return Stream.FRandRange(Lo, Hi);
}

FVector UNightmareEnemyRoller::RollWanderDirection()
{
	return RollWanderDirectionWithStream(RandomStream);
}

FVector UNightmareEnemyRoller::RollWanderDirectionWithStream(FRandomStream& Stream) const
{
	const float Angle = Stream.FRandRange(0.0f, 2.0f * PI);
	return FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
}
