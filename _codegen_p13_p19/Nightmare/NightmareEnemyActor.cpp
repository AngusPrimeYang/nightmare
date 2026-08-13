// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareEnemyActor.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NightmareEnemyHealthComponent.h"
#include "NightmareEnemyRoller.h"
#include "NightmareItemEffectApply.h"
#include "NightmareStaminaComponent.h"
#include "UObject/ConstructorHelpers.h"

ANightmareEnemyActor::ANightmareEnemyActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bDespawned = false;
	MoveMode = ENightmareEnemyMoveMode::Chase;
	LocomotionType = ENightmareEnemyLocomotionType::Hover;
	ChaseTargetLocation = FVector::ZeroVector;
	WanderDirection = FVector(1.0f, 0.0f, 0.0f);
	TimeUntilRetarget = 2.0f;
	bHasChaseTarget = false;
	WanderSpeedScale = 0.55f;
	HoverHeight = 160.0f;
	GroundClearance = 4.0f;
	KnockbackHorizontalSpeed = 600.0f;
	KnockbackUpSpeed = 400.0f;
	bUseInjectedGroundZ = false;
	InjectedGroundZ = 0.0f;

	BodyCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyCollision"));
	SetRootComponent(BodyCollision);
	BodyCollision->SetMobility(EComponentMobility::Movable);
	BodyCollision->InitCapsuleSize(40.0f, 72.0f);
	BodyCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyCollision->SetCollisionObjectType(ECC_WorldDynamic);
	BodyCollision->SetCollisionResponseToAllChannels(ECR_Block);
	BodyCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	BodyCollision->SetCanEverAffectNavigation(false);

	GrayboxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrayboxMesh"));
	GrayboxMesh->SetupAttachment(BodyCollision);
	GrayboxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrayboxMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.6f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
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

	EnemyHealth = CreateDefaultSubobject<UNightmareEnemyHealthComponent>(TEXT("EnemyHealth"));
}

void ANightmareEnemyActor::BeginPlay()
{
	Super::BeginPlay();
	if (TouchSphere)
	{
		TouchSphere->OnComponentBeginOverlap.AddDynamic(this, &ANightmareEnemyActor::OnTouchBeginOverlap);
	}
	if (EnemyHealth)
	{
		EnemyHealth->ResetHealth();
	}
	ApplyLocomotionHeight();
	RefreshChaseTargetFromPlayer();
	RefreshVisualFromStats();
	RefreshVisualFromLocomotion();
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
	if (GetWorld())
	{
		Destroy();
	}
}

void ANightmareEnemyActor::ApplyRolledHealth(float MaxHealth)
{
	if (EnemyHealth)
	{
		EnemyHealth->SetMaxHealth(MaxHealth);
	}
}

void ANightmareEnemyActor::RefreshVisualFromStats()
{
	if (!GrayboxMesh)
	{
		return;
	}
	const float Alpha = FMath::GetMappedRangeValueClamped(
		FVector2D(90.0, 520.0),
		FVector2D(0.0, 1.0),
		Stats.MoveSpeed);
	const float XY = FMath::Lerp(1.05f, 0.55f, Alpha);
	const float Z = FMath::Lerp(1.15f, 2.0f, Alpha);
	GrayboxMesh->SetRelativeScale3D(FVector(XY, XY, Z));
}

void ANightmareEnemyActor::RefreshVisualFromLocomotion()
{
	if (!GrayboxMesh || !GetWorld())
	{
		return;
	}
	const TCHAR* Path = (LocomotionType == ENightmareEnemyLocomotionType::Hover)
		? TEXT("/Engine/BasicShapes/Sphere.Sphere")
		: TEXT("/Engine/BasicShapes/Cube.Cube");
	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path))
	{
		GrayboxMesh->SetStaticMesh(Mesh);
	}
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

void ANightmareEnemyActor::SetLocomotionType(ENightmareEnemyLocomotionType InType)
{
	LocomotionType = InType;
	RefreshVisualFromLocomotion();
	ApplyLocomotionHeight();
}

void ANightmareEnemyActor::SetHoverHeight(float Height)
{
	HoverHeight = FMath::Max(0.0f, Height);
	if (LocomotionType == ENightmareEnemyLocomotionType::Hover)
	{
		ApplyLocomotionHeight();
	}
}

void ANightmareEnemyActor::SetInjectedGroundZ(float GroundZ)
{
	bUseInjectedGroundZ = true;
	InjectedGroundZ = GroundZ;
}

float ANightmareEnemyActor::GetDesiredHeightAboveGround() const
{
	if (LocomotionType == ENightmareEnemyLocomotionType::Hover)
	{
		return FMath::Max(0.0f, HoverHeight);
	}

	float HalfHeight = 72.0f;
	if (BodyCollision)
	{
		HalfHeight = BodyCollision->GetUnscaledCapsuleHalfHeight();
	}
	return HalfHeight + FMath::Max(0.0f, GroundClearance);
}

float ANightmareEnemyActor::ComputeDesiredWorldZ(float GroundZ) const
{
	return GroundZ + GetDesiredHeightAboveGround();
}

bool ANightmareEnemyActor::TryResolveGroundZ(float& OutGroundZ) const
{
	if (bUseInjectedGroundZ)
	{
		OutGroundZ = InjectedGroundZ;
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector Loc = GetActorLocation();
	const FVector TraceStart(Loc.X, Loc.Y, Loc.Z + 500.0f);
	const FVector TraceEnd(Loc.X, Loc.Y, Loc.Z - 2000.0f);
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(NightmareEnemyGround), false, this);
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
	{
		OutGroundZ = Hit.ImpactPoint.Z;
		return true;
	}
	return false;
}

void ANightmareEnemyActor::ApplyLocomotionHeight()
{
	float GroundZ = 0.0f;
	if (!TryResolveGroundZ(GroundZ))
	{
		return;
	}

	const FVector Loc = GetActorLocation();
	const float DesiredZ = ComputeDesiredWorldZ(GroundZ);
	if (!FMath::IsNearlyEqual(Loc.Z, DesiredZ, 0.1f))
	{
		SetActorLocation(FVector(Loc.X, Loc.Y, DesiredZ), false, nullptr, ETeleportType::TeleportPhysics);
	}
}

bool ANightmareEnemyActor::TryAttackPlayer(UNightmareStaminaComponent* Stamina, ACharacter* KnockbackCharacter)
{
	if (bDespawned || !Stamina)
	{
		return false;
	}

	Stamina->ApplyDelta(-Stats.AttackPower);

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

bool ANightmareEnemyActor::TryTakeProjectileHit(float DamageAmount)
{
	if (bDespawned || !EnemyHealth || DamageAmount <= 0.0f)
	{
		return false;
	}

	EnemyHealth->ApplyDamage(DamageAmount);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			211,
			1.5f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Enemy shot! -%.0f HP -> %.1f"),
				DamageAmount,
				EnemyHealth->GetCurrentHealth()));
	}

	if (EnemyHealth->IsDepleted())
	{
		Despawn();
	}

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

FVector ANightmareEnemyActor::TickMovement(float DeltaSeconds)
{
	if (bDespawned || DeltaSeconds <= 0.0f || Stats.MoveSpeed <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	ApplyLocomotionHeight();

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

	ApplyLocomotionHeight();
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

	if (GetWorld() && EnemyHealth)
	{
		const TCHAR* LocoLabel = (LocomotionType == ENightmareEnemyLocomotionType::Hover)
			? TEXT("HOVER")
			: TEXT("WALK");
		const TCHAR* ModeLabel = (MoveMode == ENightmareEnemyMoveMode::Chase)
			? TEXT("CHASE")
			: TEXT("WANDER");
		const FString Label = FString::Printf(
			TEXT("%s %s HP%.0f/%.0f"),
			LocoLabel,
			ModeLabel,
			EnemyHealth->GetCurrentHealth(),
			EnemyHealth->GetMaxHealth());
		const FColor Color = (LocomotionType == ENightmareEnemyLocomotionType::Hover)
			? FColor::Cyan
			: FColor::Orange;
		DrawDebugString(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, 120.0f), Label, nullptr, Color, 0.0f, true);
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
