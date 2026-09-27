# PANO-3U Design Review Report (Error and Risk Register)

**Version v1 / 2026-09 | Scope: version B full deliverables**

**Summary:** 3 blocker-level issues, 3 high-risk issues (all closed in version C / drawing v4), 7 medium/low risks (2 fixed, 5 mitigated), and 6 open pre-flight actions.

---

## 1) Blocker-Level Issues (Closed)

### E1 Camera on ±Z endplate violated CDS and optical geometry
- **Issue:** ±Z endplate protrusion not allowed by CDS; recessed lens geometry also clipped 195° FOV.
- **Resolution:** moved cameras to ±X side faces with ≤6.0 mm protrusion; introduced MEC-111 and MEC-103A updates; adjusted array count and closed power balance.

### E2 Separation switch logic inverted
- **Issue:** original logic could energize spacecraft in launcher volume after RBF removal.
- **Resolution:** changed to NC wiring: compressed state = open/off; released state = close/on.

### E3 90×90 PCB/partition rail interference
- **Issue:** corners intersected rail envelope.
- **Resolution:** all PCBs and MEC-105 updated with 45° corner relief cuts.

---

## 2) High-Risk Issues (Closed)

### E4 CM4 branch switch rating exceeded
TPS2553 could not meet 3 A branch target. Replaced U20 with TPS2557; other branches kept TPS2553.

### E5 Camera 2 4K30 infeasible on CAM1 bandwidth
Updated target: CAM0 4K30, CAM1 2.8K30 or full-res 13 fps; still image requirements unchanged.

### E6 M12 lens interface mismatch with HQ camera
Added M12→CS adapter and post-assembly focus verification requirement.

---

## 3) Medium/Low Risks

| ID | Issue | Level | Action |
|---|---|---|---|
| R1 | KISS frame parsing slice bug in ground receiver | Medium | Fixed |
| R2 | uptime field used epoch seconds | Low | Fixed |
| R3 | MPU-9250 EOL | Medium | Alternative parts listed |
| R4 | deployed UHF element enters edge FOV | Low | accepted with mask handling |
| R5 | SAFE mode heater disable risk | Medium | Fixed to keep battery heating enabled |
| R6 | RF deploy-delay vendor differences | Low | parameterized in firmware |
| R7 | reduced ±X solar cells after side camera change | Low | energy margin still validated |

## 4) Open Actions (Required Before Flight)

1. FEA formal verification (launcher requirement)
2. Lens physical FOV/distortion validation
3. Radiation assessment for COTS stack
4. TVAC test closure
5. EMC and radio licensing
6. Full hardware-in-loop software integration

## 5) Key Recheck Data

- Link budget margin at 437 MHz remains adequate.
- Energy margin remains >2× after v4 updates.
- CDS compliance restored for end-face protrusion rules.
- PCB-rail clearance achieved after corner cut updates.

## 6) Version C (v4) Updated Deliverables

Synchronized updates across design description, v4 drawings, DXF, STEP assemblies, KiCad power component changes, BOM updates, and software fixes.

**Conclusion:** blocker/high-risk items are closed; prototype fabrication is acceptable; flight hardware release should wait until open actions are completed.
