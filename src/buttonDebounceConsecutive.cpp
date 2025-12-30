/**
 * ButtonDebounce - Consecutive Engine Implementation
 * 
 * Copyright (c) 2025 M&E Design
 * Written by Michael Garcia <michael@mandedesign.studio>
 * Version: 1.0.0
 * 
 * Requires N consecutive identical samples before state change.
 * Simple and predictable debouncing behavior.
 * 
 * Algorithm:
 * - Maintains 8-bit shift register of recent samples
 * - Changes state only when N consecutive bits match target
 * - Configurable N value (typically 2-4 samples)
 * 
 * Memory usage: 3 bytes (history + counters)
 * Debounce time: consec_n * tick_interval
 */

#include "ButtonDebounce.h"

/**
 * Update history shift register with new sample.
 * @param h Pointer to 8-bit history register
 * @param raw_down Current raw button state
 */

static void update_hist(uint8_t* h, bool raw_down)
{
    *h = (uint8_t)((*h << 1) | (raw_down ? 1u : 0u));
}

ButtonDebounce::ButtonDebounce(const Config& cfg) : cfg_(cfg)
{
    reset(false);
}

void ButtonDebounce::reset(bool start_down)
{
    state_ = start_down;
    pressed_ = false;
    released_ = false;

    eng_.history.hist = state_ ? 0xFFu : 0x00u;
    eng_.history.unstable = 0u;
    eng_.history.bounce_k = 0u;
}

void ButtonDebounce::update(bool raw_down)
{
    pressed_ = false;
    released_ = false;

    update_hist(&eng_.history.hist, raw_down);

    // Require N consecutive stable samples at the LSB end
    const uint8_t n = cfg_.consec_n;
    const uint8_t mask = (n >= 8u) ? 0xFFu : (uint8_t)((1u << n) - 1u);

    const bool all_pressed  = ((eng_.history.hist & mask) == mask);
    const bool all_released = ((eng_.history.hist & mask) == 0u);

    if (!state_ && all_pressed) {
        state_ = true;
        pressed_ = true;
    } else if (state_ && all_released) {
        state_ = false;
        released_ = true;
    }

    // Optional alternate (your "00xxx111 / 11xxx000" idea):
    // const uint8_t M = 0b11000111u;
    // if (!state_ && ((eng_.history.hist & M) == 0b00000111u)) { state_=true; pressed_=true; }
    // if ( state_ && ((eng_.history.hist & M) == 0b11000000u)) { state_=false; released_=true; }
}

uint8_t ButtonDebounce::history() const
{
    return eng_.history.hist;
}