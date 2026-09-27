# PANO-3U Dual-Fisheye Panoramic CubeSat Detailed Design Description (Engineering Build Edition)

**Version B / 2026-09 | Granularity: parts/screws/holes, directly manufacturable**

Supporting files:
- 3D models: `STEP模型/`
- Software package: `软件包/`
- Structural verification: `PANO-3U结构力学校核报告.md/.docx`
- Electrical design: `PANO-3U电子系统设计规格书.md/.docx`
- Drawing package: `图纸/3U全景立方星全套图纸_v4评审修订版.pdf`
- DXF manufacturing files: `图纸/DXF加工文件/`
- BOM: `PANO-3U_全级BOM清单.xlsx`

---

## Part I — Overall System

### 1.1 Mission and Targets

3U CubeSat (100.0×100.0×340.5 mm, Cal Poly CDS Rev.14), dual-fisheye side-opposed panoramic imaging, 500 km SSO, panoramic output 5760×2880; video 4K30 (camera 1) + 2.8K30 (camera 2), full-resolution still capture 4056×3040 on both cameras.

Data-link baseline (v4 simulation): UHF 9.6 kbps daily throughput is limited, so default upgraded to 57.6 kbps (link margin ≥10.7 dB). 4K video is stored onboard (512 GB card) and downlinked as selected frames/previews unless upgraded to S-band.

Total launch mass: 2.83 kg (dry mass by geometric rebuild + 20% growth margin).

### 1.2 Z-axis Layout

| Z range (mm) | Content |
|---|---|
| 0–30 | −Z endplate (separation switch/RBF interface) + stack base |
| 30–123 | Electronics stack: UHF → EPS → partition → OBC → CM4 carrier |
| 123–227 | Battery bay: 4×18650 (2S2P), symmetric on ±Y sides |
| 150–200 on ±X | Camera module 1/2 on ±X side plates |
| 227–340.5 | +Z bay/wiring margin + +Z endplate |

---

## Part II — Mechanical Design

### 2.1 Machined Parts Summary
Includes MEC-101/102/103A/103B/104/105/106/107/108/109/110/111 with material, quantity, mass, and tolerance constraints matching drawing package and BOM.

### 2.2 Endplate Hole Coordinates
Defines common and −Z additional interface holes (including separation switches and RBF), tolerance ±0.05 mm.

### 2.3 Side Plate Features
- K-series rail fixing holes with countersink requirements
- Camera window on MEC-103A with threaded M2 mounting points
- Routing/pass-through holes and panel keep-out treatment

### 2.4 Rail Requirements
- Section 8.5×8.5 mm, total length 340.5±0.1
- Straightness/twist/perpendicularity constraints
- Hard anodizing and post-process constraints

### 2.5 Fastener Table
All F1–F14 fastener types, quantities, torques, and thread-lock requirements as listed in BOM and drawing process docs.

Assembly rule highlights:
- Use Loctite 243 for threaded joints (except locking nuts)
- Torque tool calibration before use
- Witness marking after tightening
- Cross-pattern two-stage torqueing

---

## Part III — Payload (Dual-Fisheye Camera)

### 3.1 Optical Fixed Values
- Sensor: Sony IMX477 (4056×3040, 1.55 µm)
- Lens: M12×0.5, f=1.4 mm, F2.4, FOV ≥195°
- Output: 5760×2880 equirectangular panorama
- Overlap zone: 15°
- IR-cut filter and mechanical datum chain explicitly defined
- Camera placement on ±X side plates, outward extension ≤6.0 mm (within CDS side protrusion limit)

### 3.2 Calibration and Stitch Pipeline
1. Factory calibration with rotating checkerboard and intrinsic/extrinsic solve.
2. In-orbit capture + onboard encode + preview generation.
3. Ground stitching with feature matching and multi-band blending.
4. Periodic in-orbit overlap-based drift correction.

---

## Part IV — Electrical and Electronics

### 4.1 Stack Order
From −Z to +Z: E9 comm → E8 EPS → partition → E7 OBC → E2 CM4 carrier. Shared stack bus through 2×20P pins. PCB corners include 45° relief cuts to avoid rail interference.

### 4.2 Harness Rules
Power 22AWG, signal 28AWG, RF RG178, locked JST connectors, separated bundle routing, strain relief at fixed intervals.

### 4.3 Separation/RBF Safety Loop
BAT+ → BMS → SW1 → SW2 → SW3 → RBF → EPS main input; NC switch logic enforced so in-P-POD compressed state stays power-off, and post-separation release enables auto power-on.

---

## Part V — Power/Thermal/ADCS

- Solar array: 72 cells, 18.4 W BOL with independent MPPT channels
- Battery: 4×NCR18650B (2S2P), 49 Wh
- SAFE mode thermal revision: MLI + 3 W heater + LT3652 NTC charge lockout below 0°C
- ADCS: B-dot detumble to ≤2°/s, coarse attitude ±5°, no reaction wheel required for panoramic mission

---

## Part VI — Assembly and Verification

12-step assembly flow (A1–A12), ESD and curing requirements, torque acceptance, insulation checks, mass and CoM acceptance windows.

Validation includes vibration (GEVS), modal retest, bolt witness check, TVAC cycles with dual-camera operation checks at hot/cold soak points.

---

## Part VII — Cost

Hardware subtotal about ¥20,699; with test and ground station costs, full spacecraft estimate about ¥45,700 (excluding launch).

Version C (v4 review update): side-mounted cameras, NC separation logic, PCB/partition corner relief cuts, CM4 branch TPS2557, 72-cell array, and synchronized updates across DXF/drawings/BOM.
