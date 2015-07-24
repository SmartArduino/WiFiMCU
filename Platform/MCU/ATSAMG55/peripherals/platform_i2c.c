/**
******************************************************************************
* @file    platform_i2c.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide I2C driver functions.
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

#include "platform_peripheral.h"
#include "MiCORtos.h"

/******************************************************
*                    Constants
******************************************************/
#define I2C_TRANSACTION_TIMEOUT        (1000)

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/

/******************************************************
*                    Structures
******************************************************/

typedef struct
{
  mico_semaphore_t         transfer_complete;
  uint32_t                 transfer_size;
  bool                     i2c_inited;
  const platform_i2c_t*    peripheral;
  platform_i2c_message_t*  current_message;
  uint16_t                 data_count;
  volatile int             transfer_status;
} g55_i2c_runtime_data_t;

/******************************************************
*               Variables Definitions
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/

/******************************************************
*               Function Definitions
******************************************************/

OSStatus platform_i2c_init( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
  OSStatus err = kNoErr;
  
  platform_mcu_powersave_disable( );
  require_action_quiet( i2c != NULL, exit, err = kParamErr);
  
  twi_options_t     twi_slave_config;
  
  /* Setup SAMG pins */
  platform_gpio_peripheral_pin_init( i2c->sda_pin, ( i2c->sda_pin_mux_mode | IOPORT_MODE_OPEN_DRAIN ) );
  platform_gpio_peripheral_pin_init( i2c->scl_pin, ( i2c->scl_pin_mux_mode | IOPORT_MODE_OPEN_DRAIN ) );
  
  /* Enable the peripheral and set I2C mode. */
  if( pmc_is_periph_clk_enabled( i2c->peripheral_id ) == 0  ){
    flexcom_enable( i2c->flexcom_base );  
  }
  flexcom_set_opmode( i2c->flexcom_base, FLEXCOM_TWI );
  
  twi_enable_master_mode( i2c->port );
  
  require_action( config->address_width == I2C_ADDRESS_WIDTH_7BIT, exit, err = kParamErr);
  
  twi_slave_config.chip = (uint8_t) config->address;
  switch ( config->speed_mode )
  {
    case I2C_LOW_SPEED_MODE     : twi_slave_config.speed = 10000;  break;
    case I2C_STANDARD_SPEED_MODE: twi_slave_config.speed = 100000; break;
    case I2C_HIGH_SPEED_MODE    : twi_slave_config.speed = 100000; break;
    default                     : platform_log("Speed mode is not supported"); break;
  }
  
  twi_slave_config.master_clk = CPU_CLOCK_HZ;
  twi_slave_config.smbus      = 0;
  twi_master_init( i2c->port, &twi_slave_config );
  
exit:
  return err;
}

bool platform_i2c_probe_device( const platform_i2c_t* i2c, const platform_i2c_config_t* config, int retries )
{
  for ( ; retries != 0 ; --retries ){
    if( twi_probe( i2c->port, config->address ) == TWI_SUCCESS ){
      return true;
    }
  }
  return false;
}

OSStatus platform_i2c_init_tx_message( platform_i2c_message_t* message, const void* tx_buffer, uint16_t tx_buffer_length, uint16_t retries )
{
  memset( message, 0x00, sizeof(platform_i2c_message_t) );
  message->tx_buffer = tx_buffer;
  message->tx_length = tx_buffer_length;
  message->retries   = retries;
  
  return kNoErr;
}

OSStatus platform_i2c_init_rx_message( platform_i2c_message_t* message, void* rx_buffer, uint16_t rx_buffer_length, uint16_t retries )
{
  memset(message, 0x00, sizeof(platform_i2c_message_t));
  message->rx_buffer = rx_buffer;
  message->retries   = retries;
  message->rx_length = rx_buffer_length;
  
  return kNoErr;
}

OSStatus platform_i2c_init_combined_message( platform_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries )
{
  memset(message, 0x00, sizeof(platform_i2c_message_t));
  message->rx_buffer = rx_buffer;
  message->tx_buffer = tx_buffer;
  message->retries   = retries;
  message->tx_length = tx_buffer_length;
  message->rx_length = rx_buffer_length;
  
  return kNoErr;
}


OSStatus platform_i2c_transfer( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* messages, uint16_t number_of_messages )
{
  OSStatus err =  kNoErr;
  Twi*     twi    = ( Twi* )( i2c->port );
  int      i;
  twi_packet_t packet;
  int      retry_count;
  
  platform_mcu_powersave_disable();
  /* Check input arguments */
  require_action_quiet( (i2c != NULL) && ( config != NULL ) &&( messages != NULL ) && ( number_of_messages != 0 ), exit, err = kParamErr );
  
  for ( i = 0; i < number_of_messages && err == kNoErr; i++ )
  {
    platform_i2c_message_t* message_pointer = &messages[i];
    check_string( message_pointer != NULL, "Message pointer shouldn't be null" );
    for ( retry_count = 0; retry_count < message_pointer->retries; retry_count++ )
    {
      err = kNoErr;
      
      if ( message_pointer->tx_buffer != NULL )
      {
        /** Get the extension boards info */
        packet.chip        = config->address;
        packet.addr_length = 0;
        packet.addr[0]     = 0;
        packet.buffer      = (void *)message_pointer->tx_buffer;
        packet.length      = message_pointer->tx_length;
        if (twi_master_write(twi, &packet) != TWI_SUCCESS) {
          err = kGeneralErr;
        }
      }
      
      
      if (   message_pointer->rx_buffer != NULL )
      {
        /** Get the extension boards info */
        packet.chip        = config->address;
        packet.addr_length = 0;
        packet.addr[0]     = 0;
        packet.buffer      = message_pointer->rx_buffer;
        packet.length      = message_pointer->rx_length;
        if (twi_master_read(twi, &packet) != TWI_SUCCESS) {
          err = kGeneralErr;
        }
      }
      
      if( err == kNoErr )
        break;
    }
  }
  
exit:
  platform_mcu_powersave_enable();
  return err;
}


OSStatus platform_i2c_deinit( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
  /* Disable the RX and TX PDC transfer requests */
  if( pmc_is_periph_clk_enabled( i2c->peripheral_id ) == 1  ){
    flexcom_disable( i2c->flexcom_base );  
  }
  
  twi_reset( i2c->port );

  return kNoErr;
}

