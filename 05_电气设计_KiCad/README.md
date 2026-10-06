# PANO-3U KiCad Project README

**Version B / 2026-09** | Open with KiCad 7.0+ using `PANO-3U.kicad_pro`

## Project Structure

| File | Content |
|---|---|
| `PANO-3U.kicad_pro` | Project file (open directly) |
| `PANO-3U.kicad_sch` | Root schematic: hierarchical entry for four boards |
| `eps.kicad_sch` | E8 EPS power board |
| `obc.kicad_sch` | E7 OBC main control board |
| `comm.kicad_sch` | E9 UHF communication board |
| `cm4_carrier.kicad_sch` | E2 CM4 carrier board |
| `PANO-3U.kicad_sym` | Project symbol library (18 custom IC symbols) |
| `sym-lib-table` | Library mapping (preconfigured) |
| `预览图/` | PNG previews of schematics |

## Design Conventions

- Four schematics map to four independent PCBs (each 90×90 mm, 4-layer), interconnected via J1 2×20P stack bus.
- Net connectivity is label-based: same net label means same electrical net.
- J1 pin definitions are aligned with the electrical system specification.

## ⚠️ Mandatory Pre-Fabrication Checklist

1. Custom symbol pin numbers are logical IDs. Before layout, map every symbol pin to the correct package pad according to datasheets.
2. Validate critical parts carefully (e.g., LT3652, STM32F405RGT6, RA07H4452M, PE4259, Si4463 modules).
3. Run ERC and resolve all errors before PCB layout.
4. Power integrity targets:
   - 5 V traces ≥0.5 mm (or copper pour)
   - VBAT traces ≥0.8 mm
   - MIPI differential pairs 100 Ω, length matching within ±0.5 mm
5. CM4 dual-camera constraints:
   - CAM1 operates as 2-lane
   - Camera 2 D2/D3 are disconnected as specified
   - Validate sustained 4K30 bandwidth on Raspberry Pi platform before production

## Drawing/BOM Consistency

Net names and reference designators in this project must stay aligned with:
- full-system BOM workbook
- electrical system design specification

If any one of these changes, synchronize all related documents.
