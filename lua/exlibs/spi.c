/**
 * spi.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"

#include "MicoPlatform.h"

#define CPOL_LOW        0
#define CPOL_HIGH       1
#define CPHA_LOW        0
#define CPHA_HIGH       1
#define BITS_8          8
#define BITS_16         16

#define delay_us(nus)  MicoNanosendDelay(1000*nus)
extern const char wifimcu_gpio_map[];
#define NUM_GPIO 18
static int platform_gpio_exists( unsigned pin )
{
  return pin < NUM_GPIO;
}

uint8_t pinSCK  =255;//un assigned
uint8_t pinMISO =255;//un assigned
uint8_t pinMOSI =255;//un assigned
//spiMode=0;       cpol 0 cpha 0: sck=0 falling edge send/read data
//spiMode=1;       cpol 0 cpha 1: sck=0 rising  edge send/read data
//spiMode=2;       cpol 1 cpha 0: sck=1 rising  edge send/read data
//spiMode=3;       cpol 1 cpha 1: sck=1 falling edge send/read data
uint8_t spiMode = 0;

//make sure cs is set propoerly: Low or High before write or read
//id:0
//cpol:clock polarity
//cpha:clock phase
//pins: lua table: {sck,[miso],mosi}
//spi.setup(id,cpol,cpha,pins)
static int spi_setup( lua_State* L )
{
  uint8_t id = luaL_checkinteger( L, 1 );
  if (id !=0) return luaL_error( L, "id should assigend 0" );
  uint8_t cpol = luaL_checkinteger( L, 2 );
  if (cpol !=CPOL_LOW && cpol !=CPOL_HIGH ) return luaL_error( L, "cpol should CPOL_LOW or CPOL_HIGH" );
  uint8_t cpha = luaL_checkinteger( L, 3 );
  if (cpha !=CPHA_LOW && cpha !=CPHA_HIGH ) return luaL_error( L, "cpha should CPHA_LOW or CPHA_HIGH" );
  
  if (!lua_istable(L, 4))
    return luaL_error( L, "table arg needed" );

//pinSCK
  lua_getfield(L, 1, "sck");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the string
    {
        pinSCK = 255;
        unsigned pin = luaL_checkinteger( L, -1 );
        MOD_CHECK_ID( gpio, pin );
        pinSCK = wifimcu_gpio_map[pin];
        MicoGpioFinalize((mico_gpio_t)pinSCK);
        MicoGpioInitialize((mico_gpio_t)pinSCK,(mico_gpio_config_t)OUTPUT_PUSH_PULL);
    } 
    else
      return luaL_error( L, "wrong arg type:sck" );
  }
  else
    return luaL_error( L, "arg: sck needed" );
//pinMOSI
  lua_getfield(L, 1, "mosi");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the string
    {
        pinMOSI = 255;
        unsigned pin = luaL_checkinteger( L, -1 );
        MOD_CHECK_ID( gpio, pin );
        pinMOSI = wifimcu_gpio_map[pin];
        MicoGpioFinalize((mico_gpio_t)pinMOSI);
        MicoGpioInitialize((mico_gpio_t)pinMOSI,(mico_gpio_config_t)OUTPUT_PUSH_PULL);
    } 
    else
      return luaL_error( L, "wrong arg type:mosi" );
  }
  else
    return luaL_error( L, "arg: mosi needed" );
 
//pinMISO
  lua_getfield(L, 1, "miso");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the string
    {
        pinMISO = 255;
        unsigned pin = luaL_checkinteger( L, -1 );
        MOD_CHECK_ID( gpio, pin );
        pinMISO = wifimcu_gpio_map[pin];
        MicoGpioFinalize((mico_gpio_t)pinMISO); 
        MicoGpioInitialize((mico_gpio_t)pinMISO,(mico_gpio_config_t)INPUT_PULL_UP);
    } 
    else
      return luaL_error( L, "wrong arg type:miso" );
  }
 // else
 //   return luaL_error( L, "arg: miso needed" );
  
  spiMode = cpol<<2+cpha;
  if(cpol ==0) MicoGpioOutputLow( (mico_gpio_t)pinSCK);
  else   MicoGpioOutputHigh((mico_gpio_t)pinSCK);
  
  return 0;
}

static void _spi_write(uint8_t databits,uint8_t data)
{
  uint8_t i=0;
  for(i=0;i<databits;i++)  
  {
    if(spiMode==0 || spiMode==3)        pinSCK=1;
    else if(spiMode==1 || spiMode==2)   pinSCK=0;
    //delay_us(1);
    if(databits==8)
    {
      if((data & 0x80)==0x80) pinMOSI=1;  
      else pinMOSI=0;
    }
    else
    {
      if((data & 0x8000)==0x8000) pinMOSI=1;  
      else pinMOSI=0;
    }
    delay_us(1);
    if(spiMode==0 || spiMode==3)        pinSCK=0;
    else if(spiMode==1 || spiMode==2)   pinSCK=1;
    delay_us(1);
    data=(data<<1);
  }  
}
//spi.write(id,databits,data1,[data2],...)
static int spi_write( lua_State* L )
{
  uint8_t id = luaL_checkinteger( L, 1 );
  if (id !=0)   return luaL_error( L, "id should assigend 0" );
  uint8_t databits = luaL_checkinteger( L, 2 );
  if (databits !=BITS_8 && databits !=BITS_16 ) return luaL_error( L, "databits should BITS_8 or BITS_16" );

  if( lua_gettop( L ) < 3 )
    return luaL_error( L, "wrong arg type" );
  
  size_t datalen=0, i=0;
  int numdata=0;
  uint32_t wrote = 0;
  unsigned argn=0;
  for( argn = 2; argn <= lua_gettop( L ); argn ++ )
  {
    if( lua_type( L, argn ) == LUA_TNUMBER )
    {
      numdata = ( int )luaL_checkinteger( L, argn );
      if( databits==BITS_8 &&( numdata < 0 || numdata > 255 ))
        return luaL_error( L, "wrong arg range" );
      if( databits==BITS_16 &&( numdata < 0 || numdata > 65535 ))
        return luaL_error( L, "wrong arg range" );      
      _spi_write(databits, numdata);
      wrote ++;
    }
    else if( lua_istable( L, argn ) )
    {
      datalen = lua_objlen( L, argn );
      for( i = 0; i < datalen; i ++ )
      {
        lua_rawgeti( L, argn, i + 1 );
        numdata = ( int )luaL_checkinteger( L, -1 );
        lua_pop( L, 1 );
        if( databits==BITS_8 &&( numdata < 0 || numdata > 255 ))
        return luaL_error( L, "wrong arg range" );
        if( databits==BITS_16 &&( numdata < 0 || numdata > 65535 ))
        return luaL_error( L, "wrong arg range" );
          return luaL_error( L, "wrong arg range" );
        _spi_write(databits, numdata);
      }
      wrote += i;
      if( i < datalen )
        break;
    }
    else
    {
      const char *pdata = luaL_checklstring( L, argn, &datalen );
      for( i = 0; i < datalen; i ++ )
      {
        _spi_write(databits, (uint8_t)pdata[i]);
      }
      wrote += i;
      if( i < datalen )
        break;
    }
  }
  lua_pushinteger( L, wrote );
  return 1;
}
static uint16_t _spi_read(uint8_t databits)
{
  uint16_t data=0x00;
  
  uint8_t i=0;
  for(i=0;i<databits;i++)  
  {
    if(spiMode==0 || spiMode==3)        pinSCK=1;
    else if(spiMode==1 || spiMode==2)   pinSCK=0;
    delay_us(1);
    data<<=1;
    if(MicoGpioInputGet((mico_gpio_t)pinMISO))
      data++;
    if(spiMode==0 || spiMode==3)        pinSCK=0;
    else if(spiMode==1 || spiMode==2)   pinSCK=1;
    delay_us(1);
    data=(data<<1);
  }

  return data;
}
//spi.read(id,databits,n)
static int spi_read( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 );
  if (id !=0)   return luaL_error( L, "id should assigend 0" );
  uint8_t databits = luaL_checkinteger( L, 2 );
  if (databits !=BITS_8 && databits !=BITS_16 ) return luaL_error( L, "databits should BITS_8 or BITS_16" );
  
  uint32_t size = ( uint32_t )luaL_checkinteger( L, 3);
  if( size == 0 ) return 0;
  
  int i=0;
  static luaL_Buffer b;
  uint16_t data;
  luaL_buffinit( L, &b );
  for( i = 0; i < size; i ++ )
  {
    data = _spi_read(databits);
    lua_pushinteger( L, data );
    luaL_addvalue( &b);
  }
  luaL_pushresult(&b);
  return 1;
}

#define MIN_OPT_LEVEL   2
#include "lrodefs.h"
const LUA_REG_TYPE spi_map[] =
{
  { LSTRKEY( "setup" ), LFUNCVAL( spi_setup )},
  { LSTRKEY( "write" ), LFUNCVAL( spi_write )},
  { LSTRKEY( "read" ), LFUNCVAL( spi_read )},
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "CPOL_LOW" ), LNUMVAL( CPOL_LOW ) },
  { LSTRKEY( "CPOL_HIGH" ), LNUMVAL( CPOL_HIGH ) },  
  { LSTRKEY( "CPHA_LOW" ), LNUMVAL( CPHA_LOW ) },
  { LSTRKEY( "CPHA_HIGH" ), LNUMVAL( CPHA_HIGH ) },
  { LSTRKEY( "BITS_8" ), LNUMVAL( BITS_8 ) },
  { LSTRKEY( "BITS_16" ), LNUMVAL( BITS_16 ) },
#endif    
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_spi(lua_State *L)
{

#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else
  luaL_register( L, EXLIB_SPI, spi_map );
  MOD_REG_NUMBER( L, "CPHA_HIGH", CPHA_HIGH);
  MOD_REG_NUMBER( L, "CPHA_LOW", CPHA_LOW);
  MOD_REG_NUMBER( L, "CPOL_HIGH", CPOL_HIGH);
  MOD_REG_NUMBER( L, "CPOL_LOW", CPOL_LOW);
  MOD_REG_NUMBER( L, "BITS_8", BITS_8);
  MOD_REG_NUMBER( L, "BITS_16", BITS_16);
  return 1;
#endif
}
