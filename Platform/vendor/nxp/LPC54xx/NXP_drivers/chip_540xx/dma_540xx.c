/*
 * @brief LPC540XX DMA chip driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
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

#include "chip.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* DMA SRAM table - this can be optionally used with the Chip_DMA_SetSRAMBase()
   function if a DMA SRAM table is needed. This table is correctly aligned for
     the DMA controller. */
#if defined(__CC_ARM)
/* Keil alignement to 512 bytes */
__align(512) DMA_CHDESC_T Chip_DMA_Table[MAX_DMA_CHANNEL];
#endif /* defined (__CC_ARM) */

/* IAR support */
#if defined(__ICCARM__)
/* IAR EWARM alignement to 512 bytes */
#pragma data_alignment=512
DMA_CHDESC_T Chip_DMA_Table[MAX_DMA_CHANNEL];
#endif /* defined (__ICCARM__) */

#if defined( __GNUC__ )
/* GNU alignement to 512 bytes */
DMA_CHDESC_T Chip_DMA_Table[MAX_DMA_CHANNEL] __attribute__ ((aligned(512)));
#endif /* defined (__GNUC__) */

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Set DMA transfer register interrupt bits (safe) */
void Chip_DMA_SetTranBits(LPC_DMA_T *pDMA, DMA_CHID_T ch, uint32_t mask)
{
	uint32_t temp;

	/* Read and write values may not be the same, write 0 to
	   undefined bits */
	temp = pDMA->DMACH[ch].XFERCFG & ~0xFC000CC0;

	pDMA->DMACH[ch].XFERCFG = temp | mask;
}

/* Clear DMA transfer register interrupt bits (safe) */
void Chip_DMA_ClearTranBits(LPC_DMA_T *pDMA, DMA_CHID_T ch, uint32_t mask)
{
	uint32_t temp;

	/* Read and write values may not be the same, write 0 to
	   undefined bits */
	temp = pDMA->DMACH[ch].XFERCFG & ~0xFC000CC0;

	pDMA->DMACH[ch].XFERCFG = temp & ~mask;
}

/* Update the transfer size in an existing DMA channel transfer configuration */
void Chip_DMA_SetupChannelTransferSize(LPC_DMA_T *pDMA, DMA_CHID_T ch, uint32_t trans)
{
	Chip_DMA_ClearTranBits(pDMA, ch, (0x3FF << 16));
	Chip_DMA_SetTranBits(pDMA, ch, DMA_XFERCFG_XFERCOUNT(trans));
}

/* Sets up a DMA channel with the passed DMA transfer descriptor */
bool Chip_DMA_SetupTranChannel(LPC_DMA_T *pDMA, DMA_CHID_T ch, DMA_CHDESC_T *desc)
{
	bool good = false;
	DMA_CHDESC_T *pDesc = (DMA_CHDESC_T *) pDMA->SRAMBASE;

	if ((Chip_DMA_GetActiveChannels(pDMA) & (1 << ch)) == 0) {
		/* Channel is not active, so update the descriptor */
		pDesc[ch] = *desc;

		good = true;
	}

	return good;
}

/**
 * Initialize DMA parameters specific to a channel 
 * 
 * @param channel
 * @param src_address
 * @param dst_address
 * @param xfr_width
 * @param length_bytes
 */
void Chip_DMA_InitChannel( DMA_CHID_T channel, uint32_t src_address, uint32_t src_increment,
        uint32_t dst_address, uint32_t dst_increment, uint32_t xfr_width, uint32_t length_bytes, uint32_t priority) 
{
  Chip_DMA_EnableChannel(LPC_DMA, channel);
  Chip_DMA_EnableIntChannel(LPC_DMA, channel);

  Chip_DMA_SetupChannelConfig(LPC_DMA, channel, DMA_CFG_PERIPHREQEN |
          DMA_CFG_CHPRIORITY(priority));

  if (src_increment != DMA_XFERCFG_SRCINC_0) {
    Chip_DMA_Table[channel].source = DMA_ADDR((src_address + length_bytes)
            - (1UL << xfr_width));
  } else {
    Chip_DMA_Table[channel].source = DMA_ADDR(src_address);
  }

  if (dst_increment != DMA_XFERCFG_DSTINC_0) {
    Chip_DMA_Table[channel].dest = DMA_ADDR((dst_address + length_bytes)
            - (1UL << xfr_width));
  } else {
    Chip_DMA_Table[channel].dest = DMA_ADDR(dst_address);
  }
  Chip_DMA_Table[channel].next = DMA_ADDR(0);

}

/**
 * Start the DMA transfer 
 * 
 * @param channel
 * @param src_increment
 * @param dst_increment
 * @param xfr_width
 * @param length_bytes
 */
void Chip_DMA_StartTransfer(DMA_CHID_T channel, uint32_t src_increment, uint32_t dst_increment, uint32_t xfr_width, uint32_t length_bytes) 
{
  uint32_t xfer_count;

  /* Calculate transfer_count ( length in terms of transfers) */
  xfer_count = (xfr_width == DMA_XFERCFG_WIDTH_8) ? length_bytes :
          (xfr_width == DMA_XFERCFG_WIDTH_16) ? (length_bytes >> 1) :
          (length_bytes >> 2);

  Chip_DMA_SetupChannelTransfer(LPC_DMA, channel,
          (DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA | DMA_XFERCFG_SWTRIG |
          xfr_width | src_increment | dst_increment |
          DMA_XFERCFG_XFERCOUNT(xfer_count)));
}

