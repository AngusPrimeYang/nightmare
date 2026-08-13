// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareFlowGameMode.h"

#include "GameFramework/PlayerController.h"
#include "NightmareDevCharacter.h"
#include "NightmareEnemySpawner.h"
#include "NightmareFlowPlayerController.h"
#include "NightmareItemSpawner.h"

ANightmareFlowGameMode::ANightmareFlowGameMode()
{
	PlayerControllerClass = ANightmareFlowPlayerController::StaticClass();
	FlowPhase = ENightmareFlowPhase::MainMenu;
}

void ANightmareFlowGameMode::BeginPlay()
{
	Super::BeginPlay();
	SetSpawnersActive(false);
	ResetSpawnerSessions();
	FlowPhase = ENightmareFlowPhase::MainMenu;
}

void ANightmareFlowGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (FlowPhase == ENightmareFlowPhase::MainMenu)
	{
		if (ANightmareFlowPlayerController* FlowPC = Cast<ANightmareFlowPlayerController>(NewPlayer))
		{
			FlowPC->EnterMainMenuState();
		}
		return;
	}

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void ANightmareFlowGameMode::SetSpawnersActive(bool bActive)
{
	if (ItemSpawner)
	{
		ItemSpawner->SetSpawningEnabled(bActive);
		ItemSpawner->SetActorTickEnabled(bActive);
	}
	if (EnemySpawner)
	{
		EnemySpawner->SetSpawningEnabled(bActive);
		EnemySpawner->SetActorTickEnabled(bActive);
	}
}

void ANightmareFlowGameMode::ResetSpawnerSessions()
{
	if (ItemSpawner)
	{
		ItemSpawner->ResetSpawnerSession();
	}
	if (EnemySpawner)
	{
		EnemySpawner->ResetSpawnerSession();
	}
}

void ANightmareFlowGameMode::StartGameplay(APlayerController* PlayerController)
{
	if (!PlayerController || FlowPhase != ENightmareFlowPhase::MainMenu)
	{
		return;
	}

	FlowPhase = ENightmareFlowPhase::Playing;

	if (ANightmareFlowPlayerController* FlowPC = Cast<ANightmareFlowPlayerController>(PlayerController))
	{
		FlowPC->ExitMainMenuState();
	}

	SetSpawnersActive(true);
	RestartPlayer(PlayerController);
}

void ANightmareFlowGameMode::EnterFailedPopupPhase(APlayerController* PlayerController)
{
	if (!PlayerController || FlowPhase != ENightmareFlowPhase::Playing)
	{
		return;
	}

	FlowPhase = ENightmareFlowPhase::FailedPopup;
	SetSpawnersActive(false);

	if (ANightmareFlowPlayerController* FlowPC = Cast<ANightmareFlowPlayerController>(PlayerController))
	{
		FlowPC->ShowFailPopup();
	}
}

void ANightmareFlowGameMode::RestartRound(APlayerController* PlayerController)
{
	if (!PlayerController || FlowPhase != ENightmareFlowPhase::FailedPopup)
	{
		return;
	}

	if (ANightmareFlowPlayerController* FlowPC = Cast<ANightmareFlowPlayerController>(PlayerController))
	{
		FlowPC->ExitFailPopupState();
	}

	ResetSpawnerSessions();
	SetSpawnersActive(true);

	if (ANightmareDevCharacter* DevChar = Cast<ANightmareDevCharacter>(PlayerController->GetPawn()))
	{
		DevChar->ResetForNewRound();
		EnsurePawnSafeSpawn(DevChar);
	}

	FlowPhase = ENightmareFlowPhase::Playing;
}

void ANightmareFlowGameMode::ReturnToMainMenu(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (ANightmareFlowPlayerController* FlowPC = Cast<ANightmareFlowPlayerController>(PlayerController))
	{
		FlowPC->HideFailPopup();
	}

	SetSpawnersActive(false);
	ResetSpawnerSessions();

	if (APawn* Pawn = PlayerController->GetPawn())
	{
		PlayerController->UnPossess();
		Pawn->Destroy();
	}

	FlowPhase = ENightmareFlowPhase::MainMenu;

	if (ANightmareFlowPlayerController* FlowPC = Cast<ANightmareFlowPlayerController>(PlayerController))
	{
		FlowPC->EnterMainMenuState();
	}
}
