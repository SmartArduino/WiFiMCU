/**
******************************************************************************
* @file    MicoDriverRtc.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide RTC driver functions.
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
#include "MICOPlatform.h"
#include "MicoDefaults.h"

#include "platform.h"
#include "platform_common_config.h"

#include "board.h"

/******************************************************
*                      Macros
******************************************************/

#define MICO_VERIFY_TIME(time, valid) \
if( (time->sec > 60) || ( time->min > 60 ) || (time->hr > 24) || ( time->date > 31 ) || ( time->month > 12 )) \
  { \
    valid= false; \
  } \
else \
  { \
    valid= true; \
  }

/******************************************************
*                    Constants
******************************************************/

#ifndef MICO_DISABLE_MCU_POWERSAVE
#define RTC_Wakeup_init        MicoRtcInitialize            
#else
#ifdef MICO_ENABLE_MCU_RTC
#define platform_rtc_init      MicoRtcInitialize            
#else /* #ifdef MICO_ENABLE_MCU_RTC */
#define platform_rtc_noinit     MicoRtcInitialize
#endif /* #ifdef MICO_ENABLE_MCU_RTC */
#endif /* ifndef MICO_DISABLE_MCU_POWERSAVE */

#define USE_RTC_BKP 0x00BB32F2 // Use RTC BKP to initilize system time.

#define RTC_INTERRUPT_EXTI_LINE EXTI_Line22




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
*               Variables Definitions
******************************************************/

mico_rtc_time_t mico_default_time =
{
  /* set it to 12:20:30 08/04/2013 monday */
  .sec   = 30,
  .min   = 20,
  .hr    = 12,
  .weekday  = 1,
  .date  = 8,
  .month = 4,
  .year  = 13
};

/******************************************************
*               Function Declarations
******************************************************/


/******************************************************
*               Function Definitions
******************************************************/

void platform_rtc_noinit(void)
{
  
}

#if defined(MICO_DISABLE_MCU_POWERSAVE) && defined(MICO_ENABLE_MCU_RTC)
/*  */
void platform_rtc_init(void)
{
  /* Configure PIN0.21 as CLKOUT with pull-up, monitor the MAINCLK on scope */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 21, IOCON_MODE_PULLUP | IOCON_FUNC1 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF);
  Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_RTC, 1);
  
  /* Enable the RTC oscillator, oscillator rate can be determined by
     calling Chip_Clock_GetRTCOscRate()	*/
  Chip_Clock_EnableRTCOsc();  
  
  /* Initialize RTC driver (enables RTC clocking) */
  Chip_RTC_Init(LPC_RTC);

  /* Enable RTC as a peripheral wakeup event */
  Chip_SYSCTL_EnableWakeup(STARTERP0_RTC);

  /* RTC reset */
  Chip_RTC_Reset(LPC_RTC);

  /* Start RTC at a count of 0 when RTC is disabled. If the RTC is enabled, you
     need to disable it before setting the initial RTC count. */
  Chip_RTC_Disable(LPC_RTC);
  Chip_RTC_SetCount(LPC_RTC, 0);
  
  /* Enable RTC and high resolution timer - this can be done in a single
     call with Chip_RTC_EnableOptions(LPC_RTC, (RTC_CTRL_RTC1KHZ_EN | RTC_CTRL_RTC_EN)); */
  Chip_RTC_Enable1KHZ(LPC_RTC);
  Chip_RTC_Enable(LPC_RTC);
  
  /* Clear latched RTC interrupt statuses */
  Chip_RTC_ClearStatus(LPC_RTC, (RTC_CTRL_OFD | RTC_CTRL_ALARM1HZ | RTC_CTRL_WAKE1KHZ));
  
  /* Enable RTC interrupt */
  NVIC_EnableIRQ(RTC_IRQn); 
}
#endif


#ifndef MICO_DISABLE_MCU_POWERSAVE
void RTC_Wakeup_init(void)
{
 
}
#endif /* #ifndef MICO_DISABLE_MCU_POWERSAVE */

/**
* This function will return the value of time read from the on board CPU real time clock. Time value must be given in the format of
* the structure wiced_rtc_time_t
*
* @return    WICED_SUCCESS : on success.
* @return    WICED_ERROR   : if an error occurred with any step
*/
OSStatus MicoRtcGetTime(mico_rtc_time_t* time)
{
  uint32_t time_value;
#ifdef MICO_ENABLE_MCU_RTC
  time_value = Chip_RTC_GetCount(LPC_RTC);
  time->sec = time_value%60;
  time->min = (time_value/60)%60;
  time->hr = 0;
  time->weekday = 0;
  time->month = 0;
  time->year = 0;
  return kNoErr;
#else /* #ifdef WICED_ENABLE_MCU_RTC */ 
  UNUSED_PARAMETER(time);
  return kUnsupportedErr;
#endif /* #ifdef WICED_ENABLE_MCU_RTC */
}

/**
* This function will set MCU RTC time to a new value. Time value must be given in the format of
* the structure wiced_rtc_time_t
*
* @return    WICED_SUCCESS : on success.
* @return    WICED_ERROR   : if an error occurred with any step
*/
OSStatus MicoRtcSetTime(mico_rtc_time_t* time)
{
  uint32_t time_value;
#ifdef MICO_ENABLE_MCU_RTC
//  RTC_TimeTypeDef rtc_write_time;
//  RTC_DateTypeDef rtc_write_date;
  bool    valid = false;
  
  MicoMcuPowerSaveConfig(false);
  
//  MICO_VERIFY_TIME(time, valid);
//  if( valid == false )
//  {
//    return kParamErr;
//  }
//  rtc_write_time.RTC_Seconds = time->sec;
//  rtc_write_time.RTC_Minutes = time->min;
//  rtc_write_time.RTC_Hours   = time->hr;
//  rtc_write_date.RTC_WeekDay = time->weekday;
//  rtc_write_date.RTC_Date    = time->date;
//  rtc_write_date.RTC_Month   = time->month;
//  rtc_write_date.RTC_Year    = time->year;
//  
//  
//  RTC_SetTime( RTC_Format_BIN, &rtc_write_time );
//  RTC_SetDate( RTC_Format_BIN, &rtc_write_date );
  Chip_RTC_SetCount(LPC_RTC, time_value);
  MicoMcuPowerSaveConfig(true);
  return kNoErr;
#else /* #ifdef MICO_ENABLE_MCU_RTC */
  UNUSED_PARAMETER(time);
  return kUnsupportedErr;
#endif /* #ifdef MICO_ENABLE_MCU_RTC */
}

