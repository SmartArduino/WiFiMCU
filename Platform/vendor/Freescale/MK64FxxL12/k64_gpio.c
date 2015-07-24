/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "k64_gpio.h"
#include "core_cm4.h"
#include "string.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define GPIO_PIN(x)   (((1)<<(x & 0x1Fu)))

/******************************************************
 *                    Constants
 ******************************************************/

#define GPIO_PORTS           (5)
#define GPIO_LINES_PER_PORT (32)

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
    gpio_irq_callback callback;
    void*                 arg;
}gpio_irq_data_t;

typedef struct
{
    uint32_t            enable_mask;               // If bit = 1, interrupt for that particular line is enabled
    gpio_irq_data_t data[GPIO_LINES_PER_PORT]; // Callback and arg for all lines
}gpio_port_irq_data_t;

/******************************************************
 *               Function Declarations
 ******************************************************/

void gpio_irq( void );

/******************************************************
 *               Variables Definitions
 ******************************************************/

static gpio_port_irq_data_t gpio_irq_data[GPIO_PORTS];
#if 0 /* currently unused */
static wiced_bool_t             gpio_initted = WICED_FALSE;
#endif

static const GPIO_MemMapPtr gpio_list[] =
{
    [0] = PTA_BASE_PTR,
    [1] = PTB_BASE_PTR,
    [2] = PTC_BASE_PTR,
    [3] = PTD_BASE_PTR,
    [4] = PTE_BASE_PTR
};

static const PORT_MemMapPtr gpio_port_list[] =
{
    [0] = PORTA_BASE_PTR,
    [1] = PORTB_BASE_PTR,
    [2] = PORTC_BASE_PTR,
    [3] = PORTD_BASE_PTR,
    [4] = PORTE_BASE_PTR
};

static const IRQn_Type gpio_irq_type_list[] =
{
    [0] = PORTA_IRQn,
    [1] = PORTB_IRQn,
    [2] = PORTC_IRQn,
    [3] = PORTD_IRQn,
    [4] = PORTE_IRQn
};

static const uint32_t gpio_clock_mask_list[] =
{
    [0] = SIM_SCGC5_PORTA_MASK,
    [1] = SIM_SCGC5_PORTB_MASK,
    [2] = SIM_SCGC5_PORTC_MASK,
    [3] = SIM_SCGC5_PORTD_MASK,
    [4] = SIM_SCGC5_PORTE_MASK
};

/******************************************************
 *               Function Definitions
 ******************************************************/

#if 0 /* not used */
OSStatus gpio_management_init( void )
{
    if ( gpio_initted == WICED_TRUE )
    {
        return kGeneralErr;
    }

    memset( &gpio_irq_data, 0, sizeof( gpio_irq_data ) );
    gpio_initted = WICED_TRUE;

    return kNoErr;
}
#endif

OSStatus gpio_init( gpio_t gpio, uint8_t pin, gpio_config_t config )
{
    GPIO_MemMapPtr gpio_peripheral      = gpio_list[gpio];
    PORT_MemMapPtr gpio_port_peripheral = gpio_port_list[gpio];

    /* Enable GPIO peripheral clock */
    SIM_SCGC5 |= gpio_clock_mask_list[gpio];

    if ( config == INPUT_PULL_UP || config == INPUT_PULL_DOWN || config == INPUT_HIGH_IMPEDANCE )
    {
        /* Set to input */
        GPIO_PDDR_REG( gpio_peripheral ) &= ~GPIO_PDDR_PDD ( GPIO_PIN( pin ) );

        if ( config == INPUT_PULL_DOWN )
        {
            /* Enable pull-up/down and select pull-up (PS = 1) */
            PORT_PCR_REG( gpio_port_peripheral , pin ) |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
        }
        else if ( config == INPUT_PULL_UP )
        {
            /* Enable pull-up/down and select pull-down (PS = 0) */
            PORT_PCR_REG( gpio_port_peripheral , pin ) |= PORT_PCR_PE_MASK;
            PORT_PCR_REG( gpio_port_peripheral , pin ) &= ~(uint32_t)PORT_PCR_PS_MASK;
        }
        else
        {
            /* Disable pull-up/down (floating) */
            PORT_PCR_REG( gpio_port_peripheral , pin ) &= ~(uint32_t)PORT_PCR_PE_MASK;
        }
    }
    else if ( config == OUTPUT_PUSH_PULL || config == OUTPUT_OPEN_DRAIN_NO_PULL || config == OUTPUT_OPEN_DRAIN_PULL_UP )
    {
        /* Set to output */
        GPIO_PDDR_REG( gpio_peripheral ) |= GPIO_PDDR_PDD ( GPIO_PIN( pin ) );

        if ( config == OUTPUT_OPEN_DRAIN_NO_PULL )
        {
            /* Enable open-drain and pull-up */
            PORT_PCR_REG( gpio_port_peripheral , pin ) |= PORT_PCR_ODE_MASK | PORT_PCR_PS_MASK;
        }
        else if ( config == OUTPUT_OPEN_DRAIN_NO_PULL )
        {
            /* Enable pull-up/down and select pull-down (PS = 0) */
            PORT_PCR_REG( gpio_port_peripheral , pin ) |= PORT_PCR_ODE_MASK;
            PORT_PCR_REG( gpio_port_peripheral , pin ) &= ~(uint32_t)PORT_PCR_PS_MASK;
        }
    }

    PORT_PCR_REG( gpio_port_peripheral, pin ) = PORT_PCR_MUX( 1 );
    return kNoErr;
}

OSStatus gpio_select_mux( gpio_t gpio, uint8_t pin, uint8_t pin_control_register_mux )
{
    /* Enable GPIO peripheral clock */
    SIM_SCGC5 |= gpio_clock_mask_list[gpio];

    PORT_PCR_REG( gpio_port_list[gpio], pin ) = PORT_PCR_MUX( pin_control_register_mux );
    return kNoErr;
}

OSStatus gpio_output_high( gpio_t gpio, uint8_t pin )
{
    GPIO_PDOR_REG( gpio_list[gpio] ) |= (uint32_t) GPIO_PIN( pin );
    return kNoErr;
}

OSStatus gpio_output_low( gpio_t gpio, uint8_t pin )
{
    GPIO_PDOR_REG( gpio_list[gpio] ) &= (uint32_t) ~GPIO_PIN( pin );
    return kNoErr;
}

bool gpio_input_get( gpio_t gpio, uint8_t pin )
{
    UNUSED_PARAMETER( gpio );
    UNUSED_PARAMETER( pin );
    return kNoErr;
}

OSStatus gpio_irq_enable( gpio_t gpio, uint8_t pin, gpio_irq_trigger_t trigger, gpio_irq_callback callback, void* arg )
{
    if ( ( gpio_irq_data[gpio].enable_mask & (uint32_t)( 1 << pin ) ) != 0 )
    {
        /* Interrupt is already enabled */
        return kGeneralErr;
    }

    gpio_irq_data[gpio].enable_mask       |= (uint32_t)( 1 << pin );
    gpio_irq_data[gpio].data[pin].callback = callback;
    gpio_irq_data[gpio].data[pin].arg      = arg;

    /* Enable interrupt at the source */
    PORT_PCR_REG( gpio_port_list[gpio], pin ) |= PORT_PCR_IRQC ( 0x8 | trigger ) ;

    /* Enable interrupt at the ARM core */
    NVIC_EnableIRQ( gpio_irq_type_list[gpio] );

    /* Set interrupt priority */
    NVIC_SetPriority( gpio_irq_type_list[gpio], 0xfe );

    return kNoErr;
}

OSStatus gpio_irq_disable( gpio_t gpio, uint8_t pin )
{
    if ( ( gpio_irq_data[gpio].enable_mask & (uint32_t)( 1 << pin ) ) == 0 )
    {
        /* Interrupt is already disabled */
        return kGeneralErr;
    }

    gpio_irq_data[gpio].enable_mask       &= ~(uint32_t)( 1 << pin );
    gpio_irq_data[gpio].data[pin].callback = 0;
    gpio_irq_data[gpio].data[pin].arg      = 0;

    /* Disable interrupt at the source */
    PORT_PCR_REG( gpio_port_list[gpio] , pin ) &= ~(uint32_t)PORT_PCR_IRQC ( 0 );

    /* All interrupt lines on this port is already disabled. Disable shared interrupt vector in ARM core */
    if ( gpio_irq_data[gpio].enable_mask == 0 )
    {
        NVIC_DisableIRQ( gpio_irq_type_list[gpio] );
    }

    return kNoErr;
}

/*!
 * @brief gpio IRQ handler with the same name in startup code
 */
void PORTA_IRQHandler(void)
{
    gpio_irq();
}
void PORTB_IRQHandler(void)
{
    gpio_irq();
}
void PORTC_IRQHandler(void)
{
    gpio_irq();
}
void PORTD_IRQHandler(void)
{
    gpio_irq();
}
void PORTE_IRQHandler(void)
{
    gpio_irq();
}




/******************************************************
 *                  ISR Definitions
 ******************************************************/

void gpio_irq( void )
{
    uint32_t   status;
    uint32_t   line;
    gpio_t gpio;

    for ( gpio = 0; gpio < GPIO_PORTS; gpio++ )
    {
        /* Get interrupt flags */
        status = PORT_ISFR_REG( gpio_port_list[gpio] );

        /* Clear interrupts */
        PORT_ISFR_REG( gpio_port_list[gpio] ) = status;

        if ( status != 0 )
        {
            /* Call the respective GPIO interrupt handler/callback */
            for ( line = 0; line < GPIO_LINES_PER_PORT; line++, status >>= 1 )
            {
                if ( ( status & 0x1 ) != 0 && ( gpio_irq_data[gpio].data[line].callback != NULL ) )
                {
                    gpio_irq_data[gpio].data[line].callback( gpio_irq_data[gpio].data[line].arg );
                }
            }
        }
    }
}
