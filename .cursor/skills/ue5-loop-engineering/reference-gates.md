# Validation gates (read on demand)

## Gate map

| Gate | Command | Blocks | On failure |
|------|---------|--------|------------|
| Apply | `Scripts/apply_codegen.ps1 -SourcePath <staging>` | Incomplete / rejected files | Emit complete files; re-apply |
| Compile | `Scripts/build_and_test.ps1` (or `-SkipTests`) | UBT / link errors | Fix compile only; do not edit Specs |
| Logic | `build_and_test.ps1` with tests enabled | Spec assert failures | Fix **implementation** only |

Exit code `0` means every gate that actually ran succeeded.

**Product requirement:** full Loop Engineering needs **both** Compile and Logic gates (`docs/SETUP_PHASES.md` phase **F**, after **E**). Compile-only is a temporary bring-up state, not the target dual-gate loop. Do not treat Hook/CI (phase **G**) as a substitute for Logic Gate.

## Env / flags (intended contract)

From process env or `ue.local.env` (see `ue.local.env.example`):

| Variable / flag | Role |
|-----------------|------|
| `UE_ROOT` | Engine root (required to build/run) |
| `UE_TEST_FILTER` | Default `Nightmare.` — never run the whole engine suite |
| `UE_RUN_TESTS` | `1` = run logic gate after successful build |
| `-SkipTests` | Compile only (fast iterate / SETUP E only — not a substitute for completing F) |

If the script does not yet expose RunTests (SETUP F incomplete), treat logic gate as **unavailable**: report that, do not fake results. Prefer extending `build_and_test.ps1` over ad-hoc one-off Editor Cmd in chat. Completing SETUP F is required before claiming dual-gate readiness.

## Editor Cmd shape (when scripting RunTests)

```text
UnrealEditor-Cmd.exe <uproject>
  -ExecCmds="Automation RunTests <UE_TEST_FILTER>; Quit"
  -unattended -nop4 -nosound -NullRHI
```

Use Editor target (`UE_TARGET=NightmareEditor`). Log under `Saved/LoopEngineering/`.

## Summaries for the agent

Prefer reading summaries, not raw multi‑MB logs (token control):

- `Saved/LoopEngineering/build_*.summary.txt` — compile errors (first lines)
- `Saved/LoopEngineering/test_*.summary.txt` — failed Spec names + assert text (when present)

Do not paste entire UBT logs into the turn context.

## Retry / anti-dead-loop

1. **Compile gate**: attempt 1..3 on the same turn; after **3** consecutive failures → **STOP** and ask the user (macro/UHT/toolchain sinkholes).
2. **Logic gate**: same Spec filter → attempt 1..3: change implementation only; keep contract. After **3** → **STOP**. Report expected vs actual, hypotheses tried, design question.
3. Do not expand scope (no drive-by refactors) while red.
4. Flaky World/input/animation tests → stop and narrow to pure logic Specs (see [reference-specs.md](reference-specs.md)); do not burn retries on timing flakes.
5. Suspected Live Coding / file-lock link failures → **STOP** immediately (do not spend the 3 compile retries rewriting code). See [reference-pitfalls.md](reference-pitfalls.md).
