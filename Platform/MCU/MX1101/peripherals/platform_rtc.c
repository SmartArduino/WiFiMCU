/**
******************************************************************************
* @file    platform_rtc.c 
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

#include "platform.h"
#include "platform_peripheral.h"
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

platform_rtc_time_t mico_default_time =
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

OSStatus platform_rtc_init(void)
{
  //platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_rtc_get_time( platform_rtc_time_t* time)
{
  UNUSED_PARAMETER(time);
  platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_rtc_set_time( const platform_rtc_time_t* time )
{
  UNUSED_PARAMETER(time);
  platform_log("unimplemented");
  return kUnsupportedErr;
}




