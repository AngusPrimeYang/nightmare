# Nightmare — 多人連線（Steam）設計與待辦

> 更新日期：2026-08-13  
> 狀態：**僅文件／規格** — 尚未開始任何實作階段（階段 0–3 皆 `[ ]`）  
> 用途：Steam 為例的多人連線架構分析、產品契約、技術缺口、分階段計畫與詳細待辦。  
> 行為對錯（實作後）以 Automation Spec 為準；本頁為設計與進度追蹤。

---

## 1. 與現況的關係

| 項目 | 現況 |
|------|------|
| 玩法切片 | v1 單人垂直切片已完成（見 `docs/GAMEPLAY_SLICE.md` §3） |
| 主選單 U3 | **多人連線** 按鈕占位、灰字、`IsEnabled=false`（U3 已落地） |
| 網路程式 | `Source/Nightmare/` **零** replication／RPC／`HasAuthority` |
| Build 模組 | `Nightmare.Build.cs` 無 `OnlineSubsystem` / `OnlineSubsystemUtils` |
| 引擎 ini | `DefaultEngine.ini` 無 Steam OSS／SteamSockets 設定 |
| 本文件 | 定義多人產品契約 + 技術路線 + 待辦；**不取代** 既有單人 Spec |

---

## 2. 架構：三層分離

多人連線必須分三層理解；**不可把 Steam 當成「複製玩法」的替代品**。

```text
┌─────────────────────────────────────────────────────────┐
│  Gameplay 層：Listen Server 權威 + Actor/Component 複製   │
│  （體力、背包、刷怪、子彈、勝負、UI 流程）                  │
├─────────────────────────────────────────────────────────┤
│  Session 層：IOnlineSession（OnlineSubsystemSteam）      │
│  （建房／搜房／加入／銷毀 Lobby、好友邀請、Steam 身分）     │
├─────────────────────────────────────────────────────────┤
│  Transport 層：NetDriver（SteamSockets / IpNetDriver）   │
│  （P2P、NAT punch、加密通道、ClientTravel 網址）          │
└─────────────────────────────────────────────────────────┘
         Steam 官方 API          UE Online Subsystem
         （不直接寫 C++）         （不直接碰 Steamworks SDK）
```

| 層 | 責任 | Steam 角色 | Nightmare 現況 |
|----|------|------------|----------------|
| **Transport** | 封包、連線 | P2P、NAT | 未設定 |
| **Session** | 誰和誰玩、Lobby 生命週期 | AppId、Overlay 邀請 | `UNightmareGameInstance` 僅 `QuitNightmare` |
| **Gameplay** | 世界狀態、規則 | **不參與** | 全本機 Tick + 本機 `SpawnActor` |

**建議實作順序（開始實作時）：**

1. 階段 1：Gameplay 權威 + PIE Listen Server（**不接 Steam**）
2. 階段 2：Session 接 Steam OSS
3. 階段 3：Transport 換 SteamSockets + 發行設定

---

## 3. v1 產品契約（已拍板）

### 3.1 連線模式

| 決策 | v1 選擇 | 備註 |
|------|---------|------|
| 拓撲 | **Listen Server**（房主兼 Server + 玩家） | 不做 Dedicated Server |
| 平台 | **Steam**（`OnlineSubsystemSteam` + 可選 SteamSockets） | 開發可用 AppId **480**（Spacewar） |
| 人數 | **2–4**（上限可配置，預設 4） | Lobby `NumPublicConnections` |
| 世界 | **共用同一關卡、同一套刷物／刷怪** | Server 權威 spawn |
| 體力／背包 | **每人一份**（維持 per-Pawn） | 不改成全局共享體力 |
| 對局計時 | **全場共用一個 Survive 時鐘** | 從 per-Character `Match` 抽到 GameState |
| 中途加入 | **v1 關閉** | Host 開房 → 全員 Ready → 開始 |
| 單人 | **不走 Steam** | 主選單「開始遊戲」維持現有本機流程 |

### 3.2 勝負與死亡（2026-08-13 拍板）

> 取代原單人「體力 ≤0 立刻 Failed popup」在**多人**下的語意；單人模式行為不變（見 §3.3）。

**規則 M1 — 個人死亡 → 自由視角（Spectator）**

- 觸發：該玩家 **體力 ≤0**（與現有 `UNightmareStaminaComponent` 耗盡一致）。
- 行為：
  - 解除對原 Pawn 的操作（或 Pawn 進入不可互動狀態）。
  - 切換至 **自由視角**：可在關卡內 **任意飛行／移動** 觀看全圖（無碰撞或僅與世界簡單碰撞，v1 以灰盒 fly cam 為準）。
  - **不再**消耗體力、不撿物、不開火、不受敵人攻擊（已視為出局）。
  - 仍可看到場上存活玩家與世界進行中的模擬。
- 不觸發個人 Failed popup（U5）。

**規則 M2 — 全滅 → 房主失敗 popup → 接上 U5–U8**

- 觸發：**所有已連線、曾參與本局的玩家** 皆進入死亡／Spectator 狀態（含 Host 本人）。
- 行為：
  - **權威判定在 Server**（Listen Server = Host 機器）。
  - **由 Host 的 `ANightmareFlowPlayerController` 觸發** 既有 `EnterFailedPopupPhase` → U5 popup → U6 重開／U7 回選單／U8 倒數歸零。
  - Client 端 PC：全滅後 **同步** 進入 FailedPopup 流程（FlowPhase 複製驅動 UI，而非各 Client 自行讀本地 Match）。
- 「Won」（撐過 `SurviveDurationSeconds` 且仍有人存活）仍沿用單人 Evaluate 邏輯，但改為 **全場 GameState 判定**；v1 若時間到且至少一人存活 → 全員 Won（Won UI 可後做，不阻塞連線）。

**規則 M3 — 單人相容**

- 僅一名玩家時：體力 ≤0 ＝ 全滅 → **直接** Host Failed popup（U5–U8），**可跳過** Spectator 或僅瞬間過渡（實作時二選一，Spec 鎖「單人不顯示持久 Spectator」）。

```text
[Playing] 玩家 A 體力 > 0, B 體力 > 0
     │
     ├─ A 體力 ≤0 → A → Spectator（自由視角），B 繼續 Playing
     │
     ├─ B 體力 ≤0 → B → Spectator；若 A 已是 Spectator → 全滅
     │
     └─ 全滅 → Server 設 FlowPhase=FailedPopup
              → Host PC EnterFailedPopupPhase
              → 全員 UI 同步（U5–U8）
```

### 3.3 與現有 U1–U8 的對接

| 既有項 | 單人（不變） | 多人（新增） |
|--------|--------------|--------------|
| U1 主選單 | 開始／結束 | **多人連線** 啟用 → 子流程：建立房間／加入 |
| U2 開始遊戲 | 本機 `StartGameplay` | 同左；**不**強制走 Steam |
| U3 多人占位 | 灰字禁用 | 改為 Host / Join 入口 |
| U5 Failed popup | 體力 ≤0 即觸發 | **僅全滅** 觸發；個人死亡只 Spectator |
| U6 繼續遊戲 | Server 重置全場 | 全員重生 Playing；Spectator 回 Pawn |
| U7 回選單 | DestroySession + 回 U1 | 需 **DestroySession** 再回主選單 |
| U8 倒數歸零 | 同 U7 | 同左，權威在 Host |

---

## 4. 現有程式缺口（為何現在不能直接開 Steam）

以下基於 `Source/Nightmare/` 靜態分析（2026-08-13）。

### 4.1 GameMode 僅 Server 可見

`ANightmareFlowPlayerController` 直接呼叫 `GetAuthGameMode()->StartGameplay()` 等。Client 上 `GetAuthGameMode()` 為 `nullptr`，開始／重開／回選單會失效。

**需改：** UI → `Server` RPC → GameMode；`FlowPhase` 移至 **`ANightmareGameState`** 並複製。

### 4.2 Client 會自行模擬規則

`ANightmareDevCharacter::Tick` 在本機執行 `TickStamina` / `TickMatch`；`IsGameplaySimulationActive()` 在找不到 GameMode 時 **回傳 true**。

**需改：** 僅 `HasAuthority()`（或 Dedicated 規則元件）在 Server Tick；Client 只收複製結果。

### 4.3 世界物為本機 Spawn

`ANightmareItemSpawner`、`ANightmareEnemySpawner`、`TryFireProjectile` 皆 `World->SpawnActor`，無 `bReplicates`。

**需改：** 僅 Server spawn；Pickup / Enemy / Projectile 設 `bReplicates = true`。

### 4.4 背包物件無法直接複製

`UNightmareInventoryComponent` 持有 `TArray<TObjectPtr<UNightmareItemInstance>>`；`UNightmareItemInstance` 為普通 `UObject`，非 Actor。

**需改：** 複製 **結構化槽位**（`FNightmareItemDef` + `RemainingUses`）；規則層 API 保留給 Spec。

### 4.5 勝負綁在單一 Character

`UNightmareMatchComponent` 在 Pawn 上；`TryDetectMatchFailed` 在 PC 讀本地 Match → 立刻 popup。

**需改：**

- 共用計時 → GameState
- 個人死亡 → PlayerState 或 Pawn 旗標 `bEliminated` + Spectator 切換
- 全滅判定 → GameState / GameMode（Server）
- Failed popup → **全滅後** 才由 Host 權威觸發

### 4.6 Pause 與 Listen Server

現有 Failed popup 使用 `DisableInput` + UI-only；若誤用 `SetGamePaused(true)` 會凍結整台 Host 模擬。

**需改：** 維持「只關玩家輸入、不暫停 World」；Spectator 期間 World 繼續跑。

---

## 5. 建議權威切分

| 系統 | 權威 | 複製／RPC |
|------|------|-----------|
| 移動／跳躍 | Server（CharacterMovement 預測） | 位置、速度 |
| 體力增減 | Server | `CurrentStamina` RepNotify |
| 個人死亡 / Spectator | Server | `bEliminated`、SpectatorPawn 切換 |
| 全滅 / FlowPhase | Server GameState | `ENightmareFlowPhase` OnRep |
| 背包 | Server | 3 槽 struct 陣列 |
| 選格 1/2/3 | Client 本地 | 不必複製 |
| Survive 計時 | Server GameState | `ElapsedSeconds` |
| 刷物／刷怪 | Server | Spawned Actor replicate |
| Pickup 消失 | Server | `bCollected` 或 Destroy |
| 敵人 AI／攻擊 | Server Tick | 位置、HP、Despawn |
| 子彈 | Server spawn | 飛行 Actor 或短 RPC |
| 主選單／Failed UI | 各 PC 本地 Widget | GameState OnRep 驅動 |
| Steam Lobby | GameInstance | OSS callback |

**輸入路徑範例：**

```text
Client 按 E 撿物
  → PC::ServerTryCollect()
    → Server 驗證距離 / 背包
      → Inventory 變更 replicate
      → Pickup MarkCollected replicate
```

---

## 6. Steam 整合要點（實作階段 2–3 用）

### 6.1 設定（不寫死本機路徑）

- `.uproject` 啟用 `OnlineSubsystemSteam`（及可選 `OnlineSubsystemSteamSockets`）
- `Config/DefaultEngine.ini`：`DefaultPlatformService=Steam`、`SteamDevAppId=480`、NetDriver 定義
- 執行檔目錄 `steam_appid.txt`（開發 `480`）
- Steam 客戶端需運行；**Logic Gate 用 `OnlineSubsystemNULL`**，Spec 不依賴 Steam

### 6.2 Session API 表面（C++，不碰 Steamworks SDK）

建議新增 `UNightmareOnlineSessionSubsystem`（GameInstanceSubsystem）或擴充 `UNightmareGameInstance`：

- `HostSession(int32 MaxPlayers)`
- `FindAndJoinFriendSession()` / `JoinSessionByIndex`
- `LeaveSession()`
- Delegates：`OnSessionCreated`、`OnSessionJoined`、`OnSessionDestroyed`、`OnSessionError`

Host 成功 → `OpenLevel(Map, "listen")`；Join 成功 → OSS 回傳 `ConnectString` → `ClientTravel`。

### 6.3 測試策略

1. PIE：**Listen Server + N Players**（無 Steam）
2. 兩份 **Standalone** + LAN
3. 兩份 **Packaged** + Steam AppId 480
4. 跨網 NAT（SteamSockets）

---

## 7. 分階段計畫（均未開始）

| 階段 | 目標 | 狀態 |
|------|------|------|
| **0** | 本文件 + 產品契約 + 待辦 | `[x]` 文件 |
| **1** | Gameplay 權威、Replication、Spectator、全滅 popup；PIE Listen；Spec `Nightmare.Net` | `[ ]` |
| **2** | Steam Session（Host/Join/Leave）；U3 UI；GameInstance 狀態機 | `[ ]` |
| **3** | SteamSockets、正式 AppId、Overlay 邀請、斷線處理 | `[ ]` |

**明確停止線（本任務）：** 完成 §8 待辦清單的 **文件與契約** 即可；**不得** 在無另行指示下開始階段 1 程式碼。

---

## 8. 詳細待辦清單

### 8.0 文件與契約

| ID | 狀態 | 項目 | 備註 |
|----|------|------|------|
| D0 | `[x]` | 撰寫本文件 `docs/MULTIPLAYER_STEAM.md` | 含 M1–M3 勝負 |
| D1 | `[x]` | 更新 `docs/GAMEPLAY_SLICE.md` §6 交叉引用 | 指向本文件 |
| D2 | `[x]` | 更新 `docs/GAMEPLAY_SLICE.md` §5「刻意不做」| 多人改為「設計中，見 MULTIPLAYER_STEAM」 |
| D3 | `[ ]` | 新增 Spec 契約草案 `Nightmare.Net` 行為表 | 全滅、Spectator、權威；階段 1 再寫 `.spec.cpp` |

### 8.1 階段 1 — Gameplay 網路化（不接 Steam）

#### 8.1.1 基建類別

| ID | 狀態 | 項目 | 備註 |
|----|------|------|------|
| N1 | `[ ]` | 新增 `ANightmareGameState` | 複製 `FlowPhase`、共用 `ElapsedSeconds`、`SurviveDuration`、全滅判定 |
| N2 | `[ ]` | 新增 `ANightmarePlayerState` | `bEliminated`、PlayerName、Ready 旗標（可選） |
| N3 | `[ ]` | `NightmareFlowGameMode` 註冊 GameStateClass / PlayerStateClass | |
| N4 | `[ ]` | `DefaultEngine.ini`：`bUseNetworkGameMode` 等 PIE 預設 | 不改 Steam |

#### 8.1.2 Flow 與 RPC

| ID | 狀態 | 項目 | 備註 |
|----|------|------|------|
| N5 | `[ ]` | PC：`ServerRequestStartGameplay` / `ServerRequestRestartRound` / `ServerRequestReturnToMenu` | 取代 Client 直連 GameMode |
| N6 | `[ ]` | GameState：`OnRep_FlowPhase` → 各 PC 顯示／隱藏選單與 popup | |
| N7 | `[ ]` | 移除 Client 對 `GetAuthGameMode()` 的依賴（含 `IsGameplaySimulationActive`） | 改讀 GameState |
| N8 | `[ ]` | Failed popup：**僅** Server 在全滅時呼叫 Host 的 `EnterFailedPopupPhase` | M2 |
| N9 | `[ ]` | 單人路徑：體力 ≤0 仍走原 U5（M3） | Spec 分 `NetMode` 或玩家數 |

#### 8.1.3 死亡與 Spectator（M1）

| ID | 狀態 | 項目 | 備註 |
|----|------|------|------|
| N10 | `[ ]` | 定義 `ANightmareSpectatorPawn`（或 `ASpectatorPawn` 子類） | 自由飛行、Enhanced Input 或 axis 移動 |
| N11 | `[ ]` | Server：體力 ≤0 → 標記 `bEliminated` → `UnPossess` → `Possess(SpectatorPawn)` | 原 Pawn 可隱藏或 ragdoll 占位 |
| N12 | `[ ]` | Spectator 不 Tick 體力／不匹配個人 Failed | |
| N13 | `[ ]` | GameState：`AreAllPlayersEliminated()` | 全員 `bEliminated` → 觸發 M2 |
| N14 | `[ ]` | `RestartRound`：銷毀 Spectator、重生 Pawn、清 `bEliminated` | U6 多人語意 |
| N15 | `[ ]` | Spec：一人死一人活 → 僅死者 Eliminated；全滅 → FailedPopup 相位 | `Nightmare.Net` |

#### 8.1.4 Character / Component 複製

| ID | 狀態 | 項目 | 備註 |
|----|------|------|------|
| N16 | `[ ]` | `ANightmareDevCharacter`：`bReplicates = true`，`SetReplicateMovement(true)` | |
| N17 | `[ ]` | Stamina：`SetIsReplicated(true)` + `DOREPLIFETIME(CurrentStamina)` | Server-only Tick |
| N18 | `[ ]` | Inventory：改複製 struct 槽位 + Server RPC Use/Collect | |
| N19 | `[ ]` | Match：共用部分移 GameState；Pawn 只保留「是否已淘汰」 | |
| N20 | `[ ]` | PlayerEffects：Server 權威 + 複製剩餘時間（若需要） | |
| N21 | `[ ]` | 輸入：僅 Autonomous Proxy 綁 Enhanced Input | |

#### 8.1.5 世界 Actor

| ID | 狀態 | 項目 | 備註 |
|----|------|------|------|
| N22 | `[ ]` | Pickup / Enemy / Projectile：`bReplicates` | |
| N23 | `[ ]` | Spawner：**僅 Server** Tick；Client 關閉 spawn tick | |
| N24 | `[ ]` | `TryFireProjectile` → `ServerFireProjectile` | Server spawn 子彈 |
| N25 | `[ ]` | 敵人 overlap 攻擊：僅 Server 執行 `TryAttackPlayer` | |
| N26 | `[ ]` | Pickup overlap / Collect：僅 Server | |

#### 8.1.6 測試與 Editor

| ID | 狀態 | 項目 | 備註 |
|----|------|------|------|
| N27 | `[ ]` | Automation Spec `Nightmare.Net` | NULL OSS；多 Pawn 單 World 權威測試 |
| N28 | `[ ]` | PIE：Listen Server + 2 Players 手動驗收清單 | 寫入 `docs/EDITOR_DEV_SETUP.md` 新章 |
| N29 | `[ ]` | 確認 `GS_LotPad` 權威地面在 Client 一致 | 避免 WP 掉出世界 |

### 8.2 階段 2 — Steam Session

| ID | 狀態 | 項目 | 備註 |
|----|------|------|------|
| S1 | `[ ]` | `.uproject` 啟用 OnlineSubsystemSteam | |
| S2 | `[ ]` | `Nightmare.Build.cs` 加 `OnlineSubsystem`、`OnlineSubsystemUtils` | |
| S3 | `[ ]` | `DefaultEngine.ini` Steam OSS + NULL fallback | 不寫死 UE 路徑 |
| S4 | `[ ]` | `UNightmareOnlineSessionSubsystem`（或擴 GameInstance） | Host/Find/Join/Destroy |
| S5 | `[ ]` | U3：啟用「多人連線」→ 子選單 Host / Join | C++ UMG |
| S6 | `[ ]` | Host：`CreateSession` → `OpenLevel(..., listen)` | |
| S7 | `[ ]` | Join：`JoinSession` → `ClientTravel` | |
| S8 | `[ ]` | U7 / Quit：`DestroySession` | |
| S9 | `[ ]` | 開發用 `steam_appid.txt`（480）說明寫入 `docs/SETUP_PHASES.md` | gitignore 若需 |
| S10 | `[ ]` | Logic Gate 仍用 NULL OSS | Spec 不綁 Steam |

### 8.3 階段 3 — 傳輸與發行

| ID | 狀態 | 項目 | 備註 |
|----|------|------|------|
| T1 | `[ ]` | SteamSockets NetDriver | |
| T2 | `[ ]` | 正式 Steam AppId + Partner 後台 | 替換 480 |
| T3 | `[ ]` | Steam Overlay 邀請 | |
| T4 | `[ ]` | Host 斷線 / 踢人 / 超時 | 全員回主選單 |
| T5 | `[ ]` | Packaged 雙機 Steam 測試紀錄 | |

---

## 9. 與其他文件的關係

| 文件 | 管什麼 |
|------|--------|
| `docs/GAMEPLAY_SLICE.md` | 單人 v1 玩法契約與進度 |
| **本頁** | 多人 Steam 設計、M1–M3 勝負、待辦 |
| `docs/SETUP_PHASES.md` | 編譯／Logic Gate；Steam 環境可補章 |
| `docs/EDITOR_DEV_SETUP.md` | PIE 多人驗收步驟（待 N28 補） |
| `.cursor/skills/ue5-loop-engineering/SKILL.md` | 實作回合 SOP（開始階段 1 時遵守） |

---

## 10. 風險與踩坑

| 風險 | 緩解 |
|------|------|
| 先接 Steam、玩法仍 Client 權威 | 強制階段 1 先過 PIE Listen + Spec |
| Host 卡頓 | v1 接受；2–4 人好友房 |
| World Partition 高度／串流 | 沿用 `GS_LotPad` 權威地面；Client 出生一致 |
| `SetGamePaused` 凍結 Listen Server | Failed / Spectator 都不用全局 Pause |
| `UNightmareItemInstance` 複製 | struct 槽位；Spec 測規則、Net Spec 測槽位同步 |
| 迴圈改 `.uasset` | Session／Spectator／RPC 全 C++ |
| CI 無 Steam | NULL OSS + `Nightmare.Net` 不呼叫 Steam |

---

## 11. 完成定義（各階段）

| 階段 | Done  cuando |
|------|----------------|
| **0（本任務）** | 本文件 + M1–M3 寫入 + §8 待辦可追蹤 |
| **1** | PIE 2 人：A 死 → A Spectator；全滅 → Host popup → U6/U7；`Nightmare.Net` 綠 + compile 綠 |
| **2** | 兩份 Steam 480 客戶端 Host/Join 進同一關卡 |
| **3** | 跨網穩定 + 正式 AppId 計畫書（可與發行並行） |
