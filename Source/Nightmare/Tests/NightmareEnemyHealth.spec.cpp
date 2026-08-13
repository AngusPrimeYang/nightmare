// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareEnemyActor.h"
#include "NightmareEnemyHealthComponent.h"
#include "NightmareEnemyRoller.h"
#include "NightmareProjectileActor.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareEnemyHealthSpec,
	"Nightmare.EnemyHealth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareEnemyHealthSpec)

void FNightmareEnemyHealthSpec::Define()
{
	Describe("UNightmareEnemyHealthComponent", [this]()
	{
		It("ComputeDefaultBaseline equals PlayerMaxStamina divided by 4", [this]()
		{
			TestEqual(TEXT("100 -> 25"), UNightmareEnemyHealthComponent::ComputeDefaultBaseline(100.0f), 25.0f);
			TestEqual(TEXT("min 1"), UNightmareEnemyHealthComponent::ComputeDefaultBaseline(0.0f), 1.0f);
		});

		It("ApplyDamage clamps at zero and marks depleted", [this]()
		{
			UNightmareEnemyHealthComponent* Health = NewObject<UNightmareEnemyHealthComponent>(GetTransientPackage());
			Health->SetMaxHealth(25.0f);
			Health->ApplyDamage(10.0f);
			TestEqual(TEXT("remaining"), Health->GetCurrentHealth(), 15.0f);
			TestFalse(TEXT("not depleted"), Health->IsDepleted());

			Health->ApplyDamage(20.0f);
			TestEqual(TEXT("clamped"), Health->GetCurrentHealth(), 0.0f);
			TestTrue(TEXT("depleted"), Health->IsDepleted());
		});

		It("SetMaxHealth enforces at least 1", [this]()
		{
			UNightmareEnemyHealthComponent* Health = NewObject<UNightmareEnemyHealthComponent>(GetTransientPackage());
			Health->SetMaxHealth(0.0f);
			TestEqual(TEXT("min max"), Health->GetMaxHealth(), 1.0f);
		});
	});

	Describe("UNightmareEnemyRoller health", [this]()
	{
		It("rolls deterministic health for a fixed seed", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->HealthMin = 10.0f;
			Roller->HealthMax = 30.0f;
			Roller->SetRandomSeed(55);
			const float A = Roller->RollHealth();
			Roller->SetRandomSeed(55);
			const float B = Roller->RollHealth();
			TestEqual(TEXT("same seed"), A, B);
		});

		It("keeps rolled health inside configured Min/Max and at least 1", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->HealthMin = 18.0f;
			Roller->HealthMax = 32.0f;
			Roller->SetRandomSeed(3);
			for (int32 i = 0; i < 48; ++i)
			{
				const float Health = Roller->RollHealth();
				TestTrue(TEXT(">= min"), Health >= 18.0f);
				TestTrue(TEXT("<= max"), Health <= 32.0f);
				TestTrue(TEXT(">= 1"), Health >= 1.0f);
			}
		});

		It("default RollHealth range is 5 to 70", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			TestEqual(TEXT("min default"), Roller->HealthMin, 5.0f);
			TestEqual(TEXT("max default"), Roller->HealthMax, 70.0f);
			Roller->SetRandomSeed(9);
			bool bSawLow = false;
			bool bSawHigh = false;
			for (int32 i = 0; i < 96; ++i)
			{
				const float Health = Roller->RollHealth();
				TestTrue(TEXT(">= 5"), Health >= 5.0f);
				TestTrue(TEXT("<= 70"), Health <= 70.0f);
				if (Health <= 20.0f) { bSawLow = true; }
				if (Health >= 50.0f) { bSawHigh = true; }
			}
			TestTrue(TEXT("saw low band"), bSawLow);
			TestTrue(TEXT("saw high band"), bSawHigh);
		});

		It("RollHealthWithStream matches injected stream consumption", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->HealthMin = 12.0f;
			Roller->HealthMax = 28.0f;
			FRandomStream StreamA(808);
			FRandomStream StreamB(808);
			TestEqual(TEXT("stream health"), Roller->RollHealthWithStream(StreamA), Roller->RollHealthWithStream(StreamB));
		});
	});

	Describe("P19 projectile hit despawn", [this]()
	{
		It("TryTakeProjectileHit reduces health without despawning until depleted", [this]()
		{
			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			Enemy->ApplyRolledHealth(25.0f);

			TestTrue(TEXT("hit"), Enemy->TryTakeProjectileHit(5.0f));
			TestEqual(TEXT("hp"), Enemy->GetEnemyHealth()->GetCurrentHealth(), 20.0f);
			TestFalse(TEXT("alive"), Enemy->IsDespawned());
		});

		It("TryApplyHitToEnemy despawns enemy when damage reaches zero and consumes projectile", [this]()
		{
			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			Enemy->ApplyRolledHealth(10.0f);

			ANightmareProjectileActor* Projectile = NewObject<ANightmareProjectileActor>(GetTransientPackage());
			Projectile->ConfigureProjectile(
				FVector(1.0f, 0.0f, 0.0f),
				1000.0f,
				5.0f,
				ENightmareProjectileTrajectory::StraightLine,
				FVector(40.0f, 4.0f, 4.0f));

			TestTrue(TEXT("first hit"), Projectile->TryApplyHitToEnemy(Enemy));
			TestFalse(TEXT("enemy alive"), Enemy->IsDespawned());
			TestEqual(TEXT("hp left"), Enemy->GetEnemyHealth()->GetCurrentHealth(), 5.0f);
			TestTrue(TEXT("projectile consumed"), Projectile->IsConsumed());

			ANightmareProjectileActor* Finisher = NewObject<ANightmareProjectileActor>(GetTransientPackage());
			Finisher->ConfigureProjectile(
				FVector(1.0f, 0.0f, 0.0f),
				1000.0f,
				5.0f,
				ENightmareProjectileTrajectory::StraightLine,
				FVector(40.0f, 4.0f, 4.0f));
			TestTrue(TEXT("kill hit"), Finisher->TryApplyHitToEnemy(Enemy));
			TestTrue(TEXT("despawned"), Enemy->IsDespawned());
		});

		It("TryTakeProjectileHit rejects zero damage and despawned enemies", [this]()
		{
			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			Enemy->ApplyRolledHealth(10.0f);
			TestFalse(TEXT("zero dmg"), Enemy->TryTakeProjectileHit(0.0f));
			Enemy->Despawn();
			TestFalse(TEXT("after despawn"), Enemy->TryTakeProjectileHit(5.0f));
		});
	});
}

#endif
