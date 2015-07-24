/***
 *file: spi_flash_api.c
 *
 *implements spi_flash_platform_interface.h, which in driver/spi_flash.
 *if need to use spi flash, must do it.
 *
 *created by JerryYu @2014-DEC-03
 *Ver 0.1
 * */ 

#include "spi_flash_platform_interface.h"
#include "chip.h" //TBD! Jer

//#define SPIFLASHDEBUG 1
// Initial SPI Flash
//#define SPIMASTERIRQHANDLER             SPI1_IRQHandler
#define LPC_SPIMASTERPORT               LPC_SPI1
#define LPC_SPIMASTERIRQNUM             SPI1_IRQn
/* The slave can handle data up to the point of overflow or underflow.
   Adjust this clock rate and the master's timing delays to get a working
   SPI slave rate. */
#define LPCMASTERCLOCKRATE              (4000000)
#define BUFFER_SIZE 300

#define SPI_FLASH_CMD_READ_DEVID        0x90
#define SPI_FLASH_CMD_WRITE_ENABLE      0x06
#define SPI_FLASH_CMD_WRITE_DISABLE     0x04

#define SPI_FLASH_CMD_READ_DATA         0x03
#define SPI_FLASH_CMD_READ_FAST         0x0B

#define SPI_FLASH_CMD_WRITE_DATA        0x02
#define SPI_FLASH_CMD_ERASE             0x20
#define SPI_FLASH_CMD_ERASE_32KB        0x52
#define SPI_FLASH_CMD_ERASE_64KB        0xD8
#define SPI_FLASH_CMD_ERASE_CHIP        0xC7

#define SPI_FLASH_CMD_WRITE_STATUS      0x01
#define SPI_FLASH_CMD_READ_STATUS1      0x05
#define SPI_FLASH_CMD_READ_STATUS2      0x35

#define SPI_FLASH_PAGESIZE              256  

volatile uint8_t Flash_mEnd;
/* Master transmit and receive buffers */
uint8_t masterRXBuffer8[BUFFER_SIZE], masterTXBuffer8[BUFFER_SIZE];


static void flash_spi_pinmux_init(void)
{
  /* Connect the SPI1 signals to port pins */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 6 ,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_SCK */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 16,  (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN));	/* SPI1_MISO */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 7 ,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_MOSI */
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8 ,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_SSEL0 */
 
//  Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 15,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
//  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 15);
//  Chip_GPIO_SetPinState(LPC_GPIO, 1, 15, 1);
  
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 6);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, 0);     /* SPI1_SCK */
  
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 16);    /* SPI1_MISO */
  Chip_GPIO_SetPinState(LPC_GPIO, 1, 16, 0);
  
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 7);    /* SPI1_MOSI */
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 1);
  
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 8);/* SPI1_SSEL0 */
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, 1);
}

#define SPI_FLASH_CS_LOW         Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, 0)
#define SPI_FLASH_CS_HIGH        Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, 1)

#define SPI_FLASH_MISO_LOW
#define SPI_FLASH_MISO_HIGH

#define SPI_FLASH_MISO_GET      Chip_GPIO_GetPinState(LPC_GPIO, 1, 16)

#define SPI_FLASH_MOSI_LOW       Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 0)
#define SPI_FLASH_MOSI_HIGH      Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 1)

#define SPI_FLASH_SCK_LOW       Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, 0)
#define SPI_FLASH_SCK_HIGH      Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, 1)

uint8_t spi_wr(uint8_t data)
{
  uint32_t i = 0;
  uint8_t ret = 0;
  
  for(i=0; i<8; i++) {
    ret = ((ret<<1)&0xFE);
    SPI_FLASH_SCK_LOW;
    if(data&0x80) {
      SPI_FLASH_MOSI_HIGH;
    }
    else{
      SPI_FLASH_MOSI_LOW;
    }
    SPI_FLASH_SCK_HIGH;
    data = (data<<1);
    if((SPI_FLASH_MISO_GET&0x01) == 1) {
      ret = ret|0x01;
    }
    else {
      ret &= 0xFE;
    }
  }

  return ret;
}

void flash_spi_master_set(void)
{
  SPI_FLASH_CS_HIGH;
  SPI_FLASH_SCK_HIGH;
}

static uint8_t spi_flash_init(void)
{
  uint8_t ret = 0;
  
   /* SPI initialization */
  flash_spi_pinmux_init();
  /* Enable SysTick Timer */
  /* Setup SPI controllers */
  flash_spi_master_set();
  SPI_FLASH_CS_LOW;
  SPI_FLASH_CS_HIGH;
  SPI_FLASH_SCK_LOW;
  SPI_FLASH_SCK_HIGH;
  SPI_FLASH_MOSI_LOW;
  SPI_FLASH_MOSI_HIGH;
  
  ret = 1;
  return ret;
}


int sflash_platform_init( int peripheral_id, void** platform_peripheral_out )
{
  uint8_t temp1, temp2;
  uint32_t j;
  spi_flash_init();
  
  SPI_FLASH_CS_LOW;
  temp1 = spi_wr(0x90);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp2 = spi_wr(0x00);
  SPI_FLASH_CS_HIGH;
  for(j=0; j<5000; j++);
  printf("Flash ID1 %x, %x, %x\r\n", temp1, temp2, j);
 
  SPI_FLASH_CS_LOW;
  temp1 = spi_wr(0x90);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp1 = spi_wr(0x00);
  temp2 = spi_wr(0x00);
  SPI_FLASH_CS_HIGH;
  for(j=0; j<5000; j++);
  printf("Flash ID2 %x, %x. %x\r\n", temp1, temp2, j);
  return 0;
}

volatile uint8_t spi_flash_recv;
int sflash_platform_send_recv_byte( void* platform_peripheral, unsigned char MOSI_val, void* MISO_addr )
{
//  int ret;
  uint8_t *rcv_buf;
  if(MISO_addr != 0x00000000)
  rcv_buf = MISO_addr;
  else
  rcv_buf = (uint8_t *)&spi_flash_recv;
  *rcv_buf = spi_wr(MOSI_val);
#ifdef SPIFLASHDEBUG
  printf("SPI RW %x %d, %x\r\n", MOSI_val, 1, *rcv_buf);
#endif
  return 0;
}

int sflash_platform_chip_select( void* platform_peripheral )
{
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 8 , 0);
  return 0;
}

int sflash_platform_chip_deselect( void* platform_peripheral )
{
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 8 , 1);
  return 0;
}


