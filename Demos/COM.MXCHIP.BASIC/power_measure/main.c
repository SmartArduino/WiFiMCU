/**
******************************************************************************
* @file    main.c 
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

#include "MiCO.h" 
#include "MiCONotificationCenter.h"

#define power_log(M, ...) custom_log("PM", M, ##__VA_ARGS__)

#define POWER_MEASURE_PROGRAM STANDBY_MODE
#define MCU_POWERSAVE_ENABLED 0
#define IEEE_POWERSAVE_ENABLED 1

#define RTOS_INITIALIZED          1
#define RTOS_FULL_CPU_THREAD      2
#define RTOS_FLASH_READ           3
#define RTOS_FLASH_ERASE          4
#define RTOS_FLASH_WRITE          5
#define RTOS_WLAN_INITIALIZED     6
#define RTOS_WLAN_SOFT_AP         7
#define RTOS_WLAN_EASYLINK        8
#define RTOS_WLAN_CONNECT         9
#define RTOS_WLAN_UDP_SEND       10
#define STANDBY_MODE             11


void micoNotify_WifiStatusHandler(WiFiEvent event,  const int inContext)
{
  switch (event) {
  case NOTIFY_STATION_UP:
    power_log("Station up");
    MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    power_log("Station down");
    MicoRfLed(false);
    break;
  default:
    break;
  }
  return;
}
    
#if POWER_MEASURE_PROGRAM == STANDBY_MODE
int application_start( void )
{
#if MCU_POWERSAVE_ENABLED
  MicoMcuPowerSaveConfig(true);
#endif
    
  power_log( "Power measure program: RTOS initialized and wait 5 seconds to standy" );

  mico_thread_sleep( 5 ); //Wait a period to avoid enter standby mode when boot
  
  power_log( "Enter standby mode..., and exit in 5 seconds" );
  
#ifdef EMW1088
  //MicoInit( );
  //micoWlanPowerOff( );
  //wlan_deepsleepps_on( );
#endif
  
#if IEEE_POWERSAVE_ENABLED
   micoWlanEnablePowerSave();
#endif
    
  MicoSystemStandBy( 5 );
  
  power_log( "Enter standby mode error!" );
  
  mico_rtos_delete_thread( NULL );
  return 0;
}
#endif

#if POWER_MEASURE_PROGRAM == RTOS_INITIALIZED
int application_start( void )
{
#if MCU_POWERSAVE_ENABLED
  MicoMcuPowerSaveConfig(true);
#endif
  
  power_log( "Power measure program: RTOS initialized and no application is running" );
  
  mico_rtos_delete_thread( NULL );
  return 0;
}
#endif

#if POWER_MEASURE_PROGRAM == RTOS_FULL_CPU_THREAD
int application_start( void )
{
#if MCU_POWERSAVE_ENABLED
  MicoMcuPowerSaveConfig(true);
#endif
  
  power_log( "Power measure program: RTOS initialized and application is running full speed" );
    
  while(1);
  return 0;
}
#endif

#if POWER_MEASURE_PROGRAM == RTOS_FLASH_ERASE
int application_start( void )
{
#if MCU_POWERSAVE_ENABLED
  MicoMcuPowerSaveConfig(true);
#endif
  power_log( "Power measure program: RTOS initialized and erase flash" );
  
  MicoFlashInitialize( MICO_FLASH_FOR_UPDATE );
  MicoFlashErase( MICO_FLASH_FOR_UPDATE, UPDATE_START_ADDRESS, UPDATE_END_ADDRESS );
  MicoFlashFinalize( MICO_FLASH_FOR_UPDATE );
  
  mico_rtos_delete_thread( NULL );

  return 0;
}
#endif

#if POWER_MEASURE_PROGRAM == RTOS_WLAN_INITIALIZED
int application_start( void )
{
#if MCU_POWERSAVE_ENABLED
  MicoMcuPowerSaveConfig(true);
#endif
  power_log( "Power measure program: RTOS and wlan initialized and wlan is initialized" );
  MicoInit( );
  
#if IEEE_POWERSAVE_ENABLED
   micoWlanEnablePowerSave();
#endif
    
  mico_rtos_delete_thread( NULL );
  return 0;
}
#endif

#if POWER_MEASURE_PROGRAM == RTOS_WLAN_SOFT_AP
int application_start( void )
{
  network_InitTypeDef_st wNetConfig;
#if MCU_POWERSAVE_ENABLED
  MicoMcuPowerSaveConfig(true);
#endif
  power_log( "Power measure program: RTOS and wlan initialized and setup soft ap" );
  MicoInit( );
  
  memset(&wNetConfig, 0, sizeof(network_InitTypeDef_st));
  wNetConfig.wifi_mode = Soft_AP;
  snprintf(wNetConfig.wifi_ssid, 32, "EasyLink_PM" );
  strcpy((char*)wNetConfig.wifi_key, "");
  strcpy((char*)wNetConfig.local_ip_addr, "10.10.10.1");
  strcpy((char*)wNetConfig.net_mask, "255.255.255.0");
  strcpy((char*)wNetConfig.gateway_ip_addr, "10.10.10.1");
  wNetConfig.dhcpMode = DHCP_Server;
  micoWlanStart(&wNetConfig);
    
  mico_rtos_delete_thread( NULL );
  return 0;
}
#endif

#if POWER_MEASURE_PROGRAM == RTOS_WLAN_EASYLINK
int application_start( void )
{
  network_InitTypeDef_st wNetConfig;
#if MCU_POWERSAVE_ENABLED
  MicoMcuPowerSaveConfig(true);
#endif
  power_log( "Power measure program: RTOS and wlan initialized and start easylink" );
  MicoInit( );
  
  micoWlanStartEasyLinkPlus( MICO_NEVER_TIMEOUT );
    
  mico_rtos_delete_thread( NULL );
  return 0;
}
#endif

#if POWER_MEASURE_PROGRAM == RTOS_WLAN_CONNECT
int application_start( void )
{
  MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)micoNotify_WifiStatusHandler );
  
  network_InitTypeDef_adv_st wNetConfig;
#if MCU_POWERSAVE_ENABLED
  MicoMcuPowerSaveConfig(true);
#endif
  power_log( "Power measure program: RTOS and wlan initialized and connect wlan, wait station up to measure" );
  MicoInit( );
  
#if IEEE_POWERSAVE_ENABLED
   micoWlanEnablePowerSave();
#endif
  
  memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_adv_st));
  
  strncpy((char*)wNetConfig.ap_info.ssid, "William Xu", 32);
  wNetConfig.ap_info.security = SECURITY_TYPE_AUTO;
  strncpy((char*)wNetConfig.key, "mx099555", 64);
  wNetConfig.key_len = 8;
  wNetConfig.dhcpMode = true;
  wNetConfig.wifi_retry_interval = 100;
  micoWlanStartAdv(&wNetConfig);
  power_log("connect to %s.....", wNetConfig.ap_info.ssid);
    
  mico_rtos_delete_thread( NULL );
  return 0;
}
#endif

#if POWER_MEASURE_PROGRAM == RTOS_WLAN_UDP_SEND
int application_start( void )
{
  network_InitTypeDef_adv_st wNetConfig;
  int udp_fd = -1;
  struct sockaddr_t addr;
  socklen_t addrLen;
  uint8_t *buf = NULL;
  
#if MCU_POWERSAVE_ENABLED
  MicoMcuPowerSaveConfig(true);
#endif
  power_log( "Power measure program: RTOS and wlan initialized and connect wlan, wait station up to measure" );
  MicoInit( );

#if IEEE_POWERSAVE_ENABLED
   micoWlanEnablePowerSave();
#endif
  memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_adv_st));
  
  strncpy((char*)wNetConfig.ap_info.ssid, "William Xu", 32);
  wNetConfig.ap_info.security = SECURITY_TYPE_AUTO;
  strncpy((char*)wNetConfig.key, "mx099555", 64);
  wNetConfig.key_len = 8;
  wNetConfig.dhcpMode = true;
  wNetConfig.wifi_retry_interval = 100;
  micoWlanStartAdv(&wNetConfig);
  power_log("connect to %s.....", wNetConfig.ap_info.ssid);
  
  buf = malloc(1024);
  
  udp_fd = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);;
  addr.s_port = 2000;
  addr.s_ip = INADDR_ANY;
  bind(udp_fd, &addr, sizeof(addr));
  
  addr.s_port = 2001;
  addr.s_ip = inet_addr( "192.168.2.1" );
  
  connect( udp_fd, &addr, sizeof(addr) );

  while(1) {
    send( udp_fd, buf, 1024, 0 );
    mico_thread_msleep( 10 );
  }
    
  mico_rtos_delete_thread( NULL );
  return 0;
}
#endif

