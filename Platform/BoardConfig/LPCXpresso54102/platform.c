/**
******************************************************************************
* @file    platform.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide ADC driver functions.
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
#include "PlatformLogging.h"
#include "board.h" //Jer temp


const uint32_t mico_cpu_clock_hz = 96000000;

extern void STATUS_GPIO_Init(void);
extern void BOOT_GPIO_Init(void);
extern void STANDBY_GPIO_Init(void);
extern void EASYLINK_GPIO_Init(void);

/* Set up and initialize hardware prior to call to main */
void SystemInit(void)
{
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
	Board_SystemInit();
#endif
      LPC_SYSCTL->SYSAHBCLKCTRL[0] |= 0x00000018; // Magicoe
      
      Chip_SYSCTL_PowerUp(PDRUNCFG_PD_IRC_OSC_EN|PDRUNCFG_PD_IRC_EN);
      /* Configure PIN0.21 as CLKOUT with pull-up, monitor the MAINCLK on scope */
      Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_WLCLK_PORT , GPIO_WLCLK_PIN, IOCON_MODE_PULLUP | IOCON_FUNC1 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF);
      //Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 21, IOCON_MODE_PULLUP | IOCON_FUNC1 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF);
      Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_RTC, 1);
      Chip_Clock_EnableRTCOsc();
      Chip_RTC_Init(LPC_RTC);
      Chip_RTC_Enable1KHZ(LPC_RTC);
      Chip_RTC_Enable(LPC_RTC);
}

OSStatus mico_platform_init( void )
{
  platform_log( "Platform initialised" );
  
  return kNoErr;
}

void init_platform( void )
{
  /* INMUX and IOCON are used by many apps, enable both INMUX and IOCON clock bits here. */
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_MUX);
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
  /* Initialize GPIO */
  Chip_GPIO_Init(LPC_GPIO);
    //wifi nReset pin init.
  RESET_GPIO_Init();

  //LEDs init: MicoSysLed MicoRfLed,etc.
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 29, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 29);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, 0);
  
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 30, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 30);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 30, 1);
  
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 31, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 31);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 31, 1);
   // button init 
  STATUS_GPIO_Init();
  BOOT_GPIO_Init();
  STANDBY_GPIO_Init();
  EASYLINK_GPIO_Init();
}

void init_platform_bootloader( void )
{
  
}
/**==wifi reset, called by wlan_platform ==*/
void host_platform_reset_wifi( bool reset_asserted )
{
  if (reset_asserted)
    RESET_GPIO_Set(0);
  else
    RESET_GPIO_Set(1);
}
/**==wifi power, called by wlan_platform ==*/
void host_platform_power_wifi( bool power_enabled )
{
  
}
/**==platform(board) status LEDs==*/
void MicoSysLed(bool onoff)
{
 // Board_LED_Set(0, onoff);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, onoff);
}

void MicoRfLed(bool onoff)
{
 // Board_LED_Set(1, onoff);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 30, onoff);
}

void Mico2rdLED(bool onoff)
{
//  Board_LED_Set(2, onoff);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 31, onoff);
}


