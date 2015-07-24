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


#include "MICORTOS.h"
#include "platform.h"
#include "platform_peripheral.h"

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
  uint8_t                       port;
  uint8_t                       pin;
  platform_gpio_irq_callback_t  handler;    // User callback
  void*                         arg;        // User argument to be passed to the callbackA
} gpio_irq_data_t;

/******************************************************
*               Variables Definitions
******************************************************/

static volatile gpio_irq_data_t gpio_irq_data[ NUMBER_OF_GPIO_IRQ_LINES ];

/******************************************************
*               Function Declarations
******************************************************/

/******************************************************
*               Function Definitions
******************************************************/

OSStatus platform_gpio_init( const platform_gpio_t* gpio, platform_pin_config_t config )
{
  OSStatus          err = kNoErr;

  platform_mcu_powersave_disable();
  require_action_quiet( gpio != NULL, exit, err = kParamErr);

  GpioClrRegOneBit(GPIO_A_OUTDS + gpio->port , ((uint32_t)1 << gpio->pin) );

  if ( (config == INPUT_PULL_UP ) || (config == INPUT_HIGH_IMPEDANCE ) || (config == INPUT_PULL_DOWN )){
    GpioClrRegOneBit(GPIO_A_OE + gpio->port , ((uint32_t)1 << gpio->pin) );
    GpioSetRegOneBit(GPIO_A_IE + gpio->port , ((uint32_t)1 << gpio->pin) );
  }else{
    GpioClrRegOneBit(GPIO_A_IE + gpio->port , ((uint32_t)1 << gpio->pin) );
    GpioSetRegOneBit(GPIO_A_OE + gpio->port , ((uint32_t)1 << gpio->pin) );    
  }
  
  if ( (config == INPUT_PULL_UP ) || (config == OUTPUT_OPEN_DRAIN_PULL_UP ) )
  {
    GpioClrRegOneBit(GPIO_A_PU + gpio->port , ((uint32_t)1 << gpio->pin) );
    GpioClrRegOneBit(GPIO_A_PD + gpio->port , ((uint32_t)1 << gpio->pin) );
  }
  else if (config == INPUT_PULL_DOWN )
  {
    GpioSetRegOneBit(GPIO_A_PU + gpio->port , ((uint32_t)1 << gpio->pin) );
    GpioSetRegOneBit(GPIO_A_PD + gpio->port , ((uint32_t)1 << gpio->pin) );
  }
  else
  {
    GpioSetRegOneBit(GPIO_A_PU + gpio->port , ((uint32_t)1 << gpio->pin) );
    GpioClrRegOneBit(GPIO_A_PD + gpio->port , ((uint32_t)1 << gpio->pin) );
  }
  
  if(config == OUTPUT_PUSH_PULL)
    GpioSetRegOneBit(GPIO_A_OUTDS + gpio->port , ((uint32_t)1 << gpio->pin) );
  
exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_gpio_deinit( const platform_gpio_t* gpio )
{ 
  OSStatus          err = kNoErr;

  platform_mcu_powersave_disable();
  require_action_quiet( gpio != NULL, exit, err = kParamErr);
  
  GpioClrRegOneBit(GPIO_A_OE + gpio->port , ((uint32_t)1 << gpio->pin) );
  GpioSetRegOneBit(GPIO_A_IE + gpio->port , ((uint32_t)1 << gpio->pin) );
  
exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_gpio_output_high( const platform_gpio_t* gpio )
{
  OSStatus err = kNoErr;

  platform_mcu_powersave_disable();

  require_action_quiet( gpio != NULL, exit, err = kParamErr);

  GpioSetRegOneBit(GPIO_A_OUT + gpio->port , ((uint32_t)1 << gpio->pin) );
  
exit:
  platform_mcu_powersave_enable();
  return err;  
}

OSStatus platform_gpio_output_low( const platform_gpio_t* gpio )
{
  OSStatus err = kNoErr;

  require_action_quiet( gpio != NULL, exit, err = kParamErr);

  platform_mcu_powersave_disable();
  
  GpioClrRegOneBit(GPIO_A_OUT + gpio->port , ((uint32_t)1 << gpio->pin) );
  
exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_gpio_output_trigger( const platform_gpio_t* gpio )
{
  OSStatus err = kNoErr;
  uint32_t regValue;
  uint32_t mask = (uint32_t)1 << gpio->pin;

  platform_mcu_powersave_disable();

  require_action_quiet( gpio != NULL, exit, err = kParamErr);

  regValue = GpioGetReg( GPIO_A_OUT + gpio->port );

  (regValue&mask)? platform_gpio_output_low( gpio ) : platform_gpio_output_high( gpio );
  
exit:
  platform_mcu_powersave_enable();
  return err;  
}

bool platform_gpio_input_get( const platform_gpio_t* gpio )
{
  bool     result = false;
  uint32_t regValue;
  uint32_t mask = (uint32_t)1 << gpio->pin;

  platform_mcu_powersave_disable();

  require_quiet( gpio != NULL, exit);

  regValue = GpioGetReg( GPIO_A_IN + gpio->port );

  result =  ((regValue&mask)  == 0 )? false : true;
  
exit:
  platform_mcu_powersave_enable();
  return result;  
}

OSStatus platform_gpio_irq_enable( const platform_gpio_t* gpio, platform_gpio_irq_trigger_t trigger, platform_gpio_irq_callback_t handler, void* arg )
{
  uint8_t intPort;
  int i;
  OSStatus err = kNoErr;

  platform_mcu_powersave_disable();
  require_action_quiet( gpio != NULL && trigger != IRQ_TRIGGER_BOTH_EDGES, exit, err = kParamErr);
  
  switch( gpio->port ){
    case GPIOA:
      intPort = GPIO_A_INT;
      break;
    case GPIOB:
      intPort = GPIO_B_INT;
      break;
    case GPIOC:
      intPort = GPIO_C_INT;
      break;
    default:
      err = kParamErr;
      goto exit;
  }
    
  for( i = 0; i < NUMBER_OF_GPIO_IRQ_LINES; i++ ){
    if ( gpio_irq_data[i].port ==  gpio->port && gpio_irq_data[i].pin ==  gpio->pin){
      /* GPIO IRQ already exist */
      gpio_irq_data[ i ].handler    = handler;
      gpio_irq_data[ i ].arg        = arg;
      break;
    }
  }

  if(i == NUMBER_OF_GPIO_IRQ_LINES){
    /* GPIO IRQ not exist */
    for( i = 0; i < NUMBER_OF_GPIO_IRQ_LINES; i++ ){
      if ( gpio_irq_data[i].handler == NULL ){
        gpio_irq_data[ i ].port       = gpio->port;
        gpio_irq_data[ i ].pin        = gpio->pin;
        gpio_irq_data[ i ].handler    = handler;
        gpio_irq_data[ i ].arg        = arg;
        break;
      }
    } 
    /* No space to add one */
    if( i == NUMBER_OF_GPIO_IRQ_LINES)
      return kNoSpaceErr;
  }

  GpioIntClr(intPort, ((uint32_t)1 << gpio->pin));

  if( trigger == IRQ_TRIGGER_RISING_EDGE )
    GpioIntEn(intPort, ((uint32_t)1 << gpio->pin), GPIO_POS_EDGE_TRIGGER);
  else 
    GpioIntEn(intPort, ((uint32_t)1 << gpio->pin), GPIO_NEG_EDGE_TRIGGER);

exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_gpio_irq_disable( const platform_gpio_t* gpio )
{
  uint8_t intPort;
  int i;
  OSStatus err = kNoErr;

  platform_mcu_powersave_disable();
  require_action_quiet( gpio != NULL, exit, err = kParamErr);

  switch( gpio->port ){
  case GPIOA:
    intPort = GPIO_A_INT;
    break;
  case GPIOB:
    intPort = GPIO_B_INT;
    break;
  case GPIOC:
    intPort = GPIO_C_INT;
    break;
  default:
    err = kParamErr;
    goto exit;
  }

  GpioIntDis( intPort, ((uint32_t)1 << gpio->pin) );

  for( i = 0; i < NUMBER_OF_GPIO_IRQ_LINES; i++ ){
    if ( gpio_irq_data[i].port ==  gpio->port && gpio_irq_data[i].pin ==  gpio->pin){
      gpio_irq_data[ i ].port       = 0;
      gpio_irq_data[ i ].pin        = 0;
      gpio_irq_data[ i ].handler    = NULL;
      gpio_irq_data[ i ].arg        = NULL;
      break;
    }
  }

  if( i == NUMBER_OF_GPIO_IRQ_LINES){
    err = kNotFoundErr;
    goto exit;
  }
  
exit:
  platform_mcu_powersave_enable();
  return err;
}

/******************************************************
 *               IRQ Handler Definitions
 ******************************************************/
MICO_RTOS_DEFINE_ISR( GpioInterrupt )
{
  uint32_t intFlagA, intFlagB, intFlagC;
  int i;
  uint8_t port, pin;
  void * arg;

  intFlagA = GpioIntFlagGet( GPIO_A_INT );
  intFlagB = GpioIntFlagGet( GPIO_B_INT );
  intFlagC = GpioIntFlagGet( GPIO_C_INT );

  for( i = 0; i < NUMBER_OF_GPIO_IRQ_LINES; i++ ){
    if ( gpio_irq_data[i].handler != NULL ){
      port = gpio_irq_data[i].port;
      pin = gpio_irq_data[i].pin;

      switch( port ){
      case GPIOA:
        if( intFlagA & ((uint32_t)1<<pin) ){
          GpioIntClr(GPIO_A_INT, ((uint32_t)1<<pin));
          arg = gpio_irq_data[i].arg; 
          gpio_irq_data[i].handler( arg );
         
        }
        break;
      case GPIOB:
        if( intFlagB & ((uint32_t)1<<pin) ){
          GpioIntClr(GPIO_B_INT, ((uint32_t)1<<pin));
          arg = gpio_irq_data[i].arg; 
          gpio_irq_data[i].handler( arg );
          
        }
        break;
      case GPIOC:
        if( intFlagC & ((uint32_t)1<<pin) ){
          GpioIntClr(GPIO_C_INT, ((uint32_t)1<<pin));
          arg = gpio_irq_data[i].arg; 
          gpio_irq_data[i].handler( arg );
          
        }
        break;
      default:
        continue;
      }
    }
  }
}

/******************************************************
 *      MX1101 Internal Function Definitions
 ******************************************************/
OSStatus platform_gpio_irq_manager_init( void )
{
  memset( (void*)gpio_irq_data, 0, sizeof( gpio_irq_data ) );
  NVIC_EnableIRQ(GPIO_IRQn);

  return kNoErr;
}








