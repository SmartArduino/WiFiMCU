/*
 * @brief LPC412X SPI ROM API declarations and functions
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

#ifndef __ROM_SPI_412X_H_
#define __ROM_SPI_412X_H_

#ifdef __cplusplus
extern "C" {
#endif
	
/** @defgroup SPIROM_540XX CHIP: LPC540xx SPI ROM API declarations and functions
 * @ingroup ROMAPI_540XX
 * @{
 */

/**
 * @brief SPI handle type
 * The handle to the instance of the SPI driver. Each SPI has one handle, so there can be
 * several handles for each SPI block. This handle is created by Init API and used by the
 * transfer functions for the corresponding SPI block.
 */
typedef void *SPI_HANDLE_T;

/**
 * @brief SPI DMA configuration structure
 */
typedef struct {
	uint32_t dma_txd_num;	/*!< DMA channel number in case DMA mode is enabled. */
	uint32_t dma_rxd_num;	/*!< In order to do a SPI RX DMA, a SPI TX DMA is also needed to generated clock. */
	uint32_t hDMA;		/*!< DMA handle */
//	DMA_HANDLE_T hDMA;		/*!< DMA handle */ // FIXME
} SPI_DMA_CFG_T;

/**
 * @brief SPI interrupt callback function type
 * @param	err_code: SPI error code
 * @param	n: In interrupt mode, this parameter indicates the number of SPI frames.
 *			In DMA mode, this parameter is always zero.
 */
typedef void (*SPI_CALLBK_T)(ErrorCode_t err_code, uint32_t n);

/**
 * @brief SPI DMA setup callback function type.
 * To set up the DMA channel, the source address, destination address, DMA transfer
 * length, DMA request information must be retrieved from the driver structure which has
 * been originally passed from the ROM_SPI_PARAM_T structure.
 * @param	handle: SPI driver handle
 * @param	dma_cfg: DMA configuration.
 */
typedef ErrorCode_t (*SPI_DMA_REQ_T)(SPI_HANDLE_T handle, SPI_DMA_CFG_T *dma_cfg);

// FIXME - unique for Niobe?
typedef struct   {   // block of RAM allocated by the application program
	LPC_SPI_T *       base_addr    ;  // lspi register base address
	uint32_t          fsize_sel    ;  // frame size and SEL select in TXDATCTRL
	uint32_t          tx_rx_flag   ;  // 0 is TX only, 1, is RX only, 0x2 is TX and RX
	uint32_t          eof_flag     ;  // 0 or 1 only. EOF( end of frame ) is needed if set. EOF delay will not be asserted without this flag. 	
	uint32_t          buffer_size  ;  // txd or rxd buffer size	
	uint16_t          *buffer_txd  ;  // transmit buffer
	uint16_t          *buffer_rxd  ;  // receive buffer
	uint32_t          num_txd      ;  // Number of SPI frames transmitted 
	uint32_t          num_rxd      ;  // Number of SPI frames received 
	uint32_t          error_code   ;  // error code
	SPI_CALLBK_T      callback_txd ;  // For interrupt, it's the end of the transfer, for DMA, it's the completion of DMA
	SPI_CALLBK_T      callback_rxd ;  // For interrupt, it's the end of the transfer, for DMA, it's the completion of DMA
	uint32_t          error_en     ;  // enable bits for error detection
} SPI_DRIVER_TypeDef;

/**
 * @brief SPI configuration structure
 */
typedef struct {
	uint32_t delay;		/*!< Configures the delay between SSEL and data transfers and between frames. The
						    value is the content of the SPI DLY register. */
	uint32_t divider;	/*!< Clock divider value DIVVAL in the SPI DIV register. */
	uint32_t config;	/*!< Enable SPI, configure master/slave, configure signal phase and polarity. The
						    value is the content of the SPI CFG register. */
	uint32_t error_en;	/*!< Enables the receive overrun and transmit underrun error interrupts. */
} SPI_CONFIG_T;

/**
 * @brief SPI configuration parameter structure
 */
typedef struct {
	uint16_t *tx_buffer;	/*!< Tx buffer */
	uint16_t *rx_buffer;	/*!< Rx buffer */
	uint32_t size;				/*!< size of the SPI transfer. A transfer can consist of several transmits of the
								                TXCTLDAT register and of several frames. */
	uint32_t fsize_sel;		/*!< write the contents of the SPI TXCTL register to select the data length and the
							                    slave select lines. In slave mode, you need to only select the data length. */
	uint32_t eof_flag;		/*!< EOF flag. EOF( end of frame ) is needed if set. EOF delay will not be asserted without this flag. */
	uint32_t tx_rx_flag;	/*!< Tx & Rx mode flag. 0 is TX only, 1 is RX only, 0x2 is TX and RX */
	uint32_t driver_mode;	/*!< Driver mode.
							                    - 0x00: Polling mode. Function is blocked until transfer is finished.
							                    - 0x01: Interrupt mode. Function exits immediately and a call back function is invoked when the transfer has finished
							                    - 0x02: DMA mode. Data transferred by SPI is processed by DMA.
							            The DMA_req function is called foe SPI DMA channel set up.
							            The callback function indicates when the transfer is complete. */
	SPI_DMA_CFG_T *dma_cfg;	/*!< DMA configuration */
	SPI_CALLBK_T cb;			/*!< SPI interrupt callback function */
	SPI_DMA_REQ_T dma_cb;	/*!< SPI DMA channel set-up call back function. */
} SPI_PARAM_T;
  
/**
 * @brief SPI ROM API structure
 * The SPI API handles SPI data transfer in master and slave modes.
 */
typedef struct {
	/** Memory size for one SPI instance */
	uint32_t (*spi_get_mem_size)(void);
	/** Set up SPI instance and return handle*/
	SPI_HANDLE_T (*spi_setup)(uint32_t base_addr, uint8_t *ram);
	/** Set up SPI operating mode */
	void (*spi_init)(SPI_HANDLE_T handle, SPI_CONFIG_T *set);
	/** Send or receive data in master mode*/
	uint32_t (*spi_master_transfer)(SPI_HANDLE_T handle, SPI_PARAM_T *param);
	/** Send or receive data in slave mode*/
	uint32_t (*spi_slave_transfer)(SPI_HANDLE_T handle, SPI_PARAM_T *param);
	/** Interrupt service routine */
	void (*spi_isr)(SPI_HANDLE_T handle);
	uint32_t (*spi_get_firmware_version)(void);
} SPID_API_T;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __ROM_SPI_412X_H_ */
