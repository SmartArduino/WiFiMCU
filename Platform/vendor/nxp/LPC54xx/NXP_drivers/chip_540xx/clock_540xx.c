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

#include "chip.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 * @brief	Set System PLL clock based on the input frequency and multiplier
 * Note that this is a very cut down version of the PLL150 driver
 * It supports basic IRC integer multiplication
 * The MSEL value is limited to maximum M=16 and N is hardcoded to be a div by 2 
 * Also minimum M needs to be 2
 * This means that effectively means that we support only 5MHz input or above due to:
 * Fcco needing to be at least 75MHz and with N=2 and M=16 we get a multiply factor of 16 (M*2/N)
 * So minimal freq is 75MHz/16 =~ 4.7MHz
 * @param	msel    : PLL feedback divider value
 * @param	psel    : PLL post divider value
 * @return	Nothing
 * @note	See the user manual for how to setup the PLL
 */
void Chip_Clock_SetupSystemPLL(uint32_t multiply_by, uint32_t input_freq)
{
	uint32_t cco_freq = input_freq * multiply_by;
	uint32_t pdec = 1;
	uint32_t selr;
	uint32_t seli;
	uint32_t selp;
	uint32_t mdec, ndec;
	
	uint32_t directo = 1;

	while (cco_freq < 75000000) {
		multiply_by <<= 1; // double value in each iteration
		pdec <<= 1;        // correspondingly double pdec to cancel effect of double msel
		cco_freq = input_freq * multiply_by;
	};
	selr = 0;
	seli = (multiply_by & 0x3c) + 4;
	selp = (multiply_by>>1) + 1;	

	if (pdec > 1) {
		directo = 0; // use post divider
		pdec = pdec/2; // account for minus 1 encoding
		//  Translate P value
		pdec = (pdec == 1)  ? 0x62 :        //1  * 2
           (pdec == 2)  ? 0x42 :        //2  * 2
           (pdec == 4)  ? 0x02 :        //4  * 2
           (pdec == 8)  ? 0x0b :        //8  * 2
           (pdec == 16) ? 0x11 :        //16 * 2
           (pdec == 32) ? 0x08 : 0x08;  //32 * 2
	}                   

	mdec = 0x7fff>>(16 - (multiply_by -1)) ; // we only support values of 2 to 16 (to keep driver simple)
	ndec = 0x202;  // pre divide by 2 (hardcoded)
	
	LPC_SYSCTL->SYSPLLCTRL        =  SYS_PLL_BANDSEL(1) |SYS_PLL_DIRECTI(0) |  SYS_PLL_DIRECTO(directo) | SYS_PLL_INSELR(selr) | SYS_PLL_INSELI(seli) | SYS_PLL_INSELP(selp); // tbd
	LPC_SYSCTL->SYSPLLPDEC        = pdec    | (1<<7); // set Pdec value and assert preq
	LPC_SYSCTL->SYSPLLNDEC        = ndec    | (1<<10); // set Pdec value and assert preq
	LPC_SYSCTL->SYSPLLSSCGCTRL[0] = (1<<18) | (1<<17) | mdec;  // select non sscg MDEC value, assert mreq and select mdec value  
	return;
}

/**
 * @brief	Read System PLL status
 * @return	true if the PLL is locked, false if not locked
 */
bool Chip_Clock_IsSystemPLLLocked(void)
{
	return (bool) ((LPC_SYSCTL->SYSPLLSTAT & 1) != 0);
}


/* Compute a PLL frequency. Based on the PLL setting in MDEC and PDEC, the PLL frequency can be 
reverted. Check Chip_Clock_SetupSystemPLL() to see how PDEC and MDEC value are obtained,
this routine is to revert the multiplier value based on the SYSPLLSSCGCTRL and SYSPLLPDEC
registers. */
STATIC uint32_t Chip_Clock_GetPLLFreq(uint32_t PLLReg0, uint32_t PLLReg1, uint32_t inputRate)
{
	uint32_t mdec, pdec, i, j, multiply_by;

	pdec = PLLReg1 & 0x7F;
	mdec = PLLReg0 & 0x1FFFF;
	
	if (pdec > 1) {
		//  Translate P value
		pdec = (pdec == 0x62) ?  1 :      //1  * 2
           (pdec == 0x42) ?  2 :      //2  * 2
           (pdec == 0x02) ?  4 :      //4  * 2
           (pdec == 0x0b) ?  8 :      //8  * 2
           (pdec == 0x11) ? 16 :      //16 * 2
           (pdec == 0x08) ? 32 : 32;  //32 * 2
		pdec = pdec*2; // account for minus 1 encoding
	}

	i = (0x7fff + 1) / (mdec + 1);

	j = 0;
	while (i > 1) {
		i >>= 1;
		j += 1;
	};

	multiply_by = (16 - j + 1) / pdec;

	return inputRate * multiply_by;
}

/* Return a PLL input (common) */
STATIC uint32_t Chip_Clock_GetPLLInClockRate(uint32_t reg)
{
	uint32_t clkRate;

	switch ((CHIP_SYSCTL_PLLCLKSRC_T) (reg & 0x3)) {
	case SYSCTL_PLLCLKSRC_IRC:
		clkRate = Chip_Clock_GetIntOscRate();
		break;

	case SYSCTL_PLLCLKSRC_CLKIN:
		clkRate = Chip_Clock_GetExtClockInRate();
		break;

	default:
		clkRate = 0;
	}

	return clkRate;
}

/**
 * @brief	Set System PLL clock source
 * @param	src	: Clock source for system PLL
 * @return	Nothing
 */
void Chip_Clock_SetSystemPLLSource(CHIP_SYSCTL_PLLCLKSRC_T src)
{
	LPC_SYSCTL->SYSPLLCLKSEL  = (uint32_t) src;
}


/* Return System PLL input clock rate */
uint32_t Chip_Clock_GetSystemPLLInClockRate(void)
{
	return Chip_Clock_GetPLLInClockRate(LPC_SYSCTL->SYSPLLCLKSEL);
}

/* Return System PLL output clock rate */
uint32_t Chip_Clock_GetSystemPLLOutClockRate(void)
{
	return Chip_Clock_GetPLLFreq(LPC_SYSCTL->SYSPLLSSCGCTRL[0], LPC_SYSCTL->SYSPLLPDEC,
								 Chip_Clock_GetSystemPLLInClockRate());
}

/* Return main A clock rate */
uint32_t Chip_Clock_GetMain_A_ClockRate(void)
{
	uint32_t clkRate = 0;

	switch (Chip_Clock_GetMain_A_ClockSource()) {
	case SYSCTL_MAIN_A_CLKSRC_IRC:
		clkRate = Chip_Clock_GetIntOscRate();
		break;

	case SYSCTL_MAIN_A_CLKSRCA_CLKIN:
		clkRate = Chip_Clock_GetExtClockInRate();
		break;

	case SYSCTL_MAIN_A_CLKSRCA_WDTOSC:
		clkRate = Chip_Clock_GetWDTOSCRate();
		break;

	default:
		clkRate = 0;
		break;
	}

	return clkRate;
}

/* Return main B clock rate */
uint32_t Chip_Clock_GetMain_B_ClockRate(void)
{
	uint32_t clkRate = 0;

	switch (Chip_Clock_GetMain_B_ClockSource()) {
	case SYSCTL_MAIN_B_CLKSRC_MAINCLKSELA:
		clkRate = Chip_Clock_GetMain_A_ClockRate();
		break;

	case SYSCTL_MAIN_B_CLKSRC_SYSPLLIN:
		clkRate = Chip_Clock_GetSystemPLLInClockRate();
		break;

	case SYSCTL_MAIN_B_CLKSRC_SYSPLLOUT:
		clkRate = Chip_Clock_GetSystemPLLOutClockRate();
		break;

	case SYSCTL_MAIN_B_CLKSRC_RTC:
		clkRate = Chip_Clock_GetRTCOscRate();
		break;
	}

	return clkRate;
}

/* Set main system clock source */
void Chip_Clock_SetMainClockSource(CHIP_SYSCTL_MAINCLKSRC_T src)
{
	uint32_t clkSrc = (uint32_t) src;

	if (clkSrc >= SYSCTL_MAINCLKSRCA_OUTPUT) {
		/* Main B source only, not using main A */
		Chip_Clock_SetMain_B_ClockSource((CHIP_SYSCTL_MAIN_B_CLKSRC_T) (clkSrc - SYSCTL_MAINCLKSRCA_OUTPUT));
	}
	else {
		/* Select main A clock source and set main B source to use main A */
		Chip_Clock_SetMain_A_ClockSource((CHIP_SYSCTL_MAIN_A_CLKSRC_T) clkSrc);
		Chip_Clock_SetMain_B_ClockSource(SYSCTL_MAIN_B_CLKSRC_MAINCLKSELA);
	}
}

/* Returns the main clock source */
CHIP_SYSCTL_MAINCLKSRC_T Chip_Clock_GetMainClockSource(void)
{
	CHIP_SYSCTL_MAIN_B_CLKSRC_T srcB;
	uint32_t clkSrc;

	/* Get main B clock source */
	srcB = Chip_Clock_GetMain_B_ClockSource();
	if (srcB == SYSCTL_MAIN_B_CLKSRC_MAINCLKSELA) {
		/* Using source A, so return source A */
		clkSrc = (uint32_t) Chip_Clock_GetMain_A_ClockSource();
	}
	else {
		/* Using source B */
		clkSrc = SYSCTL_MAINCLKSRCA_OUTPUT + (uint32_t) srcB;
	}

	return (CHIP_SYSCTL_MAINCLKSRC_T) clkSrc;
}

/* Return main clock rate */
uint32_t Chip_Clock_GetMainClockRate(void)
{
	uint32_t clkRate;

	if (Chip_Clock_GetMain_B_ClockSource() == SYSCTL_MAIN_B_CLKSRC_MAINCLKSELA) {
		/* Return main A clock rate */
		clkRate = Chip_Clock_GetMain_A_ClockRate();
	}
	else {
		/* Return main B clock rate */
		clkRate = Chip_Clock_GetMain_B_ClockRate();
	}

	return clkRate;
}

/* Return Async Syscon clock rate */
uint32_t Chip_Clock_GetAsyncSyscon_A_ClockRate(void)
{
	uint32_t clkRate = 0;

	switch (Chip_Clock_GetAsyncSyscon_A_ClockSource()) {
	case ASYNC_SYSCTL_IRC:
		clkRate = Chip_Clock_GetIntOscRate();
		break;
	case ASYNC_SYSCTL_WDTOSC:
		clkRate = Chip_Clock_GetWDTOSCRate();
		break;
	default:
		clkRate = 0;
		break;
	}
	return clkRate;
}

/* Return main B clock rate */
uint32_t Chip_Clock_GetAsyncSyscon_B_ClockRate(void)
{
	uint32_t clkRate = 0;

	switch (Chip_Clock_GetAsyncSyscon_B_ClockSource()) {
	case ASYNC_SYSCTL_MAINCLK:
		clkRate = Chip_Clock_GetMainClockRate();
		break;

	case ASYNC_SYSCTL_CLKIN:
		clkRate = Chip_Clock_GetSystemPLLInClockRate();
		break;

	case ASYNC_SYSCTL_PLLOUT:
		clkRate = Chip_Clock_GetSystemPLLOutClockRate();
		break;

	case ASYNC_SYSCTL_A_OUTPUT:
		clkRate = Chip_Clock_GetAsyncSyscon_A_ClockRate();
		break;
	}

	return clkRate;
}

/* Set Async Syscon clock source */
void Chip_Clock_SetAsyncSysconClockSource(CHIP_SYSCTL_ASYNCSYSCONCLKSRC_T src)
{
	uint32_t clkSrc = (uint32_t) src;

	if (clkSrc <= ASYNC_SYSCTL_CLOCK_A_OUTPUT) {
		/* AsyncSyscon B source only, not using AsyncSyscon A */
		Chip_Clock_SetAsyncSyscon_B_ClockSource((CHIP_ASYNC_SYSCTL_B_SRC_T)clkSrc);
	}
	else {
		/* Select main A clock source and set main B source to use main A */
		Chip_Clock_SetAsyncSyscon_A_ClockSource((CHIP_ASYNC_SYSCTL_A_SRC_T) (clkSrc - ASYNC_SYSCTL_CLOCK_A_OUTPUT - 1));
		Chip_Clock_SetAsyncSyscon_B_ClockSource(ASYNC_SYSCTL_A_OUTPUT);
	}
}

/* Returns the Async Syscon clock source */
CHIP_SYSCTL_ASYNCSYSCONCLKSRC_T Chip_Clock_GetAsyncSysconClockSource(void)
{
	uint32_t clkSrc;

	/* Get AsyncSyscon clock source */
	if ((CHIP_SYSCTL_ASYNCSYSCONCLKSRC_T) Chip_Clock_GetAsyncSyscon_B_ClockSource() >= ASYNC_SYSCTL_CLOCK_A_OUTPUT) {
		/* Using source A, so return source A */
		clkSrc = (uint32_t) Chip_Clock_GetAsyncSyscon_A_ClockSource();
	}
	else {
		/* Using source B */
		clkSrc = (uint32_t) Chip_Clock_GetAsyncSyscon_B_ClockSource();
	}
	return (CHIP_SYSCTL_ASYNCSYSCONCLKSRC_T) clkSrc;
}

/* Return AsyncSyscon clock rate */
uint32_t Chip_Clock_GetAsyncSysconClockRate(void)
{
	uint32_t clkRate;

	if ((CHIP_SYSCTL_ASYNCSYSCONCLKSRC_T) Chip_Clock_GetAsyncSyscon_B_ClockSource() >= ASYNC_SYSCTL_CLOCK_A_OUTPUT) {
		/* Return main A clock rate */
		clkRate = Chip_Clock_GetAsyncSyscon_A_ClockRate() / Chip_Clock_GetAsyncSysconClockDiv();
	}
	else {
		/* Return main B clock rate */
		clkRate = Chip_Clock_GetAsyncSyscon_B_ClockRate() / Chip_Clock_GetAsyncSysconClockDiv();
	}

	return clkRate;
}

/* Return ADC asynchronous clock rate */
uint32_t Chip_Clock_GetADCASYNCRate(void)
{
	uint32_t clkRate = 0;

	switch (Chip_Clock_GetADCASYNCSource()) {
	case SYSCTL_ADCASYNCCLKSRC_IRC:
		clkRate = Chip_Clock_GetIntOscRate();
		break;
	case SYSCTL_ADCASYNCCLKSRC_SYSPLLOUT:
		clkRate = Chip_Clock_GetSystemPLLOutClockRate();
		break;
	case SYSCTL_ADCASYNCCLKSRC_MAINCLK:
		clkRate = Chip_Clock_GetMainClockRate();
		break;
	}

	return clkRate/Chip_Clock_GetADCASYNCDivider();
}

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
void Chip_Clock_SetCLKOUTSource(CHIP_SYSCTL_CLKOUTSRC_T src, uint32_t div)
{
	uint32_t srcClk = (uint32_t) src;

	/* Use a clock A source? */
	if (src >= SYSCTL_CLKOUTSRCA_OUTPUT) {
		/* Not using a CLKOUT A source */
		LPC_SYSCTL->CLKOUTCLKSELB = srcClk - SYSCTL_CLKOUTSRCA_OUTPUT;
	}
	else {
		/* Using a clock A source, select A and then switch B to A */
		LPC_SYSCTL->CLKOUTCLKSELA = srcClk;
		LPC_SYSCTL->CLKOUTCLKSELB = 0;
	}
	LPC_SYSCTL->CLKOUTDIV = div;
}

/* Enable a system or peripheral clock */
void Chip_Clock_EnablePeriphClock(CHIP_SYSCTL_CLOCK_T clk)
{
	uint32_t clkEnab = (uint32_t) clk;

	if (clkEnab >= 32) {
		LPC_SYSCTL->SYSAHBCLKCTRLSET[1] = (1 << (clkEnab - 32));
	}
	else {
		LPC_SYSCTL->SYSAHBCLKCTRLSET[0] = (1 << clkEnab);
	}
}

/* Disable a system or peripheral clock */
void Chip_Clock_DisablePeriphClock(CHIP_SYSCTL_CLOCK_T clk)
{
	uint32_t clkEnab = (uint32_t) clk;

	if (clkEnab >= 32) {
		LPC_SYSCTL->SYSAHBCLKCTRLCLR[1] = (1 << (clkEnab - 32));
	}
	else {
		LPC_SYSCTL->SYSAHBCLKCTRLCLR[0] = (1 << clkEnab);
	}
}

/* Enable async peripheral clock */
void Chip_Clock_EnableAsyncPeriphClock(CHIP_ASYNC_SYSCTL_CLOCK_T clk)
{
	LPC_ASYNC_SYSCTL->ASYNCVPBCLKCTRLSET = (1 << clk);
}

/* Disable async peripheral clock */
void Chip_Clock_DisableAsyncPeriphClock(CHIP_ASYNC_SYSCTL_CLOCK_T clk)
{
	LPC_ASYNC_SYSCTL->ASYNCVPBCLKCTRLCLR = (1 << clk);
}

/* Returns the system tick rate as used with the system tick divider */
uint32_t Chip_Clock_GetSysTickClockRate(void)
{
	uint32_t sysRate, div;

	div = LPC_SYSCTL->SYSTICKCLKDIV;

	/* If divider is 0, the system tick clock is disabled */
	if (div == 0) {
		sysRate = 0;
	}
	else {
		sysRate = Chip_Clock_GetMainClockRate() / LPC_SYSCTL->SYSTICKCLKDIV;
	}

	return sysRate;
}

/* Measure frequency of the main system clock */
uint32_t Chip_Clock_MeasureSysFreq(void)
{
	// FIXME - not yet operational without input mux and driver tie-tin
	return 0;
}

/* Get UART base rate */
uint32_t Chip_Clock_GetUARTBaseClockRate(void)
{
	uint64_t inclk;
	uint32_t div;

	div = (uint32_t) Chip_Clock_GetAsyncSysconClockDiv();
	if (div == 0) {
		/* Divider is 0 so UART clock is disabled */
		inclk = 0;
	}
	else {
		uint32_t mult, divmult;

		/* Input clock into FRG block is the divided main system clock */
		inclk = (uint64_t) (Chip_Clock_GetMainClockRate() / div);

		divmult = LPC_ASYNC_SYSCTL->FRGCTRL & 0xFF;
		if ((divmult & 0xFF) == 0xFF) {
			/* Fractional part is enabled, get multiplier */
			mult = (divmult >> 8) & 0xFF;

			/* Get fractional error */
			inclk = (inclk * 256) / (uint64_t) (256 + mult);
		}
		else {
			/* Integer divide from fractional divider */
			inclk = inclk / (uint64_t) div;
		}
	}

	return (uint32_t) inclk;
}

/* Set UART base rate */
uint32_t Chip_Clock_SetUARTBaseClockRate(uint32_t rate, bool fEnable)
{
	uint32_t pre_div, div, inclk;

	/* Input clock into FRG block is the main system cloock */
	inclk = Chip_Clock_GetAsyncSysconClockRate();
	pre_div = Chip_Clock_GetAsyncSysconClockDiv();

	/* Get integer divider for coarse rate */
	div = inclk / rate;
	if (div == 0) {
		div = 1;
	}

	/* Approximated rate with only integer divider */
	Chip_Clock_SetAsyncSysconClockDiv(div);

	if (fEnable) {
		uint32_t err;
		uint64_t uart_fra_multiplier;

		/* Enable FRG clock */
		Chip_Clock_EnableAsyncPeriphClock(ASYNC_SYSCTL_CLOCK_FRG);

		err = inclk - (rate * div);
		uart_fra_multiplier = (((uint64_t) err + (uint64_t) rate) * 256) / (uint64_t) (rate * div);

		/* Enable fractional divider and set multiplier */
		LPC_ASYNC_SYSCTL->FRGCTRL = 0xFF | (uart_fra_multiplier << 8);
	}
	else {
		/* Restore to previous AsyncSyscon clock divider value. */
		Chip_Clock_SetAsyncSysconClockDiv(pre_div);
	}

	return Chip_Clock_GetUARTBaseClockRate();
}

/* Return system clock rate */
uint32_t Chip_Clock_GetSystemClockRate(void)
{
	/* No point in checking for divide by 0 */
	return Chip_Clock_GetMainClockRate() / LPC_SYSCTL->SYSAHBCLKDIV;
}

