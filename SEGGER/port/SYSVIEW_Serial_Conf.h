#ifndef _SYSVIEW_SERIAL_CONF_H_
#define _SYSVIEW_SERIAL_CONF_H_

#include <stdint.h>
#include "gd32vf103.h"


#define CONFIG_SYSTEMVIEW_EN            1
#define CONFIG_SYSVIEW_UART_PORT        (USART0)

extern void vSYSVIEWUARTInit(void);
extern void vSYSVIEWUARTInterruptHandler(void);

#endif  /* _SYSVIEW_SERIAL_CONF_H_ */
