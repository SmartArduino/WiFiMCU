/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "fsl_uart_hal.h"

/*******************************************************************************
 * Code
 ******************************************************************************/
/*******************************************************************************
 * UART Common Configurations
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_Init
 * Description   : This function initializes the module to a known state.
 *
 *END**************************************************************************/
void UART_HAL_Init(uint32_t baseAddr)
{
    HW_UART_BDH_WR(baseAddr, 0U);
    HW_UART_BDL_WR(baseAddr, 4U);
    HW_UART_C1_WR(baseAddr, 0U);
    HW_UART_C2_WR(baseAddr, 0U);
    HW_UART_S2_WR(baseAddr, 0U);
    HW_UART_C3_WR(baseAddr, 0U);
#if FSL_FEATURE_UART_HAS_ADDRESS_MATCHING
    HW_UART_MA1_WR(baseAddr, 0U);
    HW_UART_MA2_WR(baseAddr, 0U);
#endif
    HW_UART_C4_WR(baseAddr, 0U);
#if FSL_FEATURE_UART_HAS_DMA_ENABLE
    HW_UART_C5_WR(baseAddr, 0U);
#endif
#if FSL_FEATURE_UART_HAS_MODEM_SUPPORT
    HW_UART_MODEM_WR(baseAddr, 0U);
#endif
#if FSL_FEATURE_UART_HAS_IR_SUPPORT
    HW_UART_IR_WR(baseAddr, 0U);
#endif
#if FSL_FEATURE_UART_HAS_FIFO
    HW_UART_PFIFO_WR(baseAddr, 0U);
    HW_UART_CFIFO_WR(baseAddr, 0U);
    HW_UART_SFIFO_WR(baseAddr, 0xC0U);
    HW_UART_TWFIFO_WR(baseAddr, 0U);
    HW_UART_RWFIFO_WR(baseAddr, 1U);
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetBaudRate
 * Description   : Configure the UART baud rate.
 * This function programs the UART baud rate to the desired value passed in by
 * the user. The user must also pass in the module source clock so that the
 * function can calculate the baud rate divisors to their appropriate values.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_SetBaudRate(uint32_t baseAddr, uint32_t sourceClockInHz,
                                   uint32_t baudRate)
{
    /* BaudRate = (SourceClkInHz)/[16 * (SBR +  BRFA)] */
    uint16_t sbr;

    /* calculate the baud rate modulo divisor, sbr*/
    sbr = sourceClockInHz / (baudRate * 16);

    /* check to see if sbr is out of range of register bits */
    if ( (sbr > 0x1FFF) || (sbr < 1) )
    {
        /* unsupported baud rate for given source clock input*/
        return kStatus_UART_BaudRateCalculationError;
    }

    /* write the sbr value to the BDH and BDL registers*/
    BW_UART_BDH_SBR(baseAddr, (uint8_t)(sbr >> 8));
    BW_UART_BDL_SBR(baseAddr, (uint8_t)sbr);

#if FSL_FEATURE_UART_HAS_BAUD_RATE_FINE_ADJUST_SUPPORT
    /* determine if a fractional divider is needed to fine tune closer to the
     * desired baud, each value of brfa is in 1/32 increments,
     * hence the multiply-by-32. */
    uint16_t brfa = (32*sourceClockInHz/(baudRate*16)) - 32*sbr;

    /* write the brfa value to the register*/
    BW_UART_C4_BRFA(baseAddr, brfa);
#endif

    return kStatus_UART_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetBaudRateDivisor
 * Description   : Set the UART baud rate modulo divisor value.
 * This function allows the user to program the baud rate divisor directly in
 * situations where the divisor value is known. In this case, the user may not
 * want to call the UART_HAL_SetBaudRate() function as the divisor is already
 * known to them.
 *
 *END**************************************************************************/
void UART_HAL_SetBaudRateDivisor(uint32_t baseAddr, uint16_t baudRateDivisor)
{
    /* check to see if baudRateDivisor is out of range of register bits */
    assert( (baudRateDivisor < 0x1FFF) && (baudRateDivisor > 1) );

    /* program the sbr (baudRateDivisor) value to the BDH and BDL registers*/
    BW_UART_BDH_SBR(baseAddr, (uint8_t)(baudRateDivisor >> 8));
    BW_UART_BDL_SBR(baseAddr, (uint8_t)baudRateDivisor);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetParityMode
 * Description   : Configures the parity mode in the UART controller.
 * This function allows the user to configure the parity mode of the UART
 * controller to disable it or enable it for even parity or for odd parity.
 *
 *END**************************************************************************/
void UART_HAL_SetParityMode(uint32_t baseAddr, uart_parity_mode_t parityMode)
{
    BW_UART_C1_PE(baseAddr, parityMode >> 1U);
    BW_UART_C1_PT(baseAddr, parityMode && 1U);
}

/*******************************************************************************
 * UART Transfer Functions
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_Putchar
 * Description   : This function allows the user to send an 8-bit character
 * from the UART data register.
 *
 *END**************************************************************************/
void UART_HAL_Putchar(uint32_t baseAddr, uint8_t data)
{
    /* In addition to sending a char, this function also clears the transmit 
     * status flagsfor this uart baseAddr, there is a two step process to clear
     * the transmit status flags:
     * 1. Read the status register with the status bit set
     * 2. write to the data register */
    HW_UART_S1_RD(baseAddr);
    HW_UART_D_WR(baseAddr, data);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_Putchar9
 * Description   : This function allows the user to send a 9-bit character from
 * the UART data register.
 *
 *END**************************************************************************/
void UART_HAL_Putchar9(uint32_t baseAddr, uint16_t data)
{
    uint8_t ninthDataBit = (data >> 8U) & 0x1U;

    /* Write to the ninth data bit (bit position T8, where T[0:7]=8-bits,
     * T8=9th bit)*/
    BW_UART_C3_T8(baseAddr, ninthDataBit);

    /* in addition to sending a char, this function also clears the transmit 
     * status flags for this uart baseAddr, there is a two step process to
     * clear the transmit status flags:
     * 1. Read the status register with the status bit set
     * 2. write to the data register */
    HW_UART_S1_RD(baseAddr);
    HW_UART_D_WR(baseAddr, (uint8_t)data);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_Getchar
 * Description   : This function gets a received 8-bit character from the UART
 * data register.
 *
 *END**************************************************************************/
void UART_HAL_Getchar(uint32_t baseAddr, uint8_t *readData)
{
    /* in addition to getting a char, this function also clears the receive
     * status flag RDRF along with IDLE, OR, NF, FE, and PF (these can also be
     * cleared in separate functions)
     * for this uart baseAddr, there is a two step process to clear the receive
     * status flag:
     * 1. Read the status register with the status bit set
     * 2. read from the data register */
    HW_UART_S1_RD(baseAddr);
    *readData = HW_UART_D_RD(baseAddr);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_Getchar9
 * Description   : This function gets a received 9-bit character from the UART 
 * data register.
 *
 *END**************************************************************************/
void  UART_HAL_Getchar9(uint32_t baseAddr, uint16_t *readData)
{
    /* read ninth data bit and left shift to bit position R8 before reading
     * the 8 other data bits R[7:0]*/
    *readData = (uint16_t)BR_UART_C3_R8(baseAddr) << 8;

    /* in addition to getting a char, this function also clears the receive
     * status flag RDRF along with IDLE, OR, NF, FE, and PF (these can also
     * be cleared in separate functions)
     * for this uart baseAddr, there is a two step process to clear the receive
     * status flag:
     * 1. Read the status register with the status bit set
     * 2. read from the data register */
    HW_UART_S1_RD(baseAddr);
    *readData |= HW_UART_D_RD(baseAddr);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SendDataPolling
 * Description   : Send out multiple bytes of data using polling method.
 * This function only supports 8-bit transaction.
 *
 *END**************************************************************************/
void UART_HAL_SendDataPolling(uint32_t baseAddr,
                              const uint8_t *txBuff,
                              uint32_t txSize)
{
    while (txSize--)
    {
        while (!UART_HAL_IsTxDataRegEmpty(baseAddr))
        {}

        UART_HAL_Putchar(baseAddr, *txBuff++);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_ReceiveDataPolling
 * Description   : Receive multiple bytes of data using polling method.
 * This function only supports 8-bit transaction.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_ReceiveDataPolling(uint32_t baseAddr,
                                          uint8_t *rxBuff, 
                                          uint32_t rxSize)
{
    uart_status_t retVal = kStatus_UART_Success;

    while (rxSize--)
    {
        while (!UART_HAL_IsRxDataRegFull(baseAddr))
        {}

        UART_HAL_Getchar(baseAddr, rxBuff++);
        
        /* Clear the Overrun flag since it will block receiving */
        if (BR_UART_S1_OR(baseAddr))
        {
            HW_UART_S1_RD(baseAddr);
            HW_UART_D_RD(baseAddr);
            retVal = kStatus_UART_RxOverRun;
        }
    }

    return retVal;
}

/*******************************************************************************
 * UART Interrupts and DMA
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_ConfigureInterrupts
 * Description   : Configure the UART module interrupts to enable/disable various
 * interrupt sources.
 *
 *END**************************************************************************/
void UART_HAL_SetIntMode(uint32_t baseAddr, uart_interrupt_t interrupt, bool enable)
{
    uint8_t reg = (uint32_t)interrupt >> UART_SHIFT;
    uint32_t temp = 1U << (uint8_t)interrupt;

    switch ( reg )
    {
        case 0 :
            enable ? HW_UART_BDH_SET(baseAddr, temp) : HW_UART_BDH_CLR(baseAddr, temp);
            break;
        case 1 :
            enable ? HW_UART_C2_SET(baseAddr, temp) : HW_UART_C2_CLR(baseAddr, temp);
            break;
        case 2 :
            enable ? HW_UART_C3_SET(baseAddr, temp) : HW_UART_C3_CLR(baseAddr, temp);
            break;
#if FSL_FEATURE_UART_HAS_FIFO
        case 3 :
            enable ? HW_UART_CFIFO_SET(baseAddr, temp) : HW_UART_CFIFO_CLR(baseAddr, temp);
            break;
#endif
        default :
            break;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_GetIntMode
 * Description   : Return whether the UART module interrupts is enabled/disabled.
 *
 *END**************************************************************************/
bool UART_HAL_GetIntMode(uint32_t baseAddr, uart_interrupt_t interrupt)
{
    uint8_t reg = (uint32_t)interrupt >> UART_SHIFT;
    uint8_t temp = 0;

    switch ( reg )
    {
        case 0 :
            temp = HW_UART_BDH_RD(baseAddr) >> (uint8_t)(interrupt) & 1U;
            break;
        case 1 :
            temp = HW_UART_C2_RD(baseAddr) >> (uint8_t)(interrupt) & 1U;
            break;
        case 2 :
            temp = HW_UART_C3_RD(baseAddr) >> (uint8_t)(interrupt) & 1U;
            break;
#if FSL_FEATURE_UART_HAS_FIFO
        case 3 :
            temp = HW_UART_CFIFO_RD(baseAddr) >> (uint8_t)(interrupt) & 1U;
            break;
#endif
        default :
            break;
    }
    return (bool)temp;
}

#if FSL_FEATURE_UART_HAS_DMA_SELECT 
/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetTxDmaCmd
 * Description   : Enable or disable UART DMA request for Transmitter.
 * This function allows the user to configure the receive data register full 
 * flag to generate a DMA request.
 *
 *END**************************************************************************/
void UART_HAL_SetTxDmaCmd(uint32_t baseAddr, bool enable)
{
    /* TDMAS configures the transmit data register empty flag, TDRE, to
     * generate interrupt or DMA requests if TIE is set.
     * NOTE: If UART_C2[TIE] is cleared, TDRE DMA and TDRE interrupt request 
     * signals are not asserted when the TDRE flag is set, regardless of the
     * state of TDMAS. If UART_C2[TIE] and TDMAS are both set, then UART_C2[TCIE]
     * must be cleared, and UART_D must not be written outside of servicing of
     * a DMA request.
     * 0 If TIE is set and the TDRE flag is set, the TDRE interrupt request 
     * signal is asserted to request interrupt service.
     * 1 If TIE is set and the TDRE flag is set, the TDRE DMA request signal 
     * is asserted to request a DMA transfer. */
    if (enable)
    {
        BW_UART_C2_TCIE(baseAddr, 0U);
        BW_UART_C2_TIE(baseAddr, 1U);
#if FSL_FEATURE_UART_IS_SCI 
        BW_UART_C4_TDMAS(baseAddr, 1U);
#else
        BW_UART_C5_TDMAS(baseAddr, 1U);
#endif
    }
    else
    {
        BW_UART_C2_TIE(baseAddr, 0U);
#if FSL_FEATURE_UART_IS_SCI 
        BW_UART_C4_TDMAS(baseAddr, 0U);
#else
        BW_UART_C5_TDMAS(baseAddr, 0U);
#endif
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetRxDmaCmd
 * Description   : Enable or disable UART DMA request for Receiver.
 * This function allows the user to configure the receive data register full 
 * flag to generate a DMA request.
 *
 *END**************************************************************************/
void UART_HAL_SetRxDmaCmd(uint32_t baseAddr, bool enable)
{
    /* RDMAS configures the receiver data register full flag, RDRF, to generate
     * interrupt or DMA requests if RIEis set.
     * NOTE: If RIE is cleared, the RDRF DMA and RDRF interrupt request signals
     * are not asserted when the RDRF flag is set, regardless of the state of 
     * RDMAS. 
     * 0 If RIE is set and the RDRF flag is set, the RDRF interrupt request
     * signal is asserted to request interrupt service.
     * 1 If RIE is set and the RDRF flag is set, the RDRF DMA request signal 
     * is asserted to request a DMA transfer. */
    BW_UART_C2_RIE(baseAddr, enable);
#if FSL_FEATURE_UART_IS_SCI 
    BW_UART_C4_RDMAS(baseAddr, enable);
#else
    BW_UART_C5_RDMAS(baseAddr, enable);
#endif
}
#endif /* FSL_FEATURE_UART_HAS_DMA_SELECT */

/*******************************************************************************
 * UART UART Status Flags
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_GetStatusFlag
 * Description   : Get UART status flag states.
 *
 *END**************************************************************************/
bool UART_HAL_GetStatusFlag(uint32_t baseAddr, uart_status_flag_t statusFlag)
{
    uint8_t reg = (uint32_t)statusFlag >> UART_SHIFT;
    uint8_t temp = 0;

    switch ( reg )
    {
        case 0 :
            temp = HW_UART_S1_RD(baseAddr) >> (uint8_t)(statusFlag) & 1U;
            break;
        case 1 :
            temp = HW_UART_S2_RD(baseAddr) >> (uint8_t)(statusFlag) & 1U;
            break;
#if FSL_FEATURE_UART_HAS_EXTENDED_DATA_REGISTER_FLAGS
        case 2 :
            temp = HW_UART_ED_RD(baseAddr) >> (uint8_t)(statusFlag) & 1U;
            break;
#endif
#if FSL_FEATURE_UART_HAS_FIFO
        case 3 :
            temp = HW_UART_SFIFO_RD(baseAddr) >> (uint8_t)(statusFlag) & 1U;
            break;
#endif
        default :
            break;
    }
    return (bool)temp;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_ClearStatusFlag
 * Description   : Clear an individual and specific UART status flag.
 * This function allows the user to clear an individual and specific UART 
 * status flag. Refer to structure definition uart_status_flag_t for list of
 * status bits.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_ClearStatusFlag(uint32_t baseAddr, uart_status_flag_t statusFlag)
{
    uart_status_t retVal = kStatus_UART_Success;

    switch(statusFlag)
    {
        /* These flags are cleared automatically by other uart operations and
         * cannot be manually cleared, return error code. */
        case kUartTxDataRegEmpty:
        case kUartTxComplete:
        case kUartRxDataRegFull:
        case kUartRxActive:
#if FSL_FEATURE_UART_HAS_EXTENDED_DATA_REGISTER_FLAGS
        case kUartNoiseInCurrentWord:
        case kUartParityErrInCurrentWord:
#endif
#if FSL_FEATURE_UART_HAS_FIFO
        case kUartTxBuffEmpty:
        case kUartRxBuffEmpty:
#endif
            retVal = kStatus_UART_ClearStatusFlagError;
            break;

        case kUartIdleLineDetect:
        case kUartRxOverrun:
        case kUartNoiseDetect:
        case kUartFrameErr:
        case kUartParityErr:
            HW_UART_S1_RD(baseAddr);
            HW_UART_D_RD(baseAddr);
            break;

#if FSL_FEATURE_UART_HAS_LIN_BREAK_DETECT
        case kUartLineBreakDetect:
            HW_UART_S2_WR(baseAddr, BM_UART_S2_LBKDIF);
            break;
#endif

        case kUartRxActiveEdgeDetect:
            HW_UART_S2_WR(baseAddr, BM_UART_S2_RXEDGIF);
            break;

#if FSL_FEATURE_UART_HAS_FIFO
        case kUartTxBuffOverflow:
            HW_UART_SFIFO_WR(baseAddr, BM_UART_SFIFO_TXOF);
            break;

        case kUartRxBuffUnderflow:
            HW_UART_SFIFO_WR(baseAddr, BM_UART_SFIFO_RXUF);
            break;
#endif
        default:
            break;
    }

    return retVal;
}

/*******************************************************************************
 * UART FIFO Configurations
 ******************************************************************************/
#if FSL_FEATURE_UART_HAS_FIFO
/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetTxFifo
 * Description   : Enable or disable the UART transmit FIFO.
 * This function allows the user to enable or disable the UART transmit FIFO.
 * It is required that the transmitter/receiver should be disabled before
 * calling this function and when the FIFO is empty. Additionally, TXFLUSH and
 * RXFLUSH commands should be issued after calling this function.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_SetTxFifoCmd(uint32_t baseAddr, bool enable)
{
    /* before enabling the tx fifo, UARTx_C2[TE] (transmitter) and
     * UARTx_C2[RE] (receiver) must be disabled.
     * if not, return an error code */
    uint8_t txEnable = BR_UART_C2_TE(baseAddr);
    uint8_t rxEnable = BR_UART_C2_RE(baseAddr);

    if (txEnable || rxEnable)
    {
        return kStatus_UART_TxOrRxNotDisabled;
    }
    else
    {
        BW_UART_PFIFO_TXFE(baseAddr, enable);
        return kStatus_UART_Success;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetRxFifoCmd
 * Description   : Enable or disable the UART receive FIFO.
 * This function allows the user to enable or disable the UART receive FIFO.
 * It is required that the transmitter/receiver should be disabled before calling
 * this function and when the FIFO is empty. Additionally, TXFLUSH and RXFLUSH
 * commands should be issued after calling this function.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_SetRxFifoCmd(uint32_t baseAddr, bool enable)
{
    /* before enabling the rx fifo, UARTx_C2[TE] (transmitter) and
     * UARTx_C2[RE] (receiver) must be disabled
     * if not, return an error code */
    uint8_t txEnable = BR_UART_C2_TE(baseAddr);
    uint8_t rxEnable = BR_UART_C2_RE(baseAddr);

    if (txEnable || rxEnable)
    {
        return kStatus_UART_TxOrRxNotDisabled;
    }
    else
    {
        BW_UART_PFIFO_RXFE(baseAddr, enable);
        return kStatus_UART_Success;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_FlushTxFifo
 * Description   : Flush the UART transmit FIFO.
 * This function allows you to flush the UART transmit FIFO for a particular 
 * module baseAddr. Flushing the FIFO may result in data loss. It is recommended
 * that the transmitter should be disabled before calling this function.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_FlushTxFifo(uint32_t baseAddr)
{
    /* in order to flush the tx fifo, UARTx_C2[TE] (transmitter) must be
     * disabled. If not, return an error code */
    if (BR_UART_C2_TE(baseAddr) != 0)
    {
        return kStatus_UART_TxNotDisabled;
    }
    else
    {
        /* Set the bit to flush fifo*/
        BW_UART_CFIFO_TXFLUSH(baseAddr, 1U);
        return kStatus_UART_Success;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_FlushRxFifo
 * Description   : Flush the UART receive FIFO.
 * This function allows you to flush the UART receive FIFO for a particular
 * module baseAddr. Flushing the FIFO may result in data loss. It is recommended
 * that the receiver should be disabled before calling this function.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_FlushRxFifo(uint32_t baseAddr)
{
    /* in order to flush the rx fifo, UARTx_C2[RE] (receiver) must be disabled
     * if not, return an error code. */
    if (BR_UART_C2_RE(baseAddr) != 0)
    {
        return kStatus_UART_RxNotDisabled;
    }
    else
    {
        /* Set the bit to flush fifo*/
        BW_UART_CFIFO_RXFLUSH(baseAddr, 1U);
        return kStatus_UART_Success;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetTxFifoWatermark
 * Description   : Set the UART transmit FIFO watermark value.
 * Programming the transmit watermark should be done when UART the transmitter is
 * disabled and the value must be set less than the size obtained from
 * UART_HAL_GetTxFifoSize.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_SetTxFifoWatermark(uint32_t baseAddr, uint8_t watermark)
{
    /* in order to set the tx watermark, UARTx_C2[TE] (transmitter) must be
     * disabled. If not, return an error code */
    if (BR_UART_C2_TE(baseAddr) != 0)
    {
        return kStatus_UART_TxNotDisabled;
    }
    else
    {
        /* Programming the transmit watermark should be done when the
         * transmitter is disabled and the value must be set less than 
         * the size given in PFIFO[TXFIFOSIZE] */
        HW_UART_TWFIFO_WR(baseAddr, watermark);
        return kStatus_UART_Success;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetRxFifoWatermark
 * Description   : Set the UART receive FIFO watermark value.
 * Programming the receive watermark should be done when the receiver is disabled
 * and the value must be set less than the size obtained from UART_HAL_GetRxFifoSize
 * and greater than zero.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_SetRxFifoWatermark(uint32_t baseAddr, uint8_t watermark)
{
    /* in order to set the rx watermark, UARTx_C2[RE] (receiver) must be disabled
     * if not, return an error code. */
    if (BR_UART_C2_RE(baseAddr) != 0)
    {
        return kStatus_UART_RxNotDisabled;
    }
    else
    {
        /* Programming the receive watermark should be done when the receiver is
         * disabled and the value must be set less than the size given in
         * PFIFO[RXFIFOSIZE] and greater than zero.  */
        HW_UART_RWFIFO_WR(baseAddr, watermark);
        return kStatus_UART_Success;
    }
}
#endif  /* FSL_FEATURE_UART_HAS_FIFO*/

/*******************************************************************************
 * UART Special Feature Configurations
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_PutReceiverInStandbyMode
 * Description   : Place the UART receiver in standby mode.
 * This function, when called, will place the UART receiver into standby mode.
 * In some UART baseAddrs, there is a condition that must be met before placing
 * rx in standby mode. Before placing UART in standby, you need to first
 * determine if receiver is set to wake on idle and if receiver is already in
 * idle state. Per ref manual:
 * NOTE: RWU should only be set with C1[WAKE] = 0 (wakeup on idle) if the
 * channel is currently not idle.
 * This can be determined by the S2[RAF] flag. If set to wake up FROM an IDLE
 * event and the channel is already idle, it is possible that the UART will
 * discard data since data must be received (or a LIN break detect) after an
 * IDLE is detected before IDLE is allowed to reasserted.
 *
 *END**************************************************************************/
uart_status_t UART_HAL_PutReceiverInStandbyMode(uint32_t baseAddr)
{
    uart_wakeup_method_t rxWakeMethod;
    bool uart_current_rx_state;

    /* see if wake is set for idle or */
    rxWakeMethod = UART_HAL_GetReceiverWakeupMethod(baseAddr);
    uart_current_rx_state = UART_HAL_GetStatusFlag(baseAddr, kUartRxActive);

    /* if both rxWakeMethod is set for idle and current rx state is idle,
     * don't put in standy*/
    if ((rxWakeMethod == kUartIdleLineWake) && (uart_current_rx_state == 0))
    {
        return kStatus_UART_RxStandbyModeError;
    }
    else
    {
        /* set the RWU bit to place receiver into standby mode*/
        HW_UART_C2_SET(baseAddr, BM_UART_C2_RWU);
        return kStatus_UART_Success;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_ConfigIdleLineDetect
 * Description   : Configure the operation options of the UART idle line detect.
 * This function allows the user to configure the UART idle-line detect 
 * operation. There are two separate operations for the user to configure,
 * the idle line bit-count start and the receive wake up affect on IDLE status
 * bit. The user will pass in a stucture of type uart_idle_line_config_t.
 *
 *END**************************************************************************/
void UART_HAL_ConfigIdleLineDetect(uint32_t baseAddr, uint8_t idleLine,
                                   uint8_t rxWakeIdleDetect)
{
    BW_UART_C1_ILT(baseAddr, idleLine);
    BW_UART_S2_RWUID(baseAddr, rxWakeIdleDetect);
}

#if FSL_FEATURE_UART_HAS_ADDRESS_MATCHING
/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetMatchAddress
 * Description   : Configure the UART match address mode control operation.
 * (Note: Feature available on select UART baseAddrs)
 * The function allows the user to configure the UART match address control
 * operation. The user has the option to enable the match address mode and to
 * program the match address value. There are two match address modes, each with
 * it's own enable and programmable match address value.
 *
 *END**************************************************************************/
void UART_HAL_SetMatchAddress(uint32_t baseAddr,
                              bool matchAddrMode1,
                              bool matchAddrMode2,
                              uint8_t matchAddrValue1,
                              uint8_t matchAddrValue2)
{ 
    /* Match Address Mode Enable 1 */
    BW_UART_C4_MAEN1(baseAddr, matchAddrMode1);
    /* Match Address Mode Enable 2 */
    BW_UART_C4_MAEN2(baseAddr, matchAddrMode2);
    /* match address register 1 */
    HW_UART_MA1_WR(baseAddr, matchAddrValue1);
    /* match address register 2 */
    HW_UART_MA2_WR(baseAddr, matchAddrValue2);
}
#endif

#if FSL_FEATURE_UART_HAS_IR_SUPPORT
/*FUNCTION**********************************************************************
 *
 * Function Name : UART_HAL_SetInfraredOperation
 * Description   : Configure the UART infrared operation.
 * The function allows the user to enable or disable the UART infrared (IR) 
 * operation and to configure the IR pulse width.
 *
 *END**************************************************************************/
void UART_HAL_SetInfraredOperation(uint32_t baseAddr, bool enable,
                                   uart_ir_tx_pulsewidth_t pulseWidth)
{
    /* enable or disable infrared */
    BW_UART_IR_IREN(baseAddr, enable);
    /* configure the narrow pulse width of the IR pulse */
    BW_UART_IR_TNP(baseAddr, pulseWidth);
}
#endif  /* FSL_FEATURE_UART_HAS_IR_SUPPORT */

/*******************************************************************************
 * EOF
 ******************************************************************************/

