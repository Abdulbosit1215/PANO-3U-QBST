# Translation Policy (English Migration)

Version: 2026-09-27

## Scope
This policy defines how Chinese-language repository content is translated to English while preserving engineering traceability.

## Rules
1. Keep all technical IDs, part numbers, drawing numbers, and revision tags unchanged.
2. Keep existing file/folder paths unchanged unless a mapped migration is explicitly approved.
3. Preserve command lines, protocol constants, numeric limits, and hardware interfaces exactly.
4. Prefer full English translation for user-facing Markdown in active folders.
5. For archived/reference materials, translation may be deferred and tracked in the inventory.
6. If ambiguity exists, preserve the original term once in parentheses on first use.

## Consistency Baseline
Use these canonical terms consistently:
- 立方星 → CubeSat
- 全景图 → panoramic image
- 载荷计算机/CM4 → payload computer (CM4)
- 主控/OBC → on-board computer (OBC)
- 地面站 → ground station
- 下行/上行 → downlink/uplink
- 断点续传 → resume transfer

## Safety Constraints
- No path renaming during this phase.
- No binary document rewrites in this phase (.docx/.xlsx/.pdf/.png/.svg text layers).
- Translation changes must not alter executable behavior.
