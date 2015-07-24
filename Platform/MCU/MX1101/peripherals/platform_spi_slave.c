/**
******************************************************************************
* @file    platform_spi_slave.c 
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

OSStatus platform_spi_slave_init( platform_spi_slave_driver_t* driver, const platform_spi_t* peripheral, const platform_spi_slave_config_t* config )
{
  UNUSED_PARAMETER(driver);
  UNUSED_PARAMETER(peripheral);
  UNUSED_PARAMETER(config);
  platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_spi_slave_deinit( platform_spi_slave_driver_t* driver )
{
  UNUSED_PARAMETER( driver );
  platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_spi_slave_receive_command( platform_spi_slave_driver_t* driver, platform_spi_slave_command_t* command, uint32_t timeout_ms )
{
  UNUSED_PARAMETER( driver );
  UNUSED_PARAMETER( command );
  UNUSED_PARAMETER( timeout_ms );
  platform_log("unimplemented");
  return kUnsupportedErr;
}


OSStatus platform_spi_slave_transfer_data( platform_spi_slave_driver_t* driver, platform_spi_slave_transfer_direction_t direction, platform_spi_slave_data_buffer_t* buffer, uint32_t timeout_ms )
{
  UNUSED_PARAMETER(driver);
  UNUSED_PARAMETER(direction);
  UNUSED_PARAMETER(buffer);
  UNUSED_PARAMETER(timeout_ms);
  platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_spi_slave_send_error_status( platform_spi_slave_driver_t* driver, platform_spi_slave_transfer_status_t error_status )
{
  UNUSED_PARAMETER(driver);
  UNUSED_PARAMETER(error_status);
  platform_log("unimplemented");
  return kUnsupportedErr;
}

OSStatus platform_spi_slave_generate_interrupt( platform_spi_slave_driver_t* driver, uint32_t pulse_duration_ms )
{
  UNUSED_PARAMETER(driver);
  UNUSED_PARAMETER(pulse_duration_ms);
  platform_log("unimplemented");
  return kUnsupportedErr;
}


