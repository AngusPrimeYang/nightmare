# Nightmare — 新 Session 精簡交接（2026-08-12）

> 開新 chat 時把本檔貼給 Agent，或說「依 docs/SESSION_HANDOFF.md 繼續」。

## 現況（一句）

物品 P1–P3／P8–P11 與 G1 加油站灰盒已可用；站區踩 **`GS_LotPad`**（勿再跟 Open World 地景 Z 對賭）。下一刀：**P4+ 敵人**。

## 站區高度（已拍板）

| 項目 | 約定 |
|------|------|
| 權威地面 | `GS_LotPad`（`BP_GrayPad`，BlockAll） |
| 站體世界 Z | `STATION_Z=100`（地坪頂；`GS_*` 相對座標 + 100） |
| 燈 | `StoreCeilLight_0..8` @ `STATION_Z+290` |
| 還原腳本 | `Tools/unreal-mcp-server/scripts/actors/nightmare_g1_ground_station.py` |
| 禁區 | 不要為了出生／防陷去抬整座站或改對齊 Landscape actor Z |

症狀回憶：編輯器全載入地景 vs PIE 串流高度不同 →「預覽埋土、▶懸空」。地坪有碰撞後兩邊一致。

## 請你（下一 Session）

1. 依 `docs/GAMEPLAY_SLICE.md` §7.2 做 **P4**（敵人世界邊緣隨機生成）+ Spec
2. 關 Editor 後若尚未編譯：`NightmareDevGameMode` 的 ItemSpawner／SafeSpawn Z 已對地坪上方
