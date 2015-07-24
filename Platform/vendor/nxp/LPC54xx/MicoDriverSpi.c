/**
******************************************************************************
* @file    MicoDriverSpi.c 
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


#include "MICORTOS.h"
#include "MICOPlatform.h"

#include "platform.h"
#include "platform_common_config.h"
#include "debug.h"

/******************************************************
*                    Constants
******************************************************/
#define MAX_NUM_SPI_PRESCALERS     (8)
#define SPI_DMA_TIMEOUT_LOOPS      (10000)

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/


OSStatus MicoSpiInitialize( const mico_spi_device_t* spi )
{
  
  
  return kNoErr;
}

OSStatus MicoSpiTransfer( const mico_spi_device_t* spi, mico_spi_message_segment_t* segments, uint16_t number_of_segments )
{
  
  
  return kNoErr;
}

OSStatus MicoSpiFinalize( const mico_spi_device_t* spi )
{
  
  
  return kNoErr;
}




