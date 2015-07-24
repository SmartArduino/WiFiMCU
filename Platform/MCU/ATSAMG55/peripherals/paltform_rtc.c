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

#include "MICORTOS.h"
#include "MICOPlatform.h"
#include "MicoDefaults.h"

#include "platform.h"
#include "platformLogging.h"
#include "platform_peripheral.h"

/******************************************************
*                      Macros
******************************************************/

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
 *               Static Function Declarations
 ******************************************************/

/******************************************************
*               Variables Definitions
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/


/******************************************************
*               Function Definitions
******************************************************/

OSStatus platform_rtc_init(void)
{
  platform_log("unimplemented");
  return kUnsupportedErr;
}


/**
* This function will return the value of time read from the on board CPU real time clock. Time value must be given in the format of
* the structure wiced_rtc_time_t
*
* @return    WICED_SUCCESS : on success.
* @return    WICED_ERROR   : if an error occurred with any step
*/
OSStatus platform_rtc_get_time( platform_rtc_time_t* time)
{
  UNUSED_PARAMETER(time);
  return kUnsupportedErr;
}

/**
* This function will set MCU RTC time to a new value. Time value must be given in the format of
* the structure wiced_rtc_time_t
*
* @return    WICED_SUCCESS : on success.
* @return    WICED_ERROR   : if an error occurred with any step
*/
OSStatus platform_rtc_set_time( const mico_rtc_time_t* time )
{
  UNUSED_PARAMETER(time);
  return kUnsupportedErr;
}
















