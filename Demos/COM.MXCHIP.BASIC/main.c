/*
* @file    main.c 
*/

#include "MiCO.h" 
#include "MICOCli.h"
#include "MICOSystemMonitor.h"
#include "MICONotificationCenter.h"

#include "lua.h"

#define DEFAULT_WATCHDOG_TIMEOUT 10*1000

#define main_log(M, ...) custom_log("main", M, ##__VA_ARGS__)

static mico_timer_t _watchdog_reload_timer;

static void _watchdog_reload_timer_handler( void* arg )
{
  (void)(arg);
  MicoWdgReload();
}

#define LUA_UART        (MICO_UART_1)
#define lua_printf      cli_printf
#define INBUF_SIZE      256
#define OUTBUF_SIZE     1024
static uint8_t *lua_rx_data;
static ring_buffer_t lua_rx_buffer;
static const mico_uart_config_t lua_uart_config =
{
  .baud_rate    = 115200,
  .data_width   = DATA_WIDTH_8BIT,
  .parity       = NO_PARITY,
  .stop_bits    = STOP_BITS_1,
  .flow_control = FLOW_CONTROL_DISABLED,
  .flags        = UART_WAKEUP_DISABLE,
};

int lua_getchar(char *inbuf)
{
  if (MicoUartRecv(LUA_UART, inbuf, 1, 50) == 0)
    return 1;
  else
    return 0;
}

int readline4lua(const char *prompt, char *buffer, int buffer_size)
{
    char ch;
    int line_position;
    lua_printf("\r"); //doit
start:
    lua_printf(prompt);/* show prompt */
    line_position = 0;
    memset(buffer, 0, buffer_size);
    while (1)
    {
       while (lua_getchar(&ch) == 1)// while (rt_device_read(dev4lua.device, 0, &ch, 1) == 1)
        {
            if (ch == '\r')/* handle CR key */
            {
                char next;
                if (lua_getchar(&next)== 1)//if (rt_device_read(dev4lua.device, 0, &next, 1) == 1)
                  ch = next;
            }
            else if (ch == 0x7f || ch == 0x08)/* backspace key */
            {
                if (line_position > 0)
                {
                    lua_printf("%c %c", ch, ch);
                    line_position--;
                }
                buffer[line_position] = 0;
                continue;
            }
            else if (ch == 0x04)/* EOF(ctrl+d) */
            {
                if (line_position == 0)
                    return 0;/* No input which makes lua interpreter close */
                else
                    continue;
            }            
            if (ch == '\r' || ch == '\n')/* end of line */
            {
                buffer[line_position] = 0;
                lua_printf("\r\n"); //doit
                if (line_position == 0)
                {
                    goto start;/* Get a empty line, then go to get a new line */
                }
                else
                {
                    return line_position;
                }
            }
            if (ch < 0x20 || ch >= 0x80)/* other control character or not an acsii character */
            {
                continue;
            }
            lua_printf("%c", ch);/* echo */
            buffer[line_position] = ch;
            ch = 0;
            line_position++;
            if (line_position >= buffer_size)/* it's a large line, discard it */
                line_position = 0;
       }
    }    
}


static void lua_main_thread(void *data)
{
  //lua setup  
  char *argv[] = {"lua", NULL};
  lua_main(1, argv);
  
//if error happened  
  lua_printf("lua exited,will reboot\r\n");
  free(lua_rx_data);
  lua_rx_data = NULL;
  mico_rtos_delete_thread(NULL);
  MicoSystemReboot();
}

int application_start( void )
{
//start
//  lua_printf( "\r\n\r\nMiCO starting...(Free memory %d bytes)\r\n",MicoGetMemoryInfo()->free_memory);
  MicoInit();

//watch dog 
  MicoWdgInitialize( DEFAULT_WATCHDOG_TIMEOUT);
  mico_init_timer(&_watchdog_reload_timer,DEFAULT_WATCHDOG_TIMEOUT/2, _watchdog_reload_timer_handler, NULL);
  mico_start_timer(&_watchdog_reload_timer);
  
#if 0
  #include "tm_stm32f4_usb_vcp.h"
  lua_printf("\r\n\r\n TM_USB_VCP_Init:%d",TM_USB_VCP_Init());
  uint8_t c;
  //NVIC_SetVectorTable(NVIC_VectTab_FLASH, new_addr);
  while(1)
  {
   if (TM_USB_VCP_GetStatus() == TM_USB_VCP_CONNECTED)
   {
     if (TM_USB_VCP_Getc(&c) == TM_USB_VCP_DATA_OK) 
     {
       TM_USB_VCP_Putc(c);/* Return data back */
     }
   }
  }
#endif
//usrinterface
  //MicoCliInit();
#if 1
//  lua_printf("Free memory %d bytes\r\n", MicoGetMemoryInfo()->free_memory); 
  lua_rx_data = (uint8_t*)malloc(INBUF_SIZE);
  ring_buffer_init( (ring_buffer_t*)&lua_rx_buffer, (uint8_t*)lua_rx_data, INBUF_SIZE );
  MicoUartInitialize( LUA_UART, &lua_uart_config, (ring_buffer_t*)&lua_rx_buffer );
  mico_rtos_create_thread(NULL, MICO_DEFAULT_WORKER_PRIORITY, "lua_main_thread", lua_main_thread, 20*1024, 0);
#endif
//  while(1) {;}
  mico_rtos_delete_thread(NULL);
  lua_printf("application_start exit\r\n");
  return 0;
 }