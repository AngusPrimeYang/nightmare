# Practical pitfalls (read on demand)

## 1. Live Coding vs UBT (false “code is wrong”)

Auto compile via `Scripts/build_and_test.ps1` uses UBT/`link.exe`. If Unreal Editor has **Live Coding** (or Hot Reload) locking binaries, the link step fails even when C++ is fine.

**Do:**

- Prefer **closing the Editor** before an automated compile/fix loop.
- If Editor must stay open: disable Live Coding for that session / avoid triggering Live Coding compile; do not fight a locked PDB/DLL.
- Treat link errors that mention file in use, permission, or Live Coding / hot-reload locks as **environment conflicts**, not logic bugs.

**Do not:**

- “Fix” random source to clear a lock-related link failure.
- Burn compile retries on lock noise — **STOP** and ask the user to close Editor or disable Live Coding, then re-run the gate.

## 2. Token cost / max interrupt (compile)

UE build logs are huge (headers, macro spam). Always prefer `Saved/LoopEngineering/build_*.summary.txt` (and short excerpts), not full logs.

**Hard stop:**

- Same turn, **compile gate** fails **3** times in a row → **STOP immediately** and ask the user.
- Do not start a 4th blind compile-fix cycle (especially on UHT/macro/toolchain mysteries).
- Same bound already applies to Spec filter failures (see [reference-gates.md](reference-gates.md)).

When stopping, report: attempt count, summary excerpt, whether failure looks like code vs lock/toolchain.

## 3. Blueprint ↔ C++ (AI cannot edit `.uasset`)

Loop engineering iterates **text** (`Source/**`, configs). Binary Blueprints (`.uasset`) are **out of scope** for AI apply_codegen.

**Do:**

- Put gameplay rules in **C++** base classes / components.
- Mark classes `UCLASS(Blueprintable)` (or `BlueprintType` where appropriate).
- Expose tunables and hooks with `UPROPERTY(BlueprintReadOnly/BlueprintReadWrite)` and `UFUNCTION(BlueprintCallable/BlueprintImplementableEvent/BlueprintNativeEvent)` as needed.
- Let humans assemble assets, animation, and Blueprint graphs in Editor.

**Do not:**

- Promise to “fix the Blueprint asset” in the loop.
- Push core damage/jump/inventory rules only into Blueprint graphs the AI cannot regenerate.
- Edit or invent binary `.uasset` contents in codegen staging.
