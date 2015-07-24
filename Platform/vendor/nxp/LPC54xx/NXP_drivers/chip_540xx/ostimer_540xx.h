/*
 * @brief LPC540XX OS timer driver
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

#ifndef __OSTIMER_540XX_H_
#define __OSTIMER_540XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup OSTIMER_540XX CHIP: LPc540XX 32-bit Timer driver
 * @ingroup CHIP_540XX_Drivers
 * @{
 */

/// The OS_TIMER register file definition.
typedef struct {
  __IO uint32_t CMPVAL;      // @ offset (0x00)
  __IO uint32_t MASK;        // @ offset (0x04)
  __IO uint32_t CTRL;        // @ offset (0x08)
  __IO uint32_t CUNT;        // @ offset (0x0C)
  __IO uint32_t CMPVAL_H;    // @ offset (0x10)
  __IO uint32_t MASK_H;      // @ offset (0x14)
  __IO uint32_t RESERVED;    // @ offset (0x18), // Reserved
  __IO uint32_t CUNT_H;      // @ offset (0x1C)
} LPC_OSTIMER_T;

// bit definitions, _ostimer 
#define OSTIMER_INT_FLAG    (1U << 0)
#define OSTIMER_EN_CLR      (1U << 1)
#define OSTIMER_EN_4_DEBUG  (1U << 2)
#define OSTIMER_EN_TIMER    (1U << 3)

/**
 * @brief	Initialize the OS timer
 * @param	pOSTMR	: Pointer to timer IP register address
 * @return	Nothing
 */
void Chip_OSTIMER_Init(void);

/**
 * @brief	Shutdown the OS timer
 * @param	pOSTMR	: Pointer to OStimer IP register address
 * @return	Nothing
 */
void Chip_OSTIMER_DeInit(void);

/**
 * @brief	Set OS timer mask value
 * @param	pOSTMR	: Pointer to timer IP register address
 * @param	mask_l	: Mask Low
 * @param	mask_h	: Mask High
 * @return			: nothing 
 */
STATIC INLINE void Chip_OSTIMER_SetMask(LPC_OSTIMER_T *pOSTMR, uint32_t mask_l, uint32_t mask_h)
{
	pOSTMR->MASK = mask_l;
	pOSTMR->MASK_H = mask_h;
	return;
}

/**
 * @brief	Set OS timer compare value
 * @param	pOSTMR	: Pointer to timer IP register address
 * @param	mask_l	: COMP Low
 * @param	mask_h	: COMP High
 * @return			: nothing 
 */
STATIC INLINE void Chip_OSTIMER_SetCOMP(LPC_OSTIMER_T *pOSTMR, uint32_t comp_l, uint32_t comp_h)
{
	pOSTMR->CMPVAL = comp_l;
	pOSTMR->CMPVAL_H = comp_h;
	return;
}

/**
 * @brief	Clears a (pending) match interrupt
 * @param	pOSTMR	: Pointer to OS timer IP register address
 * @return	Nothing
 */
STATIC INLINE void Chip_OSTIMER_ClearInt(LPC_OSTIMER_T *pOSTMR)
{
	pOSTMR->CTRL |= OSTIMER_INT_FLAG;
}

/**
 * @brief	Enables the OS timer (starts count)
 * @param	pOSTMR	: Pointer to OS timer IP register address
 * @param	break_en	: debugger enabled or not
 * @param	en_clr	: clear enabled or not when enabled
 * @return	Nothing
 */
STATIC INLINE void Chip_OSTIMER_Enable(LPC_OSTIMER_T *pOSTMR, bool en_clr, bool break_en)
{
	if ( break_en )
		pOSTMR->CTRL |= OSTIMER_EN_4_DEBUG;
	else
		pOSTMR->CTRL &= ~OSTIMER_EN_4_DEBUG;
	if ( en_clr )
		pOSTMR->CTRL |= OSTIMER_EN_CLR;
	else
		pOSTMR->CTRL &= ~OSTIMER_EN_CLR;
	pOSTMR->CTRL |= OSTIMER_EN_TIMER;

}

/**
 * @brief	Disables the OS timer
 * @param	pOSTMR : Pointer to OS timer IP register address
 * @param	break_en	: debugger enabled or not
 * @param	en_clr	: clear enabled or not when enabled
 * @return	Nothing
 */
STATIC INLINE void Chip_OSTIMER_Disable(LPC_OSTIMER_T *pOSTMR, bool en_clr, bool break_en)
{
	if ( break_en )
		pOSTMR->CTRL |= OSTIMER_EN_4_DEBUG;
	else
		pOSTMR->CTRL &= ~OSTIMER_EN_4_DEBUG;
	if ( en_clr )
		pOSTMR->CTRL |= OSTIMER_EN_CLR;
	else
		pOSTMR->CTRL &= ~OSTIMER_EN_CLR;
	pOSTMR->CTRL &= ~OSTIMER_EN_TIMER;
}

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __OSTIMER_540XX_H_ */
