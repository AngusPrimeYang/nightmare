// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareGameInstance.h"

#include "Kismet/KismetSystemLibrary.h"

void UNightmareGameInstance::QuitNightmare()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
