# Nightmare — Editor 開發組裝追蹤（步驟 1–9）

> 更新日期：2026-08-12  
> 目的：把「規則層已完成」之後、在 **Unreal Editor** 要做的人工組裝收成一頁，方便勾選追蹤。  
> 狀態：`[x]` 已完成　`[ ]` 待執行　`[~]` 可選／替代路徑　`[—]` 僅說明、無需勾選  
> 相關：規則進度見 `docs/GAMEPLAY_SLICE.md`；灰盒／玩法延伸待辦見同文件 **§7**；基建閉環見 `docs/SETUP_PHASES.md`。

---

## 總覽進度

| # | 步驟 | 狀態 | 備註 |
|---|------|------|------|
| 1 | 搞懂名詞 | `[—]` | 閱讀即可 |
| 2 | 打開 Editor | `[ ]` | 雙擊 `.uproject` |
| 3 | 關卡（空白或沿用 Open World） | `[ ]` | 見步驟 3 二選一 |
| 4 | 地板／燈／Player Start | `[ ]` | Open World 可能已有部分 |
| 5 | 建 `BP_DevPawn` 並掛 Component | `[ ]` | |
| 6 | 讓玩家控制該 Pawn | `[ ]` | Auto Possess 或 GameMode |
| 7 | 放置 `NightmarePickupActor` | `[ ]` | |
| 8 | PIE 煙霧驗證 | `[ ]` | 能站關卡、Outliner 看得到即可 |
| 9 | （延伸）自動 Tick／撿物橋接 | `[ ]` | 否則體力不會自動掉、不會自動撿 |

**目前卡點：** 步驟 2 起皆為待執行（本機人工／可選之後用官方或社群 MCP 協助 Editor）。

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
| **Content Browser** | 資源瀏覽器 |
| **World Outliner** | 本關卡 Actor 清單 |
| **Details** | 選中物件的屬性面板 |

```text
關卡（Level）
  └── Actor（玩家、地上物品）
        └── Component（Stamina / Inventory / Match…）
```

---

## 步驟 2 — 打開 Editor `[ ]`

**待執行：**

1. `[ ]` 確認沒有殘留 `UnrealEditor` 鎖住二進位（若剛編譯過，先關乾淨再開）
2. `[ ]` 雙擊 `D:\DDD\nightmare\Nightmare.uproject`（或 Epic Launcher 用 UE 5.8 開啟）
3. `[ ]` 等到出現中央 3D 視窗與上方選單（File / Edit / Window…）

**說明：** 專案預設 `EditorStartupMap` 指向引擎 **Open World** 模板（見 `Config/DefaultEngine.ini`），一進來看到大世界**不是錯覺**。

---

## 步驟 3 — 關卡準備 `[ ]`

可二選一（勾一個即可推進）：

### 路徑 A — 沿用現有 Open World（較快） `[ ]`

1. `[ ]` 直接使用開啟後的 Open World 當沙盒
2. `[ ]` **強烈建議：** File → **Save Current Level As…** → `Content/Maps/L_Dev`（避免只改到引擎模板路徑）

### 路徑 B — 新建空白關卡（較單純） `[ ]`

1. `[ ]` File → New Level… → **Empty Level**
2. `[ ]` File → Save Current Level As… → `Content/Maps/L_Dev`
3. `[ ]`（可選）Edit → Project Settings → Maps & Modes：把 Editor Startup Map / Game Default Map 設成 `L_Dev`

**完成判準：** 有一份專案內可存的關卡（建議 `Content/Maps/L_Dev.umap`）。

---

## 步驟 4 — 關卡基本物件 `[ ]`

空關卡必做；Open World 若已有地形／光，可略過重複項並勾選「已確認足夠」。

1. `[ ]` 地板：Place Actors → Cube 或 Plane，Scale 放大當地面（或確認 Open World 可站）
2. `[ ]` 燈：Directional Light（或確認場景已夠亮）
3. `[ ]` **Player Start** 拖進場景（出生點）

**完成判準：** 按 ▶ 不會立刻掉虛空／全黑（至少有站立處與出生點）。

---

## 步驟 5 — 建立 `BP_DevPawn` 並掛 Component `[ ]`

Component **不能**直接拖到空地；要掛在玩家 Actor 上。

1. `[ ]` Content Browser → 建資料夾如 `Content/Blueprints`
2. `[ ]` 右鍵 → Blueprint Class → 父類選 **Character** → 命名 `BP_DevPawn`
3. `[ ]` 開啟 `BP_DevPawn` → Components → **Add**：
   - `[ ]` `NightmareStamina`（`UNightmareStaminaComponent`）
   - `[ ]` `NightmareInventory`（`UNightmareInventoryComponent`）
   - `[ ]` `NightmareMatch`（`UNightmareMatchComponent`）
4. `[ ]` Compile + Save

**說明：** `UNightmareSpawnScheduler` 是 UObject，不是 ActorComponent，**不要**用 Add Component 掛；刷物之後由 GameMode／邏輯 `NewObject` 使用即可。

**完成判準：** `BP_DevPawn` 的 Components 列表看得到上述三個 Nightmare 組件。

---

## 步驟 6 — 讓玩家控制該 Pawn `[ ]`

二選一：

### 方法 A — 關卡內 Auto Possess（煙霧最快） `[ ]`

1. `[ ]` 把 `BP_DevPawn` 拖進關卡
2. `[ ]` Details → Auto Possess Player → **Player 0**

### 方法 B — GameMode 預設 Pawn（較正規） `[ ]`

1. `[ ]` 建 `BP_DevGameMode`（父類 Game Mode Base）
2. `[ ]` Default Pawn Class = `BP_DevPawn`
3. `[ ]` Project Settings → Maps & Modes → Default GameMode = `BP_DevGameMode`  
   或本關 World Settings → GameMode Override

**完成判準：** PIE 時控制的是 `BP_DevPawn`（可在 Outliner 確認）。

---

## 步驟 7 — 放置 PickupActor `[ ]`

1. `[ ]` Place Actors／Content → **C++ Classes → Nightmare** → 找到 **NightmarePickupActor**
2. `[ ]` 拖 midair 到地板上（可多放幾個）
3. `[ ]` Details → **Item Def**：設 Item Id、Max Uses、Stamina Delta On Use（可正可負）
4. `[ ]`（可選）建 `BP_Pickup` 父類為 NightmarePickupActor，加 Static Mesh 才看得見

**完成判準：** Outliner 有 Pickup；Details 看得到 Item Def。

---

## 步驟 8 — PIE 煙霧驗證 `[ ]`

1. `[ ]` 按 ▶（PIE）
2. `[ ]` 能出現在關卡／能移動（Character）
3. `[ ]` Outliner 或暫停後確認：Pawn 上有 Stamina／Inventory／Match；地上有 Pickup

**此時預期（重要）：**

| 已掛上 | 尚未自動發生（見步驟 9） |
|--------|--------------------------|
| 組件存在、數值可在 Details 看 | 體力**不會**自動每幀下降 |
| Pickup 在關卡裡 | **不會**走近就撿（尚未呼叫 `TryCollectInto`） |
| Match 組件存在 | **不會**自動計時勝負 |

步驟 8 通過 =「組裝看得見」；不等于「一局可玩」。

---

## 步驟 9 — 可玩橋接（Tick／撿物） `[ ]`

規則層 API 需被呼叫才會動（Spec 用注入時間；預設 `bCanEverTick=false`）。

### 路徑 A — 藍圖橋接（人工） `[ ]`

在 `BP_DevPawn`：

1. `[ ]` Event Tick → 呼叫 Stamina 的 **Tick Stamina**（Delta Seconds）
2. `[ ]` Event Tick → 呼叫 Match 的 **Tick Match**（Delta Seconds + Stamina 組件）
3. `[ ]` 按鍵（例如 E）→ 對附近 Pickup 呼叫 **Try Collect Into**（傳入自己的 Inventory）
4. `[ ]`（可選）再按鍵呼叫 Inventory **Try Use Slot**（slot 0 + Stamina）

### 路徑 B — C++ DevCharacter／DevGameMode（Loop Engineering） `[ ]`

用 Agent 回合實作：建構子掛組件、Tick 呼叫、重疊／輸入撿物；Editor 只剩擺關卡與拖 Pickup。  
（與社群／官方 MCP 無關；屬迴圈產碼。）

### 路徑 C — Editor MCP 協助組裝 `[ ]`

| 選項 | 狀態 |
|------|------|
| Epic 官方 Unreal MCP（5.8 Experimental） | `[ ]` 未接上 Cursor |
| **chongdashu/unreal-mcp（專案級）** | `[~]` 檔案已落地；**啟用步驟見 `docs/UNREAL_MCP.md`** |
| 其他社群（如 db-lyon/ue-mcp） | `[ ]` 未安裝 |

MCP 可協助步驟 3–7 的「點 Editor」；**不能取代** Spec／雙閘門。

**步驟 9 完成判準：** PIE 中體力會降、可撿物進背包、可使用物品改體力、超時／歸零有勝負狀態（至少能在螢幕或 log 觀察）。

---

## 與 Loop Engineering 的分工

| 已完成（C++／Spec／閘門） | 本文件追蹤（Editor／橋接） |
|---------------------------|----------------------------|
| Stamina / Item / Inventory / Match / Spawn / Pickup 規則 | 步驟 2–8 組裝 |
| `Nightmare.*` Automation 綠燈 | 步驟 9 讓規則在 PIE「動起來」 |
| 不在迴圈改 `.uasset` | `.umap`／Blueprint 由人（或 MCP）組 |

---

## 勾選紀錄（可手寫日期）

| 日期 | 完成至步驟 | 備註 |
|------|------------|------|
| （例） | | |

---

## 變更紀錄

| 日期 | 說明 |
|------|------|
| 2026-08-11 | 初版：彙整先前說明步驟 1–9，全部標待執行以便追蹤 |
