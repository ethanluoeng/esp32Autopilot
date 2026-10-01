/*
 * ws2812.h - WS2812 LED driver using RMT peripheral (ESP-IDF v5.0+)
 *
 * Provides a static helper to transmit a single GRB pixel to a WS2812 LED
 * via the RMT hardware peripheral. Designed for single-LED use on ESP32-S3.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"

/* RMT resolution: 10 MHz (100 ns per tick), equivalent to clk_div=8 on 80 MHz APB */
#define WS2812_RMT_RESOLUTION_HZ  (10000000U)

/* WS2812 timing in RMT ticks (100 ns each) */
#define WS2812_T0H    4      /* 400 ns  */
#define WS2812_T0L    5      /* 500 ns  */
#define WS2812_T1H    4      /* 400 ns  */
#define WS2812_T1L    5      /* 500 ns  */
#define WS2812_RESET  2800   /* 280 us  */

/* RMT symbol definitions for WS2812 bit patterns */
static const rmt_symbol_word_t s_ws2812_bit0 = {
    .level0 = 1, .duration0 = WS2812_T0H,
    .level1 = 0, .duration1 = WS2812_T0L,
};
static const rmt_symbol_word_t s_ws2812_bit1 = {
    .level0 = 1, .duration0 = WS2812_T1H,
    .level1 = 0, .duration1 = WS2812_T1L,
};
static const rmt_symbol_word_t s_ws2812_reset = {
    .level0 = 0, .duration0 = WS2812_RESET,
    .level1 = 0, .duration1 = 0,
};

/* Static RMT resources (initialized on first use) */
static rmt_channel_handle_t s_ws2812_chan = NULL;
static rmt_encoder_handle_t s_ws2812_enc = NULL;

/**
 * @brief RMT simple encoder callback for WS2812
 *
 * Encodes 24 bits of GRB data followed by a reset symbol.
 * Called by the RMT driver to fill the symbol buffer.
 */
static size_t ws2812_encoder_cb(const void *data, size_t data_size,
                                size_t symbols_written, size_t symbols_free,
                                rmt_symbol_word_t *symbols, bool *done, void *arg)
{
    const uint8_t *pixels = (const uint8_t *)data;
    (void)arg; /* reserved for user data, unused for single-pixel encode */
    size_t data_pos = symbols_written / 8;

    if (symbols_free < 8) {
        return 0;
    }

    if (data_pos < data_size) {
        /* Encode one byte (8 symbols), MSB first */
        size_t count = 0;
        for (int mask = 0x80; mask != 0; mask >>= 1) {
            symbols[count++] = (pixels[data_pos] & mask) ? s_ws2812_bit1 : s_ws2812_bit0;
        }
        return count;
    } else {
        /* All 24 data bits encoded; append reset and mark transaction done */
        symbols[0] = s_ws2812_reset;
        *done = true;
        return 1;
    }
}

/**
 * @brief Initialize the RMT channel and encoder for WS2812 output
 *
 * @param gpio_num GPIO pin connected to WS2812 data input
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t ws2812_init(int gpio_num)
{
    rmt_tx_channel_config_t tx_cfg = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = gpio_num,
        .mem_block_symbols = 64,  /* 64 symbols: sufficient for 25 (24 data + 1 reset) */
        .resolution_hz = WS2812_RMT_RESOLUTION_HZ,
        .trans_queue_depth = 4,
    };
    esp_err_t err = rmt_new_tx_channel(&tx_cfg, &s_ws2812_chan);
    if (err != ESP_OK) {
        return err;
    }

    rmt_simple_encoder_config_t enc_cfg = {
        .callback = ws2812_encoder_cb,
    };
    err = rmt_new_simple_encoder(&enc_cfg, &s_ws2812_enc);
    if (err != ESP_OK) {
        rmt_del_channel(s_ws2812_chan);
        s_ws2812_chan = NULL;
        return err;
    }

    return rmt_enable(s_ws2812_chan);
}

/**
 * @brief Set the RGB color of a single WS2812 LED
 *
 * Transmits a single GRB pixel frame to the LED. On first call,
 * initializes the RMT channel. Subsequent calls reuse the existing channel.
 *
 * @param gpio_num GPIO pin number for WS2812 data
 * @param r Red intensity (0-255)
 * @param g Green intensity (0-255)
 * @param b Blue intensity (0-255)
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t ws2812_set_rgb(int gpio_num, uint8_t r, uint8_t g, uint8_t b)
{
    if (s_ws2812_chan == NULL) {
        esp_err_t err = ws2812_init(gpio_num);
        if (err != ESP_OK) {
            return err;
        }
    }

    /* WS2812 expects GRB byte order */
    uint8_t grb[3] = { g, r, b };
    rmt_transmit_config_t tx_cfg = {
        .loop_count = 0,
    };
    esp_err_t err = rmt_transmit(s_ws2812_chan, s_ws2812_enc, grb, sizeof(grb), &tx_cfg);
    if (err != ESP_OK) {
        return err;
    }

    /* Wait for transmission to complete before returning */
    return rmt_tx_wait_all_done(s_ws2812_chan, 100);
}
