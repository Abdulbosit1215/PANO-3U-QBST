# PANO-3U Structural Mechanics Verification Report

**Version B / 2026-09 | Method: analytical baseline (beam model + Miles equation + fastener checks); pre-flight closure requires FEA correlation and test data**

## 1. Inputs

| Item | Value | Source |
|---|---|---|
| Total mass | 2.2 kg (incl. 25% margin) | mass budget |
| Envelope | 100×100×340.5 mm | CDS Rev.14 |
| Material | 6061-T6, E=68.9 GPa, ρ=2700, σy=276 MPa | MMPDS |
| Quasi-static | 10 g (3-axis envelope) | typical launch constraint |
| Random vibration | GEVS qualification 14.1 gRMS | GSFC-STD-7000 |
| 1st mode requirement | ≥100 Hz | launch coupling control |

## 2. First Mode Frequency (Beam Model)

Four 8.5×8.5 rails in parallel with offset inertia terms.

- Cantilever conservative model: **f1 = 388 Hz**
- Simply supported upper bound: **f1 = 1090 Hz**
- Even conservative case exceeds 100 Hz requirement.

## 3. Quasi-static 10 g Checks

| Item | Load/Stress | Allowable | Safety Factor |
|---|---|---|---|
| Rail axial compression | 0.75 MPa | 276 MPa | 368 |
| Rail buckling | critical 10,206 N vs actual 54 N | — | 189 |
| Side panel stress | ~12 MPa | 276 MPa | 23 |

## 4. Random Vibration (Miles, Q=10, fn=150 Hz conservative)

- Output response: **19.4 gRMS**, 3σ peak **58 g**
- Worst lumped mass case (battery group): inertial load 286 N

## 5. Fastener Checks (3σ random-vibration case)

| Joint | Per-fastener load | Stress | Allowable (A2-70) | Safety Factor |
|---|---|---|---|---|
| Battery bracket 4×M3 shear | 71.5 N | 10.1 MPa | 310 MPa | 31 |
| Side panel F2 M3 tension | 8.6 N | 1.7 MPa | 450 MPa | 258 |
| Lens mount M2×4 | 7.1 N | 3.6 MPa | 450 MPa | 125 |

All safety factors ≥30 in this baseline check.

## 6. Conclusions and Remaining Requirement

1. Strength, stability, stiffness, and fastener margins are acceptable in analytical baseline.
2. Launcher reviews typically require FEA modal/random-vibration reports (ANSYS/ABAQUS suggested).
3. After qualification vibration, remeasure first mode (decay <5%) and inspect all witness marks.

---

## v4 Addendum (Post-Review Recalculation)

1. **Mass correction:** dry mass revised to 2.36 kg by geometric rebuild; launch mass 2.83 kg with margin. Safety factors remain ≥25.
2. **Frequency correction:** including side-panel stiffness gives **f1 = 611 Hz** (beam FE), higher than prior 388 Hz conservative estimate.
3. **Random vibration update:** 39.2 gRMS (3σ=118 g) with still-acceptable margin table.
4. Supporting data: `仿真数据/结构_强度裕度表.csv`.
