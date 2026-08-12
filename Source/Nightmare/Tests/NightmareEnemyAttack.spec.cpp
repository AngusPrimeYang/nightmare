// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareEnemyActor.h"
#include "NightmareEnemyTypes.h"
#include "NightmareItemEffectApply.h"
#include "NightmareStaminaComponent.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareEnemyAttackSpec,
	"Nightmare.EnemyAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareEnemyAttackSpec)

void FNightmareEnemyAttackSpec::Define()
{
	Describe("ANightmareEnemyActor::TryAttackPlayer", [this]()
	{
		It("deducts AttackPower stamina then despawns (P6)", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			Stamina->SetMaxStamina(100.0f);
			Stamina->ResetStamina();

			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.MoveSpeed = 300.0f;
			Stats.AttackPower = 17.0f;
			Enemy->ApplyStats(Stats);

			TestTrue(TEXT("attack"), Enemy->TryAttackPlayer(Stamina, nullptr));
			TestEqual(TEXT("stamina"), Stamina->GetCurrentStamina(), 83.0f);
			TestTrue(TEXT("despawned"), Enemy->IsDespawned());
		});

		It("rejects null stamina and does not despawn", [this]()
		{
			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.AttackPower = 10.0f;
			Enemy->ApplyStats(Stats);

			TestFalse(TEXT("no stamina"), Enemy->TryAttackPlayer(nullptr, nullptr));
			TestFalse(TEXT("still alive"), Enemy->IsDespawned());
		});

		It("second attack fails after despawn", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			Stamina->SetMaxStamina(100.0f);
			Stamina->ResetStamina();

			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.AttackPower = 5.0f;
			Enemy->ApplyStats(Stats);

			TestTrue(TEXT("first"), Enemy->TryAttackPlayer(Stamina, nullptr));
			TestFalse(TEXT("second"), Enemy->TryAttackPlayer(Stamina, nullptr));
			TestEqual(TEXT("stamina once"), Stamina->GetCurrentStamina(), 95.0f);
		});

		It("clamps stamina at zero when AttackPower exceeds current", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			Stamina->SetMaxStamina(100.0f);
			Stamina->ResetStamina();
			Stamina->ApplyDelta(-90.0f);

			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.AttackPower = 25.0f;
			Enemy->ApplyStats(Stats);

			TestTrue(TEXT("attack"), Enemy->TryAttackPlayer(Stamina, nullptr));
			TestEqual(TEXT("clamped"), Stamina->GetCurrentStamina(), 0.0f);
			TestTrue(TEXT("depleted"), Stamina->IsDepleted());
			TestTrue(TEXT("despawned"), Enemy->IsDespawned());
		});
	});

	Describe("P7 shared knockback entry", [this]()
	{
		It("ApplyHitKnockback is safe with null character (shared P3 path)", [this]()
		{
			NightmareItemEffectApply::ApplyHitKnockback(nullptr, FVector::ZeroVector, 600.0f, 400.0f);
			TestTrue(TEXT("no crash"), true);
		});

		It("attack with AttackPower>0 still succeeds when knockback character is null", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			Stamina->SetMaxStamina(50.0f);
			Stamina->ResetStamina();

			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.AttackPower = 12.0f;
			Enemy->ApplyStats(Stats);

			// Knockback is best-effort (LaunchCharacter needs a live Character); P6 must not depend on it.
			TestTrue(TEXT("attack"), Enemy->TryAttackPlayer(Stamina, nullptr));
			TestEqual(TEXT("stamina"), Stamina->GetCurrentStamina(), 38.0f);
			TestTrue(TEXT("despawned"), Enemy->IsDespawned());
		});
	});
}

#endif
