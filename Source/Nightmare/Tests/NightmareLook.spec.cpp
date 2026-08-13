// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareDevCharacter.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareLookSpec,
	"Nightmare.Look",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareLookSpec)

void FNightmareLookSpec::Define()
{
	Describe("ANightmareDevCharacter look pitch", [this]()
	{
		It("defaults to FPS (InvertLookY off)", [this]()
		{
			ANightmareDevCharacter* Dev = NewObject<ANightmareDevCharacter>(GetTransientPackage());
			TestNotNull(TEXT("dev character"), Dev);
			TestFalse(TEXT("default InvertLookY"), Dev->IsLookYInverted());
		});

		It("negates mouse-up for FPS so pitch looks up", [this]()
		{
			TestEqual(TEXT("FPS mouse-up"), ANightmareDevCharacter::ComputeLookPitchInput(1.0f, false), -1.0f);
			TestEqual(TEXT("FPS mouse-down"), ANightmareDevCharacter::ComputeLookPitchInput(-1.0f, false), 1.0f);
		});

		It("passes mouse-up through when InvertLookY is on", [this]()
		{
			TestEqual(TEXT("invert mouse-up"), ANightmareDevCharacter::ComputeLookPitchInput(1.0f, true), 1.0f);
			TestEqual(TEXT("invert mouse-down"), ANightmareDevCharacter::ComputeLookPitchInput(-1.0f, true), -1.0f);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS
