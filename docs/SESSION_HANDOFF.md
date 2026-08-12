# Nightmare — 新 Session 精簡交接（2026-08-12）

> 開新 chat 時把本檔貼給 Agent，或說「依 docs/SESSION_HANDOFF.md 繼續」。

## 現況（一句）

P11 跳躍已過。契約已寫明：**背包滿（3 格）不可再撿**（規則／Spec 原本就有；Dev toast 已補強「Inventory full」）。**下一刀：P1**。

## 持有契約（剛確認）

- 上限 **3** 格；滿則 `TryCollectInto` 失敗，地上物**不消失**
- Spec：`Nightmare.Inventory`／`Nightmare.Pickup` 已鎖滿格拒絕

## 請你（可選）

- 關 Editor → 編譯後 PIE：撿滿 3 格再 E，應見 **Inventory full** toast
- 或直接說「做 P1」開刷物刀

細節：`docs/GAMEPLAY_SLICE.md` §1／§2／§7.2。
