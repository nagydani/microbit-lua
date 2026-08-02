# BLE firmware blob replacement

The micro:bit v2 runs Nordic's **S113 SoftDevice v7.0.1** — a monolithic, proprietary
BLE host + controller precompiled blob. This doc summarizes the options for replacing
it with an open-source alternative, and what each path means for microbit-lua.

**BLE is a hard requirement for this project, not optional.** The question is not
*whether* the device advertises and serves GATT services, but *which* stack provides
them — the proprietary blob today, or an open-source one.

See `SOFTDEVICE_DEBUG_SUMMARY.md` in this directory for the background: the SoftDevice
is what produced the entire "070" (radio-arming deadline) debugging saga.

## What the blob is

- ~105 KB flash (0x1000-0x1B400) + ~8 KB RAM (0x20000000-0x20002040 on the microbit-lua build), plus the MBR (0x0-0xB00) and a gateway/trampoline.
- Owns RADIO, TIMER0, RTC0, PPI channels 17-31, and a fixed set of IRQs (priority 0 after enable).
- Its proprietary radio-timeslot scheduler was the source of the "070" asserts: `svc 255`
  at pc=0x146E4 when the radio-arming deadline (TIMER0 CC[3]) was missed.

## Options

### A. Zephyr (whole runtime) — "replace everything"

- `bbc_microbit_v2` is an official Zephyr board (nRF52833) with complete devicetree
  support: LED matrix (`nordic,nrf-led-matrix`), buzzer, LIS2DH/LIS2MDL, mic SAADC,
  UARTE, USB.
- BLE is fully open source: Zephyr's host (`subsys/bluetooth`) + in-tree controller
  (`subsys/bluetooth/controller`), or Nordic's open-source "SoftDevice Controller"
  (nrfxlib). No monolithic blob anywhere.
- This is Nordic's official direction: nRF Connect SDK (NCS) = Zephyr + open BLE
  components; the S113/S140-style blobs belong to the deprecated nRF5 SDK.
- Lua: several community Zephyr modules exist (WIP quality); no first-class in-tree
  subsystem yet.
- **Cost**: replaces the SoftDevice *and* codal — the fiber scheduler, HAL, and the
  whole `microbit-lua` binding layer (`codal-lua.cpp`, `register_lua_api`) would be
  rewritten on Zephyr drivers. This is a new project, not a blob swap.

### B. NimBLE under codal — "keep the runtime, swap only BLE"

- Apache NimBLE is explicitly "an open-source Bluetooth 5.4 stack (both Host &
  Controller) that completely replaces the proprietary SoftDevice on Nordic
  chipsets." Apache-2.0, memory-efficient, nRF52 controller supported.
- Keeps the Lua REPL, codal, fibers, display, audio untouched. Removes the SD, the
  MBR/gateway, and the priority-0 radio scheduler (our 070 fixes become irrelevant).
- **Cost**: codal's `MicroBitBLEManager` is written against the `sd_ble_*` API (every
  GATT service, pairing, DFU/partial-flashing service). No drop-in exists — the BLE
  manager would be rewritten on NimBLE. Medium-large, but scoped to BLE only.

### C. Rust embassy (out of scope for a C++ codebase)

- `embassy-nrf` runs BLE on the nRF52 with no SoftDevice. Only relevant if the
  project ever leaves C++.

## Considerations for microbit-lua

- **BLE is required; the only open question is which stack.** Today the SD provides it
  through codal's `MicroBitBLEManager` (advertising plus GATT services such as
  UART/DFU). Options A and B both deliver BLE without the blob; the difference is
  scope — Zephyr replaces the whole runtime, NimBLE-under-codal replaces only the BLE
  layer.
- With BLE mandatory, the real decision is A vs B:
  - **A (Zephyr)** is the principled end-state: fully open, Nordic's official
    direction, and removes codal + SD together. It is a rewrite of the project.
  - **B (NimBLE under codal)** is the pragmatic middle ground: keeps the Lua REPL,
    codal, fibers, display, and audio intact; only `MicroBitBLEManager` (its GATT
    services, pairing, and DFU service) is rewritten against NimBLE.
- Both A and B remove the entire "070" failure class (no SD radio scheduler /
  gateway) and free ~105 KB flash + ~8-13 KB RAM.
- Licensing: Apache-2.0 end-to-end versus the SD's proprietary binary — likely the
  real motivation.
- Caveats: Zephyr's micro:bit v2 target is listed as "not actively maintained," and
  the Zephyr BLE host + controller has a nontrivial footprint (fine on 512 KB /
  128 KB). NimBLE is memory-efficient but its controller on nRF52 must be evaluated
  for feature parity with what `MicroBitBLEManager` currently uses.

## Open questions

- Which BLE features must survive the swap (services, pairing, DFU, throughput)?
  This determines how much of `MicroBitBLEManager` needs a NimBLE port.
- Next step: a feasibility write-up or a spike (e.g., "hello world" BLE on Zephyr on
  the v2, or NimBLE under codal)?
