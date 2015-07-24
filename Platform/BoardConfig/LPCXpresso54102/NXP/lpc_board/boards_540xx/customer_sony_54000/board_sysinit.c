/*
 * @brief NXP LPCXpresso 54000 Sysinit file
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

 #include "board.h"

/* The System initialization code is called prior to the application and
   initializes the board for run-time operation. Board initialization
   for the NXP LPC540XX board includes default pin muxing and clock setup
   configuration. */

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Pin muxing table, only items that need changing from their default pin
   state are in this table. Not every pin is mapped. */
STATIC const PINMUX_GRP_T pinmuxing[] = {
	/* UART */
	{0, 0,  (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* UART0 RX */
	{0, 1,  (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* UART0 TX */

	{0, 2,  (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* NC */
	{0, 3,  (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* NC */

	/* Misc */
	{0, 22, (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* NIOBE_CLKIN (GPIO input) */
	{0, 21, (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* CLKOUT-CT32B3_MAT0 (GPIO input) */
	{0, 26, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)}, /* MICROSD_CDn (GPIO input) ####NOT WORKING #####*/
//	{0, 9,  (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* I2S_RX_SDA (GPIO input) */
	{0, 11, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* SCK0-I2S_CLK (CT2_MAT1) used for DEBUG1 #####*/
	{0, 13, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* MIS0-I2S_WS (GPIO input) used for DEBUG2 ####*/
	{0, 18, (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* I2S_TX_SDA (GPIO input) */
	{0, 19, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* IF_JTAG_TDI (GPIO input) */

//	{0, 15, (IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* SWO */
//	{0, 16, (IOCON_FUNC5 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* SWCLK_TCK */
//	{0, 17, (IOCON_FUNC5 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* SWDIO */

	/* I2C */
	{0, 23, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)}, /* I2C0_SCL (SCL) */
	{0, 24, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)}, /* I2C0_SDA-WAKEUP (SDA) */

	/* Sensor related */
//	{0, 4,  (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* GYR_INT1 (GPIO input) */
	{0, 5,  (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* AM_PX_INT (GPIO input) */
	{0, 6,  (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* ACCL_INT1 (GPIO input) */
	{0, 7,  (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* ACCL_INT2 (GPIO input) */
	{0, 8,  (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* MAG_DRDY (GPIO input) */
	{0, 9,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)}, /* AM_PX_INT (GPIO input) */
	{0, 10, (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* MAG_INT3 (GPIO input) */
	{0, 20, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* AUDIO_INT (GPIO input) */
	{0, 25, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)}, /* SEN300_INT (GPIO input) */

	/* Bridge */
//	{0, 12, (IOCON_FUNC1 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /*  */
//	{0, 14, (IOCON_FUNC1 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /*  */
	{0, 27, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)}, /* BRIDGE_SCL (SCL) */
	{0, 28, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)}, /* BRIDGE_SDA (SDA) */
	{0, 29, (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* BRIDGE_GPIO (GPIO) */
	{0, 4,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)}, /* BRIDGE_INTR (GPIO) */
//	{1, 3,  (IOCON_FUNC5 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /*  */
//	{1, 4,  (IOCON_FUNC5 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /*  */
	{1, 5,  (IOCON_FUNC2 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* BRIDGE_SSEL (SPISSEL) */
	{1, 6,  (IOCON_FUNC2 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* BRIDGE_SCK (SCK) */
	{1, 7,  (IOCON_FUNC2 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* BRIDGE_MOSI (SPI MOSI) */
	{1, 8,  (IOCON_FUNC2 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* BRIDGE_MISO (SPI MISO) */

	/* SDMMC card interface NOT USED so set them as GPIOs */
	/* P1_5 left as GPIO with pull-up - the default state */
	

};

// FIXME - why are these needed?
/* ASVMB 12MHz */
#define USE_CLKIN_IN                0
#define PLL_MULTIPLIER          8				/* PLL output = PLL_MULTIPLIER * PLL input clock. */

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

// FIXME - needs to move to chip layer in sysinit
/* Setup system clocking */
STATIC void Board_SetupXtalClocking(void)
{
#if USE_CLKIN_IN
	uint32_t i;
#endif

	/* A library"pmu_library.lib) has been created to facilitate the power management 
	operation. The user needs to enter the desired frequency the application wants to
	run, the set_voltage() will set the internal voltage regulators automatically. */
	set_voltage( SYSCTL_IRC_FREQ * PLL_MULTIPLIER );
	
	/* Select the PLL input in the IRC */
#if USE_CLKIN_IN
	/* IOCON clock left on, this is needed is CLKIN is used. */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, IOCON_MODE_PULLUP | IOCON_FUNC1 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF);
	/* Delay to wait until CLKIN stablized */
	for ( i = 0; i < 500; i++ ) {}
	Chip_Clock_SetSystemPLLSource(SYSCTL_PLLCLKSRC_CLKIN);
	/* Wait State setting TBD */
	/* Setup FLASH access to 2 clocks (up to 20MHz) */
	Chip_FMC_SetFLASHAccess(FLASHTIM_72MHZ_CPU);
#else
	Chip_Clock_SetSystemPLLSource(SYSCTL_PLLCLKSRC_IRC);
	/* Wait State setting TBD */
	/* Setup FLASH access to 5 clocks (up to 72MHz) */
	Chip_FMC_SetFLASHAccess(FLASHTIM_72MHZ_CPU);
#endif

	/* Power down PLL to change the PLL divider ratio */
	Chip_SYSCTL_PowerDown(PDRUNCFG_PD_SYS_PLL0);

	/* First parameter is the multiplier, the second parameter is the input frequency in MHz */
#if USE_CLKIN_IN
	Chip_Clock_SetupSystemPLL(PLL_MULTIPLIER, ExtRateIn);
#else
	Chip_Clock_SetupSystemPLL(PLL_MULTIPLIER, SYSCTL_IRC_FREQ);
#endif

	/* Turn on the PLL by clearing the power down bit */
	Chip_SYSCTL_PowerUp(PDRUNCFG_PD_SYS_PLL0);

	/* Wait for PLL to lock */
	while (!Chip_Clock_IsSystemPLLLocked()) {}

	/* Set system clock divider to 1 */
	Chip_Clock_SetSysClockDiv(1);

	/* Set main clock source to the system PLL. This will drive 24MHz
	   for the main clock and 24MHz for the system clock */
	Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_PLLOUT);
}

STATIC void Board_SetupIRCClocking(void)
{
	/* Wait State setting TBD */
	/* Setup FLASH access to 2 clocks (up to 20MHz) */
	Chip_FMC_SetFLASHAccess(FLASHTIM_20MHZ_CPU);

	/* Set system clock divider to 1 */
	Chip_Clock_SetSysClockDiv(1);

	/* Set main clock source to the system PLL. This will drive 24MHz
	   for the main clock and 24MHz for the system clock */
	Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_IRC);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Sets up system pin muxing */
void Board_SetupMuxing(void)
{
	/* Enable IOCON clock */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);

	Chip_IOCON_SetPinMuxing(LPC_IOCON, pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));

	/* IOCON clock left on, this is needed if CLKIN is used. */
}

/* Set up and initialize clocking prior to call to main */
void Board_SetupClocking(void)
{
	/* The IRC is always the first clock source even if CLK_IN is used later.
	   Once CLK_IN is selected as the clock source. We can turned off the IRC later.
	   Turn on the IRC by clearing the power down bit */
	Chip_SYSCTL_PowerUp(PDRUNCFG_PD_IRC_OSC_EN | PDRUNCFG_PD_IRC_EN);

	//Board_SetupXtalClocking();
	Board_SetupIRCClocking();

	/* Select the CLKOUT clocking source */
	Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_MAINSYSCLK, 1);

	/* ASYSNC SYSCON needs to be on or all serial peripheral won't work.
	   Be careful if PLL is used or not, ASYNC_SYSCON source needs to be
	   selected carefully. */
	Chip_SYSCTL_Enable_ASYNC_Syscon(true);
	Chip_Clock_SetAsyncSysconClockDiv(1);
	Chip_Clock_SetAsyncSysconClockSource(ASYNC_SYSCTL_CLOCK_IRC);
}

/* Set up and initialize hardware prior to call to main */
void Board_SystemInit(void)
{
	/* Setup system clocking and muxing */
	Board_SetupMuxing();/* Muxing first as it sets up ext oscillator pins */
	Board_SetupClocking();
}
