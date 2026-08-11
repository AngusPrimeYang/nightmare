// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareMatchComponent.h"
#include "NightmareStaminaComponent.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareMatchSpec,
	"Nightmare.Match",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareMatchSpec)

void FNightmareMatchSpec::Define()
{
	Describe("UNightmareMatchComponent", [this]()
	{
		It("wins after SurviveDuration when stamina is not depleted", [this]()
		{
			UNightmareMatchComponent* Match = NewObject<UNightmareMatchComponent>(GetTransientPackage());
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			Stamina->SetMaxStamina(100.0f);
			Stamina->SetStaminaDrainPerSecond(0.0f);
			Stamina->ResetStamina();

			Match->SetSurviveDurationSeconds(10.0f);
			Match->ResetMatch();
			Match->TickMatch(10.0f, Stamina);

			TestEqual(TEXT("won"), static_cast<uint8>(Match->GetMatchState()), static_cast<uint8>(ENightmareMatchState::Won));
		});

		It("fails when stamina is depleted before time is up", [this]()
		{
			UNightmareMatchComponent* Match = NewObject<UNightmareMatchComponent>(GetTransientPackage());
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			Stamina->SetMaxStamina(10.0f);
			Stamina->ResetStamina();
			Stamina->ApplyDelta(-10.0f);

			Match->SetSurviveDurationSeconds(100.0f);
			Match->ResetMatch();
			Match->TickMatch(1.0f, Stamina);

			TestEqual(TEXT("failed"), static_cast<uint8>(Match->GetMatchState()), static_cast<uint8>(ENightmareMatchState::Failed));
		});

		It("prefers Failed over Won when depleted and duration elapsed in Evaluate", [this]()
		{
			UNightmareMatchComponent* Match = NewObject<UNightmareMatchComponent>(GetTransientPackage());
			Match->SetSurviveDurationSeconds(5.0f);

			const ENightmareMatchState State = Match->Evaluate(true, 5.0f);
			TestEqual(TEXT("failed priority"), static_cast<uint8>(State), static_cast<uint8>(ENightmareMatchState::Failed));
		});
	});
}

#endif