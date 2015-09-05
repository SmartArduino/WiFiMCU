/**
 * mcu.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"
   
#include "MicoPlatform.h"
#include "user_config.h"
#include "MicoWlan.h"
#include "MICO.h"
#include "StringUtils.h"

static int mcu_version( lua_State* L )
{
  lua_pushstring(L,MCU_VERSION);
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
static int mcu_chipid( lua_State* L )
{
    uint32_t mcuID[3];
    mcuID[0] = *(__IO uint32_t*)(0x1FFF7A20);
    mcuID[1] = *(__IO uint32_t*)(0x1FFF7A24);
    mcuID[2] = *(__IO uint32_t*)(0x1FFF7A28);
    char str[25];
    sprintf(str,"%08X%08X%08X",mcuID[0],mcuID[1],mcuID[2]);
    lua_pushstring(L,str);
    return 1;
}
extern unsigned char boot_reason;
static int mcu_bootreason( lua_State* L )
{
    char str[12]={0x00};
    switch(boot_reason)
    {
      case BOOT_REASON_NONE:            strcpy(str,"NONE");break;
      case BOOT_REASON_SOFT_RST:        strcpy(str,"SOFT_RST");break;
      case BOOT_REASON_PWRON_RST:       strcpy(str,"PWRON_RST");break;
      case BOOT_REASON_EXPIN_RST:       strcpy(str,"EXPIN_RST");break;
      case BOOT_REASON_WDG_RST:         strcpy(str,"WDG_RST");break;
      case BOOT_REASON_WWDG_RST:        strcpy(str,"WWDG_RST");break;
      case BOOT_REASON_LOWPWR_RST:      strcpy(str,"LOWPWR_RST");break;
      case BOOT_REASON_BOR_RST:         strcpy(str,"BOR_RST");break;
      default:strcpy(str,"NONE");break;
    }
    lua_pushstring(L,str);
    return 1;
}

#define MIN_OPT_LEVEL       2
#include "lrodefs.h"
const LUA_REG_TYPE mcu_map[] =
{
  { LSTRKEY( "ver" ), LFUNCVAL( mcu_version )},
  { LSTRKEY( "info" ), LFUNCVAL( mcu_wifiinfo )},
  { LSTRKEY( "reboot" ), LFUNCVAL( mcu_reboot )},
  { LSTRKEY( "mem" ), LFUNCVAL( mcu_memory )},
  { LSTRKEY( "chipid" ), LFUNCVAL( mcu_chipid )},
  { LSTRKEY( "bootreason" ), LFUNCVAL(mcu_bootreason)},
#if LUA_OPTIMIZE_MEMORY > 0
#endif      
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_mcu(lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else    
  luaL_register( L, EXLIB_MCU, mcu_map );
  return 1;
#endif
}
