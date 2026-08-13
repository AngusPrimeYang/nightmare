// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareEnemyHealthComponent.h"

UNightmareEnemyHealthComponent::UNightmareEnemyHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxHealth = 25.0f;
	CurrentHealth = 25.0f;
	bIsDepleted = false;
}

float UNightmareEnemyHealthComponent::ComputeDefaultBaseline(float PlayerMaxStamina)
{
	return FMath::Max(1.0f, FMath::Max(0.0f, PlayerMaxStamina) / 4.0f);
}

void UNightmareEnemyHealthComponent::ResetHealth()
{
	MaxHealth = FMath::Max(1.0f, MaxHealth);
	CurrentHealth = MaxHealth;
	bIsDepleted = false;
}

void UNightmareEnemyHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	MaxHealth = FMath::Max(1.0f, NewMaxHealth);
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	if (CurrentHealth <= 0.0f)
	{
		CurrentHealth = MaxHealth;
	}
	RefreshDepletedState();
}

float UNightmareEnemyHealthComponent::ApplyDamage(float DamageAmount)
{
	if (bIsDepleted || DamageAmount <= 0.0f)
	{
		return CurrentHealth;
	}

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
	RefreshDepletedState();
	return CurrentHealth;
}

void UNightmareEnemyHealthComponent::RefreshDepletedState()
{
	bIsDepleted = CurrentHealth <= 0.0f;
}
