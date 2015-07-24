/**
******************************************************************************
* @file    MicoDriverI2C.c 
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


#include "MICORTOS.h"
#include "MICOPlatform.h"

#include "platform.h"
#include "platform_common_config.h"
/******************************************************
*                    Constants
******************************************************/

#define I2C_FLAG_CHECK_TIMEOUT      ( 1000 )
#define I2C_FLAG_CHECK_LONG_TIMEOUT ( 1000 )

//#define I2C_USE_DMA           

#define DMA_FLAG_TC(stream_id) dma_flag_tc(stream_id)
#define DMA_TIMEOUT_LOOPS      (10000000)

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

#ifdef I2C_USE_DMA
static DMA_InitTypeDef  i2c_dma_init; /* Should investigate why this is global */
#endif

/******************************************************
*               Function Declarations
******************************************************/


/******************************************************
*               Function Definitions
******************************************************/

OSStatus MicoI2cInitialize( mico_i2c_device_t* device  )
{
 
  
  return kUnsupportedErr;
}


bool MicoI2cProbeDevice( mico_i2c_device_t* device, int retries )
{
  
    return false;
  
}



OSStatus MicoI2cBuildTxMessage(mico_i2c_message_t* message, const void* tx_buffer, uint16_t  tx_buffer_length, uint16_t retries)
{
  if( ( message == 0 ) || ( tx_buffer == 0 ) || ( tx_buffer_length == 0 ) )
  {
    return kUnsupportedErr;
  }
  memset(message, 0x00, sizeof(mico_i2c_message_t));
  message->tx_buffer = tx_buffer;
  message->combined = false;
  message->retries = retries;
  message->tx_length = tx_buffer_length;
  
  return kUnsupportedErr;
}

OSStatus MicoI2cBuildRxMessage(mico_i2c_message_t* message, void* rx_buffer, uint16_t  rx_buffer_length, uint16_t retries)
{
  if( ( message == 0 ) || ( rx_buffer == 0 ) || ( rx_buffer_length == 0 ) )
  {
    return kUnsupportedErr;
  }
  memset(message, 0x00, sizeof(mico_i2c_message_t));
  message->rx_buffer = rx_buffer;
  message->combined = false;
  message->retries = retries;
  message->rx_length = rx_buffer_length;
  
  return kUnsupportedErr;
}

OSStatus MicoI2cBuildCombinedMessage(mico_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries)
{
  if( ( message == 0 ) || ( rx_buffer == 0 ) || ( tx_buffer == 0 ) || ( tx_buffer_length == 0 ) || ( rx_buffer_length == 0 ) )
  {
    return kUnsupportedErr;
  }
  memset(message, 0x00, sizeof(mico_i2c_message_t));
  message->rx_buffer = rx_buffer;
  message->tx_buffer = tx_buffer;
  message->combined = true;
  message->retries = retries;
  message->tx_length = tx_buffer_length;
  message->rx_length = rx_buffer_length;
  
  return kUnsupportedErr;
}

OSStatus MicoI2cTransfer( mico_i2c_device_t* device, mico_i2c_message_t* messages, uint16_t number_of_messages )
{
 
  return kUnsupportedErr;
}

OSStatus MicoI2cFinalize( mico_i2c_device_t* device )
{

  
  return kUnsupportedErr;
}


