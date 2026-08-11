// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareStaminaComponent.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareStaminaSpec,
	"Nightmare.Stamina",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareStaminaSpec)

void FNightmareStaminaSpec::Define()
{
	Describe("UNightmareStaminaComponent", [this]()
	{
		It("resets CurrentStamina to MaxStamina", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			TestNotNull(TEXT("stamina component"), Stamina);

			Stamina->SetMaxStamina(100.0f);
			Stamina->ApplyDelta(-40.0f);
			Stamina->ResetStamina();

			TestEqual(TEXT("current after reset"), Stamina->GetCurrentStamina(), 100.0f);
			TestFalse(TEXT("not depleted after reset"), Stamina->IsDepleted());
		});

		It("sets CurrentStamina from 100 to 50 after ApplyDelta(-50)", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			TestNotNull(TEXT("stamina component"), Stamina);

			Stamina->SetMaxStamina(100.0f);
			Stamina->ResetStamina();
			Stamina->ApplyDelta(-50.0f);

			TestEqual(TEXT("current after damage"), Stamina->GetCurrentStamina(), 50.0f);
			TestFalse(TEXT("not depleted at 50"), Stamina->IsDepleted());
		});

		It("clamps CurrentStamina to MaxStamina on positive ApplyDelta", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			TestNotNull(TEXT("stamina component"), Stamina);

			Stamina->SetMaxStamina(100.0f);
			Stamina->ResetStamina();
			Stamina->ApplyDelta(25.0f);

			TestEqual(TEXT("clamped to max"), Stamina->GetCurrentStamina(), 100.0f);
		});

		It("marks depleted and clamps to 0 when ApplyDelta drives stamina to or below zero", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			TestNotNull(TEXT("stamina component"), Stamina);

			Stamina->SetMaxStamina(100.0f);
			Stamina->ResetStamina();
			Stamina->ApplyDelta(-150.0f);

			TestEqual(TEXT("current at floor"), Stamina->GetCurrentStamina(), 0.0f);
			TestTrue(TEXT("depleted signal"), Stamina->IsDepleted());
		});

		It("drains stamina over TickStamina using StaminaDrainPerSecond", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			TestNotNull(TEXT("stamina component"), Stamina);

			Stamina->SetMaxStamina(100.0f);
			Stamina->SetStaminaDrainPerSecond(10.0f);
			Stamina->ResetStamina();
			Stamina->TickStamina(2.0f);

			TestEqual(TEXT("drained by 20 over 2s at 10/s"), Stamina->GetCurrentStamina(), 80.0f);
			TestFalse(TEXT("not depleted yet"), Stamina->IsDepleted());
		});

		It("stays depleted at 0 after further negative drain ticks", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			TestNotNull(TEXT("stamina component"), Stamina);

			Stamina->SetMaxStamina(10.0f);
			Stamina->SetStaminaDrainPerSecond(10.0f);
			Stamina->ResetStamina();
			Stamina->TickStamina(1.0f);
			TestTrue(TEXT("depleted after first tick"), Stamina->IsDepleted());

			Stamina->TickStamina(5.0f);
			TestEqual(TEXT("still zero"), Stamina->GetCurrentStamina(), 0.0f);
			TestTrue(TEXT("still depleted"), Stamina->IsDepleted());
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS