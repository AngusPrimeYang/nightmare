// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareSpawnScheduler.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareSpawnSpec,
	"Nightmare.Spawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareSpawnSpec)

void FNightmareSpawnSpec::Define()
{
	Describe("UNightmareSpawnScheduler", [this]()
	{
		It("fires one spawn when injected delay elapses", [this]()
		{
			UNightmareSpawnScheduler* Spawn = NewObject<UNightmareSpawnScheduler>(GetTransientPackage());
			Spawn->SetIntervalRange(3.0f, 3.0f);
			Spawn->SetTimeUntilNextSpawn(2.0f);

			TestEqual(TEXT("not yet"), Spawn->TickSpawn(1.0f), 0);
			TestEqual(TEXT("fire"), Spawn->TickSpawn(1.0f), 1);
			TestEqual(TEXT("rolled next"), Spawn->GetTimeUntilNextSpawn(), 3.0f);
		});

		It("can fire multiple times when DeltaSeconds covers several intervals", [this]()
		{
			UNightmareSpawnScheduler* Spawn = NewObject<UNightmareSpawnScheduler>(GetTransientPackage());
			Spawn->SetIntervalRange(1.0f, 1.0f);
			Spawn->SetTimeUntilNextSpawn(1.0f);

			TestEqual(TEXT("three fires"), Spawn->TickSpawn(3.0f), 3);
		});

		It("uses seeded random range between Min and Max", [this]()
		{
			UNightmareSpawnScheduler* Spawn = NewObject<UNightmareSpawnScheduler>(GetTransientPackage());
			Spawn->SetRandomSeed(42);
			Spawn->SetIntervalRange(2.0f, 5.0f);
			Spawn->ResetScheduler();

			const float Delay = Spawn->GetTimeUntilNextSpawn();
			TestTrue(TEXT(">= min"), Delay >= 2.0f);
			TestTrue(TEXT("<= max"), Delay <= 5.0f);
		});
	});
}

#endif