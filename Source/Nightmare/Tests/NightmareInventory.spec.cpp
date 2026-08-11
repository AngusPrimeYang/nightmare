// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareInventoryComponent.h"
#include "NightmareItemInstance.h"
#include "NightmareStaminaComponent.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareInventorySpec,
	"Nightmare.Inventory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareInventorySpec)

static UNightmareItemInstance* MakeItem(UObject* Outer, int32 Uses, float Delta)
{
	UNightmareItemInstance* Item = NewObject<UNightmareItemInstance>(Outer);
	FNightmareItemDef Def;
	Def.ItemId = TEXT("Test");
	Def.MaxUses = Uses;
	Def.StaminaDeltaOnUse = Delta;
	Item->InitializeFromDef(Def);
	return Item;
}

void FNightmareInventorySpec::Define()
{
	Describe("UNightmareInventoryComponent", [this]()
	{
		It("holds up to 3 items then rejects when full", [this]()
		{
			UNightmareInventoryComponent* Inv = NewObject<UNightmareInventoryComponent>(GetTransientPackage());
			TestEqual(TEXT("slot count"), Inv->GetSlotCount(), 3);

			TestEqual(TEXT("slot0"), Inv->TryAddItem(MakeItem(Inv, 1, 5.0f)), 0);
			TestEqual(TEXT("slot1"), Inv->TryAddItem(MakeItem(Inv, 1, 5.0f)), 1);
			TestEqual(TEXT("slot2"), Inv->TryAddItem(MakeItem(Inv, 1, 5.0f)), 2);
			TestEqual(TEXT("full"), Inv->TryAddItem(MakeItem(Inv, 1, 5.0f)), static_cast<int32>(INDEX_NONE));
			TestTrue(TEXT("is full"), Inv->IsFull());
			TestEqual(TEXT("occupied"), Inv->CountOccupied(), 3);
		});

		It("TryUseSlot applies stamina delta and clears spent item", [this]()
		{
			UNightmareInventoryComponent* Inv = NewObject<UNightmareInventoryComponent>(GetTransientPackage());
			UNightmareStaminaComponent* Stamina = NewObject<UNightmareStaminaComponent>(GetTransientPackage());
			Stamina->SetMaxStamina(100.0f);
			Stamina->ResetStamina();

			Inv->TryAddItem(MakeItem(Inv, 1, -20.0f));
			TestTrue(TEXT("use"), Inv->TryUseSlot(0, Stamina));
			TestEqual(TEXT("stamina"), Stamina->GetCurrentStamina(), 80.0f);
			TestNull(TEXT("cleared"), Inv->GetSlotItem(0));
		});

		It("ClearAll empties every slot", [this]()
		{
			UNightmareInventoryComponent* Inv = NewObject<UNightmareInventoryComponent>(GetTransientPackage());
			Inv->TryAddItem(MakeItem(Inv, 1, 1.0f));
			Inv->TryAddItem(MakeItem(Inv, 1, 1.0f));
			Inv->ClearAll();
			TestEqual(TEXT("empty"), Inv->CountOccupied(), 0);
		});
	});
}

#endif