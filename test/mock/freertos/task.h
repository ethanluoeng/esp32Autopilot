/*
 * task.h - Mock FreeRTOS task API for host-based unit testing.
 *
 * Declares vTaskDelay(); the implementation is provided by the test that
 * needs to observe/cut off the blink loop (see test_main.c).
 */
#ifndef MOCK_FREERTOS_TASK_H
#define MOCK_FREERTOS_TASK_H

#include "freertos/FreeRTOS.h"

void vTaskDelay(TickType_t xTicksToDelay);

#endif /* MOCK_FREERTOS_TASK_H */
