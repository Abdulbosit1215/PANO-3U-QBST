# PANO-3U Electrical System Design Specification (PCB Schematic Basis)

**Version B / 2026-09 | Depth: component-level connectivity + full connector pin map for direct schematic/layout work**

Common board baseline: 90×90 mm, 4-layer FR4 Tg150, 1 oz copper, ENIG; 4×Ø3.2 mounting holes (76×76 pattern); inter-board stack bus via J1 2×20P 2.54 mm gold-plated pin headers. Suggested stackup: Top(signal) / GND / PWR / Bottom(signal).

## 0. Stack Bus J1 (2×20P) Pin Definitions (shared by all boards)

| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 1 | GND | 2 | GND |
| 3 | VBAT (7.4 V) | 4 | VBAT |
| 5 | +5V | 6 | +5V |
| 7 | +3V3 | 8 | +3V3 |
| 9 | I2C1_SDA | 10 | I2C1_SCL |
| 11 | UART_OBC_TX | 12 | UART_OBC_RX |
| 13 | UART_CM4_TX | 14 | UART_CM4_RX |
| 15 | CAN_H (reserved) | 16 | CAN_L (reserved) |
| 17 | DEPLOY_EN (hot-cutter enable) | 18 | ANT_STAT |
| 19 | SAFE_MODE | 20 | WD_KICK |
| 21 | MAG_X_PWM | 22 | MAG_Y_PWM |
| 23 | MAG_Z_PWM | 24 | HEAT_EN |
| 25 | SW_STAT | 26 | PAYLOAD_EN |
| 27–40 | GND/reserved | | |

Each board only routes required local nets, but the header footprint/pins must remain fully through-connected for stack mechanics.

---

## 1. E8 EPS Power Board

### 1.1 Functional Path
4 solar faces (11.4 V/0.45 A each) → LT3652×4 (independent MPPT charging) → LTC4412 ideal-diode ORing → 2S battery (through BMS + 3 separation switches + RBF) → VBAT bus → TPS54331 (5 V/3 A) → TLV70233 (3.3 V/2 A).

### 1.2 Component-Level Notes

- **LT3652 (U1–U4, one per solar side)**: MPPT set to 11.4 V, float 8.4 V, charge current 0.5 A.
- **Switch/RBF chain**: BAT+ → BMS P+ → J2.1 → SW1 → SW2 → SW3 → J3 RBF → VBAT.
- **DC/DC**:
  - TPS54331 (U6): VBAT to 5 V/3 A, L=10 µH (sat current ≥4 A)
  - TLV70233 (U7): 5 V to 3.3 V/2 A (or second TPS54331 rail)
  - Branch e-fuses: TPS2553 class rails with current limits by branch requirements
- **Telemetry**: INA226×4 on I2C1 (0x40/41/44/45) for VBAT and branch currents.
- **Connectors**:
  - J4/J5/J6/J7: JST XH-2P solar inputs
  - J2: separation switch chain
  - J3: RBF socket
  - J8: 5 V output to CM4 carrier
  - J9: heater/temperature bundle

### 1.3 Layout Highlights
Keep high-current switching loops short; isolate INA226 sensing from noisy switch nodes; place TVS on solar inputs; tie edge copper to structure ground with single-point RC path (1 MΩ + 10 nF).

---

## 2. E7 OBC Board

### 2.1 Key Devices and Interfaces

- **STM32F405RGT6 (U1)** with 8 MHz HSE and 32.768 kHz RTC crystal.
- **I2C1** to INA226×4, QMC5883L, MPU-9250.
- **UART1** to comm board (9600 8N1).
- **UART2** to CM4 board (115200 8N1).
- **TIM/PWM** outputs to DRV8837×3 magnetorquers.
- **GPIO** assignments include HEAT_EN, DEPLOY_EN, ANT_STAT, SW_STAT, PAYLOAD_EN, WD_KICK.
- **SPI2** to W25Q128JV for firmware image/parameter backup.
- **1-Wire** bus to DS18B20×6.
- **RTC** DS3231 moved to I2C2 to avoid 0x68 address collision.

Power: J1 3V3 input, with standard decoupling per VDD/VDDA.

### 2.2 Software Interfaces
- 1 Hz telemetry logging and downlink framing.
- Commands: mode switching, imaging schedule upload, hot-cutter release, parameter table update.
- Safe mode trigger: low battery or watchdog reset, with payload/heater power protection logic.

---

## 3. E9 UHF Communication Board

- DRF4463/Si4463 RF path, SPI/control linked with OBC.
- 1 W PA stage (RF2126 or RA07H4452M), 5 V rail via e-fuse.
- PE4259 T/R switching + SAW band filtering near antenna feed.
- SMA-KY antenna connector + RG178 cable.
- 30 MHz TCXO reference for frequency stability.

---

## 4. E2 CM4 Carrier Board

- CM4 high-density connector (2×100P DF40C-100DP).
- Dual 22P CSI camera connectors (J20/J21).
- 5 V/3 A rail input and required protection.
- UART link to OBC on J1.13/14.
- microSD for payload data, eMMC for OS/system.
- Thermal path: CM4 thermal pad → aluminum block → M3 standoffs → frame.
- Dual-camera sync path with shared GPIO trigger target (<1 ms sync).

---

## 5. Sensor and Actuator Interfaces

| Device | Interface | Power | Note |
|---|---|---|---|
| MPU-9250 | I2C1 0x68 | 3V3 | On OBC board |
| QMC5883L | I2C1 0x0D | 3V3 | Keep ≥40 mm from magnetorquer |
| DS18B20 ×6 | 1-Wire PC6 | 3V3 | battery/camera/board thermal points |
| DRV8837 ×3 | TIM1 CH1–3 | 5 V | coil current ≤0.8 A |
| Heaters ×2 | HEAT_EN | 5 V | in series with thermal switch |
| Hot-cutter ×2 | DEPLOY_EN | VBAT | 3 s pulse, redundant |

## 6. Layout Rules (General)

1. No electrolytic capacitors; use tantalum/ceramic.
2. Place 100 nF decouplers within 2 mm of each IC supply pin.
3. High-speed traces (MIPI, SPI>10 MHz): controlled impedance, MIPI 100 Ω±10%, length matching ±0.5 mm.
4. Keep 3 mm no-place zone at board edges (standoff area); add ground via arrays around mounting holes.
5. Shield crystal area and expose SWD/UART test points.
6. Conformal coating optional after integration tests, with connector masking.
