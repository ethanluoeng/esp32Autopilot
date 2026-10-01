/*
 * main.c - WS2812 LED blink application for ESP32-S3 DevKitC
 *
 * Blinks the on-board WS2812 RGB LED (GPIO 48) at 1 Hz (500 ms on, 500 ms off).
 * Configurable via Kconfig: CONFIG_LED_GPIO, CONFIG_BLINK_INTERVAL_MS.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ws2812.h"

static const char *TAG = "ws2812_blink";

void app_main(void)
{
    ESP_LOGI(TAG, "WS2812 LED on GPIO %d, blink interval %d ms",
             CONFIG_LED_GPIO, CONFIG_BLINK_INTERVAL_MS);

    while (1) {
        /* Turn LED on (green) */
        esp_err_t err = ws2812_set_rgb(CONFIG_LED_GPIO, 0, 255, 0);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ws2812_set_rgb ON failed: %s", esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_BLINK_INTERVAL_MS));

        /* Turn LED off */
        err = ws2812_set_rgb(CONFIG_LED_GPIO, 0, 0, 0);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ws2812_set_rgb OFF failed: %s", esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_BLINK_INTERVAL_MS));
    }
}
