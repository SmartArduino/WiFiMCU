/*
 * @brief LPC540XX VFIFO chip driver
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

#ifndef __VFIFO_540XX_H_
#define __VFIFO_540XX_H_

#ifdef __cplusplus
extern "C" {
#endif

    /** @defgroup VFIFO_540XX CHIP: LPC540XX Virtual FIFO driver
     * @ingroup CHIP_540XX_Drivers
     * @{
     */

    /**
     * @brief LPC540XX VFIFO register block structure
     */
    typedef struct {
        __IO uint32_t CFGU; // 0x0                     
        __IO uint32_t STATU; // 0x4
        __IO uint32_t INTSTATU; // 0x8
        __IO uint32_t CTLSETU; // 0xC
        __IO uint32_t CTLCLRU; // 0x10                       
        __IO uint32_t RXDATU; // 0x14
        __IO uint32_t RXDATSTATU; // 0x18
        __IO uint32_t TXDATU; // 0x1C
        uint32_t RESERVED0[(0x100 - 0x20) / 4]; // 0x20-0xFF  
    } VFIFO_UART_T;

    typedef struct {
        __IO uint32_t CFGSPI; // 0x0                     
        __IO uint32_t STATSPI; // 0x4
        __IO uint32_t INTSTATSPI; // 0x8
        __IO uint32_t CTLSETSPI; // 0xC
        __IO uint32_t CTLCLRSPI; // 0x10                       
        __I  uint32_t RXDATSPI; // 0x14
        union {
          struct {
            __O uint16_t TXDATSPI_DATA;
            __O uint16_t TXDATSPI_CTRL;
          };
          __O uint32_t TXDATSPI; // 0x18
        };
        uint32_t RESERVED0[(0x100 - 0x1C) / 4]; // 0x1C-0xFF  
    } VFIFO_SPI_T;

    typedef struct {
        uint32_t RESERVED0[0x100 / 4]; //(@0x0-0xFF) 
        __IO uint32_t FIFOCTLUART; //(@0x100) 
        __IO uint32_t FIFORSTUART; //(@0x104)
        uint32_t RESERVED1[2]; //(@0x108-0x10F)
        __IO uint32_t FIFOCFGU[(0x200 - 0x110) / 4]; //(@0x110-0x1FF)
        __IO uint32_t FIFOCTLSPI; //(@0x200) 
        __IO uint32_t FIFORSTSPI; //(@0x204)
        uint32_t RESERVED2[2]; //(@0x208-0x20C)
        __IO uint32_t FIFOCFGSPI[(0x300 - 0x210) / 4]; //(@0x210-0x2FF)  
        uint32_t RESERVED3[(0xFFC - 0x300) / 4]; //(@0x300-0xFFB)  
        __IO uint32_t ID; //(@0xFFC)
        VFIFO_UART_T UART[16]; //(@0x1000-0x1FFF)
        VFIFO_SPI_T SPI[16]; //(@0x2000-0x2FFF)
    } LPC_VFIFO_T;

    typedef struct { // block of RAM allocated by the application program
        uint32_t uart_num;
        uint32_t tx_rx_flag; // 0 is TX only, 1, is RX only, 0x2 is TX and RX	
        uint32_t rx_threshold;
        uint32_t tx_threshold;
        uint32_t timeout_base;
        uint32_t timeout_value;
        uint32_t timeout_cont_on_write;
        uint32_t timeout_cont_on_empty;
        uint32_t uart_tx_used_fifo_size;
        uint32_t uart_rx_used_fifo_size;
        uint16_t uart_tx_fifo_size;
        uint32_t uart_rx_fifo_size;
    } VFIFO_UART_DRIVER_T; //HEADER_TypeDef	 *********************************

    typedef struct { // block of RAM allocated by the application program
        uint32_t spi_num;
        uint32_t tx_rx_flag; // 0 is TX only, 1, is RX only, 0x2 is TX and RX	
        uint32_t rx_threshold;
        uint32_t tx_threshold;
        uint32_t timeout_base;
        uint32_t timeout_value;
        uint32_t timeout_cont_on_write;
        uint32_t timeout_cont_on_empty;
        uint32_t spi_tx_used_fifo_size;
        uint32_t spi_rx_used_fifo_size;
        uint16_t spi_tx_fifo_size;
        uint32_t spi_rx_fifo_size;
    } VFIFO_SPI_DRIVER_T; //HEADER_TypeDef	 *********************************

    // bit definitions
    // FIFOCtl* register
#define VFIFO_RXPAUSE  (1UL << 0)
#define VFIFO_RXPAUSED (1UL << 1)
#define VFIFO_RXEMPTY  (1UL << 2)
#define VFIFO_TXPAUSE  (1UL << 8)
#define VFIFO_TXPAUSED (1UL << 9)
#define VFIFO_RXFIFO_TOTAL(FIFOCtlUart_val)  (((FIFOCtlUart_val) >> 16) & 0xFF)
#define VFIFO_TXFIFO_TOTAL(FIFOCtlUart_val)  (((FIFOCtlUart_val) >> 24) & 0xFF)

    // FIFORst* register
#define VFIFO_RXRST(ch) (1UL << (ch))
#define VFIFO_TXRST(ch) (1UL << ((ch) + 16))

    // FIFOCfg* register
#define VFIFO_RXSIZE(d) ((d) << 0)
#define VFIFO_TXSIZE(d) ((d) << 8)

    // Cfg register
#define VFIFO_TIMEOUT_CONTINUE_ON_WRITE (1 << 4)
#define VFIFO_TIMEOUT_CONTINUE_ON_EMPTY (1 << 5) 
#define VFIFO_TIMEOUT_BASE(d)     ((d) << 8)
#define VFIFO_TIMEOUT_VALUE(d)    ((d) << 12)
#define VFIFO_RXTHRESHOLD(d)      ((d) << 16)
#define VFIFO_TXTHRESHOLD(d)      ((d) << 24)

    // Stat and IntStat register
#define VFIFO_ST_RXTHRESHOLD       (1 << 0)
#define VFIFO_ST_TXTHRESHOLD       (1 << 1)
#define VFIFO_ST_RXTIMEOUT         (1 << 4)
#define VFIFO_ST_BUSERROR          (1 << 7)
#define VFIFO_ST_RXEMPTY           (1 << 8)
#define VFIFO_ST_TXEMPTY           (1 << 9)
#define VFIFO_ST_RXCOUNT(Stat_val)     (((Stat_val) >> 16) & 0xFF)
#define VFIFO_ST_TXAVAILABLE(Stat_val) (((Stat_val) >> 24) & 0xFF) 
#define VFIFO_ST_RXCOUNTMASK           (0xFFUL << 16)
#define VFIFO_ST_TXAVAILABLEMASK       (0xFFUL << 24) 

    // CtlSet and CtlClr register
#define VFIFO_RX_THRESHOLD_INT_EN       (1 << 0)
#define VFIFO_TX_THRESHOLD_INT_EN       (1 << 1)
#define VFIFO_RX_TIMEOUT_INT_EN         (1 << 4)

    /**
     * @brief	Transmit a single data byte through the VFIFO UART peripheral
     * @param	pVFIFO	: Pointer to selected VFIFO UART peripheral
     * @param	data	: Byte to transmit
     * @return	Nothing
     * @note	This function attempts to place a byte into the VFIFO UART transmit
     *			holding register regard regardless of VFIFO UART state.
     */
    STATIC INLINE void Chip_VFIFO_EnableUARTInterrupt(LPC_VFIFO_T *pVFIFO, uint32_t uart_num, uint32_t mask) {
        pVFIFO->UART[uart_num].CTLSETU = mask;
    }

    /**
     * @brief	Disable UART interrupt(s) 
     * @param	pVFIFO	: Pointer to VFIFO peripheral
     * @param	uart_num : 
     * @param   mask : ORed mask of interrupts to clear 
     * @return	Nothing
     * @note	
     */
    STATIC INLINE void Chip_VFIFO_DisableUARTInterrupt(LPC_VFIFO_T *pVFIFO, uint32_t uart_num, uint32_t mask) {
        pVFIFO->UART[uart_num].CTLCLRU = mask;
    }

    /**
     * @brief	Transmit a single data byte through the VFIFO UART peripheral
     * @param	pVFIFO	: Pointer to selected VFIFO UART peripheral
     * @param	data	: Byte to transmit
     * @return	Nothing
     * @note	This function attempts to place a byte into the VFIFO UART transmit
     *			holding register regard regardless of VFIFO UART state.
     */
    STATIC INLINE void Chip_VFIFO_SendUARTByte(LPC_VFIFO_T *pVFIFO, uint32_t uart_num, uint8_t data) {
        pVFIFO->UART[uart_num].TXDATU = (uint32_t) data;
    }

    /**
     * @brief	Read a single byte data from the UART peripheral
     * @param	pUART	: Pointer to selected UART peripheral
     * @return	A single byte of data read
     * @note	This function reads a byte from the UART receive FIFO or
     *			receive hold register regard regardless of UART state. The
     *			FIFO status should be read first prior to using this function
     */
    STATIC INLINE uint32_t Chip_VFIFO_ReadUARTByte(LPC_VFIFO_T *pVFIFO, uint32_t uart_num) {
        /* Strip off undefined reserved bits, keep 9 lower bits */
        return (uint32_t) (pVFIFO->UART[uart_num].RXDATU & 0x000001FF);
    }

    /**
     * @brief	Get VFIFO UART interrupt status
     * @param	pVFIFO	: The base of VFIFO UART peripheral on the chip
     * @return	The Interrupt status register of VFIFO UART
     * @note	Multiple interrupts may be pending.
     */
    STATIC INLINE uint32_t Chip_VFIFO_GetUARTIntStatus(LPC_VFIFO_T *pVFIFO, uint32_t uart_num) {
        return pVFIFO->UART[uart_num].INTSTATU;
    }

    /**
     * @brief	Get the VFIFO UART status register
     * @param	pVFIFO	: Pointer to selected VFIFO UARTx peripheral
     * @return	VFIFO UART status register
     * @note	Multiple statuses may be pending.
     */
    STATIC INLINE uint32_t Chip_VFIFO_GetUARTStatus(LPC_VFIFO_T *pVFIFO, uint32_t uart_num) {
        return pVFIFO->UART[uart_num].STATU;
    }

    /**
     * @brief	Clear the VFIFO UART status register
     * @param	pVFIFO	: Pointer to selected VFIFO UARTx peripheral
     * @param	stsMask	: OR'ed status to disable
     * @return	Nothing
     * @note	Some of the status bits are RO and can't be cleared.
     */
    STATIC INLINE void Chip_VFIFO_ClearUARTStatus(LPC_VFIFO_T *pVFIFO, uint32_t uart_num, uint32_t statusMask) {
        if (statusMask & VFIFO_ST_RXTIMEOUT)
            pVFIFO->UART[uart_num].STATU = VFIFO_ST_RXTIMEOUT;
        if (statusMask & VFIFO_ST_BUSERROR)
            pVFIFO->UART[uart_num].STATU = VFIFO_ST_BUSERROR;
    }

    
    /**
     * @brief	Transmit a single data byte through the VFIFO UART peripheral
     * @param	pVFIFO	: Pointer to selected VFIFO UART peripheral
     * @param	data	: Byte to transmit
     * @return	Nothing
     * @note	This function attempts to place a byte into the VFIFO UART transmit
     *			holding register regard regardless of VFIFO UART state.
     */
    STATIC INLINE void Chip_VFIFO_EnableSPIInterrupt(LPC_VFIFO_T *pVFIFO, uint32_t spi_num, uint32_t mask) {
        pVFIFO->SPI[spi_num].CTLSETSPI = mask;
    }

    /**
     * @brief	Transmit a single data byte through the VFIFO UART peripheral
     * @param	pVFIFO	: Pointer to selected VFIFO UART peripheral
     * @param	data	: Byte to transmit
     * @return	Nothing
     * @note	This function attempts to place a byte into the VFIFO UART transmit
     *			holding register regard regardless of VFIFO UART state.
     */
    STATIC INLINE void Chip_VFIFO_DisableSPIInterrupt(LPC_VFIFO_T *pVFIFO, uint32_t spi_num, uint32_t mask) {
        pVFIFO->SPI[spi_num].CTLCLRSPI = mask;
    }

    /**
     * @brief Transmit a 32-bit word including both data and ctrl through the SPI peripheral
     * @param	pVFIFO	: Pointer to selected VFIFO peripheral
     * @param	data	: word including both data and ctrl to transmit
     * @return	Nothing
     * @note	This function attempts to place a word into the SPI transmit
     *			holding register regard regardless of VFIFO state.
     */
    STATIC INLINE void Chip_VFIFO_SendSPIDataCtrl(LPC_VFIFO_T *pVFIFO, uint32_t spi_num, uint32_t datactrl) {
        pVFIFO->SPI[spi_num].TXDATSPI = (uint32_t) datactrl;
    }

    /**
     * @brief	Transmit a 16-bit half-word data through the SPI peripheral
     * @param	pUART	: Pointer to selected SPI peripheral
     * @param	data	: half-word to transmit
     * @return	Nothing
     * @note	This function attempts to place half-word data into the SPI transmit
     *			holding register regard regardless of SPI state.
     */
    STATIC INLINE void Chip_VFIFO_SendSPIData(LPC_VFIFO_T *pVFIFO, uint32_t spi_num, uint16_t data) {
        pVFIFO->SPI[spi_num].TXDATSPI_DATA = (uint16_t) data;
    }

    /**
     * @brief	Transmit a 16-bit half-word ctrl through the SPI peripheral
     * @param	pUART	: Pointer to selected SPI peripheral
     * @param	data	: half-word to transmit
     * @return	Nothing
     * @note	This function attempts to place a half-word ctrl into the SPI transmit
     *			holding register regard regardless of SPI state.
     */
    STATIC INLINE void Chip_VFIFO_SendSPICtrl(LPC_VFIFO_T *pVFIFO, uint32_t spi_num, uint32_t ctrl) {
        pVFIFO->SPI[spi_num].TXDATSPI_CTRL = (uint16_t) (ctrl >> 16);
    }

    /**
     * @brief	Read a single byte data from the UART peripheral
     * @param	pUART	: Pointer to selected UART peripheral
     * @return	A single byte of data read
     * @note	This function reads a byte from the UART receive FIFO or
     *			receive hold register regard regardless of UART state. The
     *			FIFO status should be read first prior to using this function
     */
    STATIC INLINE uint32_t Chip_VFIFO_ReadSPIData(LPC_VFIFO_T *pVFIFO, uint32_t spi_num) {
        /* get the data portion only. */
        return (uint32_t) (pVFIFO->SPI[spi_num].RXDATSPI & 0x0000FFFF);
    }

    /**
     * @brief	Get the VFIFO SPI interrupt status
     * @param	pVFIFO	: The base of VFIFO peripheral on the chip
     * @return	The Interrupt status register of UART
     * @note	Multiple interrupts may be pending.
     */
    STATIC INLINE uint32_t Chip_VFIFO_GetSPIIntStatus(LPC_VFIFO_T *pVFIFO, uint32_t spi_num) {
        return pVFIFO->SPI[spi_num].INTSTATSPI;
    }

    /**
     * @brief	Get the VFIFO SPI status register
     * @param	pVFIFO	: The base of VFIFO peripheral on the chip
     * @return	VFIFO SPI status register
     * @note	Multiple status may be pending.
     */
    STATIC INLINE uint32_t Chip_VFIFO_GetSPIStatus(LPC_VFIFO_T *pVFIFO, uint32_t spi_num) {
        return pVFIFO->SPI[spi_num].STATSPI;
    }

    /**
     * @brief	Clear the VFIFO SPI status register
     * @param	pVFIFO	: Pointer to selected VFIFOx peripheral
     * @param	stsMask	: OR'ed statuses to disable
     * @return	Nothing
     * @note	Some of the status bits are RO and can't be cleared.
     */
    STATIC INLINE void Chip_VFIFO_ClearSPIStatus(LPC_VFIFO_T *pVFIFO, uint32_t spi_num, uint32_t statusMask) {
        if (statusMask & VFIFO_ST_RXTIMEOUT)
            pVFIFO->SPI[spi_num].STATSPI = VFIFO_ST_RXTIMEOUT;
        if (statusMask & VFIFO_ST_BUSERROR)
            pVFIFO->SPI[spi_num].STATSPI = VFIFO_ST_BUSERROR;
    }

    /**
     * @brief	Initialize the VFIFO peripheral
     * @param	pUART	: The base of VFIFO peripheral on the chip
     * @return	Nothing
     */
    void Chip_VFIFO_Init(LPC_VFIFO_T *pVFIFO);

    /**
     * @brief	Deinitialize the VFIFO peripheral
     * @param	pUART	: The base of VFIFO peripheral on the chip
     * @return	Nothing
     */
    void Chip_VFIFO_DeInit(LPC_VFIFO_T *pVFIFO);

    /******************************** 
     * EXPORTED FUNCTIONS PROTOTYPES *
     *********************************/
    //--init functions--//
    uint32_t Chip_VFIFO_InitUART(LPC_VFIFO_T *pVFIFO, VFIFO_UART_DRIVER_T *uart_setup);
    uint32_t Chip_VFIFO_InitSPI(LPC_VFIFO_T *pVFIFO, VFIFO_SPI_DRIVER_T *spi_setup);
    uint8_t Chip_VFIFO_ConfigUART(LPC_VFIFO_T *pVFIFO, VFIFO_UART_DRIVER_T *uart_setup);
    uint8_t Chip_VFIFO_ConfigSPI(LPC_VFIFO_T *pVFIFO, VFIFO_SPI_DRIVER_T *spi_setup);

    uint32_t Chip_VFIFO_SendUART_RB(LPC_VFIFO_T *pVFIFO, uint32_t uart_num, RINGBUFF_T *pRB, const void *data, int count);
    int Chip_VFIFO_ReceiveUART_RB(LPC_VFIFO_T *pVFIFO, RINGBUFF_T *pRB, void *data, int bytes);

    Status Chip_VFIFO_SPI_TransferFrame(LPC_VFIFO_T *pVFIFO, uint32_t spi_num, SPI_DATA_SETUP_T *pXfSetup);

    /**
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif /* __VFIFO_540XX_H_ */


