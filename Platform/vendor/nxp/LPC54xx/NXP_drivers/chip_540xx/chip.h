/*
 * @brief LPC540xx basic chip inclusion file
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

#ifndef __CHIP_H_
#define __CHIP_H_

#include "lpc_types.h"
#include "sys_config.h"
#include "cmsis.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CORE_M4
#ifndef CORE_M0PLUS
#error CORE_M4 or CORE_M0PLUS is not defined for the LPC540xx architecture
#error CORE_M4 or CORE_M0PLUS should be defined as part of your compiler define list
#endif
#endif

#ifndef CHIP_LPC540XX
#error The LPC540XX Chip include path is used for this build, but
#error CHIP_LPC540XX is not defined!
#endif

/** @defgroup PERIPH_540XX_BASE CHIP: LPC540xx Peripheral addresses and register set declarations
 * @ingroup CHIP_540xx_Drivers
 * @{
 */

#define LPC_GPIO_PORT_BASE         0x1C000000UL
#define LPC_DMA_BASE               0x1C004000UL
#define LPC_CRC_BASE               0x1C010000UL
#define LPC_SCT_BASE               0x1C018000UL
#define LPC_MBOX_BASE              0x1C02C000UL
#define LPC_ADC_BASE               0x1C034000UL
#define LPC_VFIFO_BASE             0x1C038000UL

#if 1
#define LPC_SYSCTL_BASE            (0x40000000UL + (0x4000 * 0))
#define LPC_CTIMER2_BASE           (0x40000000UL + (0x4000 * 1))
#define LPC_CTIMER3_BASE           (0x40000000UL + (0x4000 * 2))
#define LPC_CTIMER4_BASE           (0x40000000UL + (0x4000 * 3))
#define LPC_GPIO_GROUPINT0_BASE    (0x40000000UL + (0x4000 * 4))	/* GPIO_GROUPINT0 */
#define LPC_GPIO_GROUPINT1_BASE    (0x40000000UL + (0x4000 * 5))	/* GPIO_GROUPINT1 */
#define LPC_PIN_INT_BASE           (0x40000000UL + (0x4000 * 6))	/* GPIO_INT */
#define LPC_IOCON_BASE             (0x40000000UL + (0x4000 * 7))	/* IOCON */
#define LPC_UTICK_BASE             (0x40000000UL + (0x4000 * 8))	/* UTICK */
#define LPC_FMC_BASE               (0x40000000UL + (0x4000 * 9))	/* Flash Controller */

#define LPC_POWER_BASE             (0x40000000UL + (0x4000 * 11))	/* PMU0 */
#define LPC_WWDT_BASE              (0x40000000UL + (0x4000 * 14))	/* WDT */
#define LPC_RTC_BASE               (0x40000000UL + (0x4000 * 15))	/* RTC */

#define LPC_EZH_ARCH_A_CON_BASE    (0x40000000UL + (0x4000 * 19))
#define LPC_EZH_ARCH_B_CON_BASE    (0x40000000UL + (0x4000 * 19) + 0x20)
#define LPC_INMUX_BASE             (0x40000000UL + (0x4000 * 20))

#define LPC_OSTIMER_BASE           (0x40000000UL + (0x4000 * 28))		/* OSTIMER */
#define LPC_MRT_BASE               (0x40000000UL + (0x4000 * 29))		/* MRT */

#define LPC_ASYNC_SYSCTL_BASE      (0x40080000UL + (0x4000 * 0))
#define LPC_FRG0_BASE              (0x40080000UL + (0x4000 * 0) + 0x030)// resides in the syscon
#define LPC_USART0_BASE            (0x40080000UL + (0x4000 * 1))	/* USART0 */
#define LPC_USART1_BASE            (0x40080000UL + (0x4000 * 2))	/* USART1 */
#define LPC_USART2_BASE            (0x40080000UL + (0x4000 * 3))	/* USART2 */
#define LPC_USART3_BASE            (0x40080000UL + (0x4000 * 4))	/* USART3 */
#define LPC_I2C0_BASE              (0x40080000UL + (0x4000 * 5))
#define LPC_I2C1_BASE              (0x40080000UL + (0x4000 * 6))
#define LPC_I2C2_BASE              (0x40080000UL + (0x4000 * 7))
#define LPC_I2C3_BASE              (0x40080000UL + (0x4000 * 8))
#define LPC_SPI0_BASE              (0x40080000UL + (0x4000 * 9))
#define LPC_SPI1_BASE              (0x40080000UL + (0x4000 * 10))
#define LPC_SPI2_BASE              (0x40080000UL + (0x4000 * 11))
#define LPC_SPI3_BASE              (0x40080000UL + (0x4000 * 12))
#define LPC_CTIMER0_BASE           (0x40080000UL + (0x4000 * 13))
#define LPC_CTIMER1_BASE           (0x40080000UL + (0x4000 * 14))

#else
// FIXME - THESE ARE WHAT THE USER MANUAL SAYS - IT'S WRONG
#define LPC_ASYNC_SYSCTL_BASE      0x40000000UL
#define LPC_USART0_BASE            0x40004000UL
#define LPC_USART1_BASE            0x40008000UL
#define LPC_USART2_BASE            0x4000C000UL
#define LPC_USART3_BASE            0x40010000UL
#define LPC_I2C0_BASE              0x40014000UL
#define LPC_I2C1_BASE              0x40018000UL
#define LPC_I2C2_BASE              0x4001C000UL
#define LPC_SPI0_BASE              0x40024000UL
#define LPC_SPI1_BASE              0x40028000UL
#define LPC_CTIMER0_BASE           0x40034000UL
#define LPC_CTIMER1_BASE           0x40038000UL
#define LPC_SYSCTL_BASE            0x40080000UL
#define LPC_CTIMER2_BASE           0x40084000UL
#define LPC_CTIMER3_BASE           0x40088000UL
#define LPC_CTIMER4_BASE           0x4008C000UL
#define LPC_GPIO_GROUPINT0_BASE    0x40090000UL
#define LPC_GPIO_GROUPINT1_BASE    0x40094000UL
#define LPC_PIN_INT_BASE           0x40098000UL
#define LPC_IOCON_BASE             0x4009C000UL
#define LPC_UTICK_BASE             0x400A0000UL
#define LPC_FMC_BASE               0x400A4000UL
#define LPC_POWER_BASE             0x400AC000UL	// FIXME - does this existt?
#define LPC_BODYBIAS_BASE          0x40030000UL	// FIXME not in UM
#define LPC_PVTVF0_BASE            0x40034000UL	// FIXME not in UM
#define LPC_PVTMON0_BASE           0x40034060UL	// FIXME - array with mod structure with LPC_PVTVF0_BASE
#define LPC_PVTVF1_BASE            0x40034080UL	// FIXME - array with mod structure with LPC_PVTVF0_BASE, not in UM
#define LPC_WWDT_BASE              0x400B8000UL
#define LPC_RTC_BASE               0x400BC000UL
#define LPC_EZH_ARCH_A_CON_BASE    0x4004C000UL
// #define LPC_EZH_ARCH_B_CON_BASE    0x4004C020UL // Array with A base
#define LPC_INMUX_BASE             0x400D0000UL
#define LPC_OSTIMER_BASE           0x400F0000UL
#define LPC_MRT_BASE               0x400F4000UL
#endif

#define LPC_WWDT                  ((LPC_WWDT_T     *) LPC_WWDT_BASE)
#define LPC_ADC                   ((LPC_ADC_T      *) LPC_ADC_BASE)
#define LPC_INMUX                 ((LPC_INMUX_T    *) LPC_INMUX_BASE)
#define LPC_CTIMER0               ((LPC_TIMER_T    *) LPC_CTIMER0_BASE)
#define LPC_CTIMER1               ((LPC_TIMER_T    *) LPC_CTIMER1_BASE)
#define LPC_CTIMER2               ((LPC_TIMER_T    *) LPC_CTIMER2_BASE)
#define LPC_CTIMER3               ((LPC_TIMER_T    *) LPC_CTIMER3_BASE)
#define LPC_CTIMER4               ((LPC_TIMER_T    *) LPC_CTIMER4_BASE)
#define LPC_UTICK                 ((LPC_UTICK_T    *) LPC_UTICK_BASE)
#define LPC_DMA                   ((LPC_DMA_T      *) LPC_DMA_BASE)
#define LPC_POWER                 ((LPC_POWER_T    *) LPC_POWER_BASE)
#define LPC_BODYBIAS              ((LPC_BODYBIAS_T *) LPC_BODYBIAS_BASE)
#define LPC_OSTIMER               ((LPC_OSTIMER_T  *) LPC_OSTIMER_BASE)
#define LPC_VFIFO                 ((LPC_VFIFO_T    *) LPC_VFIFO_BASE)

#if 0	// FIXME
#define LPC_PVTMON0               ((LPC_PVTMON_Type  *)        LPC_PVTMON0_BASE)	// FIXME should be _T
#define LPC_PVTVF0                ((LPC_PVTVF_Type   *)        LPC_PVTVF0_BASE)
#define LPC_PVTVF1                ((LPC_PVTVF_Type   *)        LPC_PVTVF1_BASE)
#endif

#define LPC_RTC             ((LPC_RTC_T             *) LPC_RTC_BASE)
#define LPC_USART0          ((LPC_USART_T           *) LPC_USART0_BASE)
#define LPC_USART1          ((LPC_USART_T           *) LPC_USART1_BASE)
#define LPC_USART2          ((LPC_USART_T           *) LPC_USART2_BASE)
#define LPC_USART3          ((LPC_USART_T           *) LPC_USART3_BASE)
#define LPC_I2C0            ((LPC_I2C_T             *) LPC_I2C0_BASE)
#define LPC_I2C1            ((LPC_I2C_T             *) LPC_I2C1_BASE)
#define LPC_I2C2            ((LPC_I2C_T             *) LPC_I2C2_BASE)
#define LPC_I2C3            ((LPC_I2C_T             *) LPC_I2C3_BASE)
#define LPC_SPI0            ((LPC_SPI_T             *) LPC_SPI0_BASE)
#define LPC_SPI1            ((LPC_SPI_T             *) LPC_SPI1_BASE)
#define LPC_PMU             ((LPC_PMU_T             *) LPC_PMU_BASE)
#define LPC_CRC             ((LPC_CRC_T             *) LPC_CRC_BASE)
#define LPC_SCT             ((LPC_SCT_T             *) LPC_SCT_BASE)
#define LPC_GPIO            ((LPC_GPIO_T            *) LPC_GPIO_PORT_BASE)
#define LPC_PININT          ((LPC_PIN_INT_T         *) LPC_PIN_INT_BASE)
#define LPC_GINT            ((LPC_GPIOGROUPINT_T    *) LPC_GPIO_GROUPINT0_BASE)
#define LPC_IOCON           ((LPC_IOCON_T           *) LPC_IOCON_BASE)
#define LPC_SYSCTL          ((LPC_SYSCTL_T          *) LPC_SYSCTL_BASE)
#define LPC_SYSCON          ((LPC_SYSCTL_T          *) LPC_SYSCTL_BASE)	/* Alias for LPC_SYSCTL */
#define LPC_ASYNC_SYSCTL    ((LPC_ASYNC_SYSCTL_T    *) LPC_ASYNC_SYSCTL_BASE)
#define LPC_FMC             ((LPC_FMC_T             *) LPC_FMC_BASE)
#define LPC_MRT             ((LPC_MRT_T             *) LPC_MRT_BASE)
#define LPC_MBOX            ((LPC_MBOX_T            *) LPC_MBOX_BASE)

/**
 * @}
 */

/** @ingroup CHIP_540XX_DRIVER_OPTIONS
 * @{
 */

/**
 * @brief	Clock rate on the CLKIN pin
 * This value is defined externally to the chip layer and contains
 * the value in Hz for the CLKIN pin for the board. If this pin isn't used,
 * this rate can be 0.
 */
extern const uint32_t ExtClockIn;

/**
 * @}
 */

/* Include order is important! */
#include "syscon_540xx.h"
#include "cpuctrl_540xx.h"
#include "clock_540xx.h"
#include "iocon_540xx.h"
#include "fmc_540xx.h"						/* TBD */
#include "pinint_540xx.h"
#include "inmux_540xx.h"
#include "crc_540xx.h"
#include "gpio_540xx.h"
#include "mrt_540xx.h"
#include "uart_540xx.h"
#include "wwdt_540xx.h"
#include "sct_540xx.h"
#include "sct_pwm_540xx.h"
#include "spi_540xx.h"
#include "spim_540xx.h"
#include "spis_540xx.h"
#include "i2cm_540xx.h"
#include "i2cs_540xx.h"
#include "adc_540xx.h"
#include "rtc_540xx.h"
#include "ctimer_540xx.h"
#include "ostimer_540xx.h"
#include "utick_540xx.h"
#include "dma_540xx.h"
#include "vfifo_540xx.h"
#include "pmu_540xx.h"
#include "mailbox_540xx.h"
#include "pmu_library_540xx.h"
#include "romapi_540xx.h"
#include "fpu_init.h"
#include "gpiogroup_540xx.h"

/** @defgroup SUPPORT_540XX_FUNC CHIP: LPC540xx support functions
 * @ingroup CHIP_540XX_Drivers
 * @{
 */

/**
 * @brief	Current system clock rate, mainly used for peripherals in SYSCON
 */
extern uint32_t SystemCoreClock;

/**
 * @brief	Update system core and ASYNC syscon clock rate, should be called if the
 *			system has a clock rate change
 * @return	None
 */
void SystemCoreClockUpdate(void);

/**
 * @brief	Set up and initialize hardware prior to call to main()
 * @return	None
 * @note	Chip_SystemInit() is called prior to the application and sets up
 * system clocking prior to the application starting.
 */
void Chip_SystemInit(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __CHIP_H_ */
