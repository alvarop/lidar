#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "ws2812.h"


#define BLINK_TASK_PRI         (99)
#define BLINK_STACK_SIZE       (64)
struct os_task blink_task;
os_stack_t blink_task_stack[BLINK_STACK_SIZE];

extern uint32_t SystemCoreClock;

void blink_task_fn(void *arg) {

    // hal_gpio_init_out(LED_BLINK_PIN, 0);
    ws2812_init();

    while(1) {
        for (uint8_t led = 1; led < 24; led++) {
            os_time_delay(OS_TICKS_PER_SEC/32);
            ws2812_set_pixel(led,10,0,0);
            ws2812_write();
            os_time_delay(OS_TICKS_PER_SEC/32);
            ws2812_set_pixel(led,0,0,0);
            ws2812_write();
        }

        for (uint8_t led = 24; led > 1; led--) {
            os_time_delay(OS_TICKS_PER_SEC/32);
            ws2812_set_pixel(led,10,0,0);
            ws2812_write();
            os_time_delay(OS_TICKS_PER_SEC/32);
            ws2812_set_pixel(led,0,0,0);
            ws2812_write();
        }
        // hal_gpio_toggle(LED_BLINK_PIN);
        // console_printf("Tick...\n");
        //
    }

}

int
main(int argc, char **argv)
{
    sysinit();

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
