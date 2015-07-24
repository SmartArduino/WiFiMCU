/**
 * mcu.c
 */

#include "lua.h"
#include "lauxlib.h"

#include "lexlibs.h"
#include "MicoPlatform.h"
#include "user_version.h"
#include "MicoWlan.h"
#include "MICO.h"
#include "StringUtils.h"

static int mcu_version( lua_State* L )
{
  lua_pushstring(L,NODE_VERSION);
  lua_pushstring(L,BUILD_DATE);
  return 2;
}
static int mcu_wifiinfo( lua_State* L )
{
  char wifi_ver[64] = {0};
  IPStatusTypedef para;
  char  mac[18];
  MicoGetRfVer(wifi_ver, sizeof(wifi_ver));
  micoWlanGetIPStatus(&para, Station);
  formatMACAddr(mac, (char *)&para.mac);  
  lua_pushstring(L,MicoGetVer());//mxchipWNet library version
  lua_pushstring(L,mac);//mac
  //lua_pushstring(L,mico_get_bootloader_ver());//bootloader_ver
  lua_pushstring(L,wifi_ver);//Wi-Fi driver version    
  return 3;
}

static int mcu_reboot( lua_State* L )
{
   MicoSystemReboot();
    return 0;
}

static int mcu_memory( lua_State* L )
{
   lua_pushinteger(L,MicoGetMemoryInfo()->free_memory);/**< total free space*/
   lua_pushinteger(L,MicoGetMemoryInfo()->allocted_memory);/**< total allocated space*/
   lua_pushinteger(L,MicoGetMemoryInfo()->total_memory);/**< maximum total allocated space*/
   lua_pushinteger(L,MicoGetMemoryInfo()->num_of_chunks); /**< number of free chunks*/
   return 4;
}

#define MIN_OPT_LEVEL       2
#include "lrodefs.h"
const LUA_REG_TYPE mcu_map[] =
{
  { LSTRKEY( "ver" ), LFUNCVAL( mcu_version )},
  { LSTRKEY( "info" ), LFUNCVAL( mcu_wifiinfo )},
  { LSTRKEY( "reboot" ), LFUNCVAL( mcu_reboot )},
  { LSTRKEY( "mem" ), LFUNCVAL( mcu_memory )},
  {LNILKEY, LNILVAL}
};

/**
 * Open library
 */
LUALIB_API int luaopen_mcu(lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, EXLIB_MCU, mcu_map );
  // Add constants

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
