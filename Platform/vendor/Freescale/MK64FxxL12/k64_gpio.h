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

typedef enum
{
    GPIO_INPUT,
    GPIO_OUTPUT,
}gpio_direction_t;

typedef enum
{
    GPIO_A,
    GPIO_B,
    GPIO_C,
    GPIO_D,
    GPIO_E
}gpio_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef void (*gpio_irq_callback)( void* );

typedef mico_gpio_irq_trigger_t gpio_irq_trigger_t;
typedef mico_gpio_irq_handler_t gpio_irq_handler_t;
typedef mico_gpio_config_t      gpio_config_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

OSStatus gpio_init        ( gpio_t gpio, uint8_t pin, gpio_config_t config );

OSStatus gpio_select_mux  ( gpio_t gpio, uint8_t pin, uint8_t pin_control_register_mux );

OSStatus gpio_output_high ( gpio_t gpio, uint8_t pin );

OSStatus gpio_output_low  ( gpio_t gpio, uint8_t pin );

bool   gpio_input_get   ( gpio_t gpio, uint8_t pin );

OSStatus gpio_irq_enable  ( gpio_t gpio, uint8_t pin, gpio_irq_trigger_t trigger, gpio_irq_callback callback, void* arg );

OSStatus gpio_irq_disable ( gpio_t gpio, uint8_t pin );
