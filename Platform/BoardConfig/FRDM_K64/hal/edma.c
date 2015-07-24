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

/*******************************************************************************
 * Standard C Included Files
 ******************************************************************************/
#include <string.h>
/*******************************************************************************
 * Application Included Files
 ******************************************************************************/
#include "edma.h"

#if (defined(__ICCARM__) || defined(__CC_ARM))
#include "mem_tools.h"
#elif defined(__GNUC__)
#include <malloc.h>
#endif

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
/*
    @brief Configures an eDMA transfer loop using a loopSetup structure, and if a valid uart_state_t is passed will print out the TCD for that loop.

    @param loopSetup Data structure defined by user, containing all the parameters for the eDMA loop.

    @param uart Pointer to a valid uart_state_t for debug printing.
 */


edma_software_tcd_t ptcd[10];


void setup_edma_loop(edma_loop_setup_t *loopSetup)
{
  
#if 0
#if (defined(__ICCARM__) || defined(__CC_ARM))
    loopSetup->dmaChStcd = (edma_software_tcd_t *)mem_align(2 * sizeof(edma_software_tcd_t) * loopSetup->period, 32);
#elif defined(__GNUC__)
    loopSetup->dmaChStcd = (edma_software_tcd_t *)memalign(32, 2 * sizeof(edma_software_tcd_t) * loopSetup->period);
#endif
#endif

    loopSetup->dmaChStcd = (edma_software_tcd_t *)(((uint32_t)ptcd + 32)& 0xFFFFFFE0);
    //printf(" eDMA TCD address is %x,  them\r\n", (uint32_t)loopSetup->dmaChStcd);

    memset(loopSetup->dmaChStcd, 0, sizeof(edma_software_tcd_t));

    EDMA_DRV_RequestChannel(loopSetup->dmaChanNum, loopSetup->chSource, loopSetup->dmaCh);

    EDMA_DRV_ConfigLoopTransfer(loopSetup->dmaCh, loopSetup->dmaChStcd, loopSetup->type,
                     loopSetup->srcAddr, loopSetup->destAddr, loopSetup->size,
                     loopSetup->watermark, loopSetup->length, loopSetup->period);

    if(loopSetup->dmaCallBack != NULL)
    {
        EDMA_DRV_InstallCallback(loopSetup->dmaCh, loopSetup->dmaCallBack, loopSetup->dmaCh);
    }
}

/*
    @brief Disables user configured eDMA transfer loop. Also, will free memory allocated by eDMA transfer loop. If a valid uart_state_t is passed will print out the TCD for that loop.

    @param loopSetup Data structure defined by user, containing all the parameters for the eDMA loop.

    @param uart Pointer to a valid uart_state_t for debug printing.
 */

void disable_edma_loop(edma_loop_setup_t *loopSetup)
{
    EDMA_DRV_StopChannel(loopSetup->dmaCh);

#if (defined(__ICCARM__) || defined(__CC_ARM))
    free_align(loopSetup->dmaChStcd);
#elif defined(__GNUC__)
    //OSA_MemFree(loopSetup->dmaChStcd);
#endif

    print_edma_ch_erq(DMA0, loopSetup->dmaChanNum);

    //OSA_MemFree(loopSetup);

}
#if 0
void stop_edma_loop(void *parameter, edma_chn_status_t status)
{
    /* Increase semaphore to indicate an eDMA channel has completed transfer. */
    OSA_SemaPost(&g_statusSem);

    /* Stop eDMA channel transfers. */
    EDMA_DRV_StopChannel((edma_chn_state_t *)parameter);
}
#endif

/******************************************************************************
 * EOF
 ******************************************************************************/
