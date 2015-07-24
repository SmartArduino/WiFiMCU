/**
******************************************************************************
* @file    udp_echo.c 
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
#include "MICONotificationCenter.h"

#define udp_echo_log(M, ...) custom_log("UDP", M, ##__VA_ARGS__)

#define BUF_LEN (3*1024)

static int udp_port = 8090;
static char *ap_ssid = "sqdmz";
static char *ap_key  = "0987654321";

static network_InitTypeDef_adv_st wNetConfigAdv;
static mico_semaphore_t udp_sem;

void micoNotify_WifiStatusHandler(WiFiEvent event,  const int inContext)
{
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    udp_echo_log("Station up");
    mico_rtos_set_semaphore(&udp_sem);
    MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    udp_echo_log("Station down");
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
  udp_echo_log("Wlan Connection Err %d", err);
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
  
  udp_echo_log("connect to %s...", wNetConfigAdv.ap_info.ssid);
}

void udp_echo_thread(void *inContext)
{
  OSStatus err;
  struct sockaddr_t addr;
  struct timeval_t t;
  fd_set readfds;
  socklen_t addrLen;
  int udp_fd = -1 , len;
  char *buf;
  
  buf = (char*)malloc(BUF_LEN);
  require_action(buf, exit, err = kNoMemoryErr);
  
  /*Establish a UDP port to receive any data sent to this port*/
  udp_fd = socket( AF_INET, SOCK_DGRM, IPPROTO_UDP );
  require_action(IsValidSocket( udp_fd ), exit, err = kNoResourcesErr );
  addr.s_ip = INADDR_ANY;
  addr.s_port = udp_port;
  err = bind(udp_fd, &addr, sizeof(addr));
  require_noerr( err, exit );
  udp_echo_log("Start UDP echo mode, Open UDP port %d", addr.s_port);
  
  t.tv_sec = 20;
  t.tv_usec = 0;
  
  while(1)
  {
    /*Check status on erery sockets */
    FD_ZERO(&readfds);
    FD_SET(udp_fd, &readfds);

    select(1, &readfds, NULL, NULL, &t);
    
    /*Read data from udp and send data back */ 
    if (FD_ISSET( udp_fd, &readfds )) 
    {
      len = recvfrom(udp_fd, buf, BUF_LEN, 0, &addr, &addrLen);
      udp_echo_log("[udp_rec][%d] = %.*s", len, len, buf);
      sendto(udp_fd, buf, len, 0, &addr, sizeof(struct sockaddr_t));
    }
  }
  
exit:
  mico_rtos_delete_thread(NULL);
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
  
  err = mico_rtos_init_semaphore(&udp_sem, 1);
  require_noerr( err, exit ); 
  
  connect_ap( );
  
  mico_rtos_get_semaphore(&udp_sem, MICO_WAIT_FOREVER);
  
  micoWlanGetIPStatus(&para, Station);
  udp_echo_log("tcp server ip: %s", para.ip);
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "UDP_ECHO", udp_echo_thread, 0x800, NULL );
  require_noerr_action( err, exit, udp_echo_log("ERROR: Unable to start the UDP echo thread.") );
  
  return err;

exit:
  udp_echo_log("ERROR, err: %d", err);
  return err;
}

