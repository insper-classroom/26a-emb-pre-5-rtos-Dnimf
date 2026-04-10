/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;
QueueHandle_t xQueueButId;
SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;

void btn_callback(uint gpio, uint32_t events)
{
    int id = 1;
    int id_1 = 2;

    if (gpio == BTN_PIN_R)
    {
        xQueueSendFromISR(xQueueButId, &id, 0);
    }
    if (gpio == BTN_PIN_Y)
    {
        xQueueSendFromISR(xQueueButId, &id_1, 0);
    }
}
void btn_task(void *p)
{
    gpio_init(BTN_PIN_R);
    gpio_init(BTN_PIN_Y);

    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);

    gpio_pull_up(BTN_PIN_R);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);
    while (true)
    {
        int id = 0;

        if (xQueueReceive(xQueueButId, &id, 0))
        {
            if (id == 1)
            {
                // printf("veremelho \n");
                xSemaphoreGive(xSemaphore_r);
            }
            else if (id == 2)
            {
                // printf("amarelo \n");
                xSemaphoreGive(xSemaphore_y);

            }
        }
    }
}

void led_r_task(void *p){
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R,GPIO_OUT);
    int led_pode = 0;
    int led_status = 0;
    while(1){
        if(xSemaphoreTake(xSemaphore_r,pdMS_TO_TICKS(500))==pdTRUE){
            led_pode = !led_pode;
            if(!led_pode){
                led_status =0;
                gpio_put(LED_PIN_R,led_status);
            }
        }
        if(led_pode){
            gpio_put(LED_PIN_R,led_status);
            led_status = !led_status;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
void led_y_task(void *p){
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y,GPIO_OUT);
    int led_pode = 0;
    int led_status = 0;
    while(1){
        if(xSemaphoreTake(xSemaphore_y,pdMS_TO_TICKS(500))==pdTRUE){
            led_pode = !led_pode;
            if(!led_pode){
                led_status =0;
                gpio_put(LED_PIN_Y,led_status);
            }
        }
        if(led_pode){
            gpio_put(LED_PIN_Y,led_status);
            led_status = !led_status;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
int main()
{
    stdio_init_all();
    xQueueButId = xQueueCreate(32, sizeof(int));
    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();
    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_r_task, "Led_r_Task", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "Led_y_Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1)
    {
    }

    return 0;
}