/**
******************************************************************************
* @file    tcp_server.c 
* @author  William Xu
* @version V1.0.0
* @date    21-May-2015
* @brief   First MiCO application to say hello world!
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
#include "SocketUtils.h"
#include "MICONotificationCenter.h"

#define tcp_server_log(M, ...) custom_log("TCP", M, ##__VA_ARGS__)

#define BUF_LEN (3*1024)

static char *ap_ssid = "sqdmz";
static char *ap_key  = "0987654321";
static int tcp_server_port = 8080;

static network_InitTypeDef_adv_st wNetConfigAdv;
static mico_semaphore_t tcp_sem;

void micoNotify_WifiStatusHandler(WiFiEvent event,  const int inContext)
{
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    tcp_server_log("Station up");
    mico_rtos_set_semaphore(&tcp_sem);
    MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    tcp_server_log("Station down");
    MicoRfLed(false);
    break;
  default:
    break;
  }
  return;
}

void micoNotify_ConnectFailedHandler(OSStatus err, const int inContext)
{
  (void)inContext;
  tcp_server_log("Wlan Connection Err %d", err);
}

static void connect_ap( void )
{  
  memset(&wNetConfigAdv, 0x0, sizeof(network_InitTypeDef_adv_st));
  
  strcpy((char*)wNetConfigAdv.ap_info.ssid, ap_ssid);
  strcpy((char*)wNetConfigAdv.key, ap_key);
  wNetConfigAdv.key_len = strlen(ap_key);
  wNetConfigAdv.ap_info.security = SECURITY_TYPE_AUTO;
  wNetConfigAdv.ap_info.channel = 0; //Auto
  wNetConfigAdv.dhcpMode = DHCP_Client;
  wNetConfigAdv.wifi_retry_interval = 100;
  micoWlanStartAdv(&wNetConfigAdv);
  
  tcp_server_log("connect to %s...", wNetConfigAdv.ap_info.ssid);
}

void tcp_server_client_thread( void *inFd )
{
  OSStatus err;
  struct sockaddr_t addr;
  struct timeval_t t;
  fd_set readfds;
  int client_fd = *(int *)inFd; 
  int len;
  char *buf;
  
  buf = (char*)malloc(BUF_LEN);
  require_action(buf, exit, err = kNoMemoryErr);
  
  t.tv_sec = 4;
  t.tv_usec = 0;
      
  while(1)
  {
    /*Check status on erery sockets */
    FD_ZERO(&readfds);
    FD_SET(client_fd, &readfds);

    select(1, &readfds, NULL, NULL, &t);

    /*recv wlan data using remote server fd*/
    if (FD_ISSET( client_fd, &readfds )) 
    {
      len = recv(client_fd, buf, BUF_LEN, 0); 
      require_action_quiet(len>0, exit, err = kConnectionErr);
      tcp_server_log("[tcp_rec][%d] = %.*s", len, len, buf);
      sendto(client_fd, buf, len, 0, &addr, sizeof(struct sockaddr_t));
    }
  }
  
exit:
  tcp_server_log("Exit: Client exit with err = %d, fd:%d", err, client_fd);
  SocketClose(&client_fd);
  if(buf) free(buf);
  mico_rtos_delete_thread(NULL);
}

void tcp_server_thread(void *inContext)
{
  OSStatus err;
  int j;
  struct sockaddr_t addr;
  int sockaddr_t_size;
  fd_set readfds;
  char ip_address[16];
  
  int tcp_listener_fd = -1;

  /*Establish a TCP server fd that accept the tcp clients connections*/ 
  tcp_listener_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  require_action(IsValidSocket( tcp_listener_fd ), exit, err = kNoResourcesErr );
  addr.s_ip = INADDR_ANY;
  addr.s_port = tcp_server_port;
  err = bind(tcp_listener_fd, &addr, sizeof(addr));
  require_noerr( err, exit );

  err = listen(tcp_listener_fd, 0);
  require_noerr( err, exit );

  tcp_server_log("Server established at port: %d, fd: %d", addr.s_port, tcp_listener_fd);
  
  while(1){
    FD_ZERO(&readfds);
    FD_SET(tcp_listener_fd, &readfds);  
    select(1, &readfds, NULL, NULL, NULL);

    /*Check tcp connection requests */
    if(FD_ISSET(tcp_listener_fd, &readfds)){
      sockaddr_t_size = sizeof(struct sockaddr_t);
      j = accept(tcp_listener_fd, &addr, &sockaddr_t_size);
      if (j > 0) {
        inet_ntoa(ip_address, addr.s_ip );
        tcp_server_log("Client %s:%d connected, fd: %d", ip_address, addr.s_port, j);
        if(kNoErr != mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Local Clients", tcp_server_client_thread, 0x800, &j) ) 
          SocketClose(&j);
      }
    }
   }

exit:
    tcp_server_log("Exit: Local controller exit with err = %d", err);
    mico_rtos_delete_thread(NULL);
    return;
}

int application_start( void )
{
  OSStatus err = kNoErr;
  IPStatusTypedef para;
  
  MicoInit( );
  
  /*The notification message for the registered WiFi status change*/
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)micoNotify_WifiStatusHandler );
  require_noerr( err, exit ); 
  
  err = MICOAddNotification( mico_notify_WIFI_CONNECT_FAILED, (void *)micoNotify_ConnectFailedHandler );
  require_noerr( err, exit );
  
  err = mico_rtos_init_semaphore(&tcp_sem, 1);
  require_noerr( err, exit );
  
  connect_ap( );
  
  mico_rtos_get_semaphore(&tcp_sem, MICO_WAIT_FOREVER);
  
  micoWlanGetIPStatus(&para, Station);
  tcp_server_log("tcp server ip: %s", para.ip);

  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "TCP_server", tcp_server_thread, 0x800, NULL );
  require_noerr_action( err, exit, tcp_server_log("ERROR: Unable to start the tcp server thread.") );
  
  return err;

exit:
  tcp_server_log("ERROR, err: %d", err);
  return err;
}

