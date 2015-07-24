/**
 * External library
 */

#ifndef __LEXLIBS_H__
#define __LEXLIBS_H__

/*uncomment if not use the lib*/
#define LUA_USE_GPIOLIB
#define LUA_USE_MCULIB
#define LUA_USE_TMRLIB
#define LUA_USE_WIFILIB
#define LUA_USE_NETLIB
#define LUA_USE_FILELIB
#define LUA_USE_PWMLIB
#define LUA_USE_UARTLIB
#define LUA_USE_ADCLIB
#define LUA_USE_I2CLIB
#define LUA_USE_SPILIB

/* gpio */
#if defined(LUA_USE_GPIOLIB)
#define EXLIB_GPIO       "gpio"
#define ROM_EXLIB_GPIO   \
    _ROM(EXLIB_GPIO, luaopen_gpio, gpio_map)
#else
#define ROM_EXLIB_GPIO
#endif
/* mcu */
#if defined(LUA_USE_MCULIB)
#define EXLIB_MCU       "mcu"
#define ROM_EXLIB_MCU   \
    _ROM(EXLIB_MCU, luaopen_mcu, mcu_map)
#else
#define EXLIB_MCU
#endif
/* tmr */
#if defined(LUA_USE_TMRLIB)
#define EXLIB_TMR       "tmr"
#define ROM_EXLIB_TMR   \
    _ROM(EXLIB_TMR, luaopen_tmr, tmr_map)
#else
#define EXLIB_TMR
#endif

/* wifi */
#if defined(LUA_USE_WIFILIB)
#define EXLIB_WIFI       "wifi"
#define ROM_EXLIB_WIFI   \
    _ROM(EXLIB_WIFI, luaopen_wifi, wifi_map)
#else
#define EXLIB_WIFI
#endif

/* net */
#if defined(LUA_USE_NETLIB)
#define EXLIB_NET       "net"
#define ROM_EXLIB_NET   \
    _ROM(EXLIB_NET, luaopen_net, net_map)
#else
#define EXLIB_NET
#endif

/* file */
#if defined(LUA_USE_FILELIB)
#define EXLIB_FILE       "file"
#define ROM_EXLIB_FILE   \
    _ROM(EXLIB_FILE, luaopen_file, file_map)
#else
#define EXLIB_FILE
#endif
      
/* pwm */
#if defined(LUA_USE_PWMLIB)
#define EXLIB_PWM       "pwm"
#define ROM_EXLIB_PWM   \
    _ROM(EXLIB_PWM, luaopen_pwm, pwm_map)
#else
#define EXLIB_PWM
#endif
      
/* uart */
#if defined(LUA_USE_UARTLIB)
#define EXLIB_UART       "uart"
#define ROM_EXLIB_UART   \
    _ROM(EXLIB_UART, luaopen_uart, uart_map)
#else
#define EXLIB_UART
#endif
      
/* adc */
#if defined(LUA_USE_ADCLIB)
#define EXLIB_ADC       "adc"
#define ROM_EXLIB_ADC   \
    _ROM(EXLIB_ADC, luaopen_adc, adc_map)
#else
#define EXLIB_ADC
#endif
      
/* i2c */
#if defined(LUA_USE_I2CLIB)
#define EXLIB_I2C       "i2c"
#define ROM_EXLIB_I2C   \
    _ROM(EXLIB_I2C, luaopen_i2c, i2c_map)
#else
#define EXLIB_I2C
#endif

/* spi */
#if defined(LUA_USE_SPILIB)
#define EXLIB_SPI       "spi"
#define ROM_EXLIB_SPI   \
    _ROM(EXLIB_SPI, luaopen_spi, spi_map)
#else
#define EXLIB_SPI
#endif
      
/* libs */
#define LUA_EXLIBS_ROM  \
        ROM_EXLIB_GPIO  \
        ROM_EXLIB_MCU   \
        ROM_EXLIB_TMR   \
        ROM_EXLIB_WIFI  \
        ROM_EXLIB_NET   \
        ROM_EXLIB_FILE  \
        ROM_EXLIB_PWM   \
        ROM_EXLIB_UART  \
        ROM_EXLIB_ADC   \
        ROM_EXLIB_I2C   \
        ROM_EXLIB_SPI

//others
#define MOD_REG_NUMBER( L, name, val )\
  lua_pushnumber( L, val );\
  lua_setfield( L, -2, name )
    
#define MOD_REG_LUDATA( L, name, val )\
  lua_pushlightuserdata( L, val );\
  lua_setfield( L, -2, name )
    
#define MOD_CHECK_ID( mod, id )\
  if( !platform_ ## mod ## _exists( id ) )\
    return luaL_error( L, #mod" %d does not exist", ( unsigned )id )    

#endif

