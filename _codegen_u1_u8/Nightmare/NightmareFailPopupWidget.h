// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NightmareFailPopupWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNightmareFailContinueSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNightmareFailReturnToMenuSignature);

/** U5–U8: stamina-depleted modal with 10s countdown. */
UCLASS()
class NIGHTMARE_API UNightmareFailPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Nightmare|Flow")
	FNightmareFailContinueSignature OnContinueClicked;

	UPROPERTY(BlueprintAssignable, Category = "Nightmare|Flow")
	FNightmareFailReturnToMenuSignature OnReturnToMenuClicked;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Flow")
	void RefreshCountdown(int32 DisplaySeconds);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION()
	void HandleContinueClicked();

	UFUNCTION()
	void HandleReturnToMenuClicked();

	UPROPERTY()
	TObjectPtr<UTextBlock> CountdownText;

	UPROPERTY()
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY()
	TObjectPtr<UButton> ReturnToMenuButton;
};
