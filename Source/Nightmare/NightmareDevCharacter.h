// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NightmareDevCharacter.generated.h"

class UNightmareStaminaComponent;
class UNightmareInventoryComponent;
class UNightmareMatchComponent;
class UNightmarePlayerEffectComponent;
class UInputComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UStaticMeshComponent;

/**
 * Dev Character: components + Enhanced Input + P10 select/use + P11 jump + P8/P9 effects.
 */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmareDevCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANightmareDevCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Dev")
	void TryCollectNearbyPickups();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Dev")
	void TryUseSelectedInventorySlot();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Dev")
	void SelectInventorySlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Dev")
	int32 GetSelectedInventorySlot() const { return SelectedInventorySlot; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Dev")
	UNightmarePlayerEffectComponent* GetPlayerEffects() const { return PlayerEffects; }

protected:
	virtual void BeginPlay() override;

	void EnsureInputAssets();
	void AddMappingContext();
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void SelectSlot0();
	void SelectSlot1();
	void SelectSlot2();
	void DrawDevStatusHud() const;
	void ClampSelectedInventorySlot();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNightmareStaminaComponent> Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNightmareInventoryComponent> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNightmareMatchComponent> Match;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNightmarePlayerEffectComponent> PlayerEffects;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Graybox")
	TObjectPtr<UStaticMeshComponent> GrayboxBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Graybox")
	TObjectPtr<UStaticMeshComponent> GrayboxHead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Dev", meta = (ClampMin = "50.0"))
	float CollectRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Dev")
	int32 SelectedInventorySlot;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY()
	TObjectPtr<UInputAction> CollectAction;

	UPROPERTY()
	TObjectPtr<UInputAction> UseSelectedSlotAction;

	UPROPERTY()
	TObjectPtr<UInputAction> SelectSlot0Action;

	UPROPERTY()
	TObjectPtr<UInputAction> SelectSlot1Action;

	UPROPERTY()
	TObjectPtr<UInputAction> SelectSlot2Action;
};
