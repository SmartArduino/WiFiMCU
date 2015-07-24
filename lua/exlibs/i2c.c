/**
 * adc.c
 */

#include "lua.h"
#include "lauxlib.h"

#include "lexlibs.h"
#include "MicoPlatform.h"
#include "user_version.h"

static int i2c_setup( lua_State* L )
{
  return 0;
}

static int i2c_read( lua_State* L )
{
  return 0;
}


#define MIN_OPT_LEVEL   2
#include "lrodefs.h"
const LUA_REG_TYPE i2c_map[] =
{
  { LSTRKEY( "setup" ), LFUNCVAL( i2c_setup )},
  { LSTRKEY( "read" ), LFUNCVAL( i2c_read )},
  {LNILKEY, LNILVAL}
};

/*
 * Open library
 */
LUALIB_API int luaopen_i2c(lua_State *L)
{

#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, EXLIB_I2C, i2c_map );
  // Add constants

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
