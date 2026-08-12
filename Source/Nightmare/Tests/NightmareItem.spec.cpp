// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareItemInstance.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareItemSpec,
	"Nightmare.Item",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareItemSpec)

void FNightmareItemSpec::Define()
{
	Describe("UNightmareItemInstance", [this]()
	{
		It("initializes RemainingUses from MaxUses", [this]()
		{
			UNightmareItemInstance* Item = NewObject<UNightmareItemInstance>(GetTransientPackage());
			FNightmareItemDef Def;
			Def.ItemId = TEXT("Food");
			Def.MaxUses = 3;
			Def.StaminaDeltaOnUse = 15.0f;
			Item->InitializeFromDef(Def);

			TestEqual(TEXT("remaining"), Item->GetRemainingUses(), 3);
			TestTrue(TEXT("can use"), Item->CanUse());
		});

		It("TryUse returns stamina delta and decrements uses", [this]()
		{
			UNightmareItemInstance* Item = NewObject<UNightmareItemInstance>(GetTransientPackage());
			FNightmareItemDef Def;
			Def.MaxUses = 2;
			Def.EffectType = ENightmareItemEffectType::Stamina;
			Def.StaminaDeltaOnUse = -5.0f;
			Item->InitializeFromDef(Def);

			FNightmareItemUseResult Result;
			TestTrue(TEXT("first use"), Item->TryUse(Result));
			TestEqual(TEXT("negative delta"), Result.StaminaDelta, -5.0f);
			TestEqual(TEXT("uses left"), Item->GetRemainingUses(), 1);
		});

		It("TryUse fails when no uses remain", [this]()
		{
			UNightmareItemInstance* Item = NewObject<UNightmareItemInstance>(GetTransientPackage());
			FNightmareItemDef Def;
			Def.MaxUses = 1;
			Def.StaminaDeltaOnUse = 10.0f;
			Item->InitializeFromDef(Def);

			FNightmareItemUseResult Result;
			TestTrue(TEXT("consume last"), Item->TryUse(Result));
			TestFalse(TEXT("empty"), Item->TryUse(Result));
			TestEqual(TEXT("zero uses"), Item->GetRemainingUses(), 0);
		});

		It("TryUse fills speed payload for Speed effect", [this]()
		{
			UNightmareItemInstance* Item = NewObject<UNightmareItemInstance>(GetTransientPackage());
			FNightmareItemDef Def;
			Def.EffectType = ENightmareItemEffectType::Speed;
			Def.SpeedMultiplier = 1.5f;
			Def.EffectDuration = 4.0f;
			Def.MaxUses = 1;
			Item->InitializeFromDef(Def);

			FNightmareItemUseResult Result;
			TestTrue(TEXT("use"), Item->TryUse(Result));
			TestEqual(TEXT("type"), static_cast<uint8>(Result.EffectType), static_cast<uint8>(ENightmareItemEffectType::Speed));
			TestEqual(TEXT("speed"), Result.SpeedMultiplier, 1.5f);
			TestEqual(TEXT("dur"), Result.EffectDuration, 4.0f);
		});
	});
}

#endif
