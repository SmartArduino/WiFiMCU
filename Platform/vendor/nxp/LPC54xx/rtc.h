/**
******************************************************************************
* @file    rtc.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide all the headers of RTC functions used for time 
*          calculation in STM32 stop mode.
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


#include "MicoPlatform.h"
#pragma once

#define MICO_VERIFY_TIME(time, valid) \
    if( (time->sec > 60) || ( time->min > 60 ) || (time->hr > 24) || ( time->date > 31 ) || ( time->month > 12 )) \
    { \
        valid= false; \
    } \
    else \
    { \
        valid= true; \
    }



#ifndef MICO_DISABLE_MCU_POWERSAVE
OSStatus rtc_sleep_entry( void );
OSStatus rtc_sleep_abort( void );
OSStatus rtc_sleep_exit( unsigned long requested_sleep_time, unsigned long *cpu_sleep_time );
#endif /* #ifndef MICO_DISABLE_MCU_POWERSAVE */

OSStatus platform_set_rtc_time(mico_rtc_time_t* time);
