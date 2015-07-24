/**
******************************************************************************
* @file    stm32f2xx_platform.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide functions called by MICO to drive stm32f2xx 
*          platform: - e.g. power save, reboot, platform initialize
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


#include "ksdk_platform.h"
#include "platform.h"
#include "platform_common_config.h"
#include "MicoPlatform.h"
#include "PlatformLogging.h"
#include <string.h> // For memcmp
#include "crt0.h"
#include "MICODefaults.h"
#include "MicoRTOS.h"
#include "board.h"


#ifdef __GNUC__
#include "../../GCC/stdio_newlib.h"
#endif /* ifdef __GNUC__ */

#ifdef __GNUC__
#define WEAK __attribute__ ((weak))
#elif defined ( __IAR_SYSTEMS_ICC__ )
#define WEAK __weak
#endif /* ifdef __GNUC__ */

/******************************************************
*                      Macros
******************************************************/
#ifndef BOOTLOADER_MAGIC_NUMBER
#define BOOTLOADER_MAGIC_NUMBER 0x4d435242
#endif

#define NUMBER_OF_LSE_TICKS_PER_MILLISECOND(scale_factor) ( 32768 / 1000 / scale_factor )
#define CONVERT_FROM_TICKS_TO_MS(n,s) ( n / NUMBER_OF_LSE_TICKS_PER_MILLISECOND(s) )

/******************************************************
*                    Constants
******************************************************/

#ifndef STDIO_BUFFER_SIZE
#define STDIO_BUFFER_SIZE   64
#endif

#define RTC_INTERRUPT_EXTI_LINE EXTI_Line22

#define CK_SPRE_CLOCK_SOURCE_SELECTED 0xFFFF

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

void MCU_CLOCKS_NEEDED    ( void );
void MCU_CLOCKS_NOT_NEEDED( void );

void wake_up_interrupt_notify( void );

extern OSStatus host_platform_init( void );

/******************************************************
*               Variables Definitions
******************************************************/

#ifndef MICO_DISABLE_STDIO
static const mico_uart_config_t stdio_uart_config =
{
  .baud_rate    = 115200,
  .data_width   = DATA_WIDTH_8BIT,
  .parity       = NO_PARITY,
  .stop_bits    = STOP_BITS_1,
  .flow_control = FLOW_CONTROL_DISABLED,
};

static volatile ring_buffer_t stdio_rx_buffer;
static volatile uint8_t             stdio_rx_data[STDIO_BUFFER_SIZE];
mico_mutex_t        stdio_rx_mutex;
mico_mutex_t        stdio_tx_mutex;
#endif /* #ifndef MICO_DISABLE_STDIO */

#ifndef MICO_DISABLE_MCU_POWERSAVE
static bool wake_up_interrupt_triggered  = false;
static unsigned long rtc_timeout_start_time           = 0;
#endif /* #ifndef MICO_DISABLE_MCU_POWERSAVE */

/******************************************************
*               Function Definitions
******************************************************/
#if defined ( __ICCARM__ )

static inline void __jump_to( uint32_t addr )
{
  __asm( "MOV R1, #0x00000001" );
  __asm( "ORR R0, R0, R1" );  /* Last bit of jump address indicates whether destination is Thumb or ARM code */
  __asm( "BLX R0" );
}

#elif defined ( __GNUC__ )

__attribute__( ( always_inline ) ) static __INLINE void __jump_to( uint32_t addr )
{
  addr |= 0x00000001;  /* Last bit of jump address indicates whether destination is Thumb or ARM code */
  __ASM volatile ("BX %0" : : "r" (addr) );
}

#endif

/* STM32F2 common clock initialisation function
* This brings up enough clocks to allow the processor to run quickly while initialising memory.
* Other platform specific clock init can be done in init_platform() or init_architecture()
*/
WEAK void init_clocks( void )
{
    hardware_init();
    
}

WEAK void init_memory( void )
{
  
}

void init_architecture( void )
{
 
#ifndef MICO_DISABLE_STDIO
#ifndef NO_MICO_RTOS
   mico_rtos_init_mutex( &stdio_tx_mutex );
   mico_rtos_unlock_mutex ( &stdio_tx_mutex );
   mico_rtos_init_mutex( &stdio_rx_mutex );
   mico_rtos_unlock_mutex ( &stdio_rx_mutex );
#endif
   dbg_uart_init(); 
  // MicoStdioUartInitialize(&stdio_uart_config,NULL);
#endif
}

/******************************************************
*            Interrupt Service Routines
******************************************************/


void MCU_CLOCKS_NEEDED( void )
{
  return;

}

void MCU_CLOCKS_NOT_NEEDED( void )
{

  return;

}


void wake_up_interrupt_notify( void )
{
  wake_up_interrupt_triggered = true;
}



unsigned long platform_power_down_hook( unsigned long delay_ms )
{
  __asm("wfi");
  return 0;

}

void platform_idle_hook( void )
{
  __asm("wfi");
}

void MicoSystemReboot(void)
{ 
  NVIC_SystemReset();
}

void MicoSystemStandBy(uint32_t secondsToWakeup)
{ 

}

void MicoMcuPowerSaveConfig( int enable )
{
  if (enable == 1)
    MCU_CLOCKS_NOT_NEEDED();
  else
    MCU_CLOCKS_NEEDED();
}



#ifdef NO_MICO_RTOS
static volatile uint32_t no_os_tick = 0;

void SysTick_Handler(void)
{
  no_os_tick ++;
}

uint32_t mico_get_time_no_os(void)
{
  return no_os_tick;
}

void mico_thread_msleep_no_os(uint32_t milliseconds)
{
  int tick_delay_start = mico_get_time_no_os();
  while(mico_get_time_no_os() < tick_delay_start+milliseconds);  
}
#endif

