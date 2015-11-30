/*
** $Id: lualib.h,v 1.36.1.1 2007/12/27 13:02:25 roberto Exp $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h

#include "lua.h"
#include "user_config.h"

/* Key to file-handle type */
#define LUA_FILEHANDLE		"FILE*"


#define LUA_COLIBNAME	"coroutine"
LUALIB_API int (luaopen_base) (lua_State *L);

#define LUA_TABLIBNAME	"table"
LUALIB_API int (luaopen_table) (lua_State *L);

#define LUA_IOLIBNAME	"io"
LUALIB_API int (luaopen_io) (lua_State *L);

#define LUA_OSLIBNAME	"os"
LUALIB_API int (luaopen_os) (lua_State *L);

#define LUA_STRLIBNAME	"string"
LUALIB_API int (luaopen_string) (lua_State *L);

#define LUA_MATHLIBNAME	"math"
LUALIB_API int (luaopen_math) (lua_State *L);

#define LUA_DBLIBNAME	"debug"
LUALIB_API int (luaopen_debug) (lua_State *L);

#define LUA_LOADLIBNAME	"package"
LUALIB_API int (luaopen_package) (lua_State *L);

#ifdef USE_ADC_MODULE
#define LUA_ADCLIBNAME	"adc"
LUALIB_API int (luaopen_adc) (lua_State *L);
#endif

#ifdef USE_GPIO_MODULE
#define LUA_GPIOLIBNAME	"gpio"
LUALIB_API int (luaopen_gpio) (lua_State *L);
#endif

#ifdef USE_MCU_MODULE
#define LUA_MCULIBNAME	"mcu"
LUALIB_API int (luaopen_mcu) (lua_State *L);
#endif

#ifdef USE_WIFI_MODULE
#define LUA_WIFILIBNAME	"wifi"
LUALIB_API int (luaopen_wifi) (lua_State *L);
#endif

#ifdef USE_FILE_MODULE
#define LUA_FILELIBNAME	"file"
LUALIB_API int (luaopen_file) (lua_State *L);
#endif

#ifdef USE_I2C_MODULE
#define LUA_I2CLIBNAME	"i2c"
LUALIB_API int (luaopen_i2c) (lua_State *L);
#endif

#ifdef USE_NET_MODULE
#define LUA_NETLIBNAME	"net"
LUALIB_API int (luaopen_net) (lua_State *L);
#endif

#ifdef USE_PWM_MODULE
#define LUA_PWMLIBNAME	"pwm"
LUALIB_API int (luaopen_pwm) (lua_State *L);
#endif

#ifdef USE_SPI_MODULE
#define LUA_SPILIBNAME	"spi"
LUALIB_API int (luaopen_spi) (lua_State *L);
#endif

#ifdef USE_TMR_MODULE
#define LUA_TMRLIBNAME	"tmr"
LUALIB_API int (luaopen_tmr) (lua_State *L);
#endif

#ifdef USE_UART_MODULE
#define LUA_UARTLIBNAME	"uart"
LUALIB_API int (luaopen_uart) (lua_State *L);
#endif

#ifdef USE_BIT_MODULE
#define LUA_BITLIBNAME	"bit"
LUALIB_API int (luaopen_bit) (lua_State *L);
#endif

#ifdef USE_SENSOR_MODULE
#define LUA_SENSORLIBNAME	"sensor"
LUALIB_API int (luaopen_sensor) (lua_State *L);
#endif

#ifdef USE_MQTT_MODULE
#define LUA_MQTTLIBNAME	"mqtt"
LUALIB_API int (luaopen_mqtt) (lua_State *L);
#endif

/* open all previous libraries */
LUALIB_API void (luaL_openlibs) (lua_State *L); 



#ifndef lua_assert
#define lua_assert(x)	((void)0)
#endif


#endif
