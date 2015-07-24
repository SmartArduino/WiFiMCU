/**
  ******************************************************************************
  * @file    LocalTcpServer.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file create a TCP listener thread, accept every TCP client
  *          connection and create thread for them.
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

#include "MICO.h"
#include "MICODefine.h"
#include "MICOAppDefine.h"

#include "SppProtocol.h"
#include "SocketUtils.h"

#define server_log(M, ...) custom_log("TCP SERVER", M, ##__VA_ARGS__)
#define server_log_trace() custom_log_trace("TCP SERVER")

const int loopBackPortTable[20] = { 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 
                                    1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023};

static void localTcpClient_thread(void *inFd);
static mico_Context_t *Context;

mico_thread_t   localTcpClient_thread_handler;

void localTcpServer_thread(void *inContext)
{
  server_log_trace();
  OSStatus err = kUnknownErr;
  int i, j;
  Context = inContext;
  struct sockaddr_t addr;
  int sockaddr_t_size;
  fd_set readfds;
  char ip_address[16];
  
  int localTcpListener_fd = -1;

  for(i=0; i < MAX_Local_Client_Num; i++) 
    Context->appStatus.loopBack_PortList[i] = 0;

  /*Establish a TCP server fd that accept the tcp clients connections*/ 
  localTcpListener_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  require_action(IsValidSocket( localTcpListener_fd ), exit, err = kNoResourcesErr );
  addr.s_ip = INADDR_ANY;
  addr.s_port = Context->flashContentInRam.appConfig.localServerPort;
  err = bind(localTcpListener_fd, &addr, sizeof(addr));
  require_noerr( err, exit );

  err = listen(localTcpListener_fd, 0);
  require_noerr( err, exit );

  server_log("Server established at port: %d, fd: %d", Context->flashContentInRam.appConfig.localServerPort, localTcpListener_fd);
  
  while(1){
    FD_ZERO(&readfds);
    FD_SET(localTcpListener_fd, &readfds);  
    select(1, &readfds, NULL, NULL, NULL);

    /*Check tcp connection requests */
    if(FD_ISSET(localTcpListener_fd, &readfds)){
      sockaddr_t_size = sizeof(struct sockaddr_t);
      j = accept(localTcpListener_fd, &addr, &sockaddr_t_size);
      if (j > 0) {
        inet_ntoa(ip_address, addr.s_ip );
        server_log("Client %s:%d connected, fd: %d", ip_address, addr.s_port, j);
        if(kNoErr != mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Local Clients", localTcpClient_thread, STACK_SIZE_LOCAL_TCP_CLIENT_THREAD, &j) ) 
          SocketClose(&j);
      }
    }
   }

exit:
    server_log("Exit: Local controller exit with err = %d", err);
    mico_rtos_delete_thread(NULL);
    return;
}

void localTcpClient_thread(void *inFd)
{
  OSStatus err;
  int i;
  int indexForPortTable;
  int clientFd = *(int *)inFd;
  int clientLoopBackFd = -1;
  uint8_t *inDataBuffer = NULL;
  uint8_t *outDataBuffer = NULL;
  int len;
  struct sockaddr_t addr;
  fd_set readfds;
  struct timeval_t t;

  inDataBuffer = malloc(wlanBufferLen);
  require_action(inDataBuffer, exit, err = kNoMemoryErr);
  outDataBuffer = malloc(wlanBufferLen);
  require_action(outDataBuffer, exit, err = kNoMemoryErr);

  for(i=0; i < MAX_Local_Client_Num; i++) {
    if( Context->appStatus.loopBack_PortList[i] == 0 ){
      Context->appStatus.loopBack_PortList[i] = loopBackPortTable[clientFd];
      indexForPortTable = i;
      break;
    }
  }

  /*Loopback fd, recv data from other thread */
  clientLoopBackFd = socket( AF_INET, SOCK_DGRM, IPPROTO_UDP );
  require_action(IsValidSocket( clientLoopBackFd ), exit, err = kNoResourcesErr );
  addr.s_ip = IPADDR_LOOPBACK;
  addr.s_port = Context->appStatus.loopBack_PortList[indexForPortTable];
  err = bind( clientLoopBackFd, &addr, sizeof(addr) );
  require_noerr( err, exit );

  t.tv_sec = 4;
  t.tv_usec = 0;
  
  while(1){

    FD_ZERO(&readfds);
    FD_SET(clientFd, &readfds); 
    FD_SET(clientLoopBackFd, &readfds); 

    select(1, &readfds, NULL, NULL, &t);

    /*recv UART data using loopback fd*/
    if (FD_ISSET( clientLoopBackFd, &readfds )) {
      len = recv( clientLoopBackFd, outDataBuffer, wlanBufferLen, 0 );
      SocketSend( clientFd, outDataBuffer, len );
    }

    /*Read data from tcp clients and process these data using HA protocol */ 
    if (FD_ISSET(clientFd, &readfds)) {
      len = recv(clientFd, inDataBuffer, wlanBufferLen, 0);
      require_action_quiet(len>0, exit, err = kConnectionErr);
      sppWlanCommandProcess(inDataBuffer, &len, clientFd, Context);
    }
  }

exit:
    server_log("Exit: Client exit with err = %d", err);
    Context->appStatus.loopBack_PortList[indexForPortTable] = 0;
    if(clientLoopBackFd != -1)
      SocketClose(&clientLoopBackFd);
    SocketClose(&clientFd);
    if(inDataBuffer) free(inDataBuffer);
    if(outDataBuffer) free(outDataBuffer);
    mico_rtos_delete_thread(NULL);
    return;
}

