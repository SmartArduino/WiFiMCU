/**
******************************************************************************
* @file    platform.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provides all MICO Peripherals mapping table and platform
*          specific funcgtions.
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

#include "stdio.h"
#include "string.h"

#include "MICOPlatform.h"
#include "platform.h"

#include "platform_common_config.h"
#include "PlatformLogging.h"
#include "ksdk_platform.h"

#include "board.h"

/******************************************************
*                      Macros
******************************************************/


/******************************************************
*                    Constants
******************************************************/
const uint32_t  mico_cpu_clock_hz = 96000000; // CPU CLock is 120MHz

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/

/******************************************************
*                    Structures
******************************************************/
//PORTA, PORTB, PORTC, PORTD, PORTE
const platform_pin_mapping_t gpio_mapping[] =
{
  /* Common GPIOs for internal use */
  [MICO_GPIO_WLAN_POWERSAVE_CLOCK]    = {GPIO_PINS_OUT_OF_RANGE},
  [WL_GPIO0]                          = {GPIO_MAKE_PIN(HW_GPIOE, 24)},
  [WL_GPIO1]                          = {GPIO_PINS_OUT_OF_RANGE},
  [WL_REG_RESERVED]                   = {GPIO_PINS_OUT_OF_RANGE},
  [WL_RESET]                          = {GPIO_MAKE_PIN(HW_GPIOC,  16)},
  [MICO_SYS_LED]                      = {kGpioLED1}, 
  [MICO_RF_LED]                       = {kGpioLED2},
  [BOOT_SEL]                          = {GPIO_PINS_OUT_OF_RANGE}, 
  [MFG_SEL]                           = {GPIO_PINS_OUT_OF_RANGE}, 
  [EasyLink_BUTTON]                   = {GPIO_MAKE_PIN(HW_GPIOC,  6)}, 

};

/******************************************************
*               Function Definitions
******************************************************/

static uint32_t _default_start_time = 0;
static mico_timer_t _button_EL_timer;

static void _button_EL_irq_handler( void* arg )
{
  (void)(arg);
  int interval = -1;
  
  if ( MicoGpioInputGet( (mico_gpio_t)EasyLink_BUTTON ) == 0 ) {
    _default_start_time = mico_get_time()+1;
    mico_start_timer(&_button_EL_timer);
  } else {
    interval = mico_get_time() + 1 - _default_start_time;
    if ( (_default_start_time != 0) && interval > 50 && interval < RestoreDefault_TimeOut){
      /* EasyLink button clicked once */
      PlatformEasyLinkButtonClickedCallback();
    }
    mico_stop_timer(&_button_EL_timer);
    _default_start_time = 0;
  }
}
static void _button_EL_Timeout_handler( void* arg )
{
  (void)(arg);
  _default_start_time = 0;
  PlatformEasyLinkButtonLongPressedCallback();
}

OSStatus mico_platform_init( void )
{
  platform_log( "Platform initialised" );
  
  
  //  Initialise EasyLink buttons
  MicoGpioInitialize( (mico_gpio_t)EasyLink_BUTTON, INPUT_PULL_UP );
  mico_init_timer(&_button_EL_timer, RestoreDefault_TimeOut, _button_EL_Timeout_handler, NULL);
  MicoGpioEnableIRQ( (mico_gpio_t)EasyLink_BUTTON, IRQ_TRIGGER_BOTH_EDGES, _button_EL_irq_handler, NULL );
  return kNoErr;
}

void init_platform( void )
{
    PORT_HAL_SetMuxMode(PORTC_BASE,3u,kPortMuxAsGpio);
    PORT_HAL_SetMuxMode(PORTC_BASE,4u,kPortMuxAsGpio);
    GPIO_DRV_OutputPinInit(&ledPins[0]);
    GPIO_DRV_OutputPinInit(&ledPins[1]);
    OSA_Init();

}

void init_platform_bootloader( void )
{
  
}

void host_platform_reset_wifi( bool reset_asserted )
{
  if ( reset_asserted == true )
  {
    MicoGpioOutputLow( (mico_gpio_t)WL_RESET );  
  }
  else
  {
    MicoGpioOutputHigh( (mico_gpio_t)WL_RESET ); 
  }
}

void host_platform_power_wifi( bool power_enabled )
{
  if ( power_enabled == true )
  {
    MicoGpioOutputLow( (mico_gpio_t)WL_REG );  
  }
  else
  {
    MicoGpioOutputHigh( (mico_gpio_t)WL_REG ); 
  }
}

void MicoSysLed(bool onoff)
{
    if (onoff) {
        GPIO_DRV_ClearPinOutput(kGpioLED1);
    } else {
        GPIO_DRV_SetPinOutput(kGpioLED1);
    }
}

void MicoRfLed(bool onoff)
{
    if (onoff) {
        GPIO_DRV_ClearPinOutput(kGpioLED2);
    } else {
        GPIO_DRV_SetPinOutput(kGpioLED2);
    }
}

bool MicoShouldEnterMFGMode(void)
{
  if(MicoGpioInputGet((mico_gpio_t)BOOT_SEL)==false && MicoGpioInputGet((mico_gpio_t)MFG_SEL)==false)
    return true;
  else
    return false;
}
