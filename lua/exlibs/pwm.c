/**
 * pwm.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"

#include "platform.h"
#include "MICOPlatform.h"
#include "platform_peripheral.h"
#include "MICORTOS.h"

extern const char wifimcu_gpio_map[];
const char wifimcu_pwm_map[] =
{
  [MICO_GPIO_9]  = MICO_PWM_1,//D1
  [MICO_GPIO_17] = MICO_PWM_2,//D3
  [MICO_GPIO_29] = MICO_PWM_3,//D8
  [MICO_GPIO_30] = MICO_PWM_4,//D9
  [MICO_SYS_LED] = MICO_PWM_5,//D10
  [MICO_GPIO_18] = MICO_PWM_6,//D11
  [MICO_GPIO_33] = MICO_PWM_7,//D12
  [MICO_GPIO_34] = MICO_PWM_8,//D13
  [MICO_GPIO_35] = MICO_PWM_9,//D14
  [MICO_GPIO_36] = MICO_PWM_10,//D15
  [MICO_GPIO_37] = MICO_PWM_11,//D16
};

//platform_pwm_peripherals define gpio<--->pwm
static int platform_pwmpin_exists( unsigned pin )
{
  if(pin ==1 || pin ==3 ||  pin ==8 || pin ==9 ||  pin ==10 ||
     pin ==11 || pin ==12 ||  pin ==13 || pin ==14|| pin ==15|| pin ==16) 
       return true;
  else
    return false;
}

//pwm.setup(pin,freq,duty)
static int lpwm_start( lua_State* L )
{
  unsigned pin = luaL_checkinteger( L, 1);
  MOD_CHECK_ID( pwmpin, pin);
  int pwmPinID = wifimcu_pwm_map[wifimcu_gpio_map[pin]];
  int freq = luaL_checkinteger( L, 2);
  if(freq < 0 || freq>10000)
    return luaL_error( L, "0< freq < 10kHz" );
  int duty = luaL_checkinteger( L, 3);
  if( duty<0 || duty>100)
    return luaL_error( L, "0< duty < 100" );
  
  if(pin==12) duty = 100 - duty;
  
  MicoGpioFinalize((mico_gpio_t)wifimcu_gpio_map[pin]); 
  MicoGpioInitialize((mico_gpio_t)wifimcu_gpio_map[pin],OUTPUT_PUSH_PULL);
    
  MicoPwmInitialize((mico_pwm_t)pwmPinID,(uint32_t)freq,(float)duty);
  MicoPwmStart((mico_pwm_t)pwmPinID);
  return 0;
}

//lpwm_stop(pin)
static int lpwm_stop( lua_State* L )
{
  unsigned pin = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( pwmpin, pin);
  int pwmPinID = wifimcu_pwm_map[wifimcu_gpio_map[pin]];
   
  MicoPwmStop((mico_pwm_t)pwmPinID);
  return 0;
}

#define MIN_OPT_LEVEL       2
#include "lrodefs.h"
const LUA_REG_TYPE pwm_map[] =
{
  { LSTRKEY( "start" ), LFUNCVAL( lpwm_start ) },
  { LSTRKEY( "stop" ), LFUNCVAL( lpwm_stop ) },
#if LUA_OPTIMIZE_MEMORY > 0
#endif    
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_pwm(lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  
  luaL_register( L, EXLIB_PWM, pwm_map );
  return 1;
#endif
}


