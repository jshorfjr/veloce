# Veloce NES Plugin

NES (Nintendo Entertainment System) emulator plugin for the Veloce emulation platform.

## Building

The NES plugin is built as part of the main Veloce build, but can also be built independently:

```bash
# From the emulatorplatform root directory
mkdir -p build && cd build
cmake ..
cmake --build . --target nes_plugin

# Or build just the plugin from the plugins/nes directory
cd plugins/nes
mkdir -p build && cd build
cmake ..
cmake --build .
```

The plugin will be output as `nes.so` (Linux), `nes.dylib` (macOS), or `nes.dll` (Windows).

## Debug Mode

Debug logging can be enabled by setting the `DEBUG` environment variable or using the `--debug` flag:

```bash
# Using environment variable
DEBUG=1 ./veloce rom.nes

# Using command line flag
./veloce --debug rom.nes
```

When debug mode is enabled, the following information is logged to stderr:

### CPU State Debugging
- Periodic CPU state dumps (PC, A, X, Y, SP, P registers)
- Detection of CPU stuck in tight loops

### Mapper Debugging
- Bank switching operations
- IRQ counter state changes
- CHR/PRG bank configuration

## Supported Mappers

| Mapper | Name | Description |
|--------|------|-------------|
| 000 | NROM | No mapper, basic games (Super Mario Bros, Donkey Kong) |
| 001 | MMC1 | Nintendo MMC1 (Legend of Zelda, Metroid) |
| 002 | UxROM | PRG bank switching (Mega Man, Castlevania) |
| 003 | CNROM | CHR bank switching (Solomon's Key) |
| 004 | MMC3 | Nintendo MMC3 with scanline counter (Super Mario Bros 3, Kirby's Adventure) |
| 007 | AxROM | Single-screen mirroring (Battletoads) |
| 009 | MMC2 | Nintendo MMC2 with CHR latch (Punch-Out!!) |
| 010 | MMC4 | Nintendo MMC4 with CHR latch (Fire Emblem) |
| 011 | Color Dreams | Unlicensed mapper (Crystal Mines) |
| 034 | BNROM/NINA-001 | Discrete logic (Deadly Towers) |
| 066 | GxROM | PRG/CHR bank switching (Super Mario Bros + Duck Hunt) |
| 071 | Camerica | Codemasters games (Fire Hawk) |
| 079 | NINA-003/006 | American Video Entertainment (Krazy Kreatures) |
| 206 | DxROM/Namco 108 | Early Namco games (Babel no Tou) |

## Testing

The NES plugin includes a comprehensive test suite that validates emulator accuracy using the [nes-test-roms](https://github.com/christopherpow/nes-test-roms) collection.

### Quick Start

```bash
# Run all tests (clones test ROMs automatically, cleans up after)
cd plugins/nes/tests
./run_tests.sh

# Or use the Python runner
python3 test_runner.py
```

### Test Runner Options

#### Bash Script (`run_tests.sh`)

```bash
./run_tests.sh              # Run all tests
./run_tests.sh cpu          # Run CPU tests only
./run_tests.sh ppu          # Run PPU tests only
./run_tests.sh mapper       # Run mapper tests only
./run_tests.sh apu          # Run APU tests only
./run_tests.sh --keep       # Keep test ROMs after completion
./run_tests.sh --verbose    # Show detailed output
```

#### Python Runner (`test_runner.py`)

```bash
python3 test_runner.py              # Run all tests
python3 test_runner.py cpu ppu      # Run specific categories
python3 test_runner.py --keep       # Keep test ROMs
python3 test_runner.py --verbose    # Detailed output
python3 test_runner.py --json       # JSON output (for CI)
```

### Test Categories

| Category | Description |
|----------|-------------|
| `cpu` | CPU instruction correctness and timing |
| `ppu` | PPU rendering, VBlank, NMI, sprites |
| `mapper` | Mapper-specific tests (MMC3, etc.) |
| `apu` | Audio processing unit |
| `timing` | General timing accuracy |

### Test Results Summary

| Test Suite | Status | Notes |
|------------|--------|-------|
| CPU Instructions (v5) | **PASS** | All 16 instruction tests |
| CPU Timing | **PASS** | Cycle-accurate timing |
| CPU Interrupts | **PASS** | NMI/IRQ handling |
| PPU VBlank/NMI | **9/10** | One edge-case timing issue |
| PPU Sprites | **PASS** | Sprite 0 hit, overflow |
| MMC3 Core | **PASS** | Clocking, A12, basic behavior |
| MMC3 Details | Partial | Edge-case timing |

### Known Issues

These tests fail but represent edge cases that rarely affect games:

| Test | Issue | Impact |
|------|-------|--------|
| `03-vbl_clear_time` | VBL flag clear off by 1-2 cycles | Very few games affected |
| `ppu_open_bus` | Open bus decay not implemented | Minimal game impact |
| `2-details` (MMC3) | Counter clock count precision | Edge case |
| `4-scanline_timing` (MMC3) | Scanline 0 timing edge case | Rare |

### Continuous Integration

For CI systems, use the JSON output:

```bash
python3 test_runner.py --json > results.json
```

The exit code is 0 if all tests pass (excluding known failures), 1 otherwise.

### Adding New Tests

Edit `tests/test_config.json` to add new test cases:

```json
{
  "test_suites": {
    "my_suite": {
      "name": "My Test Suite",
      "description": "Description of tests",
      "priority": "high",
      "tests": [
        {"path": "path/to/test.nes", "expected": "pass"},
        {"path": "path/to/known_fail.nes", "expected": "known_fail", "notes": "Why it fails"}
      ]
    }
  }
}
```

## Architecture

The NES plugin implements:
- **6502 CPU** - Full instruction set with cycle-accurate timing
- **PPU** - Picture Processing Unit with background/sprite rendering
- **APU** - Audio Processing Unit with pulse, triangle, noise, and DMC channels
- **Cartridge** - iNES/NES 2.0 ROM loading with mapper support

### Component Overview

```
plugins/nes/src/
├── nes_plugin.cpp      # Plugin entry point, main loop
├── cpu.cpp/hpp         # 6502 CPU emulation
├── ppu.cpp/hpp         # PPU (graphics) emulation
├── apu.cpp/hpp         # APU (audio) emulation
├── bus.cpp/hpp         # Memory bus and I/O
├── cartridge.cpp/hpp   # ROM loading and parsing
└── mappers/
    ├── mapper.hpp      # Base mapper interface
    ├── mapper_000.cpp  # NROM
    ├── mapper_001.cpp  # MMC1
    ├── mapper_002.cpp  # UxROM
    ├── mapper_003.cpp  # CNROM
    ├── mapper_004.cpp  # MMC3
    └── ...             # Additional mappers
```

### Test ROM Detection

The emulator automatically detects blargg-style test ROMs by checking for the signature `0xDE 0xB0 0x61` at addresses $6001-$6003. When detected:
- Result byte at $6000 is monitored (0x00 = running, 0x80 = reset, 0x81 = passed)
- Debug output shows test status and result codes
- Emulator exits automatically when test completes

## Performance Notes

The emulator is optimized for speed while maintaining accuracy:

- IRQ checks performed once per CPU instruction (not per PPU cycle)
- MMC3 A12 monitoring uses early-exit optimization
- Debug logging disabled in release builds
- Hot paths avoid unnecessary allocations

For performance-critical testing, ensure `DEBUG` environment variable is not set.
