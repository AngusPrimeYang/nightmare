// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareFlowPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NightmareDevCharacter.h"
#include "NightmareFailPopupState.h"
#include "NightmareFailPopupWidget.h"
#include "NightmareFlowGameMode.h"
#include "NightmareGameInstance.h"
#include "NightmareMainMenuWidget.h"
#include "NightmareMatchComponent.h"

ANightmareFlowPlayerController::ANightmareFlowPlayerController()
{
	bShowMouseCursor = true;
	bMainMenuVisible = false;
	bFailPopupVisible = false;
}

void ANightmareFlowPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FailPopupState = NewObject<UNightmareFailPopupState>(this, TEXT("FailPopupState"));

	if (ANightmareFlowGameMode* FlowGM = GetWorld() ? GetWorld()->GetAuthGameMode<ANightmareFlowGameMode>() : nullptr)
	{
		if (FlowGM->GetFlowPhase() == ENightmareFlowPhase::MainMenu)
		{
			EnterMainMenuState();
		}
	}
}

void ANightmareFlowPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bFailPopupVisible)
	{
		TryDetectMatchFailed();
	}

	if (bFailPopupVisible && FailPopupState && FailPopupState->IsOpen())
	{
		if (FailPopupState->TickCountdown(DeltaSeconds))
		{
			HandleReturnToMenuClicked();
			return;
		}

		if (FailPopupWidget)
		{
			FailPopupWidget->RefreshCountdown(FailPopupState->GetDisplayCountdownSeconds());
		}
	}
}

void ANightmareFlowPlayerController::EnsureMainMenuWidget()
{
	if (MainMenuWidget)
	{
		return;
	}

	MainMenuWidget = CreateWidget<UNightmareMainMenuWidget>(this, UNightmareMainMenuWidget::StaticClass());
	if (!MainMenuWidget)
	{
		return;
	}

	MainMenuWidget->OnStartGameClicked.AddDynamic(this, &ANightmareFlowPlayerController::HandleStartGameClicked);
	MainMenuWidget->OnQuitGameClicked.AddDynamic(this, &ANightmareFlowPlayerController::HandleQuitGameClicked);
}

void ANightmareFlowPlayerController::EnsureFailPopupWidget()
{
	if (FailPopupWidget)
	{
		return;
	}

	FailPopupWidget = CreateWidget<UNightmareFailPopupWidget>(this, UNightmareFailPopupWidget::StaticClass());
	if (!FailPopupWidget)
	{
		return;
	}

	FailPopupWidget->OnContinueClicked.AddDynamic(this, &ANightmareFlowPlayerController::HandleContinueClicked);
	FailPopupWidget->OnReturnToMenuClicked.AddDynamic(this, &ANightmareFlowPlayerController::HandleReturnToMenuClicked);
}

void ANightmareFlowPlayerController::SetUiInputMode(bool bUiOnly)
{
	if (bUiOnly)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}

void ANightmareFlowPlayerController::EnterMainMenuState()
{
	EnsureMainMenuWidget();
	if (!MainMenuWidget)
	{
		return;
	}

	if (!MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->AddToViewport(100);
	}
	MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
	bMainMenuVisible = true;
	SetUiInputMode(true);

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->DisableInput(this);
	}
}

void ANightmareFlowPlayerController::ExitMainMenuState()
{
	if (MainMenuWidget)
	{
		MainMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	bMainMenuVisible = false;
	SetUiInputMode(false);

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->EnableInput(this);
	}
}

void ANightmareFlowPlayerController::ShowFailPopup()
{
	if (bFailPopupVisible)
	{
		return;
	}

	EnsureFailPopupWidget();
	if (!FailPopupWidget || !FailPopupState)
	{
		return;
	}

	FailPopupState->Open();
	FailPopupWidget->RefreshCountdown(FailPopupState->GetDisplayCountdownSeconds());

	if (!FailPopupWidget->IsInViewport())
	{
		FailPopupWidget->AddToViewport(200);
	}
	FailPopupWidget->SetVisibility(ESlateVisibility::Visible);
	bFailPopupVisible = true;
	SetUiInputMode(true);

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->DisableInput(this);
	}
}

void ANightmareFlowPlayerController::HideFailPopup()
{
	if (FailPopupWidget)
	{
		FailPopupWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (FailPopupState)
	{
		FailPopupState->Close();
	}
	bFailPopupVisible = false;
}

void ANightmareFlowPlayerController::ExitFailPopupState()
{
	HideFailPopup();
	if (!bMainMenuVisible)
	{
		SetUiInputMode(false);
		if (APawn* ControlledPawn = GetPawn())
		{
			ControlledPawn->EnableInput(this);
		}
	}
}

bool ANightmareFlowPlayerController::IsFailPopupVisible() const
{
	return bFailPopupVisible;
}

void ANightmareFlowPlayerController::TryDetectMatchFailed()
{
	const ANightmareFlowGameMode* FlowGM = GetWorld() ? GetWorld()->GetAuthGameMode<ANightmareFlowGameMode>() : nullptr;
	if (!FlowGM || FlowGM->GetFlowPhase() != ENightmareFlowPhase::Playing)
	{
		return;
	}

	const ANightmareDevCharacter* DevChar = Cast<ANightmareDevCharacter>(GetPawn());
	if (!DevChar)
	{
		return;
	}

	const UNightmareMatchComponent* Match = DevChar->GetNightmareMatch();
	if (!Match || Match->GetMatchState() != ENightmareMatchState::Failed)
	{
		return;
	}

	if (ANightmareFlowGameMode* MutableGM = GetWorld()->GetAuthGameMode<ANightmareFlowGameMode>())
	{
		MutableGM->EnterFailedPopupPhase(this);
	}
}

void ANightmareFlowPlayerController::HandleStartGameClicked()
{
	if (ANightmareFlowGameMode* FlowGM = GetWorld() ? GetWorld()->GetAuthGameMode<ANightmareFlowGameMode>() : nullptr)
	{
		FlowGM->StartGameplay(this);
	}
}

void ANightmareFlowPlayerController::HandleQuitGameClicked()
{
	if (UNightmareGameInstance* GI = GetGameInstance<UNightmareGameInstance>())
	{
		GI->QuitNightmare();
	}
	else
	{
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
	}
}

void ANightmareFlowPlayerController::HandleContinueClicked()
{
	if (ANightmareFlowGameMode* FlowGM = GetWorld() ? GetWorld()->GetAuthGameMode<ANightmareFlowGameMode>() : nullptr)
	{
		FlowGM->RestartRound(this);
	}
}

void ANightmareFlowPlayerController::HandleReturnToMenuClicked()
{
	if (ANightmareFlowGameMode* FlowGM = GetWorld() ? GetWorld()->GetAuthGameMode<ANightmareFlowGameMode>() : nullptr)
	{
		FlowGM->ReturnToMainMenu(this);
	}
}
