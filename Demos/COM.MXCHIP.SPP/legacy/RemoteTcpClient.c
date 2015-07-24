/**
  ******************************************************************************
  * @file    RemoteTcpClient.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   Create a TCP client thread, and connect to a remote server.
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
#include "MICODefine.h"
#include "SppProtocol.h"
#include "SocketUtils.h"
#include "MICONotificationCenter.h"

#define client_log(M, ...) custom_log("TCP client", M, ##__VA_ARGS__)
#define client_log_trace() custom_log_trace("TCP client")

#define CLOUD_RETRY  1

static bool _wifiConnected = false;
static mico_semaphore_t  _wifiConnected_sem = NULL;

void clientNotify_WifiStatusHandler(int event, mico_Context_t * const inContext)
{
  client_log_trace();
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    _wifiConnected = true;
    mico_rtos_set_semaphore(&_wifiConnected_sem);
    break;
  case NOTIFY_STATION_DOWN:
    _wifiConnected = false;
    break;
  default:
    break;
  }
  return;
}

void remoteTcpClient_thread(void *inContext)
{
  client_log_trace();
  OSStatus err = kUnknownErr;
  int len;
  mico_Context_t *Context = inContext;
  struct sockaddr_t addr;
  fd_set readfds;
  char ipstr[16];
  struct timeval_t t;
  int remoteTcpClient_loopBack_fd = -1;
  int remoteTcpClient_fd = -1;
  uint8_t *inDataBuffer = NULL;
  uint8_t *outDataBuffer = NULL;  
  
  mico_rtos_init_semaphore(&_wifiConnected_sem, 1);
  
  /* Regisist notifications */
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)clientNotify_WifiStatusHandler );
  require_noerr( err, exit ); 
  
  inDataBuffer = malloc(wlanBufferLen);
  require_action(inDataBuffer, exit, err = kNoMemoryErr);
  outDataBuffer = malloc(wlanBufferLen);
  require_action(inDataBuffer, exit, err = kNoMemoryErr);
  
  /*Loopback fd, recv data from other thread */
  remoteTcpClient_loopBack_fd = socket( AF_INET, SOCK_DGRM, IPPROTO_UDP );
  require_action(IsValidSocket( remoteTcpClient_loopBack_fd ), exit, err = kNoResourcesErr );
  addr.s_ip = IPADDR_LOOPBACK;
  addr.s_port = REMOTE_TCP_CLIENT_LOOPBACK_PORT;
  err = bind( remoteTcpClient_loopBack_fd, &addr, sizeof(addr) );
  require_noerr( err, exit );
  
  t.tv_sec = 4;
  t.tv_usec = 0;
  
  while(1) {
    if(remoteTcpClient_fd == -1 ) {
      if(_wifiConnected == false){
        require_action_quiet(mico_rtos_get_semaphore(&_wifiConnected_sem, 200000) == kNoErr, Continue, err = kTimeoutErr);
      }
      err = gethostbyname((char *)Context->flashContentInRam.appConfig.remoteServerDomain, (uint8_t *)ipstr, 16);
      require_noerr(err, ReConnWithDelay);
      
      remoteTcpClient_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      addr.s_ip = inet_addr(ipstr); 
      addr.s_port = Context->flashContentInRam.appConfig.remoteServerPort;
      
      err = connect(remoteTcpClient_fd, &addr, sizeof(addr));
      require_noerr_quiet(err, ReConnWithDelay);
      
      Context->appStatus.isRemoteConnected = true;
      client_log("Remote server connected at port: %d, fd: %d",  Context->flashContentInRam.appConfig.remoteServerPort,
                 remoteTcpClient_fd);
    }else{
      FD_ZERO(&readfds);
      FD_SET(remoteTcpClient_fd, &readfds);
      FD_SET(remoteTcpClient_loopBack_fd, &readfds);
      
      select(1, &readfds, NULL, NULL, &t);
      
      /*recv UART data using loopback fd*/
      if (FD_ISSET( remoteTcpClient_loopBack_fd, &readfds) ) {
        len = recv( remoteTcpClient_loopBack_fd, outDataBuffer, wlanBufferLen, 0 );
        SocketSend( remoteTcpClient_fd, outDataBuffer, len );
      }
      
      /*recv wlan data using remote client fd*/
      if (FD_ISSET(remoteTcpClient_fd, &readfds)) {
        len = recv(remoteTcpClient_fd, inDataBuffer, wlanBufferLen, 0);
        if(len <= 0) {
          client_log("Remote client closed, fd: %d", remoteTcpClient_fd);
          Context->appStatus.isRemoteConnected = false;
          goto ReConnWithDelay;
        }
        sppWlanCommandProcess(inDataBuffer, &len, remoteTcpClient_fd, Context);

      }

    Continue:    
      continue;
      
    ReConnWithDelay:
      if(remoteTcpClient_fd != -1){
        SocketClose(&remoteTcpClient_fd);
      }
      sleep(CLOUD_RETRY);
    }
  }
exit:
  if(inDataBuffer) free(inDataBuffer);
  if(outDataBuffer) free(outDataBuffer);
  if(remoteTcpClient_loopBack_fd != -1)
    SocketClose(&remoteTcpClient_loopBack_fd);
  client_log("Exit: Remote TCP client exit with err = %d", err);
  mico_rtos_delete_thread(NULL);
  return;
}

