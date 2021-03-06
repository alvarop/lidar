#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include <fifo/fifo.h>
#include <ws2812b/ws2812b.h>
#include <rplidar/rplidar.h>


#define RPLIDAR_TASK_PRI         (50)
#define RPLIDAR_STACK_SIZE       (256)
struct os_task rplidar_task;
os_stack_t rplidar_task_stack[RPLIDAR_STACK_SIZE];

#define RPLIDAR_PRINT_TASK_PRI         (98)
#define RPLIDAR_PRINT_STACK_SIZE       (256)
struct os_task rplidar_print_task;
os_stack_t rplidar_print_task_stack[RPLIDAR_PRINT_STACK_SIZE];

#define LEDS_TASK_PRI         (100)
#define LEDS_STACK_SIZE       (64)
struct os_task leds_task;
os_stack_t leds_task_stack[LEDS_STACK_SIZE];

extern uint32_t SystemCoreClock;

extern uint16_t distances[360];

extern ws2812b_led_t leds[WS2812B_NUM_PIXELS];

uint8_t fade_amount = 1;

void fade() {
    hal_gpio_write(MCU_GPIO_PORTB(14), 1);
    for (uint16_t led = 0; led < WS2812B_NUM_PIXELS; led++) {
        if(leds[led].green > fade_amount) {
            leds[led].green -= fade_amount;
        } else {
            leds[led].green = 0;
        }

        if(leds[led].red > fade_amount) {
            leds[led].red -= fade_amount;
        } else {
            leds[led].red = 0;
        }

        if(leds[led].blue > fade_amount) {
            leds[led].blue -= fade_amount;
        } else {
            leds[led].blue = 0;
        }
    }
    hal_gpio_write(MCU_GPIO_PORTB(14), 0);
}

void leds_task_fn(void *arg) {

    while(1) {
        uint16_t led = 0;

        while(led < WS2812B_NUM_PIXELS){
            for(uint8_t sub_count = 0; sub_count < 10; sub_count++){
                leds[led].green = 10 * sub_count + 10;
                leds[led].red = 10 * sub_count + 10;
                leds[led].blue = 10 * sub_count + 10;
                fade();
                os_time_delay(30);
            }
            led++;
        }
    }

}

void rplidar_print_task_fn(void *arg) {
    while(1) {
        for(uint16_t bin = 0; bin < 360; bin++) {
            console_printf("%04x", distances[bin]);
        }
        console_printf("\n");
        os_time_delay(30);
    }
}

void rplidar_task_fn(void *arg) {

    // console_printf("LIDAR Test!\n");

    rplidar_print_info();

    rplidar_enable_motor();
    os_time_delay(OS_TICKS_PER_SEC * 3);
    rplidar_start_scan();
    int32_t rval = rplidar_run();

    if(rval) {
        console_printf("RPLIDAR RUN ERR %ld\n", rval);
    }

    while(1) {
        os_time_delay(1000);
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

    rplidar_init();
    ws2812b_init(10);

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
        &rplidar_task,
        "rplidar_task",
        rplidar_task_fn,
        NULL,
        RPLIDAR_TASK_PRI,
        OS_WAIT_FOREVER,
        rplidar_task_stack,
        RPLIDAR_STACK_SIZE);

    os_task_init(
        &rplidar_print_task,
        "rplidar_print_task",
        rplidar_print_task_fn,
        NULL,
        RPLIDAR_PRINT_TASK_PRI,
        OS_WAIT_FOREVER,
        rplidar_print_task_stack,
        RPLIDAR_PRINT_STACK_SIZE);


    while(1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    assert(0);

    return 0;
}
