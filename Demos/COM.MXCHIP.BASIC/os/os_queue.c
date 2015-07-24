/**
******************************************************************************
* @file    os_queue.c 
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

#define os_queue_log(M, ...) custom_log("OS", M, ##__VA_ARGS__)

static mico_queue_t os_queue;

typedef struct _msg
{
  int sourse;
  int value;
} msg_t;

static msg_t msgstruct[2] = {
  {1, 100},
  {2, 200}
};

void receiver_thread(void *inContext)
{
  OSStatus err;
  msg_t *Received;
  
  while(1)
  {
    
    err = mico_rtos_pop_from_queue( &os_queue, &Received, 1000);
    if ( err == kNoErr)
    {
      if ( Received->sourse == 1 )
      {
        os_queue_log("form sender1 thread, value = %d", Received->value);
      }
      else if ( Received->sourse == 2 )
      {
        os_queue_log("form sender2 thread, value = %d", Received->value);
      }
    }
    else
    {
      os_queue_log("received timeout");
    }
  }
}

void sender_thread(void *inContext)
{
  int delay;
  srand( 1000 );
  while(1)
  {
    delay = rand()%9 + 1;
    os_queue_log("delay %ds", delay);
    mico_thread_sleep(delay);
    mico_rtos_push_to_queue(&os_queue, &inContext, 0);
  }
}

int application_start( void )
{
  OSStatus err = kNoErr;
  
  err = mico_rtos_init_queue(&os_queue, "queue", sizeof(msg_t), 3);
  require_noerr( err, exit ); 
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "receiver", receiver_thread, 0x800, NULL);
  require_noerr_action( err, exit, os_queue_log("ERROR: Unable to start the receiver thread.") );
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "sender1", sender_thread, 0x800, &(msgstruct[0]));
  require_noerr_action( err, exit, os_queue_log("ERROR: Unable to start the sender1 thread.") );

  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "sender2", sender_thread, 0x800, &(msgstruct[1]));
  require_noerr_action( err, exit, os_queue_log("ERROR: Unable to start the sender2 thread.") );
  
  return err;

exit:
  os_queue_log("ERROR, err: %d", err);
  return err;
}


