/**
******************************************************************************
* @file    os_sem.c 
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

#define os_sem_log(M, ...) custom_log("OS", M, ##__VA_ARGS__)

static mico_semaphore_t os_sem;

void get_thread(void *inContext)
{
  int get_count = 1;
  while(mico_rtos_get_semaphore(&os_sem, MICO_WAIT_FOREVER) == kNoErr)
  {
    os_sem_log("os semaphore is get, count %d", get_count++);
  }
}

void set_thread(void *inContext)
{
  int set_count = 1;
  while(1)
  {
    mico_thread_sleep(5);
    mico_rtos_set_semaphore(&os_sem);
    os_sem_log("os semaphore is set, count %d", set_count++);
  }
}

int application_start( void )
{
  OSStatus err = kNoErr;

  err = mico_rtos_init_semaphore(&os_sem, 1);
  require_noerr( err, exit ); 
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "get sem", get_thread, 0x800, NULL);
  require_noerr_action( err, exit, os_sem_log("ERROR: Unable to start the set sem thread.") );
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "set sem", set_thread, 0x800, NULL);
  require_noerr_action( err, exit, os_sem_log("ERROR: Unable to start the set sem thread.") );
  
  
  return err;

exit:
  os_sem_log("ERROR, err: %d", err);
  return err;
}



