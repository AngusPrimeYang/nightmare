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
4. **P11 跳躍已過** → **下一刀：P1 刷物呈現＋生成時 roll**  
5. 其後 P2（雙互動）／P4+（敵人）或鏡頭／美術換皮  

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
| G1 | `[x]` | **美式加油站灰盒** | 關卡以 **`GS_*`（`BP_GrayCube` 實例）+ `StoreCeilLight_*`（3×3 PointLight）** 組裝；抬升約 Z+90；mesh NoCollision。PlayerStart 宜 **`(0,-600,350)`**（Z=100 易陷 Open World 地景；腳本 `nightmare_fix_spawn_height.py`）。還原：`nightmare_g1_spawn_meshes.py`、`nightmare_g1_fill_lights.py`。**務必 Ctrl+S。** |
| G2 | `[x]` | **角色灰盒** | `NightmareDevCharacter`：`GrayboxBody` + `GrayboxHead` |
| G3 | `[x]` | **物品灰盒** | `NightmarePickupActor` 旋轉 Cube；關卡 `GrayPickup_A/B/C` |

#### 7.1.1 G1 加油站灰盒建議結構

**方案：** 單一 Blueprint（建議名 `BP_GasStationGraybox`，parent `Actor`）+ 多個 `StaticMeshComponent`，全部 mesh 用 `/Engine/BasicShapes/Cube.Cube`，**不設 material**。  
**放置：** `spawn`／擺關卡於世界座標中心 `(0, 0, 0)`（地面原點；UE 單位 cm）。  
**原則：** 寧可 30–50 個方塊，也不要少數大盒——窗框、簷口、泵島邊緣、柱帽、門楣都用小 cube 加細節。BasicShapes Cube 邊長約 100 cm；scale 以倍數想（`scale 12` ≈ 12 m 寬）。

| 元件名 | 角色 | Relative Location（約） | Relative Scale（約） |
|--------|------|----------------------|----------------------|
| `StoreFloor` | 店面地板 | `(0, 400, 5)` | `(12, 10, 0.1)` |
| `StoreWall_N/S/E/W` | 四面牆 | 依地板邊緣 | 厚 `0.2`、高 `~3` |
| `StoreRoof` | 店屋頂 | `(0, 400, 320)` | `(12.5, 10.5, 0.15)` |
| `StoreDoorGap` | 門洞 | 正面中央開縫（左右牆缺口即可） | — |
| `StoreWindow_L/R` | 窗框細節（扁盒） | 正面牆 | 薄片 |
| `CanopyDeck` | 加油棚頂 | `(0, -200, 450)` | `(20, 16, 0.2)` |
| `CanopyEdge_*` | 棚邊飾條 | 棚四邊 | 細長扁盒 |
| `CanopyPost_FL/FR/BL/BR` | 四根柱 | 棚四角 | `(0.4, 0.4, 4.5)` |
| `PumpIsland_1/2` | 泵島基座 | `(-300/0, -200, 15)` | `(3, 1.5, 0.3)` |
| `PumpBody_*` | 油機本體 | 島上 | `(0.8, 0.6, 1.8)` |
| `PumpHose_*` | 皮管暗示 | 側邊 | 細長扁盒 |
| `SignPole` | 招牌柱 | `(-900, -200, 300)` | `(0.5, 0.5, 6)` |
| `SignBoard` | 高架招牌 | 柱頂 | `(4, 0.3, 2)` |
| `PriceBoard` | 價目小牌 | 招牌下方 | 扁盒 |
| `Curb_*` | 路緣／停車線 | 棚下地面 | 扁長盒 |
| `Trash_*` / `IceBox` | 店前小物 | 店門旁 | 小方塊堆 |

**組裝順序（MCP／人工皆可）：**  
`create_blueprint` → 對每列 `add_component` + `set_static_mesh`（Cube，不傳 material）→ 調 location／scale／rotation → `compile` → spawn 於 `[0,0,0]`。

**預期：** 遠看為便利商店盒體 + 前方大雨棚 + 雙泵島 + 高架招牌；近看仍是灰立方體積木。  
**後續（2026-08-12）：** 室內天花板 **3×3** `PointLightComponent`（等距）；加油站抬升 `Z=90`；灰盒 mesh **NoCollision**；`PlayerStart` 移至棚前 `(0,-550,100)` 朝向店面，避免嵌進牆／地景卡住。

### 7.2 玩法延伸

| # | 狀態 | 項目 | 備註 |
|---|------|------|------|
| P1 | `[ ]` | **刷物呈現＋生成時 roll** | **建議下一刀。** 隨時間隨機出現；出現時懸浮低空、玩家移動可觸碰的位置。**生成當下**才決定 ±體力與數值大小。規則層已有 Scheduler／ItemDef／Pickup 殼；缺世界刷出、選位、roll、呈現與觸碰橋接。 |
| P2 | `[ ]` | **物品兩種互動** | (A) **碰到直接反應**（觸發當下改體力）；(B) **持有後使用**（現有：撿→持有→使用）。**契約：**(1) 互動方式 (A)/(B) **互斥**，**生成當下**隨機決定其一；(2) 每個隨機生成物品**只帶一種效果**（本條為體力 ±；與 **P8**／**P9** 互斥，不可同體疊加）。需擴契約／Spec。 |
| P3 | `[ ]` | **碰觸傷害反饋** | 碰到**直接觸發減少體力**的物品時，角色**向後跳躍**，用以表現受到傷害（呈現／移動反饋；需與 P2-A 一併設計 Spec／實作邊界）。 |
| P4 | `[ ]` | **敵人：世界邊緣隨機生成** | 隨時間在世界邊緣隨機刷出敵人；間隔／邊緣範圍參數化；需 Spec（可注入時間／假隨機）。 |
| P5 | `[ ]` | **敵人：移動速度／攻擊力參數化＋生成時 roll** | `MoveSpeed`、`AttackPower`（及合理 Min/Max）可調；**生成當下**隨機賦予該個體。移動行為：多半朝玩家、部分時間隨機移動（見先前討論）。 |
| P6 | `[ ]` | **敵人：碰觸消耗體力後消失** | 碰到玩家時消耗等同該敵人 `AttackPower` 的體力，之後敵人消失（despawn）。需 Spec 鎖扣量與消失。 |
| P7 | `[ ]` | **敵人攻擊傷害反饋** | 玩家被敵人攻擊（P6）後，同樣有傷害反饋（與 P3 對齊：向後跳躍等呈現／移動反饋）；可共用同一反饋入口，避免兩套邏輯。 |
| P8 | `[ ]` | **物品：短暫加速／減速** | 新互動效果類型：短暫改變玩家移動速度（加速或減速）。互動方式與 **P2** 對齊且同樣互斥：(A) **碰到直接反應** 或 (B) **持有後使用**，**生成當下**隨機決定其一。`SpeedMultiplier`、`Duration`（及合理 Min/Max）參數化；**生成當下**隨機賦予該個體。**契約：與 P2／P9 互斥——每個隨機生成物品只會有一種效果**（體力 **或** 加速／減速 **或** 跳躍／重力，不可疊加）。需 Spec（可注入時間／假隨機）。 |
| P9 | `[ ]` | **物品：短暫改變跳躍高度** | 新互動效果類型：短暫增加或減少玩家跳躍高度。互動方式與 **P1**／**P8** 對齊：(A) **碰到直接反應** 或 (B) **持有後使用**，**生成當下**隨機決定其一；數值亦於**生成當下** roll。`JumpHeightMultiplier`、`GravityScale`、`Duration`（及合理 Min/Max）參數化；允許極端——幾乎跳不起來，或難以落地。**契約：與 P2／P8 互斥——每個隨機生成物品只會有一種效果**（體力 **或** 加速／減速 **或** 跳躍／重力，不可疊加）。需 Spec（可注入時間／假隨機）。 |
| P10 | `[x]` | **背包選格使用（一次一格）** | 2026-08-12 PIE 驗過：`1`/`2`/`3` 選格 + `F` 用當前格；HUD `>n<`；一次一次 `TryUseSlot(Selected)`。 |
| P11 | `[x]` | **玩家跳躍（Space）** | 2026-08-12 PIE 驗過：`Space` → Jump；出生點抬高後可測。 |
