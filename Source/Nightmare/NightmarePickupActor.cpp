// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmarePickupActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NightmareInventoryComponent.h"
#include "NightmareItemInstance.h"
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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		GrayboxMesh->SetStaticMesh(CubeMesh.Object);
	}

	bCollected = false;
	YawRotateSpeedDegrees = 45.0f;
	ItemDef.ItemId = TEXT("Pickup");
	ItemDef.DisplayName = TEXT("Pickup");
	ItemDef.MaxUses = 1;
	ItemDef.StaminaDeltaOnUse = 10.0f;
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

bool ANightmarePickupActor::TryCollectInto(UNightmareInventoryComponent* Inventory)
{
	if (bCollected || !Inventory)
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

	bCollected = true;
	if (GrayboxMesh)
	{
		GrayboxMesh->SetVisibility(false);
		GrayboxMesh->SetHiddenInGame(true);
	}
	return true;
}
