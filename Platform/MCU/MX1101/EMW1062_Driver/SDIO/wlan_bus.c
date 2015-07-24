/**
******************************************************************************
* @file    wlan_bus.h 
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
#include "platform_common_config.h"
#include "PlatformLogging.h"
#include "gpio.h"
#include "sdio.h"

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

static uint8_t   temp_dma_buffer[2*1024];
static uint8_t*  user_data;
static uint32_t  user_data_size;
static uint8_t*  dma_data_source;
static uint32_t  dma_transfer_size;

static mico_semaphore_t sdio_transfer_finished_semaphore;

static bool             sdio_transfer_failed;
static bus_transfer_direction_t current_transfer_direction;
static uint32_t                 current_command;

/******************************************************
 *             Function declarations
 ******************************************************/

static uint32_t          sdio_get_blocksize_dctrl   ( sdio_block_size_t block_size );
static sdio_block_size_t find_optimal_block_size    ( uint32_t data_size );
static void              sdio_prepare_data_transfer ( bus_transfer_direction_t direction, sdio_block_size_t block_size, /*@unique@*/ uint8_t* data, uint16_t data_size ) /*@modifies dma_data_source, user_data, user_data_size, dma_transfer_size@*/;

void dma_irq ( void );
OSStatus host_platform_sdio_transfer( bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size, sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response );
extern void wiced_platform_notify_irq( void );

/******************************************************
 *             Function definitions
 ******************************************************/

static void sdio_oob_irq_handler( void* arg )
{
    UNUSED_PARAMETER(arg);
    wiced_platform_notify_irq( );
}

static void sdio_irq_handler( void* arg )
{
    UNUSED_PARAMETER(arg);
    wiced_platform_notify_irq( );
}

bool host_platform_is_sdio_int_asserted(void)
{
  if( MicoGpioInputGet(SDIO_INT)==0 )
    return true;
  else
    return false;
}
bool waiting = false;

void SdInterrupt(void)
{
    OSStatus result = kNoErr;
    uint8_t sdioStatus = 0;

    /* Clear interrupt */
    SdioDataInterruptClear();

    // sdioStatus = SdioGetStatus();
    // if( result != kNoErr )
    //   return;

    if(waiting == true){
      result = mico_rtos_set_semaphore( &sdio_transfer_finished_semaphore );
      waiting = false;
    }else{
      //platform_log("Unexcepted!");

    }

    /* check result if in debug mode */
    //check_string(result == kNoErr, "failed to set dma semaphore" );

    /*@-noeffect@*/
    //(void) result; /* ignore result if in release mode */
    /*@+noeffect@*/
}

void sdio_irq( void )
{
//    uint32_t intstatus = SDIO->STA;

//    if ( ( intstatus & ( SDIO_STA_CCRCFAIL | SDIO_STA_DCRCFAIL | SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR  | SDIO_STA_STBITERR )) != 0 )
//    {
//        sdio_transfer_failed = true;
//        SDIO->ICR = (uint32_t) 0xffffffff;
//        mico_rtos_set_semaphore( &sdio_transfer_finished_semaphore );
//    }
//    else
//    {
//        if ((intstatus & (SDIO_STA_CMDREND | SDIO_STA_CMDSENT)) != 0)
//        {
//            if ( ( SDIO->RESP1 & 0x800 ) != 0 )
//            {
//                sdio_transfer_failed = true;
//                mico_rtos_set_semaphore( &sdio_transfer_finished_semaphore );
//            }
//            else if (current_command == SDIO_CMD_53)
//            {
//                if (current_transfer_direction == BUS_WRITE)
//                {
//                    DMA2_Stream3->CR = DMA_DIR_MemoryToPeripheral |
//                                       DMA_Channel_4 | DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
//                                       DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
//                                       DMA_Mode_Normal | DMA_Priority_VeryHigh |
//                                       DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4 | DMA_SxCR_PFCTRL | DMA_SxCR_EN | DMA_SxCR_TCIE;
//                }
//                else
//                {
//                    DMA2_Stream3->CR = DMA_DIR_PeripheralToMemory |
//                                       DMA_Channel_4 | DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
//                                       DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
//                                       DMA_Mode_Normal | DMA_Priority_VeryHigh |
//                                       DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4 | DMA_SxCR_PFCTRL | DMA_SxCR_EN | DMA_SxCR_TCIE;
//                }
//            }

//            /* Clear all command/response interrupts */
//            SDIO->ICR = (SDIO_STA_CMDREND | SDIO_STA_CMDSENT);
//        }

//        /* Check whether the external interrupt was triggered */
//        if ( ( intstatus & SDIO_STA_SDIOIT ) != 0 )
//        {
//            /* Clear the interrupt and then inform WICED thread */
//            SDIO->ICR = SDIO_ICR_SDIOITC;
//            wiced_platform_notify_irq( );
//        }
//    }
}

/*@-exportheader@*/ /* Function picked up by linker script */
void dma_irq( void )
{
//    OSStatus result;

//    /* Clear interrupt */
//    DMA2->LIFCR = (uint32_t) (0x3F << 22);

//    result = mico_rtos_set_semaphore( &sdio_transfer_finished_semaphore );

//    /* check result if in debug mode */
//    check_string(result == kNoErr, "failed to set dma semaphore" );

//    /*@-noeffect@*/
//    (void) result; /* ignore result if in release mode */
//    /*@+noeffect@*/
}
/*@+exportheader@*/

static void sdio_enable_bus_irq( void )
{

 //   SDIO->MASK = SDIO_MASK_SDIOITIE | SDIO_MASK_CMDRENDIE | SDIO_MASK_CMDSENTIE;
}

static void sdio_disable_bus_irq( void )
{
 //   SDIO->MASK = 0;
}

OSStatus host_enable_oob_interrupt( void )
{
    /* Set GPIO_B[1:0] to input. One of them will be re-purposed as OOB interrupt */
    MicoGpioInitialize( (mico_gpio_t)WL_GPIO0, OUTPUT_OPEN_DRAIN_NO_PULL );
    MicoGpioInitialize( (mico_gpio_t)WL_GPIO1, INPUT_HIGH_IMPEDANCE );
    //MicoGpioEnableIRQ( (mico_gpio_t)WL_GPIO1, IRQ_TRIGGER_RISING_EDGE, sdio_oob_irq_handler, 0 );

    return kNoErr;
}

uint8_t host_platform_get_oob_interrupt_pin( void )
{
  return 1;
}

OSStatus host_platform_bus_init( void )
{
   OSStatus result;

   MCU_CLOCKS_NEEDED();

    result = mico_rtos_init_semaphore( &sdio_transfer_finished_semaphore, 1);

    //result = mico_rtos_set_semaphore( &sdio_transfer_finished_semaphore);

    if ( result != kNoErr )
    {
        return result;
    }

    // mico_rtos_get_semaphore( &sdio_transfer_finished_semaphore, (uint32_t) 10000 );

   GpioSdIoConfig(1);
   SdioControllerInit();
   SdioSetClk(0);
   NVIC_EnableIRQ(SD_IRQn);
   //SdioEnableClk();

    MCU_CLOCKS_NOT_NEEDED();

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
//    NVIC_InitTypeDef nvic_init_structure;
//    OSStatus   result = kNoErr;

//    result = mico_rtos_deinit_semaphore( &sdio_transfer_finished_semaphore );

//    MCU_CLOCKS_NEEDED();


//    /* Disable SPI and SPI DMA */
//    sdio_disable_bus_irq( );
//    SDIO_ClockCmd( DISABLE );
//    SDIO_SetPowerState( SDIO_PowerState_OFF );
//    SDIO_DeInit( );
////    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2, DISABLE );
//    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SDIO, DISABLE );

//    /* Clear GPIO pins for SDIO data & clock */
//    SDIO_CMD_BANK->MODER  &= ((uint32_t) ~(3 << (2*SDIO_CMD_PIN)));
//    SDIO_CLK_BANK->MODER  &= ((uint32_t) ~(3 << (2*SDIO_CLK_PIN)));
//    SDIO_D0_BANK->MODER   &= ((uint32_t) ~(3 << (2*SDIO_D0_PIN )));
//    SDIO_D1_BANK->MODER   &= ((uint32_t) ~(3 << (2*SDIO_D1_PIN )));
//    SDIO_D2_BANK->MODER   &= ((uint32_t) ~(3 << (2*SDIO_D2_PIN )));
//    SDIO_D3_BANK->MODER   &= ((uint32_t) ~(3 << (2*SDIO_D3_PIN )));
//    SDIO_CMD_BANK->OTYPER &= ((uint32_t) ~(GPIO_OType_OD << SDIO_CMD_PIN));
//    SDIO_CLK_BANK->OTYPER &= ((uint32_t) ~(GPIO_OType_OD << SDIO_CLK_PIN));
//    SDIO_D0_BANK->OTYPER  &= ((uint32_t) ~(GPIO_OType_OD << SDIO_D0_PIN ));
//    SDIO_D1_BANK->OTYPER  &= ((uint32_t) ~(GPIO_OType_OD << SDIO_D1_PIN ));
//    SDIO_D2_BANK->OTYPER  &= ((uint32_t) ~(GPIO_OType_OD << SDIO_D2_PIN ));
//    SDIO_D3_BANK->OTYPER  &= ((uint32_t) ~(GPIO_OType_OD << SDIO_D3_PIN ));

//    /* Clear GPIO_B[1:0] */
//    MicoGpioFinalize( (mico_gpio_t)WL_GPIO0 );
//    MicoGpioFinalize( (mico_gpio_t)WL_GPIO1 );

//    /* Turn off SDIO IRQ */
//    nvic_init_structure.NVIC_IRQChannel                   = SDIO_IRQ_CHANNEL;
//    nvic_init_structure.NVIC_IRQChannelCmd                = DISABLE;
//    nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 0x0;
//    nvic_init_structure.NVIC_IRQChannelSubPriority        = 0;
//    NVIC_Init( &nvic_init_structure );

//    MCU_CLOCKS_NOT_NEEDED();

    return 0;
}

OSStatus host_platform_sdio_transfer( bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size, sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response )
{
  UNUSED_PARAMETER(mode);
  uint32_t loop_count = 0;
   OSStatus result = kNoErr;
   uint16_t attempts = 0;
   uint8_t response_full[6];
  uint32_t response_tmp;
  uint32_t blockNumber, blockIdx;
  bool status;

   check_string(!((command == SDIO_CMD_53) && (data == NULL)), "Bad args" );

   if ( response != NULL )
   {
       *response = 0;
   }

    MCU_CLOCKS_NEEDED();

restart:
//    SDIO->ICR = (uint32_t) 0xFFFFFFFF;
    sdio_transfer_failed = false;
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
    current_command = command;
    if ( command == SDIO_CMD_53 )
    {
      
      if ( direction == BUS_READ )
      {
        if(mode == SDIO_BYTE_MODE)
             SdioStartReciveData(temp_dma_buffer, data_size);
        else
             SdioStartReciveData(temp_dma_buffer, block_size);
          
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
              SdioStartReciveData( (uint8_t *)temp_dma_buffer + blockIdx * block_size, block_size);
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
          memcpy( data, temp_dma_buffer, (size_t) data_size );
       }else{         
         memcpy(temp_dma_buffer, data, data_size);
         //require_noerr_quiet(SdioSendCommand(command, argument, 2), restart);
         status = SdioSendCommand(command, argument, 1);
          if(status != 0){
            goto restart;
          }

          SdioDataInterruptEn();
         if(mode == SDIO_BYTE_MODE){
            waiting = true;
            SdioStartSendData((uint8_t *)temp_dma_buffer, data_size);
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
              SdioStartSendData( (uint8_t *)temp_dma_buffer + blockIdx * block_size, block_size);
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

      status = SdioSendCommand(command, argument, 5);
      
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
    MCU_CLOCKS_NOT_NEEDED();
    return result;
}


static void sdio_prepare_data_transfer( bus_transfer_direction_t direction, sdio_block_size_t block_size, /*@unique@*/ uint8_t* data, uint16_t data_size ) /*@modifies dma_data_source, user_data, user_data_size, dma_transfer_size@*/
{
    /* Setup a single transfer using the temp buffer */
   user_data         = data;
   user_data_size    = data_size;
   dma_transfer_size = (uint32_t) ( ( ( data_size + (uint16_t) block_size - 1 ) / (uint16_t) block_size ) * (uint16_t) block_size );

   if ( direction == BUS_WRITE )
   {
       dma_data_source = data;
   }
   else
   {
       dma_data_source = temp_dma_buffer;
   }

//    SDIO->DTIMER = (uint32_t) 0xFFFFFFFF;
//    SDIO->DLEN   = dma_transfer_size;
//    SDIO->DCTRL  = (uint32_t)sdio_get_blocksize_dctrl(block_size) | bus_direction_mapping[(int)direction] | SDIO_TransferMode_Block | SDIO_DPSM_Enable | (1 << 3) | (1 << 11);

//    /* DMA2 Stream3 */
//    DMA2_Stream3->CR   = 0;
//    DMA2->LIFCR        = (uint32_t) ( 0x3F << 22 );
//    DMA2_Stream3->FCR  = (uint32_t) ( 0x00000021 | DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full );
//    DMA2_Stream3->PAR  = (uint32_t) &SDIO->FIFO;
//    DMA2_Stream3->M0AR = (uint32_t) dma_data_source;
//    DMA2_Stream3->NDTR = dma_transfer_size/4;
}



void host_platform_enable_high_speed_sdio( void )
{
//    SDIO_InitTypeDef sdio_init_structure;

//    sdio_init_structure.SDIO_ClockDiv       = (uint8_t) 0; /* 0 = 24MHz if SDIO clock = 48MHz */
//    sdio_init_structure.SDIO_ClockEdge      = SDIO_ClockEdge_Rising;
//    sdio_init_structure.SDIO_ClockBypass    = SDIO_ClockBypass_Disable;
//    sdio_init_structure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
//#ifndef SDIO_1_BIT
//    sdio_init_structure.SDIO_BusWide = SDIO_BusWide_4b;
//#else
//    sdio_init_structure.SDIO_BusWide = SDIO_BusWide_1b;
//#endif
//    sdio_init_structure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;

//    SDIO_DeInit( );
//    SDIO_Init( &sdio_init_structure );
//    SDIO_SetPowerState( SDIO_PowerState_ON );
//    SDIO_ClockCmd( ENABLE );
//    sdio_enable_bus_irq( );
      MicoGpioInitialize( (mico_gpio_t)SDIO_INT, INPUT_HIGH_IMPEDANCE );
  MicoGpioEnableIRQ( (mico_gpio_t)SDIO_INT, IRQ_TRIGGER_FALLING_EDGE, sdio_irq_handler, 0 );
}

static sdio_block_size_t find_optimal_block_size( uint32_t data_size )
{
    if ( data_size > (uint32_t) 256 )
        return SDIO_512B_BLOCK;
    if ( data_size > (uint32_t) 128 )
        return SDIO_256B_BLOCK;
    if ( data_size > (uint32_t) 64 )
        return SDIO_128B_BLOCK;
    if ( data_size > (uint32_t) 32 )
        return SDIO_64B_BLOCK;
    if ( data_size > (uint32_t) 16 )
        return SDIO_32B_BLOCK;
    if ( data_size > (uint32_t) 8 )
        return SDIO_16B_BLOCK;
    if ( data_size > (uint32_t) 4 )
        return SDIO_8B_BLOCK;
    if ( data_size > (uint32_t) 2 )
        return SDIO_4B_BLOCK;

    return SDIO_4B_BLOCK;
}

static uint32_t sdio_get_blocksize_dctrl(sdio_block_size_t block_size)
{
    switch (block_size)
    {
//        case SDIO_1B_BLOCK:    return SDIO_DataBlockSize_1b;
//        case SDIO_2B_BLOCK:    return SDIO_DataBlockSize_2b;
//        case SDIO_4B_BLOCK:    return SDIO_DataBlockSize_4b;
//        case SDIO_8B_BLOCK:    return SDIO_DataBlockSize_8b;
//        case SDIO_16B_BLOCK:   return SDIO_DataBlockSize_16b;
//        case SDIO_32B_BLOCK:   return SDIO_DataBlockSize_32b;
//        case SDIO_64B_BLOCK:   return SDIO_DataBlockSize_64b;
//        case SDIO_128B_BLOCK:  return SDIO_DataBlockSize_128b;
//        case SDIO_256B_BLOCK:  return SDIO_DataBlockSize_256b;
//        case SDIO_512B_BLOCK:  return SDIO_DataBlockSize_512b;
//        case SDIO_1024B_BLOCK: return SDIO_DataBlockSize_1024b;
//        case SDIO_2048B_BLOCK: return SDIO_DataBlockSize_2048b;
        default: return 0;
    }
}

void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  sdio_irq();
}

void DMA2_Stream3_IRQHandler(void)
{
  dma_irq();
}








