#ifndef __XBEE_UART_H__
#define __XBEE_UART_H__

#include <stdint.h>
#include <os/os.h>

int32_t raw_uart_init(void (*rx_ev_fn)(struct os_event *ev));
int32_t raw_uart_tx(void * buff, uint32_t len);
int32_t raw_uart_flush_rx();

#endif
