/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once
#include "MICOPlatform.h"

#include "board.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    SPI_MemMapPtr spi_peripheral;
    uint32_t      baud_rate_bps;
    uint8_t       chip_select;
    bool  polarity;
    bool  phase;
    bool  use_dma;

}spi_driver_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

OSStatus spi_init     ( spi_driver_t* spi_driver,
                                  SPI_MemMapPtr spi_peripheral,
                                  uint32_t baud_rate_bps,
                                  uint8_t chip_select,
                                  bool polarity,
                                  bool phase,
                                  bool use_dma );

OSStatus spi_deinit   ( spi_driver_t* spi_driver );

OSStatus spi_write    ( spi_driver_t* spi_driver, const uint8_t* data_out, uint32_t size );

OSStatus spi_read     ( spi_driver_t* spi_driver, uint8_t* data_in, uint32_t size );

OSStatus spi_transfer ( spi_driver_t* spi_driver, const uint8_t* data_out, uint8_t* data_in, uint32_t size );
