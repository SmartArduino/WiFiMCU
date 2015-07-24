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

static void localTcpClient_thread(void *inFd);
static mico_Context_t *Context;

mico_thread_t   localTcpClient_thread_handler;

void localTcpServer_thread(void *inContext)
{
  server_log_trace();
  OSStatus err = kUnknownErr;
  int j;
  Context = inContext;
  struct sockaddr_t addr;
  int sockaddr_t_size;
  fd_set readfds;
  char ip_address[16];
  
  int localTcpListener_fd = -1;

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
  int clientFd = *(int *)inFd;
  uint8_t *inDataBuffer = NULL;
  int len;
  fd_set readfds;
  fd_set writeSet;
  struct timeval_t t;
  int eventFd = -1;
  mico_queue_t queue;
  socket_msg_t *msg;
  int sent_len, errno;

  inDataBuffer = malloc(wlanBufferLen);
  require_action(inDataBuffer, exit, err = kNoMemoryErr);

  err = socket_queue_create(Context, &queue);
  require_noerr( err, exit );
  eventFd = mico_create_event_fd(queue);
  if (eventFd < 0) {
    server_log("create event fd error");
    goto exit_with_queue;
  } 

  t.tv_sec = 4;
  t.tv_usec = 0;
  
  while(1){

    FD_ZERO(&readfds);
    FD_SET(clientFd, &readfds); 
    FD_SET(eventFd, &readfds); 
    FD_ZERO(&writeSet );
    FD_SET(clientFd, &writeSet );

    select(24, &readfds, &writeSet, NULL, &t);
    /* send UART data */
    if ((FD_ISSET( eventFd, &readfds )) && (FD_ISSET( clientFd, &writeSet ))) { // have data and can write
        if(kNoErr == mico_rtos_pop_from_queue( &queue, &msg, 0)) {
           sent_len = write(clientFd, msg->data, msg->len);
           if (sent_len <= 0) {
              len = sizeof(errno);
              getsockopt(clientFd, SOL_SOCKET, SO_ERROR, &errno, &len);
              socket_msg_free(msg);
              server_log("write error, fd: %d, errno %d", clientFd, errno );
              if (errno != ENOMEM) {
                  goto exit_with_queue;
              }
           } else {
                  socket_msg_free(msg);
              }
           }
        }

    /*Read data from tcp clients and process these data using HA protocol */ 
    if (FD_ISSET(clientFd, &readfds)) {
      len = recv(clientFd, inDataBuffer, wlanBufferLen, 0);
      require_action_quiet(len>0, exit_with_queue, err = kConnectionErr);
      sppWlanCommandProcess(inDataBuffer, &len, clientFd, Context);
    }
  }

exit_with_queue:
    len = sizeof(errno);
    getsockopt(clientFd, SOL_SOCKET, SO_ERROR, &errno, &len);
    server_log("Exit: Client exit with err = %d, socket errno %d", err, errno);
    if (eventFd >= 0) {
        mico_delete_event_fd(eventFd);
    }
    socket_queue_delete(Context, &queue);
exit:
    SocketClose(&clientFd);
    if(inDataBuffer) free(inDataBuffer);
    mico_rtos_delete_thread(NULL);
    return;
}

