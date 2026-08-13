// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NightmareFlowPlayerController.generated.h"

class UNightmareMainMenuWidget;
class UNightmareFailPopupWidget;
class UNightmareFailPopupState;

/** Owns main-menu and fail-popup widgets; bridges UI to ANightmareFlowGameMode. */
UCLASS()
class NIGHTMARE_API ANightmareFlowPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANightmareFlowPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void EnterMainMenuState();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void ExitMainMenuState();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void ShowFailPopup();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void HideFailPopup();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void ExitFailPopupState();

	UFUNCTION(BlueprintPure, Category = "Nightmare|Flow")
	bool IsFailPopupVisible() const;

protected:
	void EnsureMainMenuWidget();
	void EnsureFailPopupWidget();

	UFUNCTION()
	void HandleStartGameClicked();

	UFUNCTION()
	void HandleQuitGameClicked();

	UFUNCTION()
	void HandleContinueClicked();

	UFUNCTION()
	void HandleReturnToMenuClicked();

	void SetUiInputMode(bool bUiOnly);
	void TryDetectMatchFailed();

	UPROPERTY()
	TObjectPtr<UNightmareMainMenuWidget> MainMenuWidget;

	UPROPERTY()
	TObjectPtr<UNightmareFailPopupWidget> FailPopupWidget;

	UPROPERTY()
	TObjectPtr<UNightmareFailPopupState> FailPopupState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Flow")
	bool bMainMenuVisible;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Flow")
	bool bFailPopupVisible;
};
