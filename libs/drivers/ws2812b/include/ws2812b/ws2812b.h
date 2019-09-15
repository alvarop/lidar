#ifndef __WS2812B_H__
#define __WS2812B_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t ws2812b_init(uint32_t period_ms);

void ws2812b_write(void);

void ws2812b_set_pixel(uint16_t pixel, uint8_t red, uint8_t green, uint8_t blue);

int32_t ws2812b_set_period(uint32_t ms);


#define WS2812B_NUM_PIXELS 272

#ifdef __cplusplus
}
#endif

#endif /* __WS2812B_H__ */
