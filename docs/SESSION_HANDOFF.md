# Nightmare — 新 Session 精簡交接（2026-08-12）

> 開新 chat 時把本檔貼給 Agent，或說「依 docs/SESSION_HANDOFF.md 繼續」。

## 現況（一句）

加油站改踩 **`GS_LotPad`（有碰撞灰地坪）**，不再跟 Open World 地景串流高度對賭（那會造成預覽埋土、▶懸空）。**請 Ctrl+S → PIE 確認站在灰地上、建築不懸空。**

## 請你

1. **Ctrl+S**
2. PIE：人物應站在灰色大地坪上；建築貼地坪
3. 若地坪仍被土蓋住：在 Viewport 把 `GS_LotPad` 略抬高再存
4. 關 Editor 後編譯（`ItemSpawner`／SafeSpawn Z 已改到地坪上方）
