/***
 *file: platform_wifi_config.h
 *
 *configurations of the wifi module.
 *configs the SDIO/SPI, reset, wakeup, IRQ, etc.
 *
 * created by Jerry Yu @2014-Dec-03
 *Ver 0.1
 *
 *
 * */

#pragma once

/**===wlan 32K SLeep clock In=====*/
#define GPIO_WLCLK_PORT	    0  
#define GPIO_WLCLK_PIN		21 
/**===wlan wakeup host=====*/
#define GPIO_GPOWKE_PORT	0  
#define GPIO_GPOWKE_PIN		22 
/**=====wifi power port========**/
// #define GPIO_GPOPWD_PORT	0  
// #define GPIO_GPOPWD_PIN		3 
/**=====wifi reset port========**/
#define GPIO_GPORST_PORT	0
#define GPIO_GPORST_PIN		3 
/**====wifi spi port======*/
#define WIFI_SPI_PORTNUM1   1
#define ATH_SPI_DMA         1 //DMA mode for SPI_wifi

#define GPIO_SSPSEL_PORT	1
#define GPIO_SSPSEL_PIN		15

#define GPIO_SSPCLK_PORT	1
#define GPIO_SSPCLK_PIN		6

#define GPIO_SSPMISO_PORT   1
#define GPIO_SSPMISO_PIN	14

#define GPIO_SSPMOSI_PORT	1
#define GPIO_SSPMOSI_PIN	7
/**=====wifi irq==== */
#define GPIO_SPIINT_PORT    0
#define GPIO_SPIINT_PIN		4
#define GPIO_SPIINT_INDEX	PININTSELECT0	/* PININT index used for GPIO mapping */
/**=====WLAN Power save configs=====*/
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

