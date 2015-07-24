/*
 * @brief LPC540XX SPI driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
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

/* Return clock for the passed peripheral */
static CHIP_ASYNC_SYSCTL_CLOCK_T getSPIClk(LPC_SPI_T *pSPI)
{
	if (pSPI == LPC_SPI1) {
		return ASYNC_SYSCTL_CLOCK_SPI1;
	}

	return ASYNC_SYSCTL_CLOCK_SPI0;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Initialize the SPI */
void Chip_SPI_Init(LPC_SPI_T *pSPI)
{
	/* Enable SPI clock and reset IP */
	Chip_Clock_EnableAsyncPeriphClock(getSPIClk(pSPI));
	if (pSPI == LPC_SPI1) {
		Chip_SYSCTL_AsyncPeriphReset(ASYNC_RESET_SPI1);
	}
	else {
		Chip_SYSCTL_AsyncPeriphReset(ASYNC_RESET_SPI0);
	}
}

/* De-initializes the SPI peripheral */
void Chip_SPI_DeInit(LPC_SPI_T *pSPI)
{
	Chip_SPI_Disable(pSPI);
	Chip_Clock_DisableAsyncPeriphClock(getSPIClk(pSPI));
}

/* Set SPI CFG register values */
void Chip_SPI_SetCFGRegBits(LPC_SPI_T *pSPI, uint32_t bits)
{
	uint32_t reg;

	/* Mask off bits that are write as 0, read as undefined */
	reg = pSPI->CFG & SPI_CFG_BITMASK;

	/* Update CFG register with only selected bits enabled */
	pSPI->CFG = reg | bits;
}

/* Clear SPI CFG register values */
void Chip_SPI_ClearCFGRegBits(LPC_SPI_T *pSPI, uint32_t bits)
{
	uint32_t reg;

	/* Mask off bits that are write as 0, read as undefined */
	reg = pSPI->CFG & SPI_CFG_BITMASK;

	/* Update CFG register with only selected bits disabled */
	pSPI->CFG = reg & ~bits;
}

/* Enable SPI master mode */
void Chip_SPI_EnableMasterMode(LPC_SPI_T *pSPI)
{
	Chip_SPI_SetCFGRegBits(pSPI, SPI_CFG_MASTER_EN);

	/* Deassert all chip selects, only in master mode */
	pSPI->TXCTRL = SPI_TXDATCTL_DEASSERT_ALL;
}

/* Setup SAPI configuration */
void Chip_SPI_ConfigureSPI(LPC_SPI_T *pSPI, SPI_CFGSETUP_T *pCFG)
{
	uint32_t reg;

	/* Get register and mask off config bits this function alters */
	reg = pSPI->CFG & ~(SPI_CFG_MASTER_EN | SPI_CFG_LSB_FIRST_EN |
						SPI_CFG_CPHA_SECOND | SPI_CFG_CPOL_HI);

	if (pCFG->master) {
		reg |= SPI_CFG_MASTER_EN;
	}
	if (pCFG->lsbFirst) {
		reg |= SPI_CFG_LSB_FIRST_EN;
	}
	reg |= (uint32_t) pCFG->mode;

	Chip_SPI_SetCFGRegBits(pSPI, reg);

	/* Deassert all chip selects, only in master mode */
	pSPI->TXCTRL = SPI_TXDATCTL_DEASSERT_ALL;
}

/* Flush FIFOs */
void Chip_SPI_FlushFifos(LPC_SPI_T *pSPI)
{
	Chip_SPI_Disable(pSPI);
	Chip_SPI_Enable(pSPI);
}

/* Set SPI TXCTRL register control options */
void Chip_SPI_SetTXCTRLRegBits(LPC_SPI_T *pSPI, uint32_t bits)
{
	uint32_t reg;

	reg = pSPI->TXCTRL & SPI_TXDATCTL_CTRLMASK;
	pSPI->TXCTRL = reg | bits;
}

/* Clear SPI TXCTRL register control options */
void Chip_SPI_ClearTXCTRLRegBits(LPC_SPI_T *pSPI, uint32_t bits)
{
	uint32_t reg;

	reg = pSPI->TXCTRL & SPI_TXDATCTL_CTRLMASK;
	pSPI->TXCTRL = reg & ~bits;
}
