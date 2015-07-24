/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "board.h"
#include "platform.h"
#include "k64_spi.h"
#include "k64_gpio.h"
#include "MicoRtos.h"
#include "string.h" /* For memcpy */
#include "board.h"

/******************************************************
 *             Constants
 ******************************************************/
/**
 * Transfer direction for the mico platform bus interface
 */
typedef enum
{
    /* If updating this enum, the bus_direction_mapping variable will also need to be updated */
    BUS_READ,
    BUS_WRITE
} bus_transfer_direction_t;

#define WLAN_SPI_BAUD_RATE_BPS (10000000)

/******************************************************
 *             Structures
 ******************************************************/

/******************************************************
 *             Variables
 ******************************************************/

static spi_driver_t spi_driver;

/******************************************************
 *             Function declarations
 ******************************************************/

static void spi_irq_handler( void* arg );

/******************************************************
 *             Function definitions
 ******************************************************/

OSStatus host_platform_bus_init( void )
{

    /* Configure WLAN GPIO0 pin and default to high for SPI mode */
    gpio_init( WL_GPIO1_BANK, WL_GPIO1_PIN, OUTPUT_PUSH_PULL );
    gpio_output_high( WL_GPIO1_BANK, WL_GPIO1_PIN );

    /* Configure Pin muxing for SPI CLK, MISO, MOSI */
    gpio_select_mux( SPI_BUS_CLOCK_BANK, SPI_BUS_CLOCK_PIN, 2 );
    gpio_select_mux( SPI_BUS_MISO_BANK,  SPI_BUS_MISO_PIN,  2 );
    gpio_select_mux( SPI_BUS_MOSI_BANK,  SPI_BUS_MOSI_PIN,  2 );

    /* Init SPI CS GPIO and default to high. Use GPIO to allow better control over CS line */
    gpio_init( SPI_BUS_CS_BANK, SPI_BUS_CS_PIN, OUTPUT_PUSH_PULL );
    gpio_output_high( SPI_BUS_CS_BANK, SPI_BUS_CS_PIN );

    /* Init SPI interrupt */
    gpio_init( SPI_IRQ_BANK, SPI_IRQ_PIN, INPUT_PULL_DOWN );
    gpio_irq_enable( SPI_IRQ_BANK, SPI_IRQ_PIN, IRQ_TRIGGER_RISING_EDGE, spi_irq_handler, 0 );

    /* Init SPI port. CPHA = 0 and CPOL = 0 */
   // return spi_init( &spi_driver, WL_SPI, WLAN_SPI_BAUD_RATE_BPS, 0, false, false, false );
    return spi_init( &spi_driver, WL_SPI, WLAN_SPI_BAUD_RATE_BPS, 0, false, false, true );
}

OSStatus host_platform_bus_deinit( void )
{

    /* Configure WLAN GPIO0 pin and default to high for SPI mode */
    gpio_output_high( WL_GPIO1_BANK, WL_GPIO1_PIN );

    /* Init SPI CS GPIO and default to high. Use GPIO to allow better control over CS line */
    gpio_output_high( SPI_BUS_CS_BANK, SPI_BUS_CS_PIN );

    /* Init SPI interrupt */
    gpio_irq_disable( SPI_IRQ_BANK, SPI_IRQ_PIN );
 
    return spi_deinit( &spi_driver );
}
static uint8_t dumy[2048];

OSStatus host_platform_spi_transfer( bus_transfer_direction_t dir, uint8_t* buffer, uint16_t buffer_length )
{
    OSStatus retval;
	int i;

    /* Assert chip select */
    gpio_output_low( SPI_BUS_CS_BANK, SPI_BUS_CS_PIN );

    if (BUS_WRITE == dir) 
      retval = spi_transfer( &spi_driver, buffer, dumy, buffer_length );
      else
    retval = spi_transfer( &spi_driver, buffer, buffer, buffer_length );
       
    /* Deassert chip select */ 
    gpio_output_high( SPI_BUS_CS_BANK, SPI_BUS_CS_PIN );

    return retval;
}

static void spi_irq_handler( void* arg )
{
    UNUSED_PARAMETER( arg );
    wiced_platform_notify_irq();
}
