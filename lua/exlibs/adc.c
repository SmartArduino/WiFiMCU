/**
 * adc.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lrodefs.h"

#include "lexlibs.h"
#include "MicoPlatform.h"
#include "user_version.h"

extern const char wifimcu_gpio_map[];
const char wifimcu_adc_map[] =
{
  [MICO_GPIO_9]   = MICO_ADC_1,//D1
  [MICO_GPIO_34]  = MICO_ADC_2,//D13
  [MICO_GPIO_36]  = MICO_ADC_3,//D15
  [MICO_GPIO_37]  = MICO_ADC_4,//D16
  [MICO_GPIO_38]  = MICO_ADC_5,//D17
};

static int platform_adcpin_exists( unsigned pin )
{
  if(pin ==1 || pin ==13 ||  pin ==15 || pin ==16 ||  pin ==17) 
       return true;
  else
    return false;

}
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

const LUA_REG_TYPE adc_map[] =
{
  { LSTRKEY( "read" ), LFUNCVAL( adc_read )},
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_adc(lua_State *L)
{
  luaL_register( L, EXLIB_ADC, adc_map );

  return 1;
}
