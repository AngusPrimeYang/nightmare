// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmarePlayerEffectComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UNightmarePlayerEffectComponent::UNightmarePlayerEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	BaselineMaxWalkSpeed = 600.0f;
	BaselineJumpZVelocity = 700.0f;
	BaselineGravityScale = 1.0f;
	SpeedMultiplier = 1.0f;
	SpeedTimeRemaining = 0.0f;
	JumpHeightMultiplier = 1.0f;
	ActiveGravityScale = 1.0f;
	JumpTimeRemaining = 0.0f;
	bBaselineCaptured = false;
}

void UNightmarePlayerEffectComponent::CaptureBaselineFromMovement(UCharacterMovementComponent* Movement)
{
	if (!Movement)
	{
		return;
	}
	BaselineMaxWalkSpeed = Movement->MaxWalkSpeed;
	BaselineJumpZVelocity = Movement->JumpZVelocity;
	BaselineGravityScale = Movement->GravityScale;
	bBaselineCaptured = true;
	ApplyToMovement(Movement);
}

void UNightmarePlayerEffectComponent::ApplySpeedEffect(float Multiplier, float DurationSeconds)
{
	SpeedMultiplier = FMath::Max(0.0f, Multiplier);
	SpeedTimeRemaining = FMath::Max(0.0f, DurationSeconds);
}

void UNightmarePlayerEffectComponent::ApplyJumpEffect(float InJumpHeightMultiplier, float InGravityScale, float DurationSeconds)
{
	JumpHeightMultiplier = FMath::Max(0.0f, InJumpHeightMultiplier);
	ActiveGravityScale = FMath::Max(0.0f, InGravityScale);
	JumpTimeRemaining = FMath::Max(0.0f, DurationSeconds);
}

void UNightmarePlayerEffectComponent::ClearAllEffects(UCharacterMovementComponent* Movement)
{
	SpeedMultiplier = 1.0f;
	SpeedTimeRemaining = 0.0f;
	JumpHeightMultiplier = 1.0f;
	ActiveGravityScale = BaselineGravityScale;
	JumpTimeRemaining = 0.0f;
	ApplyToMovement(Movement);
}

void UNightmarePlayerEffectComponent::TickEffects(float DeltaSeconds, UCharacterMovementComponent* Movement)
{
	if (DeltaSeconds > 0.0f)
	{
		if (SpeedTimeRemaining > 0.0f)
		{
			SpeedTimeRemaining = FMath::Max(0.0f, SpeedTimeRemaining - DeltaSeconds);
			if (SpeedTimeRemaining <= 0.0f)
			{
				SpeedMultiplier = 1.0f;
			}
		}
		if (JumpTimeRemaining > 0.0f)
		{
			JumpTimeRemaining = FMath::Max(0.0f, JumpTimeRemaining - DeltaSeconds);
			if (JumpTimeRemaining <= 0.0f)
			{
				JumpHeightMultiplier = 1.0f;
				ActiveGravityScale = BaselineGravityScale;
			}
		}
	}
	ApplyToMovement(Movement);
}

void UNightmarePlayerEffectComponent::ApplyToMovement(UCharacterMovementComponent* Movement) const
{
	if (!Movement || !bBaselineCaptured)
	{
		return;
	}

	const float SpeedMult = (SpeedTimeRemaining > 0.0f) ? SpeedMultiplier : 1.0f;
	Movement->MaxWalkSpeed = BaselineMaxWalkSpeed * SpeedMult;

	if (JumpTimeRemaining > 0.0f)
	{
		Movement->JumpZVelocity = BaselineJumpZVelocity * JumpHeightMultiplier;
		Movement->GravityScale = ActiveGravityScale;
	}
	else
	{
		Movement->JumpZVelocity = BaselineJumpZVelocity;
		Movement->GravityScale = BaselineGravityScale;
	}
}
