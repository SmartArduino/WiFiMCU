/*
 * @brief LPCXpresso LPC54000 board file
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
#include "retarget.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Be careful that these pins are set to output now once Board_LED_Init() is
   called. */
static const uint8_t ledBits[] = {6, 7, 8};

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/* Clock rate on the CLKIN pin */
const uint32_t ExtClockIn = 12000000;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initialize the LEDs on the NXP LPC54000 LPCXpresso Board */
static void Board_LED_Init(void)
{
	int i;

	/* Pin muxing setup as part of board_sysinit */
	for (i = 0; i < sizeof(ledBits); i++) {
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, ledBits[i]);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, ledBits[i], true);
	}
}

/* Board Debug UART Initialisation function */
static void Board_UART_Init(void)
{
	Chip_SYSCTL_Enable_ASYNC_Syscon(true);
	/* Divided by 1 */
	Chip_Clock_SetAsyncSysconClockDiv(1);

	/* UART0 pins are setup in board_sysinit.c */
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Set the LED to the state of "On" */
void Board_LED_Set(uint8_t LEDNumber, bool On)
{
	if (LEDNumber < sizeof(ledBits)) {
		Chip_GPIO_SetPinState(LPC_GPIO, 1, ledBits[LEDNumber], (bool) !On);
	}
}

/* Return the state of LEDNumber */
bool Board_LED_Test(uint8_t LEDNumber)
{
	if (LEDNumber < sizeof(ledBits)) {
		return (bool) !Chip_GPIO_GetPinState(LPC_GPIO, 1, ledBits[LEDNumber]);
	}

	return false;
}

/* Toggles the current state of a board LED */
void Board_LED_Toggle(uint8_t LEDNumber)
{
	if (LEDNumber < sizeof(ledBits)) {
		Chip_GPIO_SetPinToggle(LPC_GPIO, 1, ledBits[LEDNumber]);
	}
}

/* Sends a character on the UART */
void Board_UARTPutChar(char ch)
{
#if defined(DEBUG_UART)
	Chip_UART_SendBlocking(DEBUG_UART, &ch, 1);
#endif
}

/* Gets a character from the UART, returns EOF if no character is ready */
int Board_UARTGetChar(void)
{
#if defined(DEBUG_UART)
	uint8_t data;

	if (Chip_UART_Read(DEBUG_UART, &data, 1) == 1) {
		return (int) data;
	}
#endif
	return EOF;
}

/* Outputs a string on the debug UART */
void Board_UARTPutSTR(char *str)
{
#if defined(DEBUG_UART)
	while (*str != '\0') {
		Board_UARTPutChar(*str++);
	}
#endif
}

/* Initialize debug output via UART for board */
void Board_Debug_Init(void)
{
#if defined(DEBUG_UART)
	Board_UART_Init();
	Chip_UART_Init(DEBUG_UART);
	Chip_UART_ConfigData(DEBUG_UART, UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
	Chip_UART_SetBaud(DEBUG_UART, 115200);
	Chip_UART_Enable(DEBUG_UART);
	Chip_UART_TXEnable(DEBUG_UART);
#endif
}

/* Set up and initialize all required blocks and functions related to the
   board hardware */
void Board_Init(void)
{
	/* INMUX and IOCON are used by many apps, enable both INMUX and IOCON clock bits here. */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_MUX);
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);

	/* Sets up DEBUG UART */
	DEBUGINIT();

	/* Initialize GPIO */
	Chip_GPIO_Init(LPC_GPIO);

	/* Initialize the LEDs. Be careful with below routine, once it's called some of the I/O will be set to output. */
	Board_LED_Init();
}
