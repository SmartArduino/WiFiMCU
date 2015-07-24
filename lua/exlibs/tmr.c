/**
 * tmr.c
 */

#include "lua.h"
#include "lauxlib.h"

#include "lexlibs.h"

#include "platform.h"
#include "MICOPlatform.h"
#include "platform_peripheral.h"
#include "MICORTOS.h"

#define NUM_TMR 16

extern void _watchdog_reload_timer_handler( void* arg );

static int platform_tmr_exists( unsigned pin )
{
  return pin-1 < NUM_TMR;
}

static int tmr_cb_ref[NUM_TMR];
static lua_State* gL = NULL;
static mico_timer_t _timer[NUM_TMR];

//tmr.tick()
static int ltmr_tick( lua_State* L )
{
  uint32_t tick = mico_get_time();
  lua_pushinteger( L, tick );
  return 1; 
}
//tmr.stop(id)
//id:1~16
static int ltmr_stop( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 ) - 1;
  MOD_CHECK_ID( tmr, id+1 );
  mico_stop_timer(&_timer[id]);
  return 0;
}
//tmr.delay()
static int ltmr_delay( lua_State* L )
{
  uint32_t ms = luaL_checkinteger( L, 1 );
  if ( ms <= 0 ) return luaL_error( L, "wrong arg range" );
  
  uint32_t delay_start = mico_get_time();
  while(1)
  {
     MicoWdgReload();
     if(mico_get_time() >= delay_start + ms) break;
  }
  return 0;
}
//tmr.wdclr()
static int ltmr_wdclr( lua_State* L )
{
  MicoWdgReload();
  return 0;
}
static void _tmr_handler( void* arg )
{
  unsigned id = (unsigned)arg;
  if(tmr_cb_ref[id] == LUA_NOREF)
    return;
  lua_rawgeti(gL, LUA_REGISTRYINDEX, tmr_cb_ref[id]);
  lua_call(gL, 0, 0);
}

//tmr.start(id,interval,function)
//id:1~16
static int ltmr_start( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 )-1;
  MOD_CHECK_ID( tmr, id+1 );
  
  unsigned interval = luaL_checkinteger( L, 2 );
  if ( interval <= 0 ) 
    return luaL_error( L, "wrong arg range" );
  
  if (lua_type(L, 3) == LUA_TFUNCTION || lua_type(L, 3) == LUA_TLIGHTFUNCTION)
  {
    lua_pushvalue(L, 3);  // copy argument (func) to the top of stack
    if(tmr_cb_ref[id] != LUA_NOREF)
    {
      luaL_unref(L, LUA_REGISTRYINDEX, tmr_cb_ref[id]);
    }
    gL = L;
    tmr_cb_ref[id] = luaL_ref(L, LUA_REGISTRYINDEX);
    mico_stop_timer(&_timer[id]);
    mico_deinit_timer( &_timer[id] );
    mico_init_timer(&_timer[id], interval, _tmr_handler, (void*)id);
    mico_start_timer(&_timer[id]);    
  }
  else
    return luaL_error( L, "callback function needed" );
  
  return 0;
}

#define MIN_OPT_LEVEL       2
#include "lrodefs.h"
const LUA_REG_TYPE tmr_map[] =
{
  { LSTRKEY( "tick" ), LFUNCVAL( ltmr_tick ) },
  { LSTRKEY( "delay" ), LFUNCVAL( ltmr_delay ) },
  { LSTRKEY( "start" ), LFUNCVAL( ltmr_start ) },
  { LSTRKEY( "stop" ), LFUNCVAL( ltmr_stop ) },
  { LSTRKEY( "wdclr" ), LFUNCVAL( ltmr_wdclr) },
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_tmr(lua_State *L)
{
  for(int i=0;i<NUM_TMR;i++){
    tmr_cb_ref[i] = LUA_NOREF;
  }
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else
    luaL_register( L, EXLIB_TMR, tmr_map );
    return 1;
#endif
}


