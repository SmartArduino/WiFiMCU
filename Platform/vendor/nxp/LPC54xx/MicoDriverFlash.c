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

/* Private constants --------------------------------------------------------*/
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbyte */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbyte */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbyte */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbyte */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbyte */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbyte */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbyte */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbyte */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbyte */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbyte */

/* End of the Flash address */
#define FLASH_START_ADDRESS     (uint32_t)0x08000000  
#define FLASH_END_ADDRESS       (uint32_t)0x080FFFFF
#define FLASH_SIZE              (FLASH_END_ADDRESS -  FLASH_START_ADDRESS + 1)

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef USE_MICO_SPI_FLASH
static sflash_handle_t sflash_handle = {0x0, 0x0, SFLASH_WRITE_NOT_ALLOWED};
#endif
/* Private function prototypes -----------------------------------------------*/
static uint32_t _GetSector( uint32_t Address );
static OSStatus internalFlashInitialize( void );
static OSStatus internalFlashErase(uint32_t StartAddress, uint32_t EndAddress);
static OSStatus internalFlashWrite(volatile uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength);
static OSStatus internalFlashByteWrite( uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength );
static OSStatus internalFlashFinalize( void );
#ifdef USE_MICO_SPI_FLASH
static OSStatus spiFlashErase(uint32_t StartAddress, uint32_t EndAddress);
#endif

//#define DISABLEFLASH 1
static uint8_t flash_spi_flag = 0;
OSStatus MicoFlashInitialize( mico_flash_t flash )
{ 
#ifndef DISABLEFLASH
  flash_spi_flag = 1;
  platform_log_trace();
  if(flash == MICO_INTERNAL_FLASH){

  }
#ifdef USE_MICO_SPI_FLASH
  else if(flash == MICO_SPI_FLASH){
    if(sflash_handle.device_id) {
      return kNoErr;
    }
    else {
      return init_sflash( &sflash_handle, 0, SFLASH_WRITE_ALLOWED );
    }
  }  
#endif
  else {
    return kUnsupportedErr;
  }
#else
  return kNoErr;
#endif
}

OSStatus MicoFlashErase( mico_flash_t flash, uint32_t StartAddress, uint32_t EndAddress )
{ 
#ifndef DISABLEFLASH
  platform_log_trace();
  
  if(flash_spi_flag == 0) {
#ifdef USE_MICO_SPI_FLASH
    MicoFlashInitialize(MICO_SPI_FLASH);
#endif
  }
  
  if(flash == MICO_INTERNAL_FLASH){

  }
#ifdef USE_MICO_SPI_FLASH
  else if(flash == MICO_SPI_FLASH){
    if(StartAddress>=EndAddress || EndAddress > SPI_FLASH_END_ADDRESS)
      return kParamErr;
    return spiFlashErase(StartAddress, EndAddress); 
  }
#endif
  else {
    return kUnsupportedErr;
  }
#else
  return kNoErr;
#endif
}

OSStatus MicoFlashWrite(mico_flash_t flash, volatile uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength)
{
#ifndef DISABLEFLASH
  
  if(flash_spi_flag == 0) {
#ifdef USE_MICO_SPI_FLASH
    MicoFlashInitialize(MICO_SPI_FLASH);
#endif
  }
  if(flash == MICO_INTERNAL_FLASH){
    return internalFlashWrite(FlashAddress, (uint32_t *)Data, DataLength);    
  }
#ifdef USE_MICO_SPI_FLASH
  else if(flash == MICO_SPI_FLASH){
    int returnVal = sflash_write( &sflash_handle, *FlashAddress, Data, DataLength );
    *FlashAddress += DataLength;
    return returnVal;
  }
#endif
  else
    return kUnsupportedErr;
#else
  printf("MicoFlashWrite\r\n");
  return kNoErr;
#endif
}

OSStatus MicoFlashRead(mico_flash_t flash, volatile uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength)
{
#ifndef DISABLEFLASH
  
  if(flash_spi_flag == 0) {
#ifdef USE_MICO_SPI_FLASH
    MicoFlashInitialize(MICO_SPI_FLASH);
#endif
  }
  
  if(flash == MICO_INTERNAL_FLASH){
    printf("MicoFlashRead MICO_INTERNAL_FLASH\r\n");
    memcpy(Data, (void *)(*FlashAddress), DataLength);
    *FlashAddress += DataLength;
    return kNoErr;
  }
#ifdef USE_MICO_SPI_FLASH
  else if(flash == MICO_SPI_FLASH){
    
    int returnVal = sflash_read( &sflash_handle, *FlashAddress, Data, DataLength );
    *FlashAddress += DataLength;
    return returnVal;
  }
#endif
  else {
    printf("MicoFlashRead kUnsupportedErr\r\n");
    return kUnsupportedErr;
  }
#else
  return kNoErr;
#endif
}

OSStatus MicoFlashFinalize( mico_flash_t flash )
{
#ifndef DISABLEFLASH
  
  if(flash == MICO_INTERNAL_FLASH){

  }
#ifdef USE_MICO_SPI_FLASH
  else if(flash == MICO_SPI_FLASH){
    sflash_handle.device_id = 0x0;
    return kNoErr;
  }
#endif
  else
    return kUnsupportedErr;
#else
printf("MicoFlashFinalize\r\n");
  return kUnsupportedErr;
#endif
}

OSStatus internalFlashInitialize( void )
{ 
printf("internalFlashInitialize\r\n");
  return kNoErr; 
}

OSStatus internalFlashErase(uint32_t StartAddress, uint32_t EndAddress)
{
  printf("internalFlashErase\r\n");
  return kNoErr;
}

#ifdef USE_MICO_SPI_FLASH
OSStatus spiFlashErase(uint32_t StartAddress, uint32_t EndAddress)
{
#ifndef DISABLEFLASH
  
  if(flash_spi_flag == 0) {
#ifdef USE_MICO_SPI_FLASH
    MicoFlashInitialize(MICO_SPI_FLASH);
#endif
  }
  
  platform_log_trace();
  OSStatus err = kNoErr;
  uint32_t StartSector, EndSector, i = 0;
  
  /* Get the sector where start the user flash area */
  StartSector = StartAddress>>12;
  EndSector = EndAddress>>12;
  
  for(i = StartSector; i <= EndSector; i += 1)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
    be done by word */
    require_action(sflash_sector_erase(&sflash_handle, i<<12) == kNoErr, exit, err = kWriteErr); 
  }
  
exit:
  return err;
  
#else
  printf("spiFlashErase\r\n");
  return kNoErr;
#endif
}
#endif


OSStatus internalFlashWrite(volatile uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
printf("internalFlashWrite\r\n");
  return kNoErr;
}

OSStatus internalFlashFinalize( void )
{
printf("internalFlashFinalize\r\n"); 
return kNoErr;
}


OSStatus internalFlashByteWrite( uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength)
{
  printf("internalFlashByteWrite\r\n"); 
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
  printf("_PlatformFlashGetWriteProtectionStatus\r\n"); 
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
  printf("_PlatformFlashDisableWriteProtection\r\n"); 
  return (1);
}

// end file
