/**
******************************************************************************
* @file    platform_gpio.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide GPIO driver functions.
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


#include "MICOPlatform.h"
#include "MICORTOS.h"

#include "platform.h"
#include "platform_peripheral.h"
#include "platformLogging.h"

/******************************************************
*                    Constants
******************************************************/

#define PINS_PER_PORT (32) /* Px00 to Px31 */
#define TOTAL_PORTS   ( 2) /* PIOA to B    */

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/

/******************************************************
*                    Structures
******************************************************/

/* Structure of runtime GPIO IRQ data */

#pragma pack(1)
typedef struct
{
    bool                          wakeup_pin;
    platform_gpio_irq_callback_t  callback;
    void*                         arg;
} samg5x_gpio_irq_data_t;
#pragma pack()


/******************************************************
*               Variables Definitions
******************************************************/

/* Runtime GPIO IRQ data */
static samg5x_gpio_irq_data_t gpio_irq_data[TOTAL_PORTS][PINS_PER_PORT];

///* GPIO IRQ interrupt vectors */
static const IRQn_Type irq_vectors[] =
{
    [0] = PIOA_IRQn,
    [1] = PIOB_IRQn,
};

/******************************************************
*               Function Declarations
******************************************************/

/******************************************************
*               Function Definitions
******************************************************/

OSStatus platform_gpio_init( const platform_gpio_t* gpio, platform_pin_config_t config )
{
  ioport_mode_t         mode; 
  enum ioport_direction direction;  
  OSStatus              err = kNoErr;
  
  platform_mcu_powersave_disable();
  require_action_quiet( gpio != NULL, exit, err = kParamErr);
  
  switch ( config )
  {
    case INPUT_PULL_UP:
    {
      direction = IOPORT_DIR_INPUT;
      mode      = IOPORT_MODE_PULLUP;
      break;
    }
    case INPUT_PULL_DOWN:
    {
      direction = IOPORT_DIR_INPUT;
      mode      = IOPORT_MODE_PULLDOWN;
      break;
    }
    case INPUT_HIGH_IMPEDANCE:
    {
      direction = IOPORT_DIR_INPUT;
      mode      = 0;
      break;
    }
    case OUTPUT_PUSH_PULL:
    {
      direction = IOPORT_DIR_OUTPUT;
      mode      = 0;
      break;
    }
    case OUTPUT_OPEN_DRAIN_NO_PULL:
    {
      direction = IOPORT_DIR_OUTPUT;
      mode      = IOPORT_MODE_OPEN_DRAIN;
      break;
    }
    case OUTPUT_OPEN_DRAIN_PULL_UP:
    {
      direction = IOPORT_DIR_OUTPUT;
      mode      = IOPORT_MODE_OPEN_DRAIN | IOPORT_MODE_PULLUP;
      break;
    }
    default:
    {
      err = kParamErr;
      goto exit;
    }
  }
  ioport_enable_pin  ( gpio->pin );
  ioport_set_pin_mode( gpio->pin, mode );
  ioport_set_pin_dir ( gpio->pin, direction );
  
exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_gpio_peripheral_pin_init( const platform_gpio_t* gpio, ioport_mode_t pin_mode )
{
  OSStatus          err = kNoErr;

  platform_mcu_powersave_disable( );
  require_action_quiet( gpio != NULL, exit, err = kParamErr);

  /* Set pin mode and disable GPIO peripheral */
  ioport_set_pin_mode( gpio->pin, pin_mode );
  ioport_disable_pin( gpio->pin );

exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_gpio_deinit( const platform_gpio_t* gpio )
{
  OSStatus          err = kNoErr;

  platform_mcu_powersave_disable();
  require_action_quiet( gpio != NULL, exit, err = kParamErr);
  
  ioport_disable_pin( gpio->pin );
    
exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_gpio_output_high( const platform_gpio_t* gpio )
{
  OSStatus err = kNoErr;
  
  require_action_quiet( gpio != NULL, exit, err = kParamErr);
  platform_mcu_powersave_disable();
  
  ioport_set_pin_level( gpio->pin, IOPORT_PIN_LEVEL_HIGH );
  
exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_gpio_output_low( const platform_gpio_t* gpio )
{
  OSStatus err = kNoErr;
  
  require_action_quiet( gpio != NULL, exit, err = kParamErr);
  platform_mcu_powersave_disable();
  
  ioport_set_pin_level( gpio->pin, IOPORT_PIN_LEVEL_LOW );
  
exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_gpio_output_trigger( const platform_gpio_t* gpio )
{
  OSStatus err = kNoErr;
  
  require_action_quiet( gpio != NULL, exit, err = kParamErr);
  platform_mcu_powersave_disable();
  
  ioport_toggle_pin_level( gpio->pin );
  
exit:
  platform_mcu_powersave_enable();
  return err;
}

bool platform_gpio_input_get( const platform_gpio_t* gpio )
{
  bool              result = false;
  
  platform_mcu_powersave_disable();
  
  require_quiet( gpio != NULL, exit);
  
  result = ( ioport_get_pin_level( gpio->pin ) == false ) ? false : true;
  
exit:
  platform_mcu_powersave_enable();
  return result;
}

OSStatus platform_gpio_irq_enable( const platform_gpio_t* gpio, platform_gpio_irq_trigger_t trigger, platform_gpio_irq_callback_t handler, void* arg )
{
  ioport_port_mask_t  mask                = ioport_pin_to_mask( gpio->pin );
  ioport_port_t       port                = ioport_pin_to_port_id( gpio->pin );
  volatile Pio*       port_register       = arch_ioport_port_to_base( port );
  uint8_t             pin_number          = (gpio->pin) & 0x1F;
  uint32_t            temp;
  uint8_t             samg5x_irq_trigger;
  OSStatus            err                 = kNoErr;
    
  platform_mcu_powersave_disable();
  require_action_quiet( gpio != NULL, exit, err = kParamErr);

  NVIC_DisableIRQ( irq_vectors[port] );
  NVIC_ClearPendingIRQ( irq_vectors[port] );

  gpio_irq_data[port][pin_number].wakeup_pin = gpio->is_wakeup_pin;
  gpio_irq_data[port][pin_number].arg        = arg;
  gpio_irq_data[port][pin_number].callback   = handler;

  switch ( trigger )
  {
    case IRQ_TRIGGER_RISING_EDGE:  samg5x_irq_trigger = IOPORT_SENSE_RISING;    break;
    case IRQ_TRIGGER_FALLING_EDGE: samg5x_irq_trigger = IOPORT_SENSE_FALLING;   break;
    case IRQ_TRIGGER_BOTH_EDGES:   samg5x_irq_trigger = IOPORT_SENSE_BOTHEDGES; break;
    default:
      err = kParamErr;
      goto exit;
  }

  if( gpio->is_wakeup_pin == true )
  {
    platform_powersave_enable_wakeup_pin( gpio );
  }

  if ( samg5x_irq_trigger == IOPORT_SENSE_RISING || samg5x_irq_trigger == IOPORT_SENSE_BOTHEDGES )
  {
   port_register->PIO_AIMER  |= mask;
   port_register->PIO_ESR    |= mask;
   port_register->PIO_REHLSR |= mask;
  }

  if ( samg5x_irq_trigger == IOPORT_SENSE_FALLING || samg5x_irq_trigger == IOPORT_SENSE_BOTHEDGES )
  {
    port_register->PIO_AIMER  |= mask;
    port_register->PIO_ESR    |= mask;
    port_register->PIO_FELLSR |= mask;
  }

  /* Read ISR to clear residual interrupt status */
  temp = port_register->PIO_ISR;
  UNUSED_PARAMETER( temp );

  /* Enable interrupt source */
  port_register->PIO_IER |= mask;

  NVIC_EnableIRQ( irq_vectors[port] );
  
exit:
  platform_mcu_powersave_enable();
  return err;
}


OSStatus platform_gpio_irq_disable( const platform_gpio_t* gpio )
{
  OSStatus            err           = kNoErr;
  ioport_port_mask_t  mask          = ioport_pin_to_mask   ( gpio->pin );
  ioport_port_t       port          = ioport_pin_to_port_id( gpio->pin );
  volatile Pio*       port_register = arch_ioport_port_to_base( port );
  
  platform_mcu_powersave_disable();
  require_action_quiet( gpio != NULL, exit, err = kParamErr);

  /* Disable interrupt on pin */
  port_register->PIO_IDR = mask;

  /* Disable Cortex-M interrupt vector as well if no pin interrupt is enabled */
  if ( port_register->PIO_IMR == 0 )
  {
    NVIC_DisableIRQ( irq_vectors[port] );
  }

  gpio_irq_data[port][mask].wakeup_pin = false;
  gpio_irq_data[port][mask].arg        = 0;
  gpio_irq_data[port][mask].callback   = NULL;
  
exit:
  platform_mcu_powersave_enable();
  return err;
}


/******************************************************
*      STM32F2xx Internal Function Definitions
******************************************************/
OSStatus platform_gpio_irq_manager_init( void )
{
  memset( &gpio_irq_data, 0, sizeof( gpio_irq_data ) );
  
  return kNoErr;
}

/******************************************************
*               IRQ Handler Definitions
******************************************************/


void gpio_irq( ioport_port_t  port )
{
  volatile Pio* port_register = arch_ioport_port_to_base( port );
  uint32_t      status        = port_register->PIO_ISR; /* Get interrupt status. Read clears the interrupt */
  uint32_t      mask          = port_register->PIO_IMR;
  uint32_t      iter          = 0;

  if ( ( status != 0 ) && ( mask != 0 ) )
  {
    /* Call the respective GPIO interrupt handler/callback */
    for ( iter = 0; iter < PINS_PER_PORT; iter++, status >>= 1, mask >>= 1 )
    {
          if ( ( ( mask & 0x1 ) != 0 ) && ( ( status & 0x1 ) != 0 ) && ( gpio_irq_data[port][iter].callback != NULL ) )
          {
              if ( gpio_irq_data[port][iter].wakeup_pin == true )
              {
                  platform_mcu_powersave_exit_notify();
              }
              gpio_irq_data[port][iter].callback( gpio_irq_data[port][iter].arg );
          }
      }
  }
}

/******************************************************
*               IRQ Handler Mapping
******************************************************/
MICO_RTOS_DEFINE_ISR( PIOA_Handler )
{
  gpio_irq( (ioport_port_t)0 );
}

MICO_RTOS_DEFINE_ISR( PIOB_Handler )
{
  gpio_irq( (ioport_port_t)1 );
}

MICO_RTOS_DEFINE_ISR( PIOC_Handler )
{
  gpio_irq( (ioport_port_t)2 );
}

