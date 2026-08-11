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
			Def.StaminaDeltaOnUse = -5.0f;
			Item->InitializeFromDef(Def);

			float Delta = 0.0f;
			TestTrue(TEXT("first use"), Item->TryUse(Delta));
			TestEqual(TEXT("negative delta"), Delta, -5.0f);
			TestEqual(TEXT("uses left"), Item->GetRemainingUses(), 1);
		});

		It("TryUse fails when no uses remain", [this]()
		{
			UNightmareItemInstance* Item = NewObject<UNightmareItemInstance>(GetTransientPackage());
			FNightmareItemDef Def;
			Def.MaxUses = 1;
			Def.StaminaDeltaOnUse = 10.0f;
			Item->InitializeFromDef(Def);

			float Delta = 0.0f;
			TestTrue(TEXT("consume last"), Item->TryUse(Delta));
			TestFalse(TEXT("empty"), Item->TryUse(Delta));
			TestEqual(TEXT("zero uses"), Item->GetRemainingUses(), 0);
		});
	});
}

#endif