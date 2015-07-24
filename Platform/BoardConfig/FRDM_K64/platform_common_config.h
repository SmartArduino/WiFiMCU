/**
******************************************************************************
* @file    platform_common_config.h
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provides common configuration for current platform.
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

#pragma once

/******************************************************
*                      Macros
******************************************************/

/******************************************************
*                    Constants
******************************************************/

/* The clock configuration utility from ST is used to calculate these values
* http://www.st.com/st-web-ui/static/active/en/st_prod_software_internet/resource/technical/software/utility/stsw-stm32090.zip
* The CPU Clock Frequency (CPU_CLOCK_HZ) is independently defined in <WICED-SDK>/Wiced/Platform/BCM943362WCD4/BCM943362WCD4.mk
*/
#if 0
#define HSE_SOURCE              RCC_HSE_ON               /* Use external crystal                 */
#define AHB_CLOCK_DIVIDER       RCC_SYSCLK_Div1          /* AHB clock = System clock             */
#define APB1_CLOCK_DIVIDER      RCC_HCLK_Div4            /* APB1 clock = AHB clock / 4           */
#define APB2_CLOCK_DIVIDER      RCC_HCLK_Div2            /* APB2 clock = AHB clock / 2           */
#define PLL_SOURCE              RCC_PLLSource_HSE        /* PLL source = external crystal        */
#define PLL_M_CONSTANT          26                       /* PLLM = 26                            */
#define PLL_N_CONSTANT          240                      /* PLLN = 240                           */
#define PLL_P_CONSTANT          2                        /* PLLP = 2                             */
#define PPL_Q_CONSTANT          5                        /* PLLQ = 5                             */
#define SYSTEM_CLOCK_SOURCE     RCC_SYSCLKSource_PLLCLK  /* System clock source = PLL clock      */
#define SYSTICK_CLOCK_SOURCE    SysTick_CLKSource_HCLK   /* SysTick clock source = AHB clock     */
#define INT_FLASH_WAIT_STATE    FLASH_Latency_3          /* Internal flash wait state = 3 cycles */

#define SPI_BUS_CLOCK_BANK      GPIOB
#define SPI_BUS_MISO_BANK       GPIOB
#define SPI_BUS_MOSI_BANK       GPIOB
#define SPI_BUS_CS_BANK         GPIOB
#define SPI_IRQ_BANK            GPIOA
#define SPI_BUS_CLOCK_BANK_CLK  RCC_AHB1Periph_GPIOB
#define SPI_BUS_MISO_BANK_CLK   RCC_AHB1Periph_GPIOB
#define SPI_BUS_MOSI_BANK_CLK   RCC_AHB1Periph_GPIOB
#define SPI_BUS_CS_BANK_CLK     RCC_AHB1Periph_GPIOB
#define SPI_IRQ_CLK             RCC_AHB1Periph_GPIOA
#define SPI_BUS_CLOCK_PIN       13
#define SPI_BUS_MISO_PIN        14
#define SPI_BUS_MOSI_PIN        15
#define SPI_BUS_CS_PIN          12
#define SPI_IRQ_PIN             1

#define SPIX_CLK                RCC_APB1Periph_SPI2
#define SPIX                    SPI2
#define SPIX_AF                 GPIO_AF_SPI2
#define SPIX_DMA_RX_STREAM      DMA1_Stream3
#define SPIX_DMA_TX_STREAM      DMA1_Stream4
#define SPIX_DMA_RX_TCFLAG      DMA_FLAG_TCIF3
#define SPIX_DMA_TX_TCFLAG      DMA_FLAG_TCIF4
#define SPIX_DMA_RX_IRQ_CHANNEL DMA1_Stream3_IRQn
#define SPIX_DMA_RX_TCIT        DMA_IT_TCIF3
#define dma_irq                 DMA1_Stream3_IRQHandler
#endif

#define WL_GPIO1_BANK           HW_GPIOE
#define WL_GPIO1_PIN            24

#define WL_SPI                  (SPI0_BASE_PTR)
#define SPI_BUS_CLOCK_BANK      HW_GPIOD
#define SPI_BUS_MISO_BANK       HW_GPIOD
#define SPI_BUS_MOSI_BANK       HW_GPIOD
#define SPI_BUS_CS_BANK         HW_GPIOD
#define SPI_IRQ_BANK            HW_GPIOE
#define SPI_BUS_CLOCK_PIN       1
#define SPI_BUS_MISO_PIN        3
#define SPI_BUS_MOSI_PIN        2
#define SPI_BUS_CS_PIN          0
#define SPI_IRQ_PIN             25

#define WL_RESET_BANK           HW_GPIOC
#define WL_RESET_PIN            4


/* These are internal platform connections only */
typedef enum
{
  MICO_GPIO_UNUSED = -1,
  MICO_GPIO_WLAN_POWERSAVE_CLOCK = 0,
  WL_GPIO0,
  WL_GPIO1,
  WL_REG_RESERVED,
  WL_RESET,
  MICO_SYS_LED,
  MICO_RF_LED,
  BOOT_SEL,
  MFG_SEL,
  EasyLink_BUTTON,
  MICO_COMMON_GPIO_MAX,
} mico_common_gpio_t;

#define WL_REG   MICO_GPIO_UNUSED

/* How the wlan's powersave clock is connected */
typedef enum
{
  MICO_PWM_WLAN_POWERSAVE_CLOCK,
  MICO_COMMON_PWM_MAX,
} mico_common_pwm_t;

#define MICO_WLAN_POWERSAVE_CLOCK_IS_PWM 0
#define MICO_WLAN_POWERSAVE_CLOCK_IS_MCO 1

#define WLAN_POWERSAVE_CLOCK_FREQUENCY 32768 /* 32768Hz        */
#define WLAN_POWERSAVE_CLOCK_DUTY_CYCLE   50 /* 50% duty-cycle */

#define WL_32K_OUT_BANK         GPIOA
#define WL_32K_OUT_PIN          8
#define WL_32K_OUT_BANK_CLK     RCC_AHB1Periph_GPIOA

/* The number of UART interfaces this hardware platform has */
#define NUMBER_OF_UART_INTERFACES  2

#define UART_FOR_APP     MICO_UART_1
#define STDIO_UART       MICO_UART_1
#define CLI_UART         MICO_UART_2

/* Memory map */
#define INTERNAL_FLASH_START_ADDRESS    (uint32_t)0x00000000
#define INTERNAL_FLASH_END_ADDRESS      (uint32_t)0x000FFFFF
#define INTERNAL_FLASH_SIZE             (INTERNAL_FLASH_END_ADDRESS - INTERNAL_FLASH_START_ADDRESS + 1)

#define SPI_FLASH_START_ADDRESS         (uint32_t)0x00000000
#define SPI_FLASH_END_ADDRESS           (uint32_t)0x000FFFFF
#define SPI_FLASH_SIZE                  (SPI_FLASH_END_ADDRESS - SPI_FLASH_START_ADDRESS + 1)

#define MICO_FLASH_FOR_APPLICATION  MICO_INTERNAL_FLASH
#define APPLICATION_START_ADDRESS   (uint32_t)0x00004000
#define APPLICATION_END_ADDRESS     (uint32_t)0x0005FFFF
#define APPLICATION_FLASH_SIZE      (APPLICATION_END_ADDRESS - APPLICATION_START_ADDRESS + 1) /* 368k bytes*/

#define MICO_FLASH_FOR_UPDATE       MICO_SPI_FLASH
#define UPDATE_START_ADDRESS        (uint32_t)0x00040000
#define UPDATE_END_ADDRESS          (uint32_t)0x0009FFFF
#define UPDATE_FLASH_SIZE           (UPDATE_END_ADDRESS - UPDATE_START_ADDRESS + 1) /* 384k bytes*/

#define MICO_FLASH_FOR_BOOT         MICO_INTERNAL_FLASH
#define BOOT_START_ADDRESS          (uint32_t)0x00000000
#define BOOT_END_ADDRESS            (uint32_t)0x00003FFF
#define BOOT_FLASH_SIZE             (BOOT_END_ADDRESS - BOOT_START_ADDRESS + 1) /* 16k bytes*/

#define MICO_FLASH_FOR_DRIVER       MICO_SPI_FLASH
#define DRIVER_START_ADDRESS        (uint32_t)0x00002000
#define DRIVER_END_ADDRESS          (uint32_t)0x0003FFFF
#define DRIVER_FLASH_SIZE           (DRIVER_END_ADDRESS - DRIVER_START_ADDRESS + 1) /* 248k bytes*/

#define MICO_FLASH_FOR_PARA         MICO_INTERNAL_FLASH
#define PARA_START_ADDRESS          (uint32_t)0x000FD000
#define PARA_END_ADDRESS            (uint32_t)0x000FDFFF
#define PARA_FLASH_SIZE             (PARA_END_ADDRESS - PARA_START_ADDRESS + 1)   /* 4k bytes*/

#define MICO_FLASH_FOR_EX_PARA      MICO_INTERNAL_FLASH
#define EX_PARA_START_ADDRESS       (uint32_t)0x000FE000
#define EX_PARA_END_ADDRESS         (uint32_t)0x000FEFFF
#define EX_PARA_FLASH_SIZE          (EX_PARA_END_ADDRESS - EX_PARA_START_ADDRESS + 1)   /* 4k bytes*/

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
*                 Global Variables
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/
