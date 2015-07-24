/**
******************************************************************************
* @file    wifi_airkiss.c 
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
#include "StringUtils.h"
#include "MICONotificationCenter.h"

#define wifi_airkiss_log(M, ...) custom_log("WIFI", M, ##__VA_ARGS__)

#define Airkiss_ConnectWlan_Timeout    20000
static int is_airkiss_success;
static uint8_t airkiss_data;
static char ap_ssid[64], ap_key[32];
static mico_semaphore_t      airkiss_sem;
static network_InitTypeDef_adv_st wNetConfigAdv;

void micoNotify_ConnectFailedHandler(OSStatus err, const int inContext)
{
  (void)inContext;
  wifi_airkiss_log("Wlan Connection Err %d", err);
}

void AirkissNotify_WifiStatusHandler(WiFiEvent event, const int inContext)
{
  switch (event) {
  case NOTIFY_STATION_UP:
    wifi_airkiss_log("Access point connected");
    mico_rtos_set_semaphore(&airkiss_sem);
    MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    MicoRfLed(false);
    break;
  case NOTIFY_AP_UP:
    MicoRfLed(true);
    break;
  case NOTIFY_AP_DOWN:
    MicoRfLed(false);
    break;
  default:
    break;
  }

}

void AirkissNotify_AirkissCompleteHandler(network_InitTypeDef_st *nwkpara, const int inContext)
{
  OSStatus err;
  wifi_airkiss_log("airkiss return");
  require_action(nwkpara, exit, err = kTimeoutErr);
  strcpy(ap_ssid, nwkpara->wifi_ssid);
  strcpy(ap_key, nwkpara->wifi_key);
  wifi_airkiss_log("Get SSID: %s, Key: %s", nwkpara->wifi_ssid, nwkpara->wifi_key);
  return;
  
exit:
  wifi_airkiss_log("ERROR, err: %d", err);
  mico_rtos_set_semaphore(&airkiss_sem);
}

void AirkissNotify_AirkissGetExtraDataHandler(int datalen, char* data, const int inContext)
{
  char *debugString;

  debugString = DataToHexStringWithSpaces( (const uint8_t *)data, datalen );
  wifi_airkiss_log("Get user info: %s", debugString);
  free(debugString);
  airkiss_data = data[0];
  is_airkiss_success = 1;
  mico_rtos_set_semaphore(&airkiss_sem);
}

void clean_airkiss_resource( void )
{
  MICORemoveNotification( mico_notify_EASYLINK_WPS_COMPLETED, (void *)AirkissNotify_AirkissCompleteHandler );
  MICORemoveNotification( mico_notify_EASYLINK_GET_EXTRA_DATA, (void *)AirkissNotify_AirkissGetExtraDataHandler );
  MICORemoveNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)AirkissNotify_WifiStatusHandler );
    
  mico_rtos_deinit_semaphore(&airkiss_sem);
  airkiss_sem = NULL;
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
  
  wifi_airkiss_log("connect to %s...", wNetConfigAdv.ap_info.ssid);
}

void airkiss_thread(void *inContext)
{
  int fd;
  struct sockaddr_t addr;
  int i = 0;
  
  micoWlanStartAirkiss( 40 );
  wifi_airkiss_log("Start Airkiss configuration");
  mico_rtos_get_semaphore(&airkiss_sem, MICO_WAIT_FOREVER);
  
  if ( is_airkiss_success == 1 )
  {
    mico_thread_msleep(10);
    connect_ap( );
    
    mico_rtos_get_semaphore(&airkiss_sem, Airkiss_ConnectWlan_Timeout);
    fd = socket( AF_INET, SOCK_DGRM, IPPROTO_UDP );
    if (fd < 0)
      goto threadexit;
    
    addr.s_ip = INADDR_BROADCAST;
    addr.s_port = 10000;
    wifi_airkiss_log("Send UDP to WECHAT");
    while(1){
      sendto(fd, &airkiss_data, 1, 0, &addr, sizeof(addr));
      
      msleep(10);
      i++;
      if (i > 20)
        break;
    }  

  } else {
    wifi_airkiss_log("Airkiss configuration fail");
  }
  
threadexit:  
  clean_airkiss_resource( );
  mico_rtos_delete_thread(NULL);
}

int application_start( void )
{
  OSStatus err = kNoErr;
  is_airkiss_success = 0;

  MicoInit( );
  
  err = MICOAddNotification( mico_notify_WIFI_CONNECT_FAILED, (void *)micoNotify_ConnectFailedHandler );
  require_noerr( err, exit );
   
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)AirkissNotify_WifiStatusHandler );
  require_noerr(err, exit);
  
  err = MICOAddNotification( mico_notify_EASYLINK_WPS_COMPLETED, (void *)AirkissNotify_AirkissCompleteHandler );  
  require_noerr(err, exit);
  
  err = MICOAddNotification( mico_notify_EASYLINK_GET_EXTRA_DATA, (void *)AirkissNotify_AirkissGetExtraDataHandler );
  require_noerr(err, exit);
  
  // Start the Airkiss thread
  mico_rtos_init_semaphore(&airkiss_sem, 1);
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "AIRKISS", airkiss_thread, 0x800, NULL );
  require_noerr_action( err, exit, wifi_airkiss_log("ERROR: Unable to start the Airkiss thread.") );
 
  return err;
  
exit:
  wifi_airkiss_log("ERROR, err: %d", err);
  return err;
}



