/**
 * ButtonDebounce - Modular Button Debouncing Library
 * 
 * Copyright (c) 2025 M&E Design
 * Written by Michael Garcia <michael@mandedesign.studio>
 * Version: 1.0.0
 * 
 * A flexible button debouncing library with interchangeable algorithms.
 * Supports integrator, consecutive, and edge-gated debouncing methods.
 * 
 * Usage:
 *   ButtonDebounce btn;
 *   btn.update(digitalRead(PIN));  // Call every 5ms
 *   if (btn.pressed()) { ... }     // One-shot press event
 * 
 * Build: Compile exactly ONE engine implementation:
 *   - buttonDebounceIntegrator.cpp (recommended)
 *   - buttonDebounceConsecutive.cpp  
 *   - buttonDebounceEdgeGated.cpp
 */

#pragma once
#include "ButtonDebounceVersion.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * ButtonDebounce - modular debouncer with interchangeable engines.
 *
 * Contract:
 *  - Call update() at a fixed tick interval (e.g., 5 ms).
 *  - Pass raw_down: true = pressed/down, false = released/up.
 *  - pressed()/released() are one-shot (true for exactly one tick).
 *  - down()/up() are debounced level.
 *
 * Build:
 *  - Compile exactly ONE engine .cpp:
 *      ButtonDebounce_Integrator.cpp   (recommended)
 *      ButtonDebounce_Consecutive.cpp
 *      ButtonDebounce_EdgeGated.cpp
 *
 * Notes:
 *  - history() returns a meaningful value for history-based engines
 *    (Consecutive, EdgeGated). For Integrator, it returns 0.
 */

class ButtonDebounce {
public:
    struct Config {
        // Integrator (saturating counter + hysteresis)
        uint8_t integ_max = 6;   // accumulator range 0..max
        uint8_t integ_on  = 4;   // threshold to go pressed
        uint8_t integ_off = 2;   // threshold to go released

        // Consecutive (N consecutive identical samples)
        uint8_t consec_n  = 3;   // 3 samples @ 5ms = 15ms

        // Edge-gated (history + chatter suppression + timeout recenter)
        uint8_t edge_threshold   = 4;   // edges in window to call "bouncing"
        uint8_t unstable_timeout = 16;  // ticks before recenter (~80ms @ 5ms)
        uint8_t bounce_confirm   = 1;   // require bouncing for K ticks before gating
    };

    explicit ButtonDebounce(const Config& cfg = Config{});

    // Call each tick
    void update(bool raw_down);

    // Convenience for raw pin reads
    void updateActiveLow(bool pin_level_high)  { update(!pin_level_high); } // pressed when pin reads 0
    void updateActiveHigh(bool pin_level_high) { update(pin_level_high); }  // pressed when pin reads 1

    // One-shot events
    bool pressed()  const { return pressed_; }
    bool released() const { return released_; }

    // Debounced level
    bool down() const { return state_; }
    bool up()   const { return !state_; }

    // History byte (LSB = newest). 0 if engine doesn't use history.
    uint8_t history() const;

    // Reset to known debounced state
    void reset(bool start_down = false);

private:
    Config cfg_;

    bool state_    = false;
    bool pressed_  = false;
    bool released_ = false;

    // Keep engine state compact via a union.
    struct IntegratorState {
        uint8_t acc = 0;
    };

    struct HistoryState {
        uint8_t hist = 0;       // 8-sample shift register
        uint8_t unstable = 0;   // edge-gated timeout counter
        uint8_t bounce_k = 0;   // consecutive bouncing detections
    };

    union EngineState {
        IntegratorState integrator;
        HistoryState    history;
        EngineState() : integrator{} {}
    } eng_;

protected:
    // Shared helpers for history engines (implemented inline here to avoid repetition)
    static inline uint8_t popcount8(uint8_t x)
    {
        x = (uint8_t)((x & 0x55u) + ((x >> 1) & 0x55u));
        x = (uint8_t)((x & 0x33u) + ((x >> 2) & 0x33u));
        return (uint8_t)((x + (x >> 4)) & 0x0Fu);
    }

    static inline uint8_t edgeCount8(uint8_t hist)
    {
        // edges = transitions between adjacent samples
        uint8_t t = (uint8_t)(hist ^ (hist >> 1));
        return popcount8(t);
    }
};
