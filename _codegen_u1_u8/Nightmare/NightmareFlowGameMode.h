// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NightmareDevGameMode.h"
#include "NightmareFlowTypes.h"
#include "NightmareFlowGameMode.generated.h"

class ANightmareFlowPlayerController;

/** Extends dev GameMode with U1–U8 menu / fail-popup flow on the same dev map. */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmareFlowGameMode : public ANightmareDevGameMode
{
	GENERATED_BODY()

public:
	ANightmareFlowGameMode();

	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void StartGameplay(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void ReturnToMainMenu(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void RestartRound(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void EnterFailedPopupPhase(APlayerController* PlayerController);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Flow")
	ENightmareFlowPhase GetFlowPhase() const { return FlowPhase; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Flow")
	bool IsGameplayActive() const { return FlowPhase == ENightmareFlowPhase::Playing; }

protected:
	void SetSpawnersActive(bool bActive);
	void ResetSpawnerSessions();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Flow")
	ENightmareFlowPhase FlowPhase;
};
