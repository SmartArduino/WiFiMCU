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

OSStatus platform_i2c_init( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
  UNUSED_PARAMETER(i2c);
  UNUSED_PARAMETER(config);
  platform_log("unimplemented");
  return kUnsupportedErr;
}

bool platform_i2c_probe_device( const platform_i2c_t* i2c, const platform_i2c_config_t* config, int retries )
{
  UNUSED_PARAMETER(i2c);
  UNUSED_PARAMETER(config);
  UNUSED_PARAMETER(retries);
  platform_log("unimplemented");
  return false;
}

OSStatus platform_i2c_init_tx_message( platform_i2c_message_t* message, const void* tx_buffer, uint16_t tx_buffer_length, uint16_t retries )
{
  UNUSED_PARAMETER(message);
  UNUSED_PARAMETER(tx_buffer);
  UNUSED_PARAMETER(tx_buffer_length);
  UNUSED_PARAMETER(retries);
  platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_i2c_init_rx_message( platform_i2c_message_t* message, void* rx_buffer, uint16_t rx_buffer_length, uint16_t retries )
{
  UNUSED_PARAMETER(message);
  UNUSED_PARAMETER(rx_buffer);
  UNUSED_PARAMETER(rx_buffer_length);
  UNUSED_PARAMETER(retries);
  platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_i2c_init_combined_message( platform_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries )
{
  UNUSED_PARAMETER(message);
  UNUSED_PARAMETER(tx_buffer);
  UNUSED_PARAMETER(tx_buffer_length);
  UNUSED_PARAMETER(rx_buffer);
  UNUSED_PARAMETER(rx_buffer_length);
  UNUSED_PARAMETER(retries);
  platform_log("unimplemented");
  return kUnsupportedErr;
}


OSStatus platform_i2c_transfer( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* messages, uint16_t number_of_messages )
{
  UNUSED_PARAMETER(i2c);
  UNUSED_PARAMETER(config);
  UNUSED_PARAMETER(messages);
  UNUSED_PARAMETER(number_of_messages);
  platform_log("unimplemented");
  return kUnsupportedErr;
}


OSStatus platform_i2c_deinit( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
  UNUSED_PARAMETER(i2c);
  UNUSED_PARAMETER(config);
  platform_log("unimplemented");
  return kUnsupportedErr;
}


