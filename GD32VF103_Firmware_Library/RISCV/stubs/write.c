/* See LICENSE of license details. */

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include "stub.h"
#include "gd32vf103.h"
#include "app_config.h"


typedef unsigned int size_t;

extern int _put_char(int ch) __attribute__((weak));


ssize_t _write(int fd, const void* ptr, size_t len)
{
    const uint8_t * current = (const uint8_t *)ptr;

    if (isatty(fd))
    {
        for (size_t jj = 0; jj < len; jj++)
        {
            if (current[jj] == '\n')
            {
                _put_char('\r');
            }
            _put_char(current[jj]);
        }
        return len;
    }

    return _stub(EBADF);
}


int __wrap_printf(const char* fmt, ...)
{
    // You can implement your own printf to reduce the code size, because the printf is really a big function
	return 0;
}


int _put_char(int ch)
{
    #if UARTLOGEN
    usart_data_transmit(UARTLOG_PORT, (uint8_t) ch );
    while (usart_flag_get(UARTLOG_PORT, USART_FLAG_TBE)== RESET){
    }
    #endif

    return ch;
}
