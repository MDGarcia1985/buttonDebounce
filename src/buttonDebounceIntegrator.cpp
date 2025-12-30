/**
 * ButtonDebounce - Integrator Engine Implementation
 * 
 * Copyright (c) 2025 M&E Design
 * Written by Michael Garcia <michael@mandedesign.studio>
 * Version: 1.0.0
 * 
 * Saturating counter with hysteresis thresholds.
 * Recommended for general-purpose debouncing.
 * 
 * Algorithm:
 * - Increments counter on press, decrements on release
 * - Uses separate thresholds for press/release (hysteresis)
 * - Prevents oscillation around single threshold
 * 
 * Memory usage: 1 byte (accumulator)
 * Debounce time: integ_max * tick_interval
 */

#include "ButtonDebounce.h"

ButtonDebounce::ButtonDebounce(const Config& cfg) : cfg_(cfg)
{
    reset(false);
}

void ButtonDebounce::reset(bool start_down)
{
    state_ = start_down;
    pressed_ = false;
    released_ = false;

    eng_.integrator.acc = state_ ? cfg_.integ_max : 0u;
}

void ButtonDebounce::update(bool raw_down)
{
    pressed_ = false;
    released_ = false;

    // Saturating integrator
    if (raw_down) {
        if (eng_.integrator.acc < cfg_.integ_max) eng_.integrator.acc++;
    } else {
        if (eng_.integrator.acc > 0u) eng_.integrator.acc--;
    }

    // Hysteresis thresholds
    if (!state_ && eng_.integrator.acc >= cfg_.integ_on) {
        state_ = true;
        pressed_ = true;
    } else if (state_ && eng_.integrator.acc <= cfg_.integ_off) {
        state_ = false;
        released_ = true;
    }
}

uint8_t ButtonDebounce::history() const
{
    return 0u; // integrator engine does not support history
}