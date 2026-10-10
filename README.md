# Embedded Monitoring System (STM32F446 + FreeRTOS)

Multi-task firmware for an STM32F446RE (NUCLEO-F446RE) that samples an MPU6050 IMU, stores CRC-protected records in external SPI NOR flash (W25Q64), shows live status on an OLED and exposes a small UART command line. Built as a solo learning project to practice real-time firmware design: drivers written against datasheets, an RTOS task architecture, fault detection and documented validation.

> Status: Stage 1 (single node). Tags `v0.1` to `v0.9` mark each verified block.

## Key features

- MPU6050 (I2C) sampled at **10 Hz** (`ACQ_PERIOD_MS = 100`), passed to storage through a FreeRTOS queue (16 x 24 B).
- W25Q64 (SPI) driver: JEDEC ID, status/busy polling with timeout, sector/chip erase, page program, read.
- Log format: 32-byte records with CRC-32, 8 records per 256 B page, verify-after-write (`memcmp`).
- Fast boot resume: binary search for the first blank page (at most 15 page reads for 32752 pages).
- UART2 RX interrupt + SPSC ring buffer (63 B usable), lost-byte counter.
- UART CLI: `help`, `status`, `dump`.
- Independent watchdog (IWDG, 4 s) fed only when acquisition, storage and rx tasks all report alive; reset reason printed at boot.
- SSD1306 OLED (I2C, shared bus protected by a mutex): run state, sample rate, flash state, dropped samples, accel X.
- Bus transactions captured with a logic analyzer (see `docs/validation.md`).

## Hardware

| Part | Interface | Pins |
|---|---|---|
| NUCLEO-F446RE | USART2 (ST-LINK VCP, 115200) | PA2 TX, PA3 RX |
| W25Q64 SPI NOR flash (8 MB) | SPI1 mode 0 | PA5 SCK, PA6 MISO, PA7 MOSI, PB6 CS (GPIO) |
| MPU6050 (GY-521), addr 0x68 | I2C1, 100 kHz | PB8 SCL, PB9 SDA |
| SSD1306 OLED 128x64 (GM009605), addr 0x3C | I2C1 (shared) | PB8 SCL, PB9 SDA |

- All modules share the Nucleo 3V3 and GND.
- I2C pull-ups come from the modules: 2.2 kOhm on the GY-521 (SMD marking `222`); the OLED module has three 10 kOhm parts (marking `103`), probably its own pull-ups, not traced or measured.

## Software architecture

```
MPU6050 --I2C--> acquisition_task --queue--> storage_task --SPI--> W25Q64
                       |                          |
                       +---- health flags --------+--> heartbeat_task (monitor, feeds IWDG)
UART RX IRQ --> ring buffer --> rx_task --> CLI (help/status/dump)
                                oled_task --> SSD1306 (shares I2C1, mutex)
```

Layout: `firmware/main-node-f446/App/` holds the application modules (`drivers/`, `log/`, `ringbuf/`, `crc/`, `record/`, `findblank/`, `health/`, `cli/`); `Core/` is CubeMX-generated code, with my code inside `USER CODE` regions. Pure-logic modules (CRC, ring buffer, record, CLI parser, find-blank, health) have PC unit tests next to the source.

## Task architecture

| Task | Priority | Stack | Period | Job |
|---|---|---|---|---|
| heartbeat | Low | 1024 B | 1 s | prints counters, checks health flags, feeds IWDG |
| acquisition | BelowNormal | 1024 B | 100 ms | reads MPU6050 (under I2C mutex), puts sample in queue |
| storage | BelowNormal | 1024 B | queue (1 s timeout) | builds records + CRC, erases/programs/verifies pages |
| rx | BelowNormal | 2048 B | 10 ms | drains ring buffer, runs CLI |
| oled | BelowNormal | 2048 B | 500 ms | draws 4 status lines |

FreeRTOS via CMSIS-RTOS2, SysTick replaced by TIM6 as HAL timebase. Details: `docs/freertos.md`.

## Data flow

1. `acquisition_task` wakes every 100 ms (absolute `next_wake`, no drift), reads the MPU6050 under the I2C mutex and puts a 24-byte sample into `sample_queue` (timeout 0; a full queue increments `drops`).
2. `storage_task` takes samples, builds a 32-byte record (seq, timestamp, accel, gyro, temp, CRC-32) and fills a 256 B page buffer (8 records).
3. On a full page it erases the sector when the address is sector-aligned, programs the page, reads it back and compares it with `memcmp`. A mismatch increments `fe` and logs an error.
4. At boot the log resumes: `find_first_blank` binary-searches the first blank page and logging continues there.
5. `dump` (CLI) reads the first page back and prints each record with `crc=OK/BAD`.

Record layout: `docs/record_format.md`. Storage details: `docs/storage_log.md`.

## Reliability

- **Timeouts and error codes**: every driver call returns a status code; flash busy polling has a timeout; the I2C timeout was reduced to 10 ms after it starved lower-priority tasks.
- **CRC-32**: every record carries a CRC-32 over its first 28 bytes (unit-tested with the standard check value `0xCBF43926`); `storage_scan_page` counts bad records.
- **Verify-after-write**: every page is read back and compared.
- **Watchdog**: IWDG 4 s, fed only if acquisition, storage and rx all raised their alive flag since the last check. Verified by deliberately hanging acquisition: log showed `health fail alive=06` three times, then the MCU reset and reported `IWDGRST`. See `docs/watchdog.md`.
- **Counters** printed every second: `drops`, `pg`, `ae` (sensor errors), `fe` (flash errors), `lost` (UART RX bytes lost).
- **Stack overflow hook**, and a log mutex so lines printed by different tasks never mix.
- **I2C bus recovery** at boot (clock out up to 9 pulses until SDA is released) after a debugger reset left a slave holding SDA low.

## Validation

Logic analyzer captures (24 MHz analyzer, PulseView) are in `docs/images/` and explained in `docs/validation.md`:

- I2C: MPU6050 WHO_AM_I (`0x68`) and wake-up write (`PWR_MGMT_1 = 0x00`).
- SPI: W25Q64 JEDEC ID `EF 40 17`, page program, read data, busy polling (measured page program time 419 us, datasheet typical 0.4 ms).
- Sector erase measured 38-44 ms (datasheet typical 45 ms).

### Long-run test

Run on 2026-10-08/09 (log kept locally, not in the repo):

- Started from a power-on reset at a partly used flash (resume at `0x31C00`), 10 Hz sampling, one 32-byte record per sample.
- The 8 MB flash filled after about 7.1 h (tick 25 578 929 ms); firmware logged one `WARN flash full, stop logging` and stopped writing as designed. The OLED showed `FLASH:FULL`.
- Final counters after 27 112 s (7.5 h): `drops=0`, `fe=0`, `lost=0`, `pg=31972` pages written (all of the remaining flash), `ae=1`.
- `ae=1`: one MPU6050 read failed at 22:21:22 and did not repeat; cause not determined.
- Exactly one reset in the log (the start); no `IWDGRST`, no `health fail`.
- After power-cycling the full board, boot took the fast path (`pg=0`, `drops=0`) and the `dump` command read the first 8 records with `crc=OK`. `dump` only reads the first page, so the final pages written during this run were not CRC-checked.

## How to build

Build and flash with **STM32CubeIDE** (open `firmware/main-node-f446`, Build, Run/Debug on the Nucleo ST-LINK). UART output on the ST-LINK virtual COM port, 115200 8N1.

PC unit tests for the pure-logic modules (CLI parser/dispatch, CRC-32, find-blank binary search, health flags, record CRC, ring buffer) need only `gcc`. From the repo root (PowerShell):

```
powershell -ExecutionPolicy Bypass -File tools/run_pc_tests.ps1
```

It builds each test into `local/` and prints `PASS`/`FAIL` per test (8 tests, all passing).

## Demo

Video (49 s, unlisted): https://youtu.be/PI-ddXv6l48

- Board reset: boot log shows the write-address search (`resume addr=800000`), `flash full`, IWDG started, `drops=0`.
- CLI over UART: `help`, `status`, `dump` (first 8 records, all `crc=OK`).
- OLED shows `FLASH:FULL` after the 7-hour logging run.

The flash is full in this demo, so no new records are written during the video.

## Known limitations

- Sequence number restarts at 0 on every boot (no persistent metadata).
- Boot self-test erases sector 0 on every boot, so the log starts at `0x1000`; the self-test is still in the production path.
- Logging stops when the 8 MB flash is full (no wrap-around).
- Resume relies on the log having no gaps; there is no power-loss-safe metadata.
- Watchdog detects stuck tasks, not tasks that run with wrong logic.
- About 3 in 2900 UART log lines arrive truncated or merged on the PC. Both lines go through the log mutex and are ~270 ms apart, so this is byte loss on the link, not task interleaving; root cause (HSI clock tolerance or the ST-LINK virtual COM port) not yet verified. Logged samples in flash are unaffected.
- SPI1 has no mutex: the `dump` CLI command (rx task) and the storage task can access the flash at the same time.
- A flash read error during the boot scan makes the log resume at `0x1000`, which would overwrite the old log.
- OLED: 5x7 text is unreadable on my panel, so all text is 2x; the `FLASH:ERR` state is not tested on hardware yet; I2C bus recovery tested on a small sample only.
- Single sensor node; no CAN, bootloader or host tools yet (Stage 2).
- `docs/` is written in Vietnamese.

## Development notes

I wrote most of the driver and application code myself while learning embedded firmware, working out the logic from datasheets and reference manuals. AI assistance (Claude) was used as a tutor and reviewer: explaining concepts, giving function names, test cases and a few code snippets, reviewing my code and helping with documentation. I did not use AI to write complete functions for me. The 5x7 font table was generated by the AI as data. CubeMX-generated code lives outside the `USER CODE` regions.
