#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"
#include "SYSVIEW_Serial_Conf.h"
#include "gd32vf103.h"

#define _SERVER_HELLO_SIZE (4)
#define _TARGET_HELLO_SIZE (4)

static struct
{
    U8 NumBytesHelloRcvd;
    U8 NumBytesHelloSent;
    int ChannelID;
} _SVInfo;

static const U8 _abHelloMsg[_TARGET_HELLO_SIZE] = {'S', 'V', (SEGGER_SYSVIEW_VERSION / 10000), (SEGGER_SYSVIEW_VERSION / 1000) % 10}; // "Hello" message expected by SysView: [ 'S', 'V', <PROTOCOL_MAJOR>, <PROTOCOL_MINOR> ]


/**
 * @brief This function starts and initializes a SystemView session, if necessary.
 * 
 */
static void _StartSysView(void)
{
    int r;

    r = SEGGER_SYSVIEW_IsStarted();
    if (r == 0)
    {
        SEGGER_SYSVIEW_Start();
    }
}


/**
 * @brief This function is called when the UART receives data.
 * 
 * @param Data 
 */
static void _cbOnRx(U8 Data)
{
    if (_SVInfo.NumBytesHelloRcvd < _SERVER_HELLO_SIZE)
    { // Not all bytes of <Hello> message received by SysView yet?
        _SVInfo.NumBytesHelloRcvd++;
        /* 目前版本V3.10，增加这个判断才能正确启动  modify by QQM */
        if(_SVInfo.NumBytesHelloRcvd == _SERVER_HELLO_SIZE-1)
        {
        	_StartSysView();
        }
        goto Done;
    }
    _StartSysView();
    SEGGER_RTT_WriteDownBuffer(_SVInfo.ChannelID, &Data, 1); // Write data into corresponding RTT buffer for application to read and handle accordingly
Done:
    return;
}


/**
 * @brief This function is called when the UART should transmit data.
 * 
 * @param pChar 
 * @return int 
 */
static int _cbOnTx(U8 *pChar)
{
    int r;

    if (_SVInfo.NumBytesHelloSent < _TARGET_HELLO_SIZE)
    { // Not all bytes of <Hello> message sent to SysView yet?
        *pChar = _abHelloMsg[_SVInfo.NumBytesHelloSent];
        _SVInfo.NumBytesHelloSent++;
        r = 1;
        goto Done;
    }
    r = SEGGER_RTT_ReadUpBufferNoLock(_SVInfo.ChannelID, pChar, 1);
    if (r < 0)
    { // Failed to read from up buffer?
        r = 0;
    }
Done:
    return r;
}


void vSYSVIEWUARTEnableTXEInterrupt(U32 NumBytes)
{
    usart_interrupt_enable(USART0, USART_INT_TBE);
}


/**
 * @brief sysview uart handle
 * 
 */
void vSYSVIEWUARTInterruptHandler(void)
{
    U8 cChar;

    if(RESET != usart_interrupt_flag_get(CONFIG_SYSVIEW_UART_PORT, USART_INT_FLAG_RBNE)){
        /* receive data */
        cChar = usart_data_receive(CONFIG_SYSVIEW_UART_PORT);
        _cbOnRx(cChar);
    }
    if(RESET != usart_interrupt_flag_get(CONFIG_SYSVIEW_UART_PORT, USART_INT_FLAG_TBE)){
        if (0 == _cbOnTx(&cChar))
        {
            usart_interrupt_disable(CONFIG_SYSVIEW_UART_PORT, USART_INT_TBE);
        }
        else
        {
            /* transmit data */
            usart_data_transmit(CONFIG_SYSVIEW_UART_PORT, cChar);
        }
    }
}


/**
 * @brief sysview uart init
 * 
 */
void vSYSVIEWUARTInit(void)
{
    _SVInfo.ChannelID = SEGGER_SYSVIEW_GetChannelID(); // Store system view channel ID for later communication

    if(CONFIG_SYSVIEW_UART_PORT == USART0)
    {
        /* 初始化uart0 TX PA9 RX PA10 */
        rcu_periph_clock_enable(RCU_GPIOA);
        rcu_periph_clock_enable(RCU_USART0);
        gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
        gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
        usart_deinit(CONFIG_SYSVIEW_UART_PORT);
        usart_baudrate_set(CONFIG_SYSVIEW_UART_PORT, 115200U);
        usart_word_length_set(CONFIG_SYSVIEW_UART_PORT, USART_WL_8BIT);
        usart_stop_bit_set(CONFIG_SYSVIEW_UART_PORT, USART_STB_1BIT);
        usart_parity_config(CONFIG_SYSVIEW_UART_PORT, USART_PM_NONE);
        usart_hardware_flow_rts_config(CONFIG_SYSVIEW_UART_PORT, USART_RTS_DISABLE);
        usart_hardware_flow_cts_config(CONFIG_SYSVIEW_UART_PORT, USART_CTS_DISABLE);
        usart_receive_config(CONFIG_SYSVIEW_UART_PORT, USART_RECEIVE_ENABLE);
        usart_transmit_config(CONFIG_SYSVIEW_UART_PORT, USART_TRANSMIT_ENABLE);
        usart_interrupt_disable(CONFIG_SYSVIEW_UART_PORT, USART_INT_TBE);
        usart_interrupt_enable(CONFIG_SYSVIEW_UART_PORT, USART_INT_RBNE);
        usart_enable(CONFIG_SYSVIEW_UART_PORT);

        eclic_irq_enable(USART0_IRQn, 1, 0);
    }
}
