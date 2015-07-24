/**
******************************************************************************
* @file    platform_pwm.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide PWM driver functions.
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

/******************************************************
*               Function Declarations
******************************************************/

/******************************************************
*               Function Definitions
******************************************************/

OSStatus platform_pwm_init( const platform_pwm_t* pwm, uint32_t frequency, float duty_cycle )
{
  UNUSED_PARAMETER(pwm);
  UNUSED_PARAMETER(frequency);
  UNUSED_PARAMETER(duty_cycle);
  platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_pwm_start( const platform_pwm_t* pwm )
{
  UNUSED_PARAMETER(pwm);
  platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_pwm_stop( const platform_pwm_t* pwm )
{
  UNUSED_PARAMETER(pwm);
  platform_log("unimplemented");
  return kUnsupportedErr;
}




