/*
 * @brief LPC540XX SPI driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licenser disclaim any and
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

STATIC void SPI_Send_Data_RxIgnore(LPC_SPI_T *pSPI,
								   SPI_DATA_SETUP_T *pXfSetup)
{
	if (pXfSetup->TxCnt == (pXfSetup->Length - 1)) {
		Chip_SPI_SendLastFrame_RxIgnore(pSPI, pXfSetup->pTx[pXfSetup->TxCnt], pXfSetup->DataSize);
	}
	else {
		Chip_SPI_SendMidFrame(pSPI, pXfSetup->pTx[pXfSetup->TxCnt]);
	}

	pXfSetup->TxCnt++;
}

STATIC void SPI_Send_Data(LPC_SPI_T *pSPI,
						  SPI_DATA_SETUP_T *pXfSetup)
{
	if (pXfSetup->TxCnt == (pXfSetup->Length - 1)) {
		Chip_SPI_SendLastFrame(pSPI, pXfSetup->pTx[pXfSetup->TxCnt], pXfSetup->DataSize);
	}
	else {
		Chip_SPI_SendMidFrame(pSPI, pXfSetup->pTx[pXfSetup->TxCnt]);
	}

	pXfSetup->TxCnt++;
}

STATIC void SPI_Send_Dummy(LPC_SPI_T *pSPI,
						   SPI_DATA_SETUP_T *pXfSetup)
{
	if (pXfSetup->RxCnt == (pXfSetup->Length - 1)) {
		Chip_SPI_SendLastFrame(pSPI, 0x55, pXfSetup->DataSize);
	}
	else {
		Chip_SPI_SendMidFrame(pSPI, 0x55);
	}
}

STATIC void SPI_Receive_Data(LPC_SPI_T *pSPI,
							 SPI_DATA_SETUP_T *pXfSetup)
{
	pXfSetup->pRx[pXfSetup->RxCnt] = Chip_SPI_ReceiveFrame(pSPI);
	pXfSetup->RxCnt++;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Calculate the Clock Rate Divider for SPI Peripheral */
uint32_t Chip_SPI_CalClkRateDivider(LPC_SPI_T *pSPI, uint32_t bitRate)
{
	uint32_t SPIClk;
	uint32_t DivVal = 1;

	/* Get SPI clock rate */
	SPIClk = Chip_Clock_GetAsyncSysconClockRate();	/*The peripheral clock for both SPIs is the Async Syscon clock*/

	DivVal = SPIClk / bitRate;

	return DivVal;
}

/* Initialize the SPI */
void Chip_SPI_IF_Init(LPC_SPI_T *pSPI, SPI_MODECONFIG_T *pConfig)
{
	uint32_t EnStat;

	Chip_SPI_Init(pSPI);

	EnStat = pSPI->CFG & SPI_CFG_SPI_EN;
	/* Disable before update CFG register */
	if (EnStat) {
		Chip_SPI_Disable(pSPI);
	}

	/* SPI Configurate */
	pSPI->CFG = ((uint32_t) pConfig->ClockMode) | ((uint32_t) pConfig->DataOrder) | ((uint32_t) pConfig->Mode) |
				((uint32_t) pConfig->SSELPol);

	if ( pConfig->Mode == SPI_CFG_MASTER_EN ) {
		/* Rate Divider setting */
		pSPI->DIV = SPI_DIV_VAL(pConfig->ClkDiv);
	}

	/* Clear status flag*/
	Chip_SPI_ClearStatus(pSPI, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);

	/* Return the previous state */
	if (EnStat) {
		Chip_SPI_Enable(pSPI);
	}
}

/* Disable/Enable Interrupt */
void Chip_SPI_Int_Cmd(LPC_SPI_T *pSPI, uint32_t IntMask, FunctionalState NewState)
{
	if (NewState ==  ENABLE) {
		Chip_SPI_EnableInts(pSPI, IntMask);
	}
	else {
		Chip_SPI_DisableInts(pSPI, IntMask);
	}
}

/*Send and Receive SPI Data  */
uint32_t Chip_SPI_RWFrames_Blocking(LPC_SPI_T *pSPI, SPI_DATA_SETUP_T *pXfSetup)
{
	uint32_t Status;
	/* Clear status */
	Chip_SPI_ClearStatus(pSPI, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);
	Chip_SPI_SetControlInfo(pSPI, pXfSetup->DataSize, SPI_TXCTL_ASSERT_SSEL | SPI_TXCTL_EOF);
	pXfSetup->TxCnt = pXfSetup->RxCnt = 0;
	while ((pXfSetup->TxCnt < pXfSetup->Length) ||
		   (pXfSetup->RxCnt < pXfSetup->Length)) {
		Status = Chip_SPI_GetStatus(pSPI);

		/* In case of TxReady */
		if ((Status & SPI_STAT_TXRDY) && (pXfSetup->TxCnt < pXfSetup->Length)) {
			SPI_Send_Data(pSPI, pXfSetup);
		}

		/*In case of Rx ready */
		if ((Status & SPI_STAT_RXRDY) && (pXfSetup->RxCnt < pXfSetup->Length)) {
			SPI_Receive_Data(pSPI, pXfSetup);
		}
	}
	/* Check error */
	if (Chip_SPI_GetStatus(pSPI) & (SPI_STAT_RXOV | SPI_STAT_TXUR)) {
		return 0;
	}
	return pXfSetup->TxCnt;
}

uint32_t Chip_SPI_WriteFrames_Blocking(LPC_SPI_T *pSPI, SPI_DATA_SETUP_T *pXfSetup)
{
	/* Clear status */
	Chip_SPI_ClearStatus(pSPI, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);
	Chip_SPI_SetControlInfo(pSPI, pXfSetup->DataSize, SPI_TXCTL_ASSERT_SSEL | SPI_TXCTL_EOF | SPI_TXCTL_RXIGNORE);
	pXfSetup->TxCnt = pXfSetup->RxCnt = 0;
	while (pXfSetup->TxCnt < pXfSetup->Length) {
		/* Wait for TxReady */
		while (!(Chip_SPI_GetStatus(pSPI) & SPI_STAT_TXRDY)) {}

		SPI_Send_Data_RxIgnore(pSPI, pXfSetup);

	}

	/* Make sure the last frame sent completely*/
	while (!(Chip_SPI_GetStatus(pSPI) & SPI_STAT_SSD)) {}
	Chip_SPI_ClearStatus(pSPI, SPI_STAT_SSD);

	/* Check overrun error */
	if (Chip_SPI_GetStatus(pSPI) & SPI_STAT_TXUR) {
		return 0;
	}
	return pXfSetup->TxCnt;
}

uint32_t Chip_SPI_ReadFrames_Blocking(LPC_SPI_T *pSPI, SPI_DATA_SETUP_T *pXfSetup)
{
	/* Clear status */
	Chip_SPI_ClearStatus(pSPI, SPI_STAT_RXOV | SPI_STAT_TXUR | SPI_STAT_SSA | SPI_STAT_SSD);
	Chip_SPI_SetControlInfo(pSPI, pXfSetup->DataSize, SPI_TXCTL_ASSERT_SSEL | SPI_TXCTL_EOF);
	pXfSetup->TxCnt = pXfSetup->RxCnt = 0;
	while (pXfSetup->RxCnt < pXfSetup->Length) {
		/* Wait for TxReady */
		while (!(Chip_SPI_GetStatus(pSPI) & SPI_STAT_TXRDY)) {}

		SPI_Send_Dummy(pSPI, pXfSetup);

		/* Wait for receive data */
		while (!(Chip_SPI_GetStatus(pSPI) & SPI_STAT_RXRDY)) {}

		SPI_Receive_Data(pSPI, pXfSetup);

	}
	/* Check overrun error */
	if (Chip_SPI_GetStatus(pSPI) & (SPI_STAT_RXOV | SPI_STAT_TXUR)) {
		return 0;
	}
	return pXfSetup->RxCnt;
}

/* SPI Interrupt Read/Write with 8-bit frame width */
Status Chip_SPI_Int_RWFrames(LPC_SPI_T *pSPI, SPI_DATA_SETUP_T *pXfSetup)
{
	uint32_t Status;

	Status = Chip_SPI_GetStatus(pSPI);
	/* Check  error in INTSTAT register */
	if (Status & (SPI_STAT_RXOV | SPI_STAT_TXUR)) {
		return ERROR;
	}

	if (pXfSetup->TxCnt == 0) {
		Chip_SPI_SetControlInfo(pSPI, pXfSetup->DataSize, SPI_TXCTL_ASSERT_SSEL | SPI_TXCTL_EOF);
	}

	if (pXfSetup->pRx == NULL) {
		if ((Status & SPI_STAT_TXRDY) && (pXfSetup->TxCnt < pXfSetup->Length)) {
			SPI_Send_Data_RxIgnore(pSPI, pXfSetup);
		}
	}
	else {
		/* check if Tx ready */
		if ((Status & SPI_STAT_TXRDY) && (pXfSetup->TxCnt < pXfSetup->Length)) {
			SPI_Send_Data(pSPI, pXfSetup);
		}

		/* check if RX FIFO contains data */
		if ((Status & SPI_STAT_RXRDY) && (pXfSetup->RxCnt < pXfSetup->Length)) {
			SPI_Receive_Data(pSPI, pXfSetup);
		}
	}

	return SUCCESS;
}
