/**
******************************************************************************
* @file    wifi_softap.c 
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

#define wifi_softap_log(M, ...) custom_log("WIFI", M, ##__VA_ARGS__)

static char *ap_ssid = "mxchip_test";
static char *ap_key = "12345678";

static network_InitTypeDef_st wNetConfig;

void micoNotify_WifiStatusHandler(WiFiEvent event,  const int inContext)
{
  (void)inContext;
  switch (event) {
  case NOTIFY_AP_UP:
    wifi_softap_log("AP established");
    MicoRfLed(true);
    break;
  case NOTIFY_AP_DOWN:
    wifi_softap_log("AP deleted");
    MicoRfLed(false);
    break;
  default:
    break;
  }
  return;
}

int application_start( void )
{
  OSStatus err = kNoErr;
  
  MicoInit( );
  
  /*The notification message for the registered WiFi status change*/
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)micoNotify_WifiStatusHandler );
  require_noerr( err, exit ); 
  
  memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));
  
  strcpy((char*)wNetConfig.wifi_ssid, ap_ssid);
  strcpy((char*)wNetConfig.wifi_key, ap_key);
  
  wNetConfig.wifi_mode = Soft_AP;
  wNetConfig.dhcpMode = DHCP_Server;
  wNetConfig.wifi_retry_interval = 100;
  strcpy((char*)wNetConfig.local_ip_addr, "192.168.0.1");
  strcpy((char*)wNetConfig.net_mask, "255.255.255.0");
  strcpy((char*)wNetConfig.dnsServer_ip_addr, "192.168.0.1");
  micoWlanStart(&wNetConfig);
  
  wifi_softap_log("ssid:%s  key:%s", wNetConfig.wifi_ssid, wNetConfig.wifi_key);

exit:  
  return err;
}


