/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "MicoRtos.h"
#include "string.h" /* For memcpy */
#include "fsl_edma_driver.h"
#include "fsl_dspi_master_driver.h"

#include "fsl_device_registers.h"
#include "board.h"

/******************************************************
 *             Constants
 ******************************************************/
#define DMA_CH0 0x00U
#define DMA_CH1 0x01U
#define DMA_CH2 0x02U

#define DMA_TIMEOUT_LOOPS      (10000000)

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

/**
 * Transfer direction for the mico platform bus interface
 */
typedef enum
{
    /* If updating this enum, the bus_direction_mapping variable will also need to be updated */
    BUS_READ,
    BUS_WRITE
} bus_transfer_direction_t;

#define SPI0_CS_ENABLE (GPIO_DRV_WritePinOutput(spiCsPin[0].pinName, 0))
#define SPI0_CS_DISABLE (GPIO_DRV_WritePinOutput(spiCsPin[0].pinName, 1))
#define SPI0_TXCMD (SPI_PUSHR_PCS(0x01) | SPI_PUSHR_CTAS(0x00))

/******************************************************
 *             Structures
 ******************************************************/

/******************************************************
 *             Variables
 ******************************************************/

static edma_state_t g_edmaState;

/* Semaphore for checking eDMA channel completion. */
static mico_semaphore_t spi_transfer_finished_semaphore;
static dspi_master_state_t dspiMasterState;
static edma_loop_setup_t dmaSPITX;
static edma_loop_setup_t dmaSPIRX;
static edma_chn_state_t dmaCh0;                //transmitting data from Master
static edma_chn_state_t dmaCh1;                //receiving data on Master

extern uint32_t g_dspiBaseAddr[];
static uint32_t buffer_temp_32[2048];

/******************************************************
 *             Function declarations
 ******************************************************/

#ifndef MICO_DISABLE_MCU_POWERSAVE
extern void wake_up_interrupt_notify( void );
#define MCU_NOTIFY_WAKE_UP()        wake_up_interrupt_notify()
#else
#define MCU_NOTIFY_WAKE_UP()
#endif /* ifndef MICO_DISABLE_MCU_POWERSAVE */

/* Powersave functionality */
extern void MCU_CLOCKS_NEEDED( void );
extern void MCU_CLOCKS_NOT_NEEDED( void );

extern void wiced_platform_notify_irq( void );

/******************************************************
 *             Function definitions
 ******************************************************/
void *mem_align(size_t ptrSize, uint32_t alignment)
{
    char gap;   /* Variable to store difference between memory locations. */

    /* Allocate more than enough memory. */
    char *temp = (char *)OSA_MemAlloc(ptrSize + (alignment - 1));
    if(temp == NULL)
    {
        return NULL;  /* Return NULL if not enough space for allocation request. */
    }

    /* Create a pointer that is aligned correctly. */
    char *ptr = (char *)(((uint32_t)temp + (alignment - 1)) & ~(uint32_t)(alignment - 1));

    /* Calculate the gap between start of allocation and aligned pointer. */
    gap = (char)((unsigned int)ptr - (unsigned int)temp);

    if(gap)
    {
        ptr--;                /* Decrement the pointer. */
        *ptr = gap;           /* Store gap size. */
        ptr++;                /* Return the pointer to aligned address. */
        return (void *)ptr;
    } /* If there exists a gap between allocated memory and aligned pointer. */
    else
    {
        return (void *)ptr;
    }/* If no gap. */

}

void free_align(void *ptr)
{
    char pad;
    char count;

    if(ptr != NULL)
    {
        char *temp = ptr;
        temp--;        /* Decrement pointer to find the padding size. */
        pad = *temp;
        temp++;
        count = 0;
        while(count < pad)  /* Decrement pointer to start of previously allocated memory. */
        {
            temp--;
            count++;
        }
        OSA_MemFree(temp);
        temp = NULL;
    }

}

 
/*
    @brief Configures an eDMA transfer loop using a loopSetup structure, and if a valid uart_state_t is passed will print out the TCD for that loop.

    @param loopSetup Data structure defined by user, containing all the parameters for the eDMA loop.

    @param uart Pointer to a valid uart_state_t for debug printing.
 */

void setup_edma_loop(edma_loop_setup_t *loopSetup)
{
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
}

void stop_edma_loop(void *parameter, edma_chn_status_t status)
{
    /* Stop eDMA channel transfers. */
    EDMA_DRV_StopChannel((edma_chn_state_t *)parameter);
}

void stop_edma_loop_putsem(void *parameter, edma_chn_status_t status)
{
    /* Stop eDMA channel transfers. */
    EDMA_DRV_StopChannel((edma_chn_state_t *)parameter);
    /* Increase semaphore to indicate an eDMA channel has completed transfer. */
    mico_rtos_set_semaphore( &spi_transfer_finished_semaphore );
}

void spi_irq_handler( )
{

#ifndef MICO_DISABLE_MCU_POWERSAVE
    wake_up_interrupt_notify( );
#endif /* ifndef MICO_DISABLE_MCU_POWERSAVE */

    wiced_platform_notify_irq( );
}
static void spi_status_print(SPI_MemMapPtr p)
{
#define CTAR_REG_USED       (0)

    printf("MCR %x, TCR %x, CTAR %x, SR %x, RSER %x\r\n",
        SPI_MCR_REG(p), SPI_TCR_REG(p), SPI_CTAR_REG( p, CTAR_REG_USED ), SPI_SR_REG(p), SPI_RSER_REG(p));
}

OSStatus host_platform_bus_init( void )
{
    edma_user_config_t g_edmaUserConfig;

    /* Create a semephore to check for completed eDMA transfers. */
    mico_rtos_init_semaphore(&spi_transfer_finished_semaphore, 1);
    MCU_CLOCKS_NEEDED();
    
    configure_spi_pins(0); // initlize spi0 GPIO
    PORT_HAL_SetMuxMode(PORTD_BASE,0u,kPortMuxAsGpio); // configure CS as gpio, use software configure CS.
    GPIO_DRV_OutputPinInit(&spiCsPin[0]);
    SPI0_CS_DISABLE;
    /* Initialize eDMA & DMAMUX */
    g_edmaUserConfig.chnArbitration = kEDMAChnArbitrationRoundrobin;
    g_edmaUserConfig.notHaltOnError = false;
    EDMA_DRV_Init(&g_edmaState, &g_edmaUserConfig);
    /* DSPI Master Configuration */
    dspi_edma_master_setup(&dspiMasterState, 0, 10000000, 8);
    
    MCU_CLOCKS_NOT_NEEDED();
    dmaSPITX.dmaChanNum = DMA_CH0;
    dmaSPITX.dmaCh = &dmaCh0;
    dmaSPITX.type = kEDMAMemoryToPeripheral;
    dmaSPITX.chSource = kDmaRequestMux0SPI0Tx;
    dmaSPITX.srcAddr = (uint32_t)NULL;
    dmaSPITX.destAddr = (uint32_t)&SPI0->PUSHR;
    dmaSPITX.length = 0;
    dmaSPITX.size = 4;
    dmaSPITX.watermark = 4;
    dmaSPITX.period = 0x01U;
    dmaSPITX.dmaCallBack = stop_edma_loop;
    

    dmaSPIRX.dmaChanNum = DMA_CH1;
    dmaSPIRX.dmaCh = &dmaCh1;
    dmaSPIRX.type = kEDMAPeripheralToMemory;
    dmaSPIRX.chSource = kDmaRequestMux0SPI0Rx;
    dmaSPIRX.srcAddr = (uint32_t)&SPI0->POPR;
    dmaSPIRX.destAddr = (uint32_t)NULL;
    dmaSPIRX.length = 0;
    dmaSPIRX.size = 1;
    dmaSPIRX.watermark = 1;
    dmaSPIRX.period = 0x01U;
    dmaSPIRX.dmaCallBack = stop_edma_loop_putsem;
    dmaSPIRX.dmaChStcd = (edma_software_tcd_t *)mem_align(2 * sizeof(edma_software_tcd_t) * dmaSPIRX.period, 32);

    spi_status_print(SPI0);
    return kNoErr;
}

OSStatus host_platform_bus_deinit( void )
{
    MCU_CLOCKS_NEEDED();

    DSPI_DRV_MasterDeinit(0);

    MCU_CLOCKS_NOT_NEEDED();
    
    mico_rtos_deinit_semaphore(&spi_transfer_finished_semaphore);
    return kNoErr;
}

OSStatus host_platform_spi_transfer( bus_transfer_direction_t dir, uint8_t* buffer, uint16_t buffer_length )
{
    OSStatus result;
    int i;

    for(i=0; i<buffer_length; i++) {
        buffer_temp_32[i] = SPI0_TXCMD | (uint32_t)buffer[i];;
    }
    dmaSPITX.dmaChStcd = (edma_software_tcd_t *)mem_align(2 * sizeof(edma_software_tcd_t) * dmaSPITX.period, 32);
    dmaSPITX.srcAddr = (uint32_t)buffer_temp_32;
    dmaSPITX.length = buffer_length * 4;
    dmaSPIRX.dmaChStcd = (edma_software_tcd_t *)mem_align(2 * sizeof(edma_software_tcd_t) * dmaSPIRX.period, 32);
    dmaSPIRX.destAddr = (uint32_t)buffer;
    dmaSPIRX.length = buffer_length;
    
    MCU_CLOCKS_NEEDED();
    SPI0_CS_ENABLE;
    setup_edma_loop(&dmaSPITX);
    setup_edma_loop(&dmaSPIRX);

    EDMA_DRV_StartChannel(dmaSPIRX.dmaCh);
    EDMA_DRV_StartChannel(dmaSPITX.dmaCh);

    result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, 100 );
    disable_edma_loop(&dmaSPIRX);
    disable_edma_loop(&dmaSPITX);
    SPI0_CS_DISABLE;
    MCU_CLOCKS_NOT_NEEDED();
    free_align(dmaSPITX.dmaChStcd);
    free_align(dmaSPIRX.dmaChStcd);
    return result;
}

