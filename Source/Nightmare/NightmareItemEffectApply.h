// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NightmareItemTypes.h"

class UNightmareStaminaComponent;
class UNightmarePlayerEffectComponent;
class ACharacter;

/** Shared apply path for inventory use and touch-instant pickups. */
namespace NightmareItemEffectApply
{
	/** Returns false if required component for EffectType is missing. */
	NIGHTMARE_API bool ApplyUseResult(
		const FNightmareItemUseResult& Result,
		UNightmareStaminaComponent* Stamina,
		UNightmarePlayerEffectComponent* Effects);

	/** P3: backward hop when touch stamina damage applied. */
	NIGHTMARE_API void ApplyHitKnockback(ACharacter* Character, const FVector& FromWorldLocation, float HorizontalSpeed, float UpSpeed);
}
