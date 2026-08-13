// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareFailPopupState.h"

UNightmareFailPopupState::UNightmareFailPopupState()
{
	CountdownDurationSeconds = 10.0f;
	RemainingSeconds = 0.0f;
	bIsOpen = false;
}

void UNightmareFailPopupState::SetCountdownDurationSeconds(float NewDuration)
{
	CountdownDurationSeconds = FMath::Max(0.0f, NewDuration);
}

void UNightmareFailPopupState::Open()
{
	bIsOpen = true;
	RemainingSeconds = CountdownDurationSeconds;
}

void UNightmareFailPopupState::Close()
{
	bIsOpen = false;
	RemainingSeconds = 0.0f;
}

bool UNightmareFailPopupState::TickCountdown(float DeltaSeconds)
{
	if (!bIsOpen || DeltaSeconds <= 0.0f)
	{
		return false;
	}

	RemainingSeconds = FMath::Max(0.0f, RemainingSeconds - DeltaSeconds);
	if (RemainingSeconds <= 0.0f)
	{
		bIsOpen = false;
		return true;
	}

	return false;
}

int32 UNightmareFailPopupState::GetDisplayCountdownSeconds() const
{
	return FMath::Max(0, FMath::CeilToInt(RemainingSeconds));
}
