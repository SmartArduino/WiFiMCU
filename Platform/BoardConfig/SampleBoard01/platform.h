/****
 *filename: platform.h
 * the board's Peripherals define and configurations, such as gpio, button, LED, spi, uart, pwm,
 *adc,i2c,rtc, etc., not include config the WIFI module.
 *
 *Jerry Yu created.
 *Ver 0.1 2014-DEC-03
 * */ 

#include "platform_common_config.h"
#include "platform_wifi_config.h"   //option if config wifi in here or platform_common_config.h
#include "board.h"                  //can also use implemented configurations. Like this "board.h"

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define HARDWARE_REVISION   "1062_1"            //Which Wifi Module? default: 1062_x. 
#define DEFAULT_NAME        "EXAMPLE EVB"       //Named your board/platform.
#define MODEL               "EXAMPLE_EVB_01"    //Named your board model.

#ifdef __GNUC__
#define WEAK __attribute__ ((weak))
#elif defined ( __IAR_SYSTEMS_ICC__ )
#define WEAK __weak
#endif /* ifdef __GNUC__ */

#define BOARD_EXAMPLE_01                        //Want special? define it to a special application.
   
/***
 * Define some port what want to do
 * */ 
typedef enum
{
  MICO_GPIO_UNUSED = -1,                //Reserved to confirm which port unused.
  MICO_GPIO_WLAN_POWERSAVE_CLOCK = 0,   //If wifi module need external clock to sleep mode.
  WL_GPIO0,                             //Define a GPIO to wifi module
 // WL_GPIO1,                           //Commemt it if don't need it.Like this Line
  WL_REG_RESERVED,                      //Wifi module regulator
  WL_RESET,                             //Wifi Reset port 
  MICO_SYS_LED,                         //A LED for system status.
  MICO_RF_LED,                          //RF LED for Wifi
  BOOT_SEL,                             //A switch or button for boot the platform
  MFG_SEL,                              //A switch or button for MFG mode,a test mode
  EasyLink_BUTTON,                      //EasyLink button
  MICO_COMMON_GPIO_MAX,                 //How many port or Gpio defined.
} mico_common_gpio_t;                   //Named the type define.

/**********************
 * Typedef peripherals 
 * Must be defined,if want use MICO Driver to develp applications.
 * Can implement mapping(s), which mentioned in following,in platform.c (recommend to named platform.c)
 * Maybe not all of peripherals wanted,can control it by #macroes HAVE_XXX
 **********************/
#ifdef HAVE_GPIO
/**
 * Define all of the GPIOs will be used besides "mico_common_gpio_t".
 * May need to implement a good gpio mapping[] type/struct used in "MicoDriverGpio.c"
 * */
typedef enum
{
    MICO_GPIO_1 = MICO_COMMON_GPIO_MAX,
    MICO_GPIO_2,
    MICO_GPIO_3,
    MICO_GPIO_4,
    MICO_GPIO_5,
    MICO_GPIO_6,
    MICO_GPIO_7,
    MICO_GPIO_8,
    MICO_GPIO_9,
    MICO_GPIO_10,
    MICO_GPIO_11,
    MICO_GPIO_12,
    MICO_GPIO_13,
    MICO_GPIO_14,
    MICO_GPIO_15,
    MICO_GPIO_16,
    MICO_GPIO_17,
    MICO_GPIO_18,
    MICO_GPIO_19,
    MICO_GPIO_20,
    MICO_GPIO_21,
    MICO_GPIO_22,
    MICO_GPIO_23,
    MICO_GPIO_24,
    MICO_GPIO_25,
    MICO_GPIO_26,
    MICO_GPIO_27,
    MICO_GPIO_28,
    MICO_GPIO_29,
    MICO_GPIO_30,
    MICO_GPIO_MAX, /* Denotes the total number of GPIO port aliases. Not a valid GPIO alias */
} mico_gpio_t;
#endif 

#ifdef HAVE_SPI
/**
 * Define all of the SPIs will be used .
 * May need to implement a good SPI mapping[] type/struct used in "MicoDriverSpi.c"
 * */
typedef enum
{
    MICO_SPI_1,
    MICO_SPI_MAX, /* Denotes the total number of SPI port aliases. Not a valid SPI alias */
} mico_spi_t;
#endif 

#ifdef HAVE_I2C
/**
 * Define all of the I2Cs will be used .
 * May need to implement a good I2C mapping[] type/struct used in "MicoDriverI2c.c"
 * */
typedef enum
{
    MICO_I2C_1,
    MICO_I2C_MAX, /* Denotes the total number of I2C port aliases. Not a valid I2C alias */
} mico_i2c_t;
#endif 

#ifdef HAVE_PWM
/**
 * Define all of the PWMs will be used .
 * May need to implement a good PWM mapping[] type/struct used in "MicoDriverPwm.c"
 * */
typedef enum
{
    MICO_PWM_1 = MICO_COMMON_PWM_MAX,
    MICO_PWM_2,
    MICO_PWM_3,
    MICO_PWM_MAX, /* Denotes the total number of PWM port aliases. Not a valid PWM alias */
} mico_pwm_t;
#endif 

#ifdef HAVE_ADC
/**
 * Define all of the ADCs will be used .
 * May need to implement a good ADC mapping[] type/struct used in "MicoDriverAdc.c"
 * */
typedef enum
{
    MICO_ADC_1,
    MICO_ADC_2,
    MICO_ADC_3,
    MICO_ADC_MAX, /* Denotes the total number of ADC port aliases. Not a valid ADC alias */
} mico_adc_t;
#endif 

#ifdef HAVE_UART
/**
 * Define all of the UARTs will be used .
 * May need to implement a good uart mapping[] type/struct used in "MicoDriverUart.c"
 * */
typedef enum
{
    MICO_UART_1,
    MICO_UART_2,
    MICO_UART_MAX, /* Denotes the total number of UART port aliases. Not a valid UART alias */
} mico_uart_t;

/* The number of UART interfaces this hardware platform has */
#define NUMBER_OF_UART_INTERFACES   2               //How many UARTs will use.
#define STDIO_UART                  MICO_UART_1     //Which UART used as STDIO.
#endif 

#ifdef HAVE_FLASH
/**
 * Define all of the FLASHs will be used .
 * May need to implement a good flash mapping[] type/struct used in "MicoDriverFlash.c"
 * */
typedef enum
{
  MICO_SPI_FLASH,
  MICO_INTERNAL_FLASH,
} mico_flash_t;

#define USE_MICO_SPI_FLASH              //Use Mico driver of spi_flash. In Common/Drivers/spi_flash/
//#define SFLASH_SUPPORT_MACRONIX_PART 
//#define SFLASH_SUPPORT_SST_PARTS
#define SFLASH_SUPPORT_WINBOND_PARTS
#endif 


/**============== Memory map================== */
#define INTERNAL_FLASH_START_ADDRESS    (uint32_t)0x08000000
#define INTERNAL_FLASH_END_ADDRESS      (uint32_t)0x080FFFFF
#define INTERNAL_FLASH_SIZE             (INTERNAL_FLASH_END_ADDRESS - INTERNAL_FLASH_START_ADDRESS + 1)

#define SPI_FLASH_START_ADDRESS         (uint32_t)0x00000000
#define SPI_FLASH_END_ADDRESS           (uint32_t)0x000FFFFF
#define SPI_FLASH_SIZE                  (SPI_FLASH_END_ADDRESS - SPI_FLASH_START_ADDRESS + 1)

#define MICO_FLASH_FOR_APPLICATION  MICO_INTERNAL_FLASH
#define APPLICATION_START_ADDRESS   (uint32_t)0x08004000
#define APPLICATION_END_ADDRESS     (uint32_t)0x0805FFFF
#define APPLICATION_FLASH_SIZE      (APPLICATION_END_ADDRESS - APPLICATION_START_ADDRESS + 1) /* 368k bytes*/

#define MICO_FLASH_FOR_UPDATE       MICO_SPI_FLASH
#define UPDATE_START_ADDRESS        (uint32_t)0x00040000
#define UPDATE_END_ADDRESS          (uint32_t)0x0009FFFF
#define UPDATE_FLASH_SIZE           (UPDATE_END_ADDRESS - UPDATE_START_ADDRESS + 1) /* 384k bytes*/

#define MICO_FLASH_FOR_BOOT         MICO_INTERNAL_FLASH
#define BOOT_START_ADDRESS          (uint32_t)0x08000000
#define BOOT_END_ADDRESS            (uint32_t)0x08003FFF
#define BOOT_FLASH_SIZE             (BOOT_END_ADDRESS - BOOT_START_ADDRESS + 1) /* 16k bytes*/

#define MICO_FLASH_FOR_DRIVER       MICO_SPI_FLASH
#define DRIVER_START_ADDRESS        (uint32_t)0x00002000
#define DRIVER_END_ADDRESS          (uint32_t)0x0003FFFF
#define DRIVER_FLASH_SIZE           (DRIVER_END_ADDRESS - DRIVER_START_ADDRESS + 1) /* 248k bytes*/

#define MICO_FLASH_FOR_PARA         MICO_SPI_FLASH
#define PARA_START_ADDRESS          (uint32_t)0x00000000
#define PARA_END_ADDRESS            (uint32_t)0x00000FFF
#define PARA_FLASH_SIZE             (PARA_END_ADDRESS - PARA_START_ADDRESS + 1)   /* 4k bytes*/

#define MICO_FLASH_FOR_EX_PARA      MICO_SPI_FLASH
#define EX_PARA_START_ADDRESS       (uint32_t)0x00001000
#define EX_PARA_END_ADDRESS         (uint32_t)0x00001FFF
#define EX_PARA_FLASH_SIZE          (EX_PARA_END_ADDRESS - EX_PARA_START_ADDRESS + 1)   /* 4k bytes*/

/******************************************************
*               Function Declarations
******************************************************/



#ifdef __cplusplus
} //extern "C"
#endif


