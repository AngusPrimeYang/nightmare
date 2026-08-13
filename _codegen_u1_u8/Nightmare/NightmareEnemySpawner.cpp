// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareEnemySpawner.h"

#include "NightmareEdgeSpawnLocator.h"
#include "NightmareEnemyActor.h"
#include "NightmareEnemyRoller.h"
#include "NightmareSpawnScheduler.h"

ANightmareEnemySpawner::ANightmareEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayAreaHalfExtentX = 1400.0f;
	PlayAreaHalfExtentY = 1400.0f;
	EdgeBandWidth = 350.0f;
	SpawnHeight = 100.0f;
	MinSpawnInterval = 12.0f;
	MaxSpawnInterval = 20.0f;
	MaxAliveEnemies = 4;
	InitialSpawnCount = 1;
	bSpawningEnabled = true;
}

void ANightmareEnemySpawner::EnsureHelpers()
{
	if (!Scheduler)
	{
		Scheduler = NewObject<UNightmareSpawnScheduler>(this, TEXT("EnemySpawnScheduler"));
		Scheduler->SetIntervalRange(MinSpawnInterval, MaxSpawnInterval);
		Scheduler->SetRandomSeed(77);
		Scheduler->ResetScheduler();
	}
	if (!EdgeLocator)
	{
		EdgeLocator = NewObject<UNightmareEdgeSpawnLocator>(this, TEXT("EdgeSpawnLocator"));
		EdgeLocator->SetRandomSeed(77);
	}
	if (!EnemyRoller)
	{
		EnemyRoller = NewObject<UNightmareEnemyRoller>(this, TEXT("EnemyRoller"));
		EnemyRoller->SetRandomSeed(77);
	}
	SyncLocatorFromProperties();
}

void ANightmareEnemySpawner::SyncLocatorFromProperties()
{
	if (!EdgeLocator)
	{
		return;
	}
	EdgeLocator->SetOrigin(GetActorLocation());
	EdgeLocator->SetPlayAreaHalfExtent(PlayAreaHalfExtentX, PlayAreaHalfExtentY);
	EdgeLocator->SetEdgeBandWidth(EdgeBandWidth);
	EdgeLocator->SetSpawnHeight(SpawnHeight);
}

void ANightmareEnemySpawner::SetSpawningEnabled(bool bEnabled)
{
	bSpawningEnabled = bEnabled;
}

void ANightmareEnemySpawner::ResetSpawnerSession()
{
	PruneAliveList();
	for (const TObjectPtr<ANightmareEnemyActor>& Enemy : AliveEnemies)
	{
		if (Enemy && IsValid(Enemy) && !Enemy->IsDespawned())
		{
			Enemy->Destroy();
		}
	}
	AliveEnemies.Empty();
	EnsureHelpers();
	if (Scheduler)
	{
		Scheduler->ResetScheduler();
	}
}

void ANightmareEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	EnsureHelpers();
	if (bSpawningEnabled && InitialSpawnCount > 0)
	{
		SpawnEnemiesAtEdge(InitialSpawnCount);
	}
}

void ANightmareEnemySpawner::PruneAliveList()
{
	for (int32 Index = AliveEnemies.Num() - 1; Index >= 0; --Index)
	{
		ANightmareEnemyActor* Enemy = AliveEnemies[Index];
		if (!Enemy || Enemy->IsDespawned() || !IsValid(Enemy))
		{
			AliveEnemies.RemoveAt(Index);
		}
	}
}

int32 ANightmareEnemySpawner::GetAliveEnemyCount() const
{
	int32 Count = 0;
	for (const TObjectPtr<ANightmareEnemyActor>& Enemy : AliveEnemies)
	{
		if (Enemy && !Enemy->IsDespawned() && IsValid(Enemy))
		{
			++Count;
		}
	}
	return Count;
}

ANightmareEnemyActor* ANightmareEnemySpawner::SpawnOneEnemyAt(const FVector& WorldLocation)
{
	EnsureHelpers();
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANightmareEnemyActor* Enemy = World->SpawnActor<ANightmareEnemyActor>(
		ANightmareEnemyActor::StaticClass(),
		WorldLocation,
		FRotator::ZeroRotator,
		Params);
	if (!Enemy)
	{
		return nullptr;
	}

	const FNightmareEnemyStats Rolled = EnemyRoller->RollStats();
	Enemy->ApplyStats(Rolled);
	Enemy->ApplyRolledHealth(EnemyRoller->RollHealth());
	Enemy->SetLocomotionType(EnemyRoller->RollLocomotionType());
	Enemy->SetBehaviorRoller(EnemyRoller);
	Enemy->RetargetFromRoller(EnemyRoller);
	AliveEnemies.Add(Enemy);
	return Enemy;
}

int32 ANightmareEnemySpawner::SpawnEnemiesAtEdge(int32 Count)
{
	if (!bSpawningEnabled)
	{
		return 0;
	}

	EnsureHelpers();
	PruneAliveList();
	int32 Spawned = 0;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		if (AliveEnemies.Num() >= MaxAliveEnemies)
		{
			break;
		}
		const FVector Loc = EdgeLocator->RollLocation();
		if (SpawnOneEnemyAt(Loc))
		{
			++Spawned;
		}
	}
	return Spawned;
}

void ANightmareEnemySpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bSpawningEnabled)
	{
		return;
	}

	EnsureHelpers();
	const int32 Events = Scheduler->TickSpawn(DeltaSeconds);
	if (Events > 0)
	{
		SpawnEnemiesAtEdge(Events);
	}
}
