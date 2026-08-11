---
name: ue5-loop-engineering
description: >-
  Runs one Nightmare UE5 loop-engineering coding turn: full-file C++ codegen,
  apply_codegen, compile gate, Automation Spec logic gate (required for full
  dual-gate loop once SETUP F is done), bounded retries, Live Coding lock
  awareness, and C++/Blueprint split. Use when implementing or fixing Nightmare
  gameplay/C++ logic, writing Automation Specs, running apply_codegen or
  build_and_test, or when the user mentions loop engineering, validation gates,
  Live Coding conflicts, or Blueprint bridges.
---

# UE5 Loop Engineering (coding turn)

Project skill for **writing code inside an already-built loop**, not for installing UE or scaffolding the repo. Infra phases live in `docs/SETUP_PHASES.md`. Hard constraints live in `.cursorrules` and `.cursor/rules/ue5-loop.mdc`.

**Setup readiness:** compile closed loop = phase **E**; full Loop Engineering (dual gates, less human oracle) = phase **F** (Logic Gate + Spec) immediately after E. Phase **G** (hooks/CI) is optional and must not block F.

## When this applies

- Changing gameplay/core logic under `Source/Nightmare/`
- Adding or updating Automation Specs
- Running or interpreting `Scripts/apply_codegen.ps1` / `Scripts/build_and_test.ps1`

Skip this skill for pure docs, git-only, or SETUP phase D/E/F machine bring-up (until gates exist on that machine).

## Turn checklist (copy and track)

```
Turn:
- [ ] 0. Env: Editor closed or Live Coding not locking binaries (else STOP)
- [ ] 1. Contract: name the observable behavior under test
- [ ] 2. Code: complete C++ .h/.cpp (Blueprintable APIs OK); no std::string; no .uasset edits
- [ ] 3. Spec: if core logic changed → add/update Spec (same turn)
- [ ] 4. Apply: Scripts/apply_codegen.ps1 -SourcePath <staging>
- [ ] 5. Compile gate: Scripts/build_and_test.ps1 (read summary only)
- [ ] 6. Logic gate: same script with tests (required once SETUP F is available)
- [ ] 7. On fail: fix implementation only; compile OR same Spec filter ≤ 3 then STOP + ask user
```

## Non-negotiables

1. Full files only (or one complete replaceable function when updating a known file).
2. UE naming / `UPROPERTY` / `FString|FName|FText` — see rules.
3. **Tests are the contract**: fix game code to satisfy Specs. Do **not** weaken asserts or rename filters to go green unless the user explicitly changes the contract.
4. One failure cluster per turn: only files + Spec filter named in the latest summary.
5. **Max interrupt**: 3 consecutive compile failures **or** 3 failures on the same Spec filter → STOP and ask the user (no 4th blind attempt).
6. **No `.uasset` in the loop**: gameplay in C++; expose Blueprint hooks; humans wire assets in Editor.
7. Link/Live Coding lock ≠ bad game logic — do not “fix” source for environment locks.

## Progressive disclosure (read only what you need)

| Need | Read |
|------|------|
| Dual gates, CLI, summaries, retry STOP | [reference-gates.md](reference-gates.md) |
| What to Spec, filter names, Spec shape | [reference-specs.md](reference-specs.md) |
| Live Coding, token/interrupt, C++↔BP | [reference-pitfalls.md](reference-pitfalls.md) |

Default: follow the checklist. Open a reference file only when implementing Specs, wiring gates, handling a gate failure, or hitting an Editor/lock/BP question.

## Done criteria

Turn succeeds only when apply succeeded and compile gate passed, and — if logic gate ran — the scoped Spec filter is green. If logic gate is unavailable (SETUP F not done / no UE / `-SkipTests`), say so and stop after compile; do not invent pass/fail. Prefer completing SETUP F over normalizing compile-only turns as “full” Loop Engineering.
