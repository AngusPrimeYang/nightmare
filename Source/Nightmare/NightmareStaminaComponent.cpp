// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareStaminaComponent.h"

UNightmareStaminaComponent::UNightmareStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxStamina = 100.0f;
	CurrentStamina = 100.0f;
	StaminaDrainPerSecond = 5.0f;
	bIsDepleted = false;
}

void UNightmareStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	ResetStamina();
}

void UNightmareStaminaComponent::ResetStamina()
{
	CurrentStamina = FMath::Max(0.0f, MaxStamina);
	bIsDepleted = CurrentStamina <= 0.0f;
}

void UNightmareStaminaComponent::SetMaxStamina(float NewMaxStamina)
{
	MaxStamina = FMath::Max(0.0f, NewMaxStamina);
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	RefreshDepletedState();
}

void UNightmareStaminaComponent::SetStaminaDrainPerSecond(float NewDrainPerSecond)
{
	StaminaDrainPerSecond = FMath::Max(0.0f, NewDrainPerSecond);
}

float UNightmareStaminaComponent::ApplyDelta(float Delta)
{
	if (bIsDepleted && Delta <= 0.0f)
	{
		CurrentStamina = 0.0f;
		return CurrentStamina;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina + Delta, 0.0f, MaxStamina);
	RefreshDepletedState();
	return CurrentStamina;
}

void UNightmareStaminaComponent::TickStamina(float DeltaSeconds)
{
	if (DeltaSeconds <= 0.0f || StaminaDrainPerSecond <= 0.0f || bIsDepleted)
	{
		return;
	}

	ApplyDelta(-StaminaDrainPerSecond * DeltaSeconds);
}

void UNightmareStaminaComponent::RefreshDepletedState()
{
	const bool bWasDepleted = bIsDepleted;
	bIsDepleted = CurrentStamina <= 0.0f;
	if (bIsDepleted && !bWasDepleted)
	{
		OnStaminaDepleted.Broadcast();
	}
}