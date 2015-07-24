/*
 * @brief LPC540XX VFIFO chip driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "chip.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
uint32_t Chip_VFIFO_InitUART(LPC_VFIFO_T *pVFIFO, VFIFO_UART_DRIVER_T *uart_setup) {
  uint32_t ctrl_value;

  uart_setup->uart_rx_used_fifo_size += uart_setup->uart_rx_fifo_size;
  uart_setup->uart_tx_used_fifo_size += uart_setup->uart_tx_fifo_size;

  /* Check there is enough fifo space to support the requested size */
  ctrl_value = pVFIFO->FIFOCTLUART;
  if ((uart_setup->uart_tx_used_fifo_size > VFIFO_TXFIFO_TOTAL(ctrl_value)) ||
          (uart_setup->uart_rx_used_fifo_size > VFIFO_RXFIFO_TOTAL(ctrl_value)))
    return 1; // fail because of no enough fifo space left
  
  // Pause FIFO 
  pVFIFO->FIFOCTLUART = VFIFO_RXPAUSE | VFIFO_TXPAUSE;
  while ((pVFIFO->FIFOCTLUART & (VFIFO_RXPAUSED | VFIFO_TXPAUSED)) != (VFIFO_RXPAUSED | VFIFO_TXPAUSED));
  
  // Configure FIFO size 
  pVFIFO->FIFOCFGU[uart_setup->uart_num] = VFIFO_RXSIZE(uart_setup->uart_rx_fifo_size) | VFIFO_TXSIZE(uart_setup->uart_tx_fifo_size);
  
  // Reset FIFOs
   pVFIFO->FIFORSTUART = 0xFFFFFFFF;   
      
  // Unpause the FIFOs 
  pVFIFO->FIFOCTLUART = 0;
  
  return 0;
}

uint32_t Chip_VFIFO_InitSPI(LPC_VFIFO_T *pVFIFO, VFIFO_SPI_DRIVER_T *spi_setup) {
  uint32_t ctrl_value;

  spi_setup->spi_rx_used_fifo_size += spi_setup->spi_rx_fifo_size;
  spi_setup->spi_tx_used_fifo_size += spi_setup->spi_tx_fifo_size;

  ctrl_value = pVFIFO->FIFOCTLSPI;
  if ((spi_setup->spi_tx_used_fifo_size > VFIFO_TXFIFO_TOTAL(ctrl_value)) || (spi_setup->spi_rx_used_fifo_size > VFIFO_RXFIFO_TOTAL(ctrl_value)))
    return 1; // fail because of no enough fifo space left
  // pause FIFO 
  pVFIFO->FIFOCTLSPI = VFIFO_RXPAUSE | VFIFO_TXPAUSE;
  while ((pVFIFO->FIFOCTLSPI & (VFIFO_RXPAUSED | VFIFO_TXPAUSED)) != (VFIFO_RXPAUSED | VFIFO_TXPAUSED));
  // configure fifo size 
  pVFIFO->FIFOCFGSPI[spi_setup->spi_num] = VFIFO_RXSIZE(spi_setup->spi_rx_fifo_size) | VFIFO_TXSIZE(spi_setup->spi_tx_fifo_size);
  // Reset FIFOs
  pVFIFO->FIFORSTSPI = 0xFFFFFFFF; // VFIFO_RxRst(pSpi->id) | VFIFO_TxRst(pSpi->id)
  // enable FIFO 
  pVFIFO->FIFOCTLSPI = 0;
  return 0;
}

uint8_t Chip_VFIFO_ConfigUART(LPC_VFIFO_T *pVFIFO, VFIFO_UART_DRIVER_T *uart_setup) {
  
  pVFIFO->UART[uart_setup->uart_num].CFGU = VFIFO_RXTHRESHOLD(uart_setup->rx_threshold) | 
          VFIFO_TXTHRESHOLD(uart_setup->tx_threshold) | 
          VFIFO_TIMEOUT_BASE(uart_setup->timeout_base) | 
          VFIFO_TIMEOUT_VALUE(uart_setup->timeout_value) | 
          (uart_setup->timeout_cont_on_write ? VFIFO_TIMEOUT_CONTINUE_ON_WRITE : 0) | 
          (uart_setup->timeout_cont_on_empty ? VFIFO_TIMEOUT_CONTINUE_ON_EMPTY : 0);
  
  return 0;
}

uint8_t Chip_VFIFO_ConfigSPI(LPC_VFIFO_T *pVFIFO, VFIFO_SPI_DRIVER_T *spi_setup) {
  pVFIFO->SPI[spi_setup->spi_num].CFGSPI = VFIFO_RXTHRESHOLD(spi_setup->rx_threshold) | 
					VFIFO_TXTHRESHOLD(spi_setup->rx_threshold) | 
					VFIFO_TIMEOUT_BASE(spi_setup->timeout_base) | 
					VFIFO_TIMEOUT_VALUE(spi_setup->timeout_value) | 
					(spi_setup->timeout_cont_on_write ? VFIFO_TIMEOUT_CONTINUE_ON_WRITE : 0) | 
					(spi_setup->timeout_cont_on_empty ? VFIFO_TIMEOUT_CONTINUE_ON_EMPTY : 0);
  return 0;
}

uint8_t Chip_VFIFO_GetSPI_RXThreshold(LPC_VFIFO_T *pVFIFO, uint32_t spi_num) {
  return( (pVFIFO->SPI[spi_num].CFGSPI >> 16)&0xFF );
}

uint8_t Chip_VFIFO_GetSPI_TXThreshold(LPC_VFIFO_T *pVFIFO, uint32_t spi_num) {
  return( (pVFIFO->SPI[spi_num].CFGSPI >> 24)&0xFF );
}

/* Populate a transmit ring buffer and start UART transmit */
uint32_t Chip_VFIFO_SendUART_RB(LPC_VFIFO_T *pVFIFO, uint32_t uart_num, RINGBUFF_T *pRB, const void *data, int count) {
  uint32_t ret;
  uint8_t ch;
  uint8_t *p8 = (uint8_t *) data;

  /* Move as much data as possible into transmit ring buffer */
  ret = RingBuffer_InsertMult(pRB, p8, count);
  /* Fill FIFO until full or until TX ring buffer is empty */
  while ((Chip_VFIFO_GetUARTIntStatus(pVFIFO, uart_num) & VFIFO_ST_TXAVAILABLEMASK) &&
          RingBuffer_Pop(pRB, &ch)) {
    Chip_VFIFO_SendUARTByte(LPC_VFIFO, uart_num, ch);
  }

  /* Add additional data to transmit ring buffer if possible */
  ret += RingBuffer_InsertMult(pRB, (p8 + ret), (count - ret));
  return ret;
}

/* Copy data from a receive ring buffer */
int Chip_VFIFO_ReceiveUART_RB(LPC_VFIFO_T *pVFIFO, RINGBUFF_T *pRB, void *data, int bytes) {
  return RingBuffer_PopMult(pRB, (uint8_t *) data, bytes);
}


/* VFIFO SPI Read/Write one frame */
Status Chip_VFIFO_SPI_TransferFrame(LPC_VFIFO_T *pVFIFO, uint32_t spi_num, SPI_DATA_SETUP_T *pXfSetup) {
  uint32_t Status;
	uint32_t txctrl;
	uint32_t i;

	if ( pXfSetup->pRx == NULL ) {
		txctrl = SPI_TXCTL_FLEN(pXfSetup->DataSize-1)|SPI_TXCTL_ASSERT_SSEL|SPI_TXCTL_EOF|SPI_TXCTL_RXIGNORE;
	}
	else {
		txctrl = SPI_TXCTL_FLEN(pXfSetup->DataSize-1)|SPI_TXCTL_ASSERT_SSEL|SPI_TXCTL_EOF;
	}
	
	Status = Chip_VFIFO_GetSPIStatus(LPC_VFIFO, spi_num);	
	if (Status & VFIFO_ST_TXTHRESHOLD) {
		/* Fill FIFO until full or until TX is empty */
		for ( i = 0; i < (Chip_VFIFO_GetSPI_TXThreshold(LPC_VFIFO, spi_num) + 1); i++ ) {
			if (pXfSetup->TxCnt == 0) {
				/* First frame */
				Chip_VFIFO_SendSPIDataCtrl(LPC_VFIFO, spi_num, txctrl | SPI_TXDAT_DATA(pXfSetup->pTx[pXfSetup->TxCnt++]));
			}
			else if (pXfSetup->TxCnt == (pXfSetup->Length-1)) {
				/* Last frame */
				Chip_VFIFO_SendSPIDataCtrl(LPC_VFIFO, spi_num, txctrl | SPI_TXCTL_EOT | SPI_TXDAT_DATA(pXfSetup->pTx[pXfSetup->TxCnt++]));
				Chip_VFIFO_ClearSPIStatus(LPC_VFIFO, spi_num, VFIFO_ST_RXTIMEOUT|VFIFO_ST_BUSERROR);
				if ( pXfSetup->pRx == NULL ) {
					/* This is the end of transfer on TX, if RX_IGNORE is set, transfer has completed. */
					pXfSetup->completion_flag = 1;
				}
				break;
			}
			else {
				/* Middle frame */
				Chip_VFIFO_SendSPIData(LPC_VFIFO, spi_num, pXfSetup->pTx[pXfSetup->TxCnt++]);
			}
		}	
	}
	/* Handle receive interrupt */
	if ((Status & VFIFO_ST_RXTHRESHOLD) && (pXfSetup->pRx != NULL)) {
		for ( i = 0; i < (Chip_VFIFO_GetSPI_RXThreshold(LPC_VFIFO, spi_num) + 1); i++ ) {
			if ( pXfSetup->RxCnt < pXfSetup->Length ) {
				pXfSetup->pRx[pXfSetup->RxCnt++] = SPI_RXDAT_DATA(Chip_VFIFO_ReadSPIData(LPC_VFIFO, spi_num));
			}
			else if ( pXfSetup->RxCnt == pXfSetup->Length ) {
				Chip_VFIFO_ClearSPIStatus(LPC_VFIFO, spi_num, VFIFO_ST_RXTIMEOUT|VFIFO_ST_BUSERROR);
				/* This is the end of transfer. */
				pXfSetup->completion_flag = 1;
				break;
			}
		}				/* End VFIFO_ST_RXTHRESHOLD check */
	}
  return SUCCESS;
}

void Chip_VFIFO_Init(LPC_VFIFO_T *pVFIFO) {
  /* enable VFIFO base clock */
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_VFIFO);
}

void Chip_VFIFO_DeInit(LPC_VFIFO_T *pVFIFO) {
  /* enable VFIFO base clock */
  Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_VFIFO);
}


