// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightmareEnemyTypes.h"
#include "NightmareEnemyActor.generated.h"

class USceneComponent;
class UCapsuleComponent;
class UStaticMeshComponent;
class USphereComponent;
class UNightmareEnemyRoller;
class UNightmareEnemyHealthComponent;
class UNightmareStaminaComponent;
class ACharacter;

/**
 * Graybox enemy. P5 movement/stats; P6 touch damage + despawn; P7 knockback via shared ApplyHitKnockback.
 * P12: Hover vs Walk. P18 health + P19 projectile hit despawn.
 */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmareEnemyActor : public AActor
{
	GENERATED_BODY()

public:
	ANightmareEnemyActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Nightmare|Enemy")
	bool IsDespawned() const { return bDespawned; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void Despawn();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void ApplyStats(const FNightmareEnemyStats& InStats);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Enemy")
	FNightmareEnemyStats GetStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Enemy")
	UNightmareEnemyHealthComponent* GetEnemyHealth() const { return EnemyHealth; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void ApplyRolledHealth(float MaxHealth);

	/**
	 * P6/P7 Spec-friendly attack: ApplyDelta(-AttackPower), optional knockback, then Despawn.
	 * Returns false if already despawned or Stamina is null (enemy stays).
	 */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	bool TryAttackPlayer(UNightmareStaminaComponent* Stamina, ACharacter* KnockbackCharacter);

	/** P19: projectile overlap entry — ApplyDamage then Despawn when health <= 0. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	bool TryTakeProjectileHit(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetBehaviorRoller(UNightmareEnemyRoller* InRoller);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetMoveMode(ENightmareEnemyMoveMode InMode);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Enemy")
	ENightmareEnemyMoveMode GetMoveMode() const { return MoveMode; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetLocomotionType(ENightmareEnemyLocomotionType InType);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Enemy")
	ENightmareEnemyLocomotionType GetLocomotionType() const { return LocomotionType; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetHoverHeight(float Height);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Enemy")
	float GetHoverHeight() const { return HoverHeight; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Enemy")
	float GetDesiredHeightAboveGround() const;

	UFUNCTION(BlueprintPure, Category = "Nightmare|Enemy")
	float ComputeDesiredWorldZ(float GroundZ) const;

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetInjectedGroundZ(float GroundZ);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void ApplyLocomotionHeight();

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetChaseTargetLocation(const FVector& WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetWanderDirection(const FVector& WorldDirectionXY);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetWanderSpeedScale(float Scale);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void SetTimeUntilRetarget(float Seconds);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Enemy")
	float GetTimeUntilRetarget() const { return TimeUntilRetarget; }

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	FVector TickMovement(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Enemy")
	void RetargetFromRoller(UNightmareEnemyRoller* Roller);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> BodyCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GrayboxMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> TouchSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNightmareEnemyHealthComponent> EnemyHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Enemy")
	bool bDespawned;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Enemy")
	FNightmareEnemyStats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Enemy")
	ENightmareEnemyMoveMode MoveMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Enemy")
	ENightmareEnemyLocomotionType LocomotionType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Enemy")
	FVector ChaseTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Enemy")
	FVector WanderDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Enemy")
	float TimeUntilRetarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Enemy")
	bool bHasChaseTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float WanderSpeedScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy", meta = (ClampMin = "0.0"))
	float HoverHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy", meta = (ClampMin = "0.0"))
	float GroundClearance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy", meta = (ClampMin = "0.0"))
	float KnockbackHorizontalSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Enemy", meta = (ClampMin = "0.0"))
	float KnockbackUpSpeed;

	UPROPERTY()
	TObjectPtr<UNightmareEnemyRoller> BehaviorRoller;

	UPROPERTY()
	bool bUseInjectedGroundZ;

	UPROPERTY()
	float InjectedGroundZ;

	void RefreshChaseTargetFromPlayer();
	void RefreshVisualFromStats();
	void RefreshVisualFromLocomotion();
	bool TryResolveGroundZ(float& OutGroundZ) const;
	float GetEffectiveMoveSpeed() const;

	UFUNCTION()
	void OnTouchBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
