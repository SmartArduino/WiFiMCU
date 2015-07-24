/*
 * @brief LPC540XX CPU multi-core support driver
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

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Setup M0+ boot and reset M0+ core */
void Chip_CPU_CM0Boot(uint32_t *coentry, uint32_t *costackptr)
{
	uint32_t temp;

	/* Setup M0+ stack and M0+ boot location */
	LPC_SYSCTL->CPSTACK = (uint32_t) costackptr;
	LPC_SYSCTL->CPBOOT =  (uint32_t) coentry;

	temp = LPC_SYSCTL->CPUCTRL | 0xc0c40000 | MC_CM0_CLK_ENABLE; // FIXME - needs bit masking, extra bits needed? Doesn't matcn UM

	/* Enable M0+ clocking with reset asserted */
	LPC_SYSCTL->CPUCTRL = temp | MC_CM0_RESET_ENABLE;

	/* De-assert reset on M0+ */
	LPC_SYSCTL->CPUCTRL = temp;
}

/* Setup M4 boot and reset M4 core */
void Chip_CPU_CM4Boot(uint32_t *coentry, uint32_t *costackptr)
{
	uint32_t temp;

	/* Setup M0+ stack and M0+ boot location */
	LPC_SYSCTL->CPSTACK = (uint32_t) costackptr;
	LPC_SYSCTL->CPBOOT =  (uint32_t) coentry;

	temp = LPC_SYSCTL->CPUCTRL | 0xc0c40000 | MC_CM4_CLK_ENABLE;

	/* Enable M4 clocking with reset asserted */
	LPC_SYSCTL->CPUCTRL = temp | MC_CM4_RESET_ENABLE;

	/* De-assert reset on M4 */
	LPC_SYSCTL->CPUCTRL = temp;
}
