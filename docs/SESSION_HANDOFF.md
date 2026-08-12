# Nightmare — 新 Session 精簡交接（2026-08-12）

> 開新 chat 時把本檔貼給 Agent，或說「依 docs/SESSION_HANDOFF.md 繼續」。

## 現況（一句）

P11 跳躍已過。契約補記：**持有滿 3 格不可再撿**（規則層／Spec 已有；Dev toast 已加「Inventory full」）。**下一刀：P1**。

## 持有規則（已拍板）

- 上限 **3** 格；滿則 `TryCollectInto` 失敗，地上物保留
- 用掉／清出空格後才可再撿

## 請你（P1 或編譯 toast）

1. 若要看到滿格 toast：關 Editor → 編譯 → 撿滿三格再 E  
2. 說「做 P1」開始刷物呈現＋生成時 roll
