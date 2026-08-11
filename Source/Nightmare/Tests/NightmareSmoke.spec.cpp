// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

/**
 * SETUP F smoke Spec: proves Logic Gate discovers and runs Nightmare.* tests.
 * Keep free of world/PIE/timing; replace with richer Specs as systems land.
 */
BEGIN_DEFINE_SPEC(FNightmareSmokeSpec,
	"Nightmare.Smoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareSmokeSpec)

void FNightmareSmokeSpec::Define()
{
	Describe("Logic gate wiring", [this]()
	{
		It("proves Nightmare Automation Specs are discoverable and runnable", [this]()
		{
			const FString Marker = TEXT("Nightmare.Smoke");
			TestTrue(TEXT("smoke marker is not empty"), !Marker.IsEmpty());
			TestEqual(TEXT("FString equality works under Automation Spec"), Marker, FString(TEXT("Nightmare.Smoke")));
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS
