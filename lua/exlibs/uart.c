/**
 * uart.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"
#include "MicoPlatform.h"

static lua_State *gL = NULL;
static int usr_uart_cb_ref = LUA_NOREF;
#define LUA_USR_UART        (MICO_UART_2)
#define USR_UART_LENGTH     512
#define USR_INBUF_SIZE      USR_UART_LENGTH
#define USR_OUTBUF_SIZE     USR_UART_LENGTH
static uint8_t *pinbuf=NULL;
static uint8_t *lua_usr_rx_data=NULL;
static ring_buffer_t lua_usr_rx_buffer;
static mico_uart_config_t lua_usr_uart_config =
{
  .baud_rate    = 115200,
  .data_width   = DATA_WIDTH_8BIT,
  .parity       = NO_PARITY,
  .stop_bits    = STOP_BITS_1,
  .flow_control = FLOW_CONTROL_DISABLED,
  .flags        = UART_WAKEUP_DISABLE,
};
static mico_thread_t *plua_usr_usart_thread=NULL;

static int platform_uart_exists( unsigned id )
{
  if(id==1) return true;
  else
    return false;
}

static void lua_usr_usart_thread(void *data)
{
  uint16_t len=0,index=0;
  uint32_t lastTick=0;

  while(1)
  {
    if((len=MicoUartGetLengthInBuffer(LUA_USR_UART))==0) 
    {
      if(index>0 && mico_get_time() - lastTick>=100) goto doUartData;
      mico_thread_msleep(10);
      continue;
    }
    if(index ==0) lastTick =  mico_get_time();
    if(index+len>=USR_UART_LENGTH) len = USR_UART_LENGTH - index;
    MicoUartRecv(LUA_USR_UART, pinbuf+index, len, 10);
    index = index+len;pinbuf[index]=0x00;

doUartData:
    if(index>=USR_UART_LENGTH || mico_get_time() - lastTick>=100)
    {
      if(usr_uart_cb_ref == LUA_NOREF) continue;
      lua_rawgeti(gL, LUA_REGISTRYINDEX, usr_uart_cb_ref);
      lua_pushlstring(gL,(char const*)pinbuf,index);
      lua_call(gL, 1, 0);
      index = 0;
    }
  }  
}

//uart.setup(1,9600,'n','8','1')
//baud:all supported
//parity:
//      n,NO_PARITY;
//      o,ODD_PARITY;
//      e,EVEN_PARITY
//databits:
//      5:DATA_WIDTH_5BIT,
//      6:DATA_WIDTH_6BIT,
//      7:DATA_WIDTH_7BIT,
//      8:DATA_WIDTH_8BIT,
//      9:DATA_WIDTH_9BIT
//stopbits:
//      1,STOP_BITS_1
//      2,STOP_BITS_2
static int uart_setup( lua_State* L )
{
  uint16_t id, databits=DATA_WIDTH_8BIT, parity=NO_PARITY, stopbits=STOP_BITS_1;
  uint32_t baud=9600;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  baud = luaL_checkinteger( L, 2 );
  size_t sl=0;
  char const *str = luaL_checklstring( L, 3, &sl );
  if(sl == 1 && strcmp(str, "n") == 0)
    parity = NO_PARITY;
  else if(sl == 1 && strcmp(str, "o") == 0)
    parity = ODD_PARITY;
  else if(sl == 1 && strcmp(str, "e") == 0)
    parity = EVEN_PARITY;
  else
    return luaL_error( L, "arg parity should be 'n' or 'o' or 'e' " );
  
  str = luaL_checklstring( L, 4, &sl );
  if(sl == 1 && strcmp(str, "5") == 0)
    databits = DATA_WIDTH_5BIT;
  else if(sl == 1 && strcmp(str, "6") == 0)
    databits = DATA_WIDTH_6BIT;
  else if(sl == 1 && strcmp(str, "7") == 0)
    databits = DATA_WIDTH_7BIT;
  else if(sl == 1 && strcmp(str, "8") == 0)
    databits = DATA_WIDTH_8BIT;
  else if(sl == 1 && strcmp(str, "9") == 0)
    databits = DATA_WIDTH_9BIT;
  else
    return luaL_error( L, "arg databits should be '5'~'9' " );
  
  str = luaL_checklstring( L, 5, &sl );
  if(sl == 1 && strcmp(str, "1") == 0)
    stopbits = STOP_BITS_1;
  else if(sl == 1 && strcmp(str, "2") == 0)
    stopbits = STOP_BITS_2;
  else
    return luaL_error( L, "arg stopbits should be '1' or '2' " );

  lua_usr_uart_config.baud_rate = baud;
  lua_usr_uart_config.parity=(platform_uart_parity_t)parity;
  lua_usr_uart_config.data_width =(platform_uart_data_width_t)databits;
  lua_usr_uart_config.stop_bits =(platform_uart_stop_bits_t)stopbits;
  
  if(lua_usr_rx_data !=NULL) free(lua_usr_rx_data);
  lua_usr_rx_data = (uint8_t*)malloc(USR_INBUF_SIZE);
  if(pinbuf !=NULL) free(pinbuf);
  pinbuf = (uint8_t*)malloc(USR_UART_LENGTH);
  ring_buffer_init( (ring_buffer_t*)&lua_usr_rx_buffer, (uint8_t*)lua_usr_rx_data, USR_INBUF_SIZE );
  
  //MicoUartFinalize(LUA_USR_UART);
  MicoUartInitialize( LUA_USR_UART, &lua_usr_uart_config, (ring_buffer_t*)&lua_usr_rx_buffer );
  gL = L;
  usr_uart_cb_ref = LUA_NOREF;
  if(plua_usr_usart_thread !=NULL) mico_rtos_delete_thread(plua_usr_usart_thread);  
  mico_rtos_create_thread(plua_usr_usart_thread, MICO_DEFAULT_WORKER_PRIORITY, "lua_usr_usart_thread", lua_usr_usart_thread, 0x300, 0);
  return 0;
}
//uart.on(id,function(t))
static int uart_on( lua_State* L )
{
  uint16_t id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  
  size_t sl;
  const char *method = luaL_checklstring( L, 2, &sl );
  if (method == NULL)
    return luaL_error( L, "wrong arg type" );
  
  if(sl == 4 && strcmp(method, "data") == 0)
  {
    if (lua_type(L, 3) == LUA_TFUNCTION || lua_type(L, 3) == LUA_TLIGHTFUNCTION)
      {
        lua_pushvalue(L, 3);
        if(usr_uart_cb_ref != LUA_NOREF)
        {
          luaL_unref(L, LUA_REGISTRYINDEX, usr_uart_cb_ref);
        }
        usr_uart_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
      }
  }
  else
  {
    return luaL_error( L, "wrong arg type" );
  }
  return 0;
}
//uart.send(1,string1,number,...[stringn])
static int uart_send( lua_State* L )
{
  uint16_t id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  
  const char* buf;
  size_t len;
  int total = lua_gettop( L ), s;
  
  for( s = 2; s <= total; s ++ )
  {
    if( lua_type( L, s ) == LUA_TNUMBER )
    {
      len = lua_tointeger( L, s );
      if( len > 255 ) return luaL_error( L, "invalid number" );
      MicoUartSend( LUA_USR_UART,(char*)len,1);
    }
    else
    {
      luaL_checktype( L, s, LUA_TSTRING );
      buf = lua_tolstring( L, s, &len );
      MicoUartSend( LUA_USR_UART, buf,len);
    }
  }
  return 0;
}
#define MIN_OPT_LEVEL   2
#include "lrodefs.h"
const LUA_REG_TYPE uart_map[] =
{
  { LSTRKEY( "setup" ), LFUNCVAL( uart_setup )},
  { LSTRKEY( "on" ), LFUNCVAL( uart_on )},
  { LSTRKEY( "send" ), LFUNCVAL( uart_send )},
#if LUA_OPTIMIZE_MEMORY > 0
#endif      
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_uart(lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  
  luaL_register( L, EXLIB_UART, uart_map );
  return 1;
#endif
}
