/*
 * esp_log.h - Mock ESP logging for host-based unit testing.
 *
 * Captures the last formatted log message into mock_last_log[] and counts
 * calls, so tests can assert on the startup banner and on error messages.
 */
#ifndef MOCK_ESP_LOG_H
#define MOCK_ESP_LOG_H

#include <stdio.h>
#include <string.h>

extern char mock_last_log[256];
extern int  mock_log_count;

#define _MOCK_LOG_IMPL(tag, level, fmt, ...) do { \
    snprintf(mock_last_log, sizeof(mock_last_log), "%s %s: " fmt, tag, level, ##__VA_ARGS__); \
    mock_log_count++; \
    printf("%s\n", mock_last_log); \
} while (0)

#define ESP_LOGI(tag, fmt, ...) _MOCK_LOG_IMPL(tag, "I", fmt, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) _MOCK_LOG_IMPL(tag, "W", fmt, ##__VA_ARGS__)
#define ESP_LOGE(tag, fmt, ...) _MOCK_LOG_IMPL(tag, "E", fmt, ##__VA_ARGS__)
#define ESP_LOGD(tag, fmt, ...) _MOCK_LOG_IMPL(tag, "D", fmt, ##__VA_ARGS__)

#endif /* MOCK_ESP_LOG_H */
