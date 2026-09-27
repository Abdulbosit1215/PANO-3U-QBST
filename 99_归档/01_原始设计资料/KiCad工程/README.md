# PANO-3U KiCad Project README

**Version B / 2026-09** | Open with KiCad 7.0+ using `PANO-3U.kicad_pro`

## Project Structure

| File | Content |
|---|---|
| `PANO-3U.kicad_pro` | Project file (open directly) |
| `PANO-3U.kicad_sch` | Root schematic: hierarchical entry for 4 boards |
| `eps.kicad_sch` | E8 EPS board (47 parts: LT3652×4 MPPT, LTC4412 ideal diode, TPS54331 5V/3A, TLV70233, TPS2553×4 e-fuse, INA226×4 telemetry) |
| `obc.kicad_sch` | E7 OBC board (22 parts: STM32F405, TPS3823 watchdog, W25Q128, DS3231, MPU9250, QMC5883L, DRV8837×3) |
| `comm.kicad_sch` | E9 UHF comm board (Si4463 module + RA07H4452M 1W PA + PE4259 T/R switch + SAW filter + 30MHz TCXO) |
| `cm4_carrier.kicad_sch` | E2 CM4 carrier (CM4 connector, dual 22P camera FPC, TPS22965 load switch, microSD) |
| `PANO-3U.kicad_sym` | Project symbol library (18 custom IC symbols) |
| `sym-lib-table` | Library mapping (preconfigured, do not delete) |
| `预览图/` | Four schematic preview PNGs |

## Design Conventions

- Four schematics correspond to four independent PCBs (each 90×90 mm, 4-layer, Tg150, ENIG), interconnected by J1 2×20P stack bus.
- Connectivity is label-based: same label means same net.
- J1 pin definitions are aligned with the Electrical System Design Specification §0.

## ⚠️ Mandatory Checklist Before PCB Fabrication

1. **Custom symbol pin IDs are logical IDs** (18 symbols in PANO-3U library). Before layout, map each symbol pin to the correct package pad from datasheets.
   - LT3652 (actual MSOP-12EP package; DFN16 placeholder in schematic must be replaced)
   - STM32F405RGT6 (actual LQFP-64 pin numbering)
   - RA07H4452M / PE4259 / Si4463 module pins
2. R/C/L/D/connectors use **official KiCad symbols**, so pin mapping is correct by default.
3. Run **ERC** and resolve all issues before PCB layout.
4. Power integrity: 5 V traces ≥0.5 mm (or copper pour), VBAT ≥0.8 mm; MIPI differential pair 100 Ω with length matching ±0.5 mm.
5. CM4 dual-camera: CAM1 is 2-lane; camera 2 D2/D3 are disconnected per spec. Verify 4K30 bandwidth on Raspberry Pi platform before production; if insufficient, switch to CM5 or USB3 camera backup path.

## Consistency with Drawings and BOM

Net names and reference designators in this project correspond line-by-line with `PANO-3U_全级BOM清单.xlsx` (E-series IDs) and the Electrical System Design Specification. If one changes, synchronize all related sources.
