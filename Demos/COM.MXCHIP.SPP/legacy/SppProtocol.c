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
#include "MICONotificationCenter.h"
#include <stdio.h>

#define spp_log(M, ...) custom_log("SPP", M, ##__VA_ARGS__)
#define spp_log_trace() custom_log_trace("SPP")

static int _recved_uart_loopback_fd = -1;

OSStatus sppProtocolInit(mico_Context_t * const inContext)
{
  spp_log_trace();
  OSStatus err = kUnknownErr;
  (void)inContext;
  struct sockaddr_t addr;

  inContext->appStatus.isRemoteConnected = false;

  _recved_uart_loopback_fd = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
  addr.s_ip = IPADDR_LOOPBACK;
  addr.s_port = RECVED_UART_DATA_LOOPBACK_PORT;
  bind(_recved_uart_loopback_fd, &addr, sizeof(addr));

  return err;
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
  struct sockaddr_t addr;

  addr.s_ip = IPADDR_LOOPBACK;

  for(i=0; i < MAX_Local_Client_Num; i++) {
    if( inContext->appStatus.loopBack_PortList[i] != 0 ){
      addr.s_port = inContext->appStatus.loopBack_PortList[i];
      sendto(_recved_uart_loopback_fd, inBuf, inLen, 0, &addr, sizeof(addr));
    }
  }

  if(inContext->appStatus.isRemoteConnected==true){
    addr.s_ip = IPADDR_LOOPBACK;
    addr.s_port = REMOTE_TCP_CLIENT_LOOPBACK_PORT;
    sendto(_recved_uart_loopback_fd, inBuf, inLen, 0, &addr, sizeof(addr));
  }        
  return err;
}



