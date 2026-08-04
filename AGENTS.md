# Agent notes for opencode

## Build
- Use `python3 build.py --clean` as the entry point (not cmake directly)
- Output: `build/MICROBIT.hex` (also `build/MICROBIT.bin`); both are copied to the project root
- Dependencies are pinned via `"branches"` in `codal.json` — update those SHAs to bump
