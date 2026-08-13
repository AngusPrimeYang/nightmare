// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareProjectileActor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "NightmareEnemyActor.h"
#include "UObject/ConstructorHelpers.h"

ANightmareProjectileActor::ANightmareProjectileActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bConsumed = false;
	FlightDirection = FVector(1.0f, 0.0f, 0.0f);
	Speed = 6000.0f;
	Damage = 5.0f;
	Trajectory = ENightmareProjectileTrajectory::StraightLine;
	HalfExtents = FVector(40.0f, 4.0f, 4.0f);
	MaxLifetime = 3.0f;
	Age = 0.0f;

	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	SetRootComponent(HitBox);
	HitBox->SetMobility(EComponentMobility::Movable);
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitBox->SetCollisionObjectType(ECC_WorldDynamic);
	HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	HitBox->SetGenerateOverlapEvents(true);

	GrayboxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrayboxMesh"));
	GrayboxMesh->SetupAttachment(HitBox);
	GrayboxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrayboxMesh->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		GrayboxMesh->SetStaticMesh(CubeMesh.Object);
	}

	ApplyVisualFromHalfExtents();
}

float ANightmareProjectileActor::ComputeDefaultSpeed(float WalkSpeed)
{
	return FMath::Max(0.0f, WalkSpeed) * 10.0f;
}

float ANightmareProjectileActor::ComputeDefaultDamage(float PlayerMaxStamina)
{
	return FMath::Max(0.0f, PlayerMaxStamina) / 20.0f;
}

void ANightmareProjectileActor::ApplyVisualFromHalfExtents()
{
	if (HitBox)
	{
		HitBox->SetBoxExtent(HalfExtents, true);
	}
	if (GrayboxMesh)
	{
		constexpr float DefaultCubeExtent = 50.0f;
		GrayboxMesh->SetRelativeScale3D(FVector(
			(HalfExtents.X * 2.0f) / DefaultCubeExtent,
			(HalfExtents.Y * 2.0f) / DefaultCubeExtent,
			(HalfExtents.Z * 2.0f) / DefaultCubeExtent));
	}
}

void ANightmareProjectileActor::ConfigureProjectile(
	const FVector& InDirection,
	float InSpeed,
	float InDamage,
	ENightmareProjectileTrajectory InTrajectory,
	const FVector& InHalfExtents)
{
	FVector Dir = InDirection;
	if (!Dir.Normalize())
	{
		Dir = FVector(1.0f, 0.0f, 0.0f);
	}
	FlightDirection = Dir;
	Speed = FMath::Max(0.0f, InSpeed);
	Damage = FMath::Max(0.0f, InDamage);
	Trajectory = InTrajectory;
	HalfExtents = InHalfExtents.ComponentMax(FVector::ZeroVector);
	ApplyVisualFromHalfExtents();
	SetActorRotation(Dir.Rotation());
}

void ANightmareProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	if (HitBox)
	{
		HitBox->OnComponentBeginOverlap.AddDynamic(this, &ANightmareProjectileActor::OnHitBeginOverlap);
	}
}

void ANightmareProjectileActor::TryResolveSweepHit(const FHitResult& Hit)
{
	if (bConsumed || !Hit.bBlockingHit)
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();
	if (HitActor == GetOwner() || HitActor == GetInstigator())
	{
		return;
	}

	if (ANightmareEnemyActor* Enemy = Cast<ANightmareEnemyActor>(HitActor))
	{
		TryApplyHitToEnemy(Enemy);
		return;
	}

	// Walls / pad — stop the projectile so high speed does not tunnel through geometry.
	Consume();
}

FVector ANightmareProjectileActor::TickFlight(float DeltaSeconds)
{
	if (bConsumed || DeltaSeconds <= 0.0f || Speed <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	FVector Delta = FVector::ZeroVector;
	switch (Trajectory)
	{
	case ENightmareProjectileTrajectory::StraightLine:
		Delta = FlightDirection * (Speed * DeltaSeconds);
		break;
	default:
		Delta = FlightDirection * (Speed * DeltaSeconds);
		break;
	}

	const FVector Before = GetActorLocation();

	// Specs use NewObject without a World — keep teleport so TickFlight math stays deterministic.
	if (GetWorld())
	{
		FHitResult Hit;
		AddActorWorldOffset(Delta, true, &Hit);
		if (Hit.bBlockingHit)
		{
			TryResolveSweepHit(Hit);
		}

#if ENABLE_DRAW_DEBUG
		DrawDebugLine(
			GetWorld(),
			Before,
			GetActorLocation(),
			FColor::Yellow,
			false,
			0.35f,
			0,
			3.0f);
#endif
	}
	else
	{
		SetActorLocation(Before + Delta, false, nullptr, ETeleportType::TeleportPhysics);
	}

	return GetActorLocation() - Before;
}

bool ANightmareProjectileActor::TryApplyHitToEnemy(ANightmareEnemyActor* Enemy)
{
	if (bConsumed || !Enemy || Enemy->IsDespawned())
	{
		return false;
	}

	if (!Enemy->TryTakeProjectileHit(Damage))
	{
		return false;
	}

	Consume();
	return true;
}

void ANightmareProjectileActor::Consume()
{
	if (bConsumed)
	{
		return;
	}
	bConsumed = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (HitBox)
	{
		HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (GetWorld())
	{
		Destroy();
	}
}

void ANightmareProjectileActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bConsumed)
	{
		return;
	}

	Age += DeltaSeconds;
	if (Age >= MaxLifetime)
	{
		Consume();
		return;
	}

	TickFlight(DeltaSeconds);
}

void ANightmareProjectileActor::OnHitBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	if (bConsumed || !OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	if (ANightmareEnemyActor* Enemy = Cast<ANightmareEnemyActor>(OtherActor))
	{
		TryApplyHitToEnemy(Enemy);
	}
}
