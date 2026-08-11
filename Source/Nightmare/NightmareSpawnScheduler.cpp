// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareSpawnScheduler.h"

UNightmareSpawnScheduler::UNightmareSpawnScheduler()
{
	MinSpawnInterval = 3.0f;
	MaxSpawnInterval = 8.0f;
	TimeUntilNextSpawn = 0.0f;
	RandomSeed = 1;
	RandomStream.Initialize(RandomSeed);
	TimeUntilNextSpawn = RollNextInterval();
}

void UNightmareSpawnScheduler::SetIntervalRange(float InMinSeconds, float InMaxSeconds)
{
	MinSpawnInterval = FMath::Max(0.0f, InMinSeconds);
	MaxSpawnInterval = FMath::Max(MinSpawnInterval, InMaxSeconds);
}

void UNightmareSpawnScheduler::SetRandomSeed(int32 Seed)
{
	RandomSeed = Seed;
	RandomStream.Initialize(RandomSeed);
}

void UNightmareSpawnScheduler::SetTimeUntilNextSpawn(float Seconds)
{
	TimeUntilNextSpawn = FMath::Max(0.0f, Seconds);
}

void UNightmareSpawnScheduler::ResetScheduler()
{
	RandomStream.Initialize(RandomSeed);
	TimeUntilNextSpawn = RollNextInterval();
}

float UNightmareSpawnScheduler::RollNextInterval()
{
	if (FMath::IsNearlyEqual(MinSpawnInterval, MaxSpawnInterval))
	{
		return MinSpawnInterval;
	}
	return RandomStream.FRandRange(MinSpawnInterval, MaxSpawnInterval);
}

int32 UNightmareSpawnScheduler::TickSpawn(float DeltaSeconds)
{
	if (DeltaSeconds <= 0.0f)
	{
		return 0;
	}

	int32 SpawnCount = 0;
	float Remaining = DeltaSeconds;
	while (Remaining > 0.0f)
	{
		if (TimeUntilNextSpawn > Remaining)
		{
			TimeUntilNextSpawn -= Remaining;
			Remaining = 0.0f;
			break;
		}

		Remaining -= TimeUntilNextSpawn;
		TimeUntilNextSpawn = 0.0f;
		++SpawnCount;
		TimeUntilNextSpawn = RollNextInterval();
		if (TimeUntilNextSpawn <= 0.0f)
		{
			// Zero interval would infinite-loop; treat as single fire and stop.
			break;
		}
	}
	return SpawnCount;
}