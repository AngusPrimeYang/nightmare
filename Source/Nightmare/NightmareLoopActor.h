// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightmareLoopActor.generated.h"

/**
 * Practice actor used to verify the loop-engineering apply_codegen pipeline.
 * Safe to replace or delete after the first real gameplay actor lands.
 */
UCLASS()
class NIGHTMARE_API ANightmareLoopActor : public AActor
{
	GENERATED_BODY()

public:
	ANightmareLoopActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** Display label for loop-engineering smoke checks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoopEngineering")
	FString LoopLabel;

	/** Optional scene root so the actor is placeable in the editor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;
};
