/***
 *file: platform_api.c
 *implement PlatformInternal.h and MicoPlatform.h's APIs about system or platform.
 *include init_clocks, init_memory, init_architecture, MicoSystemStandBy, MicoSystemReboot, MicoSystemStandBy,etc.
 *
 *created by Jerry Yu,@2014-DEC-05
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
 * */




#include "MicoPlatform.h" //drivers' APIs 
//#include "crt0.h"
#include "PlatformInternal.h"
#include "MicoRTOS.h"

/******************************************************
*                    Constants
******************************************************/

#ifndef STDIO_BUFFER_SIZE
#define STDIO_BUFFER_SIZE   64
#endif

/******************************************************
*               Function Declarations
******************************************************/
void wake_up_interrupt_notify( void );
/******************************************************
*               Variables Definitions
******************************************************/
static char platform_inited = 0;

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
/********************
 *      functions
 * ******************/
/**
* This brings up enough clocks to allow the processor to run quickly while initialising memory.
* Other platform specific clock init can be done in init_platform() or init_architecture()
*/
WEAK void init_clocks( void ){
/** This brings up enough clocks to allow the processor to run quickly while initialising memory.
* Other platform specific clock init can be done in init_platform() or init_architecture() */
//LPC54xx clock initialized in SystemInit().
#ifdef BOOTLOADER

  LPC_SYSCTL->SYSAHBCLKCTRL[0] |= 0x00000018; // Magicoe
#if defined(__FPU_PRESENT) && __FPU_PRESENT == 1
	fpuInit();
#endif

#if defined(NO_BOARD_LIB)
	/* Chip specific SystemInit */
	Chip_SystemInit();
#else
	/* Enable RAM 2 clock */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SRAM2);
	/* Board specific SystemInit */
	Board_SystemInit(); //init pin muxing and clock.
#endif
      LPC_SYSCTL->SYSAHBCLKCTRL[0] |= 0x00000018; // Magicoe
      
      Chip_SYSCTL_PowerUp(PDRUNCFG_PD_IRC_OSC_EN|PDRUNCFG_PD_IRC_EN);
      /* Configure PIN0.21 as CLKOUT with pull-up, monitor the MAINCLK on scope */
      Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 21, IOCON_MODE_PULLUP | IOCON_FUNC1 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF);
      Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_RTC, 1);
      Chip_Clock_EnableRTCOsc();
      Chip_RTC_Init(LPC_RTC);
      Chip_RTC_Enable1KHZ(LPC_RTC);
      Chip_RTC_Enable(LPC_RTC);
#endif
}

WEAK void init_memory( void ){
/**WEAK, not necessary in every platform.*/
}

void init_architecture( void){

  /*wakeup by watchdog in standby mode, re-enter standby mode in this situation*/

  /**/
  if(platform_inited == 1) return;
    
#ifndef MICO_DISABLE_STDIO
#ifndef NO_MICO_RTOS
  mico_rtos_init_mutex( &stdio_tx_mutex );
  mico_rtos_unlock_mutex ( &stdio_tx_mutex );
  mico_rtos_init_mutex( &stdio_rx_mutex );
  mico_rtos_unlock_mutex ( &stdio_rx_mutex );
#endif
  ring_buffer_init  ( (ring_buffer_t*)&stdio_rx_buffer, (uint8_t*)stdio_rx_data, STDIO_BUFFER_SIZE );
  MicoStdioUartInitialize( &stdio_uart_config, (ring_buffer_t*)&stdio_rx_buffer );
#endif

  SystemCoreClockUpdate();//cp from set()
  platform_inited = 1;


}
/*Boot to mico application form APPLICATION_START_ADDRESS defined in platform.h */
void startApplication(){

}

void MCU_CLOCKS_NEEDED( void )
{
  
  // 有外设操作的低功耗模式
  return;

}

void MCU_CLOCKS_NOT_NEEDED( void )
{
// 进入低功耗
  return;
}

void wake_up_interrupt_notify( void )
{
  wake_up_interrupt_triggered = true;
}

unsigned long platform_power_down_hook( unsigned long delay_ms )
{
  UNUSED_PARAMETER( delay_ms );
  ENABLE_INTERRUPTS;
  __asm("wfi");
  return 0; // return 真实的睡眠时间
}
/**====referenced in mico===*/
void platform_idle_hook( void )
{
  ENABLE_INTERRUPTS;
  __asm("wfi");
}

/** @brief    Software reboot the MICO hardware
  */
void MicoSystemReboot(void){

  NVIC_SystemReset(); //in iar cmsis core_cm4 not cm3
}

/** @brief    Software reboot the MICO hardware
  */
//void MicoSystemStandBy(void){
void MicoSystemStandBy(uint32_t secondsToWakeup){
  __asm("wfi");
}

/** @brief    Enables the MCU to enter deep sleep mode when all threads are suspended.
  *
  * @note:    When all threads are suspended, mcu can shut down some peripherals to 
  *           save power. For example, STM32 enters STOP mode in this condition, 
  *           its peripherals are not working and needs to be wake up by an external
  *           interrupt or MICO core's internal timer. So if you are using a peripherals,  
  *           you should disable this function temporarily.
  *           To make this function works, you should not disable the macro in MicoDefault.h: 
  *           MICO_DISABLE_MCU_POWERSAVE
  *
  * @param    enable : 1 = enable MCU powersave, 0 = disable MCU powersave
  * @return   none
  */
void MicoMcuPowerSaveConfig( int enable ){

  if (enable == 1)
    MCU_CLOCKS_NOT_NEEDED();
  else
    MCU_CLOCKS_NEEDED();
}

bool MicoShouldEnterMFGMode(void){
  return false;
}


