// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NightmareDevGameMode.generated.h"

/** Dev smoke GameMode: default pawn is ANightmareDevCharacter. */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmareDevGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANightmareDevGameMode();
};
