# Automation Spec guidance (read on demand)

## Placement

```text
Source/Nightmare/Tests/<Feature>.spec.cpp
```

Gate Specs with `#if WITH_DEV_AUTOMATION_TESTS`. Keep filter prefixes stable under `Nightmare.*`.

**SETUP F:** at least one smoke Spec (e.g. `Nightmare.Smoke`) is **required** for behavior closed-loop readiness (`docs/SETUP_PHASES.md` phase F), not an optional add-on.

## Filter naming

| Area | Filter example |
|------|----------------|
| Health / damage | `Nightmare.Combat.Health` |
| Jump charges | `Nightmare.Movement.DoubleJump` |
| Smoke / module | `Nightmare.Smoke` |

`UE_TEST_FILTER` should stay a prefix (e.g. `Nightmare.`) so one gate run covers the project without engine noise.

## Prefer (stable, high ROI)

- Clampable numeric rules (damage, heal, min/max health, death threshold)
- Explicit state transitions (cooldown, charges reset on land — via API, not PIE)
- Pure functions / components creatable with `NewObject` in a Spec

## Defer (flaky / expensive)

- Full map load, input injection, animation notifies, physics contact timing
- “Feels right in PIE” checks — encode a formula or state flag instead

## Spec shape (Automation Spec)

```cpp
#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
// #include "YourComponent.h"

BEGIN_DEFINE_SPEC(FNightmareHealthSpec,
	"Nightmare.Combat.Health",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareHealthSpec)

void FNightmareHealthSpec::Define()
{
	Describe("UNightmareHealthComponent", [this]()
	{
		It("sets CurrentHealth from 100 to 50 after ApplyDamage(50)", [this]()
		{
			// NewObject → setup → act → TestEqual
		});
	});
}

#endif
```

Use real types from the turn’s codegen. Emit a **complete** `.spec.cpp` file for apply_codegen.

## Same-turn rule

If the turn changes core logic behavior, ship or update the matching Spec in the **same** turn before claiming done. If logic gate cannot run yet, still leave the Spec in tree so the gate is ready when UE is available.
