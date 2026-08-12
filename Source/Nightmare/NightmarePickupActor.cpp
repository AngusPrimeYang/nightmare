// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmarePickupActor.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "NightmareInventoryComponent.h"
#include "NightmareItemEffectApply.h"
#include "NightmareItemInstance.h"
#include "NightmarePlayerEffectComponent.h"
#include "NightmareStaminaComponent.h"
#include "UObject/ConstructorHelpers.h"

ANightmarePickupActor::ANightmarePickupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	GrayboxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrayboxMesh"));
	GrayboxMesh->SetupAttachment(SceneRoot);
	GrayboxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrayboxMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));

	TouchSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TouchSphere"));
	TouchSphere->SetupAttachment(SceneRoot);
	TouchSphere->InitSphereRadius(80.0f);
	TouchSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TouchSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TouchSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TouchSphere->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		GrayboxMesh->SetStaticMesh(CubeMesh.Object);
	}

	bCollected = false;
	YawRotateSpeedDegrees = 45.0f;
	KnockbackHorizontalSpeed = 600.0f;
	KnockbackUpSpeed = 400.0f;
	ItemDef.ItemId = TEXT("Pickup");
	ItemDef.DisplayName = TEXT("Pickup");
	ItemDef.MaxUses = 1;
	ItemDef.EffectType = ENightmareItemEffectType::Stamina;
	ItemDef.InteractMode = ENightmareItemInteractMode::HoldToUse;
	ItemDef.StaminaDeltaOnUse = 10.0f;
}

void ANightmarePickupActor::BeginPlay()
{
	Super::BeginPlay();
	if (TouchSphere)
	{
		TouchSphere->OnComponentBeginOverlap.AddDynamic(this, &ANightmarePickupActor::OnTouchBeginOverlap);
	}
}

void ANightmarePickupActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bCollected && YawRotateSpeedDegrees > 0.0f && GrayboxMesh)
	{
		GrayboxMesh->AddLocalRotation(FRotator(0.0f, YawRotateSpeedDegrees * DeltaSeconds, 0.0f));
	}
}

void ANightmarePickupActor::SetItemDef(const FNightmareItemDef& InDef)
{
	ItemDef = InDef;
}

void ANightmarePickupActor::MarkCollectedAndHide()
{
	bCollected = true;
	if (GrayboxMesh)
	{
		GrayboxMesh->SetVisibility(false);
		GrayboxMesh->SetHiddenInGame(true);
	}
	if (TouchSphere)
	{
		TouchSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

bool ANightmarePickupActor::TryCollectInto(UNightmareInventoryComponent* Inventory)
{
	if (bCollected || !Inventory)
	{
		return false;
	}
	if (ItemDef.InteractMode != ENightmareItemInteractMode::HoldToUse)
	{
		return false;
	}

	UNightmareItemInstance* Instance = NewObject<UNightmareItemInstance>(Inventory);
	Instance->InitializeFromDef(ItemDef);
	const int32 Slot = Inventory->TryAddItem(Instance);
	if (Slot == INDEX_NONE)
	{
		return false;
	}

	MarkCollectedAndHide();
	return true;
}

bool ANightmarePickupActor::TryTouchApply(
	UNightmareStaminaComponent* Stamina,
	UNightmarePlayerEffectComponent* Effects,
	ACharacter* KnockbackCharacter)
{
	if (bCollected)
	{
		return false;
	}
	if (ItemDef.InteractMode != ENightmareItemInteractMode::TouchInstant)
	{
		return false;
	}

	FNightmareItemUseResult Result;
	Result.EffectType = ItemDef.EffectType;
	Result.StaminaDelta = ItemDef.StaminaDeltaOnUse;
	Result.SpeedMultiplier = ItemDef.SpeedMultiplier;
	Result.JumpHeightMultiplier = ItemDef.JumpHeightMultiplier;
	Result.GravityScale = ItemDef.GravityScale;
	Result.EffectDuration = ItemDef.EffectDuration;

	if (!NightmareItemEffectApply::ApplyUseResult(Result, Stamina, Effects))
	{
		return false;
	}

	if (ItemDef.EffectType == ENightmareItemEffectType::Stamina && ItemDef.StaminaDeltaOnUse < 0.0f && KnockbackCharacter)
	{
		NightmareItemEffectApply::ApplyHitKnockback(
			KnockbackCharacter,
			GetActorLocation(),
			KnockbackHorizontalSpeed,
			KnockbackUpSpeed);
	}

	MarkCollectedAndHide();
	return true;
}

void ANightmarePickupActor::OnTouchBeginOverlap(
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

	if (bCollected || !OtherActor || ItemDef.InteractMode != ENightmareItemInteractMode::TouchInstant)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	UNightmareStaminaComponent* Stamina = Character->FindComponentByClass<UNightmareStaminaComponent>();
	UNightmarePlayerEffectComponent* Effects = Character->FindComponentByClass<UNightmarePlayerEffectComponent>();
	TryTouchApply(Stamina, Effects, Character);
}
