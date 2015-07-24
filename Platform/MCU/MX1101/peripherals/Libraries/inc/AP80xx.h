/**
  ******************************************************************************
  * @file    stm32f2xx.h
  * @author  MCD Application Team
  * @version V1.1.3
  * @date    05-March-2012
  * @brief   CMSIS Cortex-M3 Device Peripheral Access Layer Header File. 
  *          This file contains all the peripheral register's definitions, bits 
  *          definitions and memory mapping for STM32F2xx devices.
  *            
  *          The file is the unique include file that the application programmer
  *          is using in the C source code, usually in main.c. This file contains:
  *           - Configuration section that allows to select:
  *              - The device used in the target application
  *              - To use or not the peripheral’s drivers in application code(i.e.
  *                code will be based on direct access to peripheral’s registers 
  *                rather than drivers API), this option is controlled by 
  *                "#define USE_STDPERIPH_DRIVER"
  *              - To change few application-specific parameters such as the HSE 
  *                crystal frequency
  *           - Data structures and the address mapping for all peripherals
  *           - Peripheral's registers declarations and bits definition
  *           - Macros to access peripheral’s registers hardware
  *  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup stm32f2xx
  * @{
  */
    
#ifndef __STM32F2xx_H
#define __STM32F2xx_H

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */
  
/** @addtogroup Library_configuration_section
  * @{
  */
  
/* Uncomment the line below according to the target STM32 device used in your
   application 
  */

#if !defined (STM32F2XX) 
  #define STM32F2XX
#endif

/*  Tip: To avoid modifying this file each time you need to switch between these
        devices, you can define the device in your toolchain compiler preprocessor.
  */

#if !defined (STM32F2XX)
 #error "Please select first the target STM32F2XX device used in your application (in stm32f2xx.h file)"
#endif

#if !defined  (USE_STDPERIPH_DRIVER)
/**
 * @brief Comment the line below if you will not use the peripherals drivers.
   In this case, these drivers will not be included and the application code will 
   be based on direct access to peripherals registers 
   */
  /*#define USE_STDPERIPH_DRIVER*/
#endif /* USE_STDPERIPH_DRIVER */

/**
 * @brief In the following line adjust the value of External High Speed oscillator (HSE)
   used in your application 
   
   Tip: To avoid modifying this file each time you need to use different HSE, you
        can define the HSE value in your toolchain compiler preprocessor.
  */           
#if !defined  (HSE_VALUE) 
  #define HSE_VALUE    ((uint32_t)26000000) /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

/**
 * @brief In the following line adjust the External High Speed oscillator (HSE) Startup 
   Timeout value 
   */
#if !defined  (HSE_STARTUP_TIMEOUT) 
  #define HSE_STARTUP_TIMEOUT    ((uint16_t)0x0500)   /*!< Time out for HSE start up */
#endif /* HSE_STARTUP_TIMEOUT */

#if !defined  (HSI_VALUE)   
  #define HSI_VALUE    ((uint32_t)16000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

/**
 * @brief STM32F2XX Standard Peripherals Library version number V1.1.3
   */
#define __STM32F2XX_STDPERIPH_VERSION_MAIN   (0x01) /*!< [31:24] main version */                                  
#define __STM32F2XX_STDPERIPH_VERSION_SUB1   (0x01) /*!< [23:16] sub1 version */
#define __STM32F2XX_STDPERIPH_VERSION_SUB2   (0x03) /*!< [15:8]  sub2 version */
#define __STM32F2XX_STDPERIPH_VERSION_RC     (0x00) /*!< [7:0]  release candidate */ 
#define __STM32F2XX_STDPERIPH_VERSION        ((__STM32F2XX_STDPERIPH_VERSION_MAIN << 24)\
                                             |(__STM32F2XX_STDPERIPH_VERSION_SUB1 << 16)\
                                             |(__STM32F2XX_STDPERIPH_VERSION_SUB2 << 8)\
                                             |(__STM32F2XX_STDPERIPH_VERSION_RC))
                                             
/**
  * @}
  */

/** @addtogroup Configuration_section_for_CMSIS
  * @{
  */

/**
 * @brief Configuration of the Cortex-M3 Processor and Core Peripherals 
 */
#define __CM3_REV                 0x0200  /*!< Core Revision r2p0                            */
#define __MPU_PRESENT             1       /*!< STM32F2XX provides an MPU                     */
#define __NVIC_PRIO_BITS          3       /*!< STM32F2XX uses 3 Bits for the Priority Levels */
#define __Vendor_SysTickConfig    0       /*!< Set to 1 if different SysTick Config is used  */

/**
 * @brief STM32F2XX Interrupt Number Definition, according to the selected device 
 *        in @ref Library_configuration_section 
 */
typedef enum IRQn
{
/******  Cortex-M3 Processor Exceptions Numbers ****************************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                          */
  MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M3 Memory Management Interrupt                           */
  BusFault_IRQn               = -11,    /*!< 5 Cortex-M3 Bus Fault Interrupt                                   */
  UsageFault_IRQn             = -10,    /*!< 6 Cortex-M3 Usage Fault Interrupt                                 */
  SVCall_IRQn                 = -5,     /*!< 11 Cortex-M3 SV Call Interrupt                                    */
  DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M3 Debug Monitor Interrupt                              */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M3 Pend SV Interrupt                                    */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M3 System Tick Interrupt                                */
/******  STM32 specific Interrupt Numbers **********************************************************************/
  GPIO_IRQn                   = 0,      /*!< Window WatchDog Interrupt                                         */
  RTC_IRQn                    = 1,      /*!< PVD through EXTI Line detection Interrupt                         */
  IR_IRQn                     = 2,      /*!< Tamper and TimeStamp interrupts through the EXTI line             */
  FUART_IRQn                  = 3,      /*!< RTC Wakeup interrupt through the EXTI line                        */
  BUART_IRQn                  = 4,      /*!< FLASH global Interrupt                                            */
  PWC_IRQn                    = 5,      /*!< RCC global Interrupt                                              */
  TIM0_IRQn                   = 6,      /*!< EXTI Line0 Interrupt                                              */
  USB_IRQn                    = 7,      /*!< EXTI Line1 Interrupt                                              */
  DMA_CH0_IRQn                = 8,      /*!< EXTI Line2 Interrupt                                              */
  DMA_CH1_IRQn                = 9,      /*!< EXTI Line3 Interrupt                                              */
  AUDIO_DECODER_IRQn          = 10,     /*!< EXTI Line4 Interrupt                                              */
  SPIS_IRQn                   = 11,     /*!< DMA1 Stream 0 global Interrupt                                    */
  SPIN_IRQn                   = 12,     /*!< DMA1 Stream 1 global Interrupt                                    */
  SPIM_IRQn                   = 13,     /*!< DMA1 Stream 2 global Interrupt                                    */
  TIM1_IRQn                   = 14,     /*!< DMA1 Stream 3 global Interrupt                                    */
  WATCHDOG_IRQn               = 15     /*!< DMA1 Stream 4 global Interrupt                                    */
  
} IRQn_Type;

/**
  * @}
  */

#include "core_cm3.h"
#include <stdint.h>

/** @addtogroup Exported_types
  * @{
  */  
/*!< STM32F10x Standard Peripheral Library old types (maintained for legacy purpose) */
typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef const int32_t sc32;  /*!< Read Only */
typedef const int16_t sc16;  /*!< Read Only */
typedef const int8_t sc8;   /*!< Read Only */

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;

typedef __I int32_t vsc32;  /*!< Read Only */
typedef __I int16_t vsc16;  /*!< Read Only */
typedef __I int8_t vsc8;   /*!< Read Only */

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */

typedef __IO uint32_t  vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t  vu8;

typedef __I uint32_t vuc32;  /*!< Read Only */
typedef __I uint16_t vuc16;  /*!< Read Only */
typedef __I uint8_t vuc8;   /*!< Read Only */

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;

/**
  * @}
  */

/** @addtogroup Peripheral_registers_structures
  * @{
  */   

/** 
  * @brief Analog to Digital Converter  
  */



/**
  * @}
  */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32F2xx_H */

/**
  * @}
  */

  /**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
