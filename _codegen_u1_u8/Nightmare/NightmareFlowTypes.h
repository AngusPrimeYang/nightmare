// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NightmareFlowTypes.generated.h"

UENUM(BlueprintType)
enum class ENightmareFlowPhase : uint8
{
	MainMenu UMETA(DisplayName = "Main Menu"),
	Playing UMETA(DisplayName = "Playing"),
	FailedPopup UMETA(DisplayName = "Failed Popup"),
};
