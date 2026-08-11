// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareInventoryComponent.h"
#include "NightmarePickupActor.h"
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
				TestTrue(TEXT("fill"), Filler->TryCollectInto(Inv));
			}

			ANightmarePickupActor* Extra = NewObject<ANightmarePickupActor>(GetTransientPackage());
			TestFalse(TEXT("full reject"), Extra->TryCollectInto(Inv));
			TestFalse(TEXT("not collected"), Extra->IsCollected());
		});
	});
}

#endif