# Nightmare — Editor 開發組裝追蹤（步驟 1–9）

> 更新日期：2026-08-12  
> 目的：把「規則層已完成」之後、在 **Unreal Editor** 要做的人工組裝收成一頁，方便勾選追蹤。  
> 狀態：`[x]` 已完成　`[ ]` 待執行　`[~]` 可選／替代路徑　`[—]` 僅說明、無需勾選  
> 相關：規則進度見 `docs/GAMEPLAY_SLICE.md`；灰盒／玩法延伸待辦見同文件 **§7**；基建閉環見 `docs/SETUP_PHASES.md`；交接見 `docs/SESSION_HANDOFF.md`。

---

## 總覽進度

| # | 步驟 | 狀態 | 備註 |
|---|------|------|------|
| 1 | 搞懂名詞 | `[~]` | Outliner／Details／Content Browser 已用過 |
| 2 | 打開 Editor | `[x]` | |
| 3 | 關卡 | `[x]` | `Content/Map/L_Dev.umap` |
| 4 | 地板／燈／Player Start | `[~]` | 沿用 L_Dev／Open World；有 PlayerStart |
| 5 | 建 `BP_DevPawn` 並掛 Component | `[~]` | Character 版 BP 曾重建；**現行玩法改走 C++ `NightmareDevCharacter`**（BP 可留作參考） |
| 6 | 讓玩家控制該 Pawn | `[x]` | `NightmareDevGameMode` 預設 Pawn；關卡已無搶控制的 `BP_DevPawn_Dev` |
| 7 | 放置 `NightmarePickupActor` | `[x]` | 關卡已放 `GrayPickup_A/B/C`（旋轉灰盒） |
| 8 | PIE 煙霧驗證 | `[x]` | 可 WASD／滑鼠；第三人稱臂相機；Dev 螢幕字 HUD |
| 9 | Tick／撿物橋接 | `[x]` 路徑 B | Tick + E／F／1–3／Space；滿格 E 顯示 Inventory full（toast 待編譯） |

**目前焦點：** **P1** 刷物呈現＋生成時 roll。P11 跳躍已過。滿格不可再撿（契約已寫；Spec 已有）。

---

## 步驟 1 — 搞懂名詞 `[—]`

| 詞 | 白話 |
|----|------|
| **Unreal Editor** | 有視窗的 Unreal，用來開專案、擺關卡、按 ▶ 試玩（不是 Cursor） |
| **專案** | `Nightmare.uproject` 所在的遊戲資料夾 |
| **關卡／地圖（Level）** | 場景，存成 `.umap` |
| **Actor** | 能放進關卡的東西；`ANightmarePickupActor` 是 Actor |
| **Component** | 掛在 Actor 上的功能零件；體力／背包／勝負是 Component，**不能單獨丟在空地** |
| **Blueprint（BP）** | 編輯器裡組裝的子類，方便掛組件、調數值 |
| **PIE** | Play In Editor（編輯器內播放） |
| **Content Browser** | **資源資料夾面板**（通常在視窗下方）。看不到就：**Window → Content Browser**。這裡有 `Blueprints`、`Map` 等檔案；**拖進關卡**才會在 Outliner 出現實例 |
| **World Outliner** | 本關卡 Actor 清單 |
| **Details** | 選中物件的屬性面板 |

```text
關卡（Level）
  └── Actor（玩家、地上物品）
        └── Component（Stamina / Inventory / Match…）
```

---

## 步驟 2–4 — 關卡準備 `[x]`／`[~]`

見總覽：`L_Dev` 可開、有 PlayerStart；燈光／地板可沿用。

---

## 步驟 5–6 — Pawn／控制 `[x]`（路徑 B）

勿再依賴關卡內 Auto Possess 的舊 BP 搶控制。預設：`ANightmareDevGameMode` → `ANightmareDevCharacter`。

---

## 步驟 7 — Pickup `[x]`

關卡實例：`GrayPickup_A`／`B`／`C`（`ANightmarePickupActor` 旋轉灰盒）。走近 **E** 撿入三格背包。

---

## 步驟 8 — PIE 煙霧 `[x]`

| 已確認 | 備註 |
|--------|------|
| WASD／滑鼠 | Enhanced Input（程式建立 IMC） |
| 第三人稱灰盒角色 | GrayboxBody／Head |
| Dev HUD | 體力／背包／Match／操作提示 |

步驟 8 通過 =「組裝看得見」；不等于「一局可玩」。

---

## 步驟 9 — 可玩橋接（Tick／撿物） `[x]` 路徑 B

規則層 API 需被呼叫才會動（Spec 用注入時間；預設 `bCanEverTick=false`）。

### 路徑 A — 藍圖橋接（人工） `[ ]`（未採用）

在 `BP_DevPawn`：

1. `[ ]` Event Tick → 呼叫 Stamina 的 **Tick Stamina**（Delta Seconds）
2. `[ ]` Event Tick → 呼叫 Match 的 **Tick Match**（Delta Seconds + Stamina 組件）
3. `[ ]` 按鍵（例如 E）→ 對附近 Pickup 呼叫 **Try Collect Into**（傳入自己的 Inventory）
4. `[ ]` 按鍵呼叫 Inventory **Try Use Slot**（選中格 + Stamina）

### 路徑 B — C++ DevCharacter／DevGameMode（Loop Engineering） `[x]`

已落地：`ANightmareDevCharacter` + `ANightmareDevGameMode`；`DefaultEngine.ini` → `GlobalDefaultGameMode`。  
輸入：WASD／滑鼠、**E** 撿、**1／2／3** 選格、**F** 用選中格（一次一格）。  
Dev HUD：體力、三格（`>n<` 標示選中）、Match、toast。

### 路徑 C — Editor MCP 協助組裝 `[~]`

| 選項 | 狀態 |
|------|------|
| Epic 官方 Unreal MCP（5.8 Experimental） | `[ ]` 未接上 Cursor |
| **chongdashu/unreal-mcp（專案級）** | `[x]` 已通；協助過 BP／刪錯誤實例；Character parent／輸入最終以 C++ 路徑 B 為準 |
| 其他社群（如 db-lyon/ue-mcp） | `[ ]` 未安裝 |

MCP 可協助步驟 3–7 的「點 Editor」；**不能取代** Spec／雙閘門。

**步驟 9 完成判準：** PIE 左上角可見體力／背包／Match；E 撿後背包更新；**1／2／3** 切格後 **F** 只耗用選中格並改體力；超時／歸零 Match 變 WON／FAILED。

---

## 與 Loop Engineering 的分工

| 已完成（C++／Spec／閘門） | 本文件追蹤（Editor／橋接） |
|---------------------------|----------------------------|
| Stamina / Item / Inventory / Match / Spawn / Pickup 規則 | 步驟 2–9 組裝與煙霧 |
| `Nightmare.*` Automation 綠燈 | PIE 讓規則「看得見」 |
| 不在迴圈改 `.uasset` | `.umap`／Blueprint 由人（或 MCP）組 |
| 玩法延伸 backlog | 見 `GAMEPLAY_SLICE` §7（P10／G1／P1…） |

---

## 勾選紀錄（可手寫日期）

| 日期 | 完成至步驟 | 備註 |
|------|------------|------|
| 2026-08-12 | 7–9B + P10 + G1 | 選格／加油站（`GS_*`+燈）過；下一刀 **P1** |
| 2026-08-12 | 8 + 9B | PIE 可移動；C++ DevCharacter／GameMode |

---

## 變更紀錄

| 日期 | 說明 |
|------|------|
| 2026-08-12 | G1 改多實例還原；焦點 **P1**；勿巨型 BP |
| 2026-08-12 | G1 `GasStationGraybox` @ 原點；焦點曾改 P1 |
| 2026-08-12 | P10 PIE 過；焦點曾改 G1 加油站灰盒 |
| 2026-08-12 | PIE 移動通過；步驟 6／8／9B 勾完；曾卡步驟 7 Pickup |
| 2026-08-12 | 勘誤：步驟 5 MCP BP 非 Character；總覽改回待重做；標明 8 曾跳步試 PIE |
| 2026-08-11 | 初版：彙整先前說明步驟 1–9，全部標待執行以便追蹤 |
