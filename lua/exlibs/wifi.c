/**
 * wifi.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"

#include "platform.h"
#include "MICODefine.h"
#include "MicoWlan.h"
#include "MICONotificationCenter.h"

static lua_State *gL=NULL;
static int wifi_scan_succeed = LUA_NOREF;
static int wifi_status_changed_AP = LUA_NOREF;
static int wifi_status_changed_STA = LUA_NOREF;
static int wifi_smartconfig_finished = LUA_NOREF;

extern mico_queue_t os_queue;
enum{EASYLINK=0,AIRKISS};
char gWiFiSSID[33],gWiFiPSW[65];

static int smartConfigTimeout=3*60;//senconds
static int smartConfigMode=EASYLINK;//easylink:0, airkiss:1

extern mico_queue_t os_queue;
//airkiss
static mico_semaphore_t      smartconfig_sem=NULL;
static uint8_t airkiss_data=0xaa;
static void smartconfig_thread(void *inContext);
//easylink
static uint32_t FTC_IP=0;
#define FTC_PORT 8001

static int is_valid_ip(const char *ip) 
{
  int n[4];
  char c[4];
  if (sscanf(ip, "%d%c%d%c%d%c%d%c",
             &n[0], &c[0], &n[1], &c[1],
             &n[2], &c[2], &n[3], &c[3])
      == 7)
  {
    int i;
    for(i = 0; i < 3; ++i)
      if (c[i] != '.')
        return 0;
    for(i = 0; i < 4; ++i)
      if (n[i] > 255 || n[i] < 0)
        return 0;
    return 1;
  } else
    return 0;
}

void _micoNotify_WifiStatusHandler(WiFiEvent event, mico_Context_t * const inContext)
{
  (void)inContext;
  
  queue_msg_t msg;
  msg.L = gL;
  msg.source = WIFI;
switch (event) {
  case NOTIFY_STATION_UP:
    msg.para1 = 0;
    msg.para2 = wifi_status_changed_STA;
    break;
  case NOTIFY_STATION_DOWN:
    msg.para1 = 1;
    msg.para2 = wifi_status_changed_STA;
    break;
  case NOTIFY_AP_UP:
    msg.para1 = 2;
    msg.para2 = wifi_status_changed_AP;
    break;
  case NOTIFY_AP_DOWN:
    msg.para1 = 3;
    msg.para2 = wifi_status_changed_AP;
    break;
  default:
    msg.para1 = 4;
    msg.para2 = wifi_status_changed_AP;
    break;
  }
    mico_rtos_push_to_queue( &os_queue, &msg,0);
}
/*cfg={}
cfg.ssid=""
cfg.pwd=""
cfg.ip (optional,default:11.11.11.1)
cfg.netmask(optional,default:255.255.255.0)
cfg.gateway(optional,default:11.11.11.1)
cfg.dnsSrv(optional,default:11.11.11.1)
cfg.retry_interval(optional,default:1000ms)
wifi.startap(cfg,function(optional))*/
static int lwifi_startap( lua_State* L )
{//4 stations Max
  network_InitTypeDef_st wNetConfig;
  size_t len=0;
  
  memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));
  
  if (!lua_istable(L, 1))
    return luaL_error( L, "table arg needed" );
//ssid  
  lua_getfield(L, 1, "ssid");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the string
    {
      const char *ssid = luaL_checklstring( L, -1, &len );
      if(len>32)
        return luaL_error( L, "ssid:<32" );
      strncpy(wNetConfig.wifi_ssid,ssid,len);
    } 
    else
      return luaL_error( L, "wrong arg type:ssid" );
  }
  else
    return luaL_error( L, "arg: ssid needed" );
//pwd  
  lua_getfield(L, 1, "pwd");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the string
    {
      const char *pwd = luaL_checklstring( L, -1, &len );
      if(len>64)
        return luaL_error( L, "pwd:<64" );
      if(len>0)
      strncpy(wNetConfig.wifi_key,pwd,len);
      else
      strcpy(wNetConfig.wifi_key,"");  
    } 
    else
      return luaL_error( L, "wrong arg type:pwd" );
  }
  else
    return luaL_error( L, "arg: pwd needed" );
  
//ip  
  lua_getfield(L, 1, "ip");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the ssid string
    {
      const char *ip = luaL_checklstring( L, -1, &len );
      if(len>16)
        return luaL_error( L, "ip:<16" );
      if(is_valid_ip(ip)==false) 
        return luaL_error( L, "ip invalid" );
      strncpy(wNetConfig.local_ip_addr,ip,len);
    } 
    else
      return luaL_error( L, "wrong arg type:ip" );
  }
  else
  {
    strcpy(wNetConfig.local_ip_addr,"11.11.11.1");
    //return luaL_error( L, "arg: ip needed" );
  }
//netmask  
  lua_getfield(L, 1, "netmask");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the ssid string
    {
      const char *netmask = luaL_checklstring( L, -1, &len );
      if(len>16)
        return luaL_error( L, "netmask:<16" );
      if(is_valid_ip(netmask)==false) 
        return luaL_error( L, "netmask invalid" );
      strncpy(wNetConfig.net_mask,netmask,len);
    } 
    else
      return luaL_error( L, "wrong arg type:netmask" );
  }
  else
  {
    strcpy(wNetConfig.net_mask,"255.255.255.0");
    //return luaL_error( L, "arg: netmask needed" );
  }
//gateway  
  lua_getfield(L, 1, "gateway");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the ssid string
    {
      const char *gateway = luaL_checklstring( L, -1, &len );
      if(len>16)
        return luaL_error( L, "gateway:<16" );
      if(is_valid_ip(gateway)==false) 
        return luaL_error( L, "gateway invalid" );
      strncpy(wNetConfig.gateway_ip_addr,gateway,len);
    } 
    else
      return luaL_error( L, "wrong arg type:gateway" );
  }
  else
  {
    strcpy(wNetConfig.gateway_ip_addr,"11.11.11.1");
   // return luaL_error( L, "arg: gateway needed" );
  }
//dnsSrv  
  lua_getfield(L, 1, "dnsSrv");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the ssid string
    {
      const char *dnsSrv = luaL_checklstring( L, -1, &len );
      if(len>16)
        return luaL_error( L, "dnsSrv:<16" );
      if(is_valid_ip(dnsSrv)==false) 
        return luaL_error( L, "dnsSrv invalid" );
      strncpy(wNetConfig.dnsServer_ip_addr,dnsSrv,len);
    } 
    else
      return luaL_error( L, "wrong arg type:dnsSrv" );
  }
  else
  {
    strcpy(wNetConfig.dnsServer_ip_addr,"11.11.11.1");
    //return luaL_error( L, "arg: dnsSrv needed" );
  }
//retry_interval
  signed retry_interval=0;
  lua_getfield(L, 1, "retry_interval");
  if (!lua_isnil(L, -1)){  /* found? */
      retry_interval= luaL_checknumber( L, -1 );
      if(retry_interval<=0)
        return luaL_error( L, "retry_interval:>0ms" );
  }
  else
     retry_interval = 1000; 
  wNetConfig.wifi_retry_interval = retry_interval;
  
  /*MCU_DBG("wifi_ssid:%s\r\n",wNetConfig.wifi_ssid);
  MCU_DBG("wifi_key:%s\r\n",wNetConfig.wifi_key);
  MCU_DBG("local_ip_addr:%s\r\n",wNetConfig.local_ip_addr);
  MCU_DBG("net_mask:%s\r\n",wNetConfig.net_mask);
  MCU_DBG("gateway_ip_addr:%s\r\n",wNetConfig.gateway_ip_addr);
  MCU_DBG("dnsServer_ip_addr:%s\r\n",wNetConfig.dnsServer_ip_addr);
  MCU_DBG("wifi_retry_interval:%d\r\n",wNetConfig.wifi_retry_interval);*/
  
//notify
  gL = L;
  if (lua_type(L, 2) == LUA_TFUNCTION || lua_type(L, 2) == LUA_TLIGHTFUNCTION)
  {
    lua_pushvalue(L, 2);  // copy argument (func) to the top of stack
    if(wifi_status_changed_AP != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wifi_status_changed_AP);    
      
    wifi_status_changed_AP = luaL_ref(L, LUA_REGISTRYINDEX);
    MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)_micoNotify_WifiStatusHandler );
  } 
  else 
  {
    if(wifi_status_changed_AP != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wifi_status_changed_AP);
    wifi_status_changed_AP = LUA_NOREF;
  }
//start  
  wNetConfig.dhcpMode = DHCP_Server;  
  wNetConfig.wifi_mode = Soft_AP;
  micoWlanStart(&wNetConfig);  
  return 0;  
}

/*
cfg={}
cfg.ssid=""
cfg.pwd=""
cfg.dhcp=enable/disable(default:enable)
cfg.ip (depends on dhcp)
cfg.netmask (depends on dhcp)
cfg.gateway (depends on dhcp)
cfg.dnsSrv (depends on dhcp)
cfg.retry_interval(default:1000ms)
wifi.startap(cfg)*/
static int lwifi_startsta( lua_State* L )
{
  network_InitTypeDef_st wNetConfig;
  size_t len=0;
  
  memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));
  
  if (!lua_istable(L, 1))
    return luaL_error( L, "table arg needed" );
//ssid  
  lua_getfield(L, 1, "ssid");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the string
    {
      const char *ssid = luaL_checklstring( L, -1, &len );
      if(len>32)
        return luaL_error( L, "ssid:<32" );
      strncpy(wNetConfig.wifi_ssid,ssid,len);
    } 
    else
      return luaL_error( L, "wrong arg type:ssid" );
  }
  else
    return luaL_error( L, "arg: ssid needed" );
//pwd  
  lua_getfield(L, 1, "pwd");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the string
    {
      const char *pwd = luaL_checklstring( L, -1, &len );
      if(len>64)
        return luaL_error( L, "pwd:<64" );
      if(len>0)
      strncpy(wNetConfig.wifi_key,pwd,len);
      else
      strcpy(wNetConfig.wifi_key,"");  
    } 
    else
      return luaL_error( L, "wrong arg type:pwd" );
  }
  else
    return luaL_error( L, "arg: pwd needed" );
//dhcp
  wNetConfig.dhcpMode = DHCP_Client;
  lua_getfield(L, 1, "dhcp");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the string
    {
      const char *pwd = luaL_checklstring( L, -1, &len );
      if(strcmp(pwd,"disable")==0)
        wNetConfig.dhcpMode = DHCP_Disable;
    }   
    else
      return luaL_error( L, "wrong arg type:dhcp" );
  }

//ip  
  lua_getfield(L, 1, "ip");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the ssid string
    {
      const char *ip = luaL_checklstring( L, -1, &len );
      if(len>16)
        return luaL_error( L, "ip:<16" );
      if(is_valid_ip(ip)==false) 
        return luaL_error( L, "ip invalid" );
      strncpy(wNetConfig.local_ip_addr,ip,len);
    } 
    else
      return luaL_error( L, "wrong arg type:ip" );
  }
  else if(wNetConfig.dhcpMode == DHCP_Disable)
    return luaL_error( L, "arg: netmask needed" );
//netmask  
  lua_getfield(L, 1, "netmask");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the ssid string
    {
      const char *netmask = luaL_checklstring( L, -1, &len );
      if(len>16)
        return luaL_error( L, "netmask:<16" );
      if(is_valid_ip(netmask)==false) 
        return luaL_error( L, "netmask invalid" );
      strncpy(wNetConfig.net_mask,netmask,len);
    } 
    else
      return luaL_error( L, "wrong arg type:netmask" );
  }
  else if(wNetConfig.dhcpMode == DHCP_Disable)
    return luaL_error( L, "arg: netmask needed" );
  
//gateway  
  lua_getfield(L, 1, "gateway");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the ssid string
    {
      const char *gateway = luaL_checklstring( L, -1, &len );
      if(len>16)
        return luaL_error( L, "gateway:<16" );
      if(is_valid_ip(gateway)==false) 
        return luaL_error( L, "gateway invalid" );
      strncpy(wNetConfig.gateway_ip_addr,gateway,len);
    } 
    else
      return luaL_error( L, "wrong arg type:gateway" );
  }
  else if(wNetConfig.dhcpMode == DHCP_Disable)
    return luaL_error( L, "arg: gateway needed" );
  
//dnsSrv  
  lua_getfield(L, 1, "dnsSrv");
  if (!lua_isnil(L, -1)){  /* found? */
    if( lua_isstring(L, -1) )   // deal with the ssid string
    {
      const char *dnsSrv = luaL_checklstring( L, -1, &len );
      if(len>16)
        return luaL_error( L, "dnsSrv:<16" );
      if(is_valid_ip(dnsSrv)==false) 
        return luaL_error( L, "dnsSrv invalid" );
      strncpy(wNetConfig.dnsServer_ip_addr,dnsSrv,len);
    } 
    else
      return luaL_error( L, "wrong arg type:dnsSrv" );
  }
  else if(wNetConfig.dhcpMode == DHCP_Disable)
    return luaL_error( L, "arg: dnsSrv needed" );
  
//retry_interval
  signed retry_interval=0;
  lua_getfield(L, 1, "retry_interval");
  if (!lua_isnil(L, -1)){  /* found? */
      retry_interval= luaL_checknumber( L, -1 );
      if(retry_interval<=0)
        return luaL_error( L, "retry_interval:>0ms" );
  }
  else
     retry_interval = 1000; 
  wNetConfig.wifi_retry_interval = retry_interval;
  
  /*MCU_DBG("wifi_ssid:%s\r\n",wNetConfig.wifi_ssid);
  MCU_DBG("wifi_key:%s\r\n",wNetConfig.wifi_key);
  MCU_DBG("dhcpMode:%d\r\n",wNetConfig.dhcpMode);
  MCU_DBG("local_ip_addr:%s\r\n",wNetConfig.local_ip_addr);
  MCU_DBG("net_mask:%s\r\n",wNetConfig.net_mask);
  MCU_DBG("gateway_ip_addr:%s\r\n",wNetConfig.gateway_ip_addr);
  MCU_DBG("dnsServer_ip_addr:%s\r\n",wNetConfig.dnsServer_ip_addr);
  MCU_DBG("wifi_retry_interval:%d\r\n",wNetConfig.wifi_retry_interval);*/

//notify
    gL = L;
  if (lua_type(L, 2) == LUA_TFUNCTION || lua_type(L, 2) == LUA_TLIGHTFUNCTION)
  {
    lua_pushvalue(L, 2);  // copy argument (func) to the top of stack
    if(wifi_status_changed_STA != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wifi_status_changed_STA);    
      
    wifi_status_changed_STA = luaL_ref(L, LUA_REGISTRYINDEX);
    MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)_micoNotify_WifiStatusHandler );
  } 
  else 
  {
    if(wifi_status_changed_STA != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wifi_status_changed_STA);
    wifi_status_changed_STA = LUA_NOREF;
  }
//start  
  wNetConfig.wifi_mode = Station;
  micoWlanStart(&wNetConfig);
  return 0;  
}

void _micoNotify_WiFi_Scan_OK (ScanResult_adv *pApList, mico_Context_t * const inContext)
{
  (void)inContext;
  if(wifi_scan_succeed == LUA_NOREF)
    return;
  lua_rawgeti(gL, LUA_REGISTRYINDEX, wifi_scan_succeed);
   
  if(pApList->ApNum==0) 
  {
    lua_pushnil(gL);
  }
  else
  {
    char ssid[33];
    char temp[128];
    char buf_ptr[20];
    char security[12]="open";
    char *inBuf=NULL;
    lua_newtable( gL );
    for(int i=0;i<pApList->ApNum;i++)
    {
      //if(strlen(pApList->ApList[i].ssid)==0) continue;
      sprintf(ssid,pApList->ApList[i].ssid);
      //bssid
      inBuf = pApList->ApList[i].bssid;
      memset(buf_ptr,0x00,20);
      for (int j = 0; j < 6; j++)
      {
          if ( j == 6 - 1 )
          {
            sprintf(temp,"%02X",inBuf[j]);
            strcat(buf_ptr,temp);
          }
          else
          {
            sprintf(temp,"%02X:",inBuf[j]);
            strcat(buf_ptr,temp);
          }
      }
      switch(pApList->ApList[i].security)
      {
      case SECURITY_TYPE_NONE:          strcpy(security,"OPEN");break;
      case SECURITY_TYPE_WEP:           strcpy(security,"WEP");break;
      case SECURITY_TYPE_WPA_TKIP:      strcpy(security,"WPA TKIP");break;
      case SECURITY_TYPE_WPA_AES:       strcpy(security,"WPA AES");break;
      case SECURITY_TYPE_WPA2_TKIP:     strcpy(security,"WPA2 TKIP");break;
      case SECURITY_TYPE_WPA2_AES:      strcpy(security,"WPA2 AES");break;
      case SECURITY_TYPE_WPA2_MIXED:    strcpy(security,"WAP2 MIXED");break;
      case SECURITY_TYPE_AUTO:          strcpy(security,"AUTO");break;
      default:break;
      }       
      sprintf(temp,"%s,%d,%d,%s",buf_ptr,
                    pApList->ApList[i].ApPower,
                    pApList->ApList[i].channel,
                    security);
      lua_pushstring(gL, temp);//value
      lua_setfield( gL, -2, ssid );//key
    }
  }
  //free(pApList);
  lua_call(gL, 1, 0);
  return;
}
//function listap(t) if t then for k,v in pairs(t) do print(k.."\t"..v);end else print('no ap') end end wifi.scan(listap)
static int lwifi_scan( lua_State* L )
{
  if (lua_type(L, 1) == LUA_TFUNCTION || lua_type(L, 1) == LUA_TLIGHTFUNCTION)
  {
    lua_pushvalue(L, 1);  // copy argument (func) to the top of stack
    if(wifi_scan_succeed != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wifi_scan_succeed);
    
     wifi_scan_succeed = luaL_ref(L, LUA_REGISTRYINDEX);
     OSStatus err = MICOAddNotification( mico_notify_WIFI_SCAN_ADV_COMPLETED, (void *)_micoNotify_WiFi_Scan_OK );
     require_noerr( err, exit );
     micoWlanStartScanAdv();
     gL = L;
  } 
  else 
  {
    if(wifi_scan_succeed != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wifi_scan_succeed);
    wifi_scan_succeed = LUA_NOREF;
  }
 exit: 
    return 0;
}

void _smartconfigNotify_WifiStatusHandler(WiFiEvent event, mico_Context_t * const inContext)
{
  switch (event) {
  case NOTIFY_STATION_UP:
    mico_rtos_set_semaphore(&smartconfig_sem);
    break;
  case NOTIFY_STATION_DOWN:break;
  case NOTIFY_AP_UP:break;
  case NOTIFY_AP_DOWN:break;
  default:break;
  }
}
void _micoNotify_WiFi_smartconfig_finished (network_InitTypeDef_st *nwkpara, mico_Context_t * const inContext)
{
  (void)inContext;

  memset(gWiFiSSID,0x00,33);
  memset(gWiFiPSW,0x00,65);
  strncpy(gWiFiSSID,nwkpara->wifi_ssid,32);
  strncpy(gWiFiPSW,nwkpara->wifi_key,64);
  if(nwkpara==NULL) 
    gWiFiSSID[0]=0x00;
}
static void _micoNotify_WiFi_smartconfig_ExtraData(int datalen, char* data, mico_Context_t * const inContext)
{
  /*printf("[ExtraData](len:%d):\r\n",datalen);
  for(int i=0;i<datalen;i++)
  printf("data[%d]: %d(%x)\r\n",i,data[i],data[i]);
  printf("\r\n");*/
  if(smartConfigMode==AIRKISS)//airkiss
    airkiss_data=data[0];
  else//easylink
  {
    memcpy((char*)(&FTC_IP),&(data[1]),4);
    /*char address[16];
    inet_ntoa( address, FTC_IP);
    printf("EasyLink server ip address: %s\r\n",address);*/
  }
  mico_rtos_set_semaphore(&smartconfig_sem);
}

void _smartConfigConnectWiFi()
{
  network_InitTypeDef_adv_st wNetConfig;
  memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_adv_st));
  strcpy((char*)wNetConfig.ap_info.ssid,gWiFiSSID);
  wNetConfig.ap_info.security = SECURITY_TYPE_AUTO;
  strcpy(wNetConfig.key,gWiFiPSW);
  wNetConfig.key_len = strlen(gWiFiPSW);
  wNetConfig.dhcpMode = DHCP_Client;
  wNetConfig.wifi_retry_interval = 100;
  micoWlanStartAdv(&wNetConfig);
  //printf("connect to %s with %s.....\r\n", wNetConfig.ap_info.ssid,wNetConfig.key);
}
void _cleanSmartConfigResource( void )
{
  MICORemoveNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)_smartconfigNotify_WifiStatusHandler );
  MICORemoveNotification( mico_notify_EASYLINK_WPS_COMPLETED, (void *)_micoNotify_WiFi_smartconfig_finished );
  MICORemoveNotification( mico_notify_EASYLINK_GET_EXTRA_DATA, (void *)_micoNotify_WiFi_smartconfig_ExtraData );
  mico_rtos_deinit_semaphore(&smartconfig_sem);
  micoWlanSuspendStation();
  smartconfig_sem = NULL;
  micoWlanStopAirkiss();
  micoWlanStopEasyLink();
}
void smartconfig_thread(void *inContext)
{
  UNUSED_PARAMETER(inContext);
  OSStatus err = kNoErr;
  int fd;
  struct sockaddr_t addr;
  int i = 0;
  
  require_action(smartconfig_sem, threadExit, err = kNotPreparedErr);
  //printf("Start airkiss or easylink\r\n");
  if(smartConfigMode==AIRKISS)//airkiss
    micoWlanStartAirkiss(smartConfigTimeout);
  else
    micoWlanStartEasyLinkPlus(smartConfigTimeout);
  
  //printf("Wait for ssid and psw\r\n");
  mico_rtos_get_semaphore(&smartconfig_sem, (smartConfigTimeout)*1000);
  micoWlanStopAirkiss();
  micoWlanStopEasyLink();
  
  if(gWiFiSSID[0] != 0x00)
    _smartConfigConnectWiFi();
  else
  {
    //printf("Airkiss timeout!\r\n");
    goto threadExit_Timeout;
  }
  //printf("Try connect to sta\r\n");
  err = mico_rtos_get_semaphore(&smartconfig_sem, smartConfigTimeout*1000);
  if(err != kNoErr) 
  {
    //printf("Connect to sta timeout!\r\n");
    goto threadExit_Timeout;
  }
  net_para_st para;
  micoWlanGetIPStatus(&para, Station);
  //printf("Connect to sta ok, ip:%s\r\n",para.ip);
  //msleep(1000);
  //printf("Set udp socket \r\n");
    fd = socket( AF_INET, SOCK_DGRM, IPPROTO_UDP );
    //uint32_t opt=0;
    //setsockopt(fd,0,SO_BLOCKMODE,&opt,4);//non block
    if (fd < 0)
    {
      //printf("Create socket failed\r\n");
      goto threadExit_Timeout;
    }
    if(smartConfigMode==AIRKISS)
    {
      addr.s_ip = INADDR_BROADCAST;
      addr.s_port = 10000;
    }
    else
    {
      addr.s_ip   = FTC_IP; //FTC_IP;
      addr.s_port = FTC_PORT;
    }
    while(1){
      sendto(fd, &airkiss_data, 1, 0, &addr, sizeof(addr));
      msleep(10);
      i++;
      if (i > 10)
        break;
    }
    close(fd);
    goto threadExit;
  /*
  {//EASYLINK
    int i=0;
    for(i=0;i<20;i++)
    {
      fd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      struct sockaddr_t addr;
      addr.s_ip   = FTC_IP;
      addr.s_port = FTC_PORT;
      char address[16];
      inet_ntoa( address, FTC_IP);
      printf("EasyLink server ip address: %s, port:%d\r\n",address,FTC_PORT);
      
      err = connect(fd, &addr, sizeof(addr));
        if(err != kNoErr) 
        {
          printf("Connect to FTC server Failed! Try again:%d/20\r\n",i);
          close(fd);
        }
        else
        {
          break;
        }
        msleep(300);
    }
    if(i==20) 
    {
      printf("Connect to FTC server Failed!\r\n");
      goto threadExit_Timeout;
    }
    printf("Connect to FTC server OK!\r\n");
    char msg[]="ok";
    int s = send(fd,msg,strlen(msg),0);
    if(s==strlen(msg))
    {
      printf("Send to FTC ok\r\n");
    }
    else
    {
      printf("Send to FTC failed\r\n");
      goto threadExit_Timeout;
    }
    msleep(100);
    close(fd);
    goto threadExit;
  }*/
threadExit_Timeout:
  gWiFiSSID[0]=0x00;
threadExit:
  _cleanSmartConfigResource();
  
  queue_msg_t msg;
  msg.L = gL;
  msg.source = WIFI;
  msg.para1 = 5;
  msg.para2 = wifi_smartconfig_finished;
  mico_rtos_push_to_queue( &os_queue, &msg,0);
  
  mico_rtos_delete_thread( NULL );
  return;
}
//wifi.smartconfig(0,timeout,function(ssid,psw) end) //easylink: timeout in senconds
//wifi.smartconfig(1,timeout,function(ssid,psw) end) //airkiss: timeout in senconds
static int lwifi_smartconfig( lua_State* L )
{
  int mode = luaL_checkinteger( L, 1 );
  if(mode !=EASYLINK && mode !=AIRKISS)
    return luaL_error( L, "wrong arg type:mode is 0(easylink) or 1(airkiss)" );
  
  smartConfigMode=mode;//easylink:0, airkiss:1
  int timeout = luaL_checkinteger( L, 2 );
  if (timeout<=0)
    return luaL_error( L, "wrong arg type:timeout > 0");
  smartConfigTimeout = timeout;
  if (lua_type(L, 3) == LUA_TFUNCTION || lua_type(L, 3) == LUA_TLIGHTFUNCTION)
  {
    lua_pushvalue(L, 3);
    if(wifi_smartconfig_finished != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wifi_smartconfig_finished);
    
     wifi_smartconfig_finished = luaL_ref(L, LUA_REGISTRYINDEX);
     
     //micoWlanSuspendStation();
     //micoWlanSuspendSoftAP();
     
     MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)_smartconfigNotify_WifiStatusHandler );
     MICOAddNotification( mico_notify_EASYLINK_WPS_COMPLETED, (void *)_micoNotify_WiFi_smartconfig_finished );
     MICOAddNotification( mico_notify_EASYLINK_GET_EXTRA_DATA, (void *)_micoNotify_WiFi_smartconfig_ExtraData);
     gL = L;
     
     gWiFiSSID[0]=0x00;
     mico_rtos_init_semaphore(&smartconfig_sem, 1);
     mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Smartconfig", smartconfig_thread, 0x1000, NULL);
  } 
  else 
  {
    return luaL_error( L, "callback funtion needed");
  }
  return 0;
}
static int lwifi_stopsmartconfig( lua_State* L )
{
  micoWlanStopAirkiss();
  micoWlanStopEasyLink();
  if(smartconfig_sem!=NULL)
  {
    mico_rtos_set_semaphore(&smartconfig_sem);
    mico_thread_msleep(10);
    mico_rtos_set_semaphore(&smartconfig_sem);
    mico_thread_msleep(10);
    mico_rtos_deinit_semaphore(&smartconfig_sem);  
  }
  return 0;
}
static int lwifi_station_getip( lua_State* L )
{
  net_para_st para;
  micoWlanGetIPStatus(&para, Station);
  lua_pushstring(L,para.ip);
  return 1;
}
static int lwifi_station_getipadv( lua_State* L )
{
  net_para_st para;
  micoWlanGetIPStatus(&para, Station);
  char temp[32]={0};
  if(para.dhcp ==DHCP_Client)
    strcpy(temp,"DHCP_Client");
  else if(para.dhcp ==DHCP_Server)
    strcpy(temp,"DHCP_Server");
  else
    strcpy(temp,"DHCP_Disable");
  lua_pushstring(L,temp);
  lua_pushstring(L,para.ip);
  lua_pushstring(L,para.gate);
  lua_pushstring(L,para.mask);
  lua_pushstring(L,para.dns);
  lua_pushstring(L,para.mac);
  lua_pushstring(L,para.broadcastip);
  return 7;
}
static int lwifi_station_getlink (lua_State* L )
{
  LinkStatusTypeDef link;
  memset(&link,0x00,sizeof(link));
  micoWlanGetLinkStatus(&link);
  
  if(link.is_connected==0)
  {
    lua_pushstring(L,"disconnected");
    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushnil(L);
  }
  else
  {
    lua_pushstring(L,"connected");
    lua_pushinteger(L,link.wifi_strength);
    lua_pushstring(L,(char*)link.ssid);

    char temp[20]={0};
    char temp2[4];
    temp[0]=0x00;
    for (int j = 0; j < 6; j++)
        {
            if ( j == 6 - 1 )
            { 
              sprintf(temp2,"%02X",link.bssid[j]);
              strcat(temp,temp2);
            }
            else
            {
              sprintf(temp2,"%02X:",link.bssid[j]);
              strcat(temp,temp2);
            }
        }    
    lua_pushstring(L,temp);
  }
  return 4;
}
static int lwifi_ap_getip( lua_State* L )
{
  net_para_st para;
  micoWlanGetIPStatus(&para, Soft_AP);
  lua_pushstring(L,para.ip);
  return 1;
}
static int lwifi_ap_getipadv( lua_State* L )
{
  net_para_st para;
  micoWlanGetIPStatus(&para, Soft_AP);
  char temp[32]={0};
  strcpy(temp,"DHCP_Server");
  lua_pushstring(L,temp);
  lua_pushstring(L,para.ip);
  lua_pushstring(L,para.gate);
  lua_pushstring(L,para.mask);
  lua_pushstring(L,para.dns);
  lua_pushstring(L,para.mac);
  lua_pushstring(L,para.broadcastip);
  return 7;
}
static int lwifi_station_stop( lua_State* L )
{
  micoWlanSuspendStation();
  return 0;
}
static int lwifi_ap_stop( lua_State* L )
{
  micoWlanSuspendSoftAP();
  return 0;
}
static int lwifi_stop( lua_State* L )
{
  micoWlanSuspend();
  return 0;
}
extern char *gethostname( char *name, int len );
extern char *sethostname( char *name );
static int lwifi_sethostname( lua_State* L )
{
  size_t len=0;
  const char *s = luaL_checklstring(L, 1, &len);
  if(len<1)
        return luaL_error( L, "string len > 1" );
  sethostname((char*)s);
  return 0;
}
static int lwifi_gethostname( lua_State* L )
{
  char name[128];
  memset(name,0x00,128);
  gethostname(name,128);
  lua_pushstring(L,name);
  return 1;
}
static int lwifi_powersave( lua_State* L )
{
   int arg = lua_toboolean( L, 1 );
   
   if(lua_isboolean(L,1))
   {
     if(arg==true)
        micoWlanEnablePowerSave();
      else
        micoWlanDisablePowerSave();
   }
   else
     {
       luaL_error( L, "boolean arg needed" );
     }
   
  return 0;
}

#define MIN_OPT_LEVEL       2
#include "lrodefs.h"
static const LUA_REG_TYPE wifi_station_map[] =
{
  { LSTRKEY( "getip" ), LFUNCVAL ( lwifi_station_getip ) },
  { LSTRKEY( "getipadv" ), LFUNCVAL ( lwifi_station_getipadv ) },
  { LSTRKEY( "getlink" ), LFUNCVAL ( lwifi_station_getlink ) },
  { LSTRKEY( "stop" ), LFUNCVAL ( lwifi_station_stop ) },
#if LUA_OPTIMIZE_MEMORY > 0
#endif        
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE wifi_ap_map[] =
{
  { LSTRKEY( "getip" ), LFUNCVAL ( lwifi_ap_getip ) },
  { LSTRKEY( "getipadv" ), LFUNCVAL ( lwifi_ap_getipadv ) },
  { LSTRKEY( "stop" ), LFUNCVAL ( lwifi_ap_stop ) },
#if LUA_OPTIMIZE_MEMORY > 0
#endif        
  { LNILKEY, LNILVAL }
};
const LUA_REG_TYPE wifi_map[] =
{
  { LSTRKEY( "startap" ), LFUNCVAL( lwifi_startap )},
  { LSTRKEY( "startsta" ), LFUNCVAL( lwifi_startsta )},
  { LSTRKEY( "smartconfig" ), LFUNCVAL( lwifi_smartconfig )},
  { LSTRKEY( "stopsmartconfig" ), LFUNCVAL( lwifi_stopsmartconfig )},
  { LSTRKEY( "sethostname" ), LFUNCVAL( lwifi_sethostname )},
  { LSTRKEY( "gethostname" ), LFUNCVAL( lwifi_gethostname )},
  { LSTRKEY( "scan" ), LFUNCVAL( lwifi_scan ) },
  { LSTRKEY( "stop" ), LFUNCVAL( lwifi_stop ) },
  { LSTRKEY( "powersave" ), LFUNCVAL( lwifi_powersave ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "sta" ), LROVAL( wifi_station_map ) },
  { LSTRKEY( "ap" ), LROVAL( wifi_ap_map ) },
#endif        
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_wifi(lua_State *L)
{

#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else
  luaL_register( L, EXLIB_WIFI, wifi_map );
  // Setup the new tables (station and ap) inside wifi
  lua_newtable( L );
  luaL_register( L, NULL, wifi_station_map );
  lua_setfield( L, -2, "sta" );

  lua_newtable( L );
  luaL_register( L, NULL, wifi_ap_map );
  lua_setfield( L, -2, "ap" );
  return 1;
#endif  
}
