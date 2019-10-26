/**
 * @file main.c
 * @author qiming.qiao
 * @brief gd32vf103 移植 FreeRTOS
 * @version 1.0
 * @date 2019-10-20
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "gd32vf103.h"
#include "FreeRTOS.h" 
#include "task.h"


void task1(void *p)
{
    for(;;)
    {
        gpio_bit_write(GPIOA, GPIO_PIN_7, (bit_status)(1-gpio_input_bit_get(GPIOA, GPIO_PIN_7)));

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void task2(void *p)
{
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


int main(void)
{  
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    gpio_bit_reset(GPIOA, GPIO_PIN_7);
    
    xTaskCreate(task1,"task1",521,NULL,2,NULL);
    xTaskCreate(task2,"task2",521,NULL,2,NULL);

    vTaskStartScheduler();
}
