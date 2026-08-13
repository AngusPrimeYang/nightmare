# Nightmare — 新 Session 精簡交接（2026-08-13）

> 開新 chat 時把本檔貼給 Agent，或說「依 docs/SESSION_HANDOFF.md 繼續」。

## 現況（一句）

**§7.3 流程／UI U1–U8 已落地**（主選單、體力歸零 popup、10 秒倒數）。下一刀：PIE 驗收或鏡頭／美術換皮。

## PIE 備註

- **LMB** 沿相機方向射長條灰盒子彈（spd≈6000、dmg≈5）；敵人 debug 顯示 `HP cur/max`
- 預設 HP roll **5–70**；dmg=5 時約 1–14 發擊殺
- P12 仍有效：Hover 球／Walk 方塊
- 牆要擋：`BP_GrayCube` → CubeMesh → **BlockAll** → Compile → Ctrl+S

## 站區高度

權威地面 `GS_LotPad`；`STATION_Z=100`；勿對齊 Landscape Z。

## 請你（下一 Session）

1. **PIE 驗 §7.3**：▶ 先見主選單 → 開始遊戲 → 等體力歸零看 popup／倒數／繼續／回選單
2. 鏡頭／美術換皮
