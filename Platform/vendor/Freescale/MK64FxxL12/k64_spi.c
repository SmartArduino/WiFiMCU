/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "k64_spi.h"
#include "fsl_edma_driver.h"
#include "edma.h"

typedef enum _bus_status_t
{
    BUS_READ,
    BUS_WRITE        
}bus_status;


/* For EDMA init */
static edma_state_t g_edmaState;
static edma_user_config_t g_edmaUserConfig;
static edma_loop_setup_t write_dmaLoop;
static edma_loop_setup_t read_dmaLoop;
static uint32_t spi_tx_buffer[2048];
static volatile uint8_t spi_dma_sem;


/* For EDMA channel */
edma_chn_state_t dmaCh0;                //transmitting data from Master
edma_chn_state_t dmaCh1;                //receiving data on Master


#define CTAR_REG_USED       (0)
#define DOUBLE_BAUD_RATE    (0)
#define BAUD_RATE_PRESCALER (2)
#define CTAR_PBR            (0)
#define MAX_LOOP_COUNT  (10000)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

static void    set_spi_peripheral_clock( SPI_MemMapPtr spi_peripheral, bool enable );
static void    clear_spi_fifos( SPI_MemMapPtr spi_peripheral );
static uint8_t get_baud_rate_scaler_register_value( uint32_t baud_rate_Mbps );

/******************************************************
 *               Variables Definitions
 ******************************************************/

extern int periph_clk_khz;

/******************************************************
 *               Function Definitions
 ******************************************************/
static void spi_status_print(SPI_MemMapPtr p)
{
    printf("MCR %x, TCR %x, CTAR %x, SR %x, RSER %x PUSHR %x\r\n",
        SPI_MCR_REG(p), SPI_TCR_REG(p), SPI_CTAR_REG( p, CTAR_REG_USED ), SPI_SR_REG(p), SPI_RSER_REG(p),SPI_PUSHR_REG(p));
}

void DMA_init(void)
{
	spi_dma_sem = 0;
	
     /* Initialize eDMA & DMAMUX */
    g_edmaUserConfig.chnArbitration = kEDMAChnArbitrationRoundrobin;
    g_edmaUserConfig.notHaltOnError = false;
    EDMA_DRV_Init(&g_edmaState, &g_edmaUserConfig);
}

OSStatus spi_init( spi_driver_t* spi_driver, SPI_MemMapPtr spi_peripheral, uint32_t baud_rate_bps, uint8_t chip_select, bool polarity, bool phase, bool use_dma )
{
    uint8_t br = get_baud_rate_scaler_register_value( baud_rate_bps );

    spi_driver->spi_peripheral = spi_peripheral;
    spi_driver->baud_rate_bps  = baud_rate_bps;
    spi_driver->chip_select    = chip_select;
    spi_driver->polarity       = polarity;
    spi_driver->phase          = phase;
    spi_driver->use_dma        = use_dma;

    /* Enable SPI peripheral clock */
    set_spi_peripheral_clock( spi_peripheral, true );

    /* Enable SPI peripheral and clean up (stop) any previous transfer
     * MDIS     = 0 to enable
     * HALT     = 1 to stop transfer
     * MSTR     = 1 for master mode
     * DCONF    = 0 for SPI
     * PCSIS[x] = 1 for CS active low
     */
    SPI_MCR_REG( spi_peripheral ) &= ~(uint32_t) ( SPI_MCR_MDIS_MASK | SPI_MCR_DCONF(0) );
    SPI_MCR_REG( spi_peripheral ) |=  (uint32_t) ( (0x1<<24)|SPI_MCR_HALT_MASK | SPI_MCR_MSTR_MASK | SPI_MCR_PCSIS( 1 << chip_select ) );

    /* Select Clock and Transfer Attributes Register (CTAR). Always use CTAR0 */
    SPI_PUSHR_REG( spi_peripheral ) &= ~(uint32_t) SPI_PUSHR_CTAS(CTAR_REG_USED);

    /* Reset Clock and Transfer Attributes (CTAR) register */
    SPI_CTAR_REG( spi_peripheral, CTAR_REG_USED ) = 0;

    /* Set SPI configuration
     * FMSZ   = 7. Set frame size to 8-bit. frame size = FMSZ + 1
     * CPOL   = phase
     * CPHA   = polarity
     * DBR    = 00
     * PBR    = 2
     * BR     = calculate based on baud_rate_Mbps
     * PCSSCK = 0
     * PASC   = 0
     * PDT    = 0
     * CSSCK  = BR - 1
     * ASC    = BR - 1
     * DT     = 0
     */
    SPI_CTAR_REG( spi_peripheral, CTAR_REG_USED ) |= (uint32_t) ( SPI_CTAR_CPOL_MASK & (uint32_t)( polarity << SPI_CTAR_CPOL_SHIFT ) ) |
                                                     (uint32_t) ( SPI_CTAR_CPHA_MASK & (uint32_t)( phase    << SPI_CTAR_CPHA_SHIFT ) ) |
                                                     (uint32_t) ( SPI_CTAR_FMSZ( 8 - 1 ) ) |
                                                     (uint32_t) ( SPI_CTAR_DBR_MASK & ( DOUBLE_BAUD_RATE << SPI_CTAR_DBR_SHIFT ) ) |
                                                     (uint32_t) ( SPI_CTAR_PBR( CTAR_PBR ) ) |
                                                     (uint32_t) ( SPI_CTAR_BR( br ) ) |
                                                     (uint32_t) ( SPI_CTAR_CSSCK( br - 1 ) ) |
                                                     (uint32_t) ( SPI_CTAR_ASC( br - 1 ) );

    clear_spi_fifos( spi_peripheral );
        
    /* Enable the start transfer bit */
    SPI_MCR_REG( spi_peripheral ) &= ~(uint32_t) ( SPI_MCR_HALT_MASK );

	if(use_dma)
	{
		SPI_RSER_REG( spi_peripheral ) |= (0x3<<24)|(0x3<<16);
		DMA_init();
	}

    spi_status_print(spi_peripheral);
    return kNoErr;
}

OSStatus spi_deinit( spi_driver_t* spi_driver )
{
    clear_spi_fifos( spi_driver->spi_peripheral );

    /* Halt transfer */
    SPI_MCR_REG( spi_driver->spi_peripheral ) |=  (uint32_t) ( SPI_MCR_HALT_MASK );

    /* Disable module */
    SPI_MCR_REG( spi_driver->spi_peripheral ) &= ~(uint32_t) ( SPI_MCR_MDIS_MASK );

    /* Disable SPI peripheral clock */
    set_spi_peripheral_clock( spi_driver->spi_peripheral, false );
    return kNoErr;
}

void stop_tx_edma(void *parameter, edma_chn_status_t status)
{
    /* Stop eDMA channel transfers. */
    EDMA_DRV_StopChannel((edma_chn_state_t *)parameter);
}

void stop_rx_edma(void *parameter, edma_chn_status_t status)
{
    spi_dma_sem = 1;

    /* Stop eDMA channel transfers. */
    EDMA_DRV_StopChannel((edma_chn_state_t *)parameter);
}


OSStatus spi_dma_transfer(const uint8_t* data_out, uint8_t* data_in, uint32_t size )
{
	uint32_t i,loop_count = MAX_LOOP_COUNT;
		
	spi_dma_sem = 0;

    for(i=0;i<size;i++)
      spi_tx_buffer[i] = (uint32_t)  *data_out++;
	
	
    write_dmaLoop.dmaChanNum = DMA_CH0;             
    write_dmaLoop.dmaCh = &dmaCh0;
    write_dmaLoop.type = kEDMAMemoryToPeripheral;
    write_dmaLoop.chSource = kDmaRequestMux0SPI0Tx;
   	write_dmaLoop.srcAddr = (uint32_t)spi_tx_buffer;
    write_dmaLoop.destAddr = (uint32_t)&SPI0->PUSHR;
    write_dmaLoop.length = size*4;
    write_dmaLoop.size = 4;
    write_dmaLoop.watermark = 4;
    write_dmaLoop.period = 1;
    write_dmaLoop.dmaCallBack = stop_tx_edma;

    setup_edma_loop(&write_dmaLoop);

   	read_dmaLoop.dmaChanNum = DMA_CH1;
   	read_dmaLoop.dmaCh = &dmaCh1;
   	read_dmaLoop.type = kEDMAPeripheralToMemory;
   	read_dmaLoop.chSource = kDmaRequestMux0SPI0Rx;
   	read_dmaLoop.srcAddr = (uint32_t)&SPI0->POPR;
	read_dmaLoop.destAddr = (uint32_t)data_in;
	read_dmaLoop.length = size;
    read_dmaLoop.size = 1;
   	read_dmaLoop.watermark = 1;
    read_dmaLoop.period = 1;
    read_dmaLoop.dmaCallBack = stop_rx_edma;

    setup_edma_loop(&read_dmaLoop);

	
   	EDMA_DRV_StartChannel(read_dmaLoop.dmaCh);	
	EDMA_DRV_StartChannel(write_dmaLoop.dmaCh);

	
	while(loop_count>0)
	{
		if(spi_dma_sem)
			return kNoErr;

		//loop_count--;
	}

	return kTimeoutErr;
}

OSStatus spi_transfer( spi_driver_t* spi_driver, const uint8_t* data_out, uint8_t* data_in, uint32_t size )
{
    uint32_t loop_count = MAX_LOOP_COUNT;

    clear_spi_fifos( spi_driver->spi_peripheral );

	if(spi_driver->use_dma)
	{
		return spi_dma_transfer(data_out,data_in,size);
	}



    while ( size > 0 )
    {
        /* Push frame to TX FIFO */
        //SPI_PUSHR_REG( spi_driver->spi_peripheral ) = (uint32_t) ( *data_out++ | SPI_PUSHR_PCS( 1 << spi_driver->chip_select ) | ( ( size != 1 ) ? SPI_PUSHR_CONT_MASK : 0 ) );
        SPI_PUSHR_REG( spi_driver->spi_peripheral ) = (uint32_t) ( *data_out++);

        /* Wait until RX FIFO is not empty */
        while ( ( SPI_SR_RXCTR_MASK & SPI_SR_REG( spi_driver->spi_peripheral ) ) == 0 && loop_count > 0 )
        {
            loop_count--;
        }

        if ( loop_count == 0 )
        {
            return kTimeoutErr;
        }

        /* Pop frame from RX FIFO */
        *data_in++ = (uint8_t)SPI_POPR_REG( spi_driver->spi_peripheral );

        size--;
    }

    return kNoErr;
}

static void set_spi_peripheral_clock( SPI_MemMapPtr spi_peripheral, bool enable )
{
    switch ( (uint32_t) spi_peripheral )
    {
        case SPI0_BASE :
            SIM_SCGC6 = ( enable == true ) ? ( SIM_SCGC6 | (uint32_t) ( 1 << 12 ) ) : ( SIM_SCGC6 & ~(uint32_t) ( 1 << 12 ) );
            return;

        case SPI1_BASE :
            SIM_SCGC6 = ( enable == true ) ? ( SIM_SCGC6 | (uint32_t) ( 1 << 13 ) ) : ( SIM_SCGC6 & ~(uint32_t) ( 1 << 13 ) );
            return;

        case SPI2_BASE :
            SIM_SCGC3 = ( enable == true ) ? ( SIM_SCGC6 | (uint32_t) ( 1 << 12 ) ) : ( SIM_SCGC6 & ~(uint32_t) ( 1 << 12 ) );
            return;

        default:
            return;
    }
}

static uint8_t get_baud_rate_scaler_register_value( uint32_t baud_rate_bps )
{
    uint8_t baud_rate_scaler_reg_val = 0;
    uint32_t baud_rate_scaler         = 2;
    uint32_t current_baud_rate_bps    = 60000000 * ( 1 + DOUBLE_BAUD_RATE ) / ( BAUD_RATE_PRESCALER * baud_rate_scaler );

    /* Find closest scaler value */
    while ( current_baud_rate_bps >= baud_rate_bps )
    {
        baud_rate_scaler        = ( baud_rate_scaler < 8 ) ? ( baud_rate_scaler + 2 ) : ( baud_rate_scaler * 2 );
        current_baud_rate_bps   = 60000000 * ( 1 + DOUBLE_BAUD_RATE ) / ( BAUD_RATE_PRESCALER * baud_rate_scaler );
        baud_rate_scaler_reg_val++;
    }

    return baud_rate_scaler_reg_val;
}

static void clear_spi_fifos( SPI_MemMapPtr spi_peripheral  )
{
    SPI_MCR_REG( spi_peripheral ) |= (uint32_t)( SPI_MCR_CLR_RXF_MASK | SPI_MCR_CLR_TXF_MASK );
    SPI_SR_REG ( spi_peripheral ) |= (uint32_t)( SPI_SR_RFOF_MASK );
}
