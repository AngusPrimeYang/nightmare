// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareItemRoller.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareItemRollSpec,
	"Nightmare.ItemRoll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareItemRollSpec)

void FNightmareItemRollSpec::Define()
{
	Describe("UNightmareItemRoller", [this]()
	{
		It("rolls deterministic defs for a fixed seed", [this]()
		{
			UNightmareItemRoller* Roller = NewObject<UNightmareItemRoller>(GetTransientPackage());
			Roller->SetRandomSeed(123);
			const FNightmareItemDef A = Roller->RollItemDef();
			Roller->SetRandomSeed(123);
			const FNightmareItemDef B = Roller->RollItemDef();
			TestEqual(TEXT("id"), A.ItemId, B.ItemId);
			TestEqual(TEXT("effect"), static_cast<uint8>(A.EffectType), static_cast<uint8>(B.EffectType));
			TestEqual(TEXT("mode"), static_cast<uint8>(A.InteractMode), static_cast<uint8>(B.InteractMode));
			TestEqual(TEXT("stamina"), A.StaminaDeltaOnUse, B.StaminaDeltaOnUse);
		});

		It("each roll has exactly one effect type and one interact mode", [this]()
		{
			UNightmareItemRoller* Roller = NewObject<UNightmareItemRoller>(GetTransientPackage());
			Roller->SetRandomSeed(7);
			bool bSawTouch = false;
			bool bSawHold = false;
			bool bSawStamina = false;
			bool bSawSpeed = false;
			bool bSawJump = false;
			for (int32 i = 0; i < 64; ++i)
			{
				const FNightmareItemDef Def = Roller->RollItemDef();
				TestTrue(TEXT("max uses"), Def.MaxUses >= 1);
				if (Def.InteractMode == ENightmareItemInteractMode::TouchInstant) { bSawTouch = true; }
				if (Def.InteractMode == ENightmareItemInteractMode::HoldToUse) { bSawHold = true; }
				if (Def.EffectType == ENightmareItemEffectType::Stamina) { bSawStamina = true; }
				if (Def.EffectType == ENightmareItemEffectType::Speed) { bSawSpeed = true; }
				if (Def.EffectType == ENightmareItemEffectType::Jump) { bSawJump = true; }
			}
			TestTrue(TEXT("saw touch"), bSawTouch);
			TestTrue(TEXT("saw hold"), bSawHold);
			TestTrue(TEXT("saw stamina"), bSawStamina);
			TestTrue(TEXT("saw speed"), bSawSpeed);
			TestTrue(TEXT("saw jump"), bSawJump);
		});
	});
}

#endif
