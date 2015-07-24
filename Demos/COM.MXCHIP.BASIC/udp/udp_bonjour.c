/**
******************************************************************************
* @file    udp_bonjour.c 
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

#include "MicoDefine.h"
#include "platform_config.h"
#include "MICONotificationCenter.h"

#include "MDNSUtils.h"
#include "StringUtils.h"

#define udp_bonjour_log(M, ...) custom_log("UDP", M, ##__VA_ARGS__)

static char *ap_ssid = "sqdmz";
static char *ap_key  = "0987654321";

static network_InitTypeDef_adv_st wNetConfigAdv;
static mico_semaphore_t udp_sem;

void micoNotify_WifiStatusHandler(WiFiEvent event,  const int inContext)
{
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    udp_bonjour_log("Station up");
    mico_rtos_set_semaphore(&udp_sem);
    MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    udp_bonjour_log("Station down");
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
  udp_bonjour_log("Wlan Connection Err %d", err);
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
  
  udp_bonjour_log("connect to %s...", wNetConfigAdv.ap_info.ssid);
}

void bonjour_server( WiFi_Interface interface )
{
  char *temp_txt= NULL;
  char *temp_txt2;
  net_para_st para;
  bonjour_init_t init;

  temp_txt = malloc(500);

  memset(&init, 0x0, sizeof(bonjour_init_t));

  micoWlanGetIPStatus(&para, Station);

  init.service_name = BONJOUR_SERVICE;

  /*   name#xxxxxx.local.  */
  snprintf( temp_txt, 100, "%s#%c%c%c%c%c%c.local.", BONJOURNANE, 
                                                     para.mac[7],   para.mac[8], \
                                                     para.mac[9],   para.mac[10], \
                                                     para.mac[11],  para.mac[12]  );
  init.host_name = (char*)__strdup(temp_txt);

  /*   name#xxxxxx.   */
  snprintf( temp_txt, 100, "%s#%c%c%c%c%c%c",        BONJOURNANE, 
                                                     para.mac[7],   para.mac[8], \
                                                     para.mac[9],   para.mac[10], \
                                                     para.mac[11],  para.mac[12]   );
  init.instance_name = (char*)__strdup(temp_txt);

  init.service_port = LOCAL_PORT;
  init.interface = interface;

  temp_txt2 = __strdup_trans_dot(para.mac);
  sprintf(temp_txt, "MAC=%s.", temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(FIRMWARE_REVISION);
  sprintf(temp_txt, "%sFirmware Rev=%s.", temp_txt, temp_txt2);
  free(temp_txt2);
  
  temp_txt2 = __strdup_trans_dot(HARDWARE_REVISION);
  sprintf(temp_txt, "%sHardware Rev=%s.", temp_txt, temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(MicoGetVer());
  sprintf(temp_txt, "%sMICO OS Rev=%s.", temp_txt, temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(MODEL);
  sprintf(temp_txt, "%sModel=%s.", temp_txt, temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(PROTOCOL);
  sprintf(temp_txt, "%sProtocol=%s.", temp_txt, temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(MANUFACTURER);
  sprintf(temp_txt, "%sManufacturer=%s.", temp_txt, temp_txt2);
  free(temp_txt2);
  
  init.txt_record = (char*)__strdup(temp_txt);

  bonjour_service_init(init);

  free(init.host_name);
  free(init.instance_name);
  free(init.txt_record);

  /*start bonjour server*/
  start_bonjour_service( );
 
  if(temp_txt) free(temp_txt);
  return;
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
  
  err = mico_rtos_init_semaphore(&udp_sem, 1);
  require_noerr( err, exit ); 
  
  connect_ap( );
  
  mico_rtos_get_semaphore(&udp_sem, MICO_WAIT_FOREVER);
  
  /*registered bonjour server*/
  bonjour_server( Station );
 
  return err;

exit:
  udp_bonjour_log("ERROR, err: %d", err);
  return err;
}

