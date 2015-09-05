/**
 * net.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"

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

#define MAX_SVR_SOCKET 4
#define MAX_SVRCLT_SOCKET 5
#define MAX_CLT_SOCKET 4
enum _req_actions{
  NO_ACTION=0,
  REQ_ACTION_GOTIP,
  REQ_ACTION_SENT,
  REQ_ACTION_DISCONNECT
};
//for server-client
typedef struct {
  int client;//socket type
  uint8_t clientFlag;//sent or disconnect
  struct sockaddr_t addr;//ip and port 
}_lsvrCltsocket_t;
//for server
typedef struct {
  int socket;//socket type
  int port;//port
  uint8_t type;//TCP or UDP
  int accept_cb;
  int receive_cb;
  int sent_cb;
  int disconnect_cb;
  _lsvrCltsocket_t *psvrCltsocket[MAX_SVRCLT_SOCKET];
}svrsockt_t;
svrsockt_t *psvrsockt[MAX_SVR_SOCKET];

//for client
typedef struct _lcltsocket{
  int socket;// TCP or UDP 
  uint8_t type;//TCP or UDP
  struct sockaddr_t addr;//ip and port for tcp or udp use
  int connect_cb;
  int dnsfound_cb;
  int receive_cb;
  int sent_cb;
  int disconnect_cb;
  uint8_t clientFlag;//sent or disconnect or got ip
}cltsockt_t;
cltsockt_t *pcltsockt[MAX_CLT_SOCKET];

static lua_State *gL = NULL;
static char *pDomain4Dns=NULL;
static mico_timer_t _timer_net;
#define MAX_RECV_LEN 1024
static char recvBuf[MAX_RECV_LEN];
static int clientIndexK=0;

// socket=net.new(net.TCP/UDP,net.SERVER/net.CLIENT)
static int lnet_new( lua_State* L )
{
  int protocalType = luaL_checkinteger( L, 1 );
  if(protocalType !=TCP && protocalType !=UDP )
   return  luaL_error( L, "wrong arg type, net.TCP or net.UDP is needed" );
  
  int socketType = luaL_checkinteger( L, 2);
  if(socketType != SOCKET_SERVER && socketType !=SOCKET_CLIENT )
   return  luaL_error( L, "wrong arg type, net.SERVER or net.CLIENT is needed" );
    
  int socketHandle=INVALID_HANDLE;
  if(protocalType==TCP)
    socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  else
    socketHandle = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
  
  if(socketType==SOCKET_SERVER)
  {//server
    int k=0;
    for(k=0;k<MAX_SVR_SOCKET;k++){
      if(psvrsockt[k]==NULL) break;
    }
    if(k==MAX_SVR_SOCKET) return luaL_error( L, "Max SOCKET Number is reached" );
    psvrsockt[k] = (svrsockt_t*)malloc(sizeof(svrsockt_t));
    if (psvrsockt[k] ==NULL) return luaL_error( L, "memery allocated failed" );
    psvrsockt[k]->socket = socketHandle;
    psvrsockt[k]->port = INVALID_HANDLE;
    psvrsockt[k]->type = protocalType;
    psvrsockt[k]->accept_cb = LUA_NOREF;
    psvrsockt[k]->receive_cb = LUA_NOREF;
    psvrsockt[k]->sent_cb = LUA_NOREF;
    psvrsockt[k]->disconnect_cb = LUA_NOREF;
    for(int m=0;m<MAX_SVRCLT_SOCKET;m++){
      //psvrsockt[k]->psvrCltsocket[m]->client=INVALID_HANDLE;
      //psvrsockt[k]->psvrCltsocket[m]->clientFlag = NO_ACTION;
      psvrsockt[k]->psvrCltsocket[m] = NULL;
    }
  }
  else
  {//client
    int k=0;
    for(k=0;k<MAX_CLT_SOCKET;k++){
      if(pcltsockt[k]==NULL) break;
    }
    if(k==MAX_CLT_SOCKET) return luaL_error( L, "Max SOCKET Number is reached" );
    pcltsockt[k] = (cltsockt_t*)malloc(sizeof(cltsockt_t));
    if (pcltsockt[k] ==NULL) return luaL_error( L, "memery allocated failed" );
    pcltsockt[k]->socket = socketHandle;
    pcltsockt[k]->type = protocalType;
    pcltsockt[k]->connect_cb = LUA_NOREF;
    pcltsockt[k]->dnsfound_cb = LUA_NOREF;
    pcltsockt[k]->receive_cb = LUA_NOREF;
    pcltsockt[k]->sent_cb = LUA_NOREF;
    pcltsockt[k]->disconnect_cb = LUA_NOREF;
    pcltsockt[k]->clientFlag = NO_ACTION;
   
  }
  
   if(socketHandle==INVALID_HANDLE)
     lua_pushnil(L);
   else
     lua_pushinteger(L,socketHandle);
    return 1;
}
enum {
  SOCKET_TYPE_SERVER=1,
  SOCKET_TYPE_SVRCLT,
  SOCKET_TYPE_CLIENT,
};
static bool getsocketIndex(int socketHandle, int *type,int *out1,int *out2)
{//socketHandle:socket
  //type: return value,SOCKET_TYPE_SERVER or SOCKET_TYPE_SVRCLT or SOCKET_TYPE_CLIENT
  //out1:return value
  //out2:return value
  int k=0,m=0;
  *out1=0;
  *out2=0;
  for(k=0;k<MAX_SVR_SOCKET;k++){
    if(psvrsockt[k] ==NULL) continue;
      if(psvrsockt[k]->socket == socketHandle){
        *type = SOCKET_TYPE_SERVER;
        *out1 = k;
      return true;
      }
    for(m=0;m<MAX_SVRCLT_SOCKET;m++){
      if(psvrsockt[k]->psvrCltsocket[m]==NULL) continue;
      if(psvrsockt[k]->psvrCltsocket[m]->client==socketHandle){
        *type = SOCKET_TYPE_SVRCLT;
        *out1 = k;
        *out2 = m;
         return true;
      }
     }
  }
  for(k=0;k<MAX_CLT_SOCKET;k++){
      if(pcltsockt[k] ==NULL) continue;
      if(pcltsockt[k]->socket == socketHandle){
        *type = SOCKET_TYPE_CLIENT;
        *out1 = k;
        return true;
      }
  }
  return false;
}
static void closeSocket(lua_State*L, int socketHandle)
{
  //if socketHandle is server or serverclient
    int k=0,m=0;
    for(k=0;k<MAX_SVR_SOCKET;k++){
      if(psvrsockt[k] ==NULL) continue;
      if(psvrsockt[k]->socket == socketHandle){
        //close all serverClient
        //close server socket
        //unref cb function
        //free memery
        for(m=0;m<MAX_SVRCLT_SOCKET;m++){
          if(psvrsockt[k]->psvrCltsocket[m] ==NULL) continue;
          if(psvrsockt[k]->type==TCP&&
             psvrsockt[k]->psvrCltsocket[m]->client !=INVALID_HANDLE)
              close(psvrsockt[k]->psvrCltsocket[m]->client);
          free(psvrsockt[k]->psvrCltsocket[m]);
          psvrsockt[k]->psvrCltsocket[m]=NULL;
        }
        if(psvrsockt[k]->accept_cb!= LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, psvrsockt[k]->accept_cb);
        psvrsockt[k]->accept_cb = LUA_NOREF;
        if(psvrsockt[k]->receive_cb!= LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, psvrsockt[k]->receive_cb);
        psvrsockt[k]->receive_cb = LUA_NOREF;
        if(psvrsockt[k]->sent_cb!= LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, psvrsockt[k]->sent_cb);
        psvrsockt[k]->sent_cb = LUA_NOREF;
        if(psvrsockt[k]->disconnect_cb!= LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, psvrsockt[k]->disconnect_cb);
        psvrsockt[k]->disconnect_cb = LUA_NOREF;
        
        close(socketHandle);
        free(psvrsockt[k]);
        psvrsockt[k] = NULL;
        return ;
      }
      for(m=0;m<MAX_SVRCLT_SOCKET;m++){
        if(psvrsockt[k]->psvrCltsocket[m]==NULL) continue;
        if(psvrsockt[k]->psvrCltsocket[m]->client==socketHandle){
        //close serverClient
        //free memery
          if(psvrsockt[k]->type==TCP)
            close(socketHandle);
          free(psvrsockt[k]->psvrCltsocket[m]);
          psvrsockt[k]->psvrCltsocket[m] = NULL;
          return ;
        }
      }
    }
 //if socketHandle is client
    for(k=0;k<MAX_CLT_SOCKET;k++){
      if(pcltsockt[k] ==NULL) continue;
      if(pcltsockt[k]->socket == socketHandle){
        //close client socket
        //unref cb function
        //free memery
        if(pcltsockt[k]->connect_cb!= LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, pcltsockt[k]->connect_cb);
        pcltsockt[k]->connect_cb = LUA_NOREF;
        if(pcltsockt[k]->dnsfound_cb!= LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, pcltsockt[k]->dnsfound_cb);
        pcltsockt[k]->dnsfound_cb = LUA_NOREF;
        if(pcltsockt[k]->receive_cb!= LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, pcltsockt[k]->receive_cb);
        pcltsockt[k]->receive_cb = LUA_NOREF;
        if(pcltsockt[k]->sent_cb!= LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, pcltsockt[k]->sent_cb);
        pcltsockt[k]->sent_cb = LUA_NOREF;
        if(pcltsockt[k]->disconnect_cb!= LUA_NOREF)
          luaL_unref(L, LUA_REGISTRYINDEX, pcltsockt[k]->disconnect_cb);
        pcltsockt[k]->disconnect_cb = LUA_NOREF;
        pcltsockt[k]->clientFlag = NO_ACTION;
        close(socketHandle);
        free(pcltsockt[k]);
        pcltsockt[k] = NULL;
        return ;
      }
    }
}
static void lgethostbyname_thread(void *inContext)
{
  int k = clientIndexK;
  char pIPstr[16]={0};
  //MCU_DBG("lgethostbyname_thread called:%d",k);
  
  if(pDomain4Dns ==NULL) goto exit;

  gethostbyname((char *)pDomain4Dns, (uint8_t *)pIPstr, 16);
  free(pDomain4Dns);pDomain4Dns=NULL;
  
  if(pcltsockt[k] !=NULL)
  {
    pcltsockt[k]->clientFlag = REQ_ACTION_GOTIP;
    pcltsockt[k]->addr.s_ip = inet_addr(pIPstr);
  }
  
exit:
  mico_rtos_delete_thread(NULL);
}

//net.close(socket)
static int lnet_close( lua_State* L )
{
  int socketHandle = luaL_checkinteger( L, 1 );
  closeSocket(L, socketHandle);
  return 0;
}

void _micoNotify_TCPClientConnectedHandler(int fd)
{
  int type=0,k=0,m=0;
  if(false == getsocketIndex(fd,&type,&k,&m))
  {
    //MCU_DBG("socket is not valid\r\n" );
    return;
  }
  if(pcltsockt[k] ==NULL) return;;
  if(pcltsockt[k]->connect_cb == LUA_NOREF) return;
  lua_rawgeti(gL, LUA_REGISTRYINDEX,pcltsockt[k]->connect_cb);//function
  lua_pushinteger(gL,pcltsockt[k]->socket);//para1
  lua_call(gL, 1, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
}
/*
  step1:check if ACTION required  gotip/connect/disconnect
  step2:check if event is set
    2.1,tcpserver accept£ºnew a serverclt
    2.2,tcp serverclt recieve data or disconnect
    2.3,udp server:recieve or disconnect
    2.4,tcp client/udp client: or recieve or disconnect
*/
static void _timer_net_handle( void* arg )
{
//step 0
  static fd_set readset;
  static struct timeval_t t_val;
  t_val.tv_sec=0;
  t_val.tv_usec=10*1000;
//step 1
  int k=0,m=0;
  for(k=0;k<MAX_SVR_SOCKET;k++){
    if(psvrsockt[k] ==NULL) continue;
      if(psvrsockt[k]->socket != INVALID_HANDLE ){
        for(m=0;m<MAX_SVRCLT_SOCKET;m++){
          if(psvrsockt[k]->psvrCltsocket[m]==NULL) continue;
          if(psvrsockt[k]->psvrCltsocket[m]->client!= INVALID_HANDLE){
            //REQ_ACTION_SENT or REQ_ACTION_DISCONNECT
            if(psvrsockt[k]->psvrCltsocket[m]->clientFlag==REQ_ACTION_SENT){
              psvrsockt[k]->psvrCltsocket[m]->clientFlag=NO_ACTION;
              if(psvrsockt[k]->sent_cb == LUA_NOREF) continue;
              lua_rawgeti(gL, LUA_REGISTRYINDEX,psvrsockt[k]->sent_cb);//function
              lua_pushinteger(gL,psvrsockt[k]->psvrCltsocket[m]->client);//para1
              lua_call(gL, 1, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
            }//REQ_ACTION_DISCONNECT
            else if(psvrsockt[k]->psvrCltsocket[m]->clientFlag==REQ_ACTION_DISCONNECT){
              psvrsockt[k]->psvrCltsocket[m]->clientFlag=NO_ACTION;
              if(psvrsockt[k]->disconnect_cb != LUA_NOREF) {
                lua_rawgeti(gL, LUA_REGISTRYINDEX,psvrsockt[k]->disconnect_cb);//function
                lua_pushinteger(gL,psvrsockt[k]->psvrCltsocket[m]->client);//para1
                lua_call(gL, 1, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
              }
              closeSocket(gL, psvrsockt[k]->psvrCltsocket[m]->client);
            }
          }
         }//end for(m=0...
       }//end if(psvr...
  }
  for(k=0;k<MAX_CLT_SOCKET;k++){
      if(pcltsockt[k] ==NULL) continue;
      if(pcltsockt[k]->socket != INVALID_HANDLE){
        //REQ_ACTION_SENT or REQ_ACTION_DISCONNECT or REQ_ACTION_GOTIP
        if(pcltsockt[k]->clientFlag==REQ_ACTION_SENT){
          pcltsockt[k]->clientFlag=NO_ACTION;
          if(pcltsockt[k]->sent_cb == LUA_NOREF) continue;
          lua_rawgeti(gL, LUA_REGISTRYINDEX,pcltsockt[k]->sent_cb);//function
          lua_pushinteger(gL,pcltsockt[k]->socket);//para1
          lua_call(gL, 1, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
        }//REQ_ACTION_DISCONNECT
        else if(pcltsockt[k]->clientFlag==REQ_ACTION_DISCONNECT){
          pcltsockt[k]->clientFlag=NO_ACTION;
          if(pcltsockt[k]->disconnect_cb != LUA_NOREF){
            lua_rawgeti(gL, LUA_REGISTRYINDEX,pcltsockt[k]->disconnect_cb);//function
            lua_pushinteger(gL,pcltsockt[k]->socket);//para1
            lua_call(gL, 1, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
          }
          closeSocket(gL, pcltsockt[k]->socket);
        }//REQ_ACTION_GOTIP
        else if(pcltsockt[k]->clientFlag==REQ_ACTION_GOTIP){
          pcltsockt[k]->clientFlag=NO_ACTION;
          if(pcltsockt[k]->dnsfound_cb != LUA_NOREF){
            lua_rawgeti(gL, LUA_REGISTRYINDEX,pcltsockt[k]->dnsfound_cb);//function
            lua_pushinteger(gL,pcltsockt[k]->socket);//para1
            char ip[17];memset(ip,0x00,17);
            inet_ntoa(ip, pcltsockt[k]->addr.s_ip);
            lua_pushstring(gL,ip);//para2
            lua_call(gL, 2, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
          }
          //auto connect
          if(pcltsockt[k]->type==TCP){
            struct sockaddr_t *paddr=&(pcltsockt[k]->addr);
            int slen=sizeof(pcltsockt[k]->addr);
            //_micoNotify will be called if connected
            connect(pcltsockt[k]->socket, paddr, slen);
          }//udp client do not need connect
        }
      }
  }
//step 2
  //select all
  int maxfd = -1;
  FD_ZERO(&readset);
  for(k=0;k<MAX_SVR_SOCKET;k++){
    if(psvrsockt[k] ==NULL) continue;
       if(psvrsockt[k]->socket != INVALID_HANDLE ){
         //tcpserver accept(tcp only)
         //serverclt recieve or disconnect
         //if(psvrsockt[k]->type==TCP){
           if (psvrsockt[k]->socket > maxfd) 
             maxfd = psvrsockt[k]->socket;
           FD_SET(psvrsockt[k]->socket, &readset);
         //}
         
         for(m=0;m<MAX_SVRCLT_SOCKET;m++){
            if(psvrsockt[k]->psvrCltsocket[m]==NULL || 
               psvrsockt[k]->type==UDP||
               psvrsockt[k]->psvrCltsocket[m]->client==INVALID_HANDLE) 
              continue;
            if (psvrsockt[k]->psvrCltsocket[m]->client > maxfd) 
              maxfd = psvrsockt[k]->psvrCltsocket[m]->client;
            FD_SET(psvrsockt[k]->psvrCltsocket[m]->client, &readset);
         }
       }
  }//end for(k=0...
  for(k=0;k<MAX_CLT_SOCKET;k++){
    if(pcltsockt[k] ==NULL) continue;
    if(pcltsockt[k]->socket != INVALID_HANDLE){
    //tcp client/udp client: recieve or disconnect
       if (pcltsockt[k]->socket > maxfd) 
           maxfd = pcltsockt[k]->socket;
         FD_SET(pcltsockt[k]->socket, &readset);
    }
  }
  select(maxfd+1, &readset, NULL, NULL, &t_val);
  
  for(k=0;k<MAX_SVR_SOCKET;k++){
    if(psvrsockt[k] ==NULL) continue;
       if(psvrsockt[k]->socket != INVALID_HANDLE ){
         //tcpserver accept (tcp only)
         //udpserver recievefrom
         //serverclt recieve or disconnect
         if(psvrsockt[k]->type==TCP){
           if(FD_ISSET(psvrsockt[k]->socket, &readset))
           {
             struct sockaddr_t clientaddr;
             int len = sizeof(clientaddr);
             int clientTmp = accept(psvrsockt[k]->socket, &clientaddr, &len);
             if(clientTmp>0)
             {
               //get a new index
               //new a psvrCltsocket
               //call accept_cb
               int mi=0;
               for(mi=0;mi<MAX_SVRCLT_SOCKET;mi++){
                  if(psvrsockt[k]->psvrCltsocket[mi]==NULL) break;
                }
               if(mi==MAX_SVRCLT_SOCKET) {l_message(NULL, "Max SOCKET Client Number is reached" );continue;};
                psvrsockt[k]->psvrCltsocket[mi] = (_lsvrCltsocket_t*)malloc(sizeof(_lsvrCltsocket_t));
                if (psvrsockt[k]->psvrCltsocket[mi] ==NULL) { l_message(NULL, "memery allocated failed" );continue;}
                psvrsockt[k]->psvrCltsocket[mi]->client= clientTmp;
                psvrsockt[k]->psvrCltsocket[mi]->addr.s_ip= clientaddr.s_ip;
                psvrsockt[k]->psvrCltsocket[mi]->addr.s_port= clientaddr.s_port;
                psvrsockt[k]->psvrCltsocket[mi]->clientFlag= NO_ACTION;
                
                if(psvrsockt[k]->accept_cb != LUA_NOREF){
                  lua_rawgeti(gL, LUA_REGISTRYINDEX,psvrsockt[k]->accept_cb);//function
                  lua_pushinteger(gL,clientTmp);//para1
                  char ip_address[17];
                  memset(ip_address,0x00,17);
                  inet_ntoa(ip_address, clientaddr.s_ip);
                  lua_pushstring(gL,ip_address);//para2
                  lua_pushinteger(gL, clientaddr.s_port);//para3
                  lua_call(gL, 3, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
                }
             }
           }
         }
         else if(psvrsockt[k]->type==UDP)
         {//udp read  recvfrom   
          //get a new index/new a psvrClrtsocket/ call recieve_cb
           if(FD_ISSET(psvrsockt[k]->socket, &readset)){
             int recv_len=0;
             struct sockaddr_t clientaddr;
             int slen = sizeof(clientaddr);
             if(-1 == (recv_len = recvfrom(psvrsockt[k]->socket,
                                            recvBuf, 
                                            MAX_RECV_LEN-1, 
                                            0, 
                                            (struct sockaddr_t *) &clientaddr, 
                                            &slen)))
              {//if failed
                closeSocket(gL, psvrsockt[k]->socket);
                continue;
              }
             recvBuf[recv_len]=0x00;
             int mi=0;
             //if the aockaddr_t is the same
             for(mi=0;mi<MAX_SVRCLT_SOCKET;mi++){
               if(psvrsockt[k]->psvrCltsocket[mi] !=NULL&&
                  psvrsockt[k]->psvrCltsocket[mi]->addr.s_ip==clientaddr.s_ip) goto doUdpRecieve;
             }
             //else new one
             for(mi=0;mi<MAX_SVRCLT_SOCKET;mi++){
               if(psvrsockt[k]->psvrCltsocket[mi]==NULL) break;
             }
            if(mi==MAX_SVRCLT_SOCKET){//if reach max, return to 0
              closeSocket(gL, psvrsockt[k]->psvrCltsocket[mi]->client);
              mi=0;
            }
              psvrsockt[k]->psvrCltsocket[mi] = (_lsvrCltsocket_t*)malloc(sizeof(_lsvrCltsocket_t));
             if (psvrsockt[k]->psvrCltsocket[mi] == NULL) {l_message(NULL, "memery allocated failed" );continue;}
              psvrsockt[k]->psvrCltsocket[mi]->client= 32767 - mi;
              psvrsockt[k]->psvrCltsocket[mi]->addr.s_ip= clientaddr.s_ip;
              psvrsockt[k]->psvrCltsocket[mi]->addr.s_port= clientaddr.s_port;
              psvrsockt[k]->psvrCltsocket[mi]->clientFlag= NO_ACTION;
           doUdpRecieve://call recieve_cb
             if(psvrsockt[k]->receive_cb != LUA_NOREF) {
                  lua_rawgeti(gL, LUA_REGISTRYINDEX,psvrsockt[k]->receive_cb);//function
                  lua_pushinteger(gL,psvrsockt[k]->psvrCltsocket[mi]->client);//para1
                  lua_pushlstring(gL,recvBuf,recv_len);                       //para2
                  lua_call(gL, 2, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
             }
           }//if(FD_ISSET...
         }
         for(m=0;m<MAX_SVRCLT_SOCKET;m++){
            if(psvrsockt[k]->psvrCltsocket[m]==NULL || 
               psvrsockt[k]->type==UDP||
               psvrsockt[k]->psvrCltsocket[m]->client==INVALID_HANDLE) continue;
            //deal with tcp server client
            if(FD_ISSET(psvrsockt[k]->psvrCltsocket[m]->client, &readset)){
            //tcp read  recv
              int recv_len = recv(psvrsockt[k]->psvrCltsocket[m]->client, 
                                recvBuf, 
                                MAX_RECV_LEN-1,
                                0);
              if(recv_len<=0)
              {//failed
                psvrsockt[k]->psvrCltsocket[m]->clientFlag = REQ_ACTION_DISCONNECT;
                continue;
              }//else success call recieve_cb
              recvBuf[recv_len]=0x00;
              if(psvrsockt[k]->receive_cb != LUA_NOREF) {        
                lua_rawgeti(gL, LUA_REGISTRYINDEX,psvrsockt[k]->receive_cb);//function
                lua_pushinteger(gL,psvrsockt[k]->psvrCltsocket[m]->client);//para1
                lua_pushlstring(gL, recvBuf,recv_len);                     //para2
                lua_call(gL, 2, 0); 
                lua_gc(gL, LUA_GCCOLLECT, 0);
              }
            }//if(FD_ISSET...
         }
       }
  }//end for(k=0...
  for(k=0;k<MAX_CLT_SOCKET;k++){
    if(pcltsockt[k] ==NULL) continue;
    if(pcltsockt[k]->socket != INVALID_HANDLE){
    //tcp client/udp client: recieve or disconnect
      if(FD_ISSET(pcltsockt[k]->socket, &readset)){
        if(pcltsockt[k]->type==TCP)
        {//tcp client: recieve or disconnect
          int recv_len = recv(pcltsockt[k]->socket, 
                                recvBuf, 
                                MAX_RECV_LEN-1,
                                0);
          if(recv_len<=0)
              {//failed
                pcltsockt[k]->clientFlag = REQ_ACTION_DISCONNECT;
                continue;
              }//else success call recieve_cb
              recvBuf[recv_len]=0x00;
          if(pcltsockt[k]->receive_cb != LUA_NOREF){
                lua_rawgeti(gL, LUA_REGISTRYINDEX,pcltsockt[k]->receive_cb);//function
                lua_pushinteger(gL,pcltsockt[k]->socket);       //para1
                lua_pushlstring(gL,recvBuf,recv_len);                     //para2
                lua_call(gL, 2, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
            }
        }
        else if(pcltsockt[k]->type==UDP)
        {//udp client: recieve or disconnect
           int recv_len=0;
           struct sockaddr_t *pclientaddr=&pcltsockt[k]->addr;
             int slen = sizeof(*pclientaddr);
             if(-1 == (recv_len = recvfrom(pcltsockt[k]->socket,
                                            recvBuf, 
                                            MAX_RECV_LEN-1, 
                                            0, 
                                            (struct sockaddr_t *) pclientaddr, 
                                            &slen)))
              {//if failed
                closeSocket(gL, pcltsockt[k]->socket);
                continue;
              }
             recvBuf[recv_len]=0x00;
             if(pcltsockt[k]->receive_cb != LUA_NOREF){
                lua_rawgeti(gL, LUA_REGISTRYINDEX,pcltsockt[k]->receive_cb);//function
                lua_pushinteger(gL,pcltsockt[k]->socket);//para1
                lua_pushlstring(gL,recvBuf,recv_len);              //para2
                lua_call(gL, 2, 0); lua_gc(gL, LUA_GCCOLLECT, 0);
             }
        }
      }
    }
  }//for(k=0;..  
}
static void startNetTimer(void)
{
  static bool timer_net_is_started=false;
  //start timer
  if( !timer_net_is_started)
  {
    timer_net_is_started = true;
    mico_deinit_timer( &_timer_net);
    mico_init_timer(&_timer_net, 50,_timer_net_handle ,NULL);
    mico_start_timer(&_timer_net);
  }
}

//net.start(socket,port)
//net.start(socket,port,"domain",[local port])
static int lnet_start( lua_State* L )
{
  int socketHandle = luaL_checkinteger( L, 1 );
  int port = luaL_checkinteger( L, 2 );
  int type=0,k=0,m=0;
  if(false == getsocketIndex(socketHandle,&type,&k,&m))
    return luaL_error( L, "socket is not valid" );
  if(type==SOCKET_TYPE_SVRCLT)
    return luaL_error( L, "socket is not valid" );

  gL = L;

  if(type==SOCKET_TYPE_SERVER)
  {//server
     uint32_t opt=0;
     struct sockaddr_t addr;
     setsockopt(socketHandle,0,SO_BLOCKMODE,&opt,4);//non block
     //opt = MAX_CLIENT_NUM;
     //setsockopt(socketHandle,0,TCP_MAX_CONN_NUM,&opt,1);//max client num
     addr.s_ip = INADDR_ANY;
     addr.s_port = port;
     psvrsockt[k]->port = port;
     bind(socketHandle, &addr, sizeof(addr));
     if(psvrsockt[k]->type==TCP){
        listen(socketHandle, 0);
      }
     startNetTimer();
  }
  else
  {//client
    size_t len=0;
    if( pDomain4Dns !=NULL) {free( pDomain4Dns);pDomain4Dns=NULL;}
    const char *domain = luaL_checklstring( L, 3, &len );
    if (len>128 || domain == NULL)
      return luaL_error( L, "domain needed or its length < 128" );
    
    pDomain4Dns=(char*)malloc(len);
    if(pDomain4Dns==NULL) 
       return luaL_error( L, "memory allocated failed" );
    strcpy(pDomain4Dns,domain);
    //if assgiend local port
    if(lua_gettop(L)>=4)
    {
      int localPort = luaL_checkinteger( L, 4 );
      struct sockaddr_t addr;
      addr.s_ip = INADDR_ANY;
      addr.s_port = localPort;
      bind(socketHandle, &addr, sizeof(addr));
    }
    
    pcltsockt[k]->addr.s_port = port;
    uint32_t opt=0;
    setsockopt(socketHandle,0,SO_BLOCKMODE,&opt,4);//non block
    clientIndexK = k;
    //setup a thread to get ip address arg:socketHandle
    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "gethostip", lgethostbyname_thread, 0x300,NULL);
    startNetTimer();
    MICOAddNotification( mico_notify_TCP_CLIENT_CONNECTED, (void *)_micoNotify_TCPClientConnectedHandler );
  }
  
  return 0;
}
//server
//net.on(socket,"accept",accept_cb)  //(sktclt,ip,port)
//net.on(socket,"receive",receive_cb)//(sktclt,data)
//net.on(socket,"sent",sent_cb)//(sktclt)
//net.on(socket,"disconnect",disconnect_cb)//(sktclt)
//client
//net.on(socket,"dnsfound",dnsfound_cb)//(socket,ip)
//net.on(socket,"connect",connect_cb)//(socket)
//net.on(socket,"receive",receive_cb)//(socket,data)
//net.on(socket,"sent",sent_cb)//(socket)
//net.on(socket,"disconnect",disconnect_cb)//(socket)
static int lnet_on( lua_State* L )
{
  int socketHandle = luaL_checkinteger( L, 1 );
  int type=0,k=0,m=0;
  if(false == getsocketIndex(socketHandle,&type,&k,&m))
    return luaL_error( L, "socket is not valid" );
  if(type==SOCKET_TYPE_SVRCLT)
    return luaL_error( L, "socket is not valid" );
  
  size_t sl;
  const char *method = luaL_checklstring( L, 2, &sl );
  if (method == NULL)
    return luaL_error( L, "method string needed" );
  
  if (lua_type(L, 3) == LUA_TFUNCTION|| lua_type(L, 3)==LUA_TLIGHTFUNCTION)
      lua_pushvalue(L, 3);
  else
    return luaL_error( L, "wrong arg type" );
  
  if(type==SOCKET_TYPE_SERVER)
  {//server
    if(psvrsockt[k]->type==TCP && 
       strcmp(method,"accept")==0&&
       sl==strlen("accept"))
    {
      if(psvrsockt[k]->accept_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX,psvrsockt[k]->accept_cb);
      psvrsockt[k]->accept_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else if(strcmp(method,"receive")==0&&sl==strlen("receive"))
    {
      if(psvrsockt[k]->receive_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX,psvrsockt[k]->receive_cb);
      psvrsockt[k]->receive_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else if(strcmp(method,"sent")==0&&sl==strlen("sent"))
    {
      if(psvrsockt[k]->sent_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX,psvrsockt[k]->sent_cb);
      psvrsockt[k]->sent_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else if(strcmp(method,"disconnect")==0&&
            sl==strlen("disconnect"))
    {
      if(psvrsockt[k]->disconnect_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX,psvrsockt[k]->disconnect_cb);
      psvrsockt[k]->disconnect_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else
    {
      lua_pop(L, 1);
      return luaL_error( L, "unknown method" );
    }
  }
  else{//client
    if(strcmp(method,"dnsfound")==0&&sl==strlen("dnsfound"))
    {
      if(pcltsockt[k]->dnsfound_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX, pcltsockt[k]->dnsfound_cb);
      pcltsockt[k]->dnsfound_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else if(pcltsockt[k]->type==TCP && 
            strcmp(method,"connect")==0&&sl==strlen("connect"))
    {
      if(pcltsockt[k]->connect_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX, pcltsockt[k]->connect_cb);
      pcltsockt[k]->connect_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else if(strcmp(method,"receive")==0&&sl==strlen("receive"))
    {
      if(pcltsockt[k]->receive_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX, pcltsockt[k]->receive_cb);
      pcltsockt[k]->receive_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else if(strcmp(method,"sent")==0&&sl==strlen("sent"))
    {
      if(pcltsockt[k]->sent_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX, pcltsockt[k]->sent_cb);
      pcltsockt[k]->sent_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else if(strcmp(method,"disconnect")==0&&sl==strlen("disconnect"))
    {
      if(pcltsockt[k]->disconnect_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX, pcltsockt[k]->disconnect_cb);
      pcltsockt[k]->disconnect_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else
    {
      lua_pop(L, 1);
      return luaL_error( L, "unknown method" );
    }
  }
  return 0;
}

//net.send(socket,"data",[function_cb])
static int lnet_send( lua_State* L )
{
  int socketHandle = luaL_checkinteger( L, 1 );
  int type=0,k=0,m=0;
  if(false == getsocketIndex(socketHandle,&type,&k,&m))
    return luaL_error( L, "socket is not valid" );
  if(type==SOCKET_TYPE_SERVER)
    return luaL_error( L, "socket is not valid" );
  
  size_t len=0;
  const char *data = luaL_checklstring( L, 2, &len );
  if (len>1024 || data == NULL)
    return luaL_error( L, "data length must <= 1024" );  

  if (lua_type(L, 3) == LUA_TFUNCTION|| lua_type(L, 3)==LUA_TLIGHTFUNCTION)
  {
    lua_pushvalue(L, 3);
    if(type==SOCKET_TYPE_SVRCLT)
    {
       if(psvrsockt[k]->sent_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX,psvrsockt[k]->sent_cb);
       psvrsockt[k]->sent_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else
    {
      if(pcltsockt[k]->sent_cb!=LUA_NOREF)
        luaL_unref(L,LUA_REGISTRYINDEX, pcltsockt[k]->sent_cb);
      pcltsockt[k]->sent_cb = luaL_ref(L, LUA_REGISTRYINDEX);
    }
  }
  if(type==SOCKET_TYPE_SVRCLT)
  {//tcp or udp
    int s=0;
    if(psvrsockt[k]->type==TCP)
    {
      s = send(socketHandle,data,len,0);
      if(s ==len)
        {//send sucess call function_cb
          psvrsockt[k]->psvrCltsocket[m]->clientFlag=REQ_ACTION_SENT;
        }
        else
        {//send failed call function_cb
          psvrsockt[k]->psvrCltsocket[m]->clientFlag=REQ_ACTION_DISCONNECT; 
        }
    }
    else
    {//if its udp server: socketHandle=psvrsockt->psvrCltsocket->client = 32767-index     sentto(s,addr_from) 
      struct sockaddr_t *paddr =&(psvrsockt[k]->psvrCltsocket[m]->addr);
      s = sendto(psvrsockt[k]->socket,data,len,0,paddr,sizeof(*paddr));
      if(s ==len)
        {//send sucess call function_cb
          psvrsockt[k]->psvrCltsocket[m]->clientFlag=REQ_ACTION_SENT;
        }
        else
        {//send failed call function_cb
          psvrsockt[k]->psvrCltsocket[m]->clientFlag=REQ_ACTION_DISCONNECT; 
        }
    }
  }
  else
  {//tcp or udp
    int s=0;
    if(pcltsockt[k]->type==TCP)
    {
      s = send(socketHandle,data,len,0);
      if(s ==len)
        {//send sucess call function_cb
          pcltsockt[k]->clientFlag=REQ_ACTION_SENT;
        }
        else
        {//send failed call function_cb
          pcltsockt[k]->clientFlag=REQ_ACTION_DISCONNECT; 
        }
    }
    else
    {//if its udp client
      struct sockaddr_t *paddr = &(pcltsockt[k]->addr);
      s = sendto(socketHandle,data,len,0,paddr,sizeof(*paddr));
      if(s ==len)
        {//send sucess call function_cb
          pcltsockt[k]->clientFlag=REQ_ACTION_SENT;
        }
        else
        {//send failed call function_cb
          pcltsockt[k]->clientFlag=REQ_ACTION_DISCONNECT; 
        }
    }
  }
  return 0;
}
//ip,port = net.getip(clientSocket)
static int lnet_getip( lua_State* L )
{
  int socketHandle = luaL_checkinteger( L, 1 );
  int type=0,k=0,m=0;
  if(false == getsocketIndex(socketHandle,&type,&k,&m))
    return luaL_error( L, "socket is not valid" );
  if(type==SOCKET_TYPE_SERVER)
    return luaL_error( L, "required client socket" );
  
  if(type == SOCKET_TYPE_SVRCLT)
  {
    if(psvrsockt[k] !=NULL && psvrsockt[k]->psvrCltsocket[m] !=NULL)
    {
      char ip_address[17];
      memset(ip_address,0x00,17);
      inet_ntoa(ip_address, psvrsockt[k]->psvrCltsocket[m]->addr.s_ip);
      lua_pushstring(L,ip_address);
      lua_pushinteger(L,psvrsockt[k]->psvrCltsocket[m]->addr.s_port);
      return 2;
    }
  }
  else if(type == SOCKET_TYPE_CLIENT)
  {
    if(pcltsockt[k] !=NULL)
    {
      char ip_address[17];
      memset(ip_address,0x00,17);
      inet_ntoa(ip_address, pcltsockt[k]->addr.s_ip);
      lua_pushstring(L,ip_address);
      lua_pushinteger(L,pcltsockt[k]->addr.s_port);
      return 2;
    }    
  }    
  lua_pushnil(L);
  lua_pushnil(L);
  return 2;
}

#define MIN_OPT_LEVEL   2
#include "lrodefs.h"
const LUA_REG_TYPE net_map[] =
{
  {LSTRKEY("new"), LFUNCVAL(lnet_new)},
  {LSTRKEY("start"), LFUNCVAL(lnet_start)},
  {LSTRKEY("on"), LFUNCVAL(lnet_on)},
  {LSTRKEY("send"), LFUNCVAL(lnet_send)},
  {LSTRKEY("close"), LFUNCVAL(lnet_close)},
  {LSTRKEY("getip"), LFUNCVAL(lnet_getip)},
#if LUA_OPTIMIZE_MEMORY > 0
   { LSTRKEY( "TCP" ), LNUMVAL( TCP ) },
   { LSTRKEY( "UDP" ), LNUMVAL( UDP ) },
   { LSTRKEY( "SERVER" ), LNUMVAL( SOCKET_SERVER ) },
   { LSTRKEY( "CLIENT" ), LNUMVAL( SOCKET_CLIENT ) },
#endif        
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_net(lua_State *L)
{
  int i=0;
  for(i=0;i<MAX_SVR_SOCKET;i++)
  {
    psvrsockt[i] = NULL;
    for(int j=0;j<MAX_SVRCLT_SOCKET;j++)
      psvrsockt[i]->psvrCltsocket[j]=NULL;
  }
  for(i=0;i<MAX_CLT_SOCKET;i++)
    pcltsockt[i] = NULL;
    
  set_tcp_keepalive(3, 60);
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else  
  luaL_register( L, EXLIB_NET, net_map );
 
  MOD_REG_NUMBER( L, "TCP", TCP );
  MOD_REG_NUMBER( L, "UDP", UDP );
  MOD_REG_NUMBER( L, "SERVER", SOCKET_SERVER);
  MOD_REG_NUMBER( L, "CLIENT", SOCKET_CLIENT);
  return 1;
#endif
}
