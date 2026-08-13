// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NightmareProjectileTypes.h"
#include "NightmareProjectileActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class ANightmareEnemyActor;

/**
 * P13–P17 graybox projectile. Straight-line flight, elongated hit box, damages enemies only.
 * Spec-friendly: TickFlight / TryApplyHitToEnemy need no World.
 */
UCLASS(Blueprintable)
class NIGHTMARE_API ANightmareProjectileActor : public AActor
{
	GENERATED_BODY()

public:
	ANightmareProjectileActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** P14 default = WalkSpeed * 10 (600 -> 6000). */
	UFUNCTION(BlueprintPure, Category = "Nightmare|Projectile")
	static float ComputeDefaultSpeed(float WalkSpeed);

	/** P17 default = PlayerMaxStamina / 20 (100 -> 5). */
	UFUNCTION(BlueprintPure, Category = "Nightmare|Projectile")
	static float ComputeDefaultDamage(float PlayerMaxStamina);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Projectile")
	void ConfigureProjectile(
		const FVector& InDirection,
		float InSpeed,
		float InDamage,
		ENightmareProjectileTrajectory InTrajectory,
		const FVector& InHalfExtents);

	UFUNCTION(BlueprintPure, Category = "Nightmare|Projectile")
	bool IsConsumed() const { return bConsumed; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Projectile")
	FVector GetFlightDirection() const { return FlightDirection; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Projectile")
	float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Projectile")
	float GetDamage() const { return Damage; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Projectile")
	ENightmareProjectileTrajectory GetTrajectory() const { return Trajectory; }

	UFUNCTION(BlueprintPure, Category = "Nightmare|Projectile")
	FVector GetHalfExtents() const { return HalfExtents; }

	/** Advances along flight direction. Returns displacement applied. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Projectile")
	FVector TickFlight(float DeltaSeconds);

	/** P19 entry: damages enemy, consumes projectile. Returns false if already consumed / invalid. */
	UFUNCTION(BlueprintCallable, Category = "Nightmare|Projectile")
	bool TryApplyHitToEnemy(ANightmareEnemyActor* Enemy);

	UFUNCTION(BlueprintCallable, Category = "Nightmare|Projectile")
	void Consume();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> HitBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GrayboxMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Projectile")
	bool bConsumed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Projectile")
	FVector FlightDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Projectile", meta = (ClampMin = "0.0"))
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Projectile", meta = (ClampMin = "0.0"))
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Projectile")
	ENightmareProjectileTrajectory Trajectory;

	/** P16: half-extents along local X (forward) / Y / Z — collision and mesh share these. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Projectile", meta = (ClampMin = "0.0"))
	FVector HalfExtents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nightmare|Projectile", meta = (ClampMin = "0.0"))
	float MaxLifetime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nightmare|Projectile")
	float Age;

	void ApplyVisualFromHalfExtents();
	void TryResolveSweepHit(const FHitResult& Hit);

	UFUNCTION()
	void OnHitBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
