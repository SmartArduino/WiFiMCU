
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define MCU_VERSION	"WiFiMCU 0.9.7"
#define PRT_VERSION "Ver. 0.9.7"
#define BUILD_DATE	"build 20151214"

#define USE_GPIO_MODULE
#define USE_ADC_MODULE
#define USE_MCU_MODULE
#define USE_WIFI_MODULE
#define USE_FILE_MODULE
#define USE_I2C_MODULE
#define USE_NET_MODULE
#define USE_PWM_MODULE
#define USE_SPI_MODULE
#define USE_TMR_MODULE
#define USE_UART_MODULE
#define USE_BIT_MODULE
#define USE_SENSOR_MODULE
#define USE_OLED_MODULE
#define USE_MQTT_MODULE

#define MOD_REG_NUMBER( L, name, val )\
  lua_pushnumber( L, val );\
  lua_setfield( L, -2, name )
    
#define MOD_REG_LUDATA( L, name, val )\
  lua_pushlightuserdata( L, val );\
  lua_setfield( L, -2, name )
    
#define MOD_CHECK_ID( mod, id )\
  if( !platform_ ## mod ## _exists( id ) )\
    return luaL_error( L, #mod" %d does not exist", ( unsigned )id )

#endif	/* __USER_CONFIG_H__ */
