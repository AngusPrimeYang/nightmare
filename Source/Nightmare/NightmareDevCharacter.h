// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NightmareDevCharacter.generated.h"

class UNightmareStaminaComponent;
class UNightmareInventoryComponent;
class UNightmareMatchComponent;
class UInputComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UStaticMeshComponent;

/**
 * Dev smoke Character: Nightmare components + Enhanced Input move/look/jump + Tick bridge + G2 graybox body.
 * Spawned by ANightmareDevGameMode (do not leave a competing AutoPossess pawn in the level).
 * P10: keys 1/2/3 select inventory slot; F uses the selected slot once.
 * Jump: Space -> ACharacter::Jump / StopJumping.
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** G2 graybox — simple block body (no walk anim). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Graybox")
	TObjectPtr<UStaticMeshComponent> GrayboxBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Graybox")
	TObjectPtr<UStaticMeshComponent> GrayboxHead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Dev", meta = (ClampMin = "50.0"))
	float CollectRadius;

	/** P10 — which inventory slot F will use (0-based). */
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
