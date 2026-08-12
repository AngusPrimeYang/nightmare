// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmarePlayerEffectComponent.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmarePlayerEffectSpec,
	"Nightmare.PlayerEffect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmarePlayerEffectSpec)

void FNightmarePlayerEffectSpec::Define()
{
	Describe("UNightmarePlayerEffectComponent", [this]()
	{
		It("speed effect expires after duration", [this]()
		{
			UNightmarePlayerEffectComponent* Effects = NewObject<UNightmarePlayerEffectComponent>(GetTransientPackage());
			Effects->ApplySpeedEffect(1.5f, 2.0f);
			TestTrue(TEXT("active"), Effects->HasActiveSpeedEffect());
			Effects->TickEffects(1.0f, nullptr);
			TestEqual(TEXT("mid"), Effects->GetSpeedTimeRemaining(), 1.0f);
			Effects->TickEffects(1.5f, nullptr);
			TestFalse(TEXT("expired"), Effects->HasActiveSpeedEffect());
		});

		It("jump effect expires after duration", [this]()
		{
			UNightmarePlayerEffectComponent* Effects = NewObject<UNightmarePlayerEffectComponent>(GetTransientPackage());
			Effects->ApplyJumpEffect(0.1f, 2.0f, 1.0f);
			TestTrue(TEXT("active"), Effects->HasActiveJumpEffect());
			Effects->TickEffects(1.0f, nullptr);
			TestFalse(TEXT("expired"), Effects->HasActiveJumpEffect());
		});
	});
}

#endif
