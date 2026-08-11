# Nightmare — 專案級 Unreal MCP（chongdashu/unreal-mcp）

> 更新日期：2026-08-11  
> 範圍：**僅本 repo**（`Plugins/UnrealMCP` + `.cursor/mcp.json`），不寫入 Cursor 全域設定。  
> 來源：https://github.com/chongdashu/unreal-mcp（Experimental）  
> 與 Loop Engineering：MCP 只協助 **Editor 組裝**；玩法對錯仍以 Spec + `build_and_test.ps1 -RunTests` 為準。

---

## 已落地的檔案

| 路徑 | 用途 |
|------|------|
| `Plugins/UnrealMCP/` | Editor C++ 外掛（TCP，預設 port **55557**） |
| `Tools/unreal-mcp-server/` | Python MCP server（`uv run unreal_mcp_server.py`） |
| `.cursor/mcp.json` | Cursor **專案級** MCP 設定 |
| `.cursor/mcp.json.example` | 範本（與上相同；換機可對照） |

可選本機殘留：`Tools/unreal-mcp/`（完整 clone，已 gitignore；不需要可自行刪除）。

---

## 啟用清單（待你本機執行）

1. `[ ]` 安裝 [uv](https://github.com/astral-sh/uv)（若尚未）：`winget install astral-sh.uv` 或見官網
2. `[ ]` 在 `Tools/unreal-mcp-server` 執行一次：`uv sync`（或 `uv pip install -e .`）
3. `[ ]` **關閉** Unreal Editor（若開著）
4. `[ ]` 右鍵 `Nightmare.uproject` → Generate Visual Studio project files（可選，有時外掛編譯會自動觸發）
5. `[ ]` 重新開啟專案；若提示編譯外掛 → 選 Yes
6. `[ ]` Edit → Plugins → 確認 **UnrealMCP**、**Editor Scripting Utilities** 已啟用
7. `[ ]` 重開 **Cursor** 工作區（或 Reload Window），讓 `.cursor/mcp.json` 生效
8. `[ ]` Cursor → MCP／工具列表中應出現 `unrealMCP`；**Editor 必須開著** 工具才連得上
9. `[ ]` 煙霧：請 Agent「列出目前關卡所有 Actor」或「在 Player Start 旁放一個 Cube」

完成後可把 `docs/EDITOR_DEV_SETUP.md` 步驟 9 路徑 C 的 chongdashu 列勾成 `[x]`。

---

## 常見問題

| 問題 | 處理 |
|------|------|
| Cursor 找不到 `uv` | 把 uv 加進 PATH，或把 `mcp.json` 的 `command` 改成 uv 絕對路徑 |
| 連不上 Editor | 確認 Editor 已開、外掛已編譯載入；看 Output Log／`Tools/unreal-mcp-server` 日誌 |
| 編譯 UnrealMCP 失敗 | 確認已有 VS Build Tools（階段 D）；關閉 Live Coding 後重編 |
| 與官方 UE 5.8 MCP 衝突 | 可先只用社群版；兩者都是 Experimental，勿同時當唯一真相 |

---

## 更新外掛

```text
# 可選：重新 shallow clone 上游後再複製
git clone --depth 1 https://github.com/chongdashu/unreal-mcp.git Tools/unreal-mcp
# 再覆蓋 Plugins/UnrealMCP 與 Tools/unreal-mcp-server
```
