/**
 * tmr.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"

#include "platform.h"
#include "MICOPlatform.h"
#include "platform_peripheral.h"
#include "MicoDrivers/MICODriverNanoSecond.h"
#include "MICORTOS.h"

extern mico_queue_t os_queue;
#define NUM_TMR 16

extern void _watchdog_reload_timer_handler( void* arg );

static int platform_tmr_exists( unsigned pin )
{
  return pin < NUM_TMR;
}

static int tmr_cb_ref[NUM_TMR];
static lua_State* gL = NULL;
static mico_timer_t _timer[NUM_TMR];
static bool tmr_is_started[NUM_TMR];

//tmr.tick()
static int ltmr_tick( lua_State* L )
{
  uint32_t tick = mico_get_time();
  lua_pushinteger( L, tick );
  return 1; 
}
//tmr.stop(id)
//id:0~15
static int ltmr_stop( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( tmr, id );
  if(tmr_is_started[id]==true)
    mico_stop_timer(&_timer[id]);
  tmr_is_started[id]=false;
  if(tmr_cb_ref[id] != LUA_NOREF)
   {
     luaL_unref(L, LUA_REGISTRYINDEX, tmr_cb_ref[id]);
   }
    tmr_cb_ref[id] = LUA_NOREF;
  return 0;
}
static int ltmr_stopall( lua_State* L )
{
  for(int i=0;i<NUM_TMR;i++)
  {
    if(tmr_is_started[i]==true)
      mico_stop_timer(&_timer[i]);
    tmr_is_started[i]=false;
    if(tmr_cb_ref[i] != LUA_NOREF)
    {
      luaL_unref(L, LUA_REGISTRYINDEX, tmr_cb_ref[i]);
    }
    tmr_cb_ref[i] = LUA_NOREF;
  }
  return 0;
}
//tmr.delayms()
static int ltmr_delayms( lua_State* L )
{
  uint32_t ms = luaL_checkinteger( L, 1 );
  if ( ms <= 0 ) return luaL_error( L, "wrong arg range" );

  mico_thread_msleep(ms);
  return 0;
}
//tmr.delayus()
static int ltmr_delayus( lua_State* L )
{
  uint32_t us = luaL_checkinteger( L, 1 );
  if ( us <= 0 ) return luaL_error( L, "wrong arg range" );

  MicoNanosendDelay(us*1000);
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
  if(id<NUM_TMR)
  {
    queue_msg_t msg;
    msg.L = gL;
    msg.source = TMR;
    //msg.para1 = tmr_cb_ref[id];
    msg.para2 = tmr_cb_ref[id];;
    mico_rtos_push_to_queue( &os_queue, &msg,0);
  }
}

//tmr.start(id,interval,function)
//id:0~15
static int ltmr_start( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( tmr, id);
  
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
    tmr_is_started[id] = true;   
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
  { LSTRKEY( "delayms" ), LFUNCVAL( ltmr_delayms ) },
  { LSTRKEY( "delayus" ), LFUNCVAL( ltmr_delayus ) },
  { LSTRKEY( "start" ), LFUNCVAL( ltmr_start ) },
  { LSTRKEY( "stop" ), LFUNCVAL( ltmr_stop ) },
  { LSTRKEY( "stopall" ), LFUNCVAL( ltmr_stopall ) },
  { LSTRKEY( "wdclr" ), LFUNCVAL( ltmr_wdclr) },
#if LUA_OPTIMIZE_MEMORY > 0
#endif   
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_tmr(lua_State *L)
{
  for(int i=0;i<NUM_TMR;i++){
    tmr_cb_ref[i] = LUA_NOREF;
    tmr_is_started[i] = false;
  }
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else
  luaL_register( L, EXLIB_TMR, tmr_map );
  return 1;
#endif
}


