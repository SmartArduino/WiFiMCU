/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "MicoRtos.h"
#include "string.h" /* For memcpy */

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

/******************************************************
 *             Function declarations
 ******************************************************/

#ifndef MICO_DISABLE_MCU_POWERSAVE
extern void wake_up_interrupt_notify( void );
#define MCU_NOTIFY_WAKE_UP()        wake_up_interrupt_notify()
#else
#define MCU_NOTIFY_WAKE_UP()
#endif /* ifndef MICO_DISABLE_MCU_POWERSAVE */

/* Powersave functionality */
extern void MCU_CLOCKS_NEEDED( void );
extern void MCU_CLOCKS_NOT_NEEDED( void );

extern void wiced_platform_notify_irq( void );

/******************************************************
 *             Function definitions
 ******************************************************/

void spi_irq_handler( )
{

#ifndef MICO_DISABLE_MCU_POWERSAVE
    wake_up_interrupt_notify( );
#endif /* ifndef MICO_DISABLE_MCU_POWERSAVE */

    wiced_platform_notify_irq( );
}

OSStatus host_platform_bus_init( void )
{

    MCU_CLOCKS_NEEDED();

    SPI_Init();

    MCU_CLOCKS_NOT_NEEDED();

    return kNoErr;
}

OSStatus host_platform_bus_deinit( void )
{
    MCU_CLOCKS_NEEDED();

    SPI_DeInit();

    MCU_CLOCKS_NOT_NEEDED();

    return kNoErr;
}

OSStatus host_platform_spi_transfer( bus_transfer_direction_t dir, uint8_t* buffer, uint16_t buffer_length )
{
    int i;
    
    MCU_CLOCKS_NEEDED();

    if ( dir == BUS_READ )
    {
        SPI_DATA_WRITEREAD(buffer, buffer, buffer_length, buffer_length);
    }
    else
    {
        SPI_DATA_WRITE(buffer, buffer_length);
    }


    MCU_CLOCKS_NOT_NEEDED();

    return kNoErr;
}
