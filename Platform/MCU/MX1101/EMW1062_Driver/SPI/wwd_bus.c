/**
******************************************************************************
* @file    wwd_bus.c 
* @author  William Xu
* @version V1.0.0
* @date    16-Sep-2014
* @brief   This file provides bus communication functions with Wi-Fi RF chip.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/

#include "MicoRtos.h"
#include "misc.h"
#include "string.h" /* For memcpy */
#include "gpio_irq.h"
#include "platform_common_config.h"
#include "stm32f2xx_platform.h"
#include "PlatformLogging.h"

/******************************************************
 *             Constants
 ******************************************************/

#define DMA_TIMEOUT_LOOPS      (10000000)

/**
 * Transfer direction for the mico platform bus interface
 */
typedef enum
{
    /* If updating this enum, the bus_direction_mapping variable will also need to be updated */
    BUS_READ,
    BUS_WRITE
} bus_transfer_direction_t;

/******************************************************
 *             Structures
 ******************************************************/

/******************************************************
 *             Variables
 ******************************************************/

static mico_semaphore_t spi_transfer_finished_semaphore;

/******************************************************
 *             Function declarations
 ******************************************************/

/* Powersave functionality */
extern void MCU_CLOCKS_NEEDED( void );
extern void MCU_CLOCKS_NOT_NEEDED( void );

extern void wiced_platform_notify_irq( void );

/******************************************************
 *             Function definitions
 ******************************************************/

static void spi_irq_handler( void* arg )
{
    UNUSED_PARAMETER(arg);
    wiced_platform_notify_irq( );
}

void dma_irq( void )
{
    /* Clear interrupt */
    DMA_ClearITPendingBit(SPIX_DMA_RX_STREAM, SPIX_DMA_RX_TCIT);
    mico_rtos_set_semaphore( &spi_transfer_finished_semaphore );
}

OSStatus host_platform_bus_init( void )
{
    SPI_InitTypeDef  spi_init;
    DMA_InitTypeDef  dma_init_structure;
    GPIO_InitTypeDef gpio_init_structure;
    NVIC_InitTypeDef nvic_init_structure;

    MCU_CLOCKS_NEEDED();

    mico_rtos_init_semaphore(&spi_transfer_finished_semaphore, 1);

    /* Enable SPI_SLAVE DMA clock */
    RCC_AHB1PeriphClockCmd( SPIX_DMA_CLK, ENABLE );

    /* Enable SPI_SLAVE Periph clock */
    SPIX_CLK_FUNCTION( SPIX_CLK, ENABLE );

    /* Enable GPIO Bank B & C */
    RCC_AHB1PeriphClockCmd( SPI_BUS_CLOCK_BANK_CLK | SPI_BUS_MISO_BANK_CLK | SPI_BUS_MOSI_BANK_CLK | SPI_BUS_CS_BANK_CLK | SPI_IRQ_CLK, ENABLE );

    /* Enable SYSCFG. Needed for selecting EXTI interrupt line */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, ENABLE );

    /* Setup the interrupt input for WLAN_IRQ */
    gpio_init_structure.GPIO_Mode = GPIO_Mode_IN;
    gpio_init_structure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_structure.GPIO_Pin = ( 1 << SPI_IRQ_PIN );
    GPIO_Init( SPI_IRQ_BANK, &gpio_init_structure );

    gpio_irq_enable(SPI_IRQ_BANK, SPI_IRQ_PIN, IRQ_TRIGGER_RISING_EDGE, spi_irq_handler, 0);

    /* Setup the SPI lines */
    /* Setup SPI slave select GPIOs */
    gpio_init_structure.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_structure.GPIO_Pin = ( 1 << SPI_BUS_CLOCK_PIN ) | ( 1 << SPI_BUS_MISO_PIN ) | ( 1 << SPI_BUS_MOSI_PIN );
    GPIO_Init( SPI_BUS_CLOCK_BANK, &gpio_init_structure );
    GPIO_PinAFConfig( SPI_BUS_CLOCK_BANK, SPI_BUS_CLOCK_PIN, SPIX_AF );
    GPIO_PinAFConfig( SPI_BUS_MISO_BANK, SPI_BUS_MISO_PIN, SPIX_AF );
    GPIO_PinAFConfig( SPI_BUS_MOSI_BANK, SPI_BUS_MOSI_PIN, SPIX_AF );

    /* Setup SPI slave select GPIOs */
    gpio_init_structure.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_structure.GPIO_Pin = ( 1 << SPI_BUS_CS_PIN );
    GPIO_Init( SPI_BUS_CS_BANK, &gpio_init_structure );
    GPIO_SetBits( SPI_BUS_CS_BANK, ( 1 << SPI_BUS_CS_PIN ) ); /* Set CS high (disabled) */

    /* Set GPIO_B[1:0] to 01 to put WLAN module into gSPI mode */
    MicoGpioInitialize( (mico_gpio_t)WL_GPIO0, OUTPUT_PUSH_PULL );
    MicoGpioOutputHigh( (mico_gpio_t)WL_GPIO0 );
    
    MicoGpioInitialize( (mico_gpio_t)WL_GPIO1, OUTPUT_PUSH_PULL );
    MicoGpioOutputLow( (mico_gpio_t)WL_GPIO1 );

    /* Setup DMA for SPIX RX */
    DMA_DeInit( SPIX_DMA_RX_STREAM );
    dma_init_structure.DMA_Channel = SPIX_DMA_RX_CHANNEL;
    dma_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &SPIX->DR;
    dma_init_structure.DMA_Memory0BaseAddr = 0;
    dma_init_structure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma_init_structure.DMA_BufferSize = 0;
    dma_init_structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init_structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init_structure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma_init_structure.DMA_Mode = DMA_Mode_Normal;
    dma_init_structure.DMA_Priority = DMA_Priority_VeryHigh;
    dma_init_structure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init_structure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    dma_init_structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init_structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init( SPIX_DMA_RX_STREAM, &dma_init_structure );

    /* Setup DMA for SPIX TX */
    DMA_DeInit( SPIX_DMA_TX_STREAM );
    dma_init_structure.DMA_Channel = SPIX_DMA_TX_CHANNEL;
    dma_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &SPIX->DR;
    dma_init_structure.DMA_Memory0BaseAddr = 0;
    dma_init_structure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma_init_structure.DMA_BufferSize = 0;
    dma_init_structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init_structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init_structure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma_init_structure.DMA_Mode = DMA_Mode_Normal;
    dma_init_structure.DMA_Priority = DMA_Priority_VeryHigh;
    dma_init_structure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init_structure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    dma_init_structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init_structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init( SPIX_DMA_TX_STREAM, &dma_init_structure );

    /* Must be lower priority than the value of configMAX_SYSCALL_INTERRUPT_PRIORITY */
    /* otherwise FreeRTOS will not be able to mask the interrupt */
    /* keep in mind that ARMCM3 interrupt priority logic is inverted, the highest value */
    /* is the lowest priority */
    nvic_init_structure.NVIC_IRQChannel                   = SPIX_DMA_RX_IRQ_CHANNEL;
    nvic_init_structure.NVIC_IRQChannelPreemptionPriority = (uint8_t) 0x3;
    nvic_init_structure.NVIC_IRQChannelSubPriority        = 0x0;
    nvic_init_structure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init( &nvic_init_structure );
    DMA_ITConfig(SPIX_DMA_RX_STREAM, DMA_IT_TC, ENABLE);

    /* Enable DMA for TX */
    SPI_I2S_DMACmd( SPIX, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE );

    /* Setup SPI */
    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode = SPI_Mode_Master;
    spi_init.SPI_DataSize = SPI_DataSize_8b;
    spi_init.SPI_CPOL = SPI_CPOL_High;
    spi_init.SPI_CPHA = SPI_CPHA_2Edge;
    spi_init.SPI_NSS = SPI_NSS_Soft;
    spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
    spi_init.SPI_CRCPolynomial = (uint16_t) 7;

    /* Init SPI and enable it */
    SPI_Init( SPIX, &spi_init );
    SPI_Cmd( SPIX, ENABLE );

    MCU_CLOCKS_NOT_NEEDED();

    return kNoErr;
}

OSStatus host_platform_bus_deinit( void )
{
    GPIO_InitTypeDef gpio_init_structure;

    MCU_CLOCKS_NEEDED();

    /* Disable SPI and SPI DMA */
    SPI_Cmd( SPIX, DISABLE );
    SPI_I2S_DeInit( SPIX );
    SPI_I2S_DMACmd( SPIX, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE );
    DMA_DeInit( SPIX_DMA_TX_STREAM );
    DMA_DeInit( SPIX_DMA_RX_STREAM );

    /* Clear GPIO_B[1:0] */
    MicoGpioFinalize( (mico_gpio_t)WL_GPIO0 );
    MicoGpioFinalize( (mico_gpio_t)WL_GPIO1 );

    /* Clear SPI slave select GPIOs */
    gpio_init_structure.GPIO_Mode = GPIO_Mode_AIN;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_structure.GPIO_Pin = ( 1 << SPI_BUS_CS_PIN );
    GPIO_Init( SPI_BUS_CS_BANK, &gpio_init_structure );

    /* Clear the SPI lines */
    gpio_init_structure.GPIO_Mode = GPIO_Mode_AIN;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_structure.GPIO_Pin = ( 1 << SPI_BUS_CLOCK_PIN ) | ( 1 << SPI_BUS_MISO_PIN ) | ( 1 << SPI_BUS_MOSI_PIN );
    GPIO_Init( SPI_BUS_CLOCK_BANK, &gpio_init_structure );

    gpio_irq_disable( SPI_IRQ_BANK, SPI_IRQ_PIN );

    /* Disable SPI_SLAVE Periph clock and DMA1 clock */
    RCC_APB1PeriphClockCmd( SPIX_CLK, DISABLE );
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA1, DISABLE );
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, DISABLE );

    MCU_CLOCKS_NOT_NEEDED();

    return kNoErr;
}

OSStatus host_platform_spi_transfer( bus_transfer_direction_t dir, uint8_t* buffer, uint16_t buffer_length )
{
    OSStatus result;
    uint32_t junk;

    MCU_CLOCKS_NEEDED();

    SPIX_DMA_TX_STREAM->NDTR = buffer_length;
    SPIX_DMA_TX_STREAM->M0AR = (uint32_t) buffer;
    if ( dir == BUS_READ )
    {
        SPIX_DMA_RX_STREAM->NDTR = buffer_length;
        SPIX_DMA_RX_STREAM->M0AR = (uint32_t) buffer;
        SPIX_DMA_RX_STREAM->CR |= DMA_MemoryInc_Enable  | ( 1 << 4);
    }
    else
    {
        SPIX_DMA_RX_STREAM->NDTR = buffer_length;
        SPIX_DMA_RX_STREAM->M0AR = (uint32_t) &junk;
        SPIX_DMA_RX_STREAM->CR &= ( ~DMA_MemoryInc_Enable ) | ( 1 << 4);
    }

    GPIO_ResetBits( SPI_BUS_CS_BANK, ( 1 << SPI_BUS_CS_PIN ) ); /* CS low (to select) */
    DMA_Cmd( SPIX_DMA_RX_STREAM, ENABLE );
    DMA_Cmd( SPIX_DMA_TX_STREAM, ENABLE );

    /* Wait for DMA TX to complete */
    result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, 100 );
//    loop_count = 0;
//    while ( ( DMA_GetFlagStatus( SPIX_DMA_RX_STREAM, DMA_FLAG_TCIF3 ) == RESET ) && ( loop_count < (uint32_t) DMA_TIMEOUT_LOOPS ) )
//    {
//        loop_count++;
//    }

    DMA_Cmd( SPIX_DMA_RX_STREAM, DISABLE );
    DMA_Cmd( SPIX_DMA_TX_STREAM, DISABLE );

    /* Clear the CS pin and the DMA status flag */
    GPIO_SetBits( SPI_BUS_CS_BANK, ( 1 << SPI_BUS_CS_PIN ) ); /* CS high (to deselect) */
    DMA_ClearFlag( SPIX_DMA_RX_STREAM, SPIX_DMA_RX_TCFLAG );
    DMA_ClearFlag( SPIX_DMA_TX_STREAM, SPIX_DMA_TX_TCFLAG );

    MCU_CLOCKS_NOT_NEEDED();

    return result;
}
