/*
 * @brief LPC540XX clock driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licenser disclaim any and
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

#ifndef __CLOCK_540XX_H_
#define __CLOCK_540XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup CLOCK_540XX CHIP: LPC540XX Clock Driver
 * @ingroup CHIP_540XX_Drivers
 * @{
 */

/* Internal oscillator frequency */
#define SYSCTL_IRC_FREQ     (12000000)
#define SYSCTL_WDTOSC_FREQ  (500000)
#define SYSCTL_RTC_FREQ     (32000)

/**
 * Clock sources for system PLLs
 */
typedef enum CHIP_SYSCTL_PLLCLKSRC {
	SYSCTL_PLLCLKSRC_IRC = 0,		/*!< Internal oscillator */
	SYSCTL_PLLCLKSRC_CLKIN, 		/*!< Crystal (system) oscillator */
	SYSCTL_PLLCLKSRC_WDTOSC, 		/*!< WDT oscillator */
	SYSCTL_PLLCLKSRC_RTC, 			/*!< RTC oscillator */
} CHIP_SYSCTL_PLLCLKSRC_T;

/**
 * System and peripheral clocks enum
 */
typedef enum CHIP_SYSCTL_CLOCK {
	/* Peripheral clock enables for SYSAHBCLKCTRL0 */
	SYSCTL_CLOCK_SYS = 0,				/*!< System clock */
	SYSCTL_CLOCK_ROM,					/*!< ROM clock */
	SYSCTL_CLOCK_SRAM1 = 3,				/*!< SRAM1 clock */
	SYSCTL_CLOCK_SRAM2,					/*!< SRAM2 clock */
	SYSCTL_CLOCK_FLASH = 7,				/*!< FLASH controller clock */
	SYSCTL_CLOCK_FMC, 				/*!< FMC clock */
	SYSCTL_CLOCK_MUX = 11,				/*!< Input mux clock */
	SYSCTL_CLOCK_IOCON = 13,		/*!< IOCON clock */
	SYSCTL_CLOCK_GPIO0,					/*!< GPIO0 clock */
	SYSCTL_CLOCK_GPIO1,					/*!< GPIO1 clock */
	SYSCTL_CLOCK_PININT = 18,			/*!< PININT clock */
	SYSCTL_CLOCK_GINT,					/*!< grouped pin interrupt block clock */
	SYSCTL_CLOCK_DMA,					/*!< DMA clock */
	SYSCTL_CLOCK_CRC,					/*!< CRC clock */
	SYSCTL_CLOCK_WWDT,				/*!< WDT clock */
	SYSCTL_CLOCK_RTC,					/*!< RTC clock */
	SYSCTL_CLOCK_MBOX = 26,				/*!< Mailbox clock */
	SYSCTL_CLOCK_ADC0,				/*!< ADC0 clock */

	/* Peripheral clock enables for SYSAHBCLKCTRL1 */
	SYSCTL_CLOCK_MRT = 32,				    /*!< multi-rate timer clock */
	SYSCTL_CLOCK_OSTIMER,					    /*!< OS timer clock */
	SYSCTL_CLOCK_SCT0,		            /*!< SCT0 clock */
	SYSCTL_CLOCK_VFIFO = 32 + 9,	    /*!< VFIFO clock */
	SYSCTL_CLOCK_UTICK,					      /*!< UTICK clock */
	SYSCTL_CLOCK_CTIMER2 = 32 + 22,	  /*!< CTIMER2 clock */
	SYSCTL_CLOCK_CTIMER3 = 32 + 26,		/*!< CTIMER3 clock */
	SYSCTL_CLOCK_CTIMER4,		          /*!< CTIMER4 clock */
	SYSCTL_CLOCK_PVT,					        /*!< PVT clock */
	SYSCTL_CLOCK_BODY_BIAS,					  /*!< Body Bias clock */
	SYSCTL_CLOCK_EZH_A,			          /*!< EZH_A clock */
	SYSCTL_CLOCK_EZH_B,			          /*!< EZH_B clock */
} CHIP_SYSCTL_CLOCK_T;

/**
 * System and peripheral clocks enum
 */
typedef enum CHIP_ASYNC_SYSCTL_CLOCK {
	/* Peripheral clock enables for SYSAHBCLKCTRL0 */
	ASYNC_SYSCTL_CLOCK_UART0 = 1,			/*!< UART0 clock */
	ASYNC_SYSCTL_CLOCK_UART1,					/*!< UART1 clock */
	ASYNC_SYSCTL_CLOCK_UART2,				  /*!< UART2 clock */
	ASYNC_SYSCTL_CLOCK_UART3,					/*!< UART3 clock */
	ASYNC_SYSCTL_CLOCK_I2C0,			 	  /*!< I2C0  clock */
	ASYNC_SYSCTL_CLOCK_I2C1, 				  /*!< I2C1  clock */
	ASYNC_SYSCTL_CLOCK_I2C2,				  /*!< I2C2  clock */
	ASYNC_SYSCTL_CLOCK_SPI0 = 9,			/*!< SPI0  clock */
	ASYNC_SYSCTL_CLOCK_SPI1,					/*!< SPI1  clock */
	ASYNC_SYSCTL_CLOCK_SPI2,			    /*!< SPI2  clock */
	ASYNC_SYSCTL_CLOCK_CTIMER0 = 13,	/*!< CTIMER0 clock */
	ASYNC_SYSCTL_CLOCK_CTIMER1,				/*!< CTIMER1 clock */
	ASYNC_SYSCTL_CLOCK_FRG,					  /*!< FRG clock */
} CHIP_ASYNC_SYSCTL_CLOCK_T;

/**
 * Clock sources for CLKOUT
 */
typedef enum CHIP_SYSCTL_CLKOUTSRC {
	SYSCTL_CLKOUTSRC_MAINSYSCLK = 0,	/*!< Main system clock for CLKOUT */
	SYSCTL_CLKOUTSRC_CLKIN,		  /*!< CLKIN for CLKOUT */
	SYSCTL_CLKOUTSRC_WDTOSC,		/*!< Watchdog oscillator for CLKOUT */
	SYSCTL_CLKOUTSRC_IRC,		    /*!< Internal oscillator for CLKOUT */
	SYSCTL_CLKOUTSRCA_OUTPUT,		  /*!< clkoutA output route to input of clkoutB */
	SYSCTL_CLKOUTSRC_RTC = 7				  /*!< RTC oscillator 32KHz output */
} CHIP_SYSCTL_CLKOUTSRC_T;

/* SYS PLL related bit fields */
#define SYS_PLL_BYPASS_PLL(d)    (d<<15)
#define SYS_PLL_BYPASS_FBDIV2(d) (d<<16)
#define SYS_PLL_LIMUPOFF(d) (d<<17)
#define SYS_PLL_BANDSEL(d)  (d<<18)
#define SYS_PLL_DIRECTI(d)  (d<<19)
#define SYS_PLL_DIRECTO(d)  (d<<20)
#define SYS_PLL_INSELR(d)   ((d&0xf)<<0)      
#define SYS_PLL_INSELI(d)   ((d&0x3f)<<4) 
#define SYS_PLL_INSELP(d)   ((d&0x1f)<<10) 

/**
 * Clock source selections for only the main A system clock. The main A system
 * clock is used as an input into the main B system clock selector. Main clock A
 * only needs to be setup if the main clock A input is used in the main clock
 * system selector.
 */
typedef enum CHIP_SYSCTL_MAIN_A_CLKSRC {
	SYSCTL_MAIN_A_CLKSRC_IRC = 0,		/*!< Internal oscillator */
	SYSCTL_MAIN_A_CLKSRCA_CLKIN,		/*!< Crystal (main) oscillator in */
	SYSCTL_MAIN_A_CLKSRCA_WDTOSC,		/*!< Watchdog oscillator rate */
} CHIP_SYSCTL_MAIN_A_CLKSRC_T;

/**
 * @brief	Set main A system clock source
 * @param	src	: Clock source for main A
 * @return	Nothing
 * @note	This function only neesd to be setup if main clock A will be
 * selected in the Chip_Clock_GetMain_B_ClockRate() function.
 */
STATIC INLINE void Chip_Clock_SetMain_A_ClockSource(CHIP_SYSCTL_MAIN_A_CLKSRC_T src)
{
	LPC_SYSCTL->MAINCLKSELA = (uint32_t) src;
}

/**
 * @brief   Returns the main A clock source
 * @return	Returns which clock is used for the main A
 */
STATIC INLINE CHIP_SYSCTL_MAIN_A_CLKSRC_T Chip_Clock_GetMain_A_ClockSource(void)
{
	return (CHIP_SYSCTL_MAIN_A_CLKSRC_T) (LPC_SYSCTL->MAINCLKSELA);
}
/**
 * @brief	Return main A clock rate
 * @return	main A clock rate in Hz
 */
uint32_t Chip_Clock_GetMain_A_ClockRate(void);

/**
 * Clock sources for only main B system clock
 */
typedef enum CHIP_SYSCTL_MAIN_B_CLKSRC {
	SYSCTL_MAIN_B_CLKSRC_MAINCLKSELA = 0,	/*!< main clock A */
	SYSCTL_MAIN_B_CLKSRC_SYSPLLIN,			/*!< System PLL input */
	SYSCTL_MAIN_B_CLKSRC_SYSPLLOUT,			/*!< System PLL output */
	SYSCTL_MAIN_B_CLKSRC_RTC,				/*!< RTC oscillator 32KHz output */
} CHIP_SYSCTL_MAIN_B_CLKSRC_T;

/**
 * @brief	Set main B system clock source
 * @param	src	: Clock source for main B
 * @return	Nothing
 */
STATIC INLINE void Chip_Clock_SetMain_B_ClockSource(CHIP_SYSCTL_MAIN_B_CLKSRC_T src)
{
	LPC_SYSCTL->MAINCLKSELB = (uint32_t) src;
}

/**
 * @brief   Returns the main B clock source
 * @return	Returns which clock is used for the main B
 */
STATIC INLINE CHIP_SYSCTL_MAIN_B_CLKSRC_T Chip_Clock_GetMain_B_ClockSource(void)
{
	return (CHIP_SYSCTL_MAIN_B_CLKSRC_T) (LPC_SYSCTL->MAINCLKSELB);
}

/**
 * @brief	Return main B clock rate
 * @return	main B clock rate
 */
uint32_t Chip_Clock_GetMain_B_ClockRate(void);


/**
 * Clock source selections for the AsyncSyscon A system clock. The AsyncSyscon A system
 * clock is used as an input into the AsyncSyscon B system clock selector. AsyncSyscon clock A
 * only needs to be setup if the AsyncSyscon A input is used in the AsyncSyscon clock
 * system selector.
 */
typedef enum CHIP_ASYNC_SYSCTL_A_SRC {
	ASYNC_SYSCTL_IRC,		        /*!< IRC or RingOSC input (TBD) */
	ASYNC_SYSCTL_WDTOSC,		    /*!< external CLK input */
} CHIP_ASYNC_SYSCTL_A_SRC_T;

/**
 * @brief	Set AsyncSyscon A system clock source
 * @param	src	: Clock source for AsyncSyscon A
 * @return	Nothing
 * @note	This function only neesd to be setup if AsyncSyscon clock A will be
 * selected in the Chip_Clock_GetAsyncSyscon_B_ClockRate() function.
 */
STATIC INLINE void Chip_Clock_SetAsyncSyscon_A_ClockSource(CHIP_ASYNC_SYSCTL_A_SRC_T src)
{
	LPC_ASYNC_SYSCTL->ASYNCVPBCLKSELA = (uint32_t) src;
}

/**
 * @brief   Returns the AsyncSyscon A clock source
 * @return	Returns which clock is used for the AsyncSyscon A
 */
STATIC INLINE CHIP_ASYNC_SYSCTL_A_SRC_T Chip_Clock_GetAsyncSyscon_A_ClockSource(void)
{
	return (CHIP_ASYNC_SYSCTL_A_SRC_T) (LPC_ASYNC_SYSCTL->ASYNCVPBCLKSELA);
}
/**
 * @brief	Return AsyncSyscon A clock rate
 * @return	AsyncSyscon A clock rate in Hz
 */
uint32_t Chip_Clock_GetAsyncSyscon_A_ClockRate(void);

/**
 * Clock sources for only AsyncSyscon B system clock
 */
typedef enum CHIP_ASYNC_SYSCTL_B_SRC {
	ASYNC_SYSCTL_MAINCLK = 0,		/*!< Internal oscillator */
	ASYNC_SYSCTL_CLKIN,		      /*!< external CLK input */
	ASYNC_SYSCTL_PLLOUT,		    /*!< PLL output */
	ASYNC_SYSCTL_A_OUTPUT,		  /*!< output from ASYNC APBCLKSEL_A */
} CHIP_ASYNC_SYSCTL_B_SRC_T;

/**
 * @brief	Set AsyncSyscon B system clock source
 * @param	src	: Clock source for AsyncSyscon B
 * @return	Nothing
 */
STATIC INLINE void Chip_Clock_SetAsyncSyscon_B_ClockSource(CHIP_ASYNC_SYSCTL_B_SRC_T src)
{
	LPC_ASYNC_SYSCTL->ASYNCVPBCLKSELB = (uint32_t) src;
}

/**
 * @brief   Returns the AsyncSyscon B clock source
 * @return	Returns which clock is used for the AsyncSyscon B
 */
STATIC INLINE CHIP_ASYNC_SYSCTL_B_SRC_T Chip_Clock_GetAsyncSyscon_B_ClockSource(void)
{
	return (CHIP_ASYNC_SYSCTL_B_SRC_T) (LPC_ASYNC_SYSCTL->ASYNCVPBCLKSELB);
}

/**
 * @brief	Return main B clock rate
 * @return	main B clock rate
 */
uint32_t Chip_Clock_GetAsyncSyscon_B_ClockRate(void);


/**
 * Clock sources for ADC asynchronous clock source select	
 */
typedef enum CHIP_SYSCTL_ADCASYNCCLKSRC {
	SYSCTL_ADCASYNCCLKSRC_MAINCLK = 0, 	  /*!< Main clock */
	SYSCTL_ADCASYNCCLKSRC_SYSPLLOUT,      /*!< PLL output */
	SYSCTL_ADCASYNCCLKSRC_IRC             /*!< Internal oscillator */
}
CHIP_SYSCTL_ADCASYNCCLKSRC_T;

/**
 * @brief	Set the ADC asynchronous clock source
 * @param	src	: ADC asynchronous clock source
 * @return	Nothing
 */
STATIC INLINE void Chip_Clock_SetADCASYNCSource(CHIP_SYSCTL_ADCASYNCCLKSRC_T src)
{
	LPC_SYSCTL->ADCASYNCCLKSEL = (uint32_t) src;
}

/**
 * @brief   Returns the ADC asynchronous clock source
 * @return	Returns which clock is used for the ADC asynchronous clock source
 */
STATIC INLINE CHIP_SYSCTL_ADCASYNCCLKSRC_T Chip_Clock_GetADCASYNCSource(void)
{
	return (CHIP_SYSCTL_ADCASYNCCLKSRC_T) (LPC_SYSCTL->ADCASYNCCLKSEL);
}

/**
 * @brief	Set the ADC asynchronous clock source
 * @param	src	: ADC asynchronous clock source
 * @return	Nothing
 */
STATIC INLINE void Chip_Clock_SetADCASYNCDivider(uint32_t div)
{
	LPC_SYSCTL->ADCASYNCCLKDIV = (uint32_t) div;
}

/**
 * @brief	Get the ADC asynchronous clock source
 * @param	src	: ADC asynchronous clock source
 * @return	Nothing
 */
STATIC INLINE uint32_t Chip_Clock_GetADCASYNCDivider(void)
{
	return (LPC_SYSCTL->ADCASYNCCLKDIV);
}

/**
 * @brief	Return ADC asynchronous clock rate
 * @return	ADC asynchronous clock rate
 */
uint32_t Chip_Clock_GetADCASYNCRate(void);

/**
 * Clock sources for main system clock. This is a mix of both main clock A
 * and B seelctions.
 */
typedef enum CHIP_SYSCTL_MAINCLKSRC {
	SYSCTL_MAINCLKSRC_IRC = 0,			/*!< Internal oscillator */
	SYSCTL_MAINCLKSRC_CLKIN,			  /*!< Crystal (main) oscillator in */
	SYSCTL_MAINCLKSRC_WDTOSC,			  /*!< Watchdog oscillator rate */
	SYSCTL_MAINCLKSRCA_OUTPUT,		  /*!< MainclkA output route to input of MainclkB */
	SYSCTL_MAINCLKSRC_PLLIN,		    /*!< System PLL input */
	SYSCTL_MAINCLKSRC_PLLOUT,		    /*!< System PLL output */
	SYSCTL_MAINCLKSRC_RTC,				  /*!< RTC oscillator 32KHz output */
} CHIP_SYSCTL_MAINCLKSRC_T;

/**
 * Clock sources for AsyncSyscon system clock. This is a mix of both AsyncSyscon clock A
 * and B seelctions.
 */
typedef enum CHIP_SYSCTL_ASYNCSYSCONCLKSRC {
	ASYNC_SYSCTL_CLOCK_MAINCLK = 0,		    /*!< MainclkA output route to input of AsyncSysconClkB */
	ASYNC_SYSCTL_CLOCK_CLKIN,		          /*!< CLKIN input */
	ASYNC_SYSCTL_CLOCK_PLLOUT,		        /*!< System PLL output */
	ASYNC_SYSCTL_CLOCK_A_OUTPUT,				  /*!< ASYNC SYSCTRL A OUTPUT to ASYNC SYSCTRL B INPUT */
	ASYNC_SYSCTL_CLOCK_IRC,		    	      /*!< Internal oscillator */
	ASYNC_SYSCTL_CLOCK_WDTOSC,			      /*!< WDT oscillator in */
} CHIP_SYSCTL_ASYNCSYSCONCLKSRC_T;

/**
 * @brief	Set system clock divider
 * @param	div	: divider for system clock
 * @return	Nothing
 * @note	Use 0 to disable, or a divider value of 1 to 255. The system clock
 * rate is the main system clock divided by this value.
 */
STATIC INLINE void Chip_Clock_SetSysClockDiv(uint32_t div)
{
	LPC_SYSCTL->SYSAHBCLKDIV  = div;
}

/**
 * @brief	Set UART divider clock
 * @param	div	: divider for UART clock
 * @return	Nothing
 * @note	Use 0 to disable, or a divider value of 1 to 255. The UART clock
 * rate is the main system clock divided by this value.
 */
STATIC INLINE void Chip_Clock_SetAsyncSysconClockDiv(uint32_t div)
{
	LPC_ASYNC_SYSCTL->ASYNCCLKDIV = div;
}

/**
 * @brief	Return UART divider
 * @return	divider for UART clock
 * @note	A value of 0 means the clock is disabled.
 */
STATIC INLINE uint32_t Chip_Clock_GetAsyncSysconClockDiv(void)
{
	return LPC_ASYNC_SYSCTL->ASYNCCLKDIV;
}

/**
 * @brief	Set The USART Fractional Generator Multiplier
 * @param   mult  :  An 8-bit value (0-255) U_PCLK = UARTCLKDIV/(1 + MULT/256)
 * @return	Nothing
 */
STATIC INLINE void Chip_SYSCTL_SetUSARTFRGCtrl(uint8_t mult, uint8_t div)
{
	LPC_ASYNC_SYSCTL->FRGCTRL = (uint32_t) ((mult<<8)|div);
}

/**
 * @brief	Get The USART Fractional Generator Multiplier
 * @return	Value of USART Fractional Generator Multiplier
 */
STATIC INLINE uint32_t Chip_SYSCTL_GetUSARTFRGCtrl(void)
{
	return LPC_ASYNC_SYSCTL->FRGCTRL;
}

/**
 * @brief	Returns the internal oscillator (IRC) clock rate
 * @return	internal oscillator (IRC) clock rate
 */
STATIC INLINE uint32_t Chip_Clock_GetIntOscRate(void)
{
	return SYSCTL_IRC_FREQ;
}

/**
 * @brief	Returns the external clock input rate
 * @return	External clock input rate
 */
STATIC INLINE uint32_t Chip_Clock_GetExtClockInRate(void)
{
	return ExtClockIn;
}

/**
 * @brief	Enable the RTC 32KHz output
 * @return	Nothing
 * @note	This clock can be used for the main clock directly, but
 *			do not use this clock with the system PLL.
 */
STATIC INLINE void Chip_Clock_EnableRTCOsc(void)
{
	LPC_SYSCTL->RTCOSCCTRL  = 1;
}

/**
 * @brief	Disable the RTC 32KHz output
 * @return	Nothing
 */
STATIC INLINE void Chip_Clock_DisableRTCOsc(void)
{
	LPC_SYSCTL->RTCOSCCTRL  = 0;
}

/**
 * @brief	Set System PLL clock source
 * @param	src	: Clock source for system PLL
 * @return	Nothing
 * @note	This function will also toggle the clock source update register
 * to update the clock source
 */
void Chip_Clock_SetSystemPLLSource(CHIP_SYSCTL_PLLCLKSRC_T src);

/**
 * @brief	Set main system clock source
 * @param	src	: Clock source for main system
 * @return	Nothing
 * @note	This function will also toggle the clock source update register
 * to update the clock source
 */
void Chip_Clock_SetMainClockSource(CHIP_SYSCTL_MAINCLKSRC_T src);

/**
 * @brief	Get main system clock source
 * @return	Clock source for main system
 * @note
 */
CHIP_SYSCTL_MAINCLKSRC_T Chip_Clock_GetMainClockSource( void );

/**
 * @brief	Set AsyncSyscon clock source
 * @param	src	: Clock source for AsyncSyscon system
 * @return	Nothing
 * @note	This function will also toggle the clock source update register
 * to update the clock source
 */
void Chip_Clock_SetAsyncSysconClockSource(CHIP_SYSCTL_ASYNCSYSCONCLKSRC_T src);

/**
 * @brief	Set CLKOUT clock source and divider
 * @param	src	: Clock source for CLKOUT
 * @param	div	: divider for CLKOUT clock
 * @return	Nothing
 * @note	Use 0 to disable, or a divider value of 1 to 255. The CLKOUT clock
 * rate is the clock source divided by the divider. This function will
 * also toggle the clock source update register to update the clock
 * source.
 */
void Chip_Clock_SetCLKOUTSource(CHIP_SYSCTL_CLKOUTSRC_T src, uint32_t div);

/**
 * @brief	Enable a system or peripheral clock
 * @param	clk	: Clock to enable
 * @return	Nothing
 */
void Chip_Clock_EnablePeriphClock(CHIP_SYSCTL_CLOCK_T clk);

/**
 * @brief	Disable a system or peripheral clock
 * @param	clk	: Clock to disable
 * @return	Nothing
 */
void Chip_Clock_DisablePeriphClock(CHIP_SYSCTL_CLOCK_T clk);

/**
 * @brief	Enable async peripheral clock
 * @param	clk	: Clock to enable
 * @return	Nothing
 */
void Chip_Clock_EnableAsyncPeriphClock(CHIP_ASYNC_SYSCTL_CLOCK_T clk);

/**
 * @brief	Disable async peripheral clock
 * @param	clk	: Clock to disable
 * @return	Nothing
 */
void Chip_Clock_DisableAsyncPeriphClock(CHIP_ASYNC_SYSCTL_CLOCK_T clk);

/**
 * @brief	Return System PLL input clock rate
 * @return	System PLL input clock rate
 */
uint32_t Chip_Clock_GetSystemPLLInClockRate(void);

/**
 * @brief	Return System PLL output clock rate
 * @return	System PLL output clock rate
 */
uint32_t Chip_Clock_GetSystemPLLOutClockRate(void);

/**
 * @brief	Return main clock rate
 * @return	main clock rate
 */
uint32_t Chip_Clock_GetMainClockRate(void);

/**
 * @brief	Return system clock rate
 * @return	system clock rate
 */
uint32_t Chip_Clock_GetSystemClockRate(void);

/**
 * @brief	Return Async Syscon clock rate
 * @return	Async syscon clock rate
 */
uint32_t Chip_Clock_GetAsyncSysconClockRate(void);

/**
 * @brief	Returns the RTC clock rate
 * @return	RTC oscillator clock rate in Hz
 */
STATIC INLINE uint32_t Chip_Clock_GetRTCOscRate(void)
{
	return SYSCTL_RTC_FREQ;
}

/**
 * @brief	Return estimated watchdog oscillator rate
 * @return	Estimated watchdog oscillator rate
 * @note	This rate is accurate to plus or minus 40%.
 */
STATIC INLINE uint32_t Chip_Clock_GetWDTOSCRate(void)
{
	return SYSCTL_WDTOSC_FREQ;
}

/**
 * @brief	Set PLL output based on the multiplier and input frequency
 * @param	multiply_by	: multiplier
 * @param	input_freq	: Clock input frequency of the PLL
 * @return	Nothing
 * @note	Note the input clock must be greater than 2MHz.
 */
void Chip_Clock_SetupSystemPLL(uint32_t multiply_by, uint32_t input_freq);

/**
 * @brief	Check if PLL is locked or not
 * @param	None
 * @return	true or false
 */
bool Chip_Clock_IsSystemPLLLocked(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_540XX_H_ */
