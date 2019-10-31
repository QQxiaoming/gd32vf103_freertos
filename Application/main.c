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
    char *taskStatus = (char *)pvPortMalloc( uxTaskGetNumberOfTasks() * sizeof( TaskStatus_t ) );
    for(;;)
    {
        vTaskList(taskStatus);
        printf("\nTaskName\tStatus\tPRI\tStack\tTaskNumber\n%s",taskStatus);
        printf("current tick is %d\n",xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}


int main(void)
{  
    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL4_PRIO0); //四位优先级组全配置为lvl
    eclic_global_interrupt_enable();                       //使能全局中断

    /* 初始化led PA7 */
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    gpio_bit_reset(GPIOA, GPIO_PIN_7);

    /* 初始化uart0 TX PA9 RX PA10 */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);

    xTaskCreate(task1,"task1",521,NULL,2,NULL);
    xTaskCreate(task2,"task2",521,NULL,2,NULL);

    vTaskStartScheduler();
}
