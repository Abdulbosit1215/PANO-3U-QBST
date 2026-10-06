# OBC Firmware Build Guide

## Toolchain
- STM32CubeMX 6.x (HAL skeleton generation) + `arm-none-eabi-gcc`
- Or PlatformIO: `platform = ststm32`, `board = genericSTM32F405RG`

## Directory Layout

```
Core/Src/
  main.c          Main program + tasks
  obc_fsm.c       Mode state machine
  adcs.c          B-dot detumbling
  comm.c          AX.25 + downlink queue
  payload_mgr.c   CM4 protocol + imaging schedule
  link_proto.c    Inter-board frame encode/decode
  drivers.c       QMC5883L/MPU9250/INA226/DS18B20/thermal/antenna
  misc_drivers.c  DRV8837/watchdog/W25Q128 logging
Core/Inc/         Matching headers (reference IDs/register addresses align with electrical spec)
```

## Build

```bash
# CubeMX flow: generate project from PANO-3U.ioc, then replace Core/Src
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
  -O2 -DUSE_HAL_DRIVER -DSTM32F405xx ...

# PlatformIO flow:
pio run -e obc
```

## Key Configuration

- FreeRTOS: 4 tasks (`comm > adcs > hk > payload`), stack config in `main.c`
- Clock: HSE 8 MHz → PLL → 168 MHz, SysTick 1 kHz
- UART1 = 9600 (comm board), UART2 = 115200 (CM4), both DMA + idle interrupt
- Flash map: first 64 KB reserved for bootloader/IAP, app starts at `0x08010000`
