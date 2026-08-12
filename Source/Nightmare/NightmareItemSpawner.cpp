// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareItemSpawner.h"

#include "NightmareItemRoller.h"
#include "NightmarePickupActor.h"
#include "NightmareSpawnScheduler.h"

ANightmareItemSpawner::ANightmareItemSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnRadius = 1200.0f;
	HoverHeight = 120.0f;
	MaxAlivePickups = 12;
}

void ANightmareItemSpawner::EnsureHelpers()
{
	if (!Scheduler)
	{
		Scheduler = NewObject<UNightmareSpawnScheduler>(this, TEXT("SpawnScheduler"));
		Scheduler->SetIntervalRange(3.0f, 8.0f);
		Scheduler->SetRandomSeed(42);
		Scheduler->ResetScheduler();
	}
	if (!Roller)
	{
		Roller = NewObject<UNightmareItemRoller>(this, TEXT("ItemRoller"));
		Roller->SetRandomSeed(42);
	}
}

void ANightmareItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	EnsureHelpers();
}

void ANightmareItemSpawner::PruneAliveList()
{
	for (int32 Index = AlivePickups.Num() - 1; Index >= 0; --Index)
	{
		ANightmarePickupActor* Pickup = AlivePickups[Index];
		if (!Pickup || Pickup->IsCollected() || !IsValid(Pickup))
		{
			AlivePickups.RemoveAt(Index);
		}
	}
}

FVector ANightmareItemSpawner::RollSpawnLocation() const
{
	const FVector Origin = GetActorLocation();
	const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
	const float Dist = FMath::FRandRange(SpawnRadius * 0.25f, SpawnRadius);
	return Origin + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, HoverHeight);
}

ANightmarePickupActor* ANightmareItemSpawner::SpawnOnePickupAt(const FVector& WorldLocation, const FNightmareItemDef& Def)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANightmarePickupActor* Pickup = World->SpawnActor<ANightmarePickupActor>(
		ANightmarePickupActor::StaticClass(),
		WorldLocation,
		FRotator::ZeroRotator,
		Params);
	if (!Pickup)
	{
		return nullptr;
	}
	Pickup->SetItemDef(Def);
	AlivePickups.Add(Pickup);
	return Pickup;
}

int32 ANightmareItemSpawner::SpawnRolledPickups(int32 Count)
{
	EnsureHelpers();
	PruneAliveList();
	int32 Spawned = 0;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		if (AlivePickups.Num() >= MaxAlivePickups)
		{
			break;
		}
		const FNightmareItemDef Def = Roller->RollItemDef();
		if (SpawnOnePickupAt(RollSpawnLocation(), Def))
		{
			++Spawned;
		}
	}
	return Spawned;
}

void ANightmareItemSpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EnsureHelpers();
	const int32 Events = Scheduler->TickSpawn(DeltaSeconds);
	if (Events > 0)
	{
		SpawnRolledPickups(Events);
	}
}
