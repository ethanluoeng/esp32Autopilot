/*
 * rmt_encoder.h - Mock RMT encoder API for host-based unit testing.
 *
 * All encoder-related declarations live in the rmt_tx.h mock so this header
 * is a thin re-include. Kept separate to mirror the real ESP-IDF layout where
 * ws2812.h includes both driver/rmt_tx.h and driver/rmt_encoder.h.
 */
#ifndef MOCK_RMT_ENCODER_H
#define MOCK_RMT_ENCODER_H

#include "driver/rmt_tx.h"

#endif /* MOCK_RMT_ENCODER_H */
