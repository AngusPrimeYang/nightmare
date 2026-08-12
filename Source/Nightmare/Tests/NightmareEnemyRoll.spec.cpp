// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareEnemyActor.h"
#include "NightmareEnemyRoller.h"
#include "NightmareEnemyTypes.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareEnemyRollSpec,
	"Nightmare.EnemyRoll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareEnemyRollSpec)

void FNightmareEnemyRollSpec::Define()
{
	Describe("UNightmareEnemyRoller", [this]()
	{
		It("rolls deterministic stats for a fixed seed", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->SetRandomSeed(321);
			const FNightmareEnemyStats A = Roller->RollStats();
			Roller->SetRandomSeed(321);
			const FNightmareEnemyStats B = Roller->RollStats();
			TestEqual(TEXT("speed"), A.MoveSpeed, B.MoveSpeed);
			TestEqual(TEXT("power"), A.AttackPower, B.AttackPower);
		});

		It("keeps MoveSpeed and AttackPower inside configured Min/Max", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->MoveSpeedMin = 220.0f;
			Roller->MoveSpeedMax = 340.0f;
			Roller->AttackPowerMin = 8.0f;
			Roller->AttackPowerMax = 15.0f;
			Roller->SetRandomSeed(11);

			for (int32 i = 0; i < 48; ++i)
			{
				const FNightmareEnemyStats Stats = Roller->RollStats();
				TestTrue(TEXT("speed >= min"), Stats.MoveSpeed >= 220.0f);
				TestTrue(TEXT("speed <= max"), Stats.MoveSpeed <= 340.0f);
				TestTrue(TEXT("power >= min"), Stats.AttackPower >= 8.0f);
				TestTrue(TEXT("power <= max"), Stats.AttackPower <= 15.0f);
			}
		});

		It("rolls both Chase and Wander when ChaseModeChance is mid-range", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->ChaseModeChance = 0.5f;
			Roller->SetRandomSeed(9);
			bool bSawChase = false;
			bool bSawWander = false;
			for (int32 i = 0; i < 64; ++i)
			{
				const ENightmareEnemyMoveMode Mode = Roller->RollMoveMode();
				if (Mode == ENightmareEnemyMoveMode::Chase) { bSawChase = true; }
				if (Mode == ENightmareEnemyMoveMode::Wander) { bSawWander = true; }
			}
			TestTrue(TEXT("saw chase"), bSawChase);
			TestTrue(TEXT("saw wander"), bSawWander);
		});

		It("RollStatsWithStream matches injected stream consumption", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			FRandomStream StreamA(555);
			FRandomStream StreamB(555);
			const FNightmareEnemyStats A = Roller->RollStatsWithStream(StreamA);
			const FNightmareEnemyStats B = Roller->RollStatsWithStream(StreamB);
			TestEqual(TEXT("stream speed"), A.MoveSpeed, B.MoveSpeed);
			TestEqual(TEXT("stream power"), A.AttackPower, B.AttackPower);
		});
	});

	Describe("ANightmareEnemyActor movement", [this]()
	{
		It("TickMovement in Chase closes XY distance toward injected target", [this]()
		{
			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.MoveSpeed = 100.0f;
			Stats.AttackPower = 10.0f;
			Enemy->ApplyStats(Stats);
			Enemy->SetActorLocation(FVector(0.0f, 0.0f, 50.0f));
			Enemy->SetMoveMode(ENightmareEnemyMoveMode::Chase);
			Enemy->SetChaseTargetLocation(FVector(400.0f, 0.0f, 50.0f));

			const float DistBefore = FVector::Dist2D(Enemy->GetActorLocation(), FVector(400.0f, 0.0f, 50.0f));
			const FVector Delta = Enemy->TickMovement(0.5f);
			const float DistAfter = FVector::Dist2D(Enemy->GetActorLocation(), FVector(400.0f, 0.0f, 50.0f));

			TestTrue(TEXT("moved"), Delta.Size2D() > 0.0);
			TestTrue(TEXT("closer"), DistAfter < DistBefore);
			TestEqual(TEXT("step size"), static_cast<float>(Delta.Size2D()), 50.0f);
		});

		It("TickMovement in Wander follows injected direction at MoveSpeed", [this]()
		{
			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.MoveSpeed = 200.0f;
			Stats.AttackPower = 7.0f;
			Enemy->ApplyStats(Stats);
			Enemy->SetActorLocation(FVector::ZeroVector);
			Enemy->SetMoveMode(ENightmareEnemyMoveMode::Wander);
			Enemy->SetWanderSpeedScale(1.0f);
			Enemy->SetWanderDirection(FVector(0.0f, 1.0f, 0.0f));

			const FVector Delta = Enemy->TickMovement(0.25f);
			TestEqual(TEXT("dx"), static_cast<float>(Delta.X), 0.0f);
			TestEqual(TEXT("dy"), static_cast<float>(Delta.Y), 50.0f);
		});

		It("RetargetFromRoller assigns mode from seeded roller", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->ChaseModeChance = 1.0f;
			Roller->RetargetIntervalMin = 2.0f;
			Roller->RetargetIntervalMax = 2.0f;
			Roller->SetRandomSeed(1);

			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			Enemy->SetMoveMode(ENightmareEnemyMoveMode::Wander);
			Enemy->RetargetFromRoller(Roller);

			TestEqual(TEXT("mode chase"), static_cast<uint8>(Enemy->GetMoveMode()), static_cast<uint8>(ENightmareEnemyMoveMode::Chase));
			TestEqual(TEXT("interval"), Enemy->GetTimeUntilRetarget(), 2.0f);
		});
	});
}

#endif
