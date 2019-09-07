#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "ws2812.h"
#include <fifo/fifo.h>
#include <raw_uart/raw_uart.h>


#define BLINK_TASK_PRI         (99)
#define BLINK_STACK_SIZE       (64)
struct os_task blink_task;
os_stack_t blink_task_stack[BLINK_STACK_SIZE];

extern uint32_t SystemCoreClock;

void blink_task_fn(void *arg) {

    console_printf("LIDAR Test!\n");

    ws2812_init();

    ws2812_write();

    while(1) {
        for (uint16_t led = 0; led < WS2812_NUM_PIXELS; led++) {
            os_time_delay(10);
            ws2812_set_pixel(led,100,0,0);
            ws2812_write();
            os_time_delay(10);
            ws2812_set_pixel(led,0,0,0);
            ws2812_write();
        }

        for (uint16_t led = WS2812_NUM_PIXELS-2; led > 1; led--) {
            os_time_delay(10);
            ws2812_set_pixel(led,100,0,0);
            ws2812_write();
            os_time_delay(10);
            ws2812_set_pixel(led,0,0,0);
            ws2812_write();
        }
    }

}

void uart_rx_handler(struct os_event *ev) {
    fifo_t *fifo = ev->ev_arg;

    while(fifo_size(fifo)) {
        console_printf("%c", (char)fifo_pop(fifo));
    }
}

int
main(int argc, char **argv)
{
    sysinit();

    raw_uart_init(&uart_rx_handler);

    os_task_init(
        &blink_task,
        "blink_task",
        blink_task_fn,
        NULL,
        BLINK_TASK_PRI,
        OS_WAIT_FOREVER,
        blink_task_stack,
        BLINK_STACK_SIZE);

    while(1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    assert(0);

    return 0;
}
