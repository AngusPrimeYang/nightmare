// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareInventoryComponent.h"
#include "NightmareItemTypes.h"
#include "NightmarePickupActor.h"
#include "NightmarePlayerEffectComponent.h"
#include "NightmareStaminaComponent.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmarePickupSpec,
	"Nightmare.Pickup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmarePickupSpec)

void FNightmarePickupSpec::Define()
{
	Describe("ANightmarePickupActor", [this]()
	{
		It("TryCollectInto adds an item and marks collected", [this]()
		{
			UNightmareInventoryComponent* Inv = NewObject<UNightmareInventoryComponent>(GetTransientPackage());
			ANightmarePickupActor* Pickup = NewObject<ANightmarePickupActor>(GetTransientPackage());

			FNightmareItemDef Def;
			Def.ItemId = TEXT("Apple");
			Def.MaxUses = 2;
			Def.EffectType = ENightmareItemEffectType::Stamina;
			Def.InteractMode = ENightmareItemInteractMode::HoldToUse;
			Def.StaminaDeltaOnUse = 8.0f;
			Pickup->SetItemDef(Def);

			TestTrue(TEXT("collect"), Pickup->TryCollectInto(Inv));
			TestTrue(TEXT("flag"), Pickup->IsCollected());
			TestEqual(TEXT("occupied"), Inv->CountOccupied(), 1);
			TestFalse(TEXT("second collect"), Pickup->TryCollectInto(Inv));
		});

		It("TryCollectInto fails when inventory is full", [this]()
		{
			UNightmareInventoryComponent* Inv = NewObject<UNightmareInventoryComponent>(GetTransientPackage());
			for (int32 i = 0; i < 3; ++i)
			{
				ANightmarePickupActor* Filler = NewObject<ANightmarePickupActor>(GetTransientPackage());
				FNightmareItemDef Def;
				Def.InteractMode = ENightmareItemInteractMode::HoldToUse;
				Def.EffectType = ENightmareItemEffectType::Stamina;
				Filler->SetItemDef(Def);
				TestTrue(TEXT("fill"), Filler->TryCollectInto(Inv));
			}

			ANightmarePickupActor* Extra = NewObject<ANightmarePickupActor>(GetTransientPackage());
			FNightmareItemDef Def;
			Def.InteractMode = ENightmareItemInteractMode::HoldToUse;
			Extra->SetItemDef(Def);
			TestFalse(TEXT("full reject"), Extra->TryCollectInto(Inv));
			TestFalse(TEXT("not collected"), Extra->IsCollected());
		});

		It("TryCollectInto rejects TouchInstant items", [this]()
		{
			UNightmareInventoryComponent* Inv = NewObject<UNightmareInventoryComponent>(GetTransientPackage());
			ANightmarePickupActor* Pickup = NewObject<ANightmarePickupActor>(GetTransientPackage());
			FNightmareItemDef Def;
			Def.InteractMode = ENightmareItemInteractMode::TouchInstant;
			Def.EffectType = ENightmareItemEffectType::Stamina;
			Pickup->SetItemDef(Def);
			TestFalse(TEXT("no collect"), Pickup->TryCollectInto(Inv));
			TestFalse(TEXT("not collected"), Pickup->IsCollected());
		});

		It("TryTouchApply applies stamina and collects", [this]()
		{
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			Stamina->SetMaxStamina(100.0f);
			Stamina->ResetStamina();
			ANightmarePickupActor* Pickup = NewObject<ANightmarePickupActor>(GetTransientPackage());
			FNightmareItemDef Def;
			Def.InteractMode = ENightmareItemInteractMode::TouchInstant;
			Def.EffectType = ENightmareItemEffectType::Stamina;
			Def.StaminaDeltaOnUse = -15.0f;
			Pickup->SetItemDef(Def);

			TestTrue(TEXT("touch"), Pickup->TryTouchApply(Stamina, nullptr, nullptr));
			TestEqual(TEXT("stamina"), Stamina->GetCurrentStamina(), 85.0f);
			TestTrue(TEXT("collected"), Pickup->IsCollected());
		});

		It("TryTouchApply applies speed effect", [this]()
		{
			UNightmarePlayerEffectComponent* Effects = NewObject<UNightmarePlayerEffectComponent>(GetTransientPackage());
			ANightmarePickupActor* Pickup = NewObject<ANightmarePickupActor>(GetTransientPackage());
			FNightmareItemDef Def;
			Def.InteractMode = ENightmareItemInteractMode::TouchInstant;
			Def.EffectType = ENightmareItemEffectType::Speed;
			Def.SpeedMultiplier = 1.4f;
			Def.EffectDuration = 2.5f;
			Pickup->SetItemDef(Def);

			TestTrue(TEXT("touch speed"), Pickup->TryTouchApply(nullptr, Effects, nullptr));
			TestTrue(TEXT("active"), Effects->HasActiveSpeedEffect());
			TestEqual(TEXT("dur"), Effects->GetSpeedTimeRemaining(), 2.5f);
		});
	});
}

#endif
