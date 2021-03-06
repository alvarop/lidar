#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "ws2812.h"

#define BLINK_TASK_PRI         (50)
#define BLINK_STACK_SIZE       (1024)
struct os_task blink_task;
os_stack_t blink_task_stack[BLINK_STACK_SIZE];

#define LEDS_TASK_PRI         (100)
#define LEDS_STACK_SIZE       (64)
struct os_task leds_task;
os_stack_t leds_task_stack[LEDS_STACK_SIZE];

extern uint32_t SystemCoreClock;

void leds_task_fn(void *arg) {
    ws2812_init();

    while(1) {

        ws2812_write();
        os_time_delay(5);
    }

}

void blink_task_fn(void *arg) {

    while(1) {
        for (uint16_t led = 0; led < WS2812_NUM_PIXELS; led++) {
            ws2812_set_pixel(led,100,0,0);
            os_time_delay(20);
            ws2812_set_pixel(led,0,0,0);
        }

        for (uint16_t led = WS2812_NUM_PIXELS-2; led > 1; led--) {
            ws2812_set_pixel(led,100,0,0);
            os_time_delay(20);
            ws2812_set_pixel(led,0,0,0);
        }
        os_time_delay(20);
    }

}

int
main(int argc, char **argv)
{
    sysinit();

    hal_gpio_init_out(MCU_GPIO_PORTB(2), 0);
    hal_gpio_init_out(MCU_GPIO_PORTB(1), 0);
    hal_gpio_init_out(MCU_GPIO_PORTB(13), 0);
    hal_gpio_init_out(MCU_GPIO_PORTB(14), 0);
    hal_gpio_init_out(MCU_GPIO_PORTB(15), 0);


    os_task_init(
        &leds_task,
        "leds_task",
        leds_task_fn,
        NULL,
        LEDS_TASK_PRI,
        OS_WAIT_FOREVER,
        leds_task_stack,
        LEDS_STACK_SIZE);

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
