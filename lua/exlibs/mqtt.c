
 /* mqtt.c
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

//#define mqtt_log(M, ...) printf(M, ##__VA_ARGS__)
#define mqtt_log(M, ...)

typedef struct {
  Client c;
  Network n;
  ssl_opts ssl_settings;
  MQTTPacket_connectData connectData;
  char *pServer;//remote server
  int port;     //remote port
  bool reqStart;//need start
  bool reqClose;//need close
  bool reqSucrible;
  bool requnSucrible;
  bool reqPublish;
  bool req_goto_disconect;
  uint32_t keepaliveTick;
  int qos;
  char *pTopic;
  char *pData;
  size_t pDataLen;
  int cb_ref_connect;
  int cb_ref_offline;
  int cb_ref_message;
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
static void messageArrived(MessageData* md)
{
  mqtt_log("messageArrived Called\r\n");
  MQTTMessage* message = md->message;
  mqtt_log("messageArrived: [%.*s]\t [%d][%.*s]\r\n",
                  md->topicName->lenstring.len, md->topicName->lenstring.data,
                  (int)message->payloadlen,
                  (int)message->payloadlen, (char*)message->payload);
  for(int i=0;i<MAX_MQTT_NUM;i++)
  {
    if(pmqtt[i]==NULL || pmqtt[i]->cb_ref_message == LUA_NOREF) continue;
      lua_rawgeti(gL, LUA_REGISTRYINDEX,  pmqtt[i]->cb_ref_message);
      lua_pushlstring(gL,(char const*)md->topicName->lenstring.data,md->topicName->lenstring.len);
      lua_pushlstring(gL,(char const*)message->payload,(int)message->payloadlen);
      lua_call(gL, 2, 0);
  }
}

static void closeMqtt(int id)
{
  mqtt_log("[mqtt:%d]closeMqtt\r\n",id);
  if(pmqtt[id]==NULL) return;
  
  pmqtt[id]->reqClose=false;
  if(pmqtt[id]->c.isconnected) {MQTTDisconnect(&(pmqtt[id]->c));}
  pmqtt[id]->n.disconnect(&(pmqtt[id]->n));
  if(MQTT_SUCCESS != MQTTClientDeinit(&(pmqtt[id]->c)))
        mqtt_log("[mqtt:%d]MQTTClientDeinit failed!",id);
   if(pmqtt[id]->cb_ref_connect != LUA_NOREF)
      luaL_unref(gL, LUA_REGISTRYINDEX, pmqtt[id]->cb_ref_connect); 
   pmqtt[id]->cb_ref_connect = LUA_NOREF;
   if(pmqtt[id]->cb_ref_offline != LUA_NOREF)
      luaL_unref(gL, LUA_REGISTRYINDEX, pmqtt[id]->cb_ref_offline); 
   pmqtt[id]->cb_ref_offline = LUA_NOREF;
   if(pmqtt[id]->cb_ref_message != LUA_NOREF)
      luaL_unref(gL, LUA_REGISTRYINDEX, pmqtt[id]->cb_ref_message); 
   pmqtt[id]->cb_ref_message = LUA_NOREF;
   free(pmqtt[id]);
   pmqtt[id]=NULL;
}
static void _thread_mqtt(void*inContext)
{
  (void)inContext;

  int rc = -1;
  fd_set readfds;
  struct timeval_t t = {0, MQTT_YIELD_TMIE*1000};
  //bool req_goto_disconect[MAX_MQTT_NUM];
  //uint32_t keepaliveTick[MAX_MQTT_NUM];
  int i=0;
  int maxSck=0;
MQTT_start:
  for(i=0;i<MAX_MQTT_NUM;i++)
  {
    if(pmqtt[i] !=NULL && pmqtt[i]->reqStart==true)
    {
      pmqtt[i]->reqStart=false;
      rc = NewNetwork(&(pmqtt[i]->n), pmqtt[i]->pServer, pmqtt[i]->port,pmqtt[i]->ssl_settings);
      if(rc < 0){
        mqtt_log("[mqtt:%d]ERROR: Create network err=%d.\r\n",i, rc);
         pmqtt[i]->req_goto_disconect=true;
      }
      else
        mqtt_log("[mqtt:%d]Create network OK!\r\n",i);
      rc = MQTTClientInit(&(pmqtt[i]->c), &(pmqtt[i]->n), MQTT_CMD_TIMEOUT);
      if(MQTT_SUCCESS != rc){
        mqtt_log("[mqtt:%d]ERROR: MQTT client init err=%d.\r\n",i, rc);
         pmqtt[i]->req_goto_disconect=true;
      }
      else{
        mqtt_log("[mqtt:%d]MQTT client init OK!\r\n",i);
      }
      mqtt_log("[mqtt:%d] client connecting...\r\n",i);
      mqtt_log("user:[len:%d]%s\r\n",strlen(pmqtt[i]->connectData.username.cstring),pmqtt[i]->connectData.username.cstring);
      mqtt_log("pass:[len:%d]%s\r\n",strlen(pmqtt[i]->connectData.password.cstring),pmqtt[i]->connectData.password.cstring);
      rc = MQTTConnect(&(pmqtt[i]->c), &(pmqtt[i]->connectData));
      if(MQTT_SUCCESS == rc){
        mqtt_log("[mqtt:%d] MQTT client connect OK!\r\n",i);
        if(pmqtt[i]->cb_ref_connect  == LUA_NOREF) continue;
        lua_rawgeti(gL, LUA_REGISTRYINDEX, pmqtt[i]->cb_ref_connect);
        lua_call(gL, 0, 0);
        lua_gc(gL, LUA_GCCOLLECT, 0);
      }
      else{
        mqtt_log("[mqtt:%d] ERROR: MQTT client connect err=%d\r\n",i, rc);
         pmqtt[i]->req_goto_disconect=true;
      }      
    }
  }
  //check goto disconnect
    for(i=0;i<MAX_MQTT_NUM;i++)
    if( pmqtt[i] !=NULL && pmqtt[i]->req_goto_disconect==true) goto MQTT_disconnect;
  mqtt_log("Go to while\r\n");
  while(1)
  {
//check subscribe or unSubcribe
    for(i=0;i<MAX_MQTT_NUM;i++)
    {
      if(pmqtt[i] ==NULL) continue;
      if(pmqtt[i]->reqSucrible==true)
      {
        pmqtt[i]->reqSucrible = false;
        rc = MQTTSubscribe(&(pmqtt[i]->c), pmqtt[i]->pTopic, (enum QoS)(pmqtt[i]->qos), messageArrived);
        if(MQTT_SUCCESS == rc)
          mqtt_log("MQTT client subscribe OK! topic=[%s]\r\n",pmqtt[i]->pTopic);
        else
          pmqtt[i]->req_goto_disconect=true;
        mqtt_log("MQTT client subscribe OK! 2\r\n");
      }
      if(pmqtt[i]->requnSucrible==true)
      {
        pmqtt[i]->requnSucrible = false;
        rc = MQTTUnsubscribe(&(pmqtt[i]->c), pmqtt[i]->pTopic);
        if(MQTT_SUCCESS == rc)
          mqtt_log("MQTT client unsubscribe OK! topic=[%s]\r\n",pmqtt[i]->pTopic);
        else
          pmqtt[i]->req_goto_disconect=true;
      }   
    }
//check publish
    for(i=0;i<MAX_MQTT_NUM;i++)
    {
      if(pmqtt[i] ==NULL) continue;
      if(pmqtt[i]->reqPublish==true)
      {
        pmqtt[i]->reqPublish = false;
        if(pmqtt[i]->pTopic == NULL || pmqtt[i]->pDataLen <= 0) continue;
        MQTTMessage publishData =  MQTTMessage_publishData_initializer;
        publishData.qos = (enum QoS)(pmqtt[i]->qos);
        publishData.payload = (void*)pmqtt[i]->pData;
        publishData.payloadlen = pmqtt[i]->pDataLen;
        rc = MQTTPublish(&(pmqtt[i]->c), pmqtt[i]->pTopic,&publishData);
        if(MQTT_SUCCESS == rc)
          mqtt_log("MQTT client publish OK!\r\n");
        else
          pmqtt[i]->req_goto_disconect=true;
      }
    }
//check close
    for(i=0;i<MAX_MQTT_NUM;i++)
    {
      if(pmqtt[i]==NULL|| !pmqtt[i]->reqClose) continue;
      closeMqtt(i);
    }
//check recieve
    for(i=0;i<MAX_MQTT_NUM;i++)
      if(pmqtt[i]!=NULL) break;
    if(i==MAX_MQTT_NUM) continue;
    FD_ZERO(&readfds);
    for(i=0;i<MAX_MQTT_NUM;i++)
    {
      if(pmqtt[i]==NULL) continue;
      FD_SET(pmqtt[i]->c.ipstack->my_socket, &readfds);
      if (pmqtt[i]->c.ipstack->my_socket > maxSck) 
           maxSck = pmqtt[i]->c.ipstack->my_socket;
    }
    select(maxSck+1, &readfds, NULL, NULL, &t);
    for(i=0;i<MAX_MQTT_NUM;i++)
    {
      if(pmqtt[i]==NULL) continue;
      if (FD_ISSET(pmqtt[i]->c.ipstack->my_socket, &readfds )){
        mqtt_log("MQTTYield 1\r\n");
        rc = MQTTYield(&(pmqtt[i]->c), (int)MQTT_YIELD_TMIE);
        mqtt_log("MQTTYield 2\r\n");
        if (MQTT_SUCCESS != rc) {
          pmqtt[i]->req_goto_disconect=true;
        }
      }
      else{
        if(mico_get_time() - pmqtt[i]->keepaliveTick> 60*1000)
          {
            pmqtt[i]->keepaliveTick = mico_get_time();
            rc = keepalive(&(pmqtt[i]->c));
            mqtt_log("keepaliveTick\r\n");
            if (MQTT_SUCCESS != rc) {
              pmqtt[i]->req_goto_disconect=true;
              }
          }
        }
    }
//check disconnect;
    for(i=0;i<MAX_MQTT_NUM;i++)
    if(pmqtt[i] !=NULL && pmqtt[i]->req_goto_disconect==true) goto MQTT_disconnect;    
  }

MQTT_disconnect:
  for(i=0;i<MAX_MQTT_NUM;i++)
  {
    if(pmqtt[i] !=NULL && pmqtt[i]->req_goto_disconect==true)
    {
      pmqtt[i]->req_goto_disconect = false;
      pmqtt[i]->reqStart=true;
      mqtt_log("[mqtt:%d] client disconnected,reconnect...\r\n",i);
      pmqtt[i]->n.disconnect(&(pmqtt[i]->n));
      if(pmqtt[i]->cb_ref_offline  == LUA_NOREF) continue;
      lua_rawgeti(gL, LUA_REGISTRYINDEX, pmqtt[i]->cb_ref_offline);
      lua_call(gL, 0, 0);
      lua_gc(gL, LUA_GCCOLLECT, 0);
    }
  }
  mico_thread_sleep(3);
  goto MQTT_start;
    
exit:
  for(i=0;i<MAX_MQTT_NUM;i++)
  {
    if(pmqtt[i]==NULL) continue;
    closeMqtt(i);
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
  if(strlen(user)==0) user=NULL;
  if(strlen(pass)==0) pass=NULL;
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
  pmqtt[k]->reqStart = false;
  pmqtt[k]->reqClose = false;
  pmqtt[k]->cb_ref_connect = LUA_NOREF;
  pmqtt[k]->cb_ref_offline = LUA_NOREF;
  pmqtt[k]->cb_ref_message = LUA_NOREF;
  pmqtt[k]->qos = QOS0;
  pmqtt[k]->pTopic = NULL;
  pmqtt[k]->pData = NULL;
  pmqtt[k]->reqSucrible = false;
  pmqtt[k]->requnSucrible = false;
  pmqtt[k]->reqPublish = false;
  pmqtt[k]->req_goto_disconect=false;
  pmqtt[k]->keepaliveTick = 0;
    
  memset(&(pmqtt[k]->c), 0, sizeof(pmqtt[k]->c));
  memset(&(pmqtt[k]->n), 0, sizeof(pmqtt[k]->n));
  gL = L;
  
  lua_pushinteger(L,k);
  return 1;
}
//mqtt.start(mqttClt, server, port)
static int lmqtt_start( lua_State* L )
{
  unsigned mqttClt;
  mqttClt = luaL_checkinteger( L, 1);
  
  if(pmqtt[mqttClt]==NULL|| mqttClt>=MAX_MQTT_NUM)
    return luaL_error( L, "mqttClt arg is wrong!" );
  
  size_t sl=0;
  char const *server = luaL_checklstring( L, 2, &sl );
  if (server == NULL) return luaL_error( L, "wrong arg type" );

  unsigned port = luaL_checkinteger( L, 3);
  
  pmqtt[mqttClt]->pServer = (char*)server;
  pmqtt[mqttClt]->port = port;
  pmqtt[mqttClt]->reqStart = true;
  
  mqtt_log("clientID:%s\r\n",pmqtt[mqttClt]->connectData.clientID.cstring);
  mqtt_log("username:%s\r\n",pmqtt[mqttClt]->connectData.username.cstring);
  mqtt_log("password:%s\r\n",pmqtt[mqttClt]->connectData.password.cstring);
  mqtt_log("pServer:%s\r\n",pmqtt[mqttClt]->pServer);
  mqtt_log("port:%d\r\n",pmqtt[mqttClt]->port);
  
  if( !mqtt_thread_is_started)
  {
    mqtt_thread_is_started = true;
    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Mqtt_Thread", _thread_mqtt, 0x4000, NULL);
  }
  return 0;
}

//mqtt.close(mqttClt)
static int lmqtt_close( lua_State* L )
{
  unsigned mqttClt = luaL_checkinteger( L, 1);
  if(mqttClt>=MAX_MQTT_NUM)
    return luaL_error( L, "mqttClt arg is wrong!" );
  if(pmqtt[mqttClt]!=NULL) 
  {
    pmqtt[mqttClt]->reqClose=true;
  }
  return 0;
}
//mqtt.subscribe(mqttClt,topic,QoS,cb_messagearrived(topic,message))
static int lmqtt_subscribe( lua_State* L )
{
  unsigned mqttClt = luaL_checkinteger( L, 1);
  if(pmqtt[mqttClt]==NULL|| mqttClt>=MAX_MQTT_NUM)
    return luaL_error( L, "mqttClt arg is wrong!" );
  size_t sl=0;
  char const *topic = luaL_checklstring( L, 2, &sl );
  if (topic == NULL) return luaL_error( L, "wrong arg type" );
  
  unsigned qos=luaL_checkinteger( L, 3);
  if (!(qos == QOS0 || qos == QOS1 ||qos == QOS2))
      return luaL_error( L, "QoS wrong arg type" );
    
  if (lua_type(L, 4) == LUA_TFUNCTION || lua_type(L, 4) == LUA_TLIGHTFUNCTION)
      {
        lua_pushvalue(L, 4);
        if( pmqtt[mqttClt]->cb_ref_message!= LUA_NOREF)
        {
          luaL_unref(L, LUA_REGISTRYINDEX, pmqtt[mqttClt]->cb_ref_message);
        }
        pmqtt[mqttClt]->cb_ref_message = luaL_ref(L, LUA_REGISTRYINDEX);
      }
    //  else
    //    return luaL_error( L, "callback function needed" );

  pmqtt[mqttClt]->pTopic = (char*)topic;
  pmqtt[mqttClt]->qos = qos;
  pmqtt[mqttClt]->reqSucrible = true;
  return 0;
}
//mqtt.unsubscribe(mqttClt,topic)
static int lmqtt_unsubscribe( lua_State* L )
{
  unsigned mqttClt = luaL_checkinteger( L, 1);
  if(pmqtt[mqttClt]==NULL|| mqttClt>=MAX_MQTT_NUM)
    return luaL_error( L, "mqttClt arg is wrong!" );
  
   size_t sl=0;
  char const *topic = luaL_checklstring( L, 2, &sl );
  if (topic == NULL) return luaL_error( L, "wrong arg type" );
  
  pmqtt[mqttClt]->pTopic = (char*)topic;
  pmqtt[mqttClt]->requnSucrible = true;
  return 0;
}
//mqtt.publish(mqttClt,topic,QoS, data)
static int lmqtt_publish( lua_State* L )
{
  unsigned mqttClt = luaL_checkinteger( L, 1);
  if(pmqtt[mqttClt]==NULL|| mqttClt>=MAX_MQTT_NUM)
    return luaL_error( L, "mqttClt arg is wrong!" );
  size_t sl=0;
  char const *topic = luaL_checklstring( L, 2, &sl );
  if (topic == NULL) return luaL_error( L, "wrong arg type" );
  
  unsigned qos=luaL_checkinteger( L, 3);
  if (!(qos == QOS0 || qos == QOS1 ||qos == QOS2))
      return luaL_error( L, "QoS wrong arg type" );
    
  char const *data = luaL_checklstring( L, 2, &sl );
  if (data == NULL) return luaL_error( L, "wrong arg type" );
  
  pmqtt[mqttClt]->pTopic = (char*)topic;
  pmqtt[mqttClt]->qos = qos;
  pmqtt[mqttClt]->pData = (char*)data;
  pmqtt[mqttClt]->pDataLen = sl;
  pmqtt[mqttClt]->reqPublish = true;
  return 0;
}
//mqtt.on(mqttClt,'connect',function())
//mqtt.on(mqttClt,'offline',function())
//mqtt.on(mqttClt,'message',cb_messagearrived(topic,message))
//  different topics with same callback
static int lmqtt_on( lua_State* L )
{
  unsigned mqttClt = luaL_checkinteger( L, 1);
  if(pmqtt[mqttClt]==NULL|| mqttClt>=MAX_MQTT_NUM)
    return luaL_error( L, "mqttClt arg is wrong!" );
  size_t sl=0;
  char const *method = luaL_checklstring( L, 2, &sl );
  if (method == NULL) return luaL_error( L, "wrong arg type" );
  
  if (lua_type(L, 3) == LUA_TFUNCTION || lua_type(L, 3) == LUA_TLIGHTFUNCTION)
      lua_pushvalue(L, 3);
  else
      return luaL_error( L, "callback function needed" );
  
  if(strcmp(method,"connect")==0&&sl==strlen("connect"))
   {
      if( pmqtt[mqttClt]->cb_ref_connect!= LUA_NOREF)
      {
        luaL_unref(L, LUA_REGISTRYINDEX, pmqtt[mqttClt]->cb_ref_connect);
      }
      pmqtt[mqttClt]->cb_ref_connect = luaL_ref(L, LUA_REGISTRYINDEX);
   }
  else if(strcmp(method,"message")==0&&sl==strlen("message"))
  {
      if( pmqtt[mqttClt]->cb_ref_message!= LUA_NOREF)
      {
        luaL_unref(L, LUA_REGISTRYINDEX, pmqtt[mqttClt]->cb_ref_message);
      }
      pmqtt[mqttClt]->cb_ref_message = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  else if(strcmp(method,"offline")==0&&sl==strlen("offline"))
  {
      if( pmqtt[mqttClt]->cb_ref_offline!= LUA_NOREF)
      {
        luaL_unref(L, LUA_REGISTRYINDEX, pmqtt[mqttClt]->cb_ref_offline);
      }
      pmqtt[mqttClt]->cb_ref_offline = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  else
    return luaL_error( L, "wrong method" );
  return 0;
}
/*
mqttClt = mqtt.new(clientid,keepalive,user,pass)
mqtt.start(mqttClt, server, port)
mqtt.on(mqttClt,'connect',function())
mqtt.on(mqttClt,'offline',function())
//different topics with same callback
mqtt.on(mqttClt,'message',cb_messagearrived(topic,message))
mqtt.close(mqttClt)
mqtt.publish(mqttClt,topic,QoS, data)
mqtt.subscribe(mqttClt,topic,QoS,cb_messagearrived(topic,message))
mqtt.unsubscribe(mqttClt,topic)
*/

#define MIN_OPT_LEVEL  2
#include "lrodefs.h"
const LUA_REG_TYPE mqtt_map[] =
{
  { LSTRKEY( "ver" ), LFUNCVAL( lmqtt_ver )},
  { LSTRKEY( "new" ), LFUNCVAL( lmqtt_new )},
  { LSTRKEY( "start" ), LFUNCVAL( lmqtt_start )},
  { LSTRKEY( "close" ), LFUNCVAL( lmqtt_close )},
  { LSTRKEY( "subscribe" ), LFUNCVAL( lmqtt_subscribe )},
  { LSTRKEY( "unsubscribe" ), LFUNCVAL( lmqtt_unsubscribe )},
  { LSTRKEY( "publish" ), LFUNCVAL( lmqtt_publish )},
  { LSTRKEY( "on" ), LFUNCVAL( lmqtt_on )},
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "QOS0" ), LNUMVAL( QOS0 ) },
  { LSTRKEY( "QOS1" ), LNUMVAL( QOS1 ) },
  { LSTRKEY( "QOS2" ), LNUMVAL( QOS2 ) },
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
    luaL_register( L, EXLIB_MQTT, mqtt_map );
    MOD_REG_NUMBER( L, "QOS0", QOS0);
    MOD_REG_NUMBER( L, "QOS1", QOS1);
    MOD_REG_NUMBER( L, "QOS2", QOS2);
    return 1;
#endif
}


