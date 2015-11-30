/**
 * i2c.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"

#include "MicoPlatform.h"
#include "user_config.h"

#define delay_us(nus)  MicoNanosendDelay(1000*nus)

#define SDA_OUT()       MicoGpioFinalize((mico_gpio_t)pinSDA);MicoGpioInitialize((mico_gpio_t)pinSDA,(mico_gpio_config_t)OUTPUT_PUSH_PULL)
#define SDA_IN()        MicoGpioFinalize((mico_gpio_t)pinSDA); MicoGpioInitialize((mico_gpio_t)pinSDA,(mico_gpio_config_t)INPUT_PULL_UP)
#define IIC_SCL(d) if (d==1) MicoGpioOutputHigh( (mico_gpio_t)pinSCL);\
                   else MicoGpioOutputLow( (mico_gpio_t)pinSCL); 
#define IIC_SDA(d) if (d==1) MicoGpioOutputHigh( (mico_gpio_t)pinSDA);\
                   else MicoGpioOutputLow((mico_gpio_t)pinSDA); 
#define READ_SDA   MicoGpioInputGet((mico_gpio_t)pinSDA)
                   
extern const char wifimcu_gpio_map[];
#define NUM_GPIO 18
static int platform_gpio_exists( unsigned pin )
{
  return pin < NUM_GPIO;
}

//i2c.setup(id,sda,scl)
//write a data at reg_addd in dev_id
//i2c.start(id)
//i2c.address(id,device_id,'w')
//i2c.write(id,addrHigh)
//[i2c.write(id,addrLow)]
//i2c.write(id,data)
//i2c.stop(id)

//read data at reg_addd in dev_id
//i2c.start(id)
//i2c.address(id,device_id,'w')
//i2c.write(id,addr)
//i2c.start(id)
//i2c.address(id,device_id,'r')
//data = i2c.read(id,1)
//i2c.stop()

uint8_t pinSDA = 0;
uint8_t pinSCL = 0;

//i2c.setup(id,pinSDA, pinSCL)
static int i2c_setup( lua_State* L )
{
  unsigned id =  luaL_checkinteger( L, 1 );
  unsigned sda = luaL_checkinteger( L, 2 );
  unsigned scl = luaL_checkinteger( L, 3 );
  if (id !=0) return luaL_error( L, "id should assigend 0" );
  MOD_CHECK_ID( gpio, sda );
  MOD_CHECK_ID( gpio, scl );
  pinSDA = wifimcu_gpio_map[sda];
  pinSCL = wifimcu_gpio_map[scl];
  
  MicoGpioFinalize((mico_gpio_t)pinSDA);
  MicoGpioInitialize((mico_gpio_t)pinSDA,(mico_gpio_config_t)OUTPUT_PUSH_PULL);  
  MicoGpioOutputHigh( (mico_gpio_t)pinSDA);
  MicoGpioFinalize((mico_gpio_t)pinSCL);
  MicoGpioInitialize((mico_gpio_t)pinSCL,(mico_gpio_config_t)OUTPUT_PUSH_PULL);  
  MicoGpioOutputHigh( (mico_gpio_t)pinSCL);
  return 0;
}

static void IIC_Start(void)
{
  SDA_OUT();
  IIC_SDA(1);
  IIC_SCL(1);
  delay_us(4);
  IIC_SDA(0);
  delay_us(4);
  IIC_SCL(0);
  delay_us(4);
}
static void IIC_Stop(void)
{
  SDA_OUT();
  IIC_SDA(0);
  IIC_SCL(1);  
  delay_us(4);
  IIC_SDA(1);
  delay_us(4);							   	
}
//return: 1 failed
//return: 0 ok
static uint8_t IIC_Wait_Ack(void)
{
  uint8_t ucErrTime=0;
  SDA_OUT();
  IIC_SDA(1);delay_us(1);
  IIC_SCL(1);delay_us(1);
  SDA_IN();
  while(READ_SDA)
  {
    ucErrTime++;
    if(ucErrTime>250)
    {
	IIC_Stop();
	return 1;
    }
  }
  IIC_SCL(0);
  return 0;  
} 

static void IIC_Ack(void)
{
  IIC_SCL(0);
  SDA_OUT();
  IIC_SDA(0);
  delay_us(2);
  IIC_SCL(1);
  delay_us(2);
  IIC_SCL(0);
}
    
static void IIC_NAck(void)
{
  IIC_SCL(0);
  SDA_OUT();
  IIC_SDA(1);
  delay_us(2);
  IIC_SCL(1);
  delay_us(2);
  IIC_SCL(0);
}	
static void IIC_Send_Byte(uint8_t txd)
{
  uint8_t t;   
  SDA_OUT();
  IIC_SCL(0);
  for(t=0;t<8;t++)
    {
      //IIC_SDA=(txd&0x80)>>7;
      if((txd&0x80)>>7)
      {
        IIC_SDA(1);
      }
      else
      {
        IIC_SDA(0);
      }
      txd<<=1;
      delay_us(2);
      IIC_SCL(1);
      delay_us(2); 
      IIC_SCL(0);	
      delay_us(2);
    }
}
//ack=1 send ACK; ack=0 send nACK
uint8_t IIC_Read_Byte(unsigned char ack)
{
  unsigned char i,receive=0;
  SDA_IN();
  for(i=0;i<8;i++ )
  {
     IIC_SCL(0); 
     delay_us(2);
     IIC_SCL(1);
     receive<<=1;
     if(READ_SDA)receive++;
      delay_us(1); 
   }					 
  if (!ack)
    IIC_NAck();
  else
    IIC_Ack();
  return receive;
}
//i2c.start(id)
static int i2c_start( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 );
  if (id !=0)   return luaL_error( L, "id should assigend 0" );
  IIC_Start();
  return 0;
}
//i2c.stop(id)
static int i2c_stop( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 );
  if (id !=0)   return luaL_error( L, "id should assigend 0" );
  IIC_Stop();
  return 0;
}
//i2c.address(id, dev_id, 'r'/'w')
static int i2c_address( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 );
  if (id !=0)   return luaL_error( L, "id should assigend 0" );
  
  unsigned int temp = luaL_checkinteger( L, 2 ); 
  if( temp>127) return luaL_error( L, "dev_id is wrong" );
  uint8_t dev_id = (uint8_t)(temp<<1);
  
  size_t sl=0;
  const char *str = luaL_checklstring( L, 3, &sl );
  if (str == NULL)
   return luaL_error( L, "wrong arg type" );
  if(sl == 1 && strcmp(str, "w") == 0)
  {
      IIC_Send_Byte(dev_id);
      if(IIC_Wait_Ack()==1) lua_pushnil(L);  else lua_pushboolean(L, true);
  }
  else if(sl == 1 && strcmp(str, "r") == 0)
  {
      IIC_Send_Byte(dev_id|0x01);
      if(IIC_Wait_Ack()==1) lua_pushnil(L);  else lua_pushboolean(L, true);
  }
  return 1;
}
//i2c.write(id, data1,[data2],...)
static int i2c_write( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 );
  if (id !=0)   return luaL_error( L, "id should assigend 0" );
  if( lua_gettop( L ) < 2 )
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
      if( numdata < 0 || numdata > 255 )
        return luaL_error( L, "wrong arg range" );
      IIC_Send_Byte((uint8_t)numdata);
      if(IIC_Wait_Ack()==1) break;
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
        if( numdata < 0 || numdata > 255 )
          return luaL_error( L, "wrong arg range" );
        IIC_Send_Byte((uint8_t)numdata);
        if(IIC_Wait_Ack()==1) break;
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
        IIC_Send_Byte((uint8_t)pdata[i]);
        if(IIC_Wait_Ack()==1) break;
      }
      wrote += i;
      if( i < datalen )
        break;
    }
  }
  lua_pushinteger( L, wrote );
  return 1;
}
//i2c.read(id,n)
static int i2c_read( lua_State* L )
{
  unsigned id = luaL_checkinteger( L, 1 );
  if (id !=0)   return luaL_error( L, "id should assigend 0" );
  uint32_t size = ( uint32_t )luaL_checkinteger( L, 2 );
  if( size == 0 ) return 0;
  
  int i=0;
  static luaL_Buffer b;
  int data;
  luaL_buffinit( L, &b );
  for( i = 0; i < size; i ++ )
  {
    if ( i == size -1)
    data = IIC_Read_Byte(0);
    else
    data = IIC_Read_Byte(1);
    luaL_addchar( &b, ( char )data );
  /*  if( ( data = IIC_Send_Byte( id, i >= size - 1 ) ) == -1 )
        break;
    else
  luaL_addchar( &b, ( char )data );*/
  }
  luaL_pushresult(&b);
  return 1;
}
#define MIN_OPT_LEVEL  2
#include "lrodefs.h"
const LUA_REG_TYPE i2c_map[] =
{
  { LSTRKEY( "setup" ), LFUNCVAL( i2c_setup )},
  { LSTRKEY( "start" ), LFUNCVAL( i2c_start )},
  { LSTRKEY( "stop" ),  LFUNCVAL( i2c_stop )},
  { LSTRKEY( "address" ),  LFUNCVAL( i2c_address )},
  { LSTRKEY( "write" ), LFUNCVAL( i2c_write )},
  { LSTRKEY( "read" ),  LFUNCVAL( i2c_read )},
#if LUA_OPTIMIZE_MEMORY > 0
#endif          
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_i2c(lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  
  luaL_register( L, EXLIB_I2C, i2c_map );
  return 1;
#endif
}
