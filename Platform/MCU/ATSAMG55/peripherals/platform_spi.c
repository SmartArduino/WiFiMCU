/**
******************************************************************************
* @file    platform_spi.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide SPI driver functions.
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


#include "platform.h"
#include "platform_peripheral.h"

/******************************************************
*                    Constants
******************************************************/

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

/******************************************************
*               Function Definitions
******************************************************/

OSStatus platform_spi_init( platform_spi_driver_t* driver, const platform_spi_t* peripheral, const platform_spi_config_t* config )
{
  pdc_packet_t  pdc_spi_packet;
  Pdc*          spi_pdc;
  OSStatus      err = kNoErr;

  platform_mcu_powersave_disable( );

  require_action_quiet( ( driver != NULL ) && ( peripheral != NULL ) && ( config != NULL ), exit, err = kParamErr);

  driver->peripheral = (platform_spi_t *)peripheral;

  spi_pdc = spi_get_pdc_base( peripheral->port );

  /* Setup chip select pin */
  platform_gpio_init( config->chip_select, OUTPUT_PUSH_PULL );
  platform_gpio_output_high( config->chip_select );

  /* Setup other pins */
  platform_gpio_peripheral_pin_init( peripheral->mosi_pin,   ( peripheral->mosi_pin_mux_mode | IOPORT_MODE_PULLUP ) );
  platform_gpio_peripheral_pin_init( peripheral->miso_pin,   ( peripheral->miso_pin_mux_mode | IOPORT_MODE_PULLUP ) );
  platform_gpio_peripheral_pin_init( peripheral->clock_pin,  ( peripheral->clock_pin_mux_mode | IOPORT_MODE_PULLUP ) );

  /* Enable the peripheral and set SPI mode. */
  if( pmc_is_periph_clk_enabled( driver->peripheral->peripheral_id ) == 0  ){
    flexcom_enable( driver->peripheral->flexcom_base );
  }
  flexcom_set_opmode( driver->peripheral->flexcom_base, FLEXCOM_SPI );

  /* Init pdc, and clear RX TX. */
  pdc_spi_packet.ul_addr = 0;
  pdc_spi_packet.ul_size = 1;
  pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );
  pdc_rx_init( spi_pdc, &pdc_spi_packet, NULL );

  /* Configure an SPI peripheral. */
  spi_disable( peripheral->port );
  spi_reset( peripheral->port );
  spi_set_lastxfer( peripheral->port );
  spi_set_master_mode( peripheral->port );
  spi_disable_mode_fault_detect( peripheral->port );
  spi_set_peripheral_chip_select_value( peripheral->port, 0 );

  spi_set_clock_polarity( peripheral->port, 0, ( ( config->mode & SPI_CLOCK_IDLE_HIGH )   ? (1) : (0) ) );
  spi_set_clock_phase( peripheral->port, 0,    ( ( config->mode & SPI_CLOCK_FALLING_EDGE ) ? (1) : (0) ) );

  spi_set_bits_per_transfer( peripheral->port, 0, SPI_CSR_BITS_8_BIT );
  spi_set_baudrate_div( peripheral->port, 0, (uint8_t)( sysclk_get_cpu_hz()  / config->speed ) );
  spi_set_transfer_delay( peripheral->port, 0, 0, 0 );
  spi_enable( peripheral->port );
  pdc_disable_transfer( spi_pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS );

exit:
  platform_mcu_powersave_enable( );
  return err;
}

OSStatus platform_spi_deinit( platform_spi_driver_t* driver )
{
  /* Disable the RX and TX PDC transfer requests */
  if( pmc_is_periph_clk_enabled( driver->peripheral->peripheral_id ) == 1  ){
    flexcom_disable( driver->peripheral->flexcom_base );
  }
  
  spi_disable( driver->peripheral->port );
  spi_reset( driver->peripheral->port );

  return kNoErr;
}

OSStatus samg5x_spi_transfer_internal( const platform_spi_t* spi, const uint8_t* data_out, uint8_t* data_in, uint32_t data_length )
{
  Pdc*         spi_pdc  = spi_get_pdc_base( spi->port );
  pdc_packet_t pdc_spi_packet;
  uint8_t junk3;
  uint16_t junk2;

  /* Send and read */
  if( data_in != NULL && data_out != NULL )
  {
    pdc_spi_packet.ul_addr = (uint32_t)data_in;
    pdc_spi_packet.ul_size = (uint32_t)data_length;
    pdc_rx_init( spi_pdc, &pdc_spi_packet, NULL );  

    pdc_spi_packet.ul_addr = (uint32_t)data_out;
    pdc_spi_packet.ul_size = (uint32_t)data_length;
    pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );  

    /* Enable the RX and TX PDC transfer requests */
    pdc_enable_transfer( spi_pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN );

    /* Waiting transfer done*/
    while ( ( spi_read_status( spi->port ) & SPI_SR_RXBUFF ) == 0 )
    {
    }

    /* Disable the RX and TX PDC transfer requests */
    pdc_disable_transfer(spi_pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS);
    
    pdc_spi_packet.ul_addr = 0;
    pdc_spi_packet.ul_size = 1;
    pdc_rx_init( spi_pdc, &pdc_spi_packet, NULL ); 
    pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );  
  }
  /* Send Only */
  else if( data_in == NULL && data_out != NULL)  
  {
    pdc_spi_packet.ul_addr = (uint32_t)data_out;
    pdc_spi_packet.ul_size = (uint32_t)data_length;
    pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL ); 

    /* Enable the TX PDC transfer requests */
    pdc_enable_transfer( spi_pdc, PERIPH_PTCR_TXTEN );

    /* Waiting transfer done*/
    while ( ( spi_read_status( spi->port ) & SPI_SR_ENDTX ) == 0 )
    {
    }
    
    /* Disable the RX and TX PDC transfer requests */
    pdc_disable_transfer(spi_pdc, PERIPH_PTCR_TXTDIS);
    
    pdc_spi_packet.ul_addr = 0;
    pdc_spi_packet.ul_size = 1;
    pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );  

    spi_read( spi->port, &junk2, &junk3);
  }
  /* Read Only */
  else if( data_in != NULL && data_out == NULL)
  {
    pdc_spi_packet.ul_addr = (uint32_t)data_in;
    pdc_spi_packet.ul_size = (uint32_t)data_length;
    pdc_rx_init( spi_pdc, &pdc_spi_packet, NULL );  

    pdc_spi_packet.ul_addr = (uint32_t)data_in;
    pdc_spi_packet.ul_size = (uint32_t)data_length;
    pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );  

    /* Enable the RX and TX PDC transfer requests */
    pdc_enable_transfer( spi_pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN );

    /* Waiting transfer done*/
    while ( ( spi_read_status( spi->port ) & SPI_SR_RXBUFF ) == 0 )
    {
    }

    /* Disable the RX and TX PDC transfer requests */
    pdc_disable_transfer(spi_pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS);
    
    pdc_spi_packet.ul_addr = 0;
    pdc_spi_packet.ul_size = 1;
    pdc_rx_init( spi_pdc, &pdc_spi_packet, NULL ); 
    pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );  
  }

  return kNoErr;
}


OSStatus platform_spi_transfer( platform_spi_driver_t* driver, const platform_spi_config_t* config, const platform_spi_message_segment_t* segments, uint16_t number_of_segments )
{
  int i = 0;
  OSStatus result;
  platform_mcu_powersave_disable( );

  platform_gpio_output_low( config->chip_select );

  for ( i = 0; i < number_of_segments; i++ )
  {
    if( segments[i].length == 0)
      continue;
    result = samg5x_spi_transfer_internal( driver->peripheral, segments[i].tx_buffer, segments[i].rx_buffer, segments[i].length );

    if ( result != kNoErr )
    {
      return result;
    }
  }

  platform_gpio_output_high( config->chip_select );

  platform_mcu_powersave_enable( );

  return kNoErr;
}



