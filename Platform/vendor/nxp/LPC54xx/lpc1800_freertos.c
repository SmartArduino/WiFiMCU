/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
/* lwIP includes. */
#include <stdio.h>
#include "MicoRtos.h"

#include "board.h"
#include "string.h"
#include "stdio.h"

#define ATH_SPI_DMA             1
/* DMA completion flag */
volatile uint8_t bDMASPITXDoneFlag = false;
volatile uint8_t bDMASPIRXDoneFlag = false;

DMA_CHDESC_T dmaSPITxDesc, dmaSPIRxDesc;

//#define SEMAPHORE       1
#define SEMDELAY        100
//#define SPIDEBUG 1
mico_semaphore_t spi_transfer_finished_semaphore;

#define WIFI_SPI_PORTNUM1          1

extern void spi_irq_handler( );

/**
 * @brief	DMA Interrupt Handler
 * @return	None
 */
/*****************************************************************************
 * Public functiion
*****************************************************************************/
/**
 * @brief	DMA Interrupt Handler
 * @return	None
 */
extern void uart0_rx_dma_handler(void);
extern void uart0_tx_dma_handler(void);

extern void spi_cs_Set(uint8_t status);
extern void uart0_dma_tx_handler(void);
extern void uart0_dma_rx_handler(void);
void DMA_IRQHandler(void)
{
  /* Clear DMA interrupt for the channel */
  if ( (Chip_DMA_GetIntStatus(LPC_DMA) & DMA_INTSTAT_ACTIVEINT) != 0 ) {
    if ( Chip_DMA_GetActiveIntAChannels(LPC_DMA) & (1 << DMAREQ_SPI1_TX) ) {
      Chip_DMA_ClearActiveIntAChannel(LPC_DMA, DMAREQ_SPI1_TX);
      bDMASPITXDoneFlag = true;
    }
    if ( Chip_DMA_GetActiveIntAChannels(LPC_DMA) & (1 << DMAREQ_SPI1_RX) ) {
      Chip_DMA_ClearActiveIntAChannel(LPC_DMA, DMAREQ_SPI1_RX);
      bDMASPIRXDoneFlag = true;
      mico_rtos_set_semaphore( &spi_transfer_finished_semaphore );
    }
    if ( Chip_DMA_GetActiveIntAChannels(LPC_DMA) & (1 << DMAREQ_UART0_TX) ) {
      Chip_DMA_ClearActiveIntAChannel(LPC_DMA, DMAREQ_UART0_TX);
      uart0_dma_tx_handler();
    }
    if ( Chip_DMA_GetActiveIntAChannels(LPC_DMA) & (1 << DMAREQ_UART0_RX) ) {
      Chip_DMA_ClearActiveIntAChannel(LPC_DMA, DMAREQ_UART0_RX);
      uart0_dma_rx_handler();
    }
  }
}

#define SPI_FRAME_LENGTH		8

uint8_t SPI_DMA_Init(void)
{
  uint8_t ret = false;
  //bool    err = false;
  //uint32_t i = 0;
  bDMASPITXDoneFlag = bDMASPIRXDoneFlag = false;
/* DMA initialization - enable DMA clocking and reset DMA if needed */
  Chip_DMA_Init(LPC_DMA);
/* Enable DMA controller and use driver provided DMA table for current descriptors */
  Chip_DMA_Enable(LPC_DMA);
  Chip_DMA_SetSRAMBase(LPC_DMA, DMA_ADDR(Chip_DMA_Table));

  Chip_SPI_SetControlInfo(LPC_SPI1, SPI_FRAME_LENGTH, SPI_TXDATCTL_SSELN(SLAVE0));
  Chip_DMA_SetValidChannel(LPC_DMA, DMAREQ_SPI1_TX);
  
  Chip_SPI_SetControlInfo(LPC_SPI1, SPI_FRAME_LENGTH, SPI_TXDATCTL_SSELN(SLAVE0));
  Chip_DMA_SetValidChannel(LPC_DMA, DMAREQ_SPI1_RX);
  
  LPC_SYSCON->AHBMATPRIO |= 0x03 << 8;
  /* Enable DMA interrupt */
  NVIC_SetPriority(DMA_IRQn, 5);
  NVIC_EnableIRQ(DMA_IRQn);
  return ret;
}

uint8_t SPI_DMA_DeInit(void)
{
  uint8_t ret = false;
  //bool    err = false;
  
  bDMASPITXDoneFlag = bDMASPIRXDoneFlag = false;
  
  Chip_DMA_DisableChannel(LPC_DMA, DMAREQ_SPI1_TX);
  Chip_DMA_DisableIntChannel(LPC_DMA, DMAREQ_SPI1_TX);
  Chip_DMA_DisableChannel(LPC_DMA, DMAREQ_SPI1_RX);
  Chip_DMA_DisableIntChannel(LPC_DMA, DMAREQ_SPI1_RX);  

  Chip_DMA_Disable(LPC_DMA);
  Chip_DMA_DeInit(LPC_DMA);
  NVIC_DisableIRQ(DMA_IRQn);  
  ret = true;
  return ret;
}

uint8_t ADC_Init(void)
{
  return true;
}

uint8_t ADC_DeInit(void)
{
	/* Setup sequencer A for all 12 ADC channels, EOS interrupt */
#if defined(BOARD_NXP_LPCXPRESSO_54000)
  /* All pins to inactive, neither pull-up nor pull-down. */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 29, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 30, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 31, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  0, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  1, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  2, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  3, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  4, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  5, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  6, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  7, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  8, IOCON_MODE_INACT | IOCON_FUNC0 | IOCON_ANALOG_EN);

  Chip_ADC_DisableInt(LPC_ADC, (ADC_INTEN_SEQA_ENABLE | ADC_INTEN_OVRRUN_ENABLE));
  Chip_ADC_DisableSequencer(LPC_ADC, ADC_SEQA_IDX);
/* Enable ADC NVIC interrupt */
  NVIC_DisableIRQ(ADC_SEQA_IRQn);
  NVIC_DisableIRQ(ADC_SEQB_IRQn);
/* Clear all pending interrupts */
  Chip_ADC_ClearFlags(LPC_ADC, Chip_ADC_GetFlags(LPC_ADC));
  
  Chip_ADC_DeInit(LPC_ADC);
#else
#warning "No ADC setup for this example"
#endif
  return true;
}

//void QCA_SYS_DELAY(uint32_t time)
//{
//  vTaskDelay(time);
//}
/**wlan wakeup ===*/
#define GPIO_GPOWKE_PORT	0  // 0
#define GPIO_GPOWKE_PIN		22 // 29

#define GPIO_GPOPWD_PORT	0  // 0
#define GPIO_GPOPWD_PIN		3 // 29

#define GPIO_GPOFET_PORT	0
#define GPIO_GPOFET_PIN		25
/**=====spi wireless=======**/
#define GPIO_SSPSEL_PORT	1
#define GPIO_SSPSEL_PIN		15

#define GPIO_SSPCLK_PORT	1
#define GPIO_SSPCLK_PIN		6

#define GPIO_SSPMISO_PORT       1
#define GPIO_SSPMISO_PIN	14

#define GPIO_SSPMOSI_PORT	1
#define GPIO_SSPMOSI_PIN	7

#define GPIO_SPIINT_PORT        	0
#define GPIO_SPIINT_PIN		4
#define GPIO_SPIINT_INDEX	PININTSELECT0	/* PININT index used for GPIO mapping */
/**=====spi wireless end=======**/
#define GPIO_STATUS_PORT        1
#define GPIO_STATUS_PIN         2
#define GPIO_STATUS_INDEX	PININTSELECT4	/* PININT index used for GPIO mapping */

#define GPIO_BOOT_PORT          1
#define GPIO_BOOT_PIN           4
#define GPIO_BOOT_INDEX	        PININTSELECT5	/* PININT index used for GPIO mapping */

#define GPIO_STANBY_PORT        1
#define GPIO_STANBY_PIN         8
#define GPIO_STANBY_INDEX	PININTSELECT6	/* PININT index used for GPIO mapping */

#define GPIO_EASYLINK_PORT      0
#define GPIO_EASYLINK_PIN       18
#define GPIO_EASYLINK_INDEX	PININTSELECT7	/* PININT index used for GPIO mapping */

void PIN_INT0_IRQHandler(void)
{
  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX)); 
  spi_irq_handler();
}

void PIN_INT1_IRQHandler(void)
{
 // Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX)); 

}

void PIN_INT2_IRQHandler(void)
{
//  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX)); 

}

void PIN_INT3_IRQHandler(void)
{
//  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX)); 

}

void PIN_INT4_IRQHandler(void)
{
//  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_STATUS_INDEX)); 
  printf("status button\r\n");
}

void PIN_INT5_IRQHandler(void)
{
//  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_BOOT_INDEX)); 
  printf("boot button\r\n");
}

void PIN_INT6_IRQHandler(void)
{
  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_STANBY_INDEX)); 
  printf("standby button\r\n");
}

//static uint32_t _default_start_time = 0;
//static mico_timer_t _button_EL_timer;
#define RestoreDefault_TimeOut          3000  /**< Restore default and start easylink after 
                                                press down EasyLink button for 3 seconds. */

void PIN_INT7_IRQHandler(void)
{
//  int interval = -1;
  
  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_EASYLINK_INDEX)); 
  
  if ( Chip_GPIO_ReadPortBit(LPC_GPIO, GPIO_EASYLINK_PORT, GPIO_EASYLINK_PIN) == 0 ) {
    printf("easy link button\r\n");
//    _default_start_time = mico_get_time()+1;
//    mico_start_timer(&_button_EL_timer);
//  } else {
//    interval = mico_get_time() + 1 - _default_start_time;
//    if ( (_default_start_time != 0) && interval > 50 && interval < RestoreDefault_TimeOut){
//      /* EasyLink button clicked once */
      PlatformEasyLinkButtonClickedCallback();
//    }
//    mico_stop_timer(&_button_EL_timer);
//    _default_start_time = 0;
  }
}

/*
uint32_t gint0_count = 0;
void GINT0_IRQHandler(void)
{
  Chip_GPIOGP_ClearIntStatus(LPC_GINT, 0);
  gint0_count++;
  printf("GINT0 \r\n");	
}
*/

 /*  
void PIN_INT1_IRQHandler(void)
{
 int interval = -1;
  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_EASYLINK_INDEX)); 
  printf("Easylink\r\n");
 
  if ( Chip_GPIO_ReadPortBit(LPC_GPIO, GPIO_EASYLINK_PORT, GPIO_EASYLINK_PIN) == 0 ) {
    _default_start_time = mico_get_time()+1;
    mico_start_timer(&_button_EL_timer);
  } else {
    interval = mico_get_time() + 1 - _default_start_time;
    if ( (_default_start_time != 0) && interval > 50 && interval < RestoreDefault_TimeOut){
      PlatformEasyLinkButtonClickedCallback();
    }
    mico_stop_timer(&_button_EL_timer);
    _default_start_time = 0;
  }
}
*/

uint8_t STATUS_GPIO_Init(void)
{
  Chip_GPIO_SetPinState(LPC_GPIO, GPIO_STATUS_PORT, GPIO_STATUS_PIN, 1);
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_STATUS_PORT, GPIO_STATUS_PIN, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_STATUS_PORT, GPIO_STATUS_PIN);
  return true;
}

uint8_t BOOT_GPIO_Init(void)
{
  Chip_GPIO_SetPinState(LPC_GPIO, GPIO_BOOT_PORT, GPIO_BOOT_PIN, 1);
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_BOOT_PORT, GPIO_BOOT_PIN, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_BOOT_PORT, GPIO_BOOT_PIN);
  return true;
}

uint8_t STANDBY_GPIO_Init(void)
{
  Chip_GPIO_SetPinState(LPC_GPIO, GPIO_STANBY_PORT, GPIO_STANBY_PIN, 1);
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_STANBY_PORT, GPIO_STANBY_PIN, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_STANBY_PORT, GPIO_STANBY_PIN);
  

  return true;
}

uint8_t EASYLINK_GPIO_Init(void)
{
  Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EASYLINK_PORT, GPIO_EASYLINK_PIN, 1);
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_EASYLINK_PORT, GPIO_EASYLINK_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_GPIO_MODE | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_EASYLINK_PORT, GPIO_EASYLINK_PIN);
  return true;
}

uint8_t EASYLINK_GPIO_Get(void)
{
  return (Chip_GPIO_GetPinState(LPC_GPIO, GPIO_EASYLINK_PORT, GPIO_EASYLINK_PIN));
}

uint8_t RESET_GPIO_Init(void)
{
  Chip_GPIO_SetPinState(LPC_GPIO, GPIO_GPOPWD_PORT, GPIO_GPOPWD_PIN, 0);
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_GPOPWD_PORT, GPIO_GPOPWD_PIN, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_GPOPWD_PORT, GPIO_GPOPWD_PIN);
  return true;
}

uint8_t RESET_GPIO_DeInit(void)
{
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_GPOPWD_PORT, GPIO_GPOPWD_PIN, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_GPOPWD_PORT, GPIO_GPOPWD_PIN);
  return true;
}

void RESET_GPIO_Set(uint8_t status)
{
  if(status == 0) Chip_GPIO_SetPinState(LPC_GPIO, GPIO_GPOPWD_PORT, GPIO_GPOPWD_PIN, 0);
  else            Chip_GPIO_SetPinState(LPC_GPIO, GPIO_GPOPWD_PORT, GPIO_GPOPWD_PIN, 1);
}

uint8_t QCAFET_GPIO_Init(void)
{
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_GPOFET_PORT, GPIO_GPOFET_PIN, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_GPOFET_PORT, GPIO_GPOFET_PIN);
  Chip_GPIO_SetPinState(LPC_GPIO, GPIO_GPOFET_PORT, GPIO_GPOFET_PIN, 0);
  return true;
}

void QCAFET_GPIO_Set(uint8_t status)
{
  if(status == 0) Chip_GPIO_SetPinState(LPC_GPIO, GPIO_GPOFET_PORT, GPIO_GPOFET_PIN, 0);
  else            Chip_GPIO_SetPinState(LPC_GPIO, GPIO_GPOFET_PORT, GPIO_GPOFET_PIN, 1);
}

void QCALED_GPIO_Set(uint8_t status)
{
  if(status == 0) Chip_GPIO_SetPinState(LPC_GPIO, 1, 5, 0);
  else            Chip_GPIO_SetPinState(LPC_GPIO, 1, 5, 1);
}

void spi_cs_init(void)
{
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SSPSEL_PORT, GPIO_SSPSEL_PIN, IOCON_MODE_REPEATER);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_SSPSEL_PORT, GPIO_SSPSEL_PIN);
  Chip_GPIO_SetPinState(LPC_GPIO, GPIO_SSPSEL_PORT, GPIO_SSPSEL_PIN, 1);
}
void spi_cs_Set(uint8_t status)
{
  if(status == 0) Chip_GPIO_SetPinState(LPC_GPIO, GPIO_SSPSEL_PORT, GPIO_SSPSEL_PIN, 0);
  else            Chip_GPIO_SetPinState(LPC_GPIO, GPIO_SSPSEL_PORT, GPIO_SSPSEL_PIN, 1);
}

uint8_t SPI_GPIO_Init(void)
{
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, (IOCON_FUNC0 | IOCON_GPIO_MODE | IOCON_DIGITAL_EN));
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 22);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 22, 1);
  
#if (defined(BOARD_NXP_LPCXPRESSO_54000))
  spi_cs_init();
//  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SSPSEL_PORT,  GPIO_SSPSEL_PIN,  (IOCON_FUNC1 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SSEL */
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SSPCLK_PORT,  GPIO_SSPCLK_PIN,  (IOCON_FUNC2 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)); //(IOCON_FUNC5 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SCK */
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SSPMISO_PORT, GPIO_SSPMISO_PIN, (IOCON_FUNC4 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)); // (IOCON_FUNC5 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* MISO */
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SSPMOSI_PORT, GPIO_SSPMOSI_PIN, (IOCON_FUNC2 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* MOSI */

  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 2,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)); //(IOCON_FUNC5 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SCK */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 3, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)); // (IOCON_FUNC5 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* MISO */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 4, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
//  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 5, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 8, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 9, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 12, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 13, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 16, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
  
  
#else
	/* Configure your own SPI pin muxing here if needed */
#warning "No SPI pin muxing defined"
#endif
	
  return true;
}

uint8_t SPI_GPIO_DeInit(void)
{
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SSPSEL_PORT,  GPIO_SSPSEL_PIN,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SSEL */
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SSPCLK_PORT,  GPIO_SSPCLK_PIN,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SCK */
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SSPMISO_PORT, GPIO_SSPMISO_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* MISO */
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SSPMOSI_PORT, GPIO_SSPMOSI_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* MOSI */	
  return true;
}

static volatile bool mEnd, sEnd;

/* SPI master select assertion callback function */
static void SPIMasterAssert(SPIM_XFER_T *pMasterXfer)
{
	/* Indicates tha master just asserted the slave select
	   signal */
}

/* SPI master send data callback function */
static void SPIMasterSendData(SPIM_XFER_T *pMasterXfer)
{
	/* This callback is called when the master needs more data to
	   send. The pMasterXfer->pTXData buffer pointer and transfer size
	   (in items) in pMasterXfer->txCount should be updated. */

	/* If this function sets the pMasterXfer->terminate flag to true,
	   this function won't be called again and the transfer will
	   terminate when the current transmit buffer is complete. */

	/* This example sets up the entire transfer structure without
	   using this callback. */
}

/* SPI master receive data callback function */
static void SPIMasterRecvData(SPIM_XFER_T *pMasterXfer)
{
	/* This callback is called when the master needs another receive
	   buffer. The pMasterXfer->pRXData buffer pointer and transfer size
	   (in items) in pMasterXfer->rxCount shoudl be updated. */

	/* This example sets up the entire transfer structure without
	   using this callback. */
}

/* SPI master select de-assertion callback function */
static void SPIMasterDeAssert(SPIM_XFER_T *pMasterXfer)
{
  /* Indicates tha master just deasserted the slave select
     signal */
}

/* SPI master transfer done callback */
static void SPIMasterDone(SPIM_XFER_T *pMasterXfer)
{
  /* Indicates tha transfer is complete */
  mEnd = true;
}

/* SPI master driver callbacks */
static const SPIM_CALLBACKS_T spiMasterCallbacks = {
  &SPIMasterAssert,
  &SPIMasterSendData,
  &SPIMasterRecvData,
  &SPIMasterDeAssert,
  &SPIMasterDone
};


SPIM_XFER_T spiMasterXfer;

void SPI1_IRQHandler(void)
{
  uint32_t ints = Chip_SPI_GetPendingInts(LPC_SPI1);

  /* Handle SPI slave interrupts only */
  if ((ints &  (SPI_INTENSET_RXDYEN | SPI_INTENSET_RXOVEN |
                SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN | SPI_INTENSET_SSDEN)) != 0) {
  /* SPI slave handler */
    Chip_SPIM_XferHandler(LPC_SPI1, &spiMasterXfer);
  }
}


extern void SPI_IRQ_ENABLE(void);
extern void SPI_IRQ_DISABLE(void);
extern uint8_t SPI_Int_Init(void);
uint8_t SPI_Init(void)
{
  SPI_CFGSETUP_T spiSetup;
  SPIM_DELAY_CONFIG_T masterDelay;
  SPI_GPIO_Init();
  
  Chip_SPI_Init(LPC_SPI1);
  
  spiSetup.master = 1;
  spiSetup.lsbFirst = 0;
  spiSetup.mode = SPI_CLOCK_MODE3;
  Chip_SPI_ConfigureSPI(LPC_SPI1, &spiSetup);
  /* Setup master controller SSEL0 for active low select */
  Chip_SPI_SetCSPolLow(LPC_SPI1, 0);
  /* Setup master clock rate, slave clock doesn't need to be setup */
  Chip_SPIM_SetClockRate(LPC_SPI1, 20000000);

  /* Setup master delay (all chip selects) */
  masterDelay.PreDelay			= 0x01;
  masterDelay.PostDelay			= 0x01;
  masterDelay.FrameDelay		= 0x01;
  masterDelay.TransferDelay             = 0x01;
  Chip_SPIM_DelayConfig(LPC_SPI1, &masterDelay);

  /* Setup master transfer callbacks in the transfer descriptor */
  spiMasterXfer.pCB = &spiMasterCallbacks;
  Chip_SPI_SetXferSize(LPC_SPI1, 8);

	/* Setup master trasnfer options - 16 data bits per transfer, EOT, EOF */
  spiMasterXfer.options =
		SPI_TXCTL_FLEN(8) |		/* This must be enabled as a minimum, use 16 data bits */
		// SPI_TXCTL_EOT |			/* Enable this to assert and deassert SSEL for each individual byte/word, current slave functions for this example do not support this */
		// SPI_TXCTL_EOF |			/* Insert a delay between bytes/words as defined by frame delay time */
		// SPI_TXCTL_RXIGNORE |		/* Enable this to ignore incoming data, or set spiMasterXfer.pRXData16 to NULL to ignore RX data  */
		0;

  /* Transfer will terminate after current buffer is sent. If terminate is not set, the buffers
    must be setup by the callbacks		*/
  spiMasterXfer.terminate = true;

  /* Use SPI select 0 */
  spiMasterXfer.sselNum = 0;
  mico_rtos_init_semaphore(&spi_transfer_finished_semaphore, 1);
  
#ifndef ATH_SPI_DMA
  Chip_SPI_Enable(LPC_SPI1);
  /* For the SPI controller configured in master mode, enable SPI master interrupts
     for interrupt service. Do not enable SPI_INTENSET_TXDYEN. */
  Chip_SPI_EnableInts(LPC_SPI1, (SPI_INTENSET_RXDYEN |
                      SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN |
                      SPI_INTENSET_SSDEN));	
  Chip_SPI_FlushFifos(LPC_SPI1);	
  NVIC_EnableIRQ(SPI1_IRQn);
#else
//  Chip_SPI_Int_Cmd(LPC_SPI0, SPI_INTENSET_TXDYEN | SPI_INTENSET_RXDYEN | SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN, ENABLE);
  
  Chip_SPI_Enable(LPC_SPI1);
  SPI_DMA_Init();
#ifdef SEMAPHORE 
  mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
#endif
  
#endif 

  SPI_Int_Init();
  
//  EASYLINK_GPIO_Init();
 // STANDBY_GPIO_Init();
  /*
  BOOT_GPIO_Init();
  STATUS_GPIO_Init();
  
  */
  return true;
}

uint8_t SPI_DeInit(void)
{
#ifndef WIFI_SPI_PORTNUM1
#ifndef ATH_SPI_DMA
  SPI_IRQ_DISABLE();
  NVIC_ClearPendingIRQ(SPI0_IRQn);
  NVIC_DisableIRQ(SPI0_IRQn);
#else
  SPI_IRQ_DISABLE();
  
  SPI_DMA_DeInit();
  Chip_SPI_Disable(LPC_SPI0);
  SPI_GPIO_DeInit();
  Chip_SPI_DeInit(LPC_SPI0);
#endif
  
#else
 #ifndef ATH_SPI_DMA
  SPI_IRQ_DISABLE();
  NVIC_ClearPendingIRQ(SPI1_IRQn);
  NVIC_DisableIRQ(SPI1_IRQn);
#else
  SPI_IRQ_DISABLE();
  
  SPI_DMA_DeInit();
  Chip_SPI_Disable(LPC_SPI1);
  SPI_GPIO_DeInit();
  Chip_SPI_DeInit(LPC_SPI1);
#endif 
#endif
  return true;
}


uint8_t SPI_Int_Init(void)
{

  Chip_PININT_Init(LPC_PININT);
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SPIINT_PORT, GPIO_SPIINT_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN  | IOCON_GPIO_MODE));
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_SPIINT_PORT, GPIO_SPIINT_PIN);

  /* Configure pin interrupt selection for the GPIO pin in Input Mux Block */
  Chip_INMUX_PININT_Config(LPC_INMUX, GPIO_SPIINT_INDEX, GPIO_SPIINT_PORT, GPIO_SPIINT_PIN);

  /* Configure channel interrupt as edge sensitive and falling edge interrupt */
  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX));
  Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX));
  Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX));
//  Chip_PININT_EnableIntHigh(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX));
  
//  NVIC_SetPriority(PININT0_IRQn, 1);
  /* Enable interrupt in the NVIC */
  NVIC_ClearPendingIRQ(PININT0_IRQn);
  NVIC_EnableIRQ(PININT0_IRQn);	
  
  
  /* Configure pin interrupt selection for the GPIO pin in Input Mux Block */
  Chip_INMUX_PININT_Config(LPC_INMUX, GPIO_EASYLINK_INDEX, GPIO_EASYLINK_PORT, GPIO_EASYLINK_PIN);

  /* Configure channel interrupt as edge sensitive and falling edge interrupt */
  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_EASYLINK_INDEX));
  Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(GPIO_EASYLINK_INDEX));
//  Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(GPIO_EASYLINK_INDEX));
  Chip_PININT_EnableIntHigh(LPC_PININT, PININTCH(GPIO_EASYLINK_INDEX));

  /* Enable interrupt in the NVIC */
  NVIC_ClearPendingIRQ(PININT7_IRQn);
  NVIC_EnableIRQ(PININT7_IRQn);
  
  Chip_INMUX_PININT_Config(LPC_INMUX, GPIO_STANBY_INDEX, GPIO_STANBY_PORT, GPIO_STANBY_PIN);

  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_STANBY_INDEX));
  Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(GPIO_STANBY_INDEX));
  Chip_PININT_EnableIntHigh(LPC_PININT, PININTCH(GPIO_STANBY_INDEX));

  NVIC_ClearPendingIRQ(PININT6_IRQn);
  NVIC_EnableIRQ(PININT6_IRQn);  
  
  return true;
}

uint8_t SPI_Int_DeInit(void)
{
  NVIC_DisableIRQ(PININT0_IRQn);	
  NVIC_ClearPendingIRQ(PININT0_IRQn);
  
  Chip_SPI_DeInit(LPC_SPI1);  

  return true;
}

void SPI_Endian_Set(uint32_t ENDIAN)
{

}

void SPI_DATA_START(void)
{
  /* Setup master trasnfer options - 16 data bits per transfer, EOT, EOF */
  spiMasterXfer.options =
          SPI_TXCTL_FLEN(8) |		/* This must be enabled as a minimum, use 16 data bits */
          // SPI_TXCTL_EOT |			/* Enable this to assert and deassert SSEL for each individual byte/word, current slave functions for this example do not support this */
          // SPI_TXCTL_EOF |			/* Insert a delay between bytes/words as defined by frame delay time */
          // SPI_TXCTL_RXIGNORE |		/* Enable this to ignore incoming data, or set spiMasterXfer.pRXData16 to NULL to ignore RX data  */
          0;

  /* Transfer will terminate after current buffer is sent. If terminate is not set, the buffers
    must be setup by the callbacks		*/
  spiMasterXfer.terminate = true;

  /* Use SPI select 0 */
  spiMasterXfer.sselNum = 0;
  mEnd = false;
}

void SPI_DATA_STOP(void)
{
#ifndef ATH_SPI_DMA
  /* Sleep until transfers are complete */
  while (mEnd == false) {
    //__WFI();
    asm("wfi");
  }
#else
  /*Force an end to the current transfer */
  Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_EOT);  
#endif
}

extern volatile uint32_t timer0_cnt;
uint32_t SPI_DATA_READ(uint8_t * pBuffer, uint32_t length)
{
  uint32_t transfer_size;
  OSStatus result;
  spi_cs_Set(0);
  
#ifndef WIFI_SPI_PORTNUM1
  
#ifndef ATH_SPI_DMA
  spiMasterXfer.pTXData8 = NULL;
  spiMasterXfer.txCount = length;/* Count is in transfer size */	
  spiMasterXfer.pRXData8 = pBuffer;
  spiMasterXfer.rxCount = length;/* Count is in transfer size */	
	
  Chip_SPIM_Xfer(LPC_SPI0, &spiMasterXfer);	
	
  while (spiMasterXfer.dataRXferred != length) {
    //__WFI();
    asm("wfi");
  }
  
  if(length == 1) {
    if(spiMasterXfer.dataRXferred == length) {
      mEnd = true;
    }
  }
  
  transfer_size = spiMasterXfer.dataRXferred;
#else
  bDMASPITXDoneFlag = false;
  bDMASPIRXDoneFlag = false;
  memset(pBuffer, 0xFF, length);
	Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

  timer0_cnt=0;
  LPC_CTIMER0->TC = 0;
   if(length<=1000) {
    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI0_RX, DMA_ADDR(&LPC_SPI0->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          length, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, length);

    Chip_DMA_InitChannel( DMAREQ_SPI0_TX, DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI0->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length);
  }
  else {
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI0_RX, DMA_ADDR(&LPC_SPI0->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI0_TX, DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI0->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, 1000);
#ifdef SEMAPHORE
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while(bDMASPITXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  } 
  while(bDMASPIRXDoneFlag == false) {
    //__WFI();s
    asm("wfi");
  }  
#else
  while(bDMASPITXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  } 
  while(bDMASPIRXDoneFlag == false) {
    //__WFI();s
    asm("wfi");
  }
#endif
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI0_RX, DMA_ADDR(&LPC_SPI0->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBuffer[1000]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          length - 1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, length - 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI0_TX, DMA_ADDR(&pBuffer[1000]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI0->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length - 1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length - 1000);    
  }  
  
#ifdef SEMAPHORE
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }  
#else 
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }  
#endif 
  
#ifdef SPIDEBUG
  if(length >= 1000) //1023)
  printf("timer0_cnt = %d, size %d\r\n", LPC_CTIMER0->TC, length);
#endif
//  Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_EOT);
  transfer_size = length;
#endif
  
#else
  

#ifndef ATH_SPI_DMA
  spiMasterXfer.pTXData8 = NULL;
  spiMasterXfer.txCount = length;/* Count is in transfer size */	
  spiMasterXfer.pRXData8 = pBuffer;
  spiMasterXfer.rxCount = length;/* Count is in transfer size */	
	
  Chip_SPIM_Xfer(LPC_SPI1, &spiMasterXfer);	
	
  while (spiMasterXfer.dataRXferred != length) {
    //__WFI();
    asm("wfi");
  }
  
  if(length == 1) {
    if(spiMasterXfer.dataRXferred == length) {
      mEnd = true;
    }
  }
  
  transfer_size = spiMasterXfer.dataRXferred;
#else
  bDMASPITXDoneFlag = false;
  bDMASPIRXDoneFlag = false;
  memset(pBuffer, 0xFF, length);
	Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

  timer0_cnt=0;
  LPC_CTIMER0->TC = 0;
   if(length<=1000) {
    Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI1_RX, DMA_ADDR(&LPC_SPI1->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          length, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, length);

    Chip_DMA_InitChannel( DMAREQ_SPI1_TX, DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI1->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length);
  }
  else {
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI1_RX, DMA_ADDR(&LPC_SPI1->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI1_TX, DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI1->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, 1000);
#ifdef SEMAPHORE
    result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
    while(bDMASPITXDoneFlag == false) {
      asm("wfi");
    } 
    while(bDMASPIRXDoneFlag == false) {
      asm("wfi");
    }
#else
    while(bDMASPITXDoneFlag == false) {
      asm("wfi");
    } 
    while(bDMASPIRXDoneFlag == false) {
      asm("wfi");
    }
#endif
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI1_RX, DMA_ADDR(&LPC_SPI1->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBuffer[1000]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          length - 1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, length - 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI1_TX, DMA_ADDR(&pBuffer[1000]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI1->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length - 1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length - 1000);    
  }  
  
#ifdef SEMAPHORE
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );  
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }  
#else 
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }  
#endif 
  
#ifdef SPIDEBUG
  if(length >= 1000) //1023)
  printf("timer0_cnt = %d, size %d\r\n", LPC_CTIMER0->TC, length);
#endif
//  Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_EOT);
  transfer_size = length;
#endif
  
#endif
  spi_cs_Set(1);
  return transfer_size;
}

uint32_t SPI_DATA_WRITE(uint8_t * pBuffer, uint32_t length)
{
  uint32_t transfer_size;
  OSStatus result;
  uint32_t junk;
  
//  printf("SPI_DATA_WRITE 0x%x\r\n", length);
  spi_cs_Set(0);
  
#ifndef WIFI_SPI_PORTNUM1
  
#ifndef ATH_SPI_DMA
  spiMasterXfer.pTXData8 = pBuffer;
  spiMasterXfer.txCount  = length;/* Count is in transfer size */	
/* Start master transfer */
  Chip_SPIM_Xfer(LPC_SPI0, &spiMasterXfer);	
//	for(i=0; i<100; i++);
  while (spiMasterXfer.dataTXferred < length) {
    //__WFI();
    asm("wfi");
  }
  
  if(length == 1) {
    if(spiMasterXfer.dataTXferred == length) {
      mEnd = true;
    }
  }
  
  transfer_size = spiMasterXfer.dataTXferred;
#else
  bDMASPITXDoneFlag = false;
  bDMASPIRXDoneFlag = false;
  timer0_cnt = 0;
  LPC_CTIMER0->TC = 0;
  if(length<=1000) {
    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI0_RX, DMA_ADDR(&LPC_SPI0->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&junk), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length);

    Chip_DMA_InitChannel( DMAREQ_SPI0_TX, DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI0->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length);
  }
  else {
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI0_RX, DMA_ADDR(&LPC_SPI0->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&junk), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI0_TX, DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI0->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, 1000);
#ifdef SEMAPHORE
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }    
#else
  while(bDMASPITXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  } 
  while(bDMASPIRXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  }
#endif
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI0_RX, DMA_ADDR(&LPC_SPI0->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&junk), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length-1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length - 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI0_TX, DMA_ADDR(&pBuffer[1000]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI0->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length-1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length - 1000);    
  }  
#ifdef SEMAPHORE
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }    
#else
  while(bDMASPITXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  } 
  while(bDMASPIRXDoneFlag == false) {
    //__WFI();s
    asm("wfi");
  }
#endif
  
#ifdef SPIDEBUG
  if(length >= 1000) // 1023)
  printf("timer0_cnt = %d, size %d\r\n", LPC_CTIMER0->TC, length);
#endif
//  Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_EOT);
  transfer_size = length;
#endif
  
#else


#ifndef ATH_SPI_DMA
  spiMasterXfer.pTXData8 = pBuffer;
  spiMasterXfer.txCount  = length;/* Count is in transfer size */	
/* Start master transfer */
  Chip_SPIM_Xfer(LPC_SPI1, &spiMasterXfer);	
//	for(i=0; i<100; i++);
  while (spiMasterXfer.dataTXferred < length) {
    //__WFI();
    asm("wfi");
  }
  
  if(length == 1) {
    if(spiMasterXfer.dataTXferred == length) {
      mEnd = true;
    }
  }
  
  transfer_size = spiMasterXfer.dataTXferred;
#else
  bDMASPITXDoneFlag = false;
  bDMASPIRXDoneFlag = false;
  timer0_cnt = 0;
  LPC_CTIMER0->TC = 0;
  if(length<=1000) {
    Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI1_RX, DMA_ADDR(&LPC_SPI1->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&junk), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length);

    Chip_DMA_InitChannel( DMAREQ_SPI1_TX, DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI1->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length);
  }
  else {
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI1_RX, DMA_ADDR(&LPC_SPI1->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&junk), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI1_TX, DMA_ADDR(&pBuffer[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI1->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, 1000);
#ifdef SEMAPHORE
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }    
#else
  while(bDMASPITXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  } 
  while(bDMASPIRXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  }
#endif
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI1_RX, DMA_ADDR(&LPC_SPI1->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&junk), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length-1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length - 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI1_TX, DMA_ADDR(&pBuffer[1000]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI1->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          length-1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, length - 1000);    
  }  
#ifdef SEMAPHORE
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }   
#else
  while(bDMASPITXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  } 
  while(bDMASPIRXDoneFlag == false) {
    //__WFI();s
    asm("wfi");
  }
#endif
  
#ifdef SPIDEBUG
  if(length >= 1000) // 1023)
  printf("timer0_cnt = %d, size %d\r\n", LPC_CTIMER0->TC, length);
#endif
//  Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_EOT);
  transfer_size = length;
#endif

#endif  
  spi_cs_Set(1);
  return transfer_size;
}

uint32_t SPI_DATA_WRITEREAD(uint8_t * pBufferW, uint8_t * pBufferR, uint32_t lengthW, uint32_t lengthR)
{
  uint32_t transfer_size;
  OSStatus result;
 // printf("SPI_DATA_WRITEREAD 0x%x\r\n", lengthW);
  spi_cs_Set(0);
#ifndef WIFI_SPI_PORTNUM1
  
  
#ifndef ATH_SPI_DMA
  spiMasterXfer.pTXData8 = pBufferW;
  spiMasterXfer.txCount  = lengthW;/* Count is in transfer size */	
  spiMasterXfer.pRXData8 = pBufferR;
  spiMasterXfer.rxCount  = lengthR;/* Count is in transfer size */		

/* Start master transfer */
  Chip_SPIM_Xfer(LPC_SPI0, &spiMasterXfer);
//	for(i=0; i<100; i++);
  while (spiMasterXfer.dataTXferred < lengthW) {
    //__WFI();
    asm("wfi");
  }
  transfer_size = spiMasterXfer.dataTXferred;
#else
  uint32_t i;
  
  bDMASPITXDoneFlag = false;
  bDMASPIRXDoneFlag = false;
  timer0_cnt = 0;
  LPC_CTIMER0->TC = 0;
   if(lengthR<=1000) {
    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI0_RX, DMA_ADDR(&LPC_SPI0->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBufferR[0]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          lengthR, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, lengthR);

    Chip_DMA_InitChannel( DMAREQ_SPI0_TX, DMA_ADDR(&pBufferW[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI0->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          lengthW, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, lengthW);
  }
  else {
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI0_RX, DMA_ADDR(&LPC_SPI0->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBufferR[0]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI0_TX, DMA_ADDR(&pBufferW[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI0->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, 1000);
#ifdef SEMAPHORE
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }    
#else
  while(bDMASPITXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  } 
  while(bDMASPIRXDoneFlag == false) {
    //__WFI();s
    asm("wfi");
  }
#endif
  
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
//    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);
    Chip_DMA_InitChannel( DMAREQ_SPI0_RX, DMA_ADDR(&LPC_SPI0->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBufferR[1000]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          lengthR-1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, lengthR - 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI0_TX, DMA_ADDR(&pBufferW[1000]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI0->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          lengthW-1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI0_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, lengthW - 1000);    
  }  
  
#ifdef SEMAPHORE
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }   
#else
   while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
 while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }   
#endif
  transfer_size = lengthR;
#endif
  
#ifdef SPIDEBUG
  if(lengthR > 1000) // 1023)
  printf("timer0_cnt = %d, size %d\r\n", LPC_CTIMER0->TC, lengthR);
#endif
  
#else
  
#ifndef ATH_SPI_DMA
  spiMasterXfer.pTXData8 = pBufferW;
  spiMasterXfer.txCount  = lengthW;/* Count is in transfer size */	
  spiMasterXfer.pRXData8 = pBufferR;
  spiMasterXfer.rxCount  = lengthR;/* Count is in transfer size */		

/* Start master transfer */
  Chip_SPIM_Xfer(LPC_SPI1, &spiMasterXfer);
//	for(i=0; i<100; i++);
  while (spiMasterXfer.dataTXferred < lengthW) {
    //__WFI();
    asm("wfi");
  }
  transfer_size = spiMasterXfer.dataTXferred;
#else
//  uint32_t i;
  
  bDMASPITXDoneFlag = false;
  bDMASPIRXDoneFlag = false;
  timer0_cnt = 0;
  LPC_CTIMER0->TC = 0;
   if(lengthR<=1000) {
    Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI1_RX, DMA_ADDR(&LPC_SPI1->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBufferR[0]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          lengthR, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, lengthR);

    Chip_DMA_InitChannel( DMAREQ_SPI1_TX, DMA_ADDR(&pBufferW[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI1->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          lengthW, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, lengthW);
  }
  else {
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    Chip_SPI_ClearStatus(LPC_SPI1, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

    Chip_DMA_InitChannel( DMAREQ_SPI1_RX, DMA_ADDR(&LPC_SPI1->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBufferR[0]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI1_TX, DMA_ADDR(&pBufferW[0]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI1->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, 1000);
#ifdef SEMAPHORE
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }    
#else
  while(bDMASPITXDoneFlag == false) {
    //__WFI();
    asm("wfi");
  } 
  while(bDMASPIRXDoneFlag == false) {
    //__WFI();s
    asm("wfi");
  }
#endif
  
    bDMASPITXDoneFlag = false;
    bDMASPIRXDoneFlag = false;
    
    Chip_DMA_InitChannel( DMAREQ_SPI1_RX, DMA_ADDR(&LPC_SPI1->RXDAT), DMA_XFERCFG_SRCINC_0, 
                          DMA_ADDR(&pBufferR[1000]), DMA_XFERCFG_DSTINC_1, WIDTH_8_BITS,
                          lengthR-1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(1)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_RX, DMA_XFERCFG_SRCINC_0, DMA_XFERCFG_DSTINC_1, DMA_XFERCFG_WIDTH_8, lengthR - 1000);

    Chip_DMA_InitChannel( DMAREQ_SPI1_TX, DMA_ADDR(&pBufferW[1000]), DMA_XFERCFG_SRCINC_1,
                          DMA_ADDR(&LPC_SPI1->TXDAT), DMA_XFERCFG_DSTINC_0, WIDTH_8_BITS,
                          lengthW-1000, (DMA_CFG_PERIPHREQEN | DMA_CFG_BURSTPOWER_1 | DMA_CFG_CHPRIORITY(2)));
    Chip_DMA_StartTransfer(DMAREQ_SPI1_TX, DMA_XFERCFG_SRCINC_1, DMA_XFERCFG_DSTINC_0, DMA_XFERCFG_WIDTH_8, lengthW - 1000);    
  }  
  
#ifdef SEMAPHORE
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, SEMDELAY );
  while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
  while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }   
#else
   while (bDMASPITXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
   //__WFI();
    asm("wfi");
  }
 while (bDMASPIRXDoneFlag == false) { // &&(bDMASPIRXDoneFlag == false)) {
    //__WFI();
    asm("wfi");
  }   
#endif
  transfer_size = lengthR;
#endif
  
#ifdef SPIDEBUG
  if(lengthR > 1000) // 1023)
  printf("timer0_cnt = %d, size %d\r\n", LPC_CTIMER0->TC, lengthR);
#endif
  
#endif
  spi_cs_Set(1);
  return transfer_size;
}

void SPI_IRQ_ENABLE(void)
{
  Chip_PININT_Init(LPC_PININT);
  Chip_IOCON_PinMuxSet(LPC_IOCON, GPIO_SPIINT_PORT, GPIO_SPIINT_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN  | IOCON_GPIO_MODE));
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_SPIINT_PORT, GPIO_SPIINT_PIN);
  /* Configure pin interrupt selection for the GPIO pin in Input Mux Block */
  Chip_INMUX_PININT_Config(LPC_INMUX, GPIO_SPIINT_INDEX, GPIO_SPIINT_PORT, GPIO_SPIINT_PIN);
  /* Configure channel interrupt as edge sensitive and falling edge interrupt */
  Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX));
  Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX));
//  Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX));
  Chip_PININT_EnableIntHigh(LPC_PININT, PININTCH(GPIO_SPIINT_INDEX));
  NVIC_SetPriority(PININT0_IRQn, 2);
  /* Enable interrupt in the NVIC */
  NVIC_ClearPendingIRQ(PININT0_IRQn);
  NVIC_EnableIRQ(PININT0_IRQn);
}

void SPI_IRQ_DISABLE(void)
{
  /* Enable interrupt in the NVIC */
  NVIC_ClearPendingIRQ(PININT0_IRQn);
  NVIC_DisableIRQ(PININT0_IRQn);
}

// ---------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------

#include "spi_flash_platform_interface.h"
//#define SPIFLASHDEBUG 1
// Initial SPI Flash
//#define SPIMASTERIRQHANDLER             SPI1_IRQHandler
#define LPC_SPIMASTERPORT               LPC_SPI1
#define LPC_SPIMASTERIRQNUM             SPI1_IRQn
/* The slave can handle data up to the point of overflow or underflow.
   Adjust this clock rate and the master's timing delays to get a working
   SPI slave rate. */
#define LPCMASTERCLOCKRATE              (4000000)
#define BUFFER_SIZE 300

#define SPI_FLASH_CMD_READ_DEVID        0x90
#define SPI_FLASH_CMD_WRITE_ENABLE      0x06
#define SPI_FLASH_CMD_WRITE_DISABLE     0x04

#define SPI_FLASH_CMD_READ_DATA         0x03
#define SPI_FLASH_CMD_READ_FAST         0x0B

#define SPI_FLASH_CMD_WRITE_DATA        0x02
#define SPI_FLASH_CMD_ERASE             0x20
#define SPI_FLASH_CMD_ERASE_32KB        0x52
#define SPI_FLASH_CMD_ERASE_64KB        0xD8
#define SPI_FLASH_CMD_ERASE_CHIP        0xC7

#define SPI_FLASH_CMD_WRITE_STATUS      0x01
#define SPI_FLASH_CMD_READ_STATUS1      0x05
#define SPI_FLASH_CMD_READ_STATUS2      0x35

#define SPI_FLASH_PAGESIZE              256  

volatile uint8_t Flash_mEnd;
/* Master transmit and receive buffers */
uint8_t masterRXBuffer8[BUFFER_SIZE], masterTXBuffer8[BUFFER_SIZE];


static void flash_spi_pinmux_init(void)
{
  /* Connect the SPI1 signals to port pins */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 6 ,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_SCK */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 16,  (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN));	/* SPI1_MISO */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 7 ,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_MOSI */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8 ,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_SSEL0 */
 
//  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 15,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
//  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 15);
//  Chip_GPIO_SetPinState(LPC_GPIO, 1, 15, 1);
  
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 6);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, 0);     /* SPI1_SCK */
  
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 16);    /* SPI1_MISO */
  Chip_GPIO_SetPinState(LPC_GPIO, 1, 16, 0);
  
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 7);    /* SPI1_MOSI */
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 1);
  
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 8);/* SPI1_SSEL0 */
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, 1);
}

#define SPI_FLASH_CS_LOW         Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, 0)
#define SPI_FLASH_CS_HIGH        Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, 1)

#define SPI_FLASH_MISO_LOW
#define SPI_FLASH_MISO_HIGH

#define SPI_FLASH_MISO_GET      Chip_GPIO_GetPinState(LPC_GPIO, 1, 16)

#define SPI_FLASH_MOSI_LOW       Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 0)
#define SPI_FLASH_MOSI_HIGH      Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 1)

#define SPI_FLASH_SCK_LOW       Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, 0)
#define SPI_FLASH_SCK_HIGH      Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, 1)

uint8_t spi_wr(uint8_t data)
{
  uint32_t i = 0;
  uint8_t ret = 0;
  
  for(i=0; i<8; i++) {
    ret = ((ret<<1)&0xFE);
    SPI_FLASH_SCK_LOW;
    if(data&0x80) {
      SPI_FLASH_MOSI_HIGH;
    }
    else{
      SPI_FLASH_MOSI_LOW;
    }
    SPI_FLASH_SCK_HIGH;
    data = (data<<1);
    if((SPI_FLASH_MISO_GET&0x01) == 1) {
      ret = ret|0x01;
    }
    else {
      ret &= 0xFE;
    }
  }

  return ret;
}

void flash_spi_master_set(void)
{
  SPI_FLASH_CS_HIGH;
  SPI_FLASH_SCK_HIGH;
}

static uint8_t spi_flash_init(void)
{
  uint8_t ret = 0;
  
   /* SPI initialization */
  flash_spi_pinmux_init();
  /* Enable SysTick Timer */
  /* Setup SPI controllers */
  flash_spi_master_set();
  SPI_FLASH_CS_LOW;
  SPI_FLASH_CS_HIGH;
  SPI_FLASH_SCK_LOW;
  SPI_FLASH_SCK_HIGH;
  SPI_FLASH_MOSI_LOW;
  SPI_FLASH_MOSI_HIGH;
  
  ret = 1;
  return ret;
}


int sflash_platform_init( int peripheral_id, void** platform_peripheral_out )
{
  uint8_t temp1, temp2;
  uint32_t j;
  spi_flash_init();
  
  SPI_FLASH_CS_LOW;
  temp1 = spi_wr(0x90);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp2 = spi_wr(0x00);
  SPI_FLASH_CS_HIGH;
  for(j=0; j<5000; j++);
  printf("Flash ID1 %x, %x, %x\r\n", temp1, temp2, j);
 
  SPI_FLASH_CS_LOW;
  temp1 = spi_wr(0x90);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp2 = spi_wr(0x00);
  SPI_FLASH_CS_HIGH;
  for(j=0; j<5000; j++);
  printf("Flash ID2 %x, %x. %x\r\n", temp1, temp2, j);
  return 0;
}

volatile uint8_t spi_flash_recv;
int sflash_platform_send_recv_byte( void* platform_peripheral, unsigned char MOSI_val, void* MISO_addr )
{
//  int ret;
  uint8_t *rcv_buf;
  if(MISO_addr != 0x00000000)
  rcv_buf = MISO_addr;
  else
  rcv_buf = (uint8_t *)&spi_flash_recv;
  *rcv_buf = spi_wr(MOSI_val);
#ifdef SPIFLASHDEBUG
  printf("SPI RW %x %d, %x\r\n", MOSI_val, 1, *rcv_buf);
#endif
  return 0;
}

int sflash_platform_chip_select( void* platform_peripheral )
{
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 8 , 0);
  return 0;
}

int sflash_platform_chip_deselect( void* platform_peripheral )
{
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 8 , 1);
  return 0;
}


// end file
