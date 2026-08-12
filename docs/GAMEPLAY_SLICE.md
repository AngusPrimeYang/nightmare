# Nightmare — 玩法契約（第一垂直切片）

> 更新日期：2026-08-12  
> 用途：追蹤「做了什麼／做到哪」；行為對錯以 Automation Spec（`Nightmare.*`）為準，本頁只記規則與進度。  
> 不綁鏡頭／類型（第一人稱、第三人稱、橫版等之後再掛呈現層）。

---

## 1. 一局規則（產品契約）

玩家有**體力**。場上有可互動物品，可**撿拾**、**持有**，且有**使用次數上限**。使用物品會**增加或減少**體力。  
**持有格數有上限（v1＝3）；背包已滿時不可再撿**（地上物保留，不消費、不消失）。  
體力**歸零 = 失敗**。在**固定時限內活著 = 獲勝**。  
物品會在**不固定間隔**刷新出現。體力會隨時間下降；若不靠物品維持，最終會歸零失敗。

```text
時間流逝 → 體力自然下降
     ↓
撿物（未滿才可）→ 持有（≤3 格）→ 消耗次數使用 → 體力 ±
     ↓
≤0 = Failed ｜ 撐過 SurviveDuration 且體力 >0 = Won
     +
[MinSpawnInterval, MaxSpawnInterval] 隨機刷物
```

---

## 2. v1 已拍板決策

| 決策 | v1 選擇 | 備註 |
|------|---------|------|
| 持有格數 | **3** | Dev 快捷：`1`/`2`/`3` 選格 + `F` 用當前格（**P10**）；正式 UI 快捷欄仍可後做 |
| 滿格撿拾 | **拒絕** | 已滿不可再撿；地上物保留。`TryAddItem`→`INDEX_NONE`／`TryCollectInto` false；Spec 已鎖 |
| 體力下降 | **全局持續消耗**（每秒或固定 tick） | 「不使用物品就會死」= 不補就會被消耗打穿，不另做「未使用加速」 |
| 物品 delta | 可正可負 | 風險／報酬；負值不是 bug |
| 刷物節奏 | 隨機區間 `[Min, Max]` 秒 | Spec 可注入時間／假隨機 |
| 同時勝負 | **Failed 優先** | 同一幀體力 ≤0 與時限到達 → Failed |
| 呈現 | 未定 | Dev Map + 預設 Pawn 僅煙霧；不鎖人稱 |
| Dev 移動 | WASD＋滑鼠＋**Space 跳躍** | `ANightmareDevCharacter` Enhanced Input；`JumpZVelocity=700` |

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
3. **Editor 組裝：** 見 **`docs/EDITOR_DEV_SETUP.md`** — 步驟 6–9B + Pickup 煙霧已過（移動／E 撿／F 用／Dev HUD）  
4. **物品玩法 P1–P3／P8／P9 已落地（待編譯＋Spec 閘）** → 下一刀可做 **P4+ 敵人**  
5. 其後鏡頭／美術換皮  

---

## 5. 刻意不做（本切片）

- Speckit／重型規格工具（單頁契約 + Spec + git 足夠）  
- 多人、大逃殺縮圈、精美關卡／動畫  
- 在迴圈裡改 Blueprint 資產當玩法來源  
- 角色手腳隨移動擺動（MCP 能力不足；靜態灰盒即可）  
- 單靠 unreal-mcp 做「？」貼圖、定時刷物、生成時 roll 體力（屬 C++／Editor 分工）  

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

---

## 7. 待辦 backlog（2026-08-12）

> 來源：產品／呈現討論收斂。灰盒可用 MCP／人工組裝；玩法規則仍以 C++ + Spec 為準。

### 7.1 物件／灰盒

| # | 狀態 | 項目 | 備註 |
|---|------|------|------|
| G1 | `[x]` | **美式加油站灰盒** | **`GS_*` + `StoreCeilLight_*` + 權威地坪 `GS_LotPad`（`BP_GrayPad`／BlockAll）**。站體對齊地坪頂 `STATION_Z=100`（勿跟 Open World 地景串流高度對賭：編輯器全載入 vs PIE 串流會「預覽埋土、▶懸空」）。腳本：`nightmare_g1_ground_station.py`。**務必 Ctrl+S。** |
| G2 | `[x]` | **角色灰盒** | `NightmareDevCharacter`：`GrayboxBody` + `GrayboxHead` |
| G3 | `[x]` | **物品灰盒** | `NightmarePickupActor` 旋轉 Cube；關卡 `GrayPickup_A/B/C` |

#### 7.1.1 G1 加油站灰盒（現行組裝）

**實際落地（勿改回巨型單 BP）：** 關卡內多個 **`GS_*`**（`BP_GrayCube`，mesh NoCollision）+ **`StoreCeilLight_0..8`**（世界 PointLight 3×3）+ 權威地坪 **`GS_LotPad`**（`BP_GrayPad`，BlockAll）。

| 約定 | 值 |
|------|-----|
| 權威地面 | `GS_LotPad` 頂面（勿用 Landscape／World Partition 串流高度當站區 Z） |
| `STATION_Z` | **100**（地坪頂；零件世界 Z = 相對 Z + 100） |
| PlayerStart | 棚前約 `(0, -600, 220)`；出生另由 `NightmareDevGameMode` 線跡找地 |
| 還原／對齊 | `Tools/unreal-mcp-server/scripts/actors/nightmare_g1_ground_station.py`（含建 pad、移 `GS_*`／燈／PlayerStart） |
| 僅刷 mesh | `nightmare_g1_spawn_meshes.py`（`STATION_Z=100`） |
| 禁忌 | 巨型 `BP_GasStationGraybox`（易炸 Editor／MCP）；為防陷地去抬整座站或改對齊 Landscape actor Z |

**相對座標（設計地面 = 0；世界加 `STATION_Z`）仍可用下表當零件表：**

| 元件名 | 角色 | Relative Location（約） | Relative Scale（約） |
|--------|------|----------------------|----------------------|
| `LotPad`（`GS_LotPad`） | 有碰撞大地坪 | 中心約 `(0, -100, STATION_Z-25)`，scale `(36, 32, 0.5)` | 頂面 = `STATION_Z` |
| `StoreFloor` | 店面地板 | `(0, 400, 5)` | `(12, 10, 0.1)` |
| `StoreWall_N/S/E/W` | 四面牆 | 依地板邊緣 | 厚 `0.2`、高 `~3` |
| `StoreRoof` | 店屋頂 | `(0, 400, 320)` | `(12.5, 10.5, 0.15)` |
| `StoreWindow_L/R` | 窗框細節 | 正面牆 | 薄片 |
| `CanopyDeck` | 加油棚頂 | `(0, -200, 450)` | `(20, 16, 0.2)` |
| `CanopyEdge_*` | 棚邊飾條 | 棚四邊 | 細長扁盒 |
| `CanopyPost_FL/FR/BL/BR` | 四根柱 | 棚四角 | `(0.4, 0.4, 4.5)` |
| `PumpIsland_1/2` | 泵島基座 | `(-300/200, -200, 15)` | `(3, 1.5, 0.3)` |
| `PumpBody_*`／`PumpHead_*` | 油機 | 島上 | 見腳本 `PARTS` |
| `SignPole`／`SignBoard`／`PriceBoard` | 高架招牌 | `(-900, -200, …)` | 見腳本 |
| `Curb_*`／`Trash_*`／`IceBox` | 路緣／小物 | 棚下／店門旁 | 小方塊 |

**高度踩坑（已踩過）：** Editor 常全載入地景（表面≈100），PIE World Partition 串流後當地表面變低／缺塊 → 同一 `GS_*` Z 會「預覽埋土、▶懸空」。**只調 `STATION_Z` 無法兩頭兼顧**；必須有 `GS_LotPad`（或同等有碰撞地坪）當權威地面。

**預期：** 遠看便利店 + 雨棚 + 雙泵島 + 招牌，脚下灰色大地坪；近看仍是灰立方體積木。  
MCP 改完關卡後 **務必 Ctrl+S**。

### 7.2 玩法延伸

| # | 狀態 | 項目 | 備註 |
|---|------|------|------|
| P1 | `[x]` | **刷物呈現＋生成時 roll** | `ANightmareItemSpawner` + `UNightmareItemRoller`；生成當下 roll 效果／互動；懸浮刷出。`Nightmare.ItemRoll` Spec。 |
| P2 | `[x]` | **物品兩種互動** | TouchInstant vs HoldToUse 互斥；每物單一 EffectType（Stamina／Speed／Jump）。 |
| P3 | `[x]` | **碰觸傷害反饋** | Touch + 負體力 → `ApplyHitKnockback`（後跳）。 |
| P4 | `[ ]` | **敵人：世界邊緣隨機生成** | 隨時間在世界邊緣隨機刷出敵人；間隔／邊緣範圍參數化；需 Spec（可注入時間／假隨機）。 |
| P5 | `[ ]` | **敵人：移動速度／攻擊力參數化＋生成時 roll** | `MoveSpeed`、`AttackPower`（及合理 Min/Max）可調；**生成當下**隨機賦予該個體。移動行為：多半朝玩家、部分時間隨機移動（見先前討論）。 |
| P6 | `[ ]` | **敵人：碰觸消耗體力後消失** | 碰到玩家時消耗等同該敵人 `AttackPower` 的體力，之後敵人消失（despawn）。需 Spec 鎖扣量與消失。 |
| P7 | `[ ]` | **敵人攻擊傷害反饋** | 玩家被敵人攻擊（P6）後，同樣有傷害反饋（與 P3 對齊：向後跳躍等呈現／移動反饋）；可共用同一反饋入口，避免兩套邏輯。 |
| P8 | `[x]` | **物品：短暫加速／減速** | `UNightmarePlayerEffectComponent` Speed；Touch 或 Hold 使用。 |
| P9 | `[x]` | **物品：短暫改變跳躍高度** | Effect Jump／Gravity；Touch 或 Hold 使用。 |
| P10 | `[x]` | **背包選格使用（一次一格）** | 2026-08-12 PIE 驗過：`1`/`2`/`3` 選格 + `F` 用當前格；HUD `>n<`；一次一次 `TryUseSlot(Selected)`。 |
| P11 | `[x]` | **玩家跳躍（Space）** | 2026-08-12 PIE 驗過：`Space` → Jump；出生點抬高後可測。 |
