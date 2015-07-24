/*
 * @brief LPC540xx I2C slave driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
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

#define NUMI2CDEVS 3

/* Clock and reset indexes */
static const uint8_t resetIdx[NUMI2CDEVS] = {
	(uint8_t) ASYNC_RESET_I2C0, (uint8_t) ASYNC_RESET_I2C1,
	(uint8_t) ASYNC_RESET_I2C2
};
static const uint8_t ClockIdx[NUMI2CDEVS] = {
	(uint8_t) ASYNC_SYSCTL_CLOCK_I2C0, (uint8_t) ASYNC_SYSCTL_CLOCK_I2C1,
	(uint8_t) ASYNC_SYSCTL_CLOCK_I2C2
};

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Returns the index needed for clock and reset lookup */
static int returnClkIdx(LPC_I2C_T *pI2C)
{
	if (pI2C == LPC_I2C1) {
		return 1;
	}
	else if (pI2C == LPC_I2C2) {
		return 2;
	}

	return 0;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Initializes the LPC_I2C peripheral */
void Chip_I2C_Init(LPC_I2C_T *pI2C)
{
	int clkIndex = returnClkIdx(pI2C);

	/* Enable clock to I2C peripheral and reset */
	Chip_Clock_EnableAsyncPeriphClock((CHIP_ASYNC_SYSCTL_CLOCK_T) ClockIdx[clkIndex]);
	Chip_SYSCTL_AsyncPeriphReset((CHIP_ASYNC_SYSCTL_PERIPH_RESET_T) resetIdx[clkIndex]);
}

/* Shuts down the I2C controller block */
void Chip_I2C_DeInit(LPC_I2C_T *pI2C)
{
	int clkIndex = returnClkIdx(pI2C);

	/* Disable clock to I2C peripheral */
	Chip_Clock_DisableAsyncPeriphClock((CHIP_ASYNC_SYSCTL_CLOCK_T) ClockIdx[clkIndex]);
}

/* Sets register bit values with mask */
void Chip_I2C_SetRegMask(volatile uint32_t *pReg32, uint32_t mask, uint32_t bits)
{
	*pReg32 = (*pReg32 & mask) | bits;
}

/* Clears register bit values with mask */
void Chip_I2C_ClearRegMask(volatile uint32_t *pReg32, uint32_t mask, uint32_t bits)
{
	*pReg32 = (*pReg32 & mask) & ~bits;
}

/* Sets I2C Clock Divider registers */
void Chip_I2C_SetClockDiv(LPC_I2C_T *pI2C, uint32_t clkdiv)
{
	if ((clkdiv >= 1) && (clkdiv <= 65536)) {
		pI2C->CLKDIV = clkdiv - 1;
	}
	else {
		pI2C->CLKDIV = 0;
	}
}
