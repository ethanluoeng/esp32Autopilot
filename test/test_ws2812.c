/*
 * test_ws2812.c - Host-based unit tests for ws2812.h
 *
 * Verifies:
 *   1. ws2812_set_rgb transmits GRB byte order (G, R, B)
 *   2. RMT channel is initialized exactly once (idempotent init)
 *   3. Error propagation: rmt_new_tx_channel failure
 *   4. Error propagation: rmt_enable failure
 *   5. Error propagation: rmt_new_simple_encoder failure
 *   6. Error propagation: rmt_transmit failure
 *   7. Error propagation: rmt_tx_wait_all_done failure
 *   8. Timing constants are correct
 *   9. Encoder callback produces correct symbols for known data
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "ws2812.h"
#include "driver/rmt_tx.h"

/* Forward-declare the internal counter from the stub. */
extern int mock_new_tx_channel_calls;
extern int mock_new_simple_encoder_calls;
extern int mock_transmit_calls;

/* --- Test helpers --- */

static int s_checks_total = 0;
static int s_checks_pass = 0;

#define TEST(name) do { \
    printf("\n  [TEST] %s\n", name); \
} while (0)

#define PASS(msg) do { \
    s_checks_total++; \
    s_checks_pass++; \
    printf("  [PASS] %s\n", msg); \
} while (0)

#define FAIL(msg) do { \
    s_checks_total++; \
    printf("  [FAIL] %s (line %d)\n", msg, __LINE__); \
} while (0)

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        FAIL(msg); \
    } else { \
        PASS(msg); \
    } \
} while (0)

/* Reset all mock state between tests. */
static void mock_reset(void)
{
    /* Reset static RMT state defined in ws2812.h (same translation unit). */
    s_ws2812_chan = NULL;
    s_ws2812_enc  = NULL;

    mock_rmt_new_tx_channel_result   = ESP_OK;
    mock_rmt_enable_result           = ESP_OK;
    mock_rmt_new_simple_encoder_result = ESP_OK;
    mock_rmt_transmit_result         = ESP_OK;
    mock_rmt_tx_wait_all_done_result = ESP_OK;
    mock_new_tx_channel_calls        = 0;
    mock_new_simple_encoder_calls    = 0;
    mock_transmit_calls              = 0;
    mock_last_transmit_data          = NULL;
    mock_last_transmit_size          = 0;
    mock_last_gpio_num               = -1;
}

/* --- Test cases --- */

/* 1. Verify GRB byte order: ws2812_set_rgb(gpio, r, g, b) sends {g, r, b}. */
static void test_grb_byte_order(void)
{
    TEST("GRB byte order");
    mock_reset();

    esp_err_t err = ws2812_set_rgb(48, 0x11, 0x22, 0x33);
    CHECK(err == ESP_OK, "ws2812_set_rgb returns ESP_OK");
    CHECK(mock_last_transmit_size == 3, "transmitted size is 3 bytes");
    CHECK(mock_last_transmit_data[0] == 0x22, "byte 0 is G (0x22)");
    CHECK(mock_last_transmit_data[1] == 0x11, "byte 1 is R (0x11)");
    CHECK(mock_last_transmit_data[2] == 0x33, "byte 2 is B (0x33)");
}

/* 2. Verify RMT channel init is idempotent: called only once across multiple
 *    invocations of ws2812_set_rgb. */
static void test_init_idempotent(void)
{
    TEST("RMT channel init is idempotent");
    mock_reset();

    ws2812_set_rgb(48, 0, 255, 0);
    int init_calls_1 = mock_new_tx_channel_calls;
    CHECK(init_calls_1 == 1, "first call: rmt_new_tx_channel called once");

    ws2812_set_rgb(48, 255, 0, 0);
    int init_calls_2 = mock_new_tx_channel_calls;
    CHECK(init_calls_2 == 1, "second call: rmt_new_tx_channel still called once total");

    ws2812_set_rgb(48, 0, 0, 255);
    CHECK(mock_new_tx_channel_calls == 1, "third call: still only 1 init total");
}

/* 3. Verify rmt_new_tx_channel failure propagates. */
static void test_rmt_new_tx_channel_failure(void)
{
    TEST("rmt_new_tx_channel failure propagation");
    mock_reset();
    mock_rmt_new_tx_channel_result = ESP_FAIL;

    esp_err_t err = ws2812_set_rgb(48, 0, 255, 0);
    CHECK(err == ESP_FAIL, "ws2812_set_rgb returns ESP_FAIL when rmt_new_tx_channel fails");
}

/* 4. Verify rmt_enable failure propagates. */
static void test_rmt_enable_failure(void)
{
    TEST("rmt_enable failure propagation");
    mock_reset();
    mock_rmt_enable_result = ESP_FAIL;

    esp_err_t err = ws2812_set_rgb(48, 0, 255, 0);
    CHECK(err == ESP_FAIL, "ws2812_set_rgb returns ESP_FAIL when rmt_enable fails");
}

/* 5. Verify rmt_new_simple_encoder failure propagates. */
static void test_rmt_new_simple_encoder_failure(void)
{
    TEST("rmt_new_simple_encoder failure propagation");
    mock_reset();
    mock_rmt_new_simple_encoder_result = ESP_FAIL;

    esp_err_t err = ws2812_set_rgb(48, 0, 255, 0);
    CHECK(err == ESP_FAIL, "ws2812_set_rgb returns ESP_FAIL when rmt_new_simple_encoder fails");
}

/* 6. Verify rmt_transmit failure propagates. */
static void test_rmt_transmit_failure(void)
{
    TEST("rmt_transmit failure propagation");
    mock_reset();
    mock_rmt_transmit_result = ESP_FAIL;

    esp_err_t err = ws2812_set_rgb(48, 0, 255, 0);
    CHECK(err == ESP_FAIL, "ws2812_set_rgb returns ESP_FAIL when rmt_transmit fails");
}

/* 7. Verify rmt_tx_wait_all_done failure propagates. */
static void test_rmt_tx_wait_all_done_failure(void)
{
    TEST("rmt_tx_wait_all_done failure propagation");
    mock_reset();
    mock_rmt_tx_wait_all_done_result = ESP_FAIL;

    esp_err_t err = ws2812_set_rgb(48, 0, 255, 0);
    CHECK(err == ESP_FAIL, "ws2812_set_rgb returns ESP_FAIL when rmt_tx_wait_all_done fails");
}

/* 8. Verify WS2812 timing constants. */
static void test_timing_constants(void)
{
    TEST("Timing constants");
    CHECK(WS2812_T0H == 4, "WS2812_T0H == 4 (400 ns)");
    CHECK(WS2812_T0L == 5, "WS2812_T0L == 5 (500 ns)");
    CHECK(WS2812_T1H == 4, "WS2812_T1H == 4 (400 ns)");
    CHECK(WS2812_T1L == 5, "WS2812_T1L == 5 (500 ns)");
    CHECK(WS2812_RESET == 2800, "WS2812_RESET == 2800 (280 us)");
    CHECK(WS2812_RMT_RESOLUTION_HZ == 10000000U, "RMT resolution is 10 MHz");
}

/* 9. Verify encoder callback produces correct symbol patterns.
 *
 * The encoder callback (ws2812_encoder_cb) is a static function in ws2812.h.
 * We can call it directly since it's defined in the header.
 * It encodes one byte per invocation, MSB first, producing 8 symbols per call.
 * After 3 calls (24 bits), the 4th call produces the reset symbol.
 */
static void test_encoder_callback(void)
{
    TEST("Encoder callback produces correct symbols");

    /* Set up test data: single byte 0b10100000 (0xA0) */
    uint8_t data = 0xA0;
    rmt_symbol_word_t symbols[8];
    bool done = false;

    /* Call encoder for byte 0 */
    size_t written = ws2812_encoder_cb(&data, 1, 0, 8, symbols, &done, NULL);
    CHECK(written == 8, "first call returns 8 symbols");
    CHECK(done == false, "not done after first byte");

    /* Verify bit 7 (MSB) is a "1" bit: level0=1, dur0=T1H, level1=0, dur1=T1L */
    CHECK(symbols[0].level0 == 1, "bit 7 (MSB) high level is 1");
    CHECK(symbols[0].duration0 == WS2812_T1H, "bit 7 high duration = T1H");
    CHECK(symbols[0].level1 == 0, "bit 7 low level is 0");
    CHECK(symbols[0].duration1 == WS2812_T1L, "bit 7 low duration = T1L");

    /* Verify bit 6 is a "0" bit: level0=1, dur0=T0H, level1=0, dur1=T0L */
    CHECK(symbols[1].level0 == 1, "bit 6 high level is 1");
    CHECK(symbols[1].duration0 == WS2812_T0H, "bit 6 high duration = T0H");
    CHECK(symbols[1].level1 == 0, "bit 6 low level is 0");
    CHECK(symbols[1].duration1 == WS2812_T0L, "bit 6 low duration = T0L");

    /* Verify bit 5 is a "1" bit */
    CHECK(symbols[2].duration0 == WS2812_T1H, "bit 5 high duration = T1H");

    /* Verify bit 4 is a "0" bit */
    CHECK(symbols[3].duration0 == WS2812_T0H, "bit 4 high duration = T0H");

    /* Now call for the reset (data_pos >= data_size) */
    written = ws2812_encoder_cb(&data, 1, 8, 8, symbols, &done, NULL);
    CHECK(written == 1, "reset call returns 1 symbol");
    CHECK(done == true, "done flag set on reset");
    CHECK(symbols[0].level0 == 0, "reset symbol level0 is 0");
    CHECK(symbols[0].duration0 == WS2812_RESET, "reset duration = WS2812_RESET");
}

/* 10. Verify that all-zero RGB (0,0,0) transmits all zero bytes. */
static void test_all_zero_rgb(void)
{
    TEST("All-zero RGB transmits {0, 0, 0}");
    mock_reset();

    esp_err_t err = ws2812_set_rgb(48, 0, 0, 0);
    CHECK(err == ESP_OK, "ws2812_set_rgb(0,0,0) returns ESP_OK");
    CHECK(mock_last_transmit_size == 3, "size is 3");
    CHECK(mock_last_transmit_data[0] == 0, "G byte is 0");
    CHECK(mock_last_transmit_data[1] == 0, "R byte is 0");
    CHECK(mock_last_transmit_data[2] == 0, "B byte is 0");
}

/* 11. Verify GPIO is passed correctly to RMT channel config. */
static void test_gpio_passing(void)
{
    TEST("GPIO number is passed to RMT channel");
    mock_reset();

    ws2812_set_rgb(3, 0, 255, 0);
    CHECK(mock_last_gpio_num == 3, "GPIO 3 is captured by mock");

    mock_reset();
    ws2812_set_rgb(48, 0, 255, 0);
    CHECK(mock_last_gpio_num == 48, "GPIO 48 is captured by mock");
}

/* --- Main --- */

int main(void)
{
    printf("=== WS2812 Unit Tests ===\n\n");

    test_timing_constants();
    test_grb_byte_order();
    test_init_idempotent();
    test_all_zero_rgb();
    test_gpio_passing();
    test_rmt_new_tx_channel_failure();
    test_rmt_enable_failure();
    test_rmt_new_simple_encoder_failure();
    test_rmt_transmit_failure();
    test_rmt_tx_wait_all_done_failure();
    test_encoder_callback();

    printf("\n=== Results: %d/%d checks passed ===\n", s_checks_pass, s_checks_total);
    if (s_checks_pass == s_checks_total) {
        printf("ALL TESTS PASSED\n");
        return 0;
    }
    printf("%d CHECK(S) FAILED\n", s_checks_total - s_checks_pass);
    return 1;
}
