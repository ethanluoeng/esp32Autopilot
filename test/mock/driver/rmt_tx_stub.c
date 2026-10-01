/*
 * rmt_tx_stub.c - Mock implementations of the RMT driver API used by ws2812.h.
 *
 * Backs the declarations in driver/rmt_tx.h with controllable stubs so the
 * host-based unit tests can:
 *   - inject success/failure for each RMT call,
 *   - capture the GPIO and the transmitted GRB payload for assertions.
 */
#include "driver/rmt_tx.h"
#include <string.h>

/* Complete definitions for the opaque handle types declared in the header. */
struct rmt_tx_channel { int _id; };
struct rmt_encoder   { int _id; };

/* --- Test-control knobs --- */
esp_err_t mock_rmt_new_tx_channel_result  = ESP_OK;
esp_err_t mock_rmt_enable_result          = ESP_OK;
esp_err_t mock_rmt_new_simple_encoder_result = ESP_OK;
esp_err_t mock_rmt_transmit_result        = ESP_OK;
esp_err_t mock_rmt_tx_wait_all_done_result = ESP_OK;

/* Captured state. Backed by a static buffer so it remains valid after
 * ws2812_set_rgb() returns (the real GRB array is stack-local). */
static uint8_t s_last_transmit_buf[64];
const uint8_t *mock_last_transmit_data = s_last_transmit_buf;
size_t         mock_last_transmit_size = 0;
int            mock_last_gpio_num      = -1;

/* Counters so tests can verify init happens exactly once. */
int mock_new_tx_channel_calls   = 0;
int mock_new_simple_encoder_calls = 0;
int mock_transmit_calls         = 0;

esp_err_t rmt_new_tx_channel(const rmt_tx_channel_config_t *tx_chan_cfg,
                             rmt_channel_handle_t *ret_tx_chan)
{
    mock_new_tx_channel_calls++;
    mock_last_gpio_num = (int)tx_chan_cfg->gpio_num;
    static struct rmt_tx_channel s_chan = { 1 };
    if (mock_rmt_new_tx_channel_result != ESP_OK) {
        return mock_rmt_new_tx_channel_result;
    }
    if (ret_tx_chan) {
        *ret_tx_chan = &s_chan;
    }
    return ESP_OK;
}

esp_err_t rmt_new_simple_encoder(const rmt_simple_encoder_config_t *config,
                                 rmt_encoder_handle_t *ret_encoder)
{
    mock_new_simple_encoder_calls++;
    static struct rmt_encoder s_enc = { 2 };
    if (config == NULL || config->callback == NULL) {
        return ESP_FAIL;
    }
    if (mock_rmt_new_simple_encoder_result != ESP_OK) {
        return mock_rmt_new_simple_encoder_result;
    }
    if (ret_encoder) {
        *ret_encoder = &s_enc;
    }
    return ESP_OK;
}

esp_err_t rmt_enable(rmt_channel_handle_t chan)
{
    (void)chan;
    return mock_rmt_enable_result;
}

esp_err_t rmt_transmit(rmt_channel_handle_t chan, void *encoder,
                       const void *data, size_t data_size,
                       const rmt_transmit_config_t *tx_cfg)
{
    (void)chan; (void)encoder; (void)tx_cfg;
    mock_transmit_calls++;
    if (data_size > sizeof(s_last_transmit_buf)) {
        data_size = sizeof(s_last_transmit_buf);
    }
    memcpy(s_last_transmit_buf, data, data_size);
    mock_last_transmit_data = s_last_transmit_buf;
    mock_last_transmit_size = data_size;
    return mock_rmt_transmit_result;
}

esp_err_t rmt_tx_wait_all_done(rmt_channel_handle_t chan, uint32_t timeout_ms)
{
    (void)chan; (void)timeout_ms;
    return mock_rmt_tx_wait_all_done_result;
}

esp_err_t rmt_del_channel(rmt_channel_handle_t chan)
{
    (void)chan;
    return ESP_OK;
}
