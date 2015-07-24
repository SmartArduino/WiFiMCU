/**
******************************************************************************
* @file    platform_uart.c 
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

#include "platform.h"
#include "platform_peripheral.h"

/******************************************************
*                    Constants
******************************************************/

#define BUART_FIFO_START        ((32-2-1)*1024)
#define BUART_RX_FIFO_SIZE      (1024*2)
#define BUART_TX_FIFO_SIZE      (1024)

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/

/******************************************************
*                    Structures
******************************************************/

/******************************************************
*               Variables Definitions
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/

/* Interrupt service functions - called from interrupt vector table */
void BuartInterrupt(void);
void FuartInterrupt(void);

/******************************************************
*               Function Definitions
******************************************************/

OSStatus platform_uart_init( platform_uart_driver_t* driver, const platform_uart_t* peripheral, const platform_uart_config_t* config, ring_buffer_t* optional_ring_buffer )
{
  OSStatus err = kNoErr;

  platform_mcu_powersave_disable();

  require_action_quiet( ( driver != NULL ) && ( peripheral != NULL ) && ( config != NULL ), exit, err = kParamErr);

  driver->rx_size              = 0;
  driver->tx_size              = 0;
  driver->last_transmit_result = kNoErr;
  driver->last_receive_result  = kNoErr;
  driver->peripheral           = (platform_uart_t*)peripheral;
  driver->buart_fifo_head      = 0;
#ifndef NO_MICO_RTOS
  mico_rtos_init_semaphore( &driver->tx_complete, 1 );
  mico_rtos_init_semaphore( &driver->rx_complete, 1 );
  mico_rtos_init_mutex    ( &driver->tx_mutex );
#else
  driver->tx_complete = false;
  driver->rx_complete = false;
#endif

  if ( peripheral->uart == FUART ){
    ClkModuleEn( FUART_CLK_EN );
    
    if( peripheral->pin_tx->port == GPIOA && peripheral->pin_tx->pin == 1 )
      GpioFuartTxIoConfig(0);
    else if( peripheral->pin_tx->port == GPIOB && peripheral->pin_tx->pin == 7 )
      GpioFuartTxIoConfig(1);
    else if( peripheral->pin_tx->port == GPIOC && peripheral->pin_tx->pin == 3 )
      GpioFuartTxIoConfig(2);
    else
      return kUnsupportedErr;

    if( peripheral->pin_rx->port == GPIOA && peripheral->pin_rx->pin == 1 )
      GpioFuartRxIoConfig(0);
    else if( peripheral->pin_rx->port == GPIOB && peripheral->pin_rx->pin == 6 )
      GpioFuartRxIoConfig(1);
    else if( peripheral->pin_rx->port == GPIOC && peripheral->pin_rx->pin == 4 )
      GpioFuartRxIoConfig(2);
    else
      return kUnsupportedErr;

    require_action( config->flow_control == FLOW_CONTROL_DISABLED, exit, err = kUnsupportedErr );

    err = FuartInit(config->baud_rate, config->data_width + 5, config->parity, config->stop_bits + 1);
    require_noerr(err, exit);

    FuartIOctl(UART_IOCTL_RXINT_SET, 1);
    
    if (optional_ring_buffer != NULL){
      /* Note that the ring_buffer should've been initialised first */
      driver->rx_buffer = optional_ring_buffer;
      driver->rx_size   = 0;
      //platform_uart_receive_bytes( uart, optional_rx_buffer->buffer, optional_rx_buffer->size, 0 );
    }

  }else if( peripheral->uart == BUART ){
    ClkModuleEn( BUART_CLK_EN );
    
    if( peripheral->pin_tx->port == GPIOA && peripheral->pin_tx->pin == 16 )
      GpioBuartTxIoConfig(0);
    else if( peripheral->pin_tx->port == GPIOA && peripheral->pin_tx->pin == 25 )
      GpioBuartTxIoConfig(1);
    else if( peripheral->pin_tx->port == GPIOB && peripheral->pin_tx->pin == 9 )
      GpioBuartTxIoConfig(2);
    else if( peripheral->pin_tx->port == GPIOB && peripheral->pin_tx->pin == 28 )
      GpioBuartTxIoConfig(3);
    else
      return kUnsupportedErr;

    if( peripheral->pin_rx->port == GPIOA && peripheral->pin_rx->pin == 13 )
      GpioBuartRxIoConfig(0);
    else if( peripheral->pin_rx->port == GPIOA && peripheral->pin_rx->pin == 24 )
      GpioBuartRxIoConfig(1);
    else if( peripheral->pin_rx->port == GPIOB && peripheral->pin_rx->pin == 8 )
      GpioBuartRxIoConfig(2);
    else if( peripheral->pin_rx->port == GPIOB && peripheral->pin_rx->pin == 29 )
      GpioBuartRxIoConfig(3);
    else
      return kUnsupportedErr;

    require_action( config->flow_control == FLOW_CONTROL_DISABLED, exit, err = kUnsupportedErr );

    err =  BuartExFifoInit( BUART_FIFO_START, BUART_RX_FIFO_SIZE, BUART_TX_FIFO_SIZE, 1 );
    require_noerr(err, exit);

    err = BuartInit( config->baud_rate, config->data_width + 5, config->parity, config->stop_bits + 1 );
    require_noerr(err, exit);
    
    BuartIOctl( UART_IOCTL_TXINT_SET, 1 );

  }else
    return kUnsupportedErr;

exit:
  return  err;
}

OSStatus platform_uart_deinit( platform_uart_driver_t* driver )
{
  return kNoErr;
}

OSStatus platform_uart_transmit_bytes( platform_uart_driver_t* driver, const uint8_t* data_out, uint32_t size )
{
  if(driver->peripheral->uart == FUART){
#ifndef NO_MICO_RTOS
  mico_rtos_lock_mutex(&driver->tx_mutex);
#endif
    FuartSend( (uint8_t *)data_out, size);
#ifndef NO_MICO_RTOS    
    mico_rtos_unlock_mutex(&driver->tx_mutex);
#endif
    return kNoErr;
  }else if(driver->peripheral->uart == BUART){
#ifndef NO_MICO_RTOS
  mico_rtos_lock_mutex(&driver->tx_mutex);
#endif
    BuartSend( (uint8_t *)data_out, size);
  }else {
    return kUnsupportedErr;
  }

#ifndef NO_MICO_RTOS
  mico_rtos_get_semaphore( &driver->tx_complete, MICO_NEVER_TIMEOUT );
  mico_rtos_unlock_mutex( &driver->tx_mutex );
#else 
  while( driver->tx_complete == false );
  driver->tx_complete = false;
#endif
  return kNoErr;

}

static OSStatus FUartRecv( platform_uart_driver_t* driver, void* data, uint32_t size, uint32_t timeout )
{

  if ( driver->rx_buffer != NULL )
  {
    while (size != 0)
    {
      uint32_t transfer_size = MIN( driver->rx_buffer->size/2, size );
      
      /* Check if ring buffer already contains the required amount of data. */
      if ( transfer_size > ring_buffer_used_space( driver->rx_buffer ) )
      {
        /* Set rx_size and wait in rx_complete semaphore until data reaches rx_size or timeout occurs */
        driver->rx_size = transfer_size;
        
#ifndef NO_MICO_RTOS
        if ( mico_rtos_get_semaphore( &driver->rx_complete, timeout) != kNoErr )
        {
          driver->rx_size = 0;
          return kTimeoutErr;
        }
#else
        driver->rx_complete = false;
        int delay_start = mico_get_time_no_os();
        while(driver->rx_complete == false){
          if(mico_get_time_no_os() >= delay_start + timeout && timeout != MICO_NEVER_TIMEOUT){
            driver->rx_size = 0;
            return kTimeoutErr;
          }
        }
#endif
        
        /* Reset rx_size to prevent semaphore being set while nothing waits for the data */
        driver->rx_size = 0;
      }
      
      size -= transfer_size;
      
      // Grab data from the buffer
      do
      {
        uint8_t* available_data;
        uint32_t bytes_available;
        
        ring_buffer_get_data( driver->rx_buffer, &available_data, &bytes_available );
        bytes_available = MIN( bytes_available, transfer_size );
        memcpy( data, available_data, bytes_available );
        transfer_size -= bytes_available;
        data = ( (uint8_t*) data + bytes_available );
        ring_buffer_consume( driver->rx_buffer, bytes_available );
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
    mico_thread_msleep(timeout);
    return kNoMemoryErr;
  }
}

static OSStatus BUartRecv( platform_uart_driver_t* driver, void* data, uint32_t size, uint32_t timeout )
{
  int next_trigger;
  uint32_t recved_data_len = (uint32_t)BuartIOctl(BUART_IOCTL_RXFIFO_DATLEN_GET, 0);
  uint32_t delay_start = 0;

  while (size != 0)
  {
    uint32_t transfer_size = MIN( BUART_RX_FIFO_SIZE / 2, size );
    
    /* Check if ring buffer already contains the required amount of data. */
    if ( transfer_size > recved_data_len )
    {
      /* Set rx_size and wait in rx_complete semaphore until data reaches rx_size or timeout occurs */
      driver->rx_size = transfer_size;
      next_trigger = (driver->buart_fifo_head + driver->rx_size - 1)% BUART_RX_FIFO_SIZE;

      /* */
      if( next_trigger < driver->buart_fifo_head && (driver->buart_fifo_head + recved_data_len) < BUART_RX_FIFO_SIZE ){
        BuartIOctl( BUART_IOCTL_RXFIFO_TRGR_DEPTH_SET, BUART_RX_FIFO_SIZE - 1 );
        BuartIOctl(UART_IOCTL_RXINT_CLR, 0);
        BuartIOctl(UART_IOCTL_RXINT_SET, 1);     
       
#ifndef NO_MICO_RTOS
#if 0
        if ( mico_rtos_get_semaphore( &driver->rx_complete, timeout) != kNoErr )
        {
          driver->rx_size = 0;
          BuartIOctl(UART_IOCTL_RXINT_SET, 0);
          return kTimeoutErr;
        }
#else
      delay_start = mico_get_time();
      while(1){
        mico_rtos_get_semaphore( &driver->rx_complete, 50);
        if( (BUART_RX_FIFO_SIZE - driver->buart_fifo_head)  <= (uint32_t)BuartIOctl(BUART_IOCTL_RXFIFO_DATLEN_GET, 0) )
          break;
        if(  mico_get_time() - delay_start > timeout ){
          driver->rx_size = 0;
          BuartIOctl(UART_IOCTL_RXINT_SET, 0);
          return kTimeoutErr;
        }
      }
#endif
        
#else
        driver->rx_complete = false;
        int delay_start = mico_get_time_no_os();
        while(driver->rx_complete == false){
          if(mico_get_time_no_os() >= delay_start + timeout && timeout != MICO_NEVER_TIMEOUT){
            driver->rx_size = 0;
            BuartIOctl(UART_IOCTL_RXINT_SET, 0);
            return kTimeoutErr;
          }
        }
#endif     
        mico_thread_msleep(20);
      }
      
#ifndef NO_MICO_RTOS
#if 1
      recved_data_len = (uint32_t)BuartIOctl(BUART_IOCTL_RXFIFO_DATLEN_GET, 0);
      if ( transfer_size > recved_data_len ){

        BuartIOctl(UART_IOCTL_RXINT_CLR, 0);
        BuartIOctl( BUART_IOCTL_RXFIFO_TRGR_DEPTH_SET, next_trigger );
        BuartIOctl(UART_IOCTL_RXINT_SET, 1);  
        
        if ( mico_rtos_get_semaphore( &driver->rx_complete, timeout) != kNoErr )
        { 
          driver->rx_size = 0;
          BuartIOctl(UART_IOCTL_RXINT_SET, 0);
          return kTimeoutErr;
        }

      }
      else{
         driver->rx_size = 0;
      }
#else
      //platform_log( "Waiting...,head:%d expext:%d trigger:%d", driver->buart_fifo_head, driver->rx_size, next_trigger );
      BuartIOctl(UART_IOCTL_RXINT_CLR, 0);
      BuartIOctl( BUART_IOCTL_RXFIFO_TRGR_DEPTH_SET, next_trigger );
      BuartIOctl(UART_IOCTL_RXINT_SET, 1);  
      
      delay_start = mico_get_time();
      while(1){
        mico_rtos_get_semaphore( &driver->rx_complete, 50);
        if( driver->rx_size <= (uint32_t)BuartIOctl(BUART_IOCTL_RXFIFO_DATLEN_GET, 0) )
          break;
        if(  mico_get_time() - delay_start > timeout ){
          driver->rx_size = 0;
          BuartIOctl(UART_IOCTL_RXINT_SET, 0);
          return kTimeoutErr;
        }
      }
#endif
#else
      BuartIOctl(UART_IOCTL_RXINT_CLR, 0);
      BuartIOctl( BUART_IOCTL_RXFIFO_TRGR_DEPTH_SET, next_trigger );
      BuartIOctl(UART_IOCTL_RXINT_SET, 1);  
      
      driver->rx_complete = false;
      delay_start = mico_get_time_no_os();
      while(driver->rx_complete == false){
        if(mico_get_time_no_os() >= delay_start + timeout && timeout != MICO_NEVER_TIMEOUT){
          driver->rx_size = 0;
          BuartIOctl(UART_IOCTL_RXINT_SET, 0);
          return kTimeoutErr;
        }
      }
#endif           
      /* Reset rx_size to prevent semaphore being set while nothing waits for the data */
      driver->rx_size = 0;
    }
  
    size -= transfer_size;
    
    // Grab data from the buffer
    do
    {
      uint32_t bytes_available;

      bytes_available = (uint32_t)BuartIOctl(BUART_IOCTL_RXFIFO_DATLEN_GET, 0);
      bytes_available = MIN( bytes_available, transfer_size );
      BuartRecv(data, bytes_available, 0);
      driver->buart_fifo_head = (driver->buart_fifo_head + bytes_available)% BUART_RX_FIFO_SIZE;
      transfer_size -= bytes_available;
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

OSStatus platform_uart_receive_bytes( platform_uart_driver_t* driver, uint8_t* data_in, uint32_t expected_data_size, uint32_t timeout_ms )
{
  if(driver->peripheral->uart == FUART)
    return FUartRecv( driver, data_in, expected_data_size, timeout_ms );
  else if(driver->peripheral->uart == BUART)
    return BUartRecv( driver, data_in, expected_data_size, timeout_ms );
  else
    return kUnsupportedErr;
}

OSStatus platform_uart_get_length_in_buffer( platform_uart_driver_t* driver )
{
  if( driver->peripheral->uart == FUART )
    return ring_buffer_used_space( driver->rx_buffer );
  else if( driver->peripheral->uart == BUART ){
    return BuartIOctl(BUART_IOCTL_RXFIFO_DATLEN_GET, 0);
  }
  return 0;
}

/******************************************************
*            Interrupt Service Routines
******************************************************/

void platform_fuart_irq( platform_uart_driver_t* driver )
{
  int status;
  uint8_t rxData;
  status = FuartIOctl(UART_IOCTL_RXSTAT_GET,0);

  if(status & 0x1E){
    /*
     * clear FIFO before clear other flags
     */
    FuartIOctl(UART_IOCTL_RXFIFO_CLR,0);
    /*
     * clear other error flags
     */
    FuartIOctl(UART_IOCTL_RXINT_CLR,0);
  }

  if(status & 0x01)
  { 
    //or,you can receive them in the interrupt directly
    while(FuartRecvByte(&rxData) > 0){
      ring_buffer_write( driver->rx_buffer, &rxData,1 );
    }

    FuartIOctl(UART_IOCTL_RXINT_CLR,0);

    // Notify thread if sufficient data are available
    if ( ( driver->rx_size > 0 ) &&
        ( ring_buffer_used_space( driver->rx_buffer ) >= driver->rx_size ) )
    {
  #ifndef NO_MICO_RTOS
      mico_rtos_set_semaphore( &driver->rx_complete );
  #else
      driver->rx_complete = true;
  #endif
      driver->rx_size = 0;
    }
  }

  if(FuartIOctl(UART_IOCTL_TXSTAT_GET,0) & 0x01)
  {
    FuartIOctl(UART_IOCTL_TXINT_CLR,0);
#ifndef NO_MICO_RTOS
    mico_rtos_set_semaphore( &driver->tx_complete );
#else
    driver->tx_complete = true;
#endif
  }
}

void platform_buart_irq( platform_uart_driver_t* driver )
{
  int status;
  status = BuartIOctl(UART_IOCTL_RXSTAT_GET,0);

  if(status & 0x1E){
    /*
     * clear FIFO before clear other flags
     */
    BuartIOctl(UART_IOCTL_RXFIFO_CLR,0);
    /*
     * clear other error flags
     */
    BuartIOctl(UART_IOCTL_RXINT_CLR,0);
  }

  if(status & 0x01 )
  {
    BuartIOctl(UART_IOCTL_RXINT_SET, 0);
    BuartIOctl(UART_IOCTL_RXINT_CLR, 0);
    
    // Notify thread if sufficient data are available
    if ( ( driver->rx_size > 0 ) &&
         ( (uint32_t)BuartIOctl(BUART_IOCTL_RXFIFO_DATLEN_GET, 0)  >= driver->rx_size ) )
    {
      driver->rx_size = 0;
    }

    {     
#ifndef NO_MICO_RTOS
      mico_rtos_set_semaphore( &driver->rx_complete );   
#else
      driver->rx_complete = true;
#endif
      
    }
  }
  
  if(BuartIOctl(UART_IOCTL_TXSTAT_GET,0) & 0x01)
  {
    BuartIOctl(UART_IOCTL_TXINT_CLR,0);

#ifndef NO_MICO_RTOS
    mico_rtos_set_semaphore( &driver->tx_complete );   
#else
    driver->tx_complete = true;
#endif
  }
}


void platform_uart_irq( platform_uart_driver_t* driver )
{
  if( driver->peripheral->uart == FUART )
    platform_fuart_irq( driver );
  else if ( driver->peripheral->uart == BUART ) 
    platform_buart_irq( driver );
  else
    return;
}


