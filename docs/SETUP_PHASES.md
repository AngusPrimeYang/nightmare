# Nightmare：UE5 迴圈工程環境 — 分段執行清單

> 更新日期：2026-08-11  
> 目的：在尚無法安裝 Epic Games Launcher / Unreal Engine 時，仍可推進可做事項；引擎就緒後再接上編譯與編輯器驗證。  
> 狀態圖例：`[x]` 已完成　`[ ]` 尚未完成　`[~]` 部分完成／需本機操作

---

## 0. 總覽

| 階段 | 名稱 | 是否需要已安裝 UE | 目前狀態 |
|------|------|-------------------|----------|
| A | 倉庫與迴圈工程基建 | 否 | **已完成** |
| B | GitHub 遠端與換機（clone 後微調） | 否 | **已完成** |
| C | 無引擎下的專案骨架與產碼演練 | 否 | **已完成** |
| D | 安裝引擎與本機編譯前置 | **是**（引擎 + MSVC 工具鏈） | **已完成**（本機 2026-08-11） |
| E | 首次編譯與編輯器驗證（需要 UE） | 是 | **已完成**（本機 2026-08-11） |
| F | 行為閉環（Logic Gate + Spec） | 是 | **已完成**（本機 2026-08-11） |
| G | 便利強化（Hook／CI／資產規範等） | 建議已有引擎 | **可選**；刻意延後 |

**閉環分層（勿混淆）：**

| 層級 | 階段 | 意義 |
|------|------|------|
| 編譯閉環 | E | 產碼 → apply → build 能通；尚缺行為契約 |
| 行為閉環 | F | RunTests + 至少一條 Spec；少人介入的 Loop Engineering **就緒** |
| 便利強化 | G | Hook／CI 等；不擋開工 |

**順序硬約束：** `D → E → F`（**不可**把 F 排在 E 之前；無成功編譯則 Logic Gate 無意義）。  
換機：架構跟 GitHub；本機只需再驗 D→E（repo 已有 F 腳本／Spec 時，再煙霧跑一次 Logic Gate）。

**工作流前提：** 專案以 GitHub 為單一來源；換電腦 = `git clone` + 調整本機 `ue.local.env`（主要是 `UE_ROOT`），不在每台機器重做專案基建。

**目前阻塞點：** 無（D／E／F 就緒；**雙閘門 Loop Engineering 可用**）。  
**建議策略：** 依 `docs/GAMEPLAY_SLICE.md` 做第一個玩法系統（建議 `Stamina`）+ Spec。

---

## 1. 倉庫現況（已落地檔案）

```
nightmare/
├── Nightmare.uproject
├── .cursorrules
├── .cursor/rules/ue5-loop.mdc
├── .gitignore / .gitattributes
├── ue.local.env.example
├── Config/                      # DefaultEngine/Game/Input.ini
├── Content/.gitkeep
├── Source/
│   ├── Nightmare.Target.cs
│   ├── NightmareEditor.Target.cs
│   └── Nightmare/
│       ├── Nightmare.Build.cs
│       ├── Nightmare.h / .cpp
│       └── NightmareLoopActor.h / .cpp   # C9 apply_codegen 演練產物
├── Scripts/
│   ├── Resolve-UeEnv.ps1        # 支援 -SkipEngineValidation（無引擎可 apply）
│   ├── build_and_test.ps1
│   └── apply_codegen.ps1
├── codegen_staging/             # gitignored；本機產碼暫存
└── docs/SETUP_PHASES.md
```

**仍為本機／日後產生：**

- 本機 `ue.local.env`（gitignored）
- Visual Studio / Rider 專案檔（GenerateProjectFiles，非必須提交）
- 真實編譯產物（`Binaries/` 等，已 ignore）

---

## 2. 沒有 UE（Epic Launcher）時仍可做的事

下列都不依賴引擎二進位；做完推上 GitHub 後，其他機器只需 clone：

1. **維持與擴充規則**：`.cursorrules`、`.cursor/rules/`、模組命名約定（已在 repo）。
2. **推送遠端**：把基建與之後的骨架 commit / push 到 GitHub。
3. **手寫／Agent 產出 C++ 骨架**：`Source/`、`.uproject`、`Config/`（純文字，進 git）。
4. **演練產碼管線**：`apply_codegen.ps1` DryRun／覆寫（不編譯）；演練用 staging 目錄可本機自建、不必進 git。
5. **設計文件**：玩法、模組邊界、迴圈 prompt 範本。
6. **CI 草稿**（可選）：僅做 lint / 禁 `std::string` 的 grep，不做 UBT。

**換機時才做（不算「專案階段」，見階段 B）：** clone → 建 `ue.local.env` 填 `UE_ROOT` →（要編譯時）本機已有 MSVC / 引擎。

**明確做不到（需 UE）：**

- 真實編譯、連結、開編輯器、Live Coding、Cook/Package、驗證反射／藍圖編譯、Automation RunTests（階段 F）。

---

## 3. 分段清單

### 階段 A — 倉庫與迴圈工程基建（無需 UE）

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| A1 | `.cursorrules` 迴圈規範 | `[x]` | 命名 / UPROPERTY / FString / 完整檔案 |
| A2 | 可攜式 `UE_ROOT` 外部參數設計 | `[x]` | env 或 `ue.local.env`，禁止寫死路徑 |
| A3 | `Scripts/Resolve-UeEnv.ps1` | `[x]` | |
| A4 | `Scripts/build_and_test.ps1` | `[x]` | 有 `.uproject` + 引擎後才真能跑通 |
| A5 | `Scripts/apply_codegen.ps1` | `[x]` | 可先 DryRun／覆寫文字檔 |
| A6 | `.cursor/rules/ue5-loop.mdc` | `[x]` | |
| A7 | `.gitignore` | `[x]` | |
| A8 | `.gitattributes`（LFS） | `[x]` | |
| A9 | Cursor Hook 自動化（原 3.4） | `[ ]` | **刻意不做**；歸階段 G（可選），不擋 E／F |

**階段 A 結論：** 基建完成；應進 git 並由階段 B 推上 GitHub，再做 C。

---

### 階段 B — GitHub 遠端與換機（無需 UE）

> **定位修正：** 這不是「每台機器重做環境搭建」，而是  
> **repo 在 GitHub；任何電腦 `git clone` 後只調本機差異（幾乎只有 `UE_ROOT`）。**  
> `UE_PROJECT` / Target 等預設已在 `ue.local.env.example` 與日後 `.uproject` 對齊，通常不必每台重填。  
> MSVC / Rider / 引擎安裝屬**機器前置**，歸階段 D（要編譯時），不列為本階段待辦。

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| B1 | 確認敏感／本機檔不會進遠端 | `[x]` | `ue.local.env` 已在 `.gitignore`；範本用 `ue.local.env.example` |
| B2 | 將目前基建 commit（若尚未） | `[x]` | working tree clean；含 Scripts、rules、gitignore、gitattributes、docs |
| B3 | Push 到 GitHub（`origin`） | `[x]` | `origin` = `https://github.com/AngusPrimeYang/nightmare.git`；`main` 與 `origin/main` 同步 |
| B4 | 驗證換機流程文件化 | `[x]` | 見下方「換機三步」；與 §5 一致 |

**換機三步（每台新電腦，不做專案重建）：**

```text
1. git clone <github-repo-url>
   （若會拉 LFS 資產：該機器曾執行過 git lfs install 即可）
2. copy ue.local.env.example ue.local.env
   → 只改 UE_ROOT=這台機器的引擎路徑
3. 要編譯時再跑 Scripts\build_and_test.ps1
   （需本機已有 UE + MSVC Build Tools，見階段 D）
```

**階段 B 完成判準：** GitHub 上有最新基建；文件寫明 clone 後只調 `ue.local.env`。

---

### 階段 C — 無引擎專案骨架與產碼演練（無需 UE）

> 目標：文字層專案齊備；迴圈「產碼 → 套用」先跑通。  
> 注意：手寫骨架與 Launcher「New C++ Project」等價於準備 Source；**第一次有引擎時仍需 Generate Project Files + 編譯驗證**。

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| C1 | 新增 `Nightmare.uproject` | `[x]` | `EngineAssociation`: `"5.8"`（與本機 `UE_ROOT` / `ue.local.env.example` 對齊） |
| C2 | 新增 `Source/Nightmare.Target.cs` | `[x]` | Game Target |
| C3 | 新增 `Source/NightmareEditor.Target.cs` | `[x]` | Editor Target |
| C4 | 新增 `Source/Nightmare/Nightmare.Build.cs` | `[x]` | Core / CoreUObject / Engine / InputCore |
| C5 | 新增模組啟動碼 `Nightmare.h` / `Nightmare.cpp` | `[x]` | `IMPLEMENT_PRIMARY_GAME_MODULE` |
| C6 | 基礎 `Config/DefaultEngine.ini` 等 | `[x]` | Engine / Game / Input |
| C7 | 空的 `Content/`（可放 `.gitkeep`） | `[x]` | |
| C8 | 建立 `codegen_staging/` 演練目錄 | `[x]` | gitignored；本機已建並用於 C9 |
| C9 | 完整 `AActor` + `apply_codegen` | `[x]` | `ANightmareLoopActor` → `Source/Nightmare/`（無引擎可跑） |
| C10 | （可選）grep CI：禁止 `std::string` | `[ ]` | **刻意延後**，非本階段必要 |

**階段 C 完成判準：**  
`apply_codegen.ps1` 能把完整 `.h/.cpp` 寫進 `Source/`；repo 具備可提交的 C++ 骨架（仍未證明能編譯）。→ **已達成（2026-08-11）**

---

### 階段 D — 安裝引擎與本機編譯前置（需要 UE）

> 這才是「這台機器要能編譯」時才做的事；與 GitHub 專案內容無關。

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| D1 | 安裝 Epic Games Launcher | `[x]` | 本機已有；跳過重裝 |
| D2 | 安裝目標 UE 版本（與 `.uproject` 的 EngineAssociation 一致） | `[x]` | `UE_ROOT=D:\Epic Games\UE_5.8`；`.uproject` Association=`5.8` |
| D3 | 本機安裝 **VS 2022 Build Tools**（MSVC + Windows SDK） | `[x]` | 裝於 `D:\DDD\VS2022BuildTools`（VCTools + SDK）；`cl` 19.44 OK |
| D4 | clone 後建立 `ue.local.env`，填 `UE_ROOT` | `[x]` | 已指向含 `Build.bat` 的引擎根目錄 |
| D5 | 驗證路徑：`build_and_test.ps1` 能通過 UE_ROOT 檢查 | `[x]` | `Resolve-UeEnv`：UE_ROOT / Build.bat / .uproject 皆 PASS |
| D6 | （可選）安裝 Rider | `[ ]` | 除錯用；非必要，跳過 |
| D7 | （替代）若不能用以 Launcher：源碼建置引擎或現成引擎包 | `[ ]` | 不需要（Launcher 引擎已可用） |

**階段 D 完成判準：** 這台機器 `UE_ROOT` 有效且 MSVC 工具鏈可用。→ **已達成（2026-08-11）**

---

### 階段 E — 首次編譯與編輯器驗證（需要 UE）

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| E1 | 對 `.uproject` 執行 Generate Project Files（可選，Rider/除錯時有用） | `[ ]` | 可選；本階段未做（編譯不依賴） |
| E2 | `.\Scripts\build_and_test.ps1` 編譯 `NightmareEditor` | `[x]` | 首次成功；Target 已升為 V7 / `Unreal5_8` |
| E3 | 用 `UnrealEditor.exe` 開啟專案 | `[x]` | `UnrealEditor-Cmd` headless 煙霧：Engine init + 載入 `Nightmare` 模組後退出 |
| E4 | 確認階段 C 的範例 Actor 可放進關卡 / 編譯進編輯器 | `[x]` | `ANightmareLoopActor` 已進 `UnrealEditor-Nightmare.dll`（UHT + link） |
| E5 | 固定「產碼 → apply → build」一次成功 | `[x]` | staging → `apply_codegen` → rebuild `NightmareLoopActor` 成功 |

**階段 E 完成判準：** Editor 可開、腳本編譯成功、至少一個 C++ Actor 可用。→ **已達成（2026-08-11）**  
**注意：** E 完成 ≠ Loop Engineering 行為閉環就緒；下一步必須進階段 F（勿跳過、勿與 G 混為「可選」）。

---

### 階段 F — 行為閉環（Loop Engineering **必要**；必須在 E 之後）

> 目標：把「行為對不對」收進自動閘門，避免每回合靠人 PIE／肉眼當 oracle。  
> 依賴：階段 E 已通過。回合 SOP／Spec 寫法見 `.cursor/skills/ue5-loop-engineering/`。

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| F1 | `build_and_test.ps1` 支援 Logic Gate（`UE_RUN_TESTS` / RunTests；可 `-SkipTests` 只編） | `[x]` | `-RunTests` / `UE_RUN_TESTS`；`-testexit="Automation Test Queue Empty"`；summary 在 `Saved/LoopEngineering/` |
| F2 | 至少一條穩定 smoke Spec（如 `Nightmare.Smoke`）進 repo | `[x]` | `Source/Nightmare/Tests/NightmareSmoke.spec.cpp` |
| F3 | 本機煙霧：編譯後 Logic Gate 對 `UE_TEST_FILTER`（預設 `Nightmare.`）一次綠燈 | `[x]` | Found 1 test；`Result={Success}`（2026-08-11） |

**階段 F 完成判準：** RunTests 可由腳本觸發；至少一條 Spec 綠；**此時才稱 Loop Engineering（雙閘門）就緒**。→ **已達成（2026-08-11）**

---

### 階段 G — 可選強化（不擋雙閘門開工）

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| G1 | Cursor Hook：存檔/套用後自動編譯 | `[ ]` | 原 A9／舊 F1；等 E+F 穩定再考慮 |
| G2 | Rider 除錯設定文件化 | `[ ]` | |
| G3 | CI 上跑 UBT（需自備 runner + 引擎） | `[ ]` | 成本高 |
| G4 | 內容管線 / 資產命名 / 模組拆分規範 | `[ ]` | |

---

## 4. 建議執行順序（分段）

### 現在就能做（無 Launcher）

```
將階段 C 骨架 commit / push 到 GitHub
（之後等待階段 D：安裝引擎）
```

### 任意新電腦（有無引擎都一樣先做）

```
git clone → copy ue.local.env.example → 填 UE_ROOT（有引擎再填）
```

### 引擎可安裝之後（該機器第一次要閉環）

```
D1 → D2 → D3 → D4 → D5
        ↓
   E2 → E3 → E4 → E5          ← 編譯閉環
        ↓
   F1 → F2 → F3               ← 行為閉環（必要；不可跳過）
        ↓
   （可選）G1…
```

### 刻意延後（階段 G）

- G1 / 原 Cursor Hook：等 E+F 穩定再考慮。

---

## 5. 換機檢查清單（GitHub 工作流）

每到一台新電腦：

1. `git clone <repo>`（若使用 LFS 資產：該機執行過 `git lfs install`）
2. `copy ue.local.env.example ue.local.env` → **只改 `UE_ROOT`**
3. 若要編譯／跑迴圈：確認本機已有 UE + VS Build Tools（階段 D）
4. `.\Scripts\build_and_test.ps1`（編譯閉環煙霧）
5. 若 repo 已完成階段 F：再開 Logic Gate（`UE_RUN_TESTS=1` 或腳本約定方式）對 `Nightmare.` 煙霧一次

**不要**把 `ue.local.env` 提交進 git。專案內容一律靠 GitHub 同步。  
**不必**每台重做 F1／F2 的設計；只需確認這台能跑既有閘門。

---

## 6. 與「非 Visual Studio IDE」方案的對應

| 需求 | 無 UE 時 | 有 UE 後 |
|------|----------|----------|
| 編輯 / AI 產碼 | Cursor | Cursor |
| 套用完整檔案 | `apply_codegen.ps1` | 同左 |
| 編譯 | 無法真實編譯 | `build_and_test.ps1`（UBT，不開 VS） |
| 行為測試（Logic Gate） | 無法真實跑 | 同腳本 + RunTests（階段 F） |
| IDE 除錯 | 可先裝 Rider | Rider 開 `.uproject` |
| MSVC 工具鏈 | 可先裝 Build Tools | UBT 使用之 |

詳見先前討論：可不用 Visual Studio **IDE**，但 Windows 上通常仍需 **Build Tools**。

---

## 7. 進度速查（請隨手改狀態）

| 階段 | 進度 |
|------|------|
| A 基建 | **完成** |
| B GitHub / 換機 | **完成** |
| C 無引擎骨架 | **完成**（待 commit/push 同步遠端） |
| D 引擎 + 本機編譯前置 | **完成**（本機；UE 5.8 + Build Tools） |
| E 編譯閉環 | **完成** |
| F 行為閉環（必要） | **完成**（雙閘門就緒） |
| G 可選強化 | 延後（不擋開工） |

---

## 8. 下一刀建議（單一任務）

> Editor 步驟 1–9 煙霧已過（見 `docs/EDITOR_DEV_SETUP.md`）。規則層垂直切片 + 階段 F 雙閘門已完成。

1. **下一刀：P1** — 刷物呈現＋生成時 roll（C++／Spec；`GAMEPLAY_SLICE` §7.2）。  
2. 已齊：規則層、Editor 煙霧、P10、P11、G1–G3；**滿格拒撿**契約已明文（Spec 早已鎖）。  
3. （可選）關 Editor 編譯以帶上 Dev「Inventory full」toast。  
4. （可選）**commit / push**。  
5. G（Hook／CI）維持可選。
