/*
 * @brief LPC540XX Chip specific SystemInit
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
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

/* Enable the USE_ROM_API definition to use the ROM API for PLL setup */
// #define USE_ROM_API

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Clock and PLL initialization based on the internal oscillator */
void Chip_SetupIrcClocking(void)
{
#if 0 // FIXME stubbed for now, untested
#if defined (USE_ROM_API)
	uint32_t cmd[4], resp[2];
#endif

	/* Turn on the IRC by clearing the power down bit */
	Chip_SYSCTL_PowerUp(PDRUNCFG_PD_IRC_OSC_EN | PDRUNCFG_PD_IRC_EN);

	/* Select the PLL input in the IRC */
	Chip_Clock_SetSystemPLLSource(SYSCTL_PLLCLKSRC_IRC);

	/* Wait State setting FIXME */
	/* Setup FLASH access to 2 clocks (up to FIXME MHz) */
	Chip_FMC_SetFLASHAccess(FLASHTIM_72MHZ_CPU);

#if defined (USE_ROM_API)
	/* Use ROM API for setting up PLL */
	cmd[0] = Chip_Clock_GetIntOscRate() / 1000; /* in KHz */
	cmd[1] = 24000000 / 1000; /* 24MHz system clock rate */
	cmd[2] = CPU_FREQ_EQU;
	cmd[2] = 24000000 / 10000;
	LPC_PWRD_API->set_pll(cmd, resp);

	/* Dead loop on fail */
	while (resp[0] != PLL_CMD_SUCCESS) {}

#else
	/* Power down PLL to change the PLL divider ratio */
	Chip_SYSCTL_PowerDown(PDRUNCFG_PD_SYS_PLL0);

	/* First parameter is the multiplier, the second parameter is the input frequency in MHz */
	Chip_Clock_SetupSystemPLL(8, 12);

	/* Turn on the PLL by clearing the power down bit */
	Chip_SYSCTL_PowerUp(PDRUNCFG_PD_SYS_PLL0);

	/* Wait for PLL to lock */
	while (!Chip_Clock_IsSystemPLLLocked()) {}

	/* Set system clock divider to 1 */
	Chip_Clock_SetSysClockDiv(1);

	/* Set main clock source to the system PLL. This will drive main clock as the system clock */
	Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_PLLOUT);
#endif
#endif
}

/* Clock and PLL initialization based on the internal oscillator */
void Chip_SetupXtalClocking(void)
{
	// FIXME - not yet miplemented, see board layer functions
}

/* Set up and initialize hardware prior to call to main */
void Chip_SystemInit(void)
{
	/* Initial internal clocking */
	Chip_SetupIrcClocking();
}
