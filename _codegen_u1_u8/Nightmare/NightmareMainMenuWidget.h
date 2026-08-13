// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NightmareMainMenuWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNightmareMainMenuStartSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNightmareMainMenuQuitSignature);

/** U1–U4: full-screen black main menu built in C++ (no .uasset). */
UCLASS()
class NIGHTMARE_API UNightmareMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Nightmare|Flow")
	FNightmareMainMenuStartSignature OnStartGameClicked;

	UPROPERTY(BlueprintAssignable, Category = "Nightmare|Flow")
	FNightmareMainMenuQuitSignature OnQuitGameClicked;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UPROPERTY()
	TObjectPtr<UButton> StartButton;

	UPROPERTY()
	TObjectPtr<UButton> MultiplayerButton;

	UPROPERTY()
	TObjectPtr<UButton> QuitButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;
};
