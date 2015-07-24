/*
 * @brief LPC540XX System & Control driver
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

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Returns the computed value for a frequency measurement cycle */
uint32_t Chip_SYSCTL_GetCompFreqMeas(uint32_t refClockRate)
{
	uint32_t capval;
	uint64_t clkrate = 0;

	/* Get raw capture value */
	capval = Chip_SYSCTL_GetRawFreqMeasCapval();

	/* Limit CAPVAL check */
	if (capval > 2) {
		clkrate = (((uint64_t) capval - 2) * (uint64_t) refClockRate) / 0x4000;
	}

	return (uint32_t) clkrate;
}

/* Power down one or more blocks or peripherals */
void Chip_SYSCTL_PowerDown(uint32_t powerdownmask)
{
	/* Disable peripheral states by setting high */
	LPC_SYSCTL->PDRUNCFGSET = powerdownmask;
}

/* Power up one or more blocks or peripherals */
void Chip_SYSCTL_PowerUp(uint32_t powerupmask)
{
	/* Enable peripheral states by setting low */
	LPC_SYSCTL->PDRUNCFGCLR = powerupmask;
}

/* Power up one or more blocks or peripherals */
uint32_t Is_Chip_SYSCTL_PowerUp(uint32_t powerupmask)
{
	/* Get current power states. if the corresponding bit(s) are zero, it's powered up already. */
	return ( LPC_SYSCTL->PDRUNCFG & powerupmask );
}

void Chip_SYSCTL_NMI_Enable( uint32_t NMI_num )
{
  LPC_SYSCTL->NMISRC = 0x80000000|NMI_num;
}
