/**
******************************************************************************
* @file    wlan_bus_sdio.h 
* @author  William Xu
* @version V1.0.0
* @date    16-Sep-2014
* @brief   This file provides bus communication functions with Wi-Fi RF chip.
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

#include "MicoRtos.h"
#include "string.h" /* For memcpy */
#include "MicoPlatform.h"
#include "platform_config.h"
#include "PlatformLogging.h"
#include "gpio.h"
#include "sdio.h"
#include "wlan_platform_common.h"


/* Powersave functionality */
extern void MCU_CLOCKS_NEEDED( void );
extern void MCU_CLOCKS_NOT_NEEDED( void );

/******************************************************
 *             Constants
 ******************************************************/

#define COMMAND_FINISHED_CMD52_TIMEOUT_LOOPS (100000)
#define COMMAND_FINISHED_CMD53_TIMEOUT_LOOPS (100000)
#define SDIO_TX_RX_COMPLETE_TIMEOUT_LOOPS    (100000)
#define SDIO_DMA_TIMEOUT_LOOPS               (1000000)
#define MAX_TIMEOUTS                         (30)

#define SDIO_ERROR_MASK   ( SDIO_STA_CCRCFAIL | SDIO_STA_DCRCFAIL | SDIO_STA_CTIMEOUT | SDIO_STA_DTIMEOUT | SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR | SDIO_STA_STBITERR )

/**
 * Transfer direction for the mico platform bus interface
 */
typedef enum
{
    /* If updating this enum, the bus_direction_mapping variable will also need to be updated */
    BUS_READ,
    BUS_WRITE
} bus_transfer_direction_t;


//static const uint32_t bus_direction_mapping[] =
//{
//    [BUS_READ]  = SDIO_TransferDir_ToSDIO,
//    [BUS_WRITE] = SDIO_TransferDir_ToCard
//};

//#define SDIO_IRQ_CHANNEL       ((u8)0x31)
//#define DMA2_3_IRQ_CHANNEL     ((u8)DMA2_Stream3_IRQn)

//#define WL_GPIO_INTR_PIN_NUM   EXTI_PinSource0
//#define WL_GPIO_INTR_PORT_SRC  EXTI_PortSourceGPIOB
//#define WL_GPIO_INTR_ILINE     EXTI_Line0
//#define WL_GPIO_INTR_CHAN      0x06

#define BUS_LEVEL_MAX_RETRIES   5

#define SDIO_ENUMERATION_TIMEOUT_MS    (500)


/******************************************************
 *                   Enumerations
 ******************************************************/

/*
 * SDIO specific constants
 */
typedef enum
{
    SDIO_CMD_0  =  0,
    SDIO_CMD_3  =  3,
    SDIO_CMD_5  =  5,
    SDIO_CMD_7  =  7,
    SDIO_CMD_52 = 52,
    SDIO_CMD_53 = 53,
    __MAX_VAL   = 64
} sdio_command_t;

typedef enum
{
    SDIO_BLOCK_MODE = ( 0 << 2 ), /* These are STM32 implementation specific */
    SDIO_BYTE_MODE  = ( 1 << 2 )  /* These are STM32 implementation specific */
} sdio_transfer_mode_t;

typedef enum
{
    SDIO_1B_BLOCK    =  1,
    SDIO_2B_BLOCK    =  2,
    SDIO_4B_BLOCK    =  4,
    SDIO_8B_BLOCK    =  8,
    SDIO_16B_BLOCK   =  16,
    SDIO_32B_BLOCK   =  32,
    SDIO_64B_BLOCK   =  64,
    SDIO_128B_BLOCK  =  128,
    SDIO_256B_BLOCK  =  256,
    SDIO_512B_BLOCK  =  512,
    SDIO_1024B_BLOCK = 1024,
    SDIO_2048B_BLOCK = 2048
} sdio_block_size_t;

typedef enum
{
    RESPONSE_NEEDED,
    NO_RESPONSE
} sdio_response_needed_t;

/******************************************************
 *             Structures
 ******************************************************/

typedef struct
{
    /*@shared@*/ /*@null@*/ uint8_t* data;
    uint16_t length;
} sdio_dma_segment_t;

/******************************************************
 *             Variables
 ******************************************************/


static mico_semaphore_t sdio_transfer_finished_semaphore;

/******************************************************
 *             Function declarations
 ******************************************************/
OSStatus host_platform_sdio_transfer( bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size, sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response );
extern void wiced_platform_notify_irq( void );

/******************************************************
 *             Function definitions
 ******************************************************/

#ifndef SDIO_1_BIT
  #error "Current platform is unsupported"
#endif


#if defined  (MICO_DISABLE_MCU_POWERSAVE) && !(defined (SDIO_1_BIT)) //SDIO 4 Bit mode and enable MCU powersave, need an OOB interrupt
static void sdio_oob_irq_handler( void* arg )
{
    UNUSED_PARAMETER(arg);
    platform_mcu_powersave_exit_notify( );
    wiced_platform_notify_irq( );
}

OSStatus host_enable_oob_interrupt( void )
{
    platform_gpio_init( &wifi_sdio_pins[EMW1062_PIN_SDIO_OOB_IRQ], INPUT_HIGH_IMPEDANCE );
    platform_gpio_irq_enable( &wifi_sdio_pins[EMW1062_PIN_SDIO_OOB_IRQ], IRQ_TRIGGER_RISING_EDGE, sdio_oob_irq_handler, 0 );
    return kNoErr;
}
#endif


#ifdef SDIO_1_BIT
static void sdio_int_pin_irq_handler( void* arg )
{
    UNUSED_PARAMETER(arg);
    wiced_platform_notify_irq( );
}

bool host_platform_is_sdio_int_asserted(void)
{
    if ( platform_gpio_input_get( &wifi_sdio_pins[WIFI_PIN_SDIO_IRQ] ) == true) //SDIO INT pin is high
        return false;
    else
        return true; // SDIO D1 is low, data need read
}

OSStatus host_enable_oob_interrupt( void )
{
    return kNoErr;
}

#endif

static bool waiting = false;

void SdInterrupt(void)
{
    /* Clear interrupt */
    SdioDataInterruptClear();

    if(waiting == true){
      mico_rtos_set_semaphore( &sdio_transfer_finished_semaphore );
      waiting = false;
    }else{
      //platform_log("Unexcepted!");
    }
}

uint8_t host_platform_get_oob_interrupt_pin( void )
{
  return 0;
}

OSStatus host_platform_bus_init( void )
{
   OSStatus result;

    platform_mcu_powersave_disable();

    result = mico_rtos_init_semaphore( &sdio_transfer_finished_semaphore, 1);


    if ( result != kNoErr )
    {
        return result;
    }

   GpioSdIoConfig(1);
    
#ifdef SDIO_1_BIT
    platform_gpio_init( &wifi_sdio_pins[WIFI_PIN_SDIO_IRQ], INPUT_PULL_UP );
    platform_gpio_irq_enable( &wifi_sdio_pins[WIFI_PIN_SDIO_IRQ], IRQ_TRIGGER_FALLING_EDGE, sdio_int_pin_irq_handler, 0 );
#endif    
    
   ClkModuleEn( SD_CLK_EN );
   SdioControllerInit();
   SdioSetClk(0);
   NVIC_EnableIRQ(SD_IRQn);
   //SdioEnableClk();

    platform_mcu_powersave_enable();

    return kNoErr;
}

OSStatus host_platform_sdio_enumerate( void )
{
   OSStatus       result;
   uint32_t       loop_count;
   uint32_t       data = 0;

   loop_count = 0;
   do
   {
       /* Send CMD0 to set it to idle state */
       host_platform_sdio_transfer( BUS_WRITE, SDIO_CMD_0, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, NO_RESPONSE, NULL );

       /* CMD5. */
       host_platform_sdio_transfer( BUS_READ, SDIO_CMD_5, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, NO_RESPONSE, NULL );

       /* Send CMD3 to get RCA. */
       result = host_platform_sdio_transfer( BUS_READ, SDIO_CMD_3, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, RESPONSE_NEEDED, &data );
       loop_count++;
       if ( loop_count >= (uint32_t) SDIO_ENUMERATION_TIMEOUT_MS )
       {
           return kTimeoutErr;
       }
   } while ( ( result != kNoErr ) && ( mico_thread_msleep( (uint32_t) 1 ), ( 1 == 1 ) ) );
   /* If you're stuck here, check the platform matches your hardware */

   /* Send CMD7 with the returned RCA to select the card */
   host_platform_sdio_transfer( BUS_WRITE, SDIO_CMD_7, SDIO_BYTE_MODE, SDIO_1B_BLOCK, data, 0, 0, RESPONSE_NEEDED, NULL );

    return kNoErr;
}

OSStatus host_platform_bus_deinit( void )
{
    return kNoErr;
}

OSStatus host_platform_sdio_transfer( bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size, sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response )
{
  UNUSED_PARAMETER(mode);
  OSStatus result = kNoErr;
  uint16_t attempts = 0;
  uint8_t response_full[6];
  uint32_t blockNumber, blockIdx;
  bool status;

   check_string(!((command == SDIO_CMD_53) && (data == NULL)), "Bad args" );

   if ( response != NULL )
   {
       *response = 0;
   }

   platform_mcu_powersave_disable();


restart:
    ++attempts;

    /* Check if we've tried too many times */
    if (attempts >= (uint16_t) BUS_LEVEL_MAX_RETRIES)
    {
        result = kGeneralErr;
        platform_log("SDIO error!");
        mico_thread_sleep(0xFFFFFFFF);
        goto exit;
    }

    /* Prepare the data transfer register */
    if ( command == SDIO_CMD_53 )
    {
      
      if ( direction == BUS_READ )
      {
        if(mode == SDIO_BYTE_MODE)
             SdioStartReciveData((uint8_t*)data, data_size);
        else
             SdioStartReciveData((uint8_t*)data, block_size);
          
          //require_noerr_quiet(SdioSendCommand(command, argument, 2), restart);
          SdioDataInterruptEn();
          waiting = true;
          status = SdioSendCommand(command, argument, 1);
          if(status != 0){
            goto restart;
          }

          result = mico_rtos_get_semaphore( &sdio_transfer_finished_semaphore, (uint32_t) 50 );
          waiting = false;
          if ( result != kNoErr )
          {
            printf("@@, %d", data_size);
            //SdioDataInterruptDis();
            goto restart;
          }

          //while(!SdioIsDatTransDone());
          if(mode == SDIO_BLOCK_MODE){
            blockNumber = argument & (uint32_t) ( 0x1FF ) ;
            for( blockIdx = 1; blockIdx < blockNumber; blockIdx++ ){
              SdioStartReciveData( (uint8_t *)data + blockIdx * block_size, block_size);
              waiting = true;
              result = mico_rtos_get_semaphore( &sdio_transfer_finished_semaphore, (uint32_t) 50 );
              waiting = false;
              if ( result != kNoErr )
              {
                printf("@, %d", data_size);
                //SdioDataInterruptDis();
                goto restart;
              }
              //while(!SdioIsDatTransDone());
            }
          }
          SdioEndDatTrans();
          SdioDataInterruptDis();
          //memcpy( data, temp_dma_buffer, (size_t) data_size );
       }else{         
         //memcpy(temp_dma_buffer, data, data_size);
         //require_noerr_quiet(SdioSendCommand(command, argument, 2), restart);
         status = SdioSendCommand(command, argument, 1);
          if(status != 0){
            goto restart;
          }

          SdioDataInterruptEn();
         if(mode == SDIO_BYTE_MODE){
            waiting = true;
            SdioStartSendData((uint8_t *)data, data_size);
            result = mico_rtos_get_semaphore( &sdio_transfer_finished_semaphore, (uint32_t) 50 );
            waiting = false;
            if ( result != kNoErr )
            {
              printf("$, %d", data_size);
              //SdioDataInterruptDis();
              goto restart;
            }
            //while(!SdioIsDatTransDone());
         }else{
            
            blockNumber = argument & (uint32_t) ( 0x1FF ) ;
            for( blockIdx = 0; blockIdx < blockNumber; blockIdx++ ){
              SdioStartSendData( (uint8_t *)data + blockIdx * block_size, block_size);
              waiting = true;
              result = mico_rtos_get_semaphore( &sdio_transfer_finished_semaphore, (uint32_t) 50 );
              waiting = false;
              if ( result != kNoErr )
              {
                printf("-, %d", data_size);
                //SdioDataInterruptDis();
                goto restart;
              }
              //while(!SdioIsDatTransDone());
            }
         }
         SdioEndDatTrans();
         SdioDataInterruptDis();
       }
       
    }
    else
    {

      status = SdioSendCommand(command, argument, 50);
      
      if( response_expected == RESPONSE_NEEDED  && status != 0  ) {
        goto restart;
      }
    }

    if ( response != NULL )
    {
      SdioGetCmdResp(response_full, 6);
      *response = hton32(*(uint32_t *)&response_full[1]);
    }

    result = kNoErr;

exit:
      platform_mcu_powersave_enable();
    return result;
}


void host_platform_enable_high_speed_sdio( void )
{

}




