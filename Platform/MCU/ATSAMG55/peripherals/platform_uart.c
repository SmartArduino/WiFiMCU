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
#include "MICOPlatform.h"

#include "platform.h"
#include "platform_peripheral.h"
#include "platformLogging.h"

/******************************************************
*                    Constants
******************************************************/

#define DMA_INTERRUPT_FLAGS  ( DMA_IT_TC | DMA_IT_TE | DMA_IT_DME | DMA_IT_FE )

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

static IRQn_Type platform_flexcom_irq_numbers[] =
{
  [0]  = FLEXCOM0_IRQn,
  [1]  = FLEXCOM1_IRQn,
  [2]  = FLEXCOM2_IRQn,
  [3]  = FLEXCOM3_IRQn,
  [4]  = FLEXCOM4_IRQn,
  [5]  = FLEXCOM5_IRQn,
  [6]  = FLEXCOM6_IRQn,
  [7]  = FLEXCOM7_IRQn,
};

/******************************************************
*        Static Function Declarations
******************************************************/

/******************************************************
*               Function Definitions
******************************************************/

OSStatus platform_uart_init( platform_uart_driver_t* driver, const platform_uart_t* peripheral, const platform_uart_config_t* config, ring_buffer_t* optional_ring_buffer )
{
  pdc_packet_t      pdc_uart_packet, pdc_uart_tx_packet;
  OSStatus          err = kNoErr;
  sam_usart_opt_t   settings;
  bool              hardware_shaking = false;

  platform_mcu_powersave_disable();
  
  require_action_quiet( ( driver != NULL ) && ( peripheral != NULL ) && ( config != NULL ), exit, err = kParamErr);
  require_action_quiet( (optional_ring_buffer->buffer != NULL ) && (optional_ring_buffer->size != 0), exit, err = kUnsupportedErr);
  
  driver->rx_size              = 0;
  driver->tx_size              = 0;
  driver->rx_ring_buffer       = optional_ring_buffer;
  driver->last_transmit_result = kNoErr;
  driver->last_receive_result  = kNoErr;
  driver->peripheral           = (platform_uart_t*)peripheral;
#ifndef NO_MICO_RTOS
  mico_rtos_init_semaphore( &driver->tx_complete, 1 );
  mico_rtos_init_semaphore( &driver->rx_complete, 1 );
  mico_rtos_init_semaphore( &driver->sem_wakeup,  1 );
  mico_rtos_init_mutex    ( &driver->tx_mutex );
#else
  driver->tx_complete = false;
  driver->rx_complete = false;
#endif

  /* Set Tx and Rx pin mode to UART peripheral */
  platform_gpio_peripheral_pin_init( peripheral->tx_pin, ( peripheral->tx_pin_mux_mode | IOPORT_MODE_PULLUP ) );
  platform_gpio_peripheral_pin_init( peripheral->rx_pin, ( peripheral->rx_pin_mux_mode | IOPORT_MODE_PULLUP ) );

  /* Init CTS and RTS pins (if available) */
  if ( peripheral->cts_pin != NULL && (config->flow_control == FLOW_CONTROL_CTS || config->flow_control == FLOW_CONTROL_CTS_RTS) )
  {
    hardware_shaking = true;
    platform_gpio_peripheral_pin_init( peripheral->cts_pin, ( peripheral->cts_pin_mux_mode | IOPORT_MODE_PULLUP ) );
  }

  if ( peripheral->rts_pin != NULL && (config->flow_control == FLOW_CONTROL_CTS || config->flow_control == FLOW_CONTROL_CTS_RTS) )
  {
    hardware_shaking = true;
    platform_gpio_peripheral_pin_init( peripheral->rts_pin, ( peripheral->rts_pin_mux_mode | IOPORT_MODE_PULLUP ) );
  }

  /* Enable the clock. */
  if( pmc_is_periph_clk_enabled( peripheral->peripheral_id ) == 0  ){
    flexcom_enable( peripheral->flexcom_base );
  }
  flexcom_set_opmode( peripheral->flexcom_base, FLEXCOM_USART );

  /* Enable the receiver and transmitter. */
  usart_reset_tx( peripheral->port );
  usart_reset_rx( peripheral->port );

  /* Disable all the interrupts. */
  usart_disable_interrupt( peripheral->port, 0xffffffff );

  switch ( config->parity ) {
  case NO_PARITY:
    settings.parity_type = US_MR_PAR_NO;
    break;
  case EVEN_PARITY:
    settings.parity_type = US_MR_PAR_EVEN;
    break;
  case ODD_PARITY:
    settings.parity_type = US_MR_PAR_ODD;
    break;
  default:
    err = kParamErr;
    goto exit;
  }
  switch ( config->data_width) {
  case DATA_WIDTH_5BIT: 
    settings.char_length = US_MR_CHRL_5_BIT;
    break;
  case DATA_WIDTH_6BIT: 
    settings.char_length = US_MR_CHRL_6_BIT;
    break;
  case DATA_WIDTH_7BIT: 
    settings.char_length = US_MR_CHRL_7_BIT;
    break;
  case DATA_WIDTH_8BIT: 
    settings.char_length = US_MR_CHRL_8_BIT;
    break;
  case DATA_WIDTH_9BIT: 
    settings.char_length = US_MR_MODE9;
    break;
  default:
    err = kParamErr;
    goto exit;
  }
  settings.baudrate = config->baud_rate;
  settings.stop_bits = ( config->stop_bits == STOP_BITS_1 ) ? US_MR_NBSTOP_1_BIT : US_MR_NBSTOP_2_BIT;
  settings.channel_mode= US_MR_CHMODE_NORMAL;

  /* Configure USART in serial mode. */  
  if (!hardware_shaking) {
    usart_init_rs232( peripheral->port, &settings, sysclk_get_peripheral_hz());
  } else {
    usart_init_hw_handshaking( peripheral->port, &settings, sysclk_get_peripheral_hz());
  }

  /* Enable uart interrupt */
  NVIC_SetPriority( platform_flexcom_irq_numbers[peripheral->uart_id], 0x06 );
  NVIC_EnableIRQ( platform_flexcom_irq_numbers[peripheral->uart_id] );

  /* Enable PDC transmit */
  pdc_enable_transfer( usart_get_pdc_base( peripheral->port ), PERIPH_PTCR_TXTEN | PERIPH_PTCR_RXTEN );
  pdc_disable_transfer( usart_get_pdc_base( driver->peripheral->port ), PERIPH_PTCR_TXTDIS );

  pdc_uart_packet.ul_addr = (uint32_t)driver->rx_ring_buffer->buffer;
  pdc_uart_packet.ul_size = (uint32_t)driver->rx_ring_buffer->size;
  pdc_rx_init( usart_get_pdc_base( peripheral->port ), &pdc_uart_packet, &pdc_uart_packet );

  pdc_uart_tx_packet.ul_addr = (uint32_t)0;
  pdc_uart_tx_packet.ul_size = (uint32_t)1;

  pdc_tx_init( usart_get_pdc_base( driver->peripheral->port ), &pdc_uart_tx_packet, NULL );

  usart_enable_interrupt( peripheral->port, US_IER_ENDRX | US_IER_RXBUFF | US_IER_RXRDY | US_IER_ENDTX );

  /* Enable the receiver and transmitter. */
  usart_enable_tx( peripheral->port );
  usart_enable_rx( peripheral->port );
  
exit:
  MicoMcuPowerSaveConfig(true);
  return err;
}

OSStatus platform_uart_deinit( platform_uart_driver_t* driver )
{
  OSStatus          err = kNoErr;
  
  platform_mcu_powersave_disable();
  require_action_quiet( ( driver != NULL ), exit, err = kParamErr);
  
  usart_disable_interrupt( driver->peripheral->port, 0xffffffff );

  NVIC_DisableIRQ( platform_flexcom_irq_numbers[driver->peripheral->uart_id] );

  pdc_disable_transfer( usart_get_pdc_base( driver->peripheral->port ), PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS );

  usart_disable_tx( driver->peripheral->port );
  usart_disable_rx( driver->peripheral->port );

  if( pmc_is_periph_clk_enabled( driver->peripheral->peripheral_id ) == 1  ){
    flexcom_disable( driver->peripheral->flexcom_base );
  }

  platform_gpio_deinit( driver->peripheral->tx_pin );
  platform_gpio_deinit( driver->peripheral->rx_pin );

  if ( driver->peripheral->cts_pin != NULL )
  {
    platform_gpio_deinit( driver->peripheral->cts_pin );
  }

  if ( driver->peripheral->rts_pin != NULL )
  {
    platform_gpio_deinit( driver->peripheral->rts_pin );
  }

#ifndef NO_MICO_RTOS
  mico_rtos_deinit_semaphore(&driver->rx_complete);
  mico_rtos_deinit_semaphore(&driver->tx_complete);
#endif

  driver->peripheral = NULL;
  memset( driver, 0, sizeof(platform_uart_driver_t) );
  
exit:
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_uart_transmit_bytes( platform_uart_driver_t* driver, const uint8_t* data_out, uint32_t size )
{
  OSStatus      err = kNoErr;
  pdc_packet_t  pdc_uart_packet;

  platform_mcu_powersave_disable();
  
#ifndef NO_MICO_RTOS
  mico_rtos_lock_mutex( &driver->tx_mutex );
#endif
  
  require_action_quiet( ( driver != NULL ) && ( data_out != NULL ) && ( size != 0 ), exit, err = kParamErr);
  
  /* reset DMA transmission result. the result is assigned in interrupt handler */
  driver->last_transmit_result                    = kGeneralErr;
  driver->tx_size                                 = size;
  
  pdc_uart_packet.ul_addr = (uint32_t) data_out;
  pdc_uart_packet.ul_size = size;
  pdc_tx_init( usart_get_pdc_base( driver->peripheral->port ), &pdc_uart_packet, NULL);

  /* Enable Tx DMA transmission */
  pdc_enable_transfer( usart_get_pdc_base( driver->peripheral->port ), PERIPH_PTCR_TXTEN );
  
#ifndef NO_MICO_RTOS
  mico_rtos_get_semaphore( &driver->tx_complete, MICO_NEVER_TIMEOUT );
#else 
  while( driver->tx_complete == false);
  driver->tx_complete = false;
#endif
  
exit:  
#ifndef NO_MICO_RTOS  
  mico_rtos_unlock_mutex( &driver->tx_mutex );
#endif
  platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_uart_receive_bytes( platform_uart_driver_t* driver, uint8_t* data_in, uint32_t expected_data_size, uint32_t timeout_ms )
{
  OSStatus err = kNoErr;
  uint32_t transfer_size;

  //platform_mcu_powersave_disable();

  require_action_quiet( ( driver != NULL ) && ( data_in != NULL ) && ( expected_data_size != 0 ), exit, err = kParamErr);
  require_action_quiet( driver->rx_ring_buffer != NULL , exit, err = kUnsupportedErr);

  while ( expected_data_size != 0 )
  {
    transfer_size = MIN(driver->rx_ring_buffer->size / 2, expected_data_size);

    /* Check if ring buffer already contains the required amount of data. */
    if ( transfer_size > ring_buffer_used_space( driver->rx_ring_buffer ) )
    {
      /* Set rx_size and wait in rx_complete semaphore until data reaches rx_size or timeout occurs */
      driver->rx_size = transfer_size;

#ifndef NO_MICO_RTOS
      if ( mico_rtos_get_semaphore( &driver->rx_complete, timeout_ms ) != kNoErr )
      {
        driver->rx_size = 0;
        err = kTimeoutErr;
        goto exit;
      }

      /* Reset rx_size to prevent semaphore being set while nothing waits for the data */
      driver->rx_size = 0;
#else
      driver->rx_complete = false;
      int delay_start = mico_get_time_no_os();
      while(driver->rx_complete == false){
        if(mico_get_time_no_os() >= delay_start + timeout_ms && timeout_ms != MICO_NEVER_TIMEOUT){
          driver->rx_size = 0;
          err = kTimeoutErr;
          goto exit;
        }
      }
    driver->rx_size = 0;
#endif
    }
    expected_data_size -= transfer_size;

    // Grab data from the buffer
    do
    {
      uint8_t* available_data;
      uint32_t bytes_available;

      ring_buffer_get_data( driver->rx_ring_buffer, &available_data, &bytes_available );
      bytes_available = MIN( bytes_available, transfer_size );
      memcpy( data_in, available_data, bytes_available );
      transfer_size -= bytes_available;
      data_in = ( (uint8_t*)data_in + bytes_available );
      ring_buffer_consume( driver->rx_ring_buffer, bytes_available );
    }
    while ( transfer_size != 0 );
  }

  require_action( expected_data_size == 0, exit, err = kReadErr);

exit:
  //platform_mcu_powersave_enable();
  return err;
}

OSStatus platform_uart_get_length_in_buffer( platform_uart_driver_t* driver )
{  
  return ring_buffer_used_space( driver->rx_ring_buffer );
}

/******************************************************
*            Interrupt Service Routines
******************************************************/

void platform_uart_irq( platform_uart_driver_t* driver )
{
  uint32_t status = usart_get_status( driver->peripheral->port );
  uint32_t mask = usart_get_interrupt_mask( driver->peripheral->port );
  Pdc* pdc_register = usart_get_pdc_base( driver->peripheral->port );

  /* ENDTX flag is set when Tx DMA transfer is done */
  if ( ( mask & US_IMR_ENDTX ) && ( status & US_CSR_ENDTX ) )
  {
    pdc_packet_t dma_packet;

    /* ENDTX is cleared when TCR or TNCR is set to a non-zero value, which effectively
     * starts another Tx DMA transaction. To work around this, disable Tx before
     * performing a dummy Tx init.
     */
    pdc_disable_transfer( usart_get_pdc_base( driver->peripheral->port ), PERIPH_PTCR_TXTDIS );

    dma_packet.ul_addr = (uint32_t)0;
    dma_packet.ul_size = (uint32_t)1;

    pdc_tx_init( usart_get_pdc_base( driver->peripheral->port ), &dma_packet, NULL );

    /* Notifies waiting thread that Tx DMA transfer is complete */
#ifndef NO_MICO_RTOS
    mico_rtos_set_semaphore( &driver->tx_complete );
#else
    driver->tx_complete = true;
#endif
  }

  /* ENDRX flag is set when RCR is 0. RNPR and RNCR values are then copied into
   * RPR and RCR, respectively, while the RX tranfer continues. We now need to
   * prepare RNPR and RNCR for the next iteration.
   */
  if ( ( mask & US_IMR_ENDRX ) && ( status & US_CSR_ENDRX ) )
  {
    pdc_register->PERIPH_RNPR = (uint32_t)driver->rx_ring_buffer->buffer;
    pdc_register->PERIPH_RNCR = (uint32_t)driver->rx_ring_buffer->size;
  }

  /* RXRDY interrupt is triggered and flag is set when a new character has been
   * received but not yet read from the US_RHR. When this interrupt executes,
   * the DMA engine already read the character out from the US_RHR and RXRDY flag
   * is no longer asserted. The code below updates the ring buffer parameters
   * to keep them current
   */
  if ( ( mask & US_IMR_RXRDY )  )
  {
    driver->rx_ring_buffer->tail = driver->rx_ring_buffer->size - pdc_register->PERIPH_RCR;

    // Notify thread if sufficient data are available
    if ( ( driver->rx_size > 0 ) && ( ring_buffer_used_space( driver->rx_ring_buffer ) >= driver->rx_size ) )
    {
#ifndef NO_MICO_RTOS
      mico_rtos_set_semaphore( &driver->rx_complete );
#else
      driver->rx_complete = true;
#endif
      driver->rx_size = 0;
    }
  }
}


