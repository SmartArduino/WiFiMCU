/**
 * net.c
 */

#include "lua.h"
#include "lauxlib.h"

#include "lexlibs.h"

#include "platform.h"
#include "MICODefine.h"
#include "MicoWlan.h"
#include "MICONotificationCenter.h"
#include "SocketUtils.h"
#include "MICORTOS.h"

#define TCP IPPROTO_TCP
#define UDP IPPROTO_UDP
#define SOCKET_SERVER 0
#define SOCKET_CLIENT 1
#define INVALID_HANDLE -1

#define MAX_SOCKET_NUM 4
#define MAX_CLIENT_NUM 4
enum _req_actions{
  NO_ACTION=0,
  REQ_ACTION_SENT,
  REQ_ACTION_DISCONNECT
};

static lua_State *gL = NULL;

//#define NET_DBG MCU_DBG
#define NET_DBG

#define MAX_RECV_LEN 1024
static char recvBuf[MAX_RECV_LEN];

typedef struct lnet_client_userdata{
  int self_ref;
  int cb_connect_ref;
  int cb_reconnect_ref;
  int cb_disconnect_ref;
  int cb_receive_ref;
  int cb_send_ref;
  int cb_dns_found_ref;
}lnet_client_userdata;

typedef struct lnet_server_userdata{
  char socketIndex;
  int self_ref;
}lnet_server_userdata;

static struct _lsocket{
  int skt; //skt
  int udref;//usrdata ref
  bool reqSetup;
  char socketType;//TCP / UDP
  bool isServer;
  int port;
  int listen_cb;//tcp server:listen callback ; tcp client: dns callback
  int client[MAX_CLIENT_NUM];
  int udref_client[MAX_CLIENT_NUM];
  char socketAction[MAX_CLIENT_NUM];//REQ_ACTION_SENT or REQ_ACTION_DISCONNECT
}lsocket[MAX_SOCKET_NUM];

static bool timer_net_is_started = false;
static mico_timer_t _timer_net;
static bool isRunningLgethostbyname_thread=false;
static char *pDomain4Dns=NULL;
static char *pIPstr=NULL;
static int socketIndexTemp=0;
  
static int getSocketIndex(int self_ref)
{
    if(self_ref == LUA_NOREF) return -1;
    for(int i=0;i<MAX_SOCKET_NUM;i++)
    {
      if(lsocket[i].udref == self_ref) 
       return i;
    }
    return -1;
}
static void free_usrdata_serverClient(lua_State* L,lnet_client_userdata *nud,int socketIndex,int clientIndex)
{
  //NET_DBG("send failed unref and delete client usrdata\r\n");
  
  if(nud->cb_connect_ref !=LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, nud->cb_connect_ref);
    nud->cb_connect_ref = LUA_NOREF;
  if(nud->cb_reconnect_ref !=LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, nud->cb_reconnect_ref);
    nud->cb_reconnect_ref = LUA_NOREF;
  if(nud->cb_disconnect_ref !=LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, nud->cb_disconnect_ref);
    nud->cb_disconnect_ref = LUA_NOREF;
  if(nud->cb_receive_ref !=LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, nud->cb_receive_ref);
    nud->cb_receive_ref = LUA_NOREF;
   if(nud->cb_send_ref !=LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, nud->cb_send_ref);
    nud->cb_send_ref = LUA_NOREF;
   if(nud->cb_dns_found_ref !=LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, nud->cb_dns_found_ref);
    nud->cb_dns_found_ref = LUA_NOREF; 
    
    int n = lua_gettop(gL);
    lua_gc(L, LUA_GCSTOP, 0);//unref client userdata
    if(lsocket[socketIndex].udref_client[clientIndex] !=LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, lsocket[socketIndex].udref_client[clientIndex]);
    lsocket[socketIndex].udref_client[clientIndex] = LUA_NOREF;
    lua_pop(L, 1);//remove it
    lua_gc(L, LUA_GCRESTART, 0);
    lua_settop(gL, n);// reset the stack top  
    
    if(lsocket[socketIndex].isServer==false)
    {
      lsocket[socketIndex].udref=LUA_NOREF;
      nud->self_ref = LUA_NOREF;      
    }
}
void lgethostbyname_thread(void *inContext)
{
  
  //NET_DBG("lgethostbyname_thread is called.\r\n");
  if(pDomain4Dns ==NULL) goto exit;
  
  //NET_DBG("pDomain4Dns:%s\r\n",pDomain4Dns);
  isRunningLgethostbyname_thread=true;
  gethostbyname((char *)pDomain4Dns, (uint8_t *)pIPstr, 16);
  free(pDomain4Dns);pDomain4Dns=NULL;
  //NET_DBG("pIPstr:%s\r\n",pIPstr);
  lsocket[socketIndexTemp].reqSetup = true;
exit:
  //NET_DBG("lgethostbyname_thread exit.\r\n");
  mico_rtos_delete_thread(NULL);
  
  //isRunningLgethostbyname_thread=false;//will be set false at _timer_net_handle
}
static void _timer_net_handle( void* arg )
{
  //NET_DBG("_timer_net_handle is called.\r\n");
  //timer_net_is_started = false;
  //mico_stop_timer(&_timer_net);

  static fd_set readfds;
  static struct timeval_t t_val;
  static struct sockaddr_t addr;
  static int len;
  static unsigned char i=0,k=0;  
  
  t_val.tv_sec=0;
  t_val.tv_usec=500;
//++++++++++++++++++++++++++++++++++++check whether need setup socket server++++++++++++++++++++++++++++++++++++
  for(i=0;i<MAX_SOCKET_NUM;i++)
  {
    if(lsocket[i].reqSetup==true)
    {
      lsocket[i].reqSetup=false;
      //tcp server-----------------------------------------------------------------
      if(lsocket[i].socketType==TCP&&
         lsocket[i].isServer==true)
      {
        if(lsocket[i].skt !=INVALID_HANDLE ) close(lsocket[i].skt);
        
        lsocket[i].skt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        uint32_t opt=0;
        setsockopt(lsocket[i].skt ,0,SO_BLOCKMODE,&opt,4);//non block
        //opt = MAX_CLIENT_NUM;
        //setsockopt(lsocket[i].skt ,0,TCP_MAX_CONN_NUM,&opt,1);//max client num
        addr.s_port = lsocket[i].port;
        bind(lsocket[i].skt, &addr, sizeof(addr));
        listen(lsocket[i].skt, 0);
        //NET_DBG("TCP server established at port: %d \r\n ", addr.s_port);
      }//end if(lsocket[i].socketType==TCP)
      
      //tcp client-----------------------------------------------------------------
      else if(lsocket[i].socketType==TCP&&
         lsocket[i].isServer==false)
      {
        if(pIPstr==NULL || strcmp(pIPstr,"255.255.255.255")==0) 
          {
            free(pIPstr);pIPstr=NULL;
            isRunningLgethostbyname_thread = false;
            lsocket[i].socketAction[0]=REQ_ACTION_DISCONNECT;continue;
          }
        if(lsocket[i].skt !=INVALID_HANDLE ) close(lsocket[i].skt);
        lsocket[i].skt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        lsocket[i].client[0]=lsocket[i].skt;
        uint32_t opt=0;
        setsockopt(lsocket[i].skt ,0,SO_BLOCKMODE,&opt,4);//non block
        addr.s_ip = inet_addr(pIPstr);
        addr.s_port=lsocket[i].port;
        //NET_DBG("tcp client connect to:%d,%s\r\n",mico_get_time(),pIPstr);
        isRunningLgethostbyname_thread = false;
        {//dns callback
          lnet_client_userdata* skt=NULL;
          lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref);
          if(lua_isuserdata(gL,-1)) skt = lua_touserdata(gL,-1);
            else { lua_pop(gL, 1);goto lcon;}
          if(skt==NULL) { lua_pop(gL, 1); goto lcon;}
          int tempRef = skt->cb_dns_found_ref;
          if(tempRef == LUA_NOREF) goto lcon;
           lua_pop(gL, 1);//remove it
             
          lua_rawgeti(gL, LUA_REGISTRYINDEX,tempRef);//get function
          lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref);
          lua_pushstring(gL,pIPstr);
          lua_call(gL, 2, 0);
          lua_gc(gL, LUA_GCRESTART, 0);
          //dns callback end
         }
         free(pIPstr);pIPstr=NULL;
      lcon:
        if(0==connect(lsocket[i].skt, &addr, sizeof(addr)))
        {//success
            //NET_DBG("connected success,%d\r\n",mico_get_time());
            lnet_client_userdata* skt=NULL;
            lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref);
            if(lua_isuserdata(gL,-1)) skt = lua_touserdata(gL,-1);
            else { lua_pop(gL, 1);continue;}
           if(skt==NULL) { lua_pop(gL, 1); continue;}
           int tempRef = skt->cb_connect_ref;
           if(tempRef == LUA_NOREF) continue;
           lua_pop(gL, 1);//remove it
           
           lua_rawgeti(gL, LUA_REGISTRYINDEX,tempRef);//get function
           lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref);
           lua_call(gL, 1, 0);
           lua_gc(gL, LUA_GCRESTART, 0);
        }
        else//failed
        {
          //NET_DBG("connected failed:%d\r\n",mico_get_time());          
          lsocket[i].socketAction[0]=REQ_ACTION_DISCONNECT;
        }
        
      }
      //udp server
      
      //udp client
      
    }
  }
//++++++++++++++++++++++++++++++++++++check status++++++++++++++++++++++++++++++++++++
  FD_ZERO(&readfds);
  for(i=0;i<MAX_SOCKET_NUM;i++)
  {
     //tcp server
     if(lsocket[i].skt != INVALID_HANDLE && 
        lsocket[i].socketType==TCP)
     {
       FD_SET(lsocket[i].skt, &readfds);
     }
     //tcp server - client
     for(k=0;k<MAX_CLIENT_NUM;k++)
     {
       if(lsocket[i].client[k] !=INVALID_HANDLE)
         FD_SET(lsocket[i].client[k],&readfds);
     }
     //tcp client
     
     //udp server
     
     //udp client
     
  }
  select(1, &readfds, NULL, NULL, &t_val);
  
//++++++++++++++++++++++++++++++++++++check wheteth the events been set++++++++++++++++++++++++++++++++++++
  for(i=0;i<MAX_SOCKET_NUM;i++)
  {
    //tcp server
    if(lsocket[i].skt != INVALID_HANDLE && 
        lsocket[i].socketType==TCP&&lsocket[i].isServer == true&&
       (FD_ISSET(lsocket[i].skt, &readfds)))
    {
      char ip_address[16];
      int clientTmp = accept(lsocket[i].skt, &addr, &len);
      if (clientTmp > 0) 
       {//connected
          inet_ntoa(ip_address, addr.s_ip );
          //NET_DBG("Client %s:%d connected \r\n", ip_address, addr.s_port);
          for(k=0;k<MAX_CLIENT_NUM;k++) 
           {
             if (lsocket[i].client[k] == INVALID_HANDLE) 
             {
              if(lsocket[i].listen_cb==LUA_NOREF) break;
              if(!gL) break;
              //NET_DBG("lsocket:%d,k:%d,\r\n",i,k);
              lsocket[i].client[k]= clientTmp;
              lua_rawgeti(gL, LUA_REGISTRYINDEX, lsocket[i].listen_cb);// get function
              // create a new client object
              lnet_client_userdata *skt = (lnet_client_userdata *)lua_newuserdata(gL, sizeof(lnet_client_userdata));
              if(!skt) {lua_pop(gL, 1); break; }
              // set its metatable
              luaL_getmetatable(gL, "net.client");
              lua_setmetatable(gL, -2);
              
              skt->self_ref=LUA_NOREF;
              lua_pushvalue(gL, -1);
              skt->self_ref = luaL_ref(gL, LUA_REGISTRYINDEX);
              lsocket[i].udref_client[k] = skt->self_ref;
              
              skt->cb_connect_ref = LUA_NOREF;
              skt->cb_disconnect_ref = LUA_NOREF;
              skt->cb_receive_ref = LUA_NOREF;
              skt->cb_dns_found_ref = LUA_NOREF;
              skt->cb_reconnect_ref = LUA_NOREF;
              skt->cb_send_ref = LUA_NOREF;
              
              lua_pushstring(gL,ip_address);
              lua_pushinteger(gL,addr.s_port);
              lua_call(gL, 3, 0);//function(conn,ip,port)
              break;
             }
          }
        }
    }
//++++++++++++++++++++++++++++++++++++tcp server client++++++++++++++++++++++++++++++++++++
    for(k=0;k<MAX_CLIENT_NUM;k++)
    {
      //recieve event
      if(lsocket[i].client[k] !=INVALID_HANDLE&&
         lsocket[i].socketType==TCP&&lsocket[i].isServer == true&&
         FD_ISSET(lsocket[i].client[k], &readfds))
      {
        int retlen = recv(lsocket[i].client[k], recvBuf, MAX_RECV_LEN, 0);
          if(retlen>0)//receive succuss
          {
            //NET_DBG("recv success\r\n"); 
            lnet_client_userdata* skt=NULL;
            lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref_client[k]);
            if(lua_isuserdata(gL,-1)) skt = lua_touserdata(gL,-1);
            else { lua_pop(gL, 1);continue;}
           if(skt==NULL) { lua_pop(gL, 1); continue;}
           int tempRef = skt->cb_receive_ref;
           if(tempRef == LUA_NOREF) continue;
           lua_pop(gL, 1);//remove it
           
           lua_rawgeti(gL, LUA_REGISTRYINDEX,tempRef);//get function
           lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref_client[k]);
           lua_pushlstring(gL, recvBuf, retlen);
           lua_call(gL, 2, 0);
           lua_gc(gL, LUA_GCRESTART, 0);
          }
          else//receive failed
          {
            //NET_DBG("recv failed:%d,%d\r\n",i,k); 
            lsocket[i].socketAction[k]=REQ_ACTION_DISCONNECT;
          }
      }
      //sent event or disconnect event
      if(lsocket[i].client[k] !=INVALID_HANDLE&&
         lsocket[i].socketType==TCP&&lsocket[i].isServer == true&&
         lsocket[i].udref_client[k] !=LUA_NOREF&&
         lsocket[i].socketAction[k] !=NO_ACTION)
      {
           lnet_client_userdata* skt=NULL;
           lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref_client[k]);
           if(lua_isuserdata(gL,-1)) skt = lua_touserdata(gL,-1);
            else { lua_pop(gL, 1);lsocket[i].socketAction[k]=REQ_ACTION_DISCONNECT;continue;}
           if(skt==NULL) { lua_pop(gL, 1);lsocket[i].socketAction[k]=REQ_ACTION_DISCONNECT; continue;}
           int tempRef = LUA_NOREF;
           if(lsocket[i].socketAction[k]==REQ_ACTION_SENT)
             tempRef = skt->cb_send_ref;
           else if (lsocket[i].socketAction[k]==REQ_ACTION_DISCONNECT)
           {
             tempRef = skt->cb_disconnect_ref;
             if(lsocket[i].client[k] !=INVALID_HANDLE)
              close(lsocket[i].client[k]);
             lsocket[i].client[k] = INVALID_HANDLE;  
             if(tempRef == LUA_NOREF) 
               free_usrdata_serverClient(gL,skt,i,k);
           }
           if(tempRef == LUA_NOREF) {lua_pop(gL, 1);lsocket[i].socketAction[k]=REQ_ACTION_DISCONNECT;continue;}
           
           lua_pop(gL, 1);//remove it
           lua_rawgeti(gL, LUA_REGISTRYINDEX,tempRef);//get function
           lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref_client[k]);
           lua_call(gL, 1, 0);
           if(lsocket[i].socketAction[k]==REQ_ACTION_DISCONNECT)
           {//free it
            free_usrdata_serverClient(gL,skt,i,k);
           }
           lsocket[i].socketAction[k] = NO_ACTION;
      }      
    }
//++++++++++++++++++++++++++++++++++++tcp client++++++++++++++++++++++++++++++++++++
    if(lsocket[i].skt != INVALID_HANDLE && 
        lsocket[i].socketType==TCP && lsocket[i].isServer == false&&
       (FD_ISSET(lsocket[i].skt, &readfds)))
    {
          int retlen = recv(lsocket[i].skt, recvBuf, MAX_RECV_LEN, 0);
          if(retlen>0)//receive succuss
          {
            //NET_DBG("client recv success\r\n"); 
            lnet_client_userdata* skt=NULL;
            lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref);
            if(lua_isuserdata(gL,-1)) skt = lua_touserdata(gL,-1);
            else { lua_pop(gL, 1);continue;}
           if(skt==NULL) { lua_pop(gL, 1); continue;}
           int tempRef = skt->cb_receive_ref;
           if(tempRef == LUA_NOREF) continue;
           lua_pop(gL, 1);//remove it
           
           lua_rawgeti(gL, LUA_REGISTRYINDEX,tempRef);//get function
           lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref);
           lua_pushlstring(gL, recvBuf, retlen);
           lua_call(gL, 2, 0);
           lua_gc(gL, LUA_GCRESTART, 0);
          }
          else//receive failed
          {
            //NET_DBG("client recv failed:%d,0\r\n",i);
            lsocket[i].socketAction[0]= REQ_ACTION_DISCONNECT;
          }
    }
    if(lsocket[i].skt != INVALID_HANDLE && 
        lsocket[i].socketType==TCP && lsocket[i].isServer == false&&
         lsocket[i].udref !=LUA_NOREF&&
         lsocket[i].socketAction[0] !=NO_ACTION)
        {
          //NET_DBG("lsocket[%d].udref :%d\r\n",i,lsocket[i].udref);
          //NET_DBG("lsocket[%d].socketAction[0]:%d\r\n",i,lsocket[i].socketAction[0]);
          
           lnet_client_userdata* skt=NULL;
           lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref);
           if(lua_isuserdata(gL,-1)) 
             skt = lua_touserdata(gL,-1);
            else 
            { 
              lsocket[i].socketAction[0] = NO_ACTION;
              lua_pop(gL, 1);continue;
            }
           if(skt==NULL) 
           { 
             lsocket[i].socketAction[0] = NO_ACTION;
             lua_pop(gL, 1); continue;
           }
           int tempRef = LUA_NOREF;
           if(lsocket[i].socketAction[0]==REQ_ACTION_SENT)
             tempRef = skt->cb_send_ref;
           else if (lsocket[i].socketAction[0]==REQ_ACTION_DISCONNECT)
           {
             tempRef = skt->cb_disconnect_ref;
             if(lsocket[i].skt !=INVALID_HANDLE)
              close(lsocket[i].skt);
             lsocket[i].skt = INVALID_HANDLE;
             if(tempRef == LUA_NOREF) 
               free_usrdata_serverClient(gL,skt,i,0);
           }
           if(tempRef == LUA_NOREF) 
           {
             lua_pop(gL, 1);
             lsocket[i].socketAction[0] = NO_ACTION;
             continue;
           }
           
           //NET_DBG("skt->cb_send_ref:%d\r\n",skt->cb_send_ref);
           //NET_DBG("skt->cb_disconnect_ref:%d\r\n",skt->cb_disconnect_ref);
           lua_pop(gL, 1);//remove it           
           lua_rawgeti(gL, LUA_REGISTRYINDEX,tempRef);//get function
           lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[i].udref);
           lua_call(gL, 1, 0);
           if(lsocket[i].socketAction[0]==REQ_ACTION_DISCONNECT)
           {//free it
            free_usrdata_serverClient(gL,skt,i,0);
           }
           lsocket[i].socketAction[0] = NO_ACTION;
        }
    
//++++++++++++++++++++++++++++++++++++udp server++++++++++++++++++++++++++++++++++++
    
//++++++++++++++++++++++++++++++++++++udp client++++++++++++++++++++++++++++++++++++
  }
}

//skt:listen(port,function(c))
static int lnet_server_listen( lua_State* L )
{
  //NET_DBG("lnet_server_listen is called.\r\n");
  
  int stack = 1;
  lnet_server_userdata *nud = (lnet_server_userdata *)luaL_checkudata(L, stack, "net.server");
  //NET_DBG("nub->self_ref:%d\r\n",nud->self_ref);
  luaL_argcheck(L, nud, stack, "server expected");
  stack++;
  if(nud->self_ref == LUA_NOREF)
    luaL_error( L, "net had been closed" );
  
  if(lsocket[nud->socketIndex].socketType !=TCP ||
     lsocket[nud->socketIndex].isServer !=true)
    luaL_error( L, "tcp server is needed" );
  
  gL = L;
  int port = luaL_checkinteger( L, stack );
  //NET_DBG("port:%d\r\n",port);
  stack++;
  
  if (lua_type(L, stack) == LUA_TFUNCTION || lua_type(L, stack) == LUA_TLIGHTFUNCTION)
  {
      lua_pushvalue(L, stack);
      {
         if(lsocket[nud->socketIndex].listen_cb != LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, lsocket[nud->socketIndex].listen_cb);
        lsocket[nud->socketIndex].listen_cb = luaL_ref(L, LUA_REGISTRYINDEX);
        //NET_DBG("listen_cb_ref:%d\r\n",lsocket[nud->socketIndex].listen_cb);
      }
  }
  else if(lsocket[nud->socketIndex].socketType==TCP&&lsocket[nud->socketIndex].isServer ==true)
  luaL_error( L, "listen callback is needed" );
  
  lsocket[nud->socketIndex].reqSetup = true;
  lsocket[nud->socketIndex].port = port;
  //start timer
  if( !timer_net_is_started)
  {
    timer_net_is_started = true;
    mico_deinit_timer( &_timer_net);
    mico_init_timer(&_timer_net, 50,_timer_net_handle ,NULL);
    mico_start_timer(&_timer_net);
  }
  return 0;
}

//sk:on("receive",fun())
static int lnet_client_on( lua_State* L )
{
  //NET_DBG("lnet_client_on is called.\r\n");
  
  lnet_client_userdata *nud = (lnet_client_userdata *)luaL_checkudata(L, 1, "net.client");
  luaL_argcheck(L, nud, 1, "server expected");
  if(nud->self_ref == LUA_NOREF)
    luaL_error( L, "net had been closed" );
  
  size_t sl;
  const char *method = luaL_checklstring( L, 2, &sl );
  //NET_DBG("method:%s\r\n",method);
  if (method == NULL)
    return luaL_error( L, "wrong arg type" );
  
  luaL_checkanyfunction(L, 3);
  lua_pushvalue(L, 3);  //copy argument (func) to the top of stack
  
  if(strcmp(method,"connect")==0&&sl==strlen("connect"))
  {
    if( nud->cb_connect_ref !=LUA_NOREF)
      luaL_unref(L,LUA_REGISTRYINDEX,nud->cb_connect_ref);
    nud->cb_connect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  else if(strcmp(method,"receive")==0&&sl==strlen("receive"))
  {
    if( nud->cb_receive_ref !=LUA_NOREF)
      luaL_unref(L,LUA_REGISTRYINDEX,nud->cb_receive_ref);
    nud->cb_receive_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  else if(strcmp(method,"sent")==0&&sl==strlen("sent"))
  {
    if( nud->cb_send_ref !=LUA_NOREF)
      luaL_unref(L,LUA_REGISTRYINDEX,nud->cb_send_ref);
    nud->cb_send_ref = luaL_ref(L, LUA_REGISTRYINDEX);      
  }
  else if(strcmp(method,"disconnect")==0&&sl==strlen("disconnect"))
  {
    if( nud->cb_disconnect_ref !=LUA_NOREF)
      luaL_unref(L,LUA_REGISTRYINDEX,nud->cb_disconnect_ref);
    nud->cb_disconnect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  else if(strcmp(method,"dnsfound")==0&&sl==strlen("dnsfound"))
  {
    if( nud->cb_dns_found_ref !=LUA_NOREF)
      luaL_unref(L,LUA_REGISTRYINDEX,nud->cb_dns_found_ref);
    nud->cb_dns_found_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  return 0;
}
//sk:send(data,fun())
static int lnet_client_send( lua_State* L )
{
  //NET_DBG("lnet_client_send is called\r\n");
  lnet_client_userdata *nud = (lnet_client_userdata *)luaL_checkudata(L, 1, "net.client");
  //NET_DBG("nub->self_ref:%d\r\n",nud->self_ref);
  luaL_argcheck(L, nud, 1, "client expected");
  if(nud->self_ref == LUA_NOREF)
    luaL_error( L, "net had been closed" );
  
  size_t len=0;
  const char *data = luaL_checklstring( L, 2, &len );
  if (len>1024 || data == NULL)
    return luaL_error( L, "data need <1024" );  
  if (lua_type(L, 3) == LUA_TFUNCTION || lua_type(L, 3) == LUA_TLIGHTFUNCTION)
  {
    lua_pushvalue(L, 3);
    if(nud->cb_send_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, nud->cb_send_ref);
    nud->cb_send_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  
  //get socketIndex
  int socketIndex = getSocketIndex(nud->self_ref);
  int clientIndex=0;
  int i=0,j=0;
  if(socketIndex == -1)
  {//get tcp server client
    for(i=0;i<MAX_SOCKET_NUM;i++)
      for(j=0;j<MAX_CLIENT_NUM;j++)
      {
        if(nud->self_ref ==lsocket[i].udref_client[j] )
        {
          socketIndex = i;
          clientIndex = j;
         break;           
        }
      }
  }
  
  if(socketIndex== -1 &&
     (i==MAX_SOCKET_NUM&&
     j==MAX_CLIENT_NUM)) 
    luaL_error( L, "net has not beed created" );
  
  //end of get socketIndex
  
  if(lsocket[socketIndex].socketType==TCP&&
     lsocket[socketIndex].isServer==true)
  {
     if(lsocket[socketIndex].client[clientIndex] !=INVALID_HANDLE)
        {
          int s = send(lsocket[socketIndex].client[clientIndex],data,len,0);
          if(s ==len)
            {//send sucess call sent function
              //NET_DBG("send success\r\n");
             lsocket[socketIndex].socketAction[clientIndex]=REQ_ACTION_SENT;
            }
            else
            {//send failed
               //NET_DBG("send failed\r\n");
               lsocket[socketIndex].socketAction[clientIndex]=REQ_ACTION_DISCONNECT; 
            }
          }
  }
  else if(lsocket[socketIndex].socketType==TCP&&
     lsocket[socketIndex].isServer==false)
  {
    if(lsocket[socketIndex].skt !=INVALID_HANDLE)
    {
        int s = send(lsocket[socketIndex].skt,data,len,0);
        if(s ==len)
          {//send sucess call sent function
            //NET_DBG("send success\r\n");
            lsocket[socketIndex].socketAction[0]=REQ_ACTION_SENT;
            }
            else
            {//send failed
               //NET_DBG("send failed");
               lsocket[socketIndex].socketAction[0]=REQ_ACTION_DISCONNECT; 
            }
    }
  }

  return 0;
}
static int lnet_client_close( lua_State* L )
{
  //NET_DBG("lnet_client_close is called.\r\n");

  int stack = 1;
  lnet_client_userdata *nud = (lnet_client_userdata *)luaL_checkudata(L, stack, "net.client");
  luaL_argcheck(L, nud, stack, "client expected");
  if(nud->self_ref == LUA_NOREF)
    luaL_error( L, "net had been closed" );

  
  //get socketIndex
  int socketIndex = getSocketIndex(nud->self_ref);
  int i=0,j=0;
  if(socketIndex == -1)
  {//get tcp server client
    for(i=0;i<MAX_SOCKET_NUM;i++)
      for(j=0;j<MAX_CLIENT_NUM;j++)
      {
        if(nud->self_ref ==lsocket[i].udref_client[j] )
        {//
          if(lsocket[i].socketType==TCP&&
           lsocket[i].isServer==true)
          {
              free_usrdata_serverClient(L,nud,i,j);    
              if(lsocket[i].client[j] != INVALID_HANDLE)
              close(lsocket[i].client[j]);
             lsocket[i].client[j] = INVALID_HANDLE;
             //unref all
          lua_gc(L, LUA_GCSTOP, 0);
          if(nud->self_ref !=LUA_NOREF)
            luaL_unref(L, LUA_REGISTRYINDEX, nud->self_ref);
          nud->self_ref =LUA_NOREF;
          lua_pop(L, 1);//remove it
          lua_gc(L, LUA_GCRESTART, 0);
          return 0;
          }          
        }
      }
  }
  
  if(socketIndex== -1 &&
     (i==MAX_SOCKET_NUM&&
     j==MAX_CLIENT_NUM)) 
    luaL_error( L, "net has not beed created" );
  
  if(lsocket[socketIndex].socketType==TCP&&
     lsocket[socketIndex].isServer==false)
  {
      free_usrdata_serverClient(L,nud,socketIndex,0);    
      if(lsocket[socketIndex].skt != INVALID_HANDLE)
      close(lsocket[socketIndex].skt);
      lsocket[socketIndex].skt = INVALID_HANDLE;
  }
  
  //unref all
  lua_gc(L, LUA_GCSTOP, 0);
  if(nud->self_ref !=LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, nud->self_ref);
  nud->self_ref =LUA_NOREF;
  
  lua_pop(L, 1);//remove it
  lua_gc(L, LUA_GCRESTART, 0);
  
  return 0;
}
static int lnet_server_close( lua_State* L )
{
  //NET_DBG("lnet_server_close is called.\r\n");
  //stack_dump(L);
  int stack = 1;
  const char *mt = "net.server";
  lnet_server_userdata *nud = (lnet_server_userdata *)luaL_checkudata(L, stack, mt);
  //NET_DBG("nub->self_ref:%d\r\n",nud->self_ref);
  luaL_argcheck(L, nud, stack, "server expected");
  if(nud->self_ref == LUA_NOREF)
    luaL_error( L, "net had been closed" );
  
  //unref all
  int n = lua_gettop(L);
  lnet_client_userdata *skt = NULL;
  for(int i=0;i<MAX_CLIENT_NUM;i++)
  {//unref all cb
    int n = lua_gettop(L);
    if(lsocket[nud->socketIndex].udref_client[i] !=LUA_NOREF)
    {
      lua_rawgeti(gL, LUA_REGISTRYINDEX,lsocket[nud->socketIndex].udref_client[i]);
      if(lua_isuserdata(L,-1)) skt = lua_touserdata(L,-1);
        else { lua_pop(L, 1);continue;  }
      
     if(skt==NULL) { lua_pop(L, 1); continue;}
      free_usrdata_serverClient(L,skt,nud->socketIndex,i);
     lua_settop(L, n);// reset the stack top
     if(lsocket[nud->socketIndex].client[i] !=INVALID_HANDLE)
        close(lsocket[nud->socketIndex].client[i]);
     lsocket[nud->socketIndex].client[i] = INVALID_HANDLE;  
    }
   }
  //close socket
  if(lsocket[nud->socketIndex].skt != INVALID_HANDLE)
  close(lsocket[nud->socketIndex].skt);

  if(lsocket[nud->socketIndex].listen_cb !=LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, lsocket[nud->socketIndex].listen_cb);
  lsocket[nud->socketIndex].listen_cb = LUA_NOREF;

  lua_gc(L, LUA_GCSTOP, 0);
  if(nud->self_ref !=LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, nud->self_ref);  
  nud->self_ref =LUA_NOREF;
  
  lua_pop(L, 1);//remove it
  lua_gc(L, LUA_GCRESTART, 0);
  return 0;
}


//sk:connect(9003,"lewei50.com")
static int lnet_client_connect( lua_State* L )
{
  //NET_DBG("lnet_client_connect is called.\r\n");
  
  if(isRunningLgethostbyname_thread==true)
    luaL_error( L, "waiting for dns finished" );
  //tcp client
  int stack = 1;
  lnet_client_userdata *nud = (lnet_client_userdata *)luaL_checkudata(L, stack, "net.client");
  //NET_DBG("nub->self_ref:%d\r\n",nud->self_ref);
  luaL_argcheck(L, nud, stack, "client expected");
  stack++;
  if(nud->self_ref == LUA_NOREF)
    luaL_error( L, "net had been closed" );
  
  //get socketIndex
  int socketIndex = getSocketIndex(nud->self_ref);
  if(socketIndex == -1)
    luaL_error( L, "net has not beed created" );
      
  if(lsocket[socketIndex].socketType !=TCP ||
     lsocket[socketIndex].isServer !=false)
    luaL_error( L, "tcp client is needed" );
  
  gL = L;
  int port = luaL_checkinteger( L, stack );
  //NET_DBG("port:%d\r\n",port);
  stack++;
  
  size_t len=0;
  if( pDomain4Dns !=NULL) {free( pDomain4Dns);pDomain4Dns=NULL;}
  const char *domain = luaL_checklstring( L, stack, &len );
  if (len>128 || domain == NULL)
    return luaL_error( L, "need < 128 domain" );
  
  pDomain4Dns=(char*)malloc(len);
  pIPstr=(char*)malloc(16);
  memset(pIPstr,0x00,16);
  strcpy(pDomain4Dns,domain);
  //NET_DBG("pDomain4Dns:%s\r\n",pDomain4Dns); 
  lsocket[socketIndex].port = port;
  lsocket[socketIndex].reqSetup = false;
  socketIndexTemp = socketIndex;
  
  //setup a thread to get ip address
  mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "gethostip", lgethostbyname_thread, 0x300,NULL);
  
  if( !timer_net_is_started)
  {
    timer_net_is_started = true;
    mico_deinit_timer( &_timer_net);
    mico_init_timer(&_timer_net, 50,_timer_net_handle ,NULL);
    mico_start_timer(&_timer_net);
  }
  return 0;
}

// skt = net.new(net.TCP,net.Server)
static int lnet_new( lua_State* L )
{
  //NET_DBG("lnet_new is called.\r\n");
  
  int socketType = luaL_checkinteger( L, 1 );
  if(socketType !=TCP && socketType !=UDP )
   return  luaL_error( L, "wrong arg type, net.TCP or net.UDP is needed" );
  
  int socketType2 = luaL_checkinteger( L, 2);
  if(socketType2 != SOCKET_SERVER && socketType2 !=SOCKET_CLIENT )
   return  luaL_error( L, "wrong arg type, net.SERVER or net.CLIENT is needed" );
  
  int socketIndex=0;
  for(socketIndex=0;socketIndex<MAX_SOCKET_NUM;socketIndex++)
    if(lsocket[socketIndex].udref ==LUA_NOREF)
      break;
  if(socketIndex>=MAX_SOCKET_NUM)
   return luaL_error( L, "MAX_SOCKET_NUM reached" );

  if(socketType ==TCP&& socketType2==SOCKET_SERVER)
  {
    lnet_server_userdata *nud = (lnet_server_userdata *)lua_newuserdata(L, sizeof(lnet_server_userdata));
    // set its metatable
    luaL_getmetatable(L, "net.server");
    lua_setmetatable(L, -2);
    
    lua_pushvalue(L, -1);// copy the top of stack
    nud->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lsocket[socketIndex].listen_cb = LUA_NOREF;
    nud->socketIndex = socketIndex;
    lsocket[socketIndex].skt = INVALID_HANDLE;
    lsocket[socketIndex].udref = nud->self_ref;
    lsocket[socketIndex].socketType = socketType;
    lsocket[socketIndex].isServer = true;
    lsocket[socketIndex].reqSetup = false;
    for(int i=0;i<MAX_CLIENT_NUM;i++)
    {
      lsocket[socketIndex].client[i]=INVALID_HANDLE;
      lsocket[socketIndex].udref_client[i]=LUA_NOREF;
      lsocket[socketIndex].socketAction[i] = NO_ACTION;
    }
  }
  else if(socketType ==TCP&& socketType2==SOCKET_CLIENT)
  {
    lnet_client_userdata *nud = (lnet_client_userdata *)lua_newuserdata(L, sizeof(lnet_client_userdata));
    // set its metatable
    luaL_getmetatable(L, "net.client");
    lua_setmetatable(L, -2);
    
    lua_pushvalue(L, -1);// copy the top of stack
    nud->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    nud->cb_connect_ref = LUA_NOREF;
    nud->cb_reconnect_ref = LUA_NOREF;
    nud->cb_disconnect_ref = LUA_NOREF;
    nud->cb_receive_ref = LUA_NOREF;
    nud->cb_send_ref = LUA_NOREF;
    nud->cb_dns_found_ref = LUA_NOREF;
    
    lsocket[socketIndex].listen_cb = LUA_NOREF;
    lsocket[socketIndex].skt = INVALID_HANDLE;
    lsocket[socketIndex].udref = nud->self_ref;
    lsocket[socketIndex].socketType = socketType;
    lsocket[socketIndex].isServer = false;
    lsocket[socketIndex].client[0]=INVALID_HANDLE;
    lsocket[socketIndex].udref_client[0]=nud->self_ref;
    lsocket[socketIndex].socketAction[0] = NO_ACTION;
    lsocket[socketIndex].reqSetup = false;
    
    
  }

  return 1;
}

#define MIN_OPT_LEVEL       2
#include "lrodefs.h"
static const LUA_REG_TYPE net_server_map[] =
{
    { LSTRKEY( "listen" ), LFUNCVAL ( lnet_server_listen ) },
    { LSTRKEY( "close" ), LFUNCVAL ( lnet_server_close ) },
//  { LSTRKEY( "on" ), LFUNCVAL ( lnet_udpserver_on ) },
//  { LSTRKEY( "send" ), LFUNCVAL ( lnet_udpserver_send ) },
//  { LSTRKEY( "__gc" ), LFUNCVAL ( lnet_server_delete ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__index" ), LROVAL ( net_server_map ) },
#endif
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE net_client_map[] =
{
  { LSTRKEY( "connect" ), LFUNCVAL( lnet_client_connect ) },
//  { LSTRKEY( "close" ), LFUNCVAL ( lnet_socket_close ) },
  { LSTRKEY( "on" ), LFUNCVAL ( lnet_client_on ) },
  { LSTRKEY( "send" ), LFUNCVAL ( lnet_client_send ) },
  { LSTRKEY( "close" ), LFUNCVAL ( lnet_client_close ) },
//  { LSTRKEY( "unhold" ), LFUNCVAL ( lnet_socket_unhold ) },
//  { LSTRKEY( "dns" ), LFUNCVAL ( lnet_socket_dns ) },
//  { LSTRKEY( "getpeer" ), LFUNCVAL ( lnet_socket_getpeer ) },
//  { LSTRKEY( "__gc" ), LFUNCVAL ( lnet_socket_delete ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__index" ), LROVAL ( net_client_map ) },
#endif
  { LNILKEY, LNILVAL }
};
const LUA_REG_TYPE net_map[] =
{
  { LSTRKEY( "new" ), LFUNCVAL ( lnet_new ) },
//  { LSTRKEY( "createConnection" ), LFUNCVAL ( lnet_createConnection ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "TCP" ), LNUMVAL( TCP ) },
  { LSTRKEY( "UDP" ), LNUMVAL( UDP ) },

  { LSTRKEY( "__metatable" ), LROVAL( net_map ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_net(lua_State *L)
{
  for(int i=0;i<MAX_SOCKET_NUM;i++)
  {
    lsocket[i].udref=LUA_NOREF;
    for(int k=0;k<MAX_CLIENT_NUM;k++)
    {
      lsocket[i].client[k]=INVALID_HANDLE;
      lsocket[i].udref_client[k]=LUA_NOREF;
      lsocket[i].socketAction[k] = NO_ACTION;
    }        
  }
  set_tcp_keepalive(3, 60);
#if LUA_OPTIMIZE_MEMORY > 0
  luaL_rometatable(L, "net.server", (void *)net_server_map);  // create metatable for net.server
  luaL_rometatable(L, "net.client", (void *)net_client_map);  // create metatable for net.client
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, EXLIB_NET, net_map );
  // Set it as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );

  // Module constants  
  MOD_REG_NUMBER( L, "TCP", TCP );
  MOD_REG_NUMBER( L, "UDP", UDP );
  MOD_REG_NUMBER( L, "SERVER", SOCKET_SERVER);
  MOD_REG_NUMBER( L, "CLIENT", SOCKET_CLIENT);
  int n = lua_gettop(L);

  // create metatable
  luaL_newmetatable(L, "net.server");
  lua_pushliteral(L, "__index");
  lua_pushvalue(L,-2);
  lua_rawset(L,-3);
  // Setup the methods inside metatable
  luaL_register( L, NULL, net_server_map );

  lua_settop(L, n);
  // create metatable
  luaL_newmetatable(L, "net.client");
  // metatable.__index = metatable
  lua_pushliteral(L, "__index");
  lua_pushvalue(L,-2);
  lua_rawset(L,-3);
  // Setup the methods inside metatable
  luaL_register( L, NULL, net_client_map );
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}


