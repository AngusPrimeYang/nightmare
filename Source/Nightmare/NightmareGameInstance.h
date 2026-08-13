// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NightmareGameInstance.generated.h"

/** Persists across level loads; exposes quit helper for menu flow. */
UCLASS()
class NIGHTMARE_API UNightmareGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void QuitNightmare();
};
