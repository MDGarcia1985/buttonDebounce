/**
 * ButtonDebounce - Edge-Gated Engine Implementation
 * 
 * Copyright (c) 2025 M&E Design
 * Written by Michael Garcia <michael@mandedesign.studio>
 * Version: 1.0.0
 * 
 * Advanced chatter detection with timeout recovery.
 * Best for noisy environments and problematic switches.
 * 
 * Algorithm:
 * - Detects bouncing via edge counting in sliding window
 * - Gates state changes during detected bounce periods
 * - Timeout mechanism prevents permanent lockup
 * - Falls back to consecutive logic when stable
 * 
 * Memory usage: 3 bytes (history + bounce counters)
 * Debounce time: Adaptive based on chatter detection
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

    // Detect chatter via edge count across the 8-sample window
    const uint8_t edges = edgeCount8(eng_.history.hist);
    const bool bouncing_now = (edges >= cfg_.edge_threshold);

    // Optional: require bouncing for K consecutive ticks before gating
    if (bouncing_now) {
        if (eng_.history.bounce_k < 255u) eng_.history.bounce_k++;
    } else {
        eng_.history.bounce_k = 0u;
    }

    const bool bouncing = (eng_.history.bounce_k >= cfg_.bounce_confirm);

    // Track how long we've been bouncing
    if (bouncing) {
        if (eng_.history.unstable < 255u) eng_.history.unstable++;
    } else {
        eng_.history.unstable = 0u;
    }

    // Timeout -> recenter to current debounced state (prevents lock-up)
    if (eng_.history.unstable >= cfg_.unstable_timeout) {
        eng_.history.hist = state_ ? 0xFFu : 0x00u;
        eng_.history.unstable = 0u;
        eng_.history.bounce_k = 0u;
        return;
    }

    // Only accept changes when not bouncing (reuse consecutive acceptance rule)
    if (!bouncing) {
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
    }
}

uint8_t ButtonDebounce::history() const
{
    return eng_.history.hist;
}
