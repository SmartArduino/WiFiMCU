/**
******************************************************************************
* @file    MicoDriverUart.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide UART driver functions.
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


#include "MICORTOS.h"
#include "MICOPlatform.h"
#include <string.h>
#include "fsl_uart_driver.h"
#include "fsl_uart_common.h"
#include "fsl_edma_driver.h"
#include "fsl_uart_edma_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_interrupt_manager.h"

#include "board.h"
#include "PlatformLogging.h"
/******************************************************
*                    Constants
******************************************************/


const uint32_t uartInstance[] ={
    [MICO_UART_1] = BOARD_DEBUG_UART_INSTANCE,
    [MICO_UART_2] = BOARD_APP_UART_INSTANCE,
};
/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/
#define RING_BUFF_ON 0

#define UART_IRQ_APP 



/******************************************************
*                    Structures
******************************************************/

typedef struct
{
  uint32_t            rx_size;
  ring_buffer_t*      rx_buffer;
#ifndef NO_MICO_RTOS
  mico_semaphore_t    rx_complete;
  mico_semaphore_t    tx_complete;
  mico_mutex_t        tx_mutex;
#else
  volatile bool       rx_complete;
  volatile bool       tx_complete;
#endif
  mico_semaphore_t    sem_wakeup;
  OSStatus            tx_dma_result;
  OSStatus            rx_dma_result;
} uart_interface_t;

/******************************************************
*               Variables Definitions
******************************************************/

static uart_interface_t uart_interfaces[NUMBER_OF_UART_INTERFACES];



#ifdef UART_IRQ_APP
static uart_user_config_t uartConfig;
static uart_state_t uartState;
#else
static edma_state_t state_app; 
static edma_user_config_t userConfig_app; 
static uart_edma_state_t uartStateEdma_app;    
static uart_edma_user_config_t uartConfig_app;
#endif

#ifndef NO_MICO_RTOS
static mico_uart_t current_uart;
#endif

/******************************************************
*               Function Declarations
******************************************************/

static OSStatus internal_uart_init ( mico_uart_t uart, const mico_uart_config_t* config, ring_buffer_t* optional_rx_buffer );
static OSStatus platform_uart_receive_bytes( mico_uart_t uart, void* data, uint32_t size, uint32_t timeout );


/* Interrupt service functions - called from interrupt vector table */
#ifndef NO_MICO_RTOS
static void thread_wakeup(void *arg);
static void RX_PIN_WAKEUP_handler(void *arg);
#endif


/******************************************************
*               Function Definitions
******************************************************/
uint32_t getInstanceBy(mico_uart_t uart){
    return uartInstance[uart];
}

mico_uart_t getUartBy(uint32_t instance){
    switch(instance){
        case BOARD_DEBUG_UART_INSTANCE:
            return  MICO_UART_1;
        case BOARD_APP_UART_INSTANCE:
            return  MICO_UART_2;
        default:
            return MICO_UART_1;
    }
}

OSStatus MicoUartInitialize( mico_uart_t uart, const mico_uart_config_t* config, ring_buffer_t* optional_rx_buffer )
{
  return internal_uart_init(uart, config, optional_rx_buffer);
  
}


OSStatus MicoStdioUartInitialize( const mico_uart_config_t* config, ring_buffer_t* optional_rx_buffer )
{
  return internal_uart_init(STDIO_UART, config, optional_rx_buffer);
}


OSStatus internal_uart_init( mico_uart_t uart, const mico_uart_config_t* config, ring_buffer_t* optional_rx_buffer )
{
    uint32_t Instance; //fsl only
#ifndef NO_MICO_RTOS
  mico_rtos_init_semaphore(&uart_interfaces[uart].tx_complete, 1);
  mico_rtos_init_semaphore(&uart_interfaces[uart].rx_complete, 1);
#else
  uart_interfaces[uart].tx_complete = false;
  uart_interfaces[uart].rx_complete = false;
#endif  
  MicoMcuPowerSaveConfig(false); 
    Instance = getInstanceBy(uart);
    /* Configure the UART TX/RX pins */
    configure_uart_pins(Instance); //BOARD_APP_UART_INSTANCE);
#ifndef NO_MICO_RTOS
  if(config->flags & UART_WAKEUP_ENABLE){
    current_uart = uart;
    mico_rtos_init_semaphore( &uart_interfaces[uart].sem_wakeup, 1 );
    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "UART_WAKEUP", thread_wakeup, 0x100, &current_uart);
  }
#endif 
	//OSA_Init();
#ifdef UART_IRQ_APP    
    /****************************************************************/
    uartConfig.baudRate = 115200;
    uartConfig.bitCountPerChar = kUart8BitsPerChar;
    uartConfig.parityMode = kUartParityDisabled;
    uartConfig.stopBitCount = kUartOneStopBit;
    /***************************************************************/
    UART_DRV_Init(Instance, &uartState, &uartConfig);
#else
    userConfig_app.chnArbitration = kEDMAChnArbitrationRoundrobin;
    userConfig_app.notHaltOnError = false;

    uartConfig_app.bitCountPerChar = kUart8BitsPerChar;
    uartConfig_app.parityMode = kUartParityDisabled;
    uartConfig_app.stopBitCount = kUartOneStopBit;
    uartConfig_app.baudRate = 115200;

    EDMA_DRV_Init(&state_app, &userConfig_app);    
    UART_DRV_EdmaInit(Instance, &uartStateEdma_app, &uartConfig_app); 
#endif
#if RING_BUFF_ON 
  if (optional_rx_buffer != NULL)
  {
     //  Note that the ring_buffer should've been initialised first
    uart_interfaces[uart].rx_buffer = optional_rx_buffer;
    uart_interfaces[uart].rx_size   = 0;
    platform_uart_receive_bytes( uart, optional_rx_buffer->buffer, optional_rx_buffer->size, 0 );
  }  
#endif
  MicoMcuPowerSaveConfig(true);  
  return kNoErr;
}

OSStatus MicoUartFinalize( mico_uart_t uart )
{
    uint32_t  Instance = getInstanceBy(uart);
    // uart = MICO_UART_1; //test
#ifdef  UART_IRQ_APP 
    UART_DRV_Deinit(Instance);
#else     
    UART_DRV_EdmaDeinit(Instance);    
    EDMA_DRV_Deinit();   
    INT_SYS_DisableIRQ(g_uartRxTxIrqId[Instance]);
#endif
#ifndef NO_MICO_RTOS
  mico_rtos_deinit_semaphore(&uart_interfaces[uart].rx_complete);
  mico_rtos_deinit_semaphore(&uart_interfaces[uart].tx_complete);
#endif  
  
  return kNoErr;
}

void UART_DRV_CompleteSendData(uint32_t instance)
{
   // assert(instance < HW_UART_UART_APP_INDEX_COUNT);
    mico_uart_t uart = getUartBy(instance);

    uint32_t baseAddr = g_uartBaseAddr[instance];
    uart_state_t * uartState = (uart_state_t *)g_uartStatePtr[instance];
    /* Disable the transmitter data register empty interrupt */
    UART_HAL_SetTxDataRegEmptyIntCmd(baseAddr, false);

    /* Signal the synchronous completion object. */
    if (uartState->isTxBlocking)
    {
        OSA_SemaPost(&uartState->txIrqSync);
        mico_rtos_set_semaphore(&uart_interfaces[uart].tx_complete);
    }

    /* Update the information of the module driver state */
    uartState->isTxBusy = false; 
}

OSStatus MicoUartSend( mico_uart_t uart, const void* data, uint32_t size )
{
//  /* Reset DMA transmission result. The result is assigned in interrupt handler */
    uint32_t  Instance = getInstanceBy(uart);
   uart_interfaces[uart].tx_dma_result = kGeneralErr;
#ifndef NO_MICO_RTOS
  mico_rtos_lock_mutex(&uart_interfaces[uart].tx_mutex);
#endif  
  MicoMcuPowerSaveConfig(false);  
#ifdef  UART_IRQ_APP
  if (UART_DRV_SendData(Instance, data, size) == kStatus_UART_Success){
#else 
  if (UART_DRV_EdmaSendData(Instance, data, size) == kStatus_UART_Success){
#endif
    
        #ifndef NO_MICO_RTOS
            mico_rtos_set_semaphore( &uart_interfaces[ uart ].tx_complete );
        #else
            uart_interfaces[ uart ].rx_complete = true;
        #endif
  }  
#ifndef NO_MICO_RTOS
  mico_rtos_get_semaphore( &uart_interfaces[ uart ].tx_complete, MICO_NEVER_TIMEOUT );
#else 
  while(uart_interfaces[ uart ].tx_complete == false);
  uart_interfaces[ uart ].tx_complete = false;
#endif
//  return uart_interfaces[uart].tx_dma_result; 
  MicoMcuPowerSaveConfig(true);
#ifndef NO_MICO_RTOS
  mico_rtos_unlock_mutex(&uart_interfaces[uart].tx_mutex);
#endif

  return kNoErr;
}

void UART_DRV_CompleteReceiveData(uint32_t instance)
{
    assert(instance < HW_UART_INSTANCE_COUNT);
    uart_state_t * uartState = (uart_state_t *)g_uartStatePtr[instance];
    uint32_t baseAddr = g_uartBaseAddr[instance];
    mico_uart_t uart = getUartBy(instance);
    
    /* Disable receive data full interrupt */
    UART_HAL_SetRxDataRegFullIntCmd(baseAddr, false);

    /* Signal the synchronous completion object. */
    if (uartState->isRxBlocking)
    {
        mico_rtos_set_semaphore(&uart_interfaces[uart].rx_complete); //OSA_SemaPost(&uartState->rxIrqSync);
    }

    /* Update the information of the module driver state */
    uartState->isRxBusy = false;
}

uart_status_t UART_DRV_ReceiveDataBlocking(uint32_t instance, uint8_t * rxBuff,
                                           uint32_t rxSize, uint32_t timeout)
{
    assert(rxBuff);
    assert(instance < HW_UART_INSTANCE_COUNT);
    
    mico_uart_t uart = getUartBy(instance);
    uart_state_t * uartState = (uart_state_t *)g_uartStatePtr[instance];
    uart_status_t error = kStatus_UART_Success;
    uint32_t baseAddr = g_uartBaseAddr[instance];
    OSStatus Status;
    
    /* Indicates current transaction is blocking.*/
    uartState->isRxBlocking = true;

    if (uartState->isRxBusy)
    {
        return kStatus_UART_RxBusy;
    }
    uartState->rxBuff = rxBuff;
    uartState->rxSize = rxSize;
    uartState->isRxBusy = true;
    /* enable the receive data full interrupt */
    UART_HAL_SetRxDataRegFullIntCmd(baseAddr, true);

    /* Wait until all the data is received or for timeout.*/
    Status = mico_rtos_get_semaphore( &uart_interfaces[uart].rx_complete, timeout );

    if (Status != kNoErr)
    {
        /* Abort the transfer so it doesn't continue unexpectedly.*/
        UART_DRV_AbortReceivingData(instance);
        error = kStatus_UART_Timeout; // kGeneralErr;
    }
    return error;
}

OSStatus MicoUartRecv( mico_uart_t uart, void* data, uint32_t size, uint32_t timeout )
{
#if RING_BUFF_ON
  if (uart_interfaces[uart].rx_buffer != NULL)
  {
    while (size != 0)
    {
      uint32_t transfer_size = MIN(uart_interfaces[uart].rx_buffer->size / 2, size);
      
      /* Check if ring buffer already contains the required amount of data. */
      if ( transfer_size > ring_buffer_used_space( uart_interfaces[uart].rx_buffer ) )
      {
        /* Set rx_size and wait in rx_complete semaphore until data reaches rx_size or timeout occurs */
        uart_interfaces[uart].rx_size = transfer_size;
#ifndef NO_MICO_RTOS
        if ( mico_rtos_get_semaphore( &uart_interfaces[uart].rx_complete, timeout) != kNoErr )
        {
          uart_interfaces[uart].rx_size = 0;
          return kTimeoutErr;
        }
#else
        uart_interfaces[uart].rx_complete = false;
        int delay_start = mico_get_time_no_os();
        while(uart_interfaces[uart].rx_complete == false){
          if(mico_get_time_no_os() >= delay_start + timeout && timeout != MICO_NEVER_TIMEOUT){
            uart_interfaces[uart].rx_size = 0;
            return kTimeoutErr;
          }
        }
#endif
        /* Reset rx_size to prevent semaphore being set while nothing waits for the data */
        uart_interfaces[uart].rx_size = 0;
      }
     
      size -= transfer_size;
      
      // Grab data from the buffer
      do
      {
        uint8_t* available_data;
        uint32_t bytes_available;
         //platform_log("uart receive 03"); 
        ring_buffer_get_data( uart_interfaces[uart].rx_buffer, &available_data, &bytes_available );
        bytes_available = MIN( bytes_available, transfer_size );
        memcpy( data, available_data, bytes_available );
        transfer_size -= bytes_available;
        data = ( (uint8_t*) data + bytes_available );
        ring_buffer_consume( uart_interfaces[uart].rx_buffer, bytes_available );
      } while ( transfer_size != 0 );
    }
    
    if ( size != 0 )
    {
      return kGeneralErr;
    }
    else
    {
      return kNoErr;
    }
  }
  else
  {
    return platform_uart_receive_bytes( uart, data, size, timeout );
  }
#else
    return platform_uart_receive_bytes( uart, data, size, timeout );
#endif 
 // return kNoErr;
}


static OSStatus platform_uart_receive_bytes( mico_uart_t uart, void* data, uint32_t size, uint32_t timeout )
{
    uint32_t  Instance = getInstanceBy(uart);
    uart_status_t retVal = kStatus_UART_Success;
  /* Reset DMA transmission result. The result is assigned in interrupt handler */
   uart_interfaces[uart].rx_dma_result = kGeneralErr;
#ifdef UART_IRQ_APP   
   retVal = UART_DRV_ReceiveDataBlocking(Instance, data, size, timeout);
  // if(UART_DRV_ReceiveData(BOARD_APP_UART_INSTANCE, data,size )==kStatus_UART_Success){  
#else   
  //  if(UART_DRV_EdmaReceiveData(BOARD_DEBUG_UART_INSTANCE, data, size)==kStatus_UART_Success){
   retVal = UART_DRV_EdmaReceiveDataBlocking(Instance, data, size, timeout);//
#endif
   if(retVal == kStatus_UART_Success) { 
        #ifndef NO_MICO_RTOS
            mico_rtos_set_semaphore( &uart_interfaces[uart].rx_complete );
        #else
            uart_interfaces[uart ].rx_complete = true;
        #endif
  return kNoErr;
    }
  if ( timeout > 0 )
   {
#ifndef NO_MICO_RTOS
    mico_rtos_get_semaphore( &uart_interfaces[uart].rx_complete, timeout );
#else
    uart_interfaces[uart].rx_complete = false;
    int delay_start = mico_get_time_no_os();
    while(uart_interfaces[uart].rx_complete == false){
      if(mico_get_time_no_os() >= delay_start + timeout && timeout != MICO_NEVER_TIMEOUT){
        break;
      }
    }    
#endif
    return uart_interfaces[uart].rx_dma_result;
  }   
  return kGeneralErr;              // kNoErr;
}


uint32_t MicoUartGetLengthInBuffer( mico_uart_t uart )
{
#if RING_BUFF_ON
  return ring_buffer_used_space( uart_interfaces[uart].rx_buffer );
#else
  uart_state_t * uState = &uartState;
  uint32_t len = 0;
   len =  uState->pRxSize;
   if(len != 0){
       uState->pRxSize = 0;
   }
  return len;
  // return 0; //test
#endif
}

#ifndef NO_MICO_RTOS
static void thread_wakeup(void *arg)
{
  mico_uart_t uart = *(mico_uart_t *)arg;
  while(1){
//     if(mico_rtos_get_semaphore(&uart_interfaces[ uart ].sem_wakeup, 1000) != kNoErr){
//      gpio_irq_enable(uart_mapping[uart].pin_rx->bank, uart_mapping[uart].pin_rx->number, IRQ_TRIGGER_FALLING_EDGE, RX_PIN_WAKEUP_handler, &uart);
//      MicoMcuPowerSaveConfig(true);
//    }   
  }
}
#endif

/******************************************************
*            Interrupt Service Routines
******************************************************/
#ifndef NO_MICO_RTOS
void RX_PIN_WAKEUP_handler(void *arg)
{
  (void)arg;
  mico_uart_t uart = *(mico_uart_t *)arg;
  
//  RCC_AHB1PeriphClockCmd(uart_mapping[ uart ].pin_rx->peripheral_clock, ENABLE);
//  uart_mapping[ uart ].usart_peripheral_clock_func ( uart_mapping[uart].usart_peripheral_clock,  ENABLE );
//  uart_mapping[uart].rx_dma_peripheral_clock_func  ( uart_mapping[uart].rx_dma_peripheral_clock, ENABLE );
//  
//  gpio_irq_disable(uart_mapping[uart].pin_rx->bank, uart_mapping[uart].pin_rx->number); 
  
  MicoMcuPowerSaveConfig(false);
  mico_rtos_set_semaphore(&uart_interfaces[uart].sem_wakeup);
}
#endif

// end file






