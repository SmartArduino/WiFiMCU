/**
 * adc.c
 */

#include "lua.h"
#include "lauxlib.h"

#include "lexlibs.h"
#include "MicoPlatform.h"
#include "user_version.h"

extern const char wifimcu_gpio_map[];
const char wifimcu_adc_map[] =
{
  [MICO_GPIO_8]   = MICO_ADC_1,
  [MICO_GPIO_9]   = MICO_ADC_2,
  [MICO_GPIO_12]  = MICO_ADC_3,
  [MICO_GPIO_34]  = MICO_ADC_4,
  [MICO_GPIO_37]  = MICO_ADC_5,
  [MICO_GPIO_38]  = MICO_ADC_6,  
};

//platform_pwm_peripherals define gpio<--->adc
static int platform_adcpin_exists( unsigned pin )
{
  if(pin ==8 || pin ==9 ||  pin ==12 || pin ==34 ||  pin ==37 ||
     pin ==38) 
       return true;
  else
    return false;

}
/*
static int adc_setup( lua_State* L )
{
  return 0;
}*/

static int adc_read( lua_State* L )
{
  unsigned pin = luaL_checkinteger( L, 1);
  MOD_CHECK_ID( adcpin, pin);
  int adcPinID = wifimcu_adc_map[wifimcu_gpio_map[pin]];  
  
  uint16_t data=0;
  // init ADC
  if(kNoErr != MicoAdcInitialize((mico_adc_t)adcPinID, 3)){
    lua_pushnil(L);
    return 1;
  }
  // get ADC data
  if(kNoErr == MicoAdcTakeSample((mico_adc_t)adcPinID, &data))
  {
      lua_pushinteger(L,data);
  }
  else
  {// get data error
    lua_pushnil(L);
  }
  
  return 1;
}


#define MIN_OPT_LEVEL   2
#include "lrodefs.h"
const LUA_REG_TYPE adc_map[] =
{
//  { LSTRKEY( "setup" ), LFUNCVAL( adc_setup )},
  { LSTRKEY( "read" ), LFUNCVAL( adc_read )},
  {LNILKEY, LNILVAL}
};

/*
 * Open library
 */
LUALIB_API int luaopen_adc(lua_State *L)
{

#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, EXLIB_ADC, adc_map );
  // Add constants

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
