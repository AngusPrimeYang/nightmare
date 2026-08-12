// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareItemEffectApply.h"
#include "GameFramework/Character.h"
#include "NightmarePlayerEffectComponent.h"
#include "NightmareStaminaComponent.h"

bool NightmareItemEffectApply::ApplyUseResult(
	const FNightmareItemUseResult& Result,
	UNightmareStaminaComponent* Stamina,
	UNightmarePlayerEffectComponent* Effects)
{
	switch (Result.EffectType)
	{
	case ENightmareItemEffectType::Stamina:
		if (!Stamina)
		{
			return false;
		}
		Stamina->ApplyDelta(Result.StaminaDelta);
		return true;
	case ENightmareItemEffectType::Speed:
		if (!Effects)
		{
			return false;
		}
		Effects->ApplySpeedEffect(Result.SpeedMultiplier, Result.EffectDuration);
		return true;
	case ENightmareItemEffectType::Jump:
		if (!Effects)
		{
			return false;
		}
		Effects->ApplyJumpEffect(Result.JumpHeightMultiplier, Result.GravityScale, Result.EffectDuration);
		return true;
	}
	return false;
}

void NightmareItemEffectApply::ApplyHitKnockback(
	ACharacter* Character,
	const FVector& FromWorldLocation,
	float HorizontalSpeed,
	float UpSpeed)
{
	if (!Character)
	{
		return;
	}

	FVector Away = Character->GetActorLocation() - FromWorldLocation;
	Away.Z = 0.0f;
	if (!Away.Normalize())
	{
		Away = -Character->GetActorForwardVector();
		Away.Z = 0.0f;
		Away.Normalize();
	}
	const FVector Launch = Away * HorizontalSpeed + FVector(0.0f, 0.0f, UpSpeed);
	Character->LaunchCharacter(Launch, true, true);
}
