/**
  ******************************************************************************
  * @file    MICOAppEntrance.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   Mico application entrance, addd user application functons and threads.
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

#include "MICODefine.h"
#include "MICOAppDefine.h"

#include "StringUtils.h"
#include "SppProtocol.h"
#include "MicoPlatform.h"
#include "platform_config.h"
#include "cfunctions.h"
#include "cppfunctions.h"

#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define app_log_trace() custom_log_trace("APP")

volatile ring_buffer_t  rx_buffer;
volatile uint8_t        rx_data[UART_BUFFER_LENGTH];

/* MICO system callback: Restore default configuration provided by application */
void appRestoreDefault_callback(mico_Context_t *inContext)
{
  inContext->flashContentInRam.appConfig.configDataVer = CONFIGURATION_VERSION;
  inContext->flashContentInRam.appConfig.localServerPort = LOCAL_PORT;
  inContext->flashContentInRam.appConfig.localServerEnable = true;
  inContext->flashContentInRam.appConfig.USART_BaudRate = 115200;
  inContext->flashContentInRam.appConfig.remoteServerEnable = true;
  sprintf(inContext->flashContentInRam.appConfig.remoteServerDomain, DEAFULT_REMOTE_SERVER);
  inContext->flashContentInRam.appConfig.remoteServerPort = DEFAULT_REMOTE_SERVER_PORT;
}

OSStatus MICOStartApplication( mico_Context_t * const inContext )
{
  app_log_trace();
  OSStatus err = kNoErr;
  mico_uart_config_t uart_config;
  
  require_action(inContext, exit, err = kParamErr);

#ifdef MICO_C_CPP_MIXING_DEMO  
  cpp_main();
#endif

  MicoGpioInitialize(MICO_GPIO_17,OUTPUT_PUSH_PULL);
  MicoGpioOutputHigh( MICO_GPIO_17);
  
  sppProtocolInit( inContext );

  /*Bonjour for service searching*/
  if(inContext->flashContentInRam.micoSystemConfig.bonjourEnable == true)
    MICOStartBonjourService( Station, inContext );

  /*UART receive thread*/
  uart_config.baud_rate    = inContext->flashContentInRam.appConfig.USART_BaudRate;
  uart_config.data_width   = DATA_WIDTH_8BIT;
  uart_config.parity       = NO_PARITY;
  uart_config.stop_bits    = STOP_BITS_1;
  uart_config.flow_control = FLOW_CONTROL_DISABLED;
  if(inContext->flashContentInRam.micoSystemConfig.mcuPowerSaveEnable == true)
    uart_config.flags = UART_WAKEUP_ENABLE;
  else
    uart_config.flags = UART_WAKEUP_DISABLE;
  ring_buffer_init  ( (ring_buffer_t *)&rx_buffer, (uint8_t *)rx_data, UART_BUFFER_LENGTH );
  MicoUartInitialize( UART_FOR_APP, &uart_config, (ring_buffer_t *)&rx_buffer );
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "UART Recv", uartRecv_thread, STACK_SIZE_UART_RECV_THREAD, (void*)inContext );
  require_noerr_action( err, exit, app_log("ERROR: Unable to start the uart recv thread.") );

 /*Local TCP server thread*/
 if(inContext->flashContentInRam.appConfig.localServerEnable == true){
   err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Local Server", localTcpServer_thread, STACK_SIZE_LOCAL_TCP_SERVER_THREAD, (void*)inContext );
   require_noerr_action( err, exit, app_log("ERROR: Unable to start the local server thread.") );
 }

  /*Remote TCP client thread*/
 if(inContext->flashContentInRam.appConfig.remoteServerEnable == true){
   err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Remote Client", remoteTcpClient_thread, STACK_SIZE_REMOTE_TCP_CLIENT_THREAD, (void*)inContext );
   require_noerr_action( err, exit, app_log("ERROR: Unable to start the remote client thread.") );
 }

exit:
  return err;
}




