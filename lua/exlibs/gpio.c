/**
 * gpio.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"
#include "user_config.h"
   
#include "platform.h"
#include "MICOPlatform.h"
#include "platform_peripheral.h"

#define NUM_GPIO 18
#define INPUT         OUTPUT_OPEN_DRAIN_PULL_UP+1
#define OUTPUT        OUTPUT_OPEN_DRAIN_PULL_UP+2
#define INTERRUPT     OUTPUT_OPEN_DRAIN_PULL_UP+3
#define HIGH          OUTPUT_OPEN_DRAIN_PULL_UP+4
#define LOW           OUTPUT_OPEN_DRAIN_PULL_UP+5

extern mico_queue_t os_queue;
const char wifimcu_gpio_map[] =
{
  [0] = MICO_GPIO_2,
  [1] = MICO_GPIO_9,  //pwm adc
  [2] = MICO_GPIO_16,
  [3] = MICO_GPIO_17, //pwm
  [4] = MICO_GPIO_19,
  [5] = MICO_GPIO_25, //swclk
  [6] = MICO_GPIO_26, //swdio
  [7] = MICO_GPIO_27,
  [8] = MICO_GPIO_29, //rx1 pwm
  [9] = MICO_GPIO_30, //tx1 pwm
  [10] = MICO_SYS_LED,//scl pwm MICO_GPIO_31 
  [11] = MICO_GPIO_18,//sda pwm
  [12] = MICO_GPIO_33,//pwm
  [13] = MICO_GPIO_34,//pwm adc
  [14] = MICO_GPIO_35,//pwm
  [15] = MICO_GPIO_36,//pwm adc
  [16] = MICO_GPIO_37,//pwm adc
  [17] = MICO_GPIO_38,//adc
};

static int platform_gpio_exists( unsigned pin )
{
  return pin < NUM_GPIO;
}
      
static int gpio_cb_ref[MICO_GPIO_MAX];
static lua_State* gL = NULL;

static void _gpio_irq_handler( void* arg )
{
  unsigned id = (unsigned)arg;
  if(id<NUM_GPIO)
  {
    queue_msg_t msg;
    msg.L = gL;
    msg.source = GPIO;
    //msg.para1;
    msg.para2 = gpio_cb_ref[id];;
    mico_rtos_push_to_queue( &os_queue, &msg,0);
  }
}

// gpio.mode(pin,mode)
//gpio.mode(pin,gpio.INT,'rising',function)
static int lgpio_mode( lua_State* L )
{
  unsigned mode=0;
  unsigned pin=0;
  unsigned platformPin=0;
  pin = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( gpio, pin );
  platformPin = wifimcu_gpio_map[pin];
  mode = luaL_checkinteger( L, 2 );
  if (  mode!=INTERRUPT&&
        mode!=INPUT && 
        mode!=INPUT_PULL_UP&&
        mode!=INPUT_PULL_DOWN&&
        mode!=INPUT_HIGH_IMPEDANCE&&
        mode!=OUTPUT && 
        mode!=OUTPUT_PUSH_PULL&&
        mode!=OUTPUT_OPEN_DRAIN_NO_PULL&&
        mode!=OUTPUT_OPEN_DRAIN_PULL_UP)
    return luaL_error( L, "wrong arg type" );

  if( mode == INPUT)    mode = INPUT_PULL_UP;//for default
  if( mode == OUTPUT)   mode = OUTPUT_PUSH_PULL;//for default

  if (mode!=INTERRUPT)
  {// disable interrupt
    if(gpio_cb_ref[pin] != LUA_NOREF)
    {
      luaL_unref(L, LUA_REGISTRYINDEX, gpio_cb_ref[pin]);
      MicoGpioDisableIRQ((mico_gpio_t)platformPin);
    }
    gpio_cb_ref[pin] = LUA_NOREF;
    MicoGpioFinalize((mico_gpio_t)platformPin);
    MicoGpioInitialize((mico_gpio_t)platformPin,(mico_gpio_config_t)mode);
  }
  else
  {//GPIO_INTERRUPT
       size_t sl=0;
       unsigned type=0;
       const char *str = luaL_checklstring( L, 3, &sl );
       if (str == NULL)
         return luaL_error( L, "wrong arg type" );
       if(sl == 4 && strcmp(str, "both") == 0){
         type = IRQ_TRIGGER_BOTH_EDGES;
       }else if(sl == 6 && strcmp(str, "rising") == 0){
         type = IRQ_TRIGGER_RISING_EDGE ;
       }else if(sl == 7 && strcmp(str, "falling") == 0){
         type = IRQ_TRIGGER_FALLING_EDGE;
       }
       else
         return luaL_error( L, "arg should be 'rising' or 'falling' or 'both' " );
        
      if (lua_type(L, 4) == LUA_TFUNCTION || lua_type(L, 4) == LUA_TLIGHTFUNCTION)
      {
        lua_pushvalue(L, 4);  // copy argument (func) to the top of stack
        if(gpio_cb_ref[pin] != LUA_NOREF)
        {
          luaL_unref(L, LUA_REGISTRYINDEX, gpio_cb_ref[pin]);
          MicoGpioDisableIRQ((mico_gpio_t)platformPin);
        }
        gpio_cb_ref[pin] = luaL_ref(L, LUA_REGISTRYINDEX);
      }
      else
        return luaL_error( L, "callback function needed" );

    gL = L;
    MicoGpioFinalize((mico_gpio_t)platformPin);
    MicoGpioInitialize((mico_gpio_t)platformPin, (mico_gpio_config_t)INPUT_PULL_UP);
    MicoGpioEnableIRQ( (mico_gpio_t)platformPin, (mico_gpio_irq_trigger_t)type, _gpio_irq_handler, (void*)pin);
  }  
  return 0;  
}

// Lua: read( pin )
static int lgpio_read( lua_State* L )
{
  unsigned pin=0;  
  pin = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( gpio, pin );
  pin = wifimcu_gpio_map[pin];
  unsigned level = MicoGpioInputGet( (mico_gpio_t)pin );
  lua_pushinteger( L, level );
  return 1; 
}

// Lua: write( pin, level )
static int lgpio_write( lua_State* L )
{
  unsigned level;
  unsigned pin;
  pin = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( gpio, pin );
  pin = wifimcu_gpio_map[pin];
  level = luaL_checkinteger( L, 2 );
  if ( level!=HIGH && level!=LOW && level!=1 && level!=0 )
    return luaL_error( L, "wrong arg type" );
  if( level == HIGH || level == 1)
    MicoGpioOutputHigh( (mico_gpio_t)pin );
  else
    MicoGpioOutputLow( (mico_gpio_t)pin );
  return 0;  
}

static int lgpio_toggle( lua_State* L )
{
  unsigned pin=0;
  pin = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( gpio, pin );
  pin = wifimcu_gpio_map[pin];
  if(MicoGpioInputGet( (mico_gpio_t)pin ))
    MicoGpioOutputLow( (mico_gpio_t)pin );
  else
    MicoGpioOutputHigh( (mico_gpio_t)pin );
  return 0;  
}

#define MIN_OPT_LEVEL  2
#include "lrodefs.h"
const LUA_REG_TYPE gpio_map[] =
{
  { LSTRKEY( "mode" ), LFUNCVAL( lgpio_mode ) },
  { LSTRKEY( "read" ), LFUNCVAL( lgpio_read ) },
  { LSTRKEY( "write" ), LFUNCVAL( lgpio_write ) },
  { LSTRKEY( "toggle" ), LFUNCVAL( lgpio_toggle ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "INPUT" ), LNUMVAL( INPUT ) },
  { LSTRKEY( "INPUT_PULL_UP" ), LNUMVAL( INPUT_PULL_UP ) },
  { LSTRKEY( "INPUT_PULL_DOWN" ), LNUMVAL( INPUT_PULL_DOWN ) },
  { LSTRKEY( "INPUT_HIGH_IMPEDANCE" ), LNUMVAL( INPUT_HIGH_IMPEDANCE ) },
  { LSTRKEY( "OUTPUT" ), LNUMVAL( OUTPUT ) },
  { LSTRKEY( "OUTPUT_PUSH_PULL" ), LNUMVAL( OUTPUT_PUSH_PULL ) },
  { LSTRKEY( "OUTPUT_OPEN_DRAIN_NO_PULL" ), LNUMVAL( OUTPUT_OPEN_DRAIN_NO_PULL ) },
  { LSTRKEY( "OUTPUT_OPEN_DRAIN_PULL_UP" ), LNUMVAL( OUTPUT_OPEN_DRAIN_PULL_UP ) },
  { LSTRKEY( "INT" ), LNUMVAL( INTERRUPT ) },
  { LSTRKEY( "HIGH" ), LNUMVAL( HIGH ) },
  { LSTRKEY( "LOW" ), LNUMVAL( LOW ) },
#endif        
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_gpio(lua_State *L)
{
  for(int i=0;i<NUM_GPIO;i++){
    gpio_cb_ref[i] = LUA_NOREF;
  }
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  
  luaL_register( L, EXLIB_GPIO, gpio_map );
  MOD_REG_NUMBER( L, "INPUT", INPUT);
  MOD_REG_NUMBER( L, "INPUT_PULL_UP", INPUT_PULL_DOWN);
  MOD_REG_NUMBER( L, "INPUT_PULL_DOWN", INPUT_PULL_DOWN);
  MOD_REG_NUMBER( L, "INPUT_HIGH_IMPEDANCE", INPUT_HIGH_IMPEDANCE);
  MOD_REG_NUMBER( L, "OUTPUT", OUTPUT);
  MOD_REG_NUMBER( L, "OUTPUT_PUSH_PULL", OUTPUT_PUSH_PULL);
  MOD_REG_NUMBER( L, "OUTPUT_OPEN_DRAIN_NO_PULL", OUTPUT_OPEN_DRAIN_NO_PULL);
  MOD_REG_NUMBER( L, "OUTPUT_OPEN_DRAIN_PULL_UP", OUTPUT_OPEN_DRAIN_PULL_UP);
  MOD_REG_NUMBER( L, "INT", INTERRUPT);
  MOD_REG_NUMBER( L, "HIGH", HIGH);
  MOD_REG_NUMBER( L, "LOW", LOW);
  return 1;
#endif
}
