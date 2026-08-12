// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareEnemyActor.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NightmareEnemyRoller.h"
#include "NightmareItemEffectApply.h"
#include "NightmareStaminaComponent.h"
#include "UObject/ConstructorHelpers.h"

ANightmareEnemyActor::ANightmareEnemyActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bDespawned = false;
	MoveMode = ENightmareEnemyMoveMode::Chase;
	ChaseTargetLocation = FVector::ZeroVector;
	WanderDirection = FVector(1.0f, 0.0f, 0.0f);
	TimeUntilRetarget = 2.0f;
	bHasChaseTarget = false;
	WanderSpeedScale = 0.55f;
	KnockbackHorizontalSpeed = 600.0f;
	KnockbackUpSpeed = 400.0f;

	BodyCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyCollision"));
	SetRootComponent(BodyCollision);
	BodyCollision->SetMobility(EComponentMobility::Movable);
	BodyCollision->InitCapsuleSize(40.0f, 72.0f);
	BodyCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyCollision->SetCollisionObjectType(ECC_WorldDynamic);
	BodyCollision->SetCollisionResponseToAllChannels(ECR_Block);
	// Attack uses TouchSphere overlap; body should not physically shove the pawn.
	BodyCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	BodyCollision->SetCanEverAffectNavigation(false);

	GrayboxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrayboxMesh"));
	GrayboxMesh->SetupAttachment(BodyCollision);
	GrayboxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrayboxMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.6f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		GrayboxMesh->SetStaticMesh(CubeMesh.Object);
	}

	TouchSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TouchSphere"));
	TouchSphere->SetupAttachment(BodyCollision);
	TouchSphere->InitSphereRadius(60.0f);
	TouchSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TouchSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TouchSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TouchSphere->SetGenerateOverlapEvents(true);
}

void ANightmareEnemyActor::BeginPlay()
{
	Super::BeginPlay();
	if (TouchSphere)
	{
		TouchSphere->OnComponentBeginOverlap.AddDynamic(this, &ANightmareEnemyActor::OnTouchBeginOverlap);
	}
	EnsureClearOfGround();
	RefreshChaseTargetFromPlayer();
	RefreshVisualFromStats();
}

void ANightmareEnemyActor::Despawn()
{
	if (bDespawned)
	{
		return;
	}
	bDespawned = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (TouchSphere)
	{
		TouchSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (BodyCollision)
	{
		BodyCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	// Specs use NewObject without a World; only Destroy when registered in one.
	if (GetWorld())
	{
		Destroy();
	}
}

void ANightmareEnemyActor::RefreshVisualFromStats()
{
	if (!GrayboxMesh)
	{
		return;
	}
	// Slow = stocky / short; fast = thin / tall — readable without UI.
	const float Alpha = FMath::GetMappedRangeValueClamped(
		FVector2D(90.0, 520.0),
		FVector2D(0.0, 1.0),
		Stats.MoveSpeed);
	const float XY = FMath::Lerp(1.05f, 0.55f, Alpha);
	const float Z = FMath::Lerp(1.15f, 2.0f, Alpha);
	GrayboxMesh->SetRelativeScale3D(FVector(XY, XY, Z));
}

void ANightmareEnemyActor::ApplyStats(const FNightmareEnemyStats& InStats)
{
	Stats.MoveSpeed = FMath::Max(0.0f, InStats.MoveSpeed);
	Stats.AttackPower = FMath::Max(0.0f, InStats.AttackPower);
	RefreshVisualFromStats();
}

float ANightmareEnemyActor::GetEffectiveMoveSpeed() const
{
	const float Scale = (MoveMode == ENightmareEnemyMoveMode::Wander) ? WanderSpeedScale : 1.0f;
	return Stats.MoveSpeed * Scale;
}

bool ANightmareEnemyActor::TryAttackPlayer(UNightmareStaminaComponent* Stamina, ACharacter* KnockbackCharacter)
{
	if (bDespawned || !Stamina)
	{
		return false;
	}

	Stamina->ApplyDelta(-Stats.AttackPower);

	// P7: reuse P3 hit feedback entry (no second knockback path).
	if (KnockbackCharacter && Stats.AttackPower > 0.0f)
	{
		NightmareItemEffectApply::ApplyHitKnockback(
			KnockbackCharacter,
			GetActorLocation(),
			KnockbackHorizontalSpeed,
			KnockbackUpSpeed);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			210,
			2.0f,
			FColor::Red,
			FString::Printf(TEXT("Enemy hit! stamina -%.0f -> %.1f"), Stats.AttackPower, Stamina->GetCurrentStamina()));
	}

	Despawn();
	return true;
}

void ANightmareEnemyActor::SetBehaviorRoller(UNightmareEnemyRoller* InRoller)
{
	BehaviorRoller = InRoller;
}

void ANightmareEnemyActor::SetMoveMode(ENightmareEnemyMoveMode InMode)
{
	MoveMode = InMode;
}

void ANightmareEnemyActor::SetChaseTargetLocation(const FVector& WorldLocation)
{
	ChaseTargetLocation = WorldLocation;
	bHasChaseTarget = true;
}

void ANightmareEnemyActor::SetWanderDirection(const FVector& WorldDirectionXY)
{
	FVector Dir = WorldDirectionXY;
	Dir.Z = 0.0f;
	if (!Dir.Normalize())
	{
		Dir = FVector(1.0f, 0.0f, 0.0f);
	}
	WanderDirection = Dir;
}

void ANightmareEnemyActor::SetWanderSpeedScale(float Scale)
{
	WanderSpeedScale = FMath::Clamp(Scale, 0.1f, 1.0f);
}

void ANightmareEnemyActor::SetTimeUntilRetarget(float Seconds)
{
	TimeUntilRetarget = FMath::Max(0.0f, Seconds);
}

void ANightmareEnemyActor::RetargetFromRoller(UNightmareEnemyRoller* Roller)
{
	if (!Roller)
	{
		return;
	}
	MoveMode = Roller->RollMoveMode();
	TimeUntilRetarget = Roller->RollRetargetInterval();
	if (MoveMode == ENightmareEnemyMoveMode::Wander)
	{
		SetWanderDirection(Roller->RollWanderDirection());
	}
}

void ANightmareEnemyActor::RefreshChaseTargetFromPlayer()
{
	if (!GetWorld())
	{
		return;
	}
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		SetChaseTargetLocation(PlayerPawn->GetActorLocation());
	}
}

void ANightmareEnemyActor::EnsureClearOfGround()
{
	UWorld* World = GetWorld();
	if (!World || !BodyCollision)
	{
		return;
	}

	const FVector Loc = GetActorLocation();
	const float HalfHeight = BodyCollision->GetScaledCapsuleHalfHeight();
	const FVector TraceStart(Loc.X, Loc.Y, Loc.Z + 500.0f);
	const FVector TraceEnd(Loc.X, Loc.Y, Loc.Z - 2000.0f);
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(NightmareEnemyGround), false, this);
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
	{
		const FVector Safe(Loc.X, Loc.Y, Hit.ImpactPoint.Z + HalfHeight + 4.0f);
		SetActorLocation(Safe, false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		// Fallback: lift a bit so we are not start-penetrating the pad.
		SetActorLocation(Loc + FVector(0.0f, 0.0f, 8.0f), false, nullptr, ETeleportType::TeleportPhysics);
	}
}

FVector ANightmareEnemyActor::TickMovement(float DeltaSeconds)
{
	if (bDespawned || DeltaSeconds <= 0.0f || Stats.MoveSpeed <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	FVector Direction = FVector::ZeroVector;
	if (MoveMode == ENightmareEnemyMoveMode::Chase && bHasChaseTarget)
	{
		FVector ToTarget = ChaseTargetLocation - GetActorLocation();
		ToTarget.Z = 0.0f;
		if (!ToTarget.Normalize())
		{
			return FVector::ZeroVector;
		}
		Direction = ToTarget;
	}
	else
	{
		Direction = WanderDirection;
	}

	const FVector Delta = Direction * (GetEffectiveMoveSpeed() * DeltaSeconds);
	const FVector Before = GetActorLocation();

	FHitResult Hit;
	AddActorWorldOffset(Delta, true, &Hit);
	if (Hit.bStartPenetrating)
	{
		const float Push = Hit.PenetrationDepth + 4.0f;
		AddActorWorldOffset(Hit.Normal * Push, false);
		AddActorWorldOffset(Delta, true, &Hit);
	}
	else if (Hit.bBlockingHit)
	{
		const FVector Slide = FVector::VectorPlaneProject(Delta * (1.0f - Hit.Time), Hit.ImpactNormal);
		if (!Slide.IsNearlyZero())
		{
			AddActorWorldOffset(Slide, true);
		}
	}

	return GetActorLocation() - Before;
}

void ANightmareEnemyActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDespawned)
	{
		return;
	}

	TimeUntilRetarget -= DeltaSeconds;
	if (TimeUntilRetarget <= 0.0f)
	{
		RefreshChaseTargetFromPlayer();
		if (BehaviorRoller)
		{
			RetargetFromRoller(BehaviorRoller);
		}
		else
		{
			MoveMode = ENightmareEnemyMoveMode::Chase;
			TimeUntilRetarget = 2.0f;
		}
	}
	else if (MoveMode == ENightmareEnemyMoveMode::Chase)
	{
		RefreshChaseTargetFromPlayer();
	}

	TickMovement(DeltaSeconds);

	if (GetWorld())
	{
		const FString Label = FString::Printf(
			TEXT("%s %.0f"),
			MoveMode == ENightmareEnemyMoveMode::Chase ? TEXT("CHASE") : TEXT("WANDER"),
			GetEffectiveMoveSpeed());
		DrawDebugString(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, 120.0f), Label, nullptr, FColor::Orange, 0.0f, true);
	}
}

void ANightmareEnemyActor::OnTouchBeginOverlap(
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

	if (bDespawned || !OtherActor)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	UNightmareStaminaComponent* Stamina = Character->FindComponentByClass<UNightmareStaminaComponent>();
	TryAttackPlayer(Stamina, Character);
}
