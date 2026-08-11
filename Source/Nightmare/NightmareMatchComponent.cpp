// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareMatchComponent.h"
#include "NightmareStaminaComponent.h"

UNightmareMatchComponent::UNightmareMatchComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SurviveDurationSeconds = 120.0f;
	ElapsedSeconds = 0.0f;
	MatchState = ENightmareMatchState::Playing;
}

void UNightmareMatchComponent::ResetMatch()
{
	ElapsedSeconds = 0.0f;
	MatchState = ENightmareMatchState::Playing;
}

void UNightmareMatchComponent::SetSurviveDurationSeconds(float NewDuration)
{
	SurviveDurationSeconds = FMath::Max(0.0f, NewDuration);
}

ENightmareMatchState UNightmareMatchComponent::Evaluate(bool bStaminaDepleted, float InElapsedSeconds) const
{
	if (bStaminaDepleted)
	{
		return ENightmareMatchState::Failed;
	}
	if (InElapsedSeconds >= SurviveDurationSeconds)
	{
		return ENightmareMatchState::Won;
	}
	return ENightmareMatchState::Playing;
}

void UNightmareMatchComponent::TickMatch(float DeltaSeconds, const UNightmareStaminaComponent* Stamina)
{
	if (MatchState != ENightmareMatchState::Playing)
	{
		return;
	}

	const bool bDepleted = Stamina && Stamina->IsDepleted();
	if (bDepleted)
	{
		MatchState = ENightmareMatchState::Failed;
		return;
	}

	if (DeltaSeconds > 0.0f)
	{
		ElapsedSeconds += DeltaSeconds;
	}

	MatchState = Evaluate(bDepleted, ElapsedSeconds);
}