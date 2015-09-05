/**
 * sensor.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"
#include "user_config.h"

#include "platform.h"
#include "MICODefine.h"
#include "MicoWlan.h"
#include "MICONotificationCenter.h"

//static lua_State *gL=NULL;

uint8_t PinID_DHT11=1;
#define DHT11_IO_IN()          MicoGpioInitialize( (mico_gpio_t)PinID_DHT11, INPUT_PULL_UP );										 
#define DHT11_IO_OUT()         MicoGpioInitialize( (mico_gpio_t)PinID_DHT11, OUTPUT_PUSH_PULL );
// Set Data output state
#define DHT11_DATA_Clr()       MicoGpioOutputLow((mico_gpio_t)PinID_DHT11) 
#define DHT11_DATA_Set()       MicoGpioOutputHigh((mico_gpio_t)PinID_DHT11)
// get DATA input state
#define	DHT11_DQ_IN            MicoGpioInputGet((mico_gpio_t)PinID_DHT11)
#define Delay_ms(ms)            mico_thread_msleep(ms)
#define Delay_us(nus)           MicoNanosendDelay( 1000*nus )

extern const char wifimcu_gpio_map[];
#define NUM_GPIO 18
static int platform_gpio_exists( unsigned pin )
{
  return pin < NUM_GPIO;
}
//dht11 functions
#if 1
static void DHT11_Rst(void)	   
{
  DHT11_IO_OUT();
  DHT11_DATA_Clr();
  Delay_ms(20); 
  DHT11_DATA_Set();
  Delay_us(40);
}
static uint8_t DHT11_Check(void) 
{   
  uint8_t retry=0;
  DHT11_IO_IN();		//SET INPUT	 
  while (DHT11_DQ_IN&&retry<100)//DHT11 Pull down 40~80us
  {
    retry++;
    Delay_us(1);
  }  
  if(retry>=100)
    return 1;
  else 
    retry=0;
  while (!DHT11_DQ_IN&&retry<100)//DHT11 Pull up 40~80us
  {
    retry++;
    Delay_us(1);
  }
  if(retry>=100)
    return 1;			//chack error	    
  return 0;
}
static uint8_t DHT11_Init(void)
{
  DHT11_IO_OUT();  
  DHT11_Rst();  
  return DHT11_Check();
}
static uint8_t DHT11_Read_Bit(void) 			 
{
  uint8_t retry=0;
  while(DHT11_DQ_IN&&retry<100)	//wait become Low level
  {
    retry++;
    Delay_us(1);
  }  
  retry=0;
  while(!DHT11_DQ_IN&&retry<100)//wait become High level
  {
    retry++;
    Delay_us(1);
  }  
  Delay_us(40);//wait 40us
  if(DHT11_DQ_IN)
    return 1;
  else 
    return 0;		   
}

static uint8_t DHT11_Read_Byte(void)    
{
  uint8_t i,dat;
  dat=0;
  for (i=0;i<8;i++) 
  {
    dat<<=1; 
    dat|=DHT11_Read_Bit();
  }						    
  return dat;
}

static uint8_t DHT11_Read_Data(uint8_t *temperature,uint8_t *humidity)    
{        
  uint8_t buf[5];
  uint8_t i;
  DHT11_Rst();
  if(DHT11_Check()==0)
  {
    for(i=0;i<5;i++)
    {
      buf[i]=DHT11_Read_Byte();
    }
    if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
    {
      *humidity=buf[0];
      *temperature=buf[2];
    }
  }
  else {
    return 1;
  }  
  return 0;	    
}
#endif
static int lsensor_dht11_init( lua_State* L )
{
  unsigned pin=0;
  pin = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( gpio, pin );
  PinID_DHT11 = wifimcu_gpio_map[pin];
  
  if(DHT11_Init()==0)
    lua_pushboolean(L, true);
  else
     lua_pushnil(L);
  return 1;
}
static int lsensor_dht11_get( lua_State* L )
{
  uint8_t t=0,h=0;
  if(DHT11_Read_Data(&t,&h)==0)
  {
    lua_pushinteger(L,t);
    lua_pushinteger(L,h);
  }
  else
  {
     lua_pushnil(L);
     lua_pushnil(L);
  }
  return 2;
}
static int lsensor_18b20_init( lua_State* L )
{
  
  return 0;
}
static int lsensor_18b20_get( lua_State* L )
{
  
  return 0;
}

#define MIN_OPT_LEVEL       2
#include "lrodefs.h"
static const LUA_REG_TYPE dht11_map[] =
{
  { LSTRKEY( "init" ), LFUNCVAL ( lsensor_dht11_init ) },
  { LSTRKEY( "get" ), LFUNCVAL ( lsensor_dht11_get ) },
#if LUA_OPTIMIZE_MEMORY > 0
#endif        
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE ds18b20_map[] =
{
  { LSTRKEY( "init" ),LFUNCVAL(lsensor_18b20_init ) },
  { LSTRKEY( "get" ), LFUNCVAL(lsensor_18b20_get ) },  
#if LUA_OPTIMIZE_MEMORY > 0
#endif        
  { LNILKEY, LNILVAL }
};
const LUA_REG_TYPE sensor_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "dht11" ), LROVAL( dht11_map ) },
//  { LSTRKEY( "18b20" ), LROVAL( ds18b20_map ) },
#endif
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_sensor(lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else
  luaL_register( L, EXLIB_WIFI, sensor_map );
  lua_newtable( L );
  luaL_register( L, NULL, ds18b20_map );
  lua_setfield( L, -2, "18b20" );

  lua_newtable( L );
  luaL_register( L, NULL, dht11_map );
  lua_setfield( L, -2, "dht11" );
  return 1;
#endif  
}
