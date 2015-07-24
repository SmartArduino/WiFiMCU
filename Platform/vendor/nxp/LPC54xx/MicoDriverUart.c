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

#include "platform.h"
#include "platform_common_config.h"

#include "gpio_irq.h"

#include "chip.h"
#include "board.h"
#include "string.h"

/******************************************************
*                    Constants
******************************************************/
/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, rxring;

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/
/* Ring buffer size */
#define UART_RB_SIZE 128

#define LPC_USART       LPC_USART0
#define LPC_IRQNUM      UART0_IRQn
#define LPC_UARTHNDLR   UART0_IRQHandler

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RB_SIZE], txbuff[UART_RB_SIZE];

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

extern void uart0_int_handler(void);

void UART0_IRQHandler(void)
{
  /* Want to handle any errors? Do it here. */
  uint32_t temp;
  temp = Chip_UART_GetStatus(LPC_USART);
  /* Use default ring buffer handler. Override this with your own
     code if you need more capability. */
  if ((temp & UART_STAT_TXRDY) != 0) {

  }
/*  while((Chip_UART_GetStatus(LPC_USART)&UART_STAT_RXIDLE) != 0) {
  }*/
//  uart0_int_handler();
  /* Handle receive interrupt */
  if ((temp & UART_STAT_RXRDY) != 0) {
 //   uart0_int_handler();
  }  
  if ((temp & UART_STAT_START) != 0) {
    uart0_int_handler();
  }    
//  Chip_UART_ClearStatus(LPC_USART, temp);
}

void uart0_int_handler(void)
{
  // Update tail
  uint32_t temp;
  
  temp = (LPC_DMA->DMACH[DMAREQ_UART0_RX].XFERCFG>>16)&0x3FF;
  
  uart_interfaces[ 0 ].rx_buffer->tail = uart_interfaces[ 0 ].rx_buffer->size - temp;
  
  // Notify thread if sufficient data are available
  if ( ( uart_interfaces[ 0 ].rx_size > 0 ) &&
      ( ring_buffer_used_space( uart_interfaces[ 0 ].rx_buffer ) >= uart_interfaces[0].rx_size ) )
  {
#ifndef NO_MICO_RTOS
    mico_rtos_set_semaphore( &uart_interfaces[ 0 ].rx_complete );
#else
    uart_interfaces[ 0 ].rx_complete = true;
#endif
    uart_interfaces[ 0 ].rx_size = 0;
  }
  
#ifndef NO_MICO_RTOS
  if(uart_interfaces[ 0 ].sem_wakeup)
    mico_rtos_set_semaphore(&uart_interfaces[ 0 ].sem_wakeup);
#endif
}

void uart0_dma_rx_handler(void)
{
  uart_interfaces[ 0 ].rx_dma_result = kNoErr;
  
#ifndef NO_MICO_RTOS
  mico_rtos_set_semaphore( &uart_interfaces[ 0 ].rx_complete );
#else
  uart_interfaces[ 0 ].rx_complete = true;
#endif
}

void uart0_dma_tx_handler(void)
{
  bool tx_complete = false;

  uart_interfaces[ 0 ].tx_dma_result = kNoErr;
  tx_complete = true;
  
//   /* TX DMA error */
//  if ( tx_complete == false )
//  {
//    uart_interfaces[0].tx_dma_result = kGeneralErr;
//  }

#ifndef NO_MICO_RTOS
  /* Set semaphore regardless of result to prevent waiting thread from locking up */
  mico_rtos_set_semaphore( &uart_interfaces[ 0 ].tx_complete);
#else
  uart_interfaces[ 0 ].tx_complete = true;
#endif
}

/******************************************************
*               Function Definitions
******************************************************/

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
#ifndef NO_MICO_RTOS
  mico_rtos_init_semaphore(&uart_interfaces[uart].tx_complete, 1);
  mico_rtos_init_semaphore(&uart_interfaces[uart].rx_complete, 1);
#else
  uart_interfaces[uart].tx_complete = false;
  uart_interfaces[uart].rx_complete = false;
#endif
  
  MicoMcuPowerSaveConfig(false);

  /* Enable GPIO peripheral clocks for TX and RX pins */
  Chip_SYSCTL_Enable_ASYNC_Syscon(true);
  Chip_Clock_SetAsyncSysconClockDiv(1);	/* divided by 1 */

  /* Configure USART TX Pin */
  /* Configure USART RX Pin */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 0, IOCON_MODE_INACT | IOCON_FUNC1 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 1, IOCON_MODE_INACT | IOCON_FUNC1 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF);
  
#ifndef NO_MICO_RTOS
  if(config->flags & UART_WAKEUP_ENABLE){
    current_uart = uart;
    mico_rtos_init_semaphore( &uart_interfaces[uart].sem_wakeup, 1 );
    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "UART_WAKEUP", thread_wakeup, 0x100, &current_uart);
  }
#endif
  /* Enable UART peripheral clock */

  
  /**************************************************************************
  * Initialise LPC54000 UART0 registers
  * NOTE:
  * - Both transmitter and receiver are disabled until usart_enable_transmitter/receiver is called.
  * - Only 1 and 2 stop bits are implemented at the moment.
  **************************************************************************/
  /* Setup UART */
  /* Initialise USART peripheral */
  Chip_UART_Init(LPC_USART0);
  Chip_UART_ConfigData(LPC_USART0, UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
  Chip_UART_SetBaud(LPC_USART0, 115200);
 
  Chip_UART_Enable(LPC_USART);
  Chip_UART_TXEnable(LPC_USART);
  
  /* Before using the ring buffers, initialize them using the ring
     buffer init function */
//  RingBuffer_Init(&rxring, rxbuff, 1, UART_RB_SIZE);
//  RingBuffer_Init(&txring, txbuff, 1, UART_RB_SIZE);  
   /* Setup ring buffer */
                                           
  /* Enable receive data and line status interrupt */
  Chip_UART_IntEnable(LPC_USART, UART_INTEN_RXRDY);
  Chip_UART_IntDisable(LPC_USART, UART_INTEN_TXRDY);	/* May not be needed */  
    
  /* preemption = 1, sub-priority = 1 */
  NVIC_EnableIRQ(LPC_IRQNUM);
  
#ifndef UART_BUFFERMODE  
/* DMA initialization - enable DMA clocking and reset DMA if needed */
  Chip_DMA_Init(LPC_DMA);
/* Enable DMA controller and use driver provided DMA table for current descriptors */
  Chip_DMA_Enable(LPC_DMA);
  Chip_DMA_SetSRAMBase(LPC_DMA, DMA_ADDR(Chip_DMA_Table));
  Chip_DMA_SetValidChannel(LPC_DMA, DMAREQ_UART0_TX);
  Chip_DMA_SetValidChannel(LPC_DMA, DMAREQ_UART0_RX);
  
  Chip_UART_ClearStatus(LPC_USART0, UART_STAT_OVERRUNINT|UART_STAT_DELTARXBRK|UART_STAT_FRM_ERRINT|UART_STAT_PAR_ERRINT|UART_STAT_RXNOISEINT);
/*
  Chip_DMA_InitChannel( DMAREQ_UART0_RX, DMA_ADDR(&LPC_USART0->RXDATA), DMA_XFERCFG_SRCINC_0, 
                        DMA_ADDR(NULL), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                        0, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(3)));
  Chip_DMA_StartTransfer(DMAREQ_UART0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, 0);
  
  Chip_UART_ClearStatus(LPC_USART0, UART_STAT_OVERRUNINT|UART_STAT_DELTARXBRK|UART_STAT_FRM_ERRINT|UART_STAT_PAR_ERRINT|UART_STAT_RXNOISEINT);
  
  Chip_DMA_InitChannel( DMAREQ_UART0_TX, DMA_ADDR(NULL), DMA_XFERCFG_SRCINC_1,
                        DMA_ADDR(&LPC_USART0->TXDATA), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                        0, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(3)));
  Chip_DMA_StartTransfer(DMAREQ_UART0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, 0);
*/    
  if (optional_rx_buffer != NULL)
  {
     /* Note that the ring_buffer should've been initialised first */
    uart_interfaces[uart].rx_buffer = optional_rx_buffer;
    uart_interfaces[uart].rx_size   = 0;
    platform_uart_receive_bytes( uart, optional_rx_buffer->buffer, optional_rx_buffer->size, 0 );
  }
   
//  LPC_SYSCON->AHBMATPRIO |= 0x03 << 8;
  /* Enable DMA interrupt */
//  NVIC_SetPriority(DMA_IRQn, 6);
  NVIC_EnableIRQ(DMA_IRQn);
#endif  
  MicoMcuPowerSaveConfig(true);
  printf("LPC54000 uart port init %d \r\n", uart);
  return kNoErr;
}

OSStatus MicoUartFinalize( mico_uart_t uart )
{
  MicoMcuPowerSaveConfig(false);
  
  Chip_UART_DeInit(LPC_USART0);

#ifndef NO_MICO_RTOS
  mico_rtos_deinit_semaphore(&uart_interfaces[uart].rx_complete);
  mico_rtos_deinit_semaphore(&uart_interfaces[uart].tx_complete);
#endif
  
  MicoMcuPowerSaveConfig(true);
  
  return kNoErr;
}

OSStatus MicoUartSend( mico_uart_t uart, const void* data, uint32_t size )
{
#ifdef UART_BUFFERMODE
#ifndef NO_MICO_RTOS
    mico_rtos_lock_mutex(&uart_interfaces[uart].tx_mutex);
#endif

  MicoMcuPowerSaveConfig(false);  
  Chip_UART_SendRB(LPC_USART0, &txring, data, size);
  MicoMcuPowerSaveConfig(true);
#ifndef NO_MICO_RTOS
    mico_rtos_unlock_mutex(&uart_interfaces[uart].tx_mutex);
#endif

  return kNoErr;
#else
  /* Reset DMA transmission result. The result is assigned in interrupt handler */
  uart_interfaces[uart].tx_dma_result = kGeneralErr;
#ifndef NO_MICO_RTOS
  mico_rtos_lock_mutex(&uart_interfaces[uart].tx_mutex);
#endif  
  MicoMcuPowerSaveConfig(false);  
  
//  Chip_UART_ClearStatus(LPC_USART0, UART_STAT_OVERRUNINT|UART_STAT_DELTARXBRK|UART_STAT_FRM_ERRINT|UART_STAT_PAR_ERRINT|UART_STAT_RXNOISEINT);
//
//  Chip_DMA_InitChannel( DMAREQ_UART0_RX, DMA_ADDR(&LPC_USART0->RXDATA), DMA_XFERCFG_SRCINC_0, 
//                        DMA_ADDR(data), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
//                        size, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(3)));
//  Chip_DMA_StartTransfer(DMAREQ_UART0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, size);
  
  Chip_UART_ClearStatus(LPC_USART0, UART_STAT_OVERRUNINT|UART_STAT_DELTARXBRK|UART_STAT_FRM_ERRINT|UART_STAT_PAR_ERRINT|UART_STAT_RXNOISEINT);
  
  Chip_DMA_InitChannel( DMAREQ_UART0_TX, DMA_ADDR(data), DMA_XFERCFG_SRCINC_1,
                        DMA_ADDR(&LPC_USART0->TXDATA), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                        size, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(3)));
  Chip_DMA_StartTransfer(DMAREQ_UART0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, size);

#ifndef NO_MICO_RTOS
  mico_rtos_get_semaphore( &uart_interfaces[ uart ].tx_complete, MICO_NEVER_TIMEOUT );
  mico_rtos_unlock_mutex(&uart_interfaces[uart].tx_mutex);

#else 
  while(uart_interfaces[ uart ].tx_complete == false);
  uart_interfaces[ uart ].tx_complete = false;
#endif
  return uart_interfaces[uart].tx_dma_result;
#endif
  
}

OSStatus MicoUartRecv( mico_uart_t uart, void* data, uint32_t size, uint32_t timeout )
{  
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
}


static OSStatus platform_uart_receive_bytes( mico_uart_t uart, void* data, uint32_t size, uint32_t timeout )
{
  /* Reset DMA transmission result. The result is assigned in interrupt handler */
  uart_interfaces[uart].rx_dma_result = kGeneralErr;
  
  Chip_UART_ClearStatus(LPC_USART0, UART_STAT_OVERRUNINT|UART_STAT_DELTARXBRK|UART_STAT_FRM_ERRINT|UART_STAT_PAR_ERRINT|UART_STAT_RXNOISEINT);

  Chip_DMA_InitChannel( DMAREQ_UART0_RX, DMA_ADDR(&LPC_USART0->RXDATA), DMA_XFERCFG_SRCINC_0, 
                        DMA_ADDR(data), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                        size, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(3)));
  Chip_DMA_StartTransfer(DMAREQ_UART0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, size);
  
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
  
  
  return kNoErr;
}


uint32_t MicoUartGetLengthInBuffer( mico_uart_t uart )
{
//  uint32_t temp;
//  temp = RingBuffer_GetCount(&rxring);
//  return temp; //ring_buffer_used_space( uart_interfaces[uart].rx_buffer );
  return ring_buffer_used_space( uart_interfaces[uart].rx_buffer );
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
  
  
  MicoMcuPowerSaveConfig(false);
  mico_rtos_set_semaphore(&uart_interfaces[uart].sem_wakeup);
}
#endif

// end file
