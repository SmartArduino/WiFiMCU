/*
 * @brief NXP LPCXpresso LPC54000 board file
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

#ifndef __BOARD_H_
#define __BOARD_H_

#include "chip.h"
/* board_api.h is included at the bottom of this file after DEBUG setup */

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BOARD_LPCXPRESSO_54000 LPC54000 LPCXpresso board support software API functions
 * @ingroup LPCOPEN_540XX_LPCXPRESSO_54000
 * The board support software API functions provide some simple abstracted
 * functions used across multiple LPCOpen board examples. See @ref BOARD_COMMON_API
 * for the functions defined by this board support layer.<br>
 * @{
 */

/** @defgroup BOARD_LPCXPRESSO_54000_OPTIONS BOARD: LPC54000 LPCXpresso board build options
 * This board has options that configure its operation at build-time.<br>
 * @{
 */

/** Define DEBUG_ENABLE to enable IO via the DEBUGSTR, DEBUGOUT, and
    DEBUGIN macros. If not defined, DEBUG* functions will be optimized
    out of the code at build time.
 */
// #define DEBUG_ENABLE

/** Define DEBUG_SEMIHOSTING along with DEBUG_ENABLE to enable IO support
    via semihosting. You may need to use a C library that supports
    semihosting with this option.
 */
// #define DEBUG_SEMIHOSTING

/** Board UART used for debug output and input using the DEBUG* macros. This
    is also the port used for Board_UARTPutChar, Board_UARTGetChar, and
    Board_UARTPutSTR functions. Although you can setup multiple UARTs here,
    the board code only supoprts UART0 in the Board_UART_Init() fucntion,
    so be sure to change it there too if not using UART0.
 */
#define DEBUG_UART                      LPC_USART0

/** Accelerometer interrupt pin defines */
#define ACCEL_INT_PORT            0x0
#define ACCEL_INT_PIN             5
#define ACCEL_PINT_SEL            PININTSELECT0
#define ACCEL_PINT_CH             PININTCH0
#define ACCEL_PINT_IRQn           PININT0_IRQn
#define ACCEL_WAKE                STARTERP0_PINT0
#define ACCEL_IRQHandler          PIN_INT0_IRQHandler
#define ACCEL_INT2_PORT           0x0
#define ACCEL_INT2_PIN            7

/** Magnetometer interrupt pin defines  */
#define MAG_INT_PORT							0x0
#define MAG_INT_PIN								7
#define MAG_PINT_SEL							PININTSELECT2
#define MAG_PINT_CH								PININTCH2
#define MAG_PINT_IRQn							PININT2_IRQn
#define MAG_WAKE									STARTERP0_PINT2
#define MAG_IRQHandler            PIN_INT2_IRQHandler
#define MAG_INT3_PORT             0x0
#define MAG_INT3_PIN              10

/** Gyroscope interrupt pin interface */
#define GYRO_INT_PORT							0x0
#define GYRO_INT_PIN							4
#define GYRO_PINT_SEL							PININTSELECT1
#define GYRO_PINT_CH							PININTCH1
#define GYRO_PINT_IRQn						PININT1_IRQn
#define GYRO_WAKE									STARTERP0_PINT1
#define GYRO_IRQHandler						PIN_INT1_IRQHandler

/** Proximity interrupt pin interface */
#define PROXI_INT_PORT						0x0
#define PROXI_INT_PIN							9
#define PROXI_PINT_SEL						PININTSELECT1
#define PROXI_PINT_CH							PININTCH1
#define PROXI_PINT_IRQn						PININT1_IRQn
#define PROXI_WAKE								STARTERP0_PINT1
#define PROXI_IRQHandler					PIN_INT1_IRQHandler


/**
 * @}
 */

/* Board name */
#define BOARD_CUSTOMER_SONY_54000


/**
 * @}
 */

#include "board_api.h"

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H_ */
