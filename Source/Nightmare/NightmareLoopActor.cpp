// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareLoopActor.h"

ANightmareLoopActor::ANightmareLoopActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	LoopLabel = TEXT("NightmareLoopActor_E5");
}

void ANightmareLoopActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[LoopEngineering] %s BeginPlay at %s"),
		*LoopLabel,
		*GetActorLocation().ToCompactString());
}

void ANightmareLoopActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}