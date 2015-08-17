/**
 * adc.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lrodefs.h"
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

const LUA_REG_TYPE i2c_map[] =
{
  { LSTRKEY( "setup" ), LFUNCVAL( i2c_setup )},
  { LSTRKEY( "read" ), LFUNCVAL( i2c_read )},
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_i2c(lua_State *L)
{
  luaL_register( L, EXLIB_I2C, i2c_map );
  return 1;
}
