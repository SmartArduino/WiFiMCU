/**
 * mqtt.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"
#include "user_config.h"
   
#include "platform.h"
#include "MICOPlatform.h"
#include "platform_peripheral.h"

#include "MICODefine.h"
#include "MQTTClient.h"

#define MQTT_CMD_TIMEOUT        5000  // 5s
#define MQTT_YIELD_TMIE         1000  // 1s

#define MAX_MQTT_NUM 3
typedef struct {
  Client c;
  Network n;
  ssl_opts ssl_settings;
  MQTTPacket_connectData connectData;
  char *pServer;//remote server
  int port;     //remote port
  bool reqStart;//need start
}mqtt_t;
mqtt_t *pmqtt[MAX_MQTT_NUM];
bool mqtt_thread_is_started = false;
static lua_State *gL = NULL;

static int lmqtt_ver( lua_State* L )
{
    uint32_t mqtt_lib_version = 0;
  char str[32]={0};
  mqtt_lib_version = MQTTClientLibVersion();
  sprintf(str,"%d.%d.%d",0xFF & (mqtt_lib_version >> 16), 
                           0xFF & (mqtt_lib_version >> 8), 
                           0xFF &  mqtt_lib_version);
  lua_pushstring(L,str);
  return 1;  
}
static void _thread_mqtt(void*inContext)
{
  (void)inContext;

  int rc = -1;
  fd_set readfds;
  struct timeval_t t = {0, MQTT_YIELD_TMIE*1000};
  bool req_goto_disconect[MAX_MQTT_NUM];
  uint32_t keepaliveTick[MAX_MQTT_NUM];
  int i=0;
  for(i=0;i<MAX_MQTT_NUM;i++)
  {
    req_goto_disconect[i]=false;
    keepaliveTick[i] = mico_get_time();
  }
MQTT_start:
  for(i=0;i<MAX_MQTT_NUM;i++)
  {
    if(pmqtt[i] !=NULL&&pmqtt[i]->reqStart==true)
    {
      pmqtt[i]->reqStart=false;
      //rc = NewNetwork(&(pmqtt[i]->n), pmqtt[i]->pServer, pmqtt[i]->port,pmqtt[i]->ssl_settings);
      if(rc < 0){
        printf("[mqtt:%d]ERROR: Create network err=%d.\r\n",i, rc);
        req_goto_disconect[i]=true;
      }
      else
        printf("[mqtt:%d]Create network OK!",i);
      rc = MQTTClientInit(&(pmqtt[i]->c), &(pmqtt[i]->n), MQTT_CMD_TIMEOUT);
      if(MQTT_SUCCESS != rc){
        printf("[mqtt:%d]ERROR: MQTT client init err=%d.\r\n",i, rc);
        req_goto_disconect[i]=true;
      }
      else{
        printf("[mqtt:%d]MQTT client init OK!\r\n",i);
      }
      printf("[mqtt:%d] client connecting...\r\n",i);
      rc = MQTTConnect(&(pmqtt[i]->c), &(pmqtt[i]->connectData));
      if(MQTT_SUCCESS == rc){
        printf("[mqtt:%d] MQTT client connect OK!\r\n",i);
      }
      else{
        printf("[mqtt:%d] ERROR: MQTT client connect err=%d\r\n",i, rc);
        req_goto_disconect[i]=true;
      }      
    }
  }
  //check goto disconnect
    for(i=0;i<MAX_MQTT_NUM;i++)
    if(req_goto_disconect[i]==true) goto MQTT_disconnect;
  
  while(1)
  {
    //check subscribe
    //check send
    //check recieve
    FD_ZERO(&readfds);
    for(i=0;i<MAX_MQTT_NUM;i++)
    {
      if(pmqtt[i]==NULL) continue;
      
      FD_SET(pmqtt[i]->c.ipstack->my_socket, &readfds);
    }
    select(1, &readfds, NULL, NULL, &t);
    for(i=0;i<MAX_MQTT_NUM;i++)
    {
      if(pmqtt[i]==NULL) continue;
      if (FD_ISSET(pmqtt[i]->c.ipstack->my_socket, &readfds )){
        rc = MQTTYield(&(pmqtt[i]->c), (int)MQTT_YIELD_TMIE);
        if (MQTT_SUCCESS != rc) {
          req_goto_disconect[i]=true;
        }
      }
      else{
        /* if MQTTYield was not called periodical, we need to check ping msg to keep alive. */
        if(mico_get_time() - keepaliveTick[i]> 5000)
          {
            keepaliveTick[i] = mico_get_time();
            rc = keepalive(&(pmqtt[i]->c));
            if (MQTT_SUCCESS != rc) {
              req_goto_disconect[i]=true;
              }
          }
        }
    }
    //check disconnect;
    for(i=0;i<MAX_MQTT_NUM;i++)
    if(req_goto_disconect[i]==true) goto MQTT_disconnect;    
  }

MQTT_disconnect:
  for(i=0;i<MAX_MQTT_NUM;i++)
  {
    if(req_goto_disconect[i]==true)
    {
      req_goto_disconect[i] = false;
      pmqtt[i]->reqStart=true;
      printf("[[mqtt:%d]] client disconnected,reconnect...\r\n",i);
      pmqtt[i]->n.disconnect(&(pmqtt[i]->n));
    }
  }
  mico_thread_sleep(3);
  goto MQTT_start;
    
exit:
  for(i=0;i<MAX_MQTT_NUM;i++)
  {
    if(pmqtt[i]->c.isconnected) {MQTTDisconnect(&(pmqtt[i]->c));}
    pmqtt[i]->n.disconnect(&(pmqtt[i]->n));
    rc = MQTTClientDeinit(&(pmqtt[i]->c));
    if(MQTT_SUCCESS != rc){
      printf("[mqtt:%d]MQTTClientDeinit failed!",i);
    }
  }
  
  mico_rtos_delete_thread( NULL );
  mqtt_thread_is_started =  false;
}
//mqttClt = mqtt.new(clientid,keepalive,user,pass)
static int lmqtt_new( lua_State* L )
{
  size_t sl=0;
  char const *clientid = luaL_checklstring( L, 1, &sl );
  if (clientid == NULL) return luaL_error( L, "wrong arg type" );
  
  unsigned keepalive;
  keepalive = luaL_checkinteger( L, 2 );
    
  char const *user = luaL_checklstring( L, 3, &sl );
  char const *pass = luaL_checklstring( L, 4, &sl );
  
  int k=0;
  for(k=0;k<MAX_MQTT_NUM;k++){
    if(pmqtt[k]==NULL) break;
  }
  if(k==MAX_MQTT_NUM) return luaL_error( L, "Max MQTT Number is reached" );
  pmqtt[k] = (mqtt_t*)malloc(sizeof(mqtt_t));
  if (pmqtt[k] ==NULL) return luaL_error( L, "memery allocated failed" );
  MQTTPacket_connectData cd=MQTTPacket_connectData_initializer;
  memcpy(&(pmqtt[k]->connectData),&cd,sizeof(MQTTPacket_connectData));
  pmqtt[k]->connectData.willFlag = 0;
  pmqtt[k]->connectData.MQTTVersion = 3;
  pmqtt[k]->connectData.clientID.cstring = (char*)clientid;
  pmqtt[k]->connectData.username.cstring = (char*)user;
  pmqtt[k]->connectData.password.cstring = (char*)pass;
  pmqtt[k]->connectData.keepAliveInterval = keepalive;
  pmqtt[k]->connectData.cleansession = 1;
  //pmqtt[k]->ssl_settings.ssl_enable = false;
  
  memset(&(pmqtt[k]->c), 0, sizeof(pmqtt[k]->c));
  memset(&(pmqtt[k]->n), 0, sizeof(pmqtt[k]->n));
  
  lua_pushinteger(L,k);
  return 1;
}
//mqtt.start(mqttClt, server, port)
static int lmqtt_start( lua_State* L )
{
  unsigned mqttClt;
  mqttClt = luaL_checkinteger( L, 1);
  if(pmqtt[mqttClt]==NULL)
    return luaL_error( L, "mqttClt has not initialized" );
  
  size_t sl=0;
  char const *server = luaL_checklstring( L, 2, &sl );
  if (server == NULL) return luaL_error( L, "wrong arg type" );

  unsigned port = luaL_checkinteger( L, 3);
  
  pmqtt[mqttClt]->pServer = (char*)server;
  pmqtt[mqttClt]->port = port;
  pmqtt[mqttClt]->reqStart = true;
  
  printf("clientID:%s\r\n",pmqtt[mqttClt]->connectData.clientID.cstring);
  printf("username:%s\r\n",pmqtt[mqttClt]->connectData.username.cstring);
  printf("password:%s\r\n",pmqtt[mqttClt]->connectData.password.cstring);
  printf("pServer:%s\r\n",pmqtt[mqttClt]->pServer);
  printf("port:%d\r\n",pmqtt[mqttClt]->port);
  
  if( !mqtt_thread_is_started)
  {
    mqtt_thread_is_started = true;
    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Mqtt_Thread", _thread_mqtt, 0x1000, NULL);
  }
  return 0;
}
/*
mqttClt = mqtt.new(clientid,keepalive,user,pass)
mqtt.start(mqttClt, server, port)
mqtt.on(mqttClt,'connect',function())
mqtt.on(mqttClt,'offline',function())
mqtt.on(mqttClt,'message',cb_messagearrived(topic,message))
mqtt.close(mqttClt)
mqtt.publish(mqttClt,topic,QoS, data)
mqtt.subscribe(mqttClt,topic,QoS,cb_messagearrived(topic,message))
mqtt.unsubscribe(mqttClt)
*/

#define MIN_OPT_LEVEL  2
#include "lrodefs.h"
const LUA_REG_TYPE mqtt_map[] =
{
  { LSTRKEY( "ver" ), LFUNCVAL( lmqtt_ver )},
  { LSTRKEY( "new" ), LFUNCVAL( lmqtt_new )},
  { LSTRKEY( "start" ), LFUNCVAL( lmqtt_start )},
#if LUA_OPTIMIZE_MEMORY > 0
#endif        
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_mqtt(lua_State *L)
{
    for(int i=0;i<MAX_MQTT_NUM;i++)
    {
      pmqtt[i]=NULL;
    }
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  
#endif
}


