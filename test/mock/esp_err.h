/*
 * esp_err.h - Mock ESP error type for host-based unit testing.
 *
 * Provides a minimal definition of esp_err_t and ESP_OK so that
 * ws2812.h can be compiled on the host without the full ESP-IDF.
 */
#ifndef MOCK_ESP_ERR_H
#define MOCK_ESP_ERR_H

#include <stdint.h>

typedef int esp_err_t;

#define ESP_OK 0
#define ESP_FAIL (-1)

static inline const char *esp_err_to_name(esp_err_t e)
{
    return (e == ESP_OK) ? "ESP_OK" : "ESP_FAIL";
}

#endif /* MOCK_ESP_ERR_H */
