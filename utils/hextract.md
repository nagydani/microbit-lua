# hextract

Intel HEX file inspector and payload manipulation tool for micro:bit v2 firmware.
Lua 5.1.5 compatible.

## Intel HEX Format

The Intel HEX format is a text-based representation of binary data for
programming flash memory. Each line is a record with the structure:

```
:LLAAAATT[DD...]CC
```

| Field | Length | Description |
|-------|--------|-------------|
| `:`   | 1 char | Start code |
| `LL`  | 2 hex  | Byte count (number of data bytes) |
| `AAAA`| 4 hex  | 16-bit address within the current segment |
| `TT`  | 2 hex  | Record type |
| `DD`  | 2*LL   | Data bytes |
| `CC`  | 2 hex  | Checksum (two's complement of sum of all preceding bytes) |

### Record Types

| Type | Name | Description |
|------|------|-------------|
| `00` | Data | Data bytes at the current address |
| `01` | EOF | End of file (must be the last record) |
| `02` | Extended Segment Address | Sets the 16-bit segment base; physical address = `(segment << 4) + AAAA` |
| `03` | Start Segment Address | Entry point (CS:IP) for 80x86 processors |
| `04` | Extended Linear Address | Sets the upper 16 bits; physical address = `(upper << 16) + AAAA` |
| `05` | Start Linear Address | Entry point (32-bit EIP) |

### Addressing

The 16-bit `AAAA` field is relative to a base set by type `02` or `04` records.
Type `02` uses segment addressing (base = segment << 4), giving a 20-bit
address space. Type `04` uses linear addressing (base = upper << 16), giving a
32-bit address space. Once a type `04` record appears, it takes precedence over
type `02`.

### Checksum

The checksum byte is the two's complement of the sum of all bytes in the record
(excluding the leading `:` and the checksum itself):

```
CC = (-(LL + AH + AL + TT + DD[0] + ... + DD[n])) & 0xFF
```

## Firmware Memory Layout (nRF52833)

Based on `libraries/codal-microbit-v2/ld/nrf52833-softdevice.ld`:

| Region | Origin | Length | Purpose |
|--------|--------|--------|---------|
| MBR | 0x0000 | 0x1000 | Master Boot Record |
| SD | 0x1000 | 0x1B000 | SoftDevice (BLE stack) |
| FLASH | 0x1C000 | 0x5AFF0 | Application code + Lua script |
| LUA_META | 0x76FF0 | 0x10 | Lua metadata block |
| BOOTLOADER | 0x77000 | 0x7000 | Bootloader |
| SETTINGS | 0x7E000 | 0x2000 | Bootloader settings |
| UICR | 0x10001014 | 0x8 | User Info Config Registers |

### Lua Metadata Block

Located at **0x76FF0**, 16 bytes, 4 little-endian 32-bit words:

| Offset | Field | Description |
|--------|-------|-------------|
| 0x00 | Magic | `0x4C554131` ("LUA1") |
| 0x04 | Start | Physical address of the Lua script in FLASH |
| 0x08 | End | Physical address of the end of the Lua script |
| 0x0C | Size | Size of the Lua script in bytes |

**Invariant**: `Size == End - Start`. The current firmware build has a bug where
the Size field incorrectly stores the End address value. The script reports this
as a warning.

## Script Architecture

```
hextract
├── Bitwise helpers          Lua 5.1 compatible bit_and, bit_or, bit_lshift, bit_rshift
├── parse_hex_line()         Parse a single Intel HEX line, verify checksum
├── parse_hex_file()         Parse entire file, track segment/linear base, collect errors
├── build_data_regions()     Group contiguous addresses into regions, find gaps
├── read_bytes_from_records() Read raw bytes from parsed records at a given address
├── u32_le()                 Read little-endian uint32 from byte array
├── cmd_structure()          Structure inspection command
└── Main                     Argument parsing, command dispatch
```

## Usage

```bash
lua5.1 tools/hextract structure <input.hex> [--verbose]
```

### Commands

**`structure`** - Inspect the structure of an Intel HEX file.

Default output:
- Data regions with physical address boundaries and sizes
- LUA metadata block analysis with sanity checks
- Summary statistics (record counts, data bytes, address range, gaps)
- Checksum validation summary

With `--verbose`:
- Full record-by-record log showing type, address, physical address, size,
  and checksum status for every line

### Example

```
$ lua5.1 tools/hextract structure MICROBIT.hex

=== DATA REGIONS ===
  Region 1: 0x00000000 - 0x00000AFF  (2.8 KB)
  Region 2: 0x00001000 - 0x0001B3FF  (105.0 KB)
  Region 3: 0x0001C000 - 0x00052EA5  (219.7 KB)
  Region 4: 0x00076FF0 - 0x0007D3EB  (25.0 KB)
  Region 5: 0x0007E000 - 0x0007F322  (4.8 KB)
  Region 6: 0x10001014 - 0x1000101B  (8 B)

=== LUA METADATA (0x00076FF0) ===
  Magic: 0x4C554131 ("LUA1")
  Start: 0x00052BC8 (338888)
  End:   0x00052C0A (338954)
  Size:  0x00052C0A (338954)

  Sanity checks:
    Magic: OK
    Start < End: OK (difference = 66 bytes)
    WARNING: Size (338954) != End - Start (66)

  Script preview:
    scroll('Lua alive')
    while true do
      sleep(5000)
      scroll('x')
    end

=== SUMMARY ===
  Total records: 22876
    Data: 22866
    Extended Segment Address: 7
    Start Segment Address: 1
    Extended Linear Address: 1
    EOF: 1
  Total data bytes: 365773 (357.2 KB)
  Address range: 0x00000000 - 0x1000101B
  Gaps: 5 (total 255.7 MB)
  Checksums: 22876/22876 valid
```
