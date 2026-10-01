/*
 * rmt_tx.h - Mock RMT transmit channel API for host-based unit testing.
 *
 * Mirrors the subset of the ESP-IDF v5.0+ RMT driver API used by ws2812.h.
 * Provides controllable stubs so tests can assert on transmitted frames.
 */
#ifndef MOCK_RMT_TX_H
#define MOCK_RMT_TX_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "esp_err.h"

/* --- Types used by ws2812.h --- */

typedef enum {
    RMT_CLK_SRC_DEFAULT = 0,
} rmt_clk_src_t;

/* RMT symbol word: two level/duration pairs (used for WS2812 bit encoding). */
typedef struct {
    uint32_t level0 : 1;
    uint32_t duration0 : 28;
    uint32_t level1 : 1;
    uint32_t duration1 : 28;
} rmt_symbol_word_t;

typedef struct rmt_tx_channel *rmt_channel_handle_t;

typedef struct {
    rmt_clk_src_t clk_src;
    int32_t gpio_num;
    uint32_t mem_block_symbols;
    uint32_t resolution_hz;
    uint32_t trans_queue_depth;
} rmt_tx_channel_config_t;

typedef struct {
    uint32_t loop_count;
} rmt_transmit_config_t;

/* RMT encoder handle and simple-encoder callback (used by ws2812.h) */
typedef struct rmt_encoder *rmt_encoder_handle_t;

typedef size_t (*rmt_encode_fn_t)(const void *data, size_t data_size,
                                  size_t symbols_written, size_t symbols_free,
                                  rmt_symbol_word_t *symbols, bool *done,
                                  void *arg);

typedef struct {
    rmt_encode_fn_t callback;
    void *arg;
} rmt_simple_encoder_config_t;

/* --- Test-control knobs (set by tests before calling rmt_new_tx_channel) --- */
extern esp_err_t mock_rmt_new_tx_channel_result;  /* return value to inject */
extern esp_err_t mock_rmt_enable_result;          /* return value to inject */
extern esp_err_t mock_rmt_new_simple_encoder_result; /* return value to inject */
extern esp_err_t mock_rmt_transmit_result;        /* return value to inject */
extern esp_err_t mock_rmt_tx_wait_all_done_result;/* return value to inject */

/* Capture of the last transmit call for assertions. */
extern int mock_last_gpio_num;
extern const uint8_t *mock_last_transmit_data;
extern size_t mock_last_transmit_size;

/* --- API stubs matching ESP-IDF signatures --- */
esp_err_t rmt_new_tx_channel(const rmt_tx_channel_config_t *tx_chan_cfg,
                             rmt_channel_handle_t *ret_tx_chan);
esp_err_t rmt_enable(rmt_channel_handle_t chan);
esp_err_t rmt_transmit(rmt_channel_handle_t chan, void *encoder,
                       const void *data, size_t data_size,
                       const rmt_transmit_config_t *tx_cfg);
esp_err_t rmt_tx_wait_all_done(rmt_channel_handle_t chan, uint32_t timeout_ms);
esp_err_t rmt_del_channel(rmt_channel_handle_t chan);
esp_err_t rmt_new_simple_encoder(const rmt_simple_encoder_config_t *config,
                                 rmt_encoder_handle_t *ret_encoder);

#endif /* MOCK_RMT_TX_H */
