# 🛰️ PANO-3U CubeSat | Dual-Fisheye Panoramic CubeSat — Full Open Design & Manufacturing Package

**3U CubeSat · 500 km SSO · 360°×180° dual-fisheye panoramic imaging · Full open design & manufacturing package**

> A satellite that can be built in a garage: from requirements and GB production drawings, to parametric STEP models, KiCad design files, BOM/process/test specs, and flight operations/safety packages.

**License: CC BY-NC 4.0** · Author: **Zhang Jiahao**

---

## ✨ Mission Overview

| Item | Specification |
|---|---|
| Configuration | 3U (100×100×340.5 mm), Cal Poly CDS Rev.14, P-POD compatible |
| Mission | Opposed dual-fisheye side imaging, 360°×180° panoramic output, 5760×2880 |
| Payload | 2× Sony IMX477 + M12 fisheye f1.4/F2.4 FOV ≥195° (equidistant projection) |
| Video | 4K30 (CAM0) + 2.8K30 (CAM1), dual-camera sync <1 ms |
| Orbit | 500 km SSO (i=97.4°), design life ≥1 year |
| Data Link | UHF 437 MHz, GFSK 57.6 kbps (10.7 dB margin), AX.25/KISS |
| Power | 72-cell solar array, 18.4 W BOL, 4-channel MPPT, 2S2P 49 Wh |
| ADCS | B-dot detumbling (17.3°/s → 2°/s @2.3 h) + coarse magnetic attitude ±5° |
| Mass/Structure | 2.83 kg (with 20% margin), first mode 611 Hz, all SF ≥28 |

## 📁 Repository Layout

```
00_包总览/          Standards index · baseline/config index · requirements traceability
01_系统设计文件/    Requirements · detailed design · ICD · electrical spec · structure/thermal/reliability/EMC/FMEA
02_评审与质量/      Design review · data consistency audit · model/data verification
03_生产图纸_GB23页/  ★ 23-page GB mechanical production drawing package (PDF/PNG/SVG)
04_三维模型_STEP/    ★ 15 parametric reconstructed solids (parts + assembly, AP214)
05_电气设计_KiCad/   Four PCB outline templates + fabrication checklist
06_物料清单/         PL-001 parts list + BOM v2 procurement sheet
07_生产与装配工艺/   Manufacturing plan + assembly work instructions
08_检验与试验/       Inspection & test spec (GEVS vibration + TVAC)
09_软件/             Payload software (CM4) · OBC firmware (STM32) · ground station (Python)
10_发射与运营/       Flight operations · ground ops manual · deorbit plan · launch safety package
99_归档/             Raw source materials + superseded sketch set (not for production)
```

## 🚀 Quick Start

- **See the complete spacecraft design quickly** → `03_生产图纸_GB23页/` and `04_三维模型_STEP/装配/`
- **Reproduce for education** → `00_包总览/` → `01_系统设计文件/` → `07_生产与装配工艺/` → `08_检验与试验/`
- **Study systems engineering flow** → start from `SPC-001` + `ANA-001` + `TRC-001`
- **Preparing for launch** → close open review actions in `REV-001`, then follow `SAF-001`

## 📐 Standards

Cal Poly **CDS Rev.14** · NASA **GSFC-STD-7000B (GEVS)** · **NASA-STD-7001A** ·
**SpaceX Rideshare PUG** · **ECSS** E-ST-10-03C / Q-ST-30-11C · **GJB** 1027A / 151B / Z 299C / Z 35 ·
**ISO 24113** · **ITU/IARU** frequency coordination · UN 3481 battery transport

## ⚠️ Disclaimer

All values are design/simulation values. This package is for education and research. Flight use requires completion of all open actions in the documents (FEA recheck, lens testing, TVAC, software integrated testing, licensing) and launcher safety approval.

## 📄 License

**CC BY-NC 4.0**
- ✅ Sharing and adaptation allowed (education/research/personal use)
- ❌ Commercial use not allowed
- 📌 Attribution required: Zhang Jiahao + repository link + change notice

See [LICENSE](LICENSE) and [NOTICE](NOTICE).
