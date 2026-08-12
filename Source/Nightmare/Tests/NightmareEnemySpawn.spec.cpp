// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareEdgeSpawnLocator.h"
#include "NightmareSpawnScheduler.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareEnemySpawnSpec,
	"Nightmare.EnemySpawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareEnemySpawnSpec)

void FNightmareEnemySpawnSpec::Define()
{
	Describe("UNightmareEdgeSpawnLocator", [this]()
	{
		It("rolls deterministic locations for a fixed seed", [this]()
		{
			UNightmareEdgeSpawnLocator* Locator = NewObject<UNightmareEdgeSpawnLocator>(GetTransientPackage());
			Locator->SetOrigin(FVector(10.0f, 20.0f, 0.0f));
			Locator->SetPlayAreaHalfExtent(1000.0f, 800.0f);
			Locator->SetEdgeBandWidth(200.0f);
			Locator->SetSpawnHeight(50.0f);

			Locator->SetRandomSeed(123);
			const FVector A = Locator->RollLocation();
			Locator->SetRandomSeed(123);
			const FVector B = Locator->RollLocation();

			TestEqual(TEXT("x"), A.X, B.X);
			TestEqual(TEXT("y"), A.Y, B.Y);
			TestEqual(TEXT("z"), A.Z, B.Z);
		});

		It("keeps rolled XY inside the edge band of the play rectangle", [this]()
		{
			UNightmareEdgeSpawnLocator* Locator = NewObject<UNightmareEdgeSpawnLocator>(GetTransientPackage());
			Locator->SetOrigin(FVector(100.0f, -50.0f, 0.0f));
			Locator->SetPlayAreaHalfExtent(1500.0f, 1200.0f);
			Locator->SetEdgeBandWidth(300.0f);
			Locator->SetSpawnHeight(75.0f);
			Locator->SetRandomSeed(42);

			for (int32 i = 0; i < 64; ++i)
			{
				const FVector Loc = Locator->RollLocation();
				TestTrue(TEXT("on edge band"), Locator->IsOnEdgeBand(Loc));
				TestEqual(TEXT("spawn height"), Loc.Z, Locator->GetOrigin().Z + Locator->GetSpawnHeight());
			}
		});

		It("rejects interior points that are not on the edge band", [this]()
		{
			UNightmareEdgeSpawnLocator* Locator = NewObject<UNightmareEdgeSpawnLocator>(GetTransientPackage());
			Locator->SetOrigin(FVector::ZeroVector);
			Locator->SetPlayAreaHalfExtent(1000.0f, 1000.0f);
			Locator->SetEdgeBandWidth(200.0f);

			TestFalse(TEXT("center"), Locator->IsOnEdgeBand(FVector(0.0f, 0.0f, 0.0f)));
			TestTrue(TEXT("east edge"), Locator->IsOnEdgeBand(FVector(950.0f, 0.0f, 0.0f)));
			TestFalse(TEXT("outside"), Locator->IsOnEdgeBand(FVector(1200.0f, 0.0f, 0.0f)));
		});

		It("RollLocationWithStream matches injected stream consumption", [this]()
		{
			UNightmareEdgeSpawnLocator* Locator = NewObject<UNightmareEdgeSpawnLocator>(GetTransientPackage());
			Locator->SetOrigin(FVector::ZeroVector);
			Locator->SetPlayAreaHalfExtent(500.0f, 500.0f);
			Locator->SetEdgeBandWidth(100.0f);
			Locator->SetSpawnHeight(10.0f);

			FRandomStream StreamA(999);
			FRandomStream StreamB(999);
			const FVector A = Locator->RollLocationWithStream(StreamA);
			const FVector B = Locator->RollLocationWithStream(StreamB);
			TestEqual(TEXT("stream x"), A.X, B.X);
			TestEqual(TEXT("stream y"), A.Y, B.Y);
			TestEqual(TEXT("stream z"), A.Z, B.Z);
		});
	});

	Describe("Enemy spawn clock (scheduler + edge roll)", [this]()
	{
		It("fires edge rolls when injected spawn delay elapses", [this]()
		{
			UNightmareSpawnScheduler* Scheduler = NewObject<UNightmareSpawnScheduler>(GetTransientPackage());
			Scheduler->SetIntervalRange(4.0f, 4.0f);
			Scheduler->SetTimeUntilNextSpawn(2.0f);

			UNightmareEdgeSpawnLocator* Locator = NewObject<UNightmareEdgeSpawnLocator>(GetTransientPackage());
			Locator->SetOrigin(FVector::ZeroVector);
			Locator->SetPlayAreaHalfExtent(2000.0f, 2000.0f);
			Locator->SetEdgeBandWidth(400.0f);
			Locator->SetSpawnHeight(100.0f);
			Locator->SetRandomSeed(7);

			TestEqual(TEXT("not yet"), Scheduler->TickSpawn(1.0f), 0);

			const int32 Events = Scheduler->TickSpawn(1.0f);
			TestEqual(TEXT("fire"), Events, 1);

			TArray<FVector> Locations;
			for (int32 i = 0; i < Events; ++i)
			{
				const FVector Loc = Locator->RollLocation();
				TestTrue(TEXT("spawn on edge"), Locator->IsOnEdgeBand(Loc));
				Locations.Add(Loc);
			}
			TestEqual(TEXT("one location"), Locations.Num(), 1);
			TestEqual(TEXT("rolled next"), Scheduler->GetTimeUntilNextSpawn(), 4.0f);
		});
	});
}

#endif
