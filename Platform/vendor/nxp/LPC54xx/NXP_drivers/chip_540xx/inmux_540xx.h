/*
 * @brief LPC540XX Input Mux Registers and Driver
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

#ifndef __INMUX_540XX_H_
#define __INMUX_540XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup INMUX_540XX CHIP: LPC540XX Input Mux Registers and Driver
 * @ingroup CHIP_540XX_Drivers
 * @{
 */

/**
 * @brief LPC540XX Input Mux Register Block Structure
 */
typedef struct {						/*!< INMUX Structure */
	__IO uint32_t SCT0_INMUX[6];		/*!< Input mux registers for SCT0 inputs */
	__I  uint32_t RESERVED0[42];
	__IO uint32_t PINTSEL[8];			/*!< Pin interrupt select registers */
	__IO uint32_t DMA_ITRIG_INMUX[22];	/*!< Input mux register for DMA trigger inputs */
	__I  uint32_t RESERVED1[2];
	__IO uint32_t DMA_OTIRG_INMUX[4];	/*!< Input mux register for DMA trigger inputs */
	__I  uint32_t RESERVED2[4];
	__IO uint32_t FREQMEAS_REF;			/*!< Clock selection for frequency measurement ref clock */
	__IO uint32_t FREQMEAS_TARGET;		/*!< Clock selection for frequency measurement target clock */
} LPC_INMUX_T;

/**
 * LPC540XX DMA input trigger source to DMA
 */
typedef enum {
	INMUX_ADC0_SEQA_DMA = 0,
	INMUX_ADC0_SEQB_DMA,
	INMUX_SCT0_A_DMA,
	INMUX_SCT0_B_DMA,
	INMUX_CTIMER0_M0_DMA,
	INMUX_CTIMER0_M1_DMA,
	INMUX_CTIMER1_M0_DMA,
	INMUX_CTIMER2_M0_DMA,
	INMUX_CTIMER2_M1_DMA,
	INMUX_CTIMER3_M0_DMA,
	INMUX_CTIMER3_M1_DMA,
	INMUX_CTIMER4_M0_DMA,
	INMUX_PINTINT0_DMA,
	INMUX_PINTINT1_DMA,
	INMUX_PINTINT2_DMA,
	INMUX_PINTINT3_DMA,
	INMUX_OTRIG0_DMA,
	INMUX_OTRIG1_DMA,
	INMUX_OTRIG2_DMA,
	INMUX_OTRIG3_DMA,
} Chip_INMUX_DMA_ITRIG_CFG_T;

/**
 * @brief	I/O Control configuration
 * @param	pINMUX	: The base of INMUX peripheral on the chip
 * @param	port	: GPIO port to INMUX function
 * @param	pin		: GPIO pin to INMUX function
 * @param	mode	:
 * @param	func	: PinMux function, value of type INMUX_FUNC?
 * @return	Nothing
 */
STATIC INLINE void Chip_INMUX_PININT_Config(LPC_INMUX_T *pINMUX,
											Chip_PININT_SELECT_T channelNum,
											uint32_t portNum,
											uint32_t bitPosi)
{
	// FIXME optimize using *32
	if ( portNum == 0 ) {
		LPC_INMUX->PINTSEL[channelNum] = bitPosi;
	}
	else if ( portNum == 1 ) {
		LPC_INMUX->PINTSEL[channelNum] = bitPosi + 32;
	}
	else if ( portNum == 2 ) {
		LPC_INMUX->PINTSEL[channelNum] = bitPosi + 64;
	}
}

/**
 * @brief	Set INMUX ITRIG number for DMA
 * @param	pINMUX	: The base of INMUX peripheral on the chip
 * @param	ch_num: channel number
 * @param	tg_num: trigger number
 * @return	Nothing
 */
STATIC INLINE void Chip_INMUX_Config_ITRIG_DMA(LPC_INMUX_T *pINMUX, uint32_t channelnum, uint32_t itriggernum)
{
	pINMUX->DMA_ITRIG_INMUX[channelnum] = itriggernum;
}

/* Freqeuency mearure reference and target clock sources */
typedef enum {
	FREQMSR_CLKIN = 0,				/*!< CLKIN pin */	// FIXME assumed CLKIN, UM says main osc
	FREQMSR_IRC,					/*!< Internal RC (IRC) oscillator */
	FREQMSR_WDOSC,					/*!< Watchdog oscillator */
	FREQMSR_32KHZOSC,				/*!< 32KHz (RTC) oscillator rate */
	FREQ_MEAS_MAIN_CLK,				/*!< main system clock */
	FREQMSR_PIO0_4,					/*!< External pin PIO0_4 as input rate */
	FREQMSR_PIO0_20,				/*!< External pin PIO0_20 as input rate */
	FREQMSR_PIO0_24,				/*!< External pin PIO0_24 as input rate */
	FREQMSR_PIO1_4					/*!< External pin PIO1_4 as input rate */
} FREQMSR_SRC_T;

/**
 * @brief	Selects a reference clock used with the frequency measure function
 * @param	ref	: Frequency measure function reference clock
 * @return	Nothing
 */
STATIC INLINE void Chip_INMUX_SetFreqMeasRefClock(FREQMSR_SRC_T ref)
{
	LPC_INMUX->FREQMEAS_REF = (uint32_t) ref;
}

/**
 * @brief	Selects a target clock used with the frequency measure function
 * @param	targ	: Frequency measure function reference clock
 * @return	Nothing
 */
STATIC INLINE void Chip_INMUX_SetFreqMeasTargClock(FREQMSR_SRC_T targ)
{
	LPC_INMUX->FREQMEAS_TARGET = (uint32_t) targ;
}

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __INMUX_540XX_H_ */
