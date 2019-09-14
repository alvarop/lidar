#include <stdio.h>
#include <stdlib.h>
#include <bsp/bsp.h>
#include <console/console.h>
#include <hal/hal_gpio.h>
#include <os/os.h>
#include <sysinit/sysinit.h>
#include <hal/hal_spi.h>
#include <mcu/stm32_hal.h>
#include "ws2812.h"

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

static struct os_sem sem_writing;

// Number of actual bytes it takes to encode WS2812 byte
#define BYTE_LEN (3)

// Last byte must be zero
static uint8_t pattern[BYTE_LEN * 3 * WS2812_NUM_PIXELS + 2];

// Look up table for all 256 bytes
static const uint8_t ws2812_lut[256][BYTE_LEN] = {
    {0x92,0x49,0x24},
    {0x92,0x49,0x26},
    {0x92,0x49,0x34},
    {0x92,0x49,0x36},
    {0x92,0x49,0xA4},
    {0x92,0x49,0xA6},
    {0x92,0x49,0xB4},
    {0x92,0x49,0xB6},
    {0x92,0x4D,0x24},
    {0x92,0x4D,0x26},
    {0x92,0x4D,0x34},
    {0x92,0x4D,0x36},
    {0x92,0x4D,0xA4},
    {0x92,0x4D,0xA6},
    {0x92,0x4D,0xB4},
    {0x92,0x4D,0xB6},
    {0x92,0x69,0x24},
    {0x92,0x69,0x26},
    {0x92,0x69,0x34},
    {0x92,0x69,0x36},
    {0x92,0x69,0xA4},
    {0x92,0x69,0xA6},
    {0x92,0x69,0xB4},
    {0x92,0x69,0xB6},
    {0x92,0x6D,0x24},
    {0x92,0x6D,0x26},
    {0x92,0x6D,0x34},
    {0x92,0x6D,0x36},
    {0x92,0x6D,0xA4},
    {0x92,0x6D,0xA6},
    {0x92,0x6D,0xB4},
    {0x92,0x6D,0xB6},
    {0x93,0x49,0x24},
    {0x93,0x49,0x26},
    {0x93,0x49,0x34},
    {0x93,0x49,0x36},
    {0x93,0x49,0xA4},
    {0x93,0x49,0xA6},
    {0x93,0x49,0xB4},
    {0x93,0x49,0xB6},
    {0x93,0x4D,0x24},
    {0x93,0x4D,0x26},
    {0x93,0x4D,0x34},
    {0x93,0x4D,0x36},
    {0x93,0x4D,0xA4},
    {0x93,0x4D,0xA6},
    {0x93,0x4D,0xB4},
    {0x93,0x4D,0xB6},
    {0x93,0x69,0x24},
    {0x93,0x69,0x26},
    {0x93,0x69,0x34},
    {0x93,0x69,0x36},
    {0x93,0x69,0xA4},
    {0x93,0x69,0xA6},
    {0x93,0x69,0xB4},
    {0x93,0x69,0xB6},
    {0x93,0x6D,0x24},
    {0x93,0x6D,0x26},
    {0x93,0x6D,0x34},
    {0x93,0x6D,0x36},
    {0x93,0x6D,0xA4},
    {0x93,0x6D,0xA6},
    {0x93,0x6D,0xB4},
    {0x93,0x6D,0xB6},
    {0x9A,0x49,0x24},
    {0x9A,0x49,0x26},
    {0x9A,0x49,0x34},
    {0x9A,0x49,0x36},
    {0x9A,0x49,0xA4},
    {0x9A,0x49,0xA6},
    {0x9A,0x49,0xB4},
    {0x9A,0x49,0xB6},
    {0x9A,0x4D,0x24},
    {0x9A,0x4D,0x26},
    {0x9A,0x4D,0x34},
    {0x9A,0x4D,0x36},
    {0x9A,0x4D,0xA4},
    {0x9A,0x4D,0xA6},
    {0x9A,0x4D,0xB4},
    {0x9A,0x4D,0xB6},
    {0x9A,0x69,0x24},
    {0x9A,0x69,0x26},
    {0x9A,0x69,0x34},
    {0x9A,0x69,0x36},
    {0x9A,0x69,0xA4},
    {0x9A,0x69,0xA6},
    {0x9A,0x69,0xB4},
    {0x9A,0x69,0xB6},
    {0x9A,0x6D,0x24},
    {0x9A,0x6D,0x26},
    {0x9A,0x6D,0x34},
    {0x9A,0x6D,0x36},
    {0x9A,0x6D,0xA4},
    {0x9A,0x6D,0xA6},
    {0x9A,0x6D,0xB4},
    {0x9A,0x6D,0xB6},
    {0x9B,0x49,0x24},
    {0x9B,0x49,0x26},
    {0x9B,0x49,0x34},
    {0x9B,0x49,0x36},
    {0x9B,0x49,0xA4},
    {0x9B,0x49,0xA6},
    {0x9B,0x49,0xB4},
    {0x9B,0x49,0xB6},
    {0x9B,0x4D,0x24},
    {0x9B,0x4D,0x26},
    {0x9B,0x4D,0x34},
    {0x9B,0x4D,0x36},
    {0x9B,0x4D,0xA4},
    {0x9B,0x4D,0xA6},
    {0x9B,0x4D,0xB4},
    {0x9B,0x4D,0xB6},
    {0x9B,0x69,0x24},
    {0x9B,0x69,0x26},
    {0x9B,0x69,0x34},
    {0x9B,0x69,0x36},
    {0x9B,0x69,0xA4},
    {0x9B,0x69,0xA6},
    {0x9B,0x69,0xB4},
    {0x9B,0x69,0xB6},
    {0x9B,0x6D,0x24},
    {0x9B,0x6D,0x26},
    {0x9B,0x6D,0x34},
    {0x9B,0x6D,0x36},
    {0x9B,0x6D,0xA4},
    {0x9B,0x6D,0xA6},
    {0x9B,0x6D,0xB4},
    {0x9B,0x6D,0xB6},
    {0xD2,0x49,0x24},
    {0xD2,0x49,0x26},
    {0xD2,0x49,0x34},
    {0xD2,0x49,0x36},
    {0xD2,0x49,0xA4},
    {0xD2,0x49,0xA6},
    {0xD2,0x49,0xB4},
    {0xD2,0x49,0xB6},
    {0xD2,0x4D,0x24},
    {0xD2,0x4D,0x26},
    {0xD2,0x4D,0x34},
    {0xD2,0x4D,0x36},
    {0xD2,0x4D,0xA4},
    {0xD2,0x4D,0xA6},
    {0xD2,0x4D,0xB4},
    {0xD2,0x4D,0xB6},
    {0xD2,0x69,0x24},
    {0xD2,0x69,0x26},
    {0xD2,0x69,0x34},
    {0xD2,0x69,0x36},
    {0xD2,0x69,0xA4},
    {0xD2,0x69,0xA6},
    {0xD2,0x69,0xB4},
    {0xD2,0x69,0xB6},
    {0xD2,0x6D,0x24},
    {0xD2,0x6D,0x26},
    {0xD2,0x6D,0x34},
    {0xD2,0x6D,0x36},
    {0xD2,0x6D,0xA4},
    {0xD2,0x6D,0xA6},
    {0xD2,0x6D,0xB4},
    {0xD2,0x6D,0xB6},
    {0xD3,0x49,0x24},
    {0xD3,0x49,0x26},
    {0xD3,0x49,0x34},
    {0xD3,0x49,0x36},
    {0xD3,0x49,0xA4},
    {0xD3,0x49,0xA6},
    {0xD3,0x49,0xB4},
    {0xD3,0x49,0xB6},
    {0xD3,0x4D,0x24},
    {0xD3,0x4D,0x26},
    {0xD3,0x4D,0x34},
    {0xD3,0x4D,0x36},
    {0xD3,0x4D,0xA4},
    {0xD3,0x4D,0xA6},
    {0xD3,0x4D,0xB4},
    {0xD3,0x4D,0xB6},
    {0xD3,0x69,0x24},
    {0xD3,0x69,0x26},
    {0xD3,0x69,0x34},
    {0xD3,0x69,0x36},
    {0xD3,0x69,0xA4},
    {0xD3,0x69,0xA6},
    {0xD3,0x69,0xB4},
    {0xD3,0x69,0xB6},
    {0xD3,0x6D,0x24},
    {0xD3,0x6D,0x26},
    {0xD3,0x6D,0x34},
    {0xD3,0x6D,0x36},
    {0xD3,0x6D,0xA4},
    {0xD3,0x6D,0xA6},
    {0xD3,0x6D,0xB4},
    {0xD3,0x6D,0xB6},
    {0xDA,0x49,0x24},
    {0xDA,0x49,0x26},
    {0xDA,0x49,0x34},
    {0xDA,0x49,0x36},
    {0xDA,0x49,0xA4},
    {0xDA,0x49,0xA6},
    {0xDA,0x49,0xB4},
    {0xDA,0x49,0xB6},
    {0xDA,0x4D,0x24},
    {0xDA,0x4D,0x26},
    {0xDA,0x4D,0x34},
    {0xDA,0x4D,0x36},
    {0xDA,0x4D,0xA4},
    {0xDA,0x4D,0xA6},
    {0xDA,0x4D,0xB4},
    {0xDA,0x4D,0xB6},
    {0xDA,0x69,0x24},
    {0xDA,0x69,0x26},
    {0xDA,0x69,0x34},
    {0xDA,0x69,0x36},
    {0xDA,0x69,0xA4},
    {0xDA,0x69,0xA6},
    {0xDA,0x69,0xB4},
    {0xDA,0x69,0xB6},
    {0xDA,0x6D,0x24},
    {0xDA,0x6D,0x26},
    {0xDA,0x6D,0x34},
    {0xDA,0x6D,0x36},
    {0xDA,0x6D,0xA4},
    {0xDA,0x6D,0xA6},
    {0xDA,0x6D,0xB4},
    {0xDA,0x6D,0xB6},
    {0xDB,0x49,0x24},
    {0xDB,0x49,0x26},
    {0xDB,0x49,0x34},
    {0xDB,0x49,0x36},
    {0xDB,0x49,0xA4},
    {0xDB,0x49,0xA6},
    {0xDB,0x49,0xB4},
    {0xDB,0x49,0xB6},
    {0xDB,0x4D,0x24},
    {0xDB,0x4D,0x26},
    {0xDB,0x4D,0x34},
    {0xDB,0x4D,0x36},
    {0xDB,0x4D,0xA4},
    {0xDB,0x4D,0xA6},
    {0xDB,0x4D,0xB4},
    {0xDB,0x4D,0xB6},
    {0xDB,0x69,0x24},
    {0xDB,0x69,0x26},
    {0xDB,0x69,0x34},
    {0xDB,0x69,0x36},
    {0xDB,0x69,0xA4},
    {0xDB,0x69,0xA6},
    {0xDB,0x69,0xB4},
    {0xDB,0x69,0xB6},
    {0xDB,0x6D,0x24},
    {0xDB,0x6D,0x26},
    {0xDB,0x6D,0x34},
    {0xDB,0x6D,0x36},
    {0xDB,0x6D,0xA4},
    {0xDB,0x6D,0xA6},
    {0xDB,0x6D,0xB4},
    {0xDB,0x6D,0xB6},
};


void ws2812_set_pixel(uint16_t pixel, uint8_t red, uint8_t green, uint8_t blue) {
    hal_gpio_write(MCU_GPIO_PORTB(1), 1);
    if (pixel < WS2812_NUM_PIXELS) {
        uint8_t *pixel_addr = &pattern[1 + pixel * BYTE_LEN * 3];
        memcpy(&pixel_addr[0], ws2812_lut[green], BYTE_LEN);
        memcpy(&pixel_addr[BYTE_LEN], ws2812_lut[red], BYTE_LEN);
        memcpy(&pixel_addr[BYTE_LEN * 2], ws2812_lut[blue], BYTE_LEN);
    }
    hal_gpio_write(MCU_GPIO_PORTB(1), 0);
}

void ws2812_write() {
    // Only write if we're not already writing
    if(os_sem_pend(&sem_writing, 0) == OS_OK) {
        HAL_SPI_Transmit_DMA(&hspi1, pattern, sizeof(pattern) - 1);
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);
  os_sem_release(&sem_writing);

}

/**
* @brief SPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hspi->Instance==SPI1)
  {

    /* Peripheral clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* SPI1 DMA Init */
    /* SPI1_TX Init */
    hdma_spi1_tx.Instance = DMA1_Channel3;
    hdma_spi1_tx.Init.Request = DMA_REQUEST_1;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_HIGH;
    assert(HAL_DMA_Init(&hdma_spi1_tx) == HAL_OK);


    __HAL_LINKDMA(hspi,hdmatx,hdma_spi1_tx);


  }

}

extern DMA_HandleTypeDef hdma_spi1_tx;
void DMA1_Channel3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_spi1_tx);
}


int ws2812_init(void){

    os_sem_init(&sem_writing, 1);

    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    NVIC_SetVector(DMA1_Channel3_IRQn, (uint32_t)DMA1_Channel3_IRQHandler);

    /* SPI1 parameter configuration*/
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    // Enabling NSSPMode causes gaps between bytes and breaks things!
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    assert(HAL_SPI_Init(&hspi1) == HAL_OK);

    HAL_SPI_MspInit(&hspi1);

    pattern[sizeof(pattern)] = 0;

    for(uint16_t pixel = 0; pixel < WS2812_NUM_PIXELS; pixel++) {
        ws2812_set_pixel(pixel, 0, 0 ,0);
    }

    ws2812_write();


    return 0;
}

