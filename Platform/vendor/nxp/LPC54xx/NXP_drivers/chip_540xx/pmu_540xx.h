/*
 * @brief LPC540XX Power Management declarations and functions
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

#ifndef __PMU_540XX_H_
#define __PMU_540XX_H_

#ifdef __cplusplus
extern "C" {
#endif
	
/** @defgroup PMU_540XX CHIP: LPC540XX Power Management declarations and functions
 * @ingroup CHIP_540XX_Drivers
 * @{
 */

/**
 * brown-out detection reset status
 */
#define POWER_BOD_RST     (1 << 6)
/**
 * brown-out detection interrupt status
 */
#define POWER_BOD_INT     (1 << 7)

typedef volatile struct {
  volatile uint32_t VDCTRL[4];     ///< (0x00) VD1, vd2, vd3, vd8 domain voltage Control 
  volatile uint32_t RESERVED0[4];  ///< (0x10) VD1, vd2, vd3, vd8 domain voltage Control 
  volatile uint32_t VDCLAMP[4];    ///< (0x20) VD1, vd2, vd3, vd8 domain voltage Control  
  volatile uint32_t RESERVED1[4];  ///< (0x30) VD1, vd2, vd3, vd8 domain voltage Control 
  volatile uint32_t LPCTRL;        ///< (0x40) LP VD control  
  volatile uint32_t BODCTRL;       ///< (0x44) bod Control   
  volatile uint32_t BODTRIM;       ///< (0x48) bod Trim  
  volatile uint32_t PWRSWACK;      ///< (0x4C) Power Switch Acknowledge  
  volatile uint32_t DPDWAKESRC;    ///< (0x50) Deep power down wakeup source flags  
} LPC_POWER_T;

typedef volatile struct{
  volatile uint32_t BBCTRL;     ///< (0x00)  Body Bias Control  
  volatile uint32_t FBBOFFSET;  ///< (0x04)  FBB offset adjust  
} LPC_BODYBIAS_T;

/**
 * Brown-out detector reset level
 */
typedef enum CHIP_POWER_BODRSTLVL {
	POWER_BODRSTLVL_0,	/*!< Brown-out reset at 1.46 ~ 1.63v */
	POWER_BODRSTLVL_1,	/*!< Brown-out reset at 2.06v ~ 2.15v */
	POWER_BODRSTLVL_2,	/*!< Brown-out reset at 2.35v ~ 2.43v */
	POWER_BODRSTLVL_3,	/*!< Brown-out reset at 2.63v ~ 2.71v */
} CHIP_POWER_BODRSTLVL_T;

/**
 * Brown-out detector interrupt level
 */
typedef enum CHIP_POWER_BODRINTVAL {
	POWER_BODINTVAL_LVL0,	/* Brown-out interrupt at 1.65 ~ 1.80v */
	POWER_BODINTVAL_LVL1,	/* Brown-out interrupt at 2.22v ~ 2.35v*/
	POWER_BODINTVAL_LVL2,	/* Brown-out interrupt at 2.52v ~ 2.66v */
	POWER_BODINTVAL_LVL3,	/* Brown-out interrupt at 2.80v ~ 2.90v */
} CHIP_POWER_BODRINTVAL_T;

/**
 * @brief	Set BBCTRL register
 * @param	pBBCTRL	: Pointer to BodyBias IP register address
 * @param	BBCtrlVal	: BBCTRL register Value
 * @return	Nothing
 */
STATIC INLINE void Chip_POWER_SetBBCTRL(LPC_BODYBIAS_T *pBBCTRL, uint32_t BBCtrlVal)
{
	pBBCTRL->BBCTRL = BBCtrlVal;
}

/**
 * @brief	Get BBCTRL register
 * @param	pBBCTRL	: Pointer to BodyBias IP register address
 * @return	BBCtrlVal
 */
STATIC INLINE uint32_t Chip_POWER_GetBBCTRL(LPC_BODYBIAS_T *pBBCTRL)
{
	return ( pBBCTRL->BBCTRL );
}

/**
 * @brief	Set FBBOffset register
 * @param	pTMR	: Pointer to BodyBias IP register address
 * @param	FBBOffsetVal	: FBBOFFSET register Value
 * @return	Nothing
 */
STATIC INLINE void Chip_POWER_SetFBBOffset(LPC_BODYBIAS_T *pBBCTRL, uint32_t FBBOffsetVal)
{
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_BODY_BIAS);
	pBBCTRL->FBBOFFSET = FBBOffsetVal;
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_BODY_BIAS);
}

/**
 * @brief	Set brown-out detection interrupt and reset levels
 * @param	rstlvl	: Brown-out detector reset level
 * @param	intlvl	: Brown-out interrupt level
 * @return	Nothing
 * @note	Brown-out detection reset will be disabled upon exiting this function.
 * Use Chip_POWER_EnableBODReset() to re-enable
 */
STATIC INLINE void Chip_POWER_SetBODLevels(CHIP_POWER_BODRSTLVL_T rstlvl,
											CHIP_POWER_BODRINTVAL_T intlvl)
{
	LPC_POWER->BODCTRL = ((uint32_t) rstlvl) | (((uint32_t) intlvl) << 3);
}

/**
 * @brief	Enable brown-out detection reset
 * @return	Nothing
 */
STATIC INLINE void Chip_POWER_EnableBODReset(void)
{
	LPC_POWER->BODCTRL |= (1 << 2);
}

/**
 * @brief	Disable brown-out detection reset
 * @return	Nothing
 */
STATIC INLINE void Chip_POWER_DisableBODReset(void)
{
	LPC_POWER->BODCTRL &= ~(1 << 2);
}

/**
 * @brief	Enable brown-out detection interrupt
 * @return	Nothing
 */
STATIC INLINE void Chip_POWER_EnableBODInterrupt(void)
{
	LPC_POWER->BODCTRL |= (1 << 5);
}

/**
 * @brief	Disable brown-out detection interrupt
 * @return	Nothing
 */
STATIC INLINE void Chip_POWER_DisableBODInterrupt(void)
{
	LPC_POWER->BODCTRL &= ~(1 << 5);
}

/**
 * @brief	Set brown-out detection control
 * @return	Nothing
 */
STATIC INLINE void Chip_POWER_SetBODControl(uint32_t bodctl)
{
	LPC_POWER->BODCTRL |= bodctl;
}

/**
 * @brief	Get brown-out detection control
 * @return	brown-out detection control
 */
STATIC INLINE uint32_t Chip_POWER_GetBODControl(void)
{
	return LPC_POWER->BODCTRL;
}

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __PMU_540XX_H_ */
