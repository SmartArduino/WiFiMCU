/*
** $Id: linit.c,v 1.14.1.1 2007/12/27 13:02:25 roberto Exp $
** Initialization of libraries for lua.c
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB

#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "lrotable.h"
#include "luaconf.h"
#ifndef LUA_CROSS_COMPILER
#include "platform_conf.h"
#endif

#ifdef LUA_RPC
#include "desktop_conf.h"
#endif

LUALIB_API int luaopen_platform (lua_State *L);
int luaopen_dummy(lua_State *L);

// Declare table
#if defined(LUA_PLATFORM_LIBS_ROM) && LUA_OPTIMIZE_MEMORY == 2
#undef _ROM
#define _ROM( name, openf, table ) extern const luaR_entry table[];
LUA_PLATFORM_LIBS_ROM;
#endif

// ****************************************************************************
// Platform module handling
// Automatically generate all the data required for platform modules

#if defined( PLATFORM_MODULES_ENABLE )

#if LUA_OPTIMIZE_MEMORY == 2
#undef _ROM
#define _ROM( name, openf, table ) extern const luaR_entry table[];
PLATFORM_MODULES_LIBS_ROM
#else // #if LUA_OPTIMIZE_MEMORY == 2
#undef _ROM
#define _ROM( name, openf, table ) extern const luaL_reg table[];
PLATFORM_MODULES_LIBS_ROM
#endif // #if LUA_OPTIMIZE_MEMORY == 2

#if LUA_OPTIMIZE_MEMORY == 2
const luaR_entry platform_map[] = {
#undef _ROM
#define _ROM( name, openf, table ) { LRO_STRKEY( name ), LRO_ROVAL( table ) },
  PLATFORM_MODULES_LIBS_ROM
  { LRO_NILKEY, LRO_NILVAL }
};
#else // #if LUA_OPTIMIZE_MEMORY == 2
typedef struct {
  const char *name;
  const luaL_reg *table;
} PLATFORM_MODULE_ENTRY;

static const PLATFORM_MODULE_ENTRY platform_map_tables[] = {
#undef _ROM
#define _ROM( name, openf, table ) { name, table },
  PLATFORM_MODULES_LIBS_ROM
  { NULL, NULL }
};
#endif // #if LUA_OPTIMIZE_MEMORY == 2

#undef _ROM
#define _ROM( name, openf, table ) int openf (lua_State*);
PLATFORM_MODULES_LIBS_ROM
static const lua_CFunction platform_open_funcs[] = {
#undef _ROM
#define _ROM( name, openf, table ) openf,
  PLATFORM_MODULES_LIBS_ROM
  luaopen_dummy
};

LUALIB_API int luaopen_platform (lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY == 0
  // Register the platform table first and each of the platform module's tables
  const PLATFORM_MODULE_ENTRY *plibs = platform_map_tables;

  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_GLOBALSINDEX, PS_LIB_TABLE_NAME);
  for(; plibs->name; plibs ++) {
    lua_newtable(L);
    luaL_register(L, NULL, plibs->table);
    lua_setfield(L, -2, plibs->name);
  }
  lua_pop(L, 1);
#endif // #if LUA_OPTIMIZE_MEMORY == 0
  // In any case, call each platform module's initialization function if present
  unsigned i;
  for (i = 0; i < sizeof(platform_open_funcs) / sizeof(lua_CFunction); i++) {
    lua_pushcfunction(L, platform_open_funcs[i]);
    lua_call(L, 0, 0);
  }
  return 0;
}
#endif // #if defined( PLATFORM_MODULES_ENABLE )

// End of platform module section
// ****************************************************************************


// Dummy open function
int luaopen_dummy(lua_State *L)
{
  return 0;
}

#undef _ROM
#define _ROM( name, openf, table ) { name, openf },

static const luaL_Reg lualibs[] = {
  {"", luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
//  {LUA_STRLIBNAME, luaopen_string},    
//  {LUA_MATHLIBNAME, luaopen_math},
//  {LUA_TABLIBNAME, luaopen_table},

#ifdef LUA_PLATFORM_LIBS_REG
  LUA_PLATFORM_LIBS_REG,
#endif 
#if defined(LUA_PLATFORM_LIBS_ROM)
  LUA_PLATFORM_LIBS_ROM
#endif
#if defined(LUA_LIBS_NOLTR)
  LUA_LIBS_NOLTR
#endif
  {NULL, NULL}
};

extern const luaR_entry strlib[];
extern const luaR_entry math_map[];
extern const luaR_entry tab_funcs[];
extern const luaR_entry lmt[];

#ifdef USE_ADC_MODULE
extern const luaR_entry adc_map[];
#endif 
#ifdef USE_GPIO_MODULE
extern const luaR_entry gpio_map[];
#endif
#ifdef USE_MCU_MODULE
extern const luaR_entry mcu_map[];
#endif
#ifdef USE_WIFI_MODULE
extern const luaR_entry wifi_map[];
#endif
#ifdef USE_FILE_MODULE
extern const luaR_entry file_map[];
#endif
#ifdef USE_I2C_MODULE
extern const luaR_entry i2c_map[];
#endif
#ifdef USE_NET_MODULE
extern const luaR_entry net_map[];
#endif
#ifdef USE_PWM_MODULE
extern const luaR_entry pwm_map[];
#endif
#ifdef USE_SPI_MODULE
extern const luaR_entry spi_map[];
#endif
#ifdef USE_TMR_MODULE
extern const luaR_entry tmr_map[];
#endif
#ifdef USE_UART_MODULE
extern const luaR_entry uart_map[];
#endif
#ifdef USE_BIT_MODULE
extern const luaR_entry bit_map[];
#endif
#ifdef USE_SENSOR_MODULE
extern const luaR_entry sensor_map[];
#endif
#ifdef USE_MQTT_MODULE
extern const luaR_entry mqtt_map[];
#endif


const luaR_table lua_rotable[] = 
{
    {LUA_STRLIBNAME, strlib},
    {LUA_MATHLIBNAME, math_map},
    {LUA_TABLIBNAME, tab_funcs},
#if LUA_OPTIMIZE_MEMORY > 0
#ifdef USE_ADC_MODULE
    {LUA_ADCLIBNAME, adc_map},
#endif
#ifdef USE_GPIO_MODULE
    {LUA_GPIOLIBNAME, gpio_map},
#endif
#ifdef USE_MCU_MODULE
    {LUA_MCULIBNAME, mcu_map},
#endif
#ifdef USE_WIFI_MODULE
    {LUA_WIFILIBNAME, wifi_map},
#endif
#ifdef USE_FILE_MODULE
    {LUA_FILELIBNAME, file_map},
#endif    
#ifdef USE_I2C_MODULE
    {LUA_I2CLIBNAME, i2c_map},
#endif
#ifdef USE_NET_MODULE
    {LUA_NETLIBNAME, net_map},
#endif
#ifdef USE_PWM_MODULE
    {LUA_PWMLIBNAME, pwm_map},
#endif
#ifdef USE_SPI_MODULE
    {LUA_SPILIBNAME, spi_map},
#endif
#ifdef USE_TMR_MODULE
    {LUA_TMRLIBNAME, tmr_map},
#endif
#ifdef USE_UART_MODULE
    {LUA_UARTLIBNAME, uart_map},
#endif
#ifdef USE_BIT_MODULE
    {LUA_BITLIBNAME, bit_map},
#endif    
#ifdef USE_SENSOR_MODULE
    {LUA_SENSORLIBNAME, sensor_map},
#endif    
#ifdef USE_MQTT_MODULE
    {LUA_MQTTLIBNAME, mqtt_map},
#endif    
    
#if defined(LUA_PLATFORM_LIBS_ROM) && LUA_OPTIMIZE_MEMORY == 2
#undef _ROM
#define _ROM( name, openf, table ) { name, table },
  LUA_PLATFORM_LIBS_ROM
#endif
#endif
    
  {NULL, NULL}
};

LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib = lualibs;
  for (; lib->name; lib++)
    if (lib->func) {
      lua_pushcfunction(L, lib->func);
      lua_pushstring(L, lib->name);
      lua_call(L, 1, 0);
    }
#ifdef USE_ADC_MODULE
  luaopen_adc(L);
#endif

#ifdef USE_GPIO_MODULE
  luaopen_gpio(L);
#endif

#ifdef USE_MCU_MODULE
  luaopen_mcu(L);
#endif

#ifdef USE_WIFI_MODULE
  luaopen_wifi(L);
#endif

#ifdef USE_FILE_MODULE
  luaopen_file(L);
#endif

#ifdef USE_I2C_MODULE
  luaopen_i2c(L);
#endif

#ifdef USE_NET_MODULE
  luaopen_net(L);
#endif

#ifdef USE_PWM_MODULE
  luaopen_pwm(L);
#endif

#ifdef USE_SPI_MODULE
  luaopen_spi(L);
#endif

#ifdef USE_TMR_MODULE
  luaopen_tmr(L);
#endif

#ifdef USE_UART_MODULE
  luaopen_uart(L);
#endif

#ifdef USE_BIT_MODULE
  luaopen_bit(L);
#endif
  
#ifdef USE_SENSOR_MODULE
  luaopen_sensor(L);
#endif

#ifdef USE_MQTT_MODULE
  luaopen_mqtt(L);
#endif
}

