// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NightmareProjectileTypes.generated.h"

/** P15: v1 locks StraightLine (no gravity / homing). */
UENUM(BlueprintType)
enum class ENightmareProjectileTrajectory : uint8
{
	StraightLine UMETA(DisplayName = "Straight Line")
};
