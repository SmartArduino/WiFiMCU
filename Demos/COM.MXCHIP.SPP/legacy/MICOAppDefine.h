/**
  ******************************************************************************
  * @file    MICOAppDefine.h 
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


#ifndef __MICOAPPDEFINE_H
#define __MICOAPPDEFINE_H

#include "Common.h"

#define APP_INFO   "mxchipWNet SPP Demo based on MICO OS"

#define FIRMWARE_REVISION   "MICO_SPP_2_3"
#define MANUFACTURER        "MXCHIP Inc."
#define SERIAL_NUMBER       "20140606"
#define PROTOCOL            "com.mxchip.spp"

/* Wi-Fi configuration mode */
#define MICO_CONFIG_MODE CONFIG_MODE_EASYLINK_WITH_SOFTAP

/*User provided configurations*/
#define CONFIGURATION_VERSION               0x00000002 // if default configuration is changed, update this number
#define MAX_Local_Client_Num                8
#define LOCAL_PORT                          8080
#define DEAFULT_REMOTE_SERVER               "192.168.2.254"
#define DEFAULT_REMOTE_SERVER_PORT          8080
#define UART_RECV_TIMEOUT                   500
#define UART_ONE_PACKAGE_LENGTH             1024
#define wlanBufferLen                       1024
#define UART_BUFFER_LENGTH                  2048

#define LOCAL_TCP_SERVER_LOOPBACK_PORT      1000
#define REMOTE_TCP_CLIENT_LOOPBACK_PORT     1002
#define RECVED_UART_DATA_LOOPBACK_PORT      1003

#define BONJOUR_SERVICE                     "_easylink._tcp.local."

/* Define thread stack size */
#ifdef DEBUG
  #define STACK_SIZE_UART_RECV_THREAD           0x2A0
  #define STACK_SIZE_LOCAL_TCP_SERVER_THREAD    0x300
  #define STACK_SIZE_LOCAL_TCP_CLIENT_THREAD    0x350
  #define STACK_SIZE_REMOTE_TCP_CLIENT_THREAD   0x500
#else
  #define STACK_SIZE_UART_RECV_THREAD           0x150
  #define STACK_SIZE_LOCAL_TCP_SERVER_THREAD    0x180
  #define STACK_SIZE_LOCAL_TCP_CLIENT_THREAD    0x200
  #define STACK_SIZE_REMOTE_TCP_CLIENT_THREAD   0x260
#endif

/*Application's configuration stores in flash*/
typedef struct
{
  uint32_t          configDataVer;
  uint32_t          localServerPort;

  /*local services*/
  bool              localServerEnable;
  bool              remoteServerEnable;
  char              remoteServerDomain[64];
  int               remoteServerPort;

  /*IO settings*/
  uint32_t          USART_BaudRate;
} application_config_t;

/*Running status*/
typedef struct _current_app_status_t {
  /*Local clients port list*/
  uint32_t          loopBack_PortList[MAX_Local_Client_Num];
  /*Remote TCP client connecte*/
  bool              isRemoteConnected;
} current_app_status_t;


void localTcpServer_thread(void *inContext);
void remoteTcpClient_thread(void *inContext);
void uartRecv_thread(void *inContext);

#endif

