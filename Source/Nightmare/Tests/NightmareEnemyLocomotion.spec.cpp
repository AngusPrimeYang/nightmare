// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareEnemyActor.h"
#include "NightmareEnemyRoller.h"
#include "NightmareEnemyTypes.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareEnemyLocomotionSpec,
	"Nightmare.EnemyLocomotion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareEnemyLocomotionSpec)

void FNightmareEnemyLocomotionSpec::Define()
{
	Describe("UNightmareEnemyRoller locomotion", [this]()
	{
		It("rolls deterministic locomotion for a fixed seed", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->HoverLocomotionChance = 0.5f;
			Roller->SetRandomSeed(77);
			const ENightmareEnemyLocomotionType A = Roller->RollLocomotionType();
			Roller->SetRandomSeed(77);
			const ENightmareEnemyLocomotionType B = Roller->RollLocomotionType();
			TestEqual(TEXT("same seed"), static_cast<uint8>(A), static_cast<uint8>(B));
		});

		It("RollLocomotionTypeWithStream matches injected stream consumption", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->HoverLocomotionChance = 0.4f;
			FRandomStream StreamA(404);
			FRandomStream StreamB(404);
			const ENightmareEnemyLocomotionType A = Roller->RollLocomotionTypeWithStream(StreamA);
			const ENightmareEnemyLocomotionType B = Roller->RollLocomotionTypeWithStream(StreamB);
			TestEqual(TEXT("stream type"), static_cast<uint8>(A), static_cast<uint8>(B));
		});

		It("always rolls Hover when HoverLocomotionChance is 1", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->HoverLocomotionChance = 1.0f;
			Roller->SetRandomSeed(3);
			for (int32 i = 0; i < 24; ++i)
			{
				TestEqual(
					TEXT("hover"),
					static_cast<uint8>(Roller->RollLocomotionType()),
					static_cast<uint8>(ENightmareEnemyLocomotionType::Hover));
			}
		});

		It("always rolls Walk when HoverLocomotionChance is 0", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->HoverLocomotionChance = 0.0f;
			Roller->SetRandomSeed(3);
			for (int32 i = 0; i < 24; ++i)
			{
				TestEqual(
					TEXT("walk"),
					static_cast<uint8>(Roller->RollLocomotionType()),
					static_cast<uint8>(ENightmareEnemyLocomotionType::Walk));
			}
		});

		It("rolls both Hover and Walk when HoverLocomotionChance is mid-range", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->HoverLocomotionChance = 0.5f;
			Roller->SetRandomSeed(9);
			bool bSawHover = false;
			bool bSawWalk = false;
			for (int32 i = 0; i < 64; ++i)
			{
				const ENightmareEnemyLocomotionType Type = Roller->RollLocomotionType();
				if (Type == ENightmareEnemyLocomotionType::Hover) { bSawHover = true; }
				if (Type == ENightmareEnemyLocomotionType::Walk) { bSawWalk = true; }
			}
			TestTrue(TEXT("saw hover"), bSawHover);
			TestTrue(TEXT("saw walk"), bSawWalk);
		});
	});

	Describe("ANightmareEnemyActor Hover vs Walk", [this]()
	{
		It("Hover desired Z is GroundZ plus HoverHeight; Walk is GroundZ plus capsule stand offset", [this]()
		{
			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			Enemy->SetHoverHeight(160.0f);

			Enemy->SetLocomotionType(ENightmareEnemyLocomotionType::Hover);
			TestEqual(TEXT("hover height"), Enemy->GetDesiredHeightAboveGround(), 160.0f);
			TestEqual(TEXT("hover z"), Enemy->ComputeDesiredWorldZ(100.0f), 260.0f);

			Enemy->SetLocomotionType(ENightmareEnemyLocomotionType::Walk);
			TestEqual(TEXT("walk height"), Enemy->GetDesiredHeightAboveGround(), 76.0f);
			TestEqual(TEXT("walk z"), Enemy->ComputeDesiredWorldZ(100.0f), 176.0f);
			TestTrue(TEXT("types differ"), Enemy->ComputeDesiredWorldZ(100.0f) < 260.0f);
		});

		It("TickMovement Hover keeps injected hover altitude while closing XY", [this]()
		{
			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.MoveSpeed = 100.0f;
			Stats.AttackPower = 10.0f;
			Enemy->ApplyStats(Stats);
			Enemy->SetHoverHeight(160.0f);
			Enemy->SetLocomotionType(ENightmareEnemyLocomotionType::Hover);
			Enemy->SetInjectedGroundZ(100.0f);
			Enemy->SetActorLocation(FVector(0.0f, 0.0f, 50.0f));
			Enemy->SetMoveMode(ENightmareEnemyMoveMode::Chase);
			Enemy->SetChaseTargetLocation(FVector(400.0f, 0.0f, 50.0f));

			const FVector Delta = Enemy->TickMovement(0.5f);
			TestTrue(TEXT("moved xy"), Delta.Size2D() > 0.0);
			TestEqual(TEXT("hover z"), static_cast<float>(Enemy->GetActorLocation().Z), 260.0f);
			TestEqual(TEXT("step xy"), static_cast<float>(Delta.Size2D()), 50.0f);
		});

		It("TickMovement Walk plants at stand offset above injected ground while closing XY", [this]()
		{
			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.MoveSpeed = 100.0f;
			Stats.AttackPower = 10.0f;
			Enemy->ApplyStats(Stats);
			Enemy->SetLocomotionType(ENightmareEnemyLocomotionType::Walk);
			Enemy->SetInjectedGroundZ(100.0f);
			Enemy->SetActorLocation(FVector(0.0f, 0.0f, 50.0f));
			Enemy->SetMoveMode(ENightmareEnemyMoveMode::Chase);
			Enemy->SetChaseTargetLocation(FVector(400.0f, 0.0f, 50.0f));

			const FVector Delta = Enemy->TickMovement(0.5f);
			TestTrue(TEXT("moved xy"), Delta.Size2D() > 0.0);
			TestEqual(TEXT("walk z"), static_cast<float>(Enemy->GetActorLocation().Z), 176.0f);
			TestEqual(TEXT("step xy"), static_cast<float>(Delta.Size2D()), 50.0f);
		});

		It("Walk follows a new injected GroundZ; Hover stays higher by HoverHeight minus stand offset", [this]()
		{
			ANightmareEnemyActor* Hover = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			ANightmareEnemyActor* Walk = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			FNightmareEnemyStats Stats;
			Stats.MoveSpeed = 80.0f;
			Hover->ApplyStats(Stats);
			Walk->ApplyStats(Stats);
			Hover->SetHoverHeight(160.0f);
			Hover->SetLocomotionType(ENightmareEnemyLocomotionType::Hover);
			Walk->SetLocomotionType(ENightmareEnemyLocomotionType::Walk);
			Hover->SetWanderSpeedScale(1.0f);
			Walk->SetWanderSpeedScale(1.0f);
			Hover->SetMoveMode(ENightmareEnemyMoveMode::Wander);
			Walk->SetMoveMode(ENightmareEnemyMoveMode::Wander);
			Hover->SetWanderDirection(FVector(1.0f, 0.0f, 0.0f));
			Walk->SetWanderDirection(FVector(1.0f, 0.0f, 0.0f));
			Hover->SetActorLocation(FVector::ZeroVector);
			Walk->SetActorLocation(FVector::ZeroVector);

			Hover->SetInjectedGroundZ(200.0f);
			Walk->SetInjectedGroundZ(200.0f);
			Hover->TickMovement(0.25f);
			Walk->TickMovement(0.25f);

			TestEqual(TEXT("hover follows"), static_cast<float>(Hover->GetActorLocation().Z), 360.0f);
			TestEqual(TEXT("walk follows"), static_cast<float>(Walk->GetActorLocation().Z), 276.0f);
			TestEqual(
				TEXT("gap"),
				static_cast<float>(Hover->GetActorLocation().Z - Walk->GetActorLocation().Z),
				84.0f);
		});

		It("RetargetFromRoller does not change locomotion type", [this]()
		{
			UNightmareEnemyRoller* Roller = NewObject<UNightmareEnemyRoller>(GetTransientPackage());
			Roller->ChaseModeChance = 1.0f;
			Roller->HoverLocomotionChance = 1.0f;
			Roller->RetargetIntervalMin = 2.0f;
			Roller->RetargetIntervalMax = 2.0f;
			Roller->SetRandomSeed(1);

			ANightmareEnemyActor* Enemy = NewObject<ANightmareEnemyActor>(GetTransientPackage());
			Enemy->SetLocomotionType(ENightmareEnemyLocomotionType::Walk);
			Enemy->SetMoveMode(ENightmareEnemyMoveMode::Wander);
			Enemy->RetargetFromRoller(Roller);

			TestEqual(
				TEXT("loco sticky"),
				static_cast<uint8>(Enemy->GetLocomotionType()),
				static_cast<uint8>(ENightmareEnemyLocomotionType::Walk));
			TestEqual(
				TEXT("mode chase"),
				static_cast<uint8>(Enemy->GetMoveMode()),
				static_cast<uint8>(ENightmareEnemyMoveMode::Chase));
		});
	});
}

#endif
