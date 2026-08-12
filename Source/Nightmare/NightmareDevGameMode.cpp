// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareDevGameMode.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "NightmareDevCharacter.h"
#include "NightmareEnemySpawner.h"
#include "NightmareItemSpawner.h"

ANightmareDevGameMode::ANightmareDevGameMode()
{
	DefaultPawnClass = ANightmareDevCharacter::StaticClass();
	ItemSpawnerLocation = FVector(0.0f, -600.0f, 150.0f);
	EnemySpawnerLocation = FVector(0.0f, -100.0f, 100.0f);
	SafeSpawnLocation = FVector(0.0f, -600.0f, 220.0f);
	bForceSafeSpawnLocation = true;
	SpawnTraceHeight = 8000.0f;
	SpawnSurfacePadding = 92.0f;
}

void ANightmareDevGameMode::ApplySafeSpawnTransforms()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), Starts);
	for (AActor* Actor : Starts)
	{
		APlayerStart* Start = Cast<APlayerStart>(Actor);
		if (!Start)
		{
			continue;
		}

		FVector Loc = Start->GetActorLocation();
		if (bForceSafeSpawnLocation || Loc.Z < SafeSpawnLocation.Z)
		{
			Loc.X = SafeSpawnLocation.X;
			Loc.Y = SafeSpawnLocation.Y;
			Loc.Z = SafeSpawnLocation.Z;
			// PlayerStart capsules are often Static; moving them without Movable spams PIE warnings.
			if (USceneComponent* Root = Start->GetRootComponent())
			{
				Root->SetMobility(EComponentMobility::Movable);
			}
			Start->SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);
			Start->SetActorRotation(FRotator(0.0f, 90.0f, 0.0f));
		}
	}
}

void ANightmareDevGameMode::EnsurePawnSafeSpawn(APawn* Pawn) const
{
	if (!Pawn || !GetWorld())
	{
		return;
	}

	FVector Target = SafeSpawnLocation;
	const FVector TraceStart(SafeSpawnLocation.X, SafeSpawnLocation.Y, SafeSpawnLocation.Z + SpawnTraceHeight);
	const FVector TraceEnd(SafeSpawnLocation.X, SafeSpawnLocation.Y, SafeSpawnLocation.Z - SpawnTraceHeight);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(NightmareSafeSpawn), false, Pawn);
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
	{
		Target = Hit.ImpactPoint + FVector(0.0f, 0.0f, SpawnSurfacePadding);
	}
	else if (bForceSafeSpawnLocation || Pawn->GetActorLocation().Z < SafeSpawnLocation.Z)
	{
		Target = SafeSpawnLocation;
	}
	else
	{
		return;
	}

	Pawn->SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);
	if (AController* Controller = Pawn->GetController())
	{
		Controller->SetControlRotation(FRotator(0.0f, 90.0f, 0.0f));
	}
}

void ANightmareDevGameMode::BeginPlay()
{
	Super::BeginPlay();
	ApplySafeSpawnTransforms();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ItemSpawner = World->SpawnActor<ANightmareItemSpawner>(
		ANightmareItemSpawner::StaticClass(),
		ItemSpawnerLocation,
		FRotator::ZeroRotator,
		Params);
	EnemySpawner = World->SpawnActor<ANightmareEnemySpawner>(
		ANightmareEnemySpawner::StaticClass(),
		EnemySpawnerLocation,
		FRotator::ZeroRotator,
		Params);
}

void ANightmareDevGameMode::RestartPlayer(AController* NewPlayer)
{
	ApplySafeSpawnTransforms();
	Super::RestartPlayer(NewPlayer);
	if (NewPlayer)
	{
		EnsurePawnSafeSpawn(NewPlayer->GetPawn());
	}
}
