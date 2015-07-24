/**
******************************************************************************
* @file    wifi_wps.c 
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

#define wifi_wps_log(M, ...) custom_log("WIFI", M, ##__VA_ARGS__)

static int is_wps_success;
static char ap_ssid[64], ap_key[32];
static mico_semaphore_t wps_sem;
static network_InitTypeDef_adv_st wNetConfigAdv;

void micoNotify_WifiStatusHandler(WiFiEvent event,  const int inContext)
{
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    wifi_wps_log("Station up");
    MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    wifi_wps_log("Station down");
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
  wifi_wps_log("Wlan Connection Err %d", err);
}

void WPSNotify_WPSCompleteHandler(network_InitTypeDef_st *nwkpara, const int inContext)
{
  OSStatus err;
  wifi_wps_log("WPS return");
  require_action(nwkpara, exit, err = kTimeoutErr);
  strcpy(ap_ssid, nwkpara->wifi_ssid);
  strcpy(ap_key, nwkpara->wifi_key);
  wifi_wps_log("Get SSID: %s, Key: %s", nwkpara->wifi_ssid, nwkpara->wifi_key);
  is_wps_success = 1;
  mico_rtos_set_semaphore(&wps_sem);
  return;
  
exit:
  wifi_wps_log("ERROR, err: %d", err); 
  mico_rtos_set_semaphore(&wps_sem);
}

void clean_wps_sesource( )
{
  MICORemoveNotification( mico_notify_EASYLINK_WPS_COMPLETED, (void *)WPSNotify_WPSCompleteHandler );
  micoWlanStopWPS();
  mico_rtos_deinit_semaphore(&wps_sem);
  wps_sem = NULL;
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
  
  wifi_wps_log("connect to %s...", wNetConfigAdv.ap_info.ssid);
}

void wps_thread(void *inContext)
{
  micoWlanStartWPS( 40 );
  wifi_wps_log("Start WPS configuration");
  mico_rtos_get_semaphore(&wps_sem, MICO_WAIT_FOREVER);
  
  if ( is_wps_success == 1 )
  {
    mico_thread_msleep(10);
    connect_ap( );
  } else {
    wifi_wps_log("WPS configuration fail");
  }
  msleep(20);
  clean_wps_sesource();
  mico_rtos_delete_thread(NULL);
}

int application_start( void )
{
  OSStatus err = kNoErr;
  is_wps_success = 0;

  MicoInit( );
  
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)micoNotify_WifiStatusHandler );
  require_noerr( err, exit ); 
  
  err = MICOAddNotification( mico_notify_WIFI_CONNECT_FAILED, (void *)micoNotify_ConnectFailedHandler );
  require_noerr( err, exit );
  
  err = MICOAddNotification( mico_notify_EASYLINK_WPS_COMPLETED, (void *)WPSNotify_WPSCompleteHandler );
  require_noerr(err, exit);
  
  // Start the WPS thread
  mico_rtos_init_semaphore(&wps_sem, 1);
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "WPS", wps_thread, 0x1000, NULL );
  require_noerr_action( err, exit, wifi_wps_log("ERROR: Unable to start the WPS thread.") );
  
  return err;
  
exit:
  wifi_wps_log("ERROR, err: %d", err);
  return err;
}



