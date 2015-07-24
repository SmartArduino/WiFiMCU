/*
 * @brief LPC412X Power ROM API declarations and functions
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

#ifndef __ROM_PWR_412X_H_
#define __ROM_PWR_412X_H_

#ifdef __cplusplus
extern "C" {
#endif
	
/** @defgroup PWRD_412X CHIP: LPC412X Power ROM API declarations and functions
 * @ingroup CHIP_412X_Drivers
 * @{
 */

/**
 * @brief LPC412X Power ROM APIs - set_pll mode options
 */
#define     AUTO_CLK_GATE_KEY  0xc0de0000
#define     ACLK_GATE_FMC      (1<<7)
#define     ACLK_GATE_FLASH    (1<<8)

//specify CPU freq mode 
#define	CPU_FREQ_EQU  		0
#define	CPU_FREQ_LTE		1
#define	CPU_FREQ_GTE		2
#define	CPU_FREQ_CLOSE		3

#define	PLL_CMD_CUCCESS		0
#define	PLL_INVALID_FREQ	1
#define	PLL_INVALID_MODE	2
#define	PLL_FREQ_NOT_FOUND	3
#define	PLL_NOT_LOCKED		4

//specify parameter mode
#define	PARAM_DEFAULT			0
#define	PARAM_CPU_EXEC			1
#define	PARAM_EFFICIENCY		2
#define	PARAM_LOW_CURRENT		3

//specify routine's feedback
#define	PARAM_CMD_CUCCESS		0
#define	PARAM_INVALID_FREQ		1
#define	PARAM_INVALID_MODE		2

#define NVIC_LP_SEVONPEND   (0x1<<4)
#define NVIC_LP_SLEEPDEEP   (0x1<<2)
#define NVIC_LP_SLEEPONEXIT (0x1<<1)

/* power mode configuration API parameter */
typedef enum  power_mode_config {
      PMU_SLEEP                  = 0,
      PMU_DEEP_SLEEP             = 1,
      PMU_POWERDOWN              = 2,
      PMU_POWERDOWN_FLASH_RETAIN = 3,
      PMU_DEEP_POWERDOWN         = 4
} power_mode_cfg_t;

typedef enum  vbb_mode {
      RBB_MODE  = 0x0,
      FBB2_MODE = 0x1,
      FBB3_MODE = 0x2,
      NBB_MODE  = 0x3,
      FBB1_MODE = 0x4
} vbbMode_t;

typedef enum  domain_name {
       VD1 = 0x0,
       VD2 = 0x1,
       VD3 = 0x2,
       VD8 = 0x3
} domain_t;

typedef enum  voltage_level{
       V0650 = 0x0,
       V0700 = 0x1,
       V0750 = 0x2,
       V0800 = 0x3,
       V0850 = 0x4,
       V0900 = 0x5,
       V0950 = 0x6,
       V1000 = 0x7,
       V1050 = 0x8,
       V1100 = 0x9,
       V1150 = 0xA,
       V1200 = 0xB,
       V1250 = 0xC,
       V1300 = 0xD,
       V1350 = 0xE,
       V1400 = 0xF
}  voltageLevel_t;

typedef enum  fine_voltage_level
{
       FINE_V_NONE = 0x0,
       FINE_V_M025 = 0x1,
       FINE_V_P025 = 0x3
} fineVoltageLevel_t;

typedef enum  lp_voltage_level{
       LP_V0700 = 0x0,
       LP_V1200 = 0x1
}  lpVoltageLevel_t;

typedef enum  lp_fine_voltage_level{
       FINE_LP_V_M050  = 0x0,         
       FINE_LP_V_NONE  = 0x1,
       FINE_LP_V_P050  = 0x2,
       FINE_LP_V_P100  = 0x3
} lpFineVoltageLevel_t;


typedef enum  clamp_level{
      CLAMP_OFF100 = 0x0,
      CLAMP_OFF050 = 0x1,
      CLAMP_OFF075 = 0x2,
      CLAMP_OFF125 = 0x3
} clampLevel_t;

///////////////////////////////////////////
// Normal Voltage level selection
//////////////////////////////////////////

#define POWER_NORMAL_VLEVEL(X)      ((X & 0xf)<<0)
#define POWER_NORMAL_FINE_VLEVEL(X) ((X & 0x3)<<4)

///////////////////////////////////////////
// Low Power Mode Voltage level selection
//////////////////////////////////////////

#define POWER_LP_MODE_VLEVEL(X)      ((X & 0x1)<<6)
#define POWER_LP_MODE_FINE_VLEVEL(X) ((X & 0x3)<<14)
#define POWER_FULL_CURRENT_ENABLE(X) ((X & 0x1)<<7) // Not intended to be used. If this bit is 1, we can source full power in Low power mode, else 10% (which is supposed to be enough)

#define POWER_LP_VD1_LEVEL(x)          ((x & 0x1)<<0)
#define POWER_LP_VD2_LEVEL(x)          ((x & 0x1)<<2)
#define POWER_LP_VD3_LEVEL(x)          ((x & 0x1)<<4)
#define POWER_LP_VD8_LEVEL(x)          ((x & 0x1)<<6)
#define POWER_LP_VD1_FULL_CURRENT(x)   ((x & 0x1)<<1)
#define POWER_LP_VD2_FULL_CURRENT(x)   ((x & 0x1)<<3)
#define POWER_LP_VD3_FULL_CURRENT(x)   ((x & 0x1)<<5)
#define POWER_LP_VD8_FULL_CURRENT(x)   ((x & 0x1)<<7)
#define POWER_LP_FINE_VLEVEL(x)        ((x & 0x3)<<8)

/////////////////////////////////////////
//  
//   Clamp selection
//
/////////////////////////////////////////

#define POWER_CLAMP_LOW_ENABLE(X)    ((X & 0x1)<<8)
#define POWER_CLAMP_HIGH_ENABLE(X)   ((X & 0x1)<<11)

#define POWER_CLAMP_LOW_LEVEL(X)     ((X & 0x3)<<9)
#define POWER_CLAMP_HIGH_LEVEL(X)    ((X & 0x3)<<12)

/////////////////////////////////////////
//  
//   Power Switch Acknowledge 
//
/////////////////////////////////////////
#define PWRSWACK_VD2_ANA        (1 << 0)
#define PWRSWACK_VD2_VDDFLASH   (1 << 1)     
#define PWRSWACK_VD3_VDDFLASH   (1 << 2)     
#define PWRSWACK_VD3_RAM0       (1 << 3) 
#define PWRSWACK_VD3_RAM1       (1 << 4) 
#define PWRSWACK_VD3_RAM2       (1 << 5)
#define PWRSWACK_VD3_RAM3       (1 << 6)
#define PWRSWACK_VD3_ROM        (1 << 7) 
#define PWRSWACK_VDDHV          (1 << 8)
#define PWRSWACK_VD7_ENA        (1 << 9)
#define PWRSWACK_VREFP_SW       (1 << 10)

/////////////////////////////////////////
//  
// BOD CTRL
//
////////////////////

#define BOD_RESET_LEVEL(X)  ((X & 0x3)<<0)
#define BOD_RESET_ENABLE(X) ((X & 0x1)<<2)
#define BOD_INTR_LEVEL(X)   ((X & 0x3)<<3)
#define BOD_INTR_ENABLE(X)  ((X & 0x1)<<5)
#define BOD_RESET_STAT(X)   ((X & 0x1)<<6)
#define BOD_INT_STAT(X)     ((X & 0x1)<<7)

///////////////////////////////////////////
// Body Bias bitfields
//////////////////////////////////////////

#define VBB_MODE(x) ((x &(0x7))<<0)  

#define RBB_ENABLE(x)     (x<<8)  
#define RBB_BYPASS_LDO(x) (x<<9)  

#define FBB_ENABLE(x)     (x<<16)  
#define FBB_ENABLE_HP(x)  (x<<17)  
#define FBB_NWS(x)        (x<<18)  
#define FBB_PWS(x)        (x<<19)  

#define FBB_WCODE(x)      ((x &(0x3f))<<0)    // Adjustment for process balancing (offset adjustment reg)


///////////////////////////////////////////
// Deep power down mode wakeup source flags
//////////////////////////////////////////
#define DPDWAKESRC_EXTRESET (1 << 0)
#define DPDWAKESRC_RTC      (1 << 1)
#define DPDWAKESRC_BODRESET (1 << 2)
#define DPDWAKESRC_BODINTR  (1 << 3)

//////////////////////////////////////////////////////////////////////
// Light Sleep ??
//////////////////////////////////////////////////////////////////////

#define PCFG_CLK_OFF_ONLY (LPC_SYSCON->PDRUNCFG | PDRUNCFG_MAINCLK_SHUTOFF)


//////////////////////////////////////////////////////////////////////
// Deep Sleep
//
//  Purpose : Quick wakeup
//  Detail  : full state retention, FLASH is left on (but turn VDDHV off), Low Power mode on VD1,2,3,8
//
//
//////////////////////////////////////////////////////////////////////
#define PCFG_DEEP_SLEEP (    PDRUNCFG_MAINCLK_SHUTOFF   \
                           | PDRUNCFG_LP_VD1            \
                           | PDRUNCFG_PD_IRC_OSC_EN     \
                           | PDRUNCFG_PD_IRC_EN         \
                           | PDRUNCFG_PD_EEPROM         \
                           | PDRUNCFG_PD_BOD_INTR       \
                           | PDRUNCFG_PD_VD2_ANA        \
                           | PDRUNCFG_PD_ADC0           \
                           | PDRUNCFG_PD_ROM            \
                           | PDRUNCFG_PD_VD7_ENA        \
                           | PDRUNCFG_PD_SYS_PLL0       \
                           | PDRUNCFG_PD_VREFP_SW       \
                           | PDRUNCFG_LP_VD2            \
                           | PDRUNCFG_LP_VD3            \
                           | PDRUNCFG_REQ_DELAY         \
                           | PDRUNCFG_LP_VD8            \
                      )

//////////////////////////////////////////////////////////////////////
// PowerDown
//////////////////////////////////////////////////////////////////////

#define PCFG_POWERDOWN  (    PDRUNCFG_MAINCLK_SHUTOFF   \
                           | PDRUNCFG_LP_VD1            \
                           | PDRUNCFG_PD_IRC_OSC_EN     \
                           | PDRUNCFG_PD_IRC_EN         \
                           | PDRUNCFG_PD_FLASH          \
                           | PDRUNCFG_PD_EEPROM         \
                           | PDRUNCFG_PD_BOD_RESET      \
                           | PDRUNCFG_PD_BOD_INTR       \
                           | PDRUNCFG_PD_VD2_ANA        \
                           | PDRUNCFG_PD_ADC0           \
                           | PDRUNCFG_PD_VDDFLASH_ENA   \
                           | PDRUNCFG_VDDFLASH_SEL_VD3  \
                           | PDRUNCFG_PD_RAM1           \
                           | PDRUNCFG_PD_RAM2           \
                           | PDRUNCFG_PD_RAM3           \
                           | PDRUNCFG_PD_ROM            \
                           | PDRUNCFG_PD_VDDHV_ENA      \
                           | PDRUNCFG_PD_VD7_ENA        \
                           | PDRUNCFG_PD_WDT_OSC        \
                           | PDRUNCFG_PD_SYS_PLL0       \
                           | PDRUNCFG_PD_VREFP_SW       \
                           | PDRUNCFG_PD_FLASH_BG       \
                           | PDRUNCFG_LP_VD2            \
                           | PDRUNCFG_LP_VD3            \
                           | PDRUNCFG_LP_VD8            \
                           | PDRUNCFG_REQ_DELAY         \
                           | PDRUNCFG_FORCE_RBB         \
                           | PDRUNCFG_PD_32K_OSC        \
                      )

//////////////////////////////////////////////////////////////////////
// DeepPowerDown
//////////////////////////////////////////////////////////////////////
#define PCFG_DEEP_POWERDOWN 0xFFFFFFFF

typedef struct	_POWER_SETTING_STRUCT2{
	unsigned char prefen_wst_pwr;
}POWER_SETTING_STRUCT2;

typedef struct	_MODE_SETTINGS_STRUCT2{
	POWER_SETTING_STRUCT2 mode[3];
}MODE_SETTINGS_STRUCT2;


typedef struct _PTABLE_DESC {
	unsigned char freq;
	unsigned char res;
} PTABLE_DESC;

typedef	struct _PWRD_API {
  void (*set_pll)(unsigned int cmd[], unsigned int resp[]);
  void (*set_power)(unsigned int cmd[], unsigned int resp[]);
  void (*power_mode_configure)(unsigned int power_mode, unsigned int peripheral_ctrl, unsigned int gint_flag);
  void (*clear_flash_data_latch)(void);
  void (*set_vd_level)(uint32_t domain, uint32_t level, uint32_t flevel);
  void (*set_lpvd_level)(uint32_t vd1LpLevel, uint32_t vd2LpLevel, uint32_t vd3LpLevel, uint32_t vd8LpLevel,
		uint32_t flevel, uint32_t vd1_lp_full_poweren, uint32_t vd2_lp_full_poweren, uint32_t vd3_lp_full_poweren , uint32_t vd8_lp_full_poweren);
  void (*set_clamp_level)(uint32_t domain, uint32_t low_level, uint32_t high_level, uint32_t enable_low_clamp, uint32_t enable_high_clamp);
  void (*enter_RBB)( void );
  void (*enter_NBB)( void );
  void (*enter_FBB)( void );
  void (*set_aclkgate)(unsigned aclkgate);
  unsigned (*get_aclkgate)(void);
}  PWRD_API_T;
  
extern const PWRD_API_T  pwr_api;  //so application program can access	pointer to function table

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __ROM_PWR_412X_H_ */
