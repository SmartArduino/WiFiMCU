/**
 * uart.c
 */

#include "lua.h"
#include "lauxlib.h"

#include "lexlibs.h"
#include "MicoPlatform.h"
#include "user_version.h"

static int uart_setup( lua_State* L )
{
  return 0;
}

static int uart_on( lua_State* L )
{
  return 0;
}

static int uart_write( lua_State* L )
{
  return 0;
}

#define MIN_OPT_LEVEL   2
#include "lrodefs.h"
const LUA_REG_TYPE uart_map[] =
{
  { LSTRKEY( "setup" ), LFUNCVAL( uart_setup )},
  { LSTRKEY( "on" ), LFUNCVAL( uart_on )},
  { LSTRKEY( "write" ), LFUNCVAL( uart_write )},
  {LNILKEY, LNILVAL}
};

/*
 * Open library
 */
LUALIB_API int luaopen_uart(lua_State *L)
{

#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, EXLIB_UART, uart_map );
  // Add constants

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
