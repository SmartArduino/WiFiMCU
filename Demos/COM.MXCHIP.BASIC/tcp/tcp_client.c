/**
******************************************************************************
* @file    tcp_client.c 
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

#define tcp_client_log(M, ...) custom_log("TCP", M, ##__VA_ARGS__)

#define BUF_LEN (3*1024)

static char *ap_ssid = "sqdmz";
static char *ap_key  = "0987654321";
static char tcp_remote_ip[16] = "192.168.1.100";
static int tcp_remote_port = 8080;

static network_InitTypeDef_adv_st wNetConfigAdv;
static mico_semaphore_t tcp_sem;

void micoNotify_WifiStatusHandler(WiFiEvent event,  const int inContext)
{
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    tcp_client_log("Station up");
    mico_rtos_set_semaphore(&tcp_sem);
    MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    tcp_client_log("Station down");
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
  tcp_client_log("Wlan Connection Err %d", err);
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
  
  tcp_client_log("connect to %s...", wNetConfigAdv.ap_info.ssid);
}

void tcp_client_thread(void *inContext)
{
  OSStatus err;
  struct sockaddr_t addr;
  struct timeval_t t;
  fd_set readfds;
  int tcp_fd = -1 , len;
  char *buf;
  
  buf = (char*)malloc(BUF_LEN);
  require_action(buf, exit, err = kNoMemoryErr);
  
  while(1)
  {
    if ( tcp_fd == -1 ) 
    {
      tcp_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
      require_action(IsValidSocket( tcp_fd ), exit, err = kNoResourcesErr );
      addr.s_ip = inet_addr(tcp_remote_ip);
      addr.s_port = tcp_remote_port;
      err = connect(tcp_fd, &addr, sizeof(addr));
      require_noerr_quiet(err, ReConnWithDelay);
      tcp_client_log("Remote server connected at port: %d, fd: %d",  addr.s_port, tcp_fd);
    }
    else
    {
      /*Check status on erery sockets */
      FD_ZERO(&readfds);
      FD_SET(tcp_fd, &readfds);
      t.tv_sec = 4;
      t.tv_usec = 0;

      select(1, &readfds, NULL, NULL, &t);
      
      /*recv wlan data using remote client fd*/
      if (FD_ISSET( tcp_fd, &readfds )) 
      {
        len = recv(tcp_fd, buf, BUF_LEN, 0);
        if( len <= 0) {
          tcp_client_log("Remote client closed, fd: %d", tcp_fd);
          goto ReConnWithDelay;
        }
        
        tcp_client_log("[tcp_rec][%d] = %.*s", len, len, buf);
        sendto(tcp_fd, buf, len, 0, &addr, sizeof(struct sockaddr_t));
      }
   
      continue;
      
    ReConnWithDelay:
        if(tcp_fd != -1){
          SocketClose(&tcp_fd);
        }
        tcp_client_log("Connect to %s failed! Reconnect in 5 sec...", tcp_remote_ip);
        sleep( 5 );
    }
  }
  
exit:
  mico_rtos_delete_thread(NULL);
}

int application_start( void )
{
  OSStatus err = kNoErr;
  
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
    
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "TCP_client", tcp_client_thread, 0x800, NULL );
  require_noerr_action( err, exit, tcp_client_log("ERROR: Unable to start the tcp client thread.") );
  
  return err;

exit:
  tcp_client_log("ERROR, err: %d", err);
  return err;
}

