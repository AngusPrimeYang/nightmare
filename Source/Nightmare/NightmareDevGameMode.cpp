// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareDevGameMode.h"
#include "NightmareDevCharacter.h"

ANightmareDevGameMode::ANightmareDevGameMode()
{
	DefaultPawnClass = ANightmareDevCharacter::StaticClass();
}
