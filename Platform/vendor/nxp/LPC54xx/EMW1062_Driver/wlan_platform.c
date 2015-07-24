/**
******************************************************************************
* @file    wlan_platform.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide functions called by MICO to wlan RF module
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

#include <stdint.h>
#include "platform.h"
#include "platform_common_config.h"
#include "MICOPlatform.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef __GNUC__
#define TRIGGER_BREAKPOINT() __asm__("bkpt")
#elif defined ( __IAR_SYSTEMS_ICC__ )
#define TRIGGER_BREAKPOINT() __asm("bkpt 0")
#endif

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

/******************************************************
 *               Function Declarations
 ******************************************************/

static OSStatus platform_reset_wlan_powersave_clock( void );

extern void host_platform_reset_wifi( bool reset_asserted );

extern void host_platform_power_wifi( bool power_enabled );

/******************************************************
 *               Variables Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

OSStatus host_platform_init( void )
{
//    platform_reset_wlan_powersave_clock( );
//    
//    MicoGpioInitialize((mico_gpio_t)WL_RESET, OUTPUT_PUSH_PULL);
//    host_platform_reset_wifi( true ); /* Start wifi chip in reset */
//    
//    MicoGpioInitialize((mico_gpio_t)WL_REG, OUTPUT_PUSH_PULL);
//    host_platform_power_wifi( false ); /* Start wifi chip with regulators off */

    return kNoErr;
}

OSStatus host_platform_deinit( void )
{
//    MicoGpioInitialize((mico_gpio_t)WL_RESET, OUTPUT_PUSH_PULL);
//    host_platform_reset_wifi( true ); /* Stop wifi chip in reset */
//    
//    MicoGpioInitialize((mico_gpio_t)WL_REG, OUTPUT_PUSH_PULL);
//    host_platform_power_wifi( false ); /* Stop wifi chip with regulators off */
//
//    platform_reset_wlan_powersave_clock( );
//
    return kNoErr;
}

bool host_platform_is_in_interrupt_context( void )
{
//    /* From the ARM Cortex-M3 Techinical Reference Manual
//     * 0xE000ED04   ICSR    RW [a]  Privileged  0x00000000  Interrupt Control and State Register */
//    uint32_t active_interrupt_vector = (uint32_t)( SCB ->ICSR & 0x3fU );
//
//    if ( active_interrupt_vector != 0 )
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }
}


OSStatus host_platform_init_wlan_powersave_clock( void )
{
//#if ( MICO_WLAN_POWERSAVE_CLOCK_SOURCE == MICO_WLAN_POWERSAVE_CLOCK_IS_PWM )
//
//    MicoPwmInitialize( (mico_pwm_t) MICO_PWM_WLAN_POWERSAVE_CLOCK, WLAN_POWERSAVE_CLOCK_FREQUENCY, WLAN_POWERSAVE_CLOCK_DUTY_CYCLE );
//    MicoPwmStart( (mico_pwm_t) MICO_PWM_WLAN_POWERSAVE_CLOCK );
//    return WICED_SUCCESS;
//
//#elif ( MICO_WLAN_POWERSAVE_CLOCK_SOURCE == MICO_WLAN_POWERSAVE_CLOCK_IS_MCO )
//
//    GPIO_InitTypeDef gpio_init_structure;
//
//    /* Enable the GPIO peripherals related to the 32kHz clock pin */
//    RCC_AHB1PeriphClockCmd( WL_32K_OUT_BANK_CLK, ENABLE );
//
//    /* Setup the 32k clock pin to 0 */
//    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
//    gpio_init_structure.GPIO_Mode  = GPIO_Mode_AF;
//    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
//    gpio_init_structure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//
//    /* configure PA8 in alternate function mode */
//    gpio_init_structure.GPIO_Pin   = 1 << WL_32K_OUT_PIN;
//    GPIO_Init( WL_32K_OUT_BANK, &gpio_init_structure );
//    GPIO_PinAFConfig( WL_32K_OUT_BANK, WL_32K_OUT_PIN, GPIO_AF_MCO );
//
//    /* enable LSE output on MCO1 */
//    RCC_MCO1Config( RCC_MCO1Source_LSE, RCC_MCO1Div_1 );

    return kNoErr;

//#else
//
//    return platform_reset_wlan_powersave_clock( );
//
//#endif
}

OSStatus host_platform_deinit_wlan_powersave_clock( void )
{
//#if ( MICO_WLAN_POWERSAVE_CLOCK_SOURCE == MICO_WLAN_POWERSAVE_CLOCK_IS_PWM )
//
//    MicoPwmStop( (mico_pwm_t) MICO_PWM_WLAN_POWERSAVE_CLOCK );
//    platform_reset_wlan_powersave_clock( );
//    return kNoErr;
//
//#else
//
//    return platform_reset_wlan_powersave_clock( );
//
//#endif
  return kNoErr;
}

static OSStatus platform_reset_wlan_powersave_clock( void )
{   
//  /* Tie the pin to ground */
//  MicoGpioInitialize( (mico_gpio_t) MICO_GPIO_WLAN_POWERSAVE_CLOCK, OUTPUT_PUSH_PULL );
//  MicoGpioOutputLow( (mico_gpio_t) MICO_GPIO_WLAN_POWERSAVE_CLOCK );
  return kNoErr;
}

// end file 
