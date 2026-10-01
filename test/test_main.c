/*
 * test_main.c - Host-based unit tests for main.c (app_main blink loop).
 *
 * Strategy: app_main() runs an infinite while(1) loop that calls
 * ws2812_set_rgb() then vTaskDelay(). We mock vTaskDelay() to longjmp() out
 * of the loop after a configurable number of delays, letting the test assert
 * on the sequence of transmitted colors and delay intervals.
 *
 * Verified behavior (from the task spec):
 *   - Startup log: "WS2812 LED on GPIO %d, blink interval %d ms"
 *   - Green ON  : ws2812_set_rgb(GPIO, 0, 255, 0)
 *   - OFF       : ws2812_set_rgb(GPIO, 0, 0, 0)
 *   - Delay     : CONFIG_BLINK_INTERVAL_MS between each
 */

#include <stdio.h>
#include <setjmp.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"

/* Pull in the app under test. main.c defines app_main(). */
#include "main.c"

/* ---- RMT mock knobs / capture (defined in rmt_tx_stub.c) ---- */
extern esp_err_t mock_rmt_new_tx_channel_result;
extern esp_err_t mock_rmt_enable_result;
extern esp_err_t mock_rmt_new_simple_encoder_result;
extern esp_err_t mock_rmt_transmit_result;
extern esp_err_t mock_rmt_tx_wait_all_done_result;
extern const uint8_t *mock_last_transmit_data;
extern size_t mock_last_transmit_size;
extern int mock_transmit_calls;

/* ---- Log capture (defined in this file / esp_log.h) ---- */
char mock_last_log[256];
int  mock_log_count = 0;

/*
 * vTaskDelay mock: records each requested interval and, once enough have
 * been observed, longjmps back to the test to escape app_main()'s loop.
 */
static jmp_buf s_break_env;
static int s_delay_calls = 0;
static int s_max_delays;          /* stop after this many delays */
static TickType_t s_delays[16];

void vTaskDelay(TickType_t xTicksToDelay)
{
    s_delay_calls++;
    if (s_delay_calls < 16) {
        s_delays[s_delay_calls - 1] = xTicksToDelay;
    }
    if (s_delay_calls >= s_max_delays) {
        longjmp(s_break_env, 1);
    }
}

/* Reset RMT mock state between sub-tests. Note: the static RMT channel
 * handle inside ws2812.h is not reset, so after the first init the channel
 * is reused (which matches real hardware behaviour). */
static void mock_reset(void)
{
    mock_rmt_new_tx_channel_result    = ESP_OK;
    mock_rmt_enable_result            = ESP_OK;
    mock_rmt_new_simple_encoder_result = ESP_OK;
    mock_rmt_transmit_result          = ESP_OK;
    mock_rmt_tx_wait_all_done_result  = ESP_OK;
    mock_transmit_calls               = 0;
    mock_last_transmit_data           = NULL;
    mock_last_transmit_size           = 0;
    mock_log_count                    = 0;
    s_delay_calls                     = 0;
}

int main(void)
{
    int failures = 0;

    /* ============ Test 1: startup log banner ============ */
    mock_reset();
    s_max_delays = 1; /* break after first delay so we observe the first cycle */
    if (setjmp(s_break_env) == 0) {
        app_main();
    }
    printf("startup log: '%s'\n", mock_last_log);
    if (strstr(mock_last_log, "WS2812 LED on GPIO 48, blink interval 500 ms") != NULL) {
        printf("[PASS] startup banner correct\n");
    } else {
        printf("[FAIL] startup banner incorrect\n");
        failures++;
    }

    /* ============ Test 2: first transmit is green ON ============ */
    /* After app_main ran, the first ws2812_set_rgb was (0,255,0) -> GRB {255,0,0} */
    printf("last transmitted: %d,%d,%d\n",
           mock_last_transmit_data[0], mock_last_transmit_data[1], mock_last_transmit_data[2]);
    if (mock_last_transmit_size == 3 &&
        mock_last_transmit_data[0] == 255 &&  /* G */
        mock_last_transmit_data[1] == 0   &&  /* R */
        mock_last_transmit_data[2] == 0   ) { /* B */
        printf("[PASS] first color is green ON (G=255)\n");
    } else {
        printf("[FAIL] first color is not green ON\n");
        failures++;
    }

    /* ============ Test 3: delay interval = CONFIG_BLINK_INTERVAL_MS ============ */
    if (s_delay_calls >= 1 && s_delays[0] == pdMS_TO_TICKS(CONFIG_BLINK_INTERVAL_MS)) {
        printf("[PASS] delay interval == %d ms\n", CONFIG_BLINK_INTERVAL_MS);
    } else {
        printf("[FAIL] delay interval is %u, expected %d\n",
               (unsigned)s_delays[0], CONFIG_BLINK_INTERVAL_MS);
        failures++;
    }

    /* ============ Test 4: full blink cycle (green ON -> OFF) ============ */
    mock_reset();
    s_max_delays = 2; /* observe two delays: one after ON, one after OFF */

    /* The mock captures the LAST transmitted frame. After 2 delays the last
     * executed set_rgb was OFF (0,0,0) -> GRB {0,0,0}. */
    if (setjmp(s_break_env) == 0) {
        app_main();
    }
    static uint8_t payload[3];
    memcpy(payload, mock_last_transmit_data, 3);
    if (mock_last_transmit_size == 3 &&
        payload[0] == 0 && payload[1] == 0 && payload[2] == 0) {
        printf("[PASS] blink cycle ends with OFF (0,0,0)\n");
    } else {
        printf("[FAIL] expected OFF but got %d,%d,%d\n",
               payload[0], payload[1], payload[2]);
        failures++;
    }

    printf("\n=== test_main: %s ===\n", failures == 0 ? "ALL PASSED" : "FAILURES");
    return failures == 0 ? 0 : 1;
}
