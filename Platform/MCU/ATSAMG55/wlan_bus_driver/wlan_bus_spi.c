/**
******************************************************************************
* @file    wlan_bus_spi.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provides SPI bus protocol for RF chip.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/ 

#include "MICORTOS.h"

#include "string.h" /* for memcpy */
#include "platform.h"
#include "platform_peripheral.h"
#include "platformLogging.h"

#include "MicoRtos.h"
#include "platform_config.h"
#include "platform_peripheral.h"
#include "wlan_platform_common.h"


/******************************************************
*             constants
******************************************************/

/* Clock polarity. */
#define SPI_CLK_POLARITY    0

/* Clock phase. */
#define SPI_CLK_PHASE       1

#define SPI_BAUD_RATE        50000000

/**
* transfer direction for the mico platform bus interface
*/
typedef enum
{
  /* if updating this enum, the bus_direction_mapping variable will also need to be updated */
  BUS_READ,
  BUS_WRITE
} bus_transfer_direction_t;

/******************************************************
*             structures
******************************************************/

/******************************************************
*             variables
******************************************************/
static IRQn_Type platform_flexcom_irq_numbers[] =
{
  [0]  = FLEXCOM0_IRQn,
  [1]  = FLEXCOM1_IRQn,
  [2]  = FLEXCOM2_IRQn,
  [3]  = FLEXCOM3_IRQn,
  [4]  = FLEXCOM4_IRQn,
  [5]  = FLEXCOM5_IRQn,
  [6]  = FLEXCOM6_IRQn,
  [7]  = FLEXCOM7_IRQn,
};

static Flexcom  *flexcom_base[] = 
{
  [0]  = FLEXCOM0,
  [1]  = FLEXCOM1,
  [2]  = FLEXCOM2,
  [3]  = FLEXCOM3,
  [4]  = FLEXCOM4,
  [5]  = FLEXCOM5,
  [6]  = FLEXCOM6,
  [7]  = FLEXCOM7,
};
static mico_semaphore_t spi_transfer_finished_semaphore;

/******************************************************
*             function declarations
******************************************************/

extern void wiced_platform_notify_irq( void );

/******************************************************
*             function definitions
******************************************************/

static void spi_irq_handler( void* arg )
{
  UNUSED_PARAMETER(arg);
#ifndef MICO_DISABLE_MCU_POWERSAVE
  platform_mcu_powersave_exit_notify( );
#endif /* ifndef MICO_DISABLE_MCU_POWERSAVE */
  wiced_platform_notify_irq( );
}


OSStatus host_platform_bus_init( void )
{
  pdc_packet_t  pdc_spi_packet;
  Pdc*          spi_pdc  = spi_get_pdc_base( wifi_spi.port );
  
  platform_mcu_powersave_disable( );
  
  mico_rtos_init_semaphore( &spi_transfer_finished_semaphore, 1 );
  
  /* Setup the SPI lines */
  platform_gpio_peripheral_pin_init(  wifi_spi.mosi_pin,  ( wifi_spi.mosi_pin_mux_mode | IOPORT_MODE_PULLUP ) );
  platform_gpio_peripheral_pin_init(  wifi_spi.miso_pin,  ( wifi_spi.miso_pin_mux_mode | IOPORT_MODE_PULLUP ) );
  platform_gpio_peripheral_pin_init(  wifi_spi.clock_pin, ( wifi_spi.clock_pin_mux_mode | IOPORT_MODE_PULLUP ) );
  
  /* Setup the interrupt input for WLAN_IRQ */
  platform_gpio_init( &wifi_spi_pins[WIFI_PIN_SPI_IRQ], INPUT_HIGH_IMPEDANCE );
  platform_gpio_irq_enable( &wifi_spi_pins[WIFI_PIN_SPI_IRQ], IRQ_TRIGGER_RISING_EDGE, spi_irq_handler, 0 );
  
  /* Setup SPI slave select GPIOs */
  platform_gpio_init( &wifi_spi_pins[WIFI_PIN_SPI_CS], OUTPUT_PUSH_PULL );
  platform_gpio_output_high( &wifi_spi_pins[WIFI_PIN_SPI_CS] );
  
#if defined ( MICO_WIFI_USE_GPIO_FOR_BOOTSTRAP )
  /* Set GPIO_B[1:0] to 01 to put WLAN module into gSPI mode */
  platform_gpio_init( &wifi_control_pins[WIFI_PIN_BOOTSTRAP_0], OUTPUT_PUSH_PULL );
  platform_gpio_output_high( &wifi_control_pins[WIFI_PIN_BOOTSTRAP_0] );
  platform_gpio_init( &wifi_control_pins[WIFI_PIN_BOOTSTRAP_1], OUTPUT_PUSH_PULL );
  platform_gpio_output_low( &wifi_control_pins[WIFI_PIN_BOOTSTRAP_1] );
#endif
  
  /* Enable the peripheral and set SPI mode. */
  flexcom_enable( flexcom_base[ wifi_spi.spi_id ] );
  flexcom_set_opmode( flexcom_base[ wifi_spi.spi_id ], FLEXCOM_SPI );
  
  /* Init pdc, and clear RX TX. */
  pdc_spi_packet.ul_addr = 0;
  pdc_spi_packet.ul_size = 1;
  pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );
  pdc_rx_init( spi_pdc, &pdc_spi_packet, NULL );
  
  spi_disable_interrupt(wifi_spi.port, 0xffffffff );
  spi_disable( wifi_spi.port );
  spi_reset( wifi_spi.port );
  spi_set_lastxfer( wifi_spi.port );
  spi_set_master_mode( wifi_spi.port );
  spi_disable_mode_fault_detect( wifi_spi.port );
  
  spi_set_clock_polarity( wifi_spi.port, 0, SPI_CLK_POLARITY );
  spi_set_clock_phase( wifi_spi.port, 0, SPI_CLK_PHASE );
  spi_set_bits_per_transfer( wifi_spi.port, 0, SPI_CSR_BITS_8_BIT );
  spi_set_baudrate_div( wifi_spi.port, 0, (sysclk_get_cpu_hz() / SPI_BAUD_RATE) );
  spi_set_transfer_delay( wifi_spi.port, 0, 0, 0 );
  
  /* Must be lower priority than the value of configMAX_SYSCALL_INTERRUPT_PRIORITY */
  /* otherwise FreeRTOS will not be able to mask the interrupt */
  /* keep in mind that ARMCM4 interrupt priority logic is inverted, the highest value */
  /* is the lowest priority */
  /* Configure SPI interrupts . */
  
  NVIC_EnableIRQ( platform_flexcom_irq_numbers[wifi_spi.spi_id] );
  
  spi_enable(wifi_spi.port);
  
  platform_mcu_powersave_enable( );
  
  return kNoErr;
}

OSStatus host_platform_bus_deinit( void )
{
  platform_mcu_powersave_disable( );
  
  NVIC_DisableIRQ( platform_flexcom_irq_numbers[wifi_spi.spi_id] );
  pdc_disable_transfer( spi_get_pdc_base( wifi_spi.port ), PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS );
  spi_disable(wifi_spi.port);
  
  /* Deinit the SPI lines */
  platform_gpio_peripheral_pin_init(  wifi_spi.mosi_pin,  INPUT_HIGH_IMPEDANCE );
  platform_gpio_peripheral_pin_init(  wifi_spi.miso_pin,  INPUT_HIGH_IMPEDANCE );
  platform_gpio_peripheral_pin_init(  wifi_spi.clock_pin, INPUT_HIGH_IMPEDANCE );
  
  /* Deinit the interrupt input for WLAN_IRQ */
  platform_gpio_init( &wifi_spi_pins[WIFI_PIN_SPI_IRQ], INPUT_HIGH_IMPEDANCE );
  platform_gpio_irq_disable( &wifi_spi_pins[WIFI_PIN_SPI_IRQ] );
  
  /* Deinit SPI slave select GPIOs */
  platform_gpio_init( &wifi_spi_pins[WIFI_PIN_SPI_CS], INPUT_HIGH_IMPEDANCE );
  
#if defined ( MICO_WIFI_USE_GPIO_FOR_BOOTSTRAP )
  platform_gpio_init( &wifi_control_pins[WIFI_PIN_BOOTSTRAP_0], INPUT_HIGH_IMPEDANCE );
  platform_gpio_init( &wifi_control_pins[WIFI_PIN_BOOTSTRAP_1], INPUT_HIGH_IMPEDANCE );
#endif
  
  platform_mcu_powersave_enable( );
  
  return kNoErr;
}

OSStatus host_platform_spi_transfer( bus_transfer_direction_t dir, uint8_t* buffer, uint16_t buffer_length )
{
  OSStatus result;
  
  pdc_packet_t pdc_spi_packet = { (uint32_t)buffer, buffer_length };
  Pdc* spi_pdc  = spi_get_pdc_base( wifi_spi.port );
  
  platform_mcu_powersave_disable();
  
  platform_gpio_output_low( &wifi_spi_pins[WIFI_PIN_SPI_CS] );
  
  pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL);
  
  if ( dir == BUS_READ ) {
    pdc_rx_init( spi_pdc, &pdc_spi_packet, NULL);
    
    spi_enable_interrupt(wifi_spi.port, SPI_IER_RXBUFF );
    pdc_enable_transfer( spi_pdc, PERIPH_PTCR_TXTEN | PERIPH_PTCR_RXTEN );
  } 
  
  if ( dir == BUS_WRITE ) {
    spi_enable_interrupt( wifi_spi.port, SPI_IER_ENDTX );
    pdc_enable_transfer( spi_pdc, PERIPH_PTCR_TXTEN );
  }
  
  result = mico_rtos_get_semaphore( &spi_transfer_finished_semaphore, 100 );
  
  platform_gpio_output_high( &wifi_spi_pins[WIFI_PIN_SPI_CS] );
  
  platform_mcu_powersave_enable();
  
  return result;
  
}

void platform_wifi_spi_rx_dma_irq(void)
{
  uint8_t junk1;
  uint16_t junk2;
  pdc_packet_t pdc_spi_packet = { 0, 1 };
  Pdc* spi_pdc  = spi_get_pdc_base( wifi_spi.port );
  
  uint32_t status = spi_read_status( wifi_spi.port );
  uint32_t mask = spi_read_interrupt_mask( wifi_spi.port );
  
  if ( ( mask & SPI_IMR_RXBUFF ) && ( status & SPI_SR_RXBUFF ) )
  {
    pdc_disable_transfer( spi_pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS );
    pdc_rx_init( spi_pdc, &pdc_spi_packet, NULL );
    pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );
    spi_disable_interrupt( wifi_spi.port, SPI_IER_RXBUFF );
  }
  
  if ( ( mask & SPI_IMR_ENDTX ) && ( status & SPI_SR_ENDTX ) )
  {
    pdc_disable_transfer( spi_pdc, PERIPH_PTCR_TXTDIS );
    pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );
    spi_disable_interrupt( wifi_spi.port, SPI_IER_ENDTX );
    /* Clear SPI RX data in a SPI send sequence */
    spi_read( wifi_spi.port, &junk2, &junk1);
  }
  
  mico_rtos_set_semaphore( &spi_transfer_finished_semaphore );
}

