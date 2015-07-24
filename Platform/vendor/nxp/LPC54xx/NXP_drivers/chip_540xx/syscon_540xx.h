/*
 * @brief LPC540XX System & Control driver inclusion file
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

#ifndef __SYSCTL_540XX_H_
#define __SYSCTL_540XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SYSCTL_540XX CHIP: LPC540XX System and Control Driver
 * @ingroup CHIP_540XX_Drivers
 * @{
 */

/**
 * System reset status values
 */
#define SYSCTL_RST_POR    (1 << 0)	/*!< POR reset status */
#define SYSCTL_RST_EXTRST (1 << 1)	/*!< External reset status */
#define SYSCTL_RST_WDT    (1 << 2)	/*!< Watchdog reset status */
#define SYSCTL_RST_BOD    (1 << 3)	/*!< Brown-out detect reset status */
#define SYSCTL_RST_SYSRST (1 << 4)	/*!< software system reset status */

// bit definitions
#define PDRUNCFG_LP_VD1           (1 << 2)
#define PDRUNCFG_PD_IRC_OSC_EN    (1 << 3)
#define PDRUNCFG_PD_IRC_EN        (1 << 4)
#define PDRUNCFG_PD_FLASH         (1 << 5)
#define PDRUNCFG_PD_BOD_RESET     (1 << 7)
#define PDRUNCFG_PD_BOD_INTR      (1 << 8)
#define PDRUNCFG_PD_VD2_ANA       (1 << 9)
#define PDRUNCFG_PD_ADC0          (1 << 10)
#define PDRUNCFG_PD_VDDFLASH_ENA  (1 << 11)
#define PDRUNCFG_VDDFLASH_SEL_VD3 (1 << 12)
#define PDRUNCFG_PD_RAM0          (1 << 13)
#define PDRUNCFG_PD_RAM1          (1 << 14)
#define PDRUNCFG_PD_RAM2          (1 << 15)
#define PDRUNCFG_PD_RAM3          (1 << 16)
#define PDRUNCFG_PD_ROM           (1 << 17)
#define PDRUNCFG_PD_VDDHV_ENA     (1 << 18)
#define PDRUNCFG_PD_VD7_ENA       (1 << 19)
#define PDRUNCFG_PD_WDT_OSC       (1 << 20)
#define PDRUNCFG_PD_SYS_PLL0      (1 << 22)
#define PDRUNCFG_PD_VREFP_SW      (1 << 23)
#define PDRUNCFG_PD_32K_OSC       (1 << 24)
#define PDRUNCFG_PD_FLASH_BG      (1 << 25)
#define PDRUNCFG_LP_VD2           (1 << 27)
#define PDRUNCFG_LP_VD3           (1 << 28)
#define PDRUNCFG_LP_VD8           (1UL << 29)

// VFIFO enable positions for VFIFOCTRL reg
#define VFIFO_MINUART0_TX_ENABLE(d) (d << 0)
#define VFIFO_MINUART1_TX_ENABLE(d) (d << 1)
#define VFIFO_MINUART2_TX_ENABLE(d) (d << 2)
#define VFIFO_MINUART3_TX_ENABLE(d) (d << 3)

#define VFIFO_MINUART0_RX_ENABLE(d) (d << 8)
#define VFIFO_MINUART1_RX_ENABLE(d) (d << 9)
#define VFIFO_MINUART2_RX_ENABLE(d) (d << 10)
#define VFIFO_MINUART3_RX_ENABLE(d) (d << 11)

#define VFIFO_LSPI0_TX_ENABLE(d) (d << 4)
#define VFIFO_LSPI1_TX_ENABLE(d) (d << 5)

#define VFIFO_LSPI0_RX_ENABLE(d) (d << 12)
#define VFIFO_LSPI1_RX_ENABLE(d) (d << 13)

/**
 * Non-Maskable Interrupt Enable/Disable value
 */
#define SYSCTL_NMISRC_ENABLE   ((uint32_t) 1 << 31)	/*!< Enable the Non-Maskable Interrupt (NMI) source */

/**
 * Non-Maskable Interrupt Enable/Disable value
 */
#define SYSCTL_FREMEA_ENABLE   ((uint32_t) 1 << 31)	/*!< Enable frequency measure */

/**
 * @brief LPC540XX System Control and Clock register block structure
 */
typedef struct {
	__IO uint32_t SYSMEMREMAP;								// /< System Remap                (0x000)
	__IO uint32_t AHBMATPRIO;								// /< AHB Matrix Priority         (0x004)
	__IO uint32_t AHBFUNPRIO;								// /< AHB Funnel Priority         (0x008)
	__IO uint32_t AHBBUFEN[2];								// /< AHB Buffering Enable        (0x00C)
	__IO uint32_t SYSTCKCAL;								// /< System Tick Calibration     (0x014)
	__IO uint32_t IRQLATENCY;								// /< IRQ Latency (for Cortex-M0) (0x018)
	__IO uint32_t NMISRC;									// /< NMI Source select           (0x01C)
	__IO uint32_t ASYNCVPBCTRL;								// /< Asynch VPB chiplet control  (0x020)
	__I  uint32_t RESERVED0[7];
	__IO uint32_t SYSRSTSTAT;								// /< System Reset Stat           (0x040)
	__IO uint32_t PRESETCTRL[2];							// /< Peripheral Reset Ctrl       (0x044 - 0x048)
	__IO uint32_t PRESETCTRLSET[2];							// /< Peripheral Reset Ctrl Set   (0x04c - 0x050)
	__IO uint32_t PRESETCTRLCLR[2];							// /< Peripheral Reset Ctrl Clr   (0x054 - 0x058)
	__IO uint32_t PIOPORCAP[3];								// /< PIO Power-On Reset Capture  (0x05C - 0x064)
	__IO uint32_t PIORESCAP[2];								// /< PIO Pad Reset Capture       (0x068 - 0x06c)
	__I  uint32_t RESERVED1[4];
	__IO uint32_t MAINCLKSELA;								// /< Main Clk sel Source Sel A   (0x080)
	__IO uint32_t MAINCLKSELB;								// /< Main Clk sel Source Sel B   (0x084)
	__IO uint32_t RESERVED2;								// /< USB Clk sel                 (0x088)
	__IO uint32_t ADCASYNCCLKSEL;							// /< ADC Async Clk Sel           (0x08C)
	__IO uint32_t RESERVED3;
	__IO uint32_t CLKOUTCLKSELA;							// /< Clk Out Sel Source A        (0x094)
	__IO uint32_t CLKOUTCLKSELB;							// /< Clk Out Sel Source B        (0x098)
	__IO uint32_t RESERVED4;
	__IO uint32_t SYSPLLCLKSEL;								// /< System PLL Clk Sel          (0x0A0)
	__I  uint32_t RESERVED5[7];
	__IO uint32_t SYSAHBCLKCTRL[2];							// /< AHB Peripheral Clk Enable    (0x0C0 - 0x0C4)
	__IO uint32_t SYSAHBCLKCTRLSET[2];						// /< AHB Peripheral Clk Enable Set(0x0C8 - 0xCC)
	__IO uint32_t SYSAHBCLKCTRLCLR[2];						// /< AHB Peripheral Clk Enable Clr(0x0D0 - 0xD4)
	__I  uint32_t RESERVED6[2];								// /<
	__IO uint32_t SYSTICKCLKDIV;							// /< Systick Clock divider       (0x0E0)
	__IO uint32_t TRACECLKDIV;								// /< Trace Clk Divider           (0x0E4)
	__I  uint32_t RESERVED7[6];
	__IO uint32_t SYSAHBCLKDIV;								// /< Main Clk Divider            (0x100)
	__IO uint32_t RESERVED8;
	__IO uint32_t ADCASYNCCLKDIV;							// /< ADC Async Clk Divider       (0x108)
	__IO uint32_t CLKOUTDIV;								// /< Clk Out Divider             (0x10c)
	__IO uint32_t RESERVED9[3];
	__IO uint32_t CLKGENUPDATELOCK;							// /< Clockgen (divider and source switch synchroniser disable), if this is one, those can not be updated (0x11C)
	__IO uint32_t FREQMECTRL;								// /< Frequency Measure Control (0x120)
	__IO uint32_t FLASHCFG;									// /< Flash Config (0x124)
	__I  uint32_t RESERVED10[8];
	__IO uint32_t VFIFOCTRL;								// /< VFIFO control (0x148)
	__I  uint32_t RESERVED11[14];
	__IO uint32_t IRCCTRL;									// /< IRC Oscillator Control (0x184)
	__IO uint32_t SYSOSCCTRL;								// /< System Oscillator Control (0x188)
	__IO uint32_t WDTOSCCTRL;								// /< Watchdog Oscillator Control (0x18C)
	__IO uint32_t RTCOSCCTRL;								// /< RTC Oscillator Control(0x190)
	__IO uint32_t PVTDCTRL;									// /< ???? (0x194)
	__I  uint32_t RESERVED12[6];
	__IO uint32_t SYSPLLCTRL;								// /< SYS PLL CTRL 0x1B0
	__IO uint32_t SYSPLLSTAT;								// /< SYS PLL STAT 0x1B4
	__IO uint32_t SYSPLLNDEC;								// /< SYS PLL NDEC 0x1B8
	__IO uint32_t SYSPLLPDEC;								// /< SYS Pll Pdec 0x1BC
	__IO uint32_t SYSPLLSSCGCTRL[2];						// /< Spread Spectrum control 0x1C0  - 0x1c4
	__I  uint32_t RESERVED13[14];
	__IO uint32_t PDSLEEPCFG;								// /< Power Down Sleep Config (0x200)
	__I  uint32_t RESERVED14[3];							// /< Reserved (0x204 - 0x20c)
	__IO uint32_t PDRUNCFG;									// /< Power Down Run Config     (0x210)
	__IO uint32_t PDRUNCFGSET;								// /< Power Down Run Config Set (0x214)
	__IO uint32_t PDRUNCFGCLR;								// /< Power Down Run Config Clr (0x218)
	__IO uint32_t RESERVED15[9];
	__IO uint32_t STARTER[2];								// /< Start Signal Enable Register     (0x240 - 0x244)
	__IO uint32_t STARTERSET[2];							// /< Start Signal Enable Set Register (0x248 - 0x24C)
	__IO uint32_t STARTERCLR[2];							// /< Start Signal Enable Clr Register (0x250 - 0x254)
	__I  uint32_t RESERVED16[37];
	__IO uint32_t IRCPDSTRETCH;								// /< Stretch entry into PD using IRC (to let regulator settle low before going LP) (0x2EC)
	__IO uint32_t IRCSAFETY;								// /< IRC Safety wakeup counter Ctrl (0x2F0)
	__IO uint32_t RETENCTRL;								// /< Retention Ctrl (0x2F4)
	__IO uint32_t PDSAFTY;									// /< Power Down Safety Control (0x2F8)
	__IO uint32_t MAINCLKSAFETY;							// /< Main Clock Wakeup Delay   (0x2FC)
	__IO uint32_t CPUCTRL;									// /< CPU CTRL   (0x300)
	__IO uint32_t CPBOOT;									// /< COPROCESSOR BOOT   (0x304)
	__IO uint32_t CPSTACK;									// /< COPROCESSOR STACK   (0x308)
	__IO uint32_t CPU_STAT;									// /< CPU STAT   (0x30C)
	__I  uint32_t RESERVED17[29];
	__IO uint32_t AUTOCGOR;									// /< Auto Clockgating Override (0x384)
	__I  uint32_t RESERVED18[27];
	__IO uint32_t JTAG_IDCODE;								// /< (0x3F4)
	__IO uint32_t DEVICE_ID0;								// /< (0x3F8)
	__IO uint32_t DEVICE_ID1;								// /< (0x3FC)
} LPC_SYSCTL_T;

typedef struct {
	__IO uint32_t ASYNCPRESETCTRL;							// /< peripheral reset             (0x000)
	__IO uint32_t ASYNCPRESETCTRLSET;						// /< peripheral reset  Set        (0x004)
	__IO uint32_t ASYNCPRESETCTRLCLR;						// /< peripheral reset  Clr        (0x008)
	__I  uint32_t RESERVED0;								// /<
	__IO uint32_t ASYNCVPBCLKCTRL;							// /< clk enable                   (0x010)
	__IO uint32_t ASYNCVPBCLKCTRLSET;						// /< clk enable        Set        (0x014)
	__IO uint32_t ASYNCVPBCLKCTRLCLR;						// /< clk enable        Clr        (0x018)
	__I  uint32_t RESERVED1;								// /<
	__IO uint32_t ASYNCVPBCLKSELA;							// /< clk source mux A             (0x020)
	__IO uint32_t ASYNCVPBCLKSELB;							// /< clk source mux B             (0x024)
	__IO uint32_t ASYNCCLKDIV;								// /< clk div                      (0x028)
	__IO uint32_t ASYNCAUTOCGOR;							// /< autoclk  override            (0x02c)
	__IO uint32_t FRGCTRL;									// /< Fraction Rate Generator Ctrl (0x030)
} LPC_ASYNC_SYSCTL_T;

/**
 * Peripheral reset identifiers
 */
typedef enum {
	STARTERP0_WWDT = 0,
	STARTERP0_BOD,
	STARTERP0_FLASH,
	STARTERP0_DMA,
	STARTERP0_GINT0,
	STARTERP0_PINT0,
	STARTERP0_PINT1,
	STARTERP0_PINT2,
	STARTERP0_PINT3,
	STARTERP0_UTICK,
	STARTERP0_MRT,
	STARTERP0_CTIMER0,
	STARTERP0_CTIMER1,
	STARTERP0_CTIMER2,
	STARTERP0_CTIMER3,
	STARTERP0_CTIMER4,
	STARTERP0_SCT,
	STARTERP0_UART0,
	STARTERP0_UART1,
	STARTERP0_UART2,
	STARTERP0_UART3,
	STARTERP0_I2C0,
	STARTERP0_I2C1,
	STARTERP0_I2C2,
	STARTERP0_SPI0,
	STARTERP0_SPI1,
	STARTERP0_ADC_SEQA,
	STARTERP0_ADC_SEQB,
	STARTERP0_ADC_OVCR,
	STARTERP0_RTC,
	STARTERP0_MBOX = 31,
	/* For M4 only */
	STARTERP1_GINT1 = 32 + 0,
	STARTERP1_PINT4,
	STARTERP1_PINT5,
	STARTERP1_PINT6,
	STARTERP1_PINT7,
	STARTERP1_OSTMR = 32 + 8,
} CHIP_SYSCTL_WAKEUP_T;

/**
 * System memory remap modes used to remap interrupt vectors
 */
typedef enum CHIP_SYSCTL_BOOT_MODE_REMAP {
	REMAP_BOOT_LOADER_MODE,	/*!< Interrupt vectors are re-mapped to Boot ROM */
	REMAP_USER_RAM_MODE,	/*!< Interrupt vectors are re-mapped to user Static RAM */
	REMAP_USER_FLASH_MODE	/*!< Interrupt vectors are not re-mapped and reside in Flash */
} CHIP_SYSCTL_BOOT_MODE_REMAP_T;

/**
 * Peripheral reset identifiers
 */
typedef enum {
	/* Peripheral reset enables for PRESETCTRL0 */
	RESET_FLASH = 7,				/*!< FLASH clock */
	RESET_GPIO0 = 14,				/*!< GPIO Port 0 */
	RESET_GPIO1,					/*!< GPIO Port 1 */
	RESET_PININT = 18,			/*!< Pin Interrupt Block */
	RESET_GINT,							/*!< Group Interrupt Block */
	RESET_DMA,							/*!< DMA clock */
	RESET_CRC,						/*!< CRC clock */
	RESET_WWDT,						/*!< WDT clock */
	RESET_RTC,						/*!< RTC clock */
	RESET_ADC0 = 27,				/*!< ADC0 clock */

	/* Peripheral reset enables for PRESETCTRL1 */
	RESET_MRT = 32,					/*!< multi-rate timer clock */
	RESET_OSTIMER,						/*!< OS timer clock */
	RESET_SCT0,					/*!< SCT0 clock */
	RESET_VFIFO = 32 + 9,		/*!< VFIFO clock */
	RESET_UTICK,							/*!< UTICK clock */
	RESET_CTIMER2 = 32 + 22,		/*!< TIMER2 clock */
	RESET_CTIMER3 = 32 + 26,		/*!< TIMER3 clock */
	RESET_CTIMER4,					/*!< TIMER4 clock */
	RESET_PVT,							/*!< PVT clock */
	RESET_BODY_BIAS,						/*!< Body Bias clock */
	RESET_EZH_A,						/*!< EZH_A clock */
	RESET_EZH_B,						/*!< EZH_B clock */
} CHIP_SYSCTL_PERIPH_RESET_T;

/**
 * Async peripheral reset identifiers
 */
typedef enum {
	/* Async peripheral reset enables for ASYNCPRESETCTRL */
	ASYNC_RESET_UART0 = 1,			/*!< UART0 clock */
	ASYNC_RESET_UART1,					/*!< UART1 clock */
	ASYNC_RESET_UART2,					/*!< UART2 clock */
	ASYNC_RESET_UART3,					/*!< UART3 clock */
	ASYNC_RESET_I2C0,					/*!< I2C0  clock */
	ASYNC_RESET_I2C1,					/*!< I2C1  clock */
	ASYNC_RESET_I2C2,					/*!< I2C2  clock */
	ASYNC_RESET_SPI0 = 9,				/*!< SPI0  clock */
	ASYNC_RESET_SPI1,						/*!< SPI1  clock */
	ASYNC_RESET_SPI2,					/*!< SPI2  clock */
	ASYNC_RESET_CTIMER0 = 13,		/*!< TIMER0 clock */
	ASYNC_RESET_CTIMER1,				/*!< TIMER1 clock */
	ASYNC_RESET_FRG,						/*!< FRG clock */
} CHIP_ASYNC_SYSCTL_PERIPH_RESET_T;

/**
 * Brown-out detector reset level
 */
typedef enum CHIP_SYSCTL_BODRSTLVL {
	SYSCTL_BODRSTLVL_0,	/*!< Brown-out reset at 1.46 ~ 1.63v */
	SYSCTL_BODRSTLVL_1,	/*!< Brown-out reset at 2.06v ~ 2.15v */
	SYSCTL_BODRSTLVL_2,	/*!< Brown-out reset at 2.35v ~ 2.43v */
	SYSCTL_BODRSTLVL_3,	/*!< Brown-out reset at 2.63v ~ 2.71v */
} CHIP_SYSCTL_BODRSTLVL_T;

/**
 * Brown-out detector interrupt level
 */
typedef enum CHIP_SYSCTL_BODRINTVAL {
	SYSCTL_BODINTVAL_LVL0,	/* Brown-out interrupt at 1.65 ~ 1.80v */
	SYSCTL_BODINTVAL_LVL1,	/* Brown-out interrupt at 2.22v ~ 2.35v*/
	SYSCTL_BODINTVAL_LVL2,	/* Brown-out interrupt at 2.52v ~ 2.66v */
	SYSCTL_BODINTVAL_LVL3,	/* Brown-out interrupt at 2.80v ~ 2.90v */
} CHIP_SYSCTL_BODRINTVAL_T;

/**
 * @brief	Re-map interrupt vectors
 * @param	remap	: system memory map value
 * @return	Nothing
 */
STATIC INLINE void Chip_SYSCTL_Map(CHIP_SYSCTL_BOOT_MODE_REMAP_T remap)
{
	LPC_SYSCTL->SYSMEMREMAP = (uint32_t) remap;
}

/**
 * @brief	Get system remap setting
 * @param	Nothing
 * @return	System remap setting
 */
STATIC INLINE CHIP_SYSCTL_BOOT_MODE_REMAP_T Chip_SYSCTL_GetMemoryMap(void)
{
	return (CHIP_SYSCTL_BOOT_MODE_REMAP_T) LPC_SYSCTL->SYSMEMREMAP;
}

/**
 * @brief	Re-map interrupt vectors
 * @param	remap	: system memory map value
 * @return	Nothing
 */
STATIC INLINE void Chip_SYSCTL_Enable_ASYNC_Syscon(bool enable)
{
	if ( enable ) {
		LPC_SYSCTL->ASYNCVPBCTRL = (uint32_t) 0x01;
	}
	else {
		LPC_SYSCTL->ASYNCVPBCTRL = (uint32_t) 0x00;
	}
}

/**
 * @brief	Resets a peripheral
 * @param	periph	:	Peripheral to reset
 * @return	Nothing
 */
STATIC INLINE void Chip_SYSCTL_PeriphReset(CHIP_SYSCTL_PERIPH_RESET_T periph)
{
	if (periph >= 32) {
		LPC_SYSCTL->PRESETCTRLSET[1] = (1 << ((uint32_t) periph - 32));
		LPC_SYSCTL->PRESETCTRLCLR[1] = (1 << ((uint32_t) periph - 32));
	}
	else {
		LPC_SYSCTL->PRESETCTRLSET[0] = (1 << (uint32_t) periph);
		LPC_SYSCTL->PRESETCTRLCLR[0] = (1 << (uint32_t) periph);
	}
}

/**
 * @brief	Resets a peripheral
 * @param	periph	:	Peripheral to reset
 * @return	Nothing
 */
STATIC INLINE void Chip_SYSCTL_AsyncPeriphReset(CHIP_ASYNC_SYSCTL_PERIPH_RESET_T periph)
{
	LPC_ASYNC_SYSCTL->ASYNCPRESETCTRLSET = (1 << (uint32_t) periph);
	LPC_ASYNC_SYSCTL->ASYNCPRESETCTRLCLR = (1 << (uint32_t) periph);

}

/**
 * @brief	Get system reset status
 * @return	An Or'ed value of SYSCTL_RST_*
 * @note	This function returns the detected reset source(s).
 */
STATIC INLINE uint32_t Chip_SYSCTL_GetSystemRSTStatus(void)
{
	return LPC_SYSCTL->SYSRSTSTAT;
}

/**
 * @brief	Clear system reset status
 * @param	reset	: An Or'ed value of SYSCTL_RST_* status to clear
 * @return	Nothing
 * @note	This function clears the specified reset source(s).
 */
STATIC INLINE void Chip_SYSCTL_ClearSystemRSTStatus(uint32_t reset)
{
	LPC_SYSCTL->SYSRSTSTAT = reset;
}

/**
 * @brief	Read POR captured PIO status
 * @return	captured POR PIO status
 * @note	Some devices only support index 0.
 */
STATIC INLINE uint32_t Chip_SYSCTL_GetPORPIOStatus(void)
{
	return LPC_SYSCTL->PIOPORCAP[0];
}

/**
 * @brief	Set System tick timer calibration value
 * @param	sysCalVal	: System tick timer calibration value
 * @return	Nothing
 */
STATIC INLINE void Chip_SYSCTL_SetSYSTCKCAL(uint32_t sysCalVal)
{
	LPC_SYSCTL->SYSTCKCAL = sysCalVal;
}

/**
 * @brief	Set System IRQ latency
 * @param	latency	: Latency in clock ticks
 * @return	Nothing
 * @note	Sets the IRQ latency, a value between 0 and 255 clocks. Lower
 * values allow better latency
 */
STATIC INLINE void Chip_SYSCTL_SetIRQLatency(uint32_t latency)
{
	LPC_SYSCTL->IRQLATENCY = latency;
}

/**
 * @brief	Get System IRQ latency value
 * @return	IRQ Latency in clock ticks
 */
STATIC INLINE uint32_t Chip_SYSCTL_GetIRQLatency(void)
{
	return LPC_SYSCTL->IRQLATENCY;
}

/**
 * @brief	Set Frequency Measure Control
 * @param	freq measure control value:
 * @return	Nothing
 * @note	Setup frequency measure control
 */
STATIC INLINE void Chip_SYSCTL_SetFreqMeasure(uint32_t freqmeasure) // FIXME - can be deleted?
{
	LPC_SYSCTL->FREQMECTRL = freqmeasure;
}

/**
 * @brief	Get frequency measure value
 * @return	frequency measure value
 */
STATIC INLINE uint32_t Chip_SYSCTL_GetFreqMeasure(void) // FIXME - can be deleted?
{
	return LPC_SYSCTL->FREQMECTRL;
}

/**
 * @brief	Starts a frequency measurement cycle
 * @return	Nothing
 * @note	This function is meant to be used with the Chip_INMUX_SetFreqMeasRefClock()
 * and Chip_INMUX_SetFreqMeasTargClock() functions.
 */
STATIC INLINE void Chip_SYSCTL_StartFreqMeas(void)
{
	LPC_SYSCTL->FREQMECTRL = 0;
	LPC_SYSCTL->FREQMECTRL = (1UL << 31);
}

/**
 * @brief	Indicates when a frequency measurement cycle is complete
 * @return	true if a measurement cycle is active, otherwise false
 */
STATIC INLINE bool Chip_SYSCTL_IsFreqMeasComplete(void)
{
	return (bool) ((LPC_SYSCTL->FREQMECTRL & (1UL << 31)) == 0);
}

/**
 * @brief	Returns the raw capture value for a frequency measurement cycle
 * @return	raw cpature value (this is not a frequency)
 */
STATIC INLINE uint32_t Chip_SYSCTL_GetRawFreqMeasCapval(void)
{
	return LPC_SYSCTL->FREQMECTRL & 0x3FFF;
}

/**
 * @brief	Returns the computed value for a frequency measurement cycle
 * @param	refClockRate	: Reference clock rate used during the frequency measurement cycle
 * @return	Computed cpature value
 */
uint32_t Chip_SYSCTL_GetCompFreqMeas(uint32_t refClockRate);

/**
 * @brief	Set source for non-maskable interrupt (NMI)
 * @param	intsrc	: IRQ number to assign to the NMI
 * @return	Nothing
 * @note	The NMI source will be disabled upon exiting this function. Use the
 * Chip_SYSCTL_EnableNMISource() function to enable the NMI source
 */
STATIC INLINE void Chip_SYSCTL_SetNMISource(uint32_t intsrc)
{
	LPC_SYSCTL->NMISRC = intsrc;
}

/**
 * @brief	Enable interrupt used for NMI source
 * @return	Nothing
 */
STATIC INLINE void Chip_SYSCTL_EnableNMISource(void)
{
	LPC_SYSCTL->NMISRC |= SYSCTL_NMISRC_ENABLE;
}

/**
 * @brief	Disable interrupt used for NMI source
 * @return	Nothing
 */
STATIC INLINE void Chip_SYSCTL_DisableNMISource(void)
{
	LPC_SYSCTL->NMISRC &= ~(SYSCTL_NMISRC_ENABLE);
}

/**
 * @brief	Enables a pin's (PINT) wakeup logic
 * @param	pin	: pin number
 * @return	Nothing
 * @note	Different devices support different pins, see the user manual
 * for supported pins
 */
STATIC INLINE void Chip_SYSCTL_EnableWakeup(CHIP_SYSCTL_WAKEUP_T periphmask)
{
	if ( periphmask < 32 ) {
		LPC_SYSCTL->STARTERSET[0] = (1 << periphmask);
	}
	else {
		LPC_SYSCTL->STARTERSET[1] = (1 << (periphmask - 32));
	}
}

/**
 * @brief	Disables peripheral's wakeup logic
 * @param	periphmask	: OR'ed values of SYSCTL_WAKEUP_* for wakeup
 * @return	Nothing
 */
STATIC INLINE void Chip_SYSCTL_DisableWakeup(CHIP_SYSCTL_WAKEUP_T periphmask)
{
	if ( periphmask < 32 ) {
		LPC_SYSCTL->STARTERCLR[0] = (1 << periphmask);
	}
	else {
		LPC_SYSCTL->STARTERCLR[1] = (1 << (periphmask - 32));
	}
}

/**
 * @brief	Power up one or more blocks or peripherals
 * @return	OR'ed values of SYSCTL_SLPWAKE_* values
 * @note	A high state indicates the peripheral is powered down.
 */
STATIC INLINE uint32_t Chip_SYSCTL_GetPowerStates(void)
{
	return LPC_SYSCTL->PDRUNCFG;
}

/**
 * @brief	Return the device ID
 * @return	Device ID
 */
STATIC INLINE uint32_t Chip_SYSCTL_GetDeviceID(void)
{
	return LPC_SYSCTL->DEVICE_ID0;
}

/**
 * @brief	Power down one or more blocks or peripherals
 * @param	powerdownmask	: OR'ed values of SYSCTL_PDRUNCFG_* values
 * @return	Nothing
 */
void Chip_SYSCTL_PowerDown(uint32_t powerdownmask);

/**
 * @brief	Power up one or more blocks or peripherals
 * @param	powerupmask	: OR'ed values of SYSCTL_PDRUNCFG_* values
 * @return	Nothing
 */
void Chip_SYSCTL_PowerUp(uint32_t powerupmask);

/**
 * @brief	If one or more blocks or peripherals are powered up or not
 * @param	powerupmask	: OR'ed values of SYSCTL_PDRUNCFG_* values
 * @return	zero is powered up, non-zero is powered down.
 */
uint32_t Is_Chip_SYSCTL_PowerUp(uint32_t powerupmask);

/**
 * @}
 */

/**
 * @brief	Set the interrupt number for NMI interrupt
 * @param	NMI_num	: IRQ number
 * @return None
 * @note	Before this module is called, the NVIC interrupt needs to be disabled.
 */
void Chip_SYSCTL_NMI_Enable(uint32_t NMI_num);

/**
 * @}
 */

/**
 * @brief FLASH Access time definitions
 */
typedef enum {
	FLASHTIM_20MHZ_CPU = 2,		/*!< Flash accesses use 2 CPU clocks. Use for up to 20 MHz CPU clock*/
	FLASHTIM_72MHZ_CPU = 5,		/*!< Flash accesses use 4 CPU clocks. Use for up to 72 MHz CPU clock*/
} FMC_FLASHTIM_T;

/**
 * @brief	Set FLASH memory access time in clocks
 * @param	clks	: Clock cycles for FLASH access
 * @return	Nothing
 * @note	For CPU speed up to 20MHz, use a value of 0. For up to 30MHz, use
 *			a value of 1
 */
STATIC INLINE void Chip_FMC_SetFLASHAccess(FMC_FLASHTIM_T clks)
{
	uint32_t tmp;
	tmp = LPC_SYSCTL->FLASHCFG & ~(0xF << 12);
	/* Don't alter lower bits */
	LPC_SYSCTL->FLASHCFG = tmp | (clks << 12);
}

#ifdef __cplusplus
}
#endif

#endif /* __SYSCTL_540XX_H_ */
