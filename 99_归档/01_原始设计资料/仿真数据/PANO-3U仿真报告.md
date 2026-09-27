# PANO-3U Full Simulation Data Report

**Version v1 / 2026-09 | Design baseline: Version C (drawing v4)**

**Method note:** simulations are Python-based numerical analyses (orbital mechanics / structural approximation / lumped thermal network / attitude dynamics) using version C inputs. CSV raw data is in the same folder. This does not replace launcher-required formal FEA or TVAC closure.

---

## S0 Mass Properties

| Metric | Value | Note |
|---|---|---|
| Dry mass | **2359 g** | v4 corrected from early underestimation |
| Launch mass (+20%) | **2.83 kg** | below 3U 4.8 kg limit |
| CoM | (0.0, −0.2, 146.7) mm | favorable z-offset |
| Inertia | Ixx=0.0168 / Iyy=0.0173 / Izz=0.00271 kg·m² | includes parallel-axis contribution |

## S1 Orbit (500 km SSO)

- ~15.2 orbits/day.
- Worst eclipse (β=0): 35.7 min/orbit.
- Beijing station (10° elevation mask): 20 passes over 7 days (~2.9/day), average 6.2 min.

## S2 Power

- Daily generation: 81.9 Wh vs daily consumption: 30 Wh, margin 2.73×.
- 7-day time-domain simulation shows SOC 80%–100%, minimum DoD 20%.

## S3 Thermal

| Case | Core | Panel | Battery | Result |
|---|---|---|---|---|
| Hot case | 15~20°C | 15~19°C | 15~20°C | pass |
| Cold case (original) | −24~3°C | −26~3°C | −21~6°C | fail charging temp limit |
| Cold case (revised) | −20~8°C | −22~7°C | −12~+8°C | mitigated (heater duty 79%) |

Design revision used in docs: battery MLI + heater 2W→3W + LT3652 NTC charging lockout below 0°C.

## S4 Structure

- First mode **f1 = 611 Hz** using updated structural model.
- GEVS random vibration margin table shows all key SF values above required threshold.

## S5 ADCS Detumble

- B-dot control reduces 17.3°/s initial rate to ≤2°/s in ~2.3 h.
- 24 h result ~0.13°/s.

## S6 Link Budget

| Rate | Min Margin | Daily Throughput | Verdict |
|---|---|---|---|
| 9.6 kbps | 18.7 dB | 0.8 MB | telemetry-only practical |
| **57.6 kbps** | **10.7 dB** | **4.6 MB** | recommended default |
| 115.2 kbps | 7.7 dB | 9.2 MB | optional |

4K video cannot be operationally downlinked via UHF; S-band upgrade required for video-grade downlink.

## S7 Optics

- Equidistant angular resolution ~15.8 px/°.
- Stitch overlap band and output resolution are consistent with design targets.

---

## Consolidated Verdict

| Domain | Result |
|---|---|
| Mass/envelope | pass |
| Orbit/pass opportunities | pass |
| Power | pass |
| Thermal | conditional pass after revision, TVAC closure required |
| Structure | pass |
| ADCS | pass |
| Link | conditional pass with 57.6 kbps default |
| Optics | pass |

**Model disclaimer:** orbit uses simplified 2-body+J2; magnetic field uses tilted dipole; thermal is 6-node lumped network; structure is reduced-order beam FE. Use high-fidelity tools for final qualification.
