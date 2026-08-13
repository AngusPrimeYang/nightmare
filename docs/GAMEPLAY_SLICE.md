# Nightmare — 玩法契約（第一垂直切片）

> 更新日期：2026-08-13  
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
| 敵人邊緣生成 | `Nightmare.EnemySpawn` | `[x]` | `UNightmareEdgeSpawnLocator` + `ANightmareEnemySpawner` |
| 敵人數值／移動 | `Nightmare.EnemyRoll` | `[x]` | `UNightmareEnemyRoller` + Chase/Wander |
| 敵人碰觸攻擊 | `Nightmare.EnemyAttack` | `[x]` | P6 扣 `AttackPower`＋Despawn；P7 共用 `ApplyHitKnockback` |
| 敵人浮空／步行 | `Nightmare.EnemyLocomotion` | `[x]` | P12：生成時 roll Hover／Walk；`TickMovement` 鎖離地高度差異 |
| 子彈 | `Nightmare.Projectile` | `[x]` | P13–P17：`ANightmareProjectileActor` 直線飛行＋可調速度／傷害／尺寸 |
| 敵人體力 | `Nightmare.EnemyHealth` | `[x]` | P18–P19：`UNightmareEnemyHealthComponent`＋中彈扣血歸零 Despawn |
| 流程／UI | `Nightmare.Flow` | `[x]` | U1–U8：`ANightmareFlowGameMode`＋C++ UMG 主選單／失敗 popup＋10 秒倒數 |

**完成定義：** 上表 Spec 綠 + 本頁勾選。Dev Map / Pawn 視覺組裝見 §4，不算本表阻塞。

---

## 4. 建議實作順序

1. 階段 **F**：**完成**  
2. 規則層 `Stamina` → `Item` → `Inventory` → `Match` → `Spawn` → `Pickup`：**完成**  
3. **Editor 組裝：** 見 **`docs/EDITOR_DEV_SETUP.md`** — 步驟 6–9B + Pickup 煙霧已過（移動／E 撿／F 用／Dev HUD）  
4. **物品 P1–P11 + 敵人 P4–P12 + 射擊 P13–P19 已落地**；§7.1 G4 子彈灰盒仍 optional  
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

## 7. 待辦 backlog（2026-08-13）

> 來源：產品／呈現討論收斂。灰盒可用 MCP／人工組裝；玩法規則仍以 C++ + Spec 為準。

### 7.1 物件／灰盒

| # | 狀態 | 項目 | 備註 |
|---|------|------|------|
| G1 | `[x]` | **美式加油站灰盒** | **`GS_*` + `StoreCeilLight_*` + 權威地坪 `GS_LotPad`（`BP_GrayPad`／BlockAll）**。站體對齊地坪頂 `STATION_Z=100`（勿跟 Open World 地景串流高度對賭：編輯器全載入 vs PIE 串流會「預覽埋土、▶懸空」）。腳本：`nightmare_g1_ground_station.py`。**務必 Ctrl+S。** |
| G2 | `[x]` | **角色灰盒** | `NightmareDevCharacter`：`GrayboxBody` + `GrayboxHead` |
| G3 | `[x]` | **物品灰盒** | `NightmarePickupActor` 旋轉 Cube；關卡 `GrayPickup_A/B/C` |
| G4 | `[x]` | **子彈灰盒（長條型）** | C++ 預設：`ANightmareProjectileActor` 長方盒 `(40,4,4)` half-extents；碰撞／mesh 同一組 `ProjectileHalfExtents`。 |

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
| P4 | `[x]` | **敵人：世界邊緣隨機生成** | `UNightmareEdgeSpawnLocator`（矩形外緣帶＋seed／stream）+ `ANightmareEnemySpawner`（重用 `UNightmareSpawnScheduler`）+ 灰盒 `ANightmareEnemyActor`；`NightmareDevGameMode` 自動掛 spawner。Spec：`Nightmare.EnemySpawn`（5 綠，2026-08-13）。 |
| P5 | `[x]` | **敵人：移動速度／攻擊力參數化＋生成時 roll** | `FNightmareEnemyStats` + `UNightmareEnemyRoller`（Speed 200–450、Power 5–20、ChaseChance 0.7）；生成時賦予個體；Chase／Wander retarget。Spec：`Nightmare.EnemyRoll`（7 綠，2026-08-13）。 |
| P6 | `[x]` | **敵人：碰觸消耗體力後消失** | `TryAttackPlayer`：`ApplyDelta(-AttackPower)` → Despawn；overlap 走同一入口。Spec：`Nightmare.EnemyAttack`（2026-08-13）。 |
| P7 | `[x]` | **敵人攻擊傷害反饋** | 與 P3 共用 `NightmareItemEffectApply::ApplyHitKnockback`（600/400）；無第二套 knockback。 |
| P8 | `[x]` | **物品：短暫加速／減速** | `UNightmarePlayerEffectComponent` Speed；Touch 或 Hold 使用。 |
| P9 | `[x]` | **物品：短暫改變跳躍高度** | Effect Jump／Gravity；Touch 或 Hold 使用。 |
| P10 | `[x]` | **背包選格使用（一次一格）** | 2026-08-12 PIE 驗過：`1`/`2`/`3` 選格 + `F` 用當前格；HUD `>n<`；一次一次 `TryUseSlot(Selected)`。 |
| P11 | `[x]` | **玩家跳躍（Space）** | 2026-08-12 PIE 驗過：`Space` → Jump；出生點抬高後可測。 |
| P12 | `[x]` | **敵人：浮空／步行兩種類型** | 生成時 `RollLocomotionType`（`HoverLocomotionChance` 預設 0.5）。**Hover** 維持 `HoverHeight`（預設 160）離地；**Walk** 貼地（capsule half-height + `GroundClearance`）。類型終身不隨 Chase／Wander retarget。PIE：球＝Hover、方塊＝Walk；debug 字 Cyan／Orange。Spec：`Nightmare.EnemyLocomotion`（10 綠，2026-08-13）。 |
| P13 | `[x]` | **玩家可射擊子彈** | Dev Character **LMB** → `TryFireProjectile` 生成 `ANightmareProjectileActor`（相機方向）。Spec：`Nightmare.Projectile`（6 綠，2026-08-13）。 |
| P14 | `[x]` | **子彈速度為可調參數** | 預設 `WalkSpeed×10`（600→**6000**）；`ProjectileSpeed` UPROPERTY；BeginPlay 從 baseline 同步。Spec 鎖公式。 |
| P15 | `[x]` | **子彈彈道為可調參數** | v1 `ENightmareProjectileTrajectory::StraightLine`（無重力）；`TickFlight` 沿 `FlightDirection` 等速。 |
| P16 | `[x]` | **子彈大小為可調參數** | 預設 half-extents `(40,4,4)` 長條；`HitBox`＋`GrayboxMesh` 共用 `ProjectileHalfExtents`。 |
| P17 | `[x]` | **子彈傷害為可調參數** | 預設 `MaxStamina/20`（100→**5**）；只打敵人（ignore Pawn）。Spec 鎖公式。 |
| P18 | `[x]` | **敵人賦予體力（可調＋生成時 roll）** | `UNightmareEnemyHealthComponent`；基準 25（100/4）；`RollHealth` 預設 **5–70**、≥1；與玩家體力分離、無全局消耗。Spec：`Nightmare.EnemyHealth`。 |
| P19 | `[x]` | **敵人中彈扣體力、歸零消失** | `TryTakeProjectileHit` / `TryApplyHitToEnemy`；HP≤0 → Despawn（碰玩家 P6 仍獨立）。Spec 9 綠，2026-08-13。 |

### 7.3 流程／UI

> 來源：2026-08-13 產品待辦。主選單＋體力歸零 popup；C++ 管流程與狀態，UMG／Widget Blueprint 管呈現（Editor 組裝，迴圈不改 `.uasset` 當玩法來源）。

| # | 狀態 | 項目 | 備註 |
|---|------|------|------|
| U1 | `[x]` | **關卡前主選單** | 全黑背景；畫面中上大字 **NIGHTMARE**；中間三選項：**開始遊戲**、**多人連線**、**結束遊戲**。預設進入遊戲先顯示此頁，不直接載入關卡。 |
| U2 | `[x]` | **開始遊戲 → 進關** | 點 **開始遊戲** 才 OpenLevel／切換至 Dev 關卡並生成 Pawn；主選單隱藏。 |
| U3 | `[x]` | **多人連線（占位、禁用）** | **多人連線** 灰字顯示；按鈕不可互動（`IsEnabled=false`／不綁 OnClicked）。 |
| U4 | `[x]` | **結束遊戲** | 點 **結束遊戲** → `UKismetSystemLibrary::QuitGame`（或同等）結束整個應用。 |
| U5 | `[x]` | **體力歸零 popup** | 玩家體力 ≤0（`UNightmareMatchComponent` Failed）時跳出遊戲內 modal；選項：**繼續遊戲**、**回到選單**；畫面明顯顯示 **10 秒倒數**。 |
| U6 | `[x]` | **繼續遊戲 → 重開一局** | 點 **繼續遊戲** 關閉 popup；重置 Match／Stamina／Inventory／刷物／敵人等 → 等同新一局（仍在關卡內，不回主選單）。 |
| U7 | `[x]` | **回到選單** | 點 **回到選單** 卸載關卡 Pawn／HUD，回到 **U1 主選單**（全黑＋NIGHTMARE）。 |
| U8 | `[x]` | **倒數歸零自動回選單** | popup 倒數至 **0** 時，等同自動執行 **回到選單**（U7）；未手動選擇亦離開本局。 |

**實作提示（非阻塞）：** 可用 `UGameInstance` 或專用 `UNightmareFlowSubsystem` 管 Menu ↔ Level 狀態；Failed 時 `SetGamePaused` + 僅 UI 可輸入；Spec filter 建議 `Nightmare.Flow`（倒數、Failed 觸發、Restart、回選單）。
