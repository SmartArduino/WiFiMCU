/**
******************************************************************************
* @file    os_mutex.c 
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

#define os_mutex_log(M, ...) custom_log("OS", M, ##__VA_ARGS__)

static mico_mutex_t os_mutex;
static const char *os_mutex1 = "mutex 1";
static const char *os_mutex2 = "mutex 2";
static const char *os_mutex3 = "mutex 3";

void mutex_thread(void *inContext)
{
  int delay;
  char *thread_name = (char *)inContext;
  srand( 1000 );
  while(1)
  {
    delay = rand()%9 + 1;
    mico_rtos_lock_mutex(&os_mutex);
    os_mutex_log("%s thread is using resources, delay %ds", thread_name, delay);
    mico_thread_sleep(delay);
    os_mutex_log("%s thread will release resource", thread_name);
    mico_rtos_unlock_mutex(&os_mutex);  
    mico_thread_sleep(5);
  }
}

int application_start( void )
{
  OSStatus err = kNoErr;
  
  err = mico_rtos_init_mutex(&os_mutex);
  require_noerr( err, exit ); 
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "mutex1", mutex_thread, 0x800, (void *)os_mutex1);
  require_noerr_action( err, exit, os_mutex_log("ERROR: Unable to start the mutex1 thread.") );

  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "mutex2", mutex_thread, 0x800, (void *)os_mutex2);
  require_noerr_action( err, exit, os_mutex_log("ERROR: Unable to start the mutex2 thread.") );
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "mutex2", mutex_thread, 0x800, (void *)os_mutex3);
  require_noerr_action( err, exit, os_mutex_log("ERROR: Unable to start the mutex2 thread.") );
  
  return err;

exit:
  os_mutex_log("ERROR, err: %d", err);
  return err;
}


