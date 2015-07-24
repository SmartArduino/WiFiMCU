/**
******************************************************************************
* @file    os_timer.c 
* @author  William Xu
* @version V1.0.0
* @date    21-May-2015
* @brief   MiCO RTOS thread control demo.
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

#include "MICO.h"

#define os_timer_log(M, ...) custom_log("OS", M, ##__VA_ARGS__)

static mico_timer_t os_timer;

static void os_timer_timeout_handler( void* inContext )
{
  int time;
  time = (int)inContext;
  os_timer_log("%dms time is up", time);
}

void stop_timer( void )
{
  mico_stop_timer(&os_timer);
  mico_deinit_timer( &os_timer );
}

int application_start( void )
{
  OSStatus err = kNoErr;
  int time_sencond = 3*1000;
  
  os_timer_log("start os timer, timeout time is %dms", time_sencond);
  
  err = mico_init_timer(&os_timer, time_sencond, os_timer_timeout_handler, (void *)time_sencond);
  require_noerr( err, exit );
  
  mico_start_timer(&os_timer);
  
exit:
    return err;
}




