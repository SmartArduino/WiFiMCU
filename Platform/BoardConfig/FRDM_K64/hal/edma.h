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
 *
 */

#ifndef __EDMA_H__
#define __EDMA_H__
/*******************************************************************************
 * Standard C Included Files
 ******************************************************************************/
#include <stdint.h>
/*******************************************************************************
 * SDK Included Files
 ******************************************************************************/
#include "fsl_edma_driver.h"
#include "fsl_uart_driver.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/  
#define DMA_CH0 0x00U
#define DMA_CH1 0x01U
#define DMA_CH2 0x02U
#define DMA_CH3 0x03U
#define DMA_CH4 0x04U
#define DMA_CH5 0x05U

/*******************************************************************************
 * Constants
 ******************************************************************************/
/* DMA software TCD arrays. TCDs must be aligned to 32-byte boundary. */
/*
    **** NOTE ****
    This has been left commented out so that the user may choose to use
    costant software tcds at their own discretion.
    **************

#define LOOP_PERIOD 0x01U

#if defined(__ICCARM__)

    #pragma data_alignment = 32 * LOOP_PERIOD
    __root edma_software_tcd_t stcd0[LOOP_PERIOD];
    #pragma data_alignment = 32 * LOOP_PERIOD
    __root edma_software_tcd_t stcd1[LOOP_PERIOD];
    #pragma data_alignment = 32 * LOOP_PERIOD
    __root edma_software_tcd_t stcd2[LOOP_PERIOD];

#elif defined(__GNUC__) || defined(__CC_ARM)

    edma_software_tcd_t stcd0[LOOP_PERIOD] __attribute__ ((aligned(32 * LOOP_PERIOD)));
    edma_software_tcd_t stcd1[LOOP_PERIOD] __attribute__ ((aligned(32 * LOOP_PERIOD)));
    edma_software_tcd_t stcd2[LOOP_PERIOD] __attribute__ ((aligned(32 * LOOP_PERIOD)));

#endif
*/
/*******************************************************************************
 * Data Structures
 ******************************************************************************/    
typedef struct EdmaLoopSetup
{
    edma_chn_state_t *dmaCh;
    uint32_t dmaChanNum;
    dma_request_source_t chSource;
    edma_transfer_type_t  type;
    uint32_t srcAddr;
    uint32_t destAddr;
    uint32_t size;
    uint32_t watermark;
    uint32_t length;
    uint8_t period;
    edma_software_tcd_t *dmaChStcd;
    edma_callback_t dmaCallBack;

} edma_loop_setup_t;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void setup_edma_loop(edma_loop_setup_t *loopSetup);

void disable_edma_loop(edma_loop_setup_t *loopSetup);

void stop_edma_loop(void *parameter, edma_chn_status_t status);

#endif /* __EDMA_H__ */

/******************************************************************************
 * EOF
 ******************************************************************************/
