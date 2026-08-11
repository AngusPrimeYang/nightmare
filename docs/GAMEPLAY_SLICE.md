# Nightmare — 玩法契約（第一垂直切片）

> 更新日期：2026-08-11  
> 用途：追蹤「做了什麼／做到哪」；行為對錯以 Automation Spec（`Nightmare.*`）為準，本頁只記規則與進度。  
> 不綁鏡頭／類型（第一人稱、第三人稱、橫版等之後再掛呈現層）。

---

## 1. 一局規則（產品契約）

玩家有**體力**。場上有可互動物品，可**撿拾**、**持有**，且有**使用次數上限**。使用物品會**增加或減少**體力。  
體力**歸零 = 失敗**。在**固定時限內活著 = 獲勝**。  
物品會在**不固定間隔**刷新出現。體力會隨時間下降；若不靠物品維持，最終會歸零失敗。

```text
時間流逝 → 體力自然下降
     ↓
撿物 → 持有 → 消耗次數使用 → 體力 ±
     ↓
≤0 = Failed ｜ 撐過 SurviveDuration 且體力 >0 = Won
     +
[MinSpawnInterval, MaxSpawnInterval] 隨機刷物
```

---

## 2. v1 已拍板決策

| 決策 | v1 選擇 | 備註 |
|------|---------|------|
| 持有格數 | **3** | 多格／快捷欄延後 |
| 體力下降 | **全局持續消耗**（每秒或固定 tick） | 「不使用物品就會死」= 不補就會被消耗打穿，不另做「未使用加速」 |
| 物品 delta | 可正可負 | 風險／報酬；負值不是 bug |
| 刷物節奏 | 隨機區間 `[Min, Max]` 秒 | Spec 可注入時間／假隨機 |
| 同時勝負 | **Failed 優先** | 同一幀體力 ≤0 與時限到達 → Failed |
| 呈現 | 未定 | Dev Map + 預設 Pawn 僅煙霧；不鎖人稱 |

數值表（實作時以 C++ / Spec 為準，此處為起始假設，可調）：

| 參數 | 起始假設 |
|------|----------|
| `MaxStamina` | 100 |
| `StaminaDrainPerSecond` | > 0（實作預設 5） |
| `SurviveDurationSeconds` | 120 |
| 物品 `MaxUses` | ≥ 1 |
| 物品 `StaminaDeltaOnUse` | 可正可負 |
| Inventory slots | 3 |
| Spawn interval | 預設 3–8 秒 |

---

## 3. 系統清單與進度

| 系統 | Spec filter | 狀態 | 說明 |
|------|-------------|------|------|
| 基建煙霧 | `Nightmare.Smoke` | `[x]` | Logic Gate 可跑（2026-08-11） |
| 體力 | `Nightmare.Stamina` | `[x]` | `UNightmareStaminaComponent` |
| 物品定義＋使用次數 | `Nightmare.Item` | `[x]` | `FNightmareItemDef` + `UNightmareItemInstance` |
| 持有（3 格） | `Nightmare.Inventory` | `[x]` | `UNightmareInventoryComponent` |
| 勝負 | `Nightmare.Match` | `[x]` | `UNightmareMatchComponent`（Failed 優先） |
| 刷物排程 | `Nightmare.Spawn` | `[x]` | `UNightmareSpawnScheduler` |
| 互動 Actor（C++） | `Nightmare.Pickup` | `[x]` | `ANightmarePickupActor::TryCollectInto`；**關卡／網格仍為人工作業** |

**完成定義：** 上表 Spec 綠 + 本頁勾選。Dev Map / Pawn 視覺組裝見 §4，不算本表阻塞。

---

## 4. 建議實作順序

1. 階段 **F**：**完成**  
2. 規則層 `Stamina` → `Item` → `Inventory` → `Match` → `Spawn` → `Pickup`：**完成**  
3. **下一刀（人工）：** Editor 組裝 — 追蹤清單見 **`docs/EDITOR_DEV_SETUP.md`**（步驟 1–9）  
4. 再決定鏡頭／類型與美術換皮  

---

## 5. 刻意不做（本切片）

- Speckit／重型規格工具（單頁契約 + Spec + git 足夠）  
- 多人、大逃殺縮圈、精美關卡／動畫  
- 在迴圈裡改 Blueprint 資產當玩法來源  

---

## 6. 與基建文件的關係

| 文件 | 管什麼 |
|------|--------|
| `docs/SETUP_PHASES.md` | 環境／編譯／行為閉環基建 |
| **本頁** | 玩法規則與系統進度 |
| `docs/EDITOR_DEV_SETUP.md` | Editor 組裝步驟 1–9（人工／可選 MCP） |
| `docs/UNREAL_MCP.md` | 專案級 chongdashu Unreal MCP 啟用清單 |
| `Source/Nightmare/Tests/*.spec.cpp` | 可執行行為契約 |
| `Saved/LoopEngineering/*` | 單次編譯／測試 summary（本機） |
