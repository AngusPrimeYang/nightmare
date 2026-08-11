# Nightmare：UE5 迴圈工程環境 — 分段執行清單

> 更新日期：2026-08-11  
> 目的：在尚無法安裝 Epic Games Launcher / Unreal Engine 時，仍可推進可做事項；引擎就緒後再接上編譯與編輯器驗證。  
> 狀態圖例：`[x]` 已完成　`[ ]` 尚未完成　`[~]` 部分完成／需本機操作

---

## 0. 總覽

| 階段 | 名稱 | 是否需要已安裝 UE | 目前狀態 |
|------|------|-------------------|----------|
| A | 倉庫與迴圈工程基建 | 否 | **已完成** |
| B | GitHub 遠端與換機（clone 後微調） | 否 | **可立即做** |
| C | 無引擎下的專案骨架與產碼演練 | 否 | **可立即做** |
| D | 安裝引擎與本機編譯前置 | **是**（引擎 + MSVC 工具鏈） | 受阻／待做 |
| E | 首次編譯與編輯器驗證 | 是 | 待做 |
| F | 迴圈閉環強化（可選） | 建議已有引擎 | 刻意延後 |

**工作流前提：** 專案以 GitHub 為單一來源；換電腦 = `git clone` + 調整本機 `ue.local.env`（主要是 `UE_ROOT`），不在每台機器重做專案基建。

**目前阻塞點：** 無法透過 Epic Launcher 安裝 UE → 階段 D/E 暫停。  
**建議策略：** 先做完 A–C 並推上 GitHub，把「clone 後一有引擎就能編譯」的狀態準備好。

---

## 1. 倉庫現況（已落地檔案）

```
nightmare/
├── .cursorrules                 # 迴圈工程規範 + 可攜式 UE_ROOT 說明
├── .cursor/rules/ue5-loop.mdc   # Cursor 常駐規則
├── .gitignore                   # UE 產物 / ue.local.env
├── .gitattributes               # Git LFS 資產規則
├── ue.local.env.example         # 本機環境範本
├── Scripts/
│   ├── Resolve-UeEnv.ps1        # 解析 UE_ROOT 等外部參數
│   ├── build_and_test.ps1       # UBT 編譯閘門
│   └── apply_codegen.ps1        # 完整檔案覆寫閘門
└── docs/
    └── SETUP_PHASES.md          # 本文件
```

**尚未存在（預期後續產生）：**

- `Nightmare.uproject`
- `Source/`、`Config/`、`Content/`
- 本機 `ue.local.env`（gitignored，每台機器自建）
- Visual Studio / Rider 專案檔（可由引擎產生，非必須提交）

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

- 真實編譯、連結、開編輯器、Live Coding、Cook/Package、驗證反射／藍圖編譯。

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
| A9 | Cursor Hook 自動化（原 3.4） | `[ ]` | **刻意不做**，之後可選 |

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
| B2 | 將目前基建 commit（若尚未） | `[ ]` | 含 Scripts、rules、gitignore、gitattributes、docs |
| B3 | Push 到 GitHub（`origin`） | `[ ]` | 之後以遠端為準繼續開發 |
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
| C1 | 新增 `Nightmare.uproject` | `[ ]` | `EngineAssociation` 填你預計安裝的版本（如 `"5.5"`） |
| C2 | 新增 `Source/Nightmare.Target.cs` | `[ ]` | Game Target |
| C3 | 新增 `Source/NightmareEditor.Target.cs` | `[ ]` | Editor Target |
| C4 | 新增 `Source/Nightmare/Nightmare.Build.cs` | `[ ]` | 模組依賴 |
| C5 | 新增模組啟動碼 `Nightmare.h` / `Nightmare.cpp` | `[ ]` | `IMPLEMENT_PRIMARY_GAME_MODULE` |
| C6 | 基礎 `Config/DefaultEngine.ini` 等 | `[ ]` | 可最小集合 |
| C7 | 空的 `Content/`（可放 `.gitkeep`） | `[ ]` | 資產之後用 LFS |
| C8 | 建立 `codegen_staging/` 演練目錄 | `[ ]` | 已被 gitignore 亦可本機自建 |
| C9 | 用 Agent 產出一個完整 `AActor` 範例並 `apply_codegen` | `[ ]` | **不編譯**，只驗證整檔覆寫 |
| C10 | （可選）grep CI：禁止 `std::string` / `#include <string>` | `[ ]` | 無 UE 也可做 |

**階段 C 完成判準：**  
`apply_codegen.ps1` 能把完整 `.h/.cpp` 寫進 `Source/`；repo 具備可提交的 C++ 骨架（仍未證明能編譯）。

---

### 階段 D — 安裝引擎與本機編譯前置（需要 UE）

> 這才是「這台機器要能編譯」時才做的事；與 GitHub 專案內容無關。

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| D1 | 安裝 Epic Games Launcher | `[ ]` | 目前受阻 |
| D2 | 安裝目標 UE 版本（與 `.uproject` 的 EngineAssociation 一致） | `[ ]` | |
| D3 | 本機安裝 **VS 2022 Build Tools**（MSVC + Windows SDK） | `[ ]` | 不必裝完整 Visual Studio IDE；每台要編譯的機器一次 |
| D4 | clone 後建立 `ue.local.env`，填 `UE_ROOT` | `[ ]` | 指向含 `Engine\Build\BatchFiles\Build.bat` 的根目錄 |
| D5 | 驗證路徑：`build_and_test.ps1` 能通過 UE_ROOT 檢查 | `[ ]` | 路徑錯會立刻失敗並提示 |
| D6 | （可選）安裝 Rider | `[ ]` | 除錯用；非 GitHub 流程必需 |
| D7 | （替代）若不能用以 Launcher：源碼建置引擎或現成引擎包 | `[ ]` | 進階、非預設 |

**階段 D 完成判準：** 這台機器 `UE_ROOT` 有效且 MSVC 工具鏈可用。

---

### 階段 E — 首次編譯與編輯器驗證（需要 UE）

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| E1 | 對 `.uproject` 執行 Generate Project Files（可選，Rider/除錯時有用） | `[ ]` | |
| E2 | `.\Scripts\build_and_test.ps1` 編譯 `NightmareEditor` | `[ ]` | log 在 `Saved/LoopEngineering/` |
| E3 | 用 `UnrealEditor.exe` 開啟專案 | `[ ]` | |
| E4 | 確認階段 C 的範例 Actor 可放進關卡 / 編譯進編輯器 | `[ ]` | |
| E5 | 固定「產碼 → apply → build」一次成功閉環 | `[ ]` | 迴圈工程最小可用 |

**階段 E 完成判準：** Editor 可開、腳本編譯成功、至少一個 C++ Actor 可用。

---

### 階段 F — 可選強化（有引擎後再做）

| # | 項目 | 狀態 | 說明 |
|---|------|------|------|
| F1 | Cursor Hook：存檔/套用後自動編譯 | `[ ]` | 原步驟 3.4，先前刻意不做 |
| F2 | Rider 除錯設定文件化 | `[ ]` | |
| F3 | CI 上跑 UBT（需自備 runner + 引擎） | `[ ]` | 成本高 |
| F4 | 內容管線 / 資產命名 / 模組拆分規範 | `[ ]` | |

---

## 4. 建議執行順序（分段）

### 現在就能做（無 Launcher）

```
B2 → B3（commit + push 到 GitHub）
        ↓
   C1 → C7（骨架，繼續 commit / push）
        ↓
   C8 → C9（本機 apply_codegen 演練，不必為 staging 推遠端）
```

### 任意新電腦（有無引擎都一樣先做）

```
git clone → copy ue.local.env.example → 填 UE_ROOT（有引擎再填）
```

### 引擎可安裝之後（該機器第一次要編譯）

```
D1 → D2 → D3 → D4 → D5
        ↓
   E2 → E3 → E4 → E5
        ↓
   （可選）F1…
```

### 刻意不做

- F1 / 原 3.4 Cursor Hook：等 E5 穩定再考慮。

---

## 5. 換機檢查清單（GitHub 工作流）

每到一台新電腦：

1. `git clone <repo>`（若使用 LFS 資產：該機執行過 `git lfs install`）
2. `copy ue.local.env.example ue.local.env` → **只改 `UE_ROOT`**
3. 若要編譯：確認本機已有 UE + VS Build Tools（階段 D）
4. `.\Scripts\build_and_test.ps1`

**不要**把 `ue.local.env` 提交進 git。專案內容一律靠 GitHub 同步。

---

## 6. 與「非 Visual Studio IDE」方案的對應

| 需求 | 無 UE 時 | 有 UE 後 |
|------|----------|----------|
| 編輯 / AI 產碼 | Cursor | Cursor |
| 套用完整檔案 | `apply_codegen.ps1` | 同左 |
| 編譯 | 無法真實編譯 | `build_and_test.ps1`（UBT，不開 VS） |
| IDE 除錯 | 可先裝 Rider | Rider 開 `.uproject` |
| MSVC 工具鏈 | 可先裝 Build Tools | UBT 使用之 |

詳見先前討論：可不用 Visual Studio **IDE**，但 Windows 上通常仍需 **Build Tools**。

---

## 7. 進度速查（請隨手改狀態）

| 階段 | 進度 |
|------|------|
| A 基建 | **完成** |
| B GitHub / 換機 | 基建可推；B2/B3 待 commit+push |
| C 無引擎骨架 | 未開始（可做，做完繼續 push） |
| D 引擎 + 本機編譯前置 | **阻塞**（無 Launcher） |
| E 編譯驗證 | 等待 D |
| F 可選強化 | 延後 |

---

## 8. 下一刀建議（單一任務）

若只選一件「現在就做」：

**優先做階段 C（骨架進 repo）→ 再 B2/B3 push 到 GitHub。**  

之後任何機器都是 clone + 改 `UE_ROOT`；引擎一裝好跑 `build_and_test.ps1` 即可，不必重搭專案。

若要 Agent 直接產骨架，指定目標引擎版本字串（例如 `5.5`）即可。
