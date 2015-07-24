/**
  ******************************************************************************
  * @file    SppProtocol.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   SPP protocol deliver any data received from UART to wlan and deliver
  * wlan data to UART.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
  ******************************************************************************
  */ 

#include "MICOAppDefine.h"
#include "SppProtocol.h"
#include "SocketUtils.h"
#include "debug.h"
#include "MicoPlatform.h"
#include "platform_config.h"
#include "MICONotificationCenter.h"
#include <stdio.h>

#define MAX_SOCK_MSG_LEN (10*1024)
int sockmsg_len = 0;
#define spp_log(M, ...) custom_log("SPP", M, ##__VA_ARGS__)
#define spp_log_trace() custom_log_trace("SPP")

void socket_msg_take(socket_msg_t*msg);
void socket_msg_free(socket_msg_t*msg);


OSStatus sppProtocolInit(mico_Context_t * const inContext)
{
  int i;
  
  spp_log_trace();
  (void)inContext;

  for(i=0; i < MAX_QUEUE_NUM; i++) {
    inContext->appStatus.socket_out_queue[i] = NULL;
  }
  mico_rtos_init_mutex(&inContext->appStatus.queue_mtx);
  return kNoErr;
}

OSStatus sppWlanCommandProcess(unsigned char *inBuf, int *inBufLen, int inSocketFd, mico_Context_t * const inContext)
{
  spp_log_trace();
  (void)inSocketFd;
  (void)inContext;
  OSStatus err = kUnknownErr;

  err = MicoUartSend(UART_FOR_APP, inBuf, *inBufLen);

  *inBufLen = 0;
  return err;
}

OSStatus sppUartCommandProcess(uint8_t *inBuf, int inLen, mico_Context_t * const inContext)
{
  spp_log_trace();
  OSStatus err = kNoErr;
  int i;
  mico_queue_t* p_queue=NULL;
  socket_msg_t *real_msg;

  for(i=0; i < MAX_QUEUE_NUM; i++) {
    p_queue = inContext->appStatus.socket_out_queue[i];
    if(p_queue  != NULL ){
      break;
    }
  }
  if (p_queue == NULL)
    return kNoErr;
  
  if (MAX_SOCK_MSG_LEN < sockmsg_len)
    return kNoMemoryErr;
  real_msg = (socket_msg_t*)malloc(sizeof(socket_msg_t) - 1 + inLen);

  if (real_msg == NULL)
    return kNoMemoryErr;
  sockmsg_len += (sizeof(socket_msg_t) - 1 + inLen);
  real_msg->len = inLen;
  memcpy(real_msg->data, inBuf, inLen);
  real_msg->ref = 0;
  
  mico_rtos_lock_mutex(&inContext->appStatus.queue_mtx);
  socket_msg_take(real_msg);
  for(i=0; i < MAX_QUEUE_NUM; i++) {
    p_queue = inContext->appStatus.socket_out_queue[i];
    if(p_queue  != NULL ){
      socket_msg_take(real_msg);
      if (kNoErr != mico_rtos_push_to_queue(p_queue, &real_msg, 0)) {
        socket_msg_free(real_msg);
    }
  }
  }        
  socket_msg_free(real_msg);
  mico_rtos_unlock_mutex(&inContext->appStatus.queue_mtx);
  return err;
}

void socket_msg_take(socket_msg_t*msg)
{
    msg->ref++;
}

void socket_msg_free(socket_msg_t*msg)
{
    msg->ref--;
    if (msg->ref == 0) {
        sockmsg_len -= (sizeof(socket_msg_t) - 1 + msg->len);
        free(msg);
    
    }
}

int socket_queue_create(mico_Context_t * const inContext, mico_queue_t *queue)
{
    OSStatus err;
    int i;
    mico_queue_t *p_queue;
    
    err = mico_rtos_init_queue(queue, "sockqueue", sizeof(int), MAX_QUEUE_LENGTH);
    if (err != kNoErr)
        return -1;
    mico_rtos_lock_mutex(&inContext->appStatus.queue_mtx);
    for(i=0; i < MAX_QUEUE_NUM; i++) {
        p_queue = inContext->appStatus.socket_out_queue[i];
        if(p_queue == NULL ){
            inContext->appStatus.socket_out_queue[i] = queue;
            mico_rtos_unlock_mutex(&inContext->appStatus.queue_mtx);
            return 0;
        }
    }        
    mico_rtos_unlock_mutex(&inContext->appStatus.queue_mtx);
    mico_rtos_deinit_queue(queue);
    return -1;
}

int socket_queue_delete(mico_Context_t * const inContext, mico_queue_t *queue)
{
    int i;
    socket_msg_t *msg;
    int ret = -1;

    mico_rtos_lock_mutex(&inContext->appStatus.queue_mtx);
    // remove queue
    for(i=0; i < MAX_QUEUE_NUM; i++) {
        if (queue == inContext->appStatus.socket_out_queue[i]) {
            inContext->appStatus.socket_out_queue[i] = NULL;
            ret = 0;
        }
    }
    mico_rtos_unlock_mutex(&inContext->appStatus.queue_mtx);
    // free queue buffer
    while(kNoErr == mico_rtos_pop_from_queue( queue, &msg, 0)) {
        socket_msg_free(msg);
    }

    // deinit queue
    mico_rtos_deinit_queue(queue);
    
    return ret;
}

