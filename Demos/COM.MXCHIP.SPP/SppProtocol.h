/**
  ******************************************************************************
  * @file    SppProtocol.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief    This file provides all the headers of SPP data convert protocol.
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

#ifndef __SPPPROTOCOL_H
#define __SPPPROTOCOL_H

#include "Common.h"
#include "MICODefine.h"

OSStatus sppProtocolInit(mico_Context_t * const inContext);
int is_network_state(int state);
OSStatus sppWlanCommandProcess(unsigned char *inBuf, int *inBufLen, int inSocketFd, mico_Context_t * const inContext);
OSStatus sppUartCommandProcess(uint8_t *inBuf, int inLen, mico_Context_t * const inContext);


void set_network_state(int state, int on);
int socket_queue_create(mico_Context_t * const inContext, mico_queue_t *queue);
int socket_queue_delete(mico_Context_t * const inContext, mico_queue_t *queue);
void socket_msg_free(socket_msg_t*msg);
void socket_msg_take(socket_msg_t*msg);

#endif
