# ButtonDebounce Library v1.0.0

A modular C++ button debouncing library with interchangeable debouncing algorithms.

**Copyright (c) 2025 M&E Design**  
**Written by Michael Garcia** <michael@mandedesign.studio>

## Features

- **Three debouncing algorithms**: Integrator (recommended), Consecutive, and Edge-Gated
- **Modular design**: Compile only the engine you need
- **Configurable parameters**: Adjust timing and sensitivity
- **One-shot events**: Clean pressed/released detection
- **History tracking**: Available for history-based engines

## Quick Start

```cpp
#include "ButtonDebounce.h"

// Create debouncer with default config
ButtonDebounce btn;

// In your main loop (call every 5ms)
void loop() {
    bool raw_pin = digitalRead(BUTTON_PIN);
    btn.updateActiveLow(raw_pin);  // For pull-up buttons
    
    if (btn.pressed()) {
        // Button was just pressed
    }
    if (btn.released()) {
        // Button was just released
    }
    if (btn.down()) {
        // Button is currently held down
    }
}
```

## Debouncing Engines

### Integrator (Recommended)
- **File**: `buttonDebounceIntegrator.cpp`
- **Method**: Saturating counter with hysteresis
- **Best for**: General purpose, reliable debouncing
- **Memory**: Minimal (1 byte)

### Consecutive
- **File**: `buttonDebounceConsecutive.cpp`
- **Method**: Requires N consecutive identical samples
- **Best for**: Simple, predictable behavior
- **Memory**: Low (3 bytes)

### Edge-Gated
- **File**: `buttonDebounceEdgeGated.cpp`
- **Method**: Chatter detection with timeout recovery
- **Best for**: Noisy environments, problematic switches
- **Memory**: Low (3 bytes)

## Configuration

```cpp
ButtonDebounce::Config cfg;
cfg.integ_max = 6;      // Integrator: counter range
cfg.integ_on = 4;       // Integrator: press threshold
cfg.integ_off = 2;      // Integrator: release threshold
cfg.consec_n = 3;       // Consecutive: required samples
cfg.edge_threshold = 4; // Edge-gated: bounce detection
ButtonDebounce btn(cfg);
```

## API Reference

### Core Methods
- `update(bool raw_down)` - Process raw button state
- `updateActiveLow(bool pin_high)` - For pull-up buttons
- `updateActiveHigh(bool pin_high)` - For pull-down buttons

### State Query
- `pressed()` - True for one tick when button pressed
- `released()` - True for one tick when button released
- `down()` - True while button is held down
- `up()` - True while button is released

### Utility
- `history()` - 8-bit history (0 for integrator engine)
- `reset(bool start_down)` - Reset to known state

## Build Instructions

1. Include `ButtonDebounce.h` in your project
2. Compile **exactly one** engine implementation:
   - `buttonDebounceIntegrator.cpp` (recommended)
   - `buttonDebounceConsecutive.cpp`
   - `buttonDebounceEdgeGated.cpp`

## Timing Recommendations

- **Update frequency**: 5ms (200 Hz)
- **Integrator**: 30ms debounce time (6 × 5ms)
- **Consecutive**: 15ms debounce time (3 × 5ms)
- **Edge-gated**: Adaptive based on chatter detection

## License

MIT License - see [LICENSE](LICENSE) file for details.