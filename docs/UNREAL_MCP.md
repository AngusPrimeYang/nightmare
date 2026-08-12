# Nightmare — 專案級 Unreal MCP（chongdashu/unreal-mcp）

> 更新日期：2026-08-12  
> 範圍：**僅本 repo**（`Plugins/UnrealMCP` + `.cursor/mcp.json`），不寫入 Cursor 全域設定。  
> 來源：https://github.com/chongdashu/unreal-mcp（Experimental）  
> 與 Loop Engineering：MCP 只協助 **Editor 組裝**；玩法對錯仍以 Spec + `build_and_test.ps1 -RunTests` 為準。

---

## 已落地的檔案

| 路徑 | 用途 |
|------|------|
| `Plugins/UnrealMCP/` | Editor C++ 外掛（TCP，預設 port **55557**） |
| `Tools/unreal-mcp-server/` | Python MCP server（`uv run unreal_mcp_server.py`） |
| `Tools/uv/dist/` | **原始** uv zip + CPython tar.gz（+ sha256） |
| `Tools/uv/bin/uv.exe` | 解壓後的可攜式 `uv` |
| `.cursor/mcp.json` | Cursor **專案級** MCP（`command` 指向 `Tools/uv/bin/uv.exe`） |
| `.cursor/mcp.json.example` | 範本 |

可選本機殘留：`Tools/unreal-mcp/`（完整 clone，已 gitignore；不需要可自行刪除）。

---

## 可攜式 uv（免 PATH／免 winget）

已下載並核對 SHA256，全部放在專案內：

| 路徑 | 內容 |
|------|------|
| `Tools/uv/dist/uv-x86_64-pc-windows-msvc.zip` | **原始** uv Windows zip |
| `Tools/uv/dist/uv-x86_64-pc-windows-msvc.zip.sha256` | uv 雜湊 |
| `Tools/uv/dist/cpython-3.12.13+20260805-x86_64-pc-windows-msvc-install_only_stripped.tar.gz` | **原始** CPython 3.12 壓縮檔 |
| `Tools/uv/bin/uv.exe` | 解壓後的可攜式 uv |
| `Tools/uv/python/` | `uv python install` 安裝的 3.12（gitignore） |
| `Tools/uv/cache/` | uv 套件快取（gitignore） |
| `Tools/uv/VERSION.txt` | 版本與來源 URL |

執行：

```powershell
D:\DDD\nightmare\Tools\uv\bin\uv.exe --version

cd D:\DDD\nightmare\Tools\unreal-mcp-server
$env:UV_CACHE_DIR = "D:\DDD\nightmare\Tools\uv\cache"
$env:UV_PYTHON_INSTALL_DIR = "D:\DDD\nightmare\Tools\uv\python"
..\uv\bin\uv.exe sync --python 3.12
```

`.cursor/mcp.json` 使用 `${workspaceFolder}/Tools/uv/bin/uv.exe` 與同前綴的 `UV_*`（勿用裸相對路徑；Windows 上 spawn cwd 不保證是 repo 根）。  
MCP server 固定 **Python 3.12**（見 `Tools/unreal-mcp-server/.python-version`；3.14 與目前 pydantic 不相容）。

---

## 啟用清單（待你本機執行）

1. `[x]` 可攜式 `uv` 已在 `Tools/uv/`（含原始 zip）
2. `[x]` CPython 3.12 原始 tar.gz 已在 `Tools/uv/dist/`
3. `[x]` `Tools/unreal-mcp-server` 已 `uv sync --python 3.12` 成功
4. `[x]` Editor 已關閉狀態下完成外掛編譯（無 Live Coding 鎖）
5. `[x]` `NightmareEditor` + **UnrealMCP** 已編譯成功（UE 5.8 API 修補：`FindFirstObject` / PNG / BufferSize）
6. `[x]` Editor 開著且 **UnrealMCP TCP 55557 Listen**（已驗證）
7. `[x]` Reload Cursor Window（已讀到 `project-0-nightmare-unrealMCP`）
8. `[x]` **Customize → MCPs** 啟用 `unrealMCP`（off→on → `ready`；Agent 工具可用）
9. `[x]` 煙霧：TCP list/spawn + Cursor `get_actor_properties`（`NightmareMcpSmokeCube`）皆 **成功**

### 保留的 TCP 煙霧指令

```powershell
Tools\uv\bin\uv.exe --directory Tools\unreal-mcp-server run --python 3.12 `
  python scripts\actors\nightmare_mcp_smoke.py list

Tools\uv\bin\uv.exe --directory Tools\unreal-mcp-server run --python 3.12 `
  python scripts\actors\nightmare_mcp_smoke.py spawn
```

腳本：`Tools/unreal-mcp-server/scripts/actors/nightmare_mcp_smoke.py`  
新 session 交接：`docs/SESSION_HANDOFF.md`

### UE 5.8 外掛修補（本 repo）

`Plugins/UnrealMCP` 相對上游 chongdashu 已做本機相容修改，否則無法在 5.8 編譯：

- `ANY_PACKAGE` → `FindFirstObject<UClass>`
- `CompressImageArray` → `PNGCompressImageArray` + `TArray64`
- 全域 `BufferSize` 更名（避免與引擎 StringConv 衝突）
- TCP **one-shot**：回覆後關閉 client，避免 `CLOSE_WAIT` 卡死 Accept（Python 端每指令重連）
- `create_blueprint` parent 亦嘗試 `/Script/Nightmare.*` + `FindFirstObject`；**`ACharacter` 改為 StaticClass 直取**（避免誤建成 Actor）
- 新增 TCP／MCP 指令 **`rename_blueprint`**（`old_name` / `new_name`，路徑 `/Game/Blueprints/`）
- 重建腳本：`Tools/unreal-mcp-server/scripts/actors/nightmare_recreate_dev_pawn.py`

完成後可把 `docs/EDITOR_DEV_SETUP.md` 步驟 9 路徑 C 的 chongdashu 列勾成 `[x]`。

---

## 常見問題

| 問題 | 處理 |
|------|------|
| Cursor 找不到 uv／MCP disconnected | 確認 `mcp.json` 用 `${workspaceFolder}/...`；**Customize → MCPs** 啟用 `unrealMCP`；必要時完全重開 Cursor |
| 連不上 Editor | Editor 開著、外掛已編譯；看 Output Log／server log |
| 編譯 UnrealMCP 失敗 | VS Build Tools（階段 D）；關 Live Coding 後重編 |
| 與官方 UE 5.8 MCP 衝突 | 可先只用社群版 |

**下一刀：** 玩法 **P4+ 敵人**（C++／Spec）。G1 用多 `GS_*` + **`GS_LotPad`**——**勿再組巨型單 BP**；站區高度以地坪為準（見 `GAMEPLAY_SLICE` §7.1.1）。玩法對錯仍不依賴 MCP。

常用腳本（Editor + MCP TCP `55557`）：

| 腳本 | 用途 |
|------|------|
| `scripts/actors/nightmare_g1_ground_station.py` | 建／對齊 `GS_LotPad` + 移站體／燈／PlayerStart |
| `scripts/actors/nightmare_g1_spawn_meshes.py` | 重刷 `GS_*`（`STATION_Z=100`） |
| `scripts/actors/nightmare_g1_fill_lights.py` | 店內 3×3 燈 |

---

## 更新外掛

```text
git clone --depth 1 https://github.com/chongdashu/unreal-mcp.git Tools/unreal-mcp
# 再覆蓋 Plugins/UnrealMCP 與 Tools/unreal-mcp-server
```
