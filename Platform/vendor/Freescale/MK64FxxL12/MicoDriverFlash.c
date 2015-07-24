/**
******************************************************************************
* @file    MicoDriverAdc.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provides flash operation functions.
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

/* Includes ------------------------------------------------------------------*/
#include "PlatformLogging.h"
#include "MicoPlatform.h"
#include "platform.h"
#include "Platform_common_config.h"
#include "stdio.h"
#ifdef USE_MICO_SPI_FLASH
#include "spi_flash.h"
#endif
#include "flash_kinetis.h"

#define FTFx_PSECTOR_SIZE 4096

static int flash_inilized = 0;

const char* flash_name[] =
{ 
#ifdef USE_MICO_SPI_FLASH
  [MICO_SPI_FLASH] = "SPI", 
#endif
  [MICO_INTERNAL_FLASH] = "Internal",
};

OSStatus MicoFlashInitialize( mico_flash_t flash )
{ 
    int ret;

    if (flash_inilized == 1)
        return kNoErr;
    
    platform_log_trace();
    if(flash == MICO_INTERNAL_FLASH){
      FLASH_Initialization();
      flash_inilized = 1;
      return kNoErr;    
    }
    
    return kUnsupportedErr;
}

OSStatus MicoFlashErase( mico_flash_t flash, uint32_t StartAddress, uint32_t EndAddress )
{ 
    platform_log_trace();
    if(flash == MICO_INTERNAL_FLASH){
      if(StartAddress<INTERNAL_FLASH_START_ADDRESS || EndAddress > INTERNAL_FLASH_END_ADDRESS)
        return kParamErr;
      return internalFlashErase(StartAddress, EndAddress);    
    }
    return kUnsupportedErr;
}

OSStatus MicoFlashWrite(mico_flash_t flash, volatile uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength)
{
    if(flash == MICO_INTERNAL_FLASH){
    if( *FlashAddress<INTERNAL_FLASH_START_ADDRESS || *FlashAddress + DataLength > INTERNAL_FLASH_END_ADDRESS + 1)
      return kParamErr;
    return internalFlashWrite(FlashAddress, (uint32_t *)Data, DataLength);    
  }
    return kUnsupportedErr;
}

OSStatus MicoFlashRead(mico_flash_t flash, volatile uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength)
{
    if(flash == MICO_INTERNAL_FLASH){
      if( *FlashAddress<INTERNAL_FLASH_START_ADDRESS || *FlashAddress + DataLength > INTERNAL_FLASH_END_ADDRESS + 1)
        return kParamErr;
      memcpy(Data, (void *)(*FlashAddress), DataLength);
      *FlashAddress += DataLength;
      return kNoErr;
    }
    return kUnsupportedErr;
}

OSStatus MicoFlashFinalize( mico_flash_t flash )
{

    return kNoErr;
}

OSStatus internalFlashErase(uint32_t StartAddress, uint32_t EndAddress)
{
  if (StartAddress % FTFx_PSECTOR_SIZE) {
    StartAddress = StartAddress - (StartAddress % FTFx_PSECTOR_SIZE);
  }
  while (StartAddress < EndAddress)
  {
      __disable_irq();
      FLASH_EraseSector(StartAddress);
      __enable_irq();
      StartAddress += FTFx_PSECTOR_SIZE;                
  }
  return kNoErr;
}

#ifdef USE_MICO_SPI_FLASH
OSStatus spiFlashErase(uint32_t StartAddress, uint32_t EndAddress)
{

  return kNoErr;
}
#endif

OSStatus internalFlashWrite(volatile uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
  uint32_t addr = *FlashAddress;
  uint8_t tmp[8], *p = (uint8_t*)Data;
  int i, padlen, offset = 0;
  
  *FlashAddress += DataLength;
  padlen = addr % 8;
  if (padlen) {
    offset = 8 - padlen;
    if (offset > DataLength)
        offset = DataLength;
    memset(tmp, 0xFF, 8);
    memcpy(&tmp[padlen], p, offset);
    __disable_irq();
    FLASH_ProgramPhrase(addr-padlen, (LWord *)tmp);
    __enable_irq();
    addr = addr + offset;
    DataLength = DataLength - offset;
  }

  while(DataLength >= 8) {
    __disable_irq();
    FLASH_ProgramPhrase(addr, (LWord *)&p[offset]);
    __enable_irq();
    offset += 8;
    DataLength -= 8;
    addr = addr + 8;
  }

  if (DataLength > 0) {
    memset(tmp, 0xFF, 8);
    memcpy(tmp, &p[offset], DataLength);
    __disable_irq();
    FLASH_ProgramPhrase(addr, (LWord *)tmp);
    __enable_irq();
  }
  
  return kNoErr;
}


/**
* @brief  Returns the write protection status of user flash area.
* @param  None
* @retval 0: No write protected sectors inside the user flash area
*         1: Some sectors inside the user flash area are write protected
*/
uint16_t _PlatformFlashGetWriteProtectionStatus(void)
{
  return 0;
}

/**
* @brief  Disables the write protection of user flash area.
* @param  None
* @retval 1: Write Protection successfully disabled
*         2: Error: Flash write unprotection failed
*/
uint32_t _PlatformFlashDisableWriteProtection(void)
{
  
  return (1);
}

