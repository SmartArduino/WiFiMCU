/**
 * wifi.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lrodefs.h"
#include "lexlibs.h"

#include "platform.h"
#include "MICODefine.h"
#include "MicoWlan.h"
#include "MICONotificationCenter.h"

static lua_State *gL=NULL;
static int wifi_scan_succeed = LUA_NOREF;
static int wifi_status_changed_AP = LUA_NOREF;
static int wifi_status_changed_STA = LUA_NOREF;

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
  if(wifi_status_changed_AP == LUA_NOREF&&
     wifi_status_changed_STA == LUA_NOREF)
    return;
  
  if(wifi_status_changed_AP != LUA_NOREF)
    lua_rawgeti(gL, LUA_REGISTRYINDEX, wifi_status_changed_AP);
  else if(wifi_status_changed_STA != LUA_NOREF)
    lua_rawgeti(gL, LUA_REGISTRYINDEX, wifi_status_changed_STA);
   switch (event) {
  case NOTIFY_STATION_UP:
    lua_pushstring(gL, "STATION_UP");
    //MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    lua_pushstring(gL, "STATION_DOWN");
    //MicoRfLed(false);
    break;
  case NOTIFY_AP_UP:
    lua_pushstring(gL, "AP_UP");
    //MicoRfLed(true);
    break;
  case NOTIFY_AP_DOWN:
    lua_pushstring(gL, "AP_DOWN");
    //MicoRfLed(false);
    break;
  default:lua_pushstring(gL,"ERROR");
    break;
  }
  lua_call(gL, 1, 0);
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

int _micoNotify_WiFi_Scan_OK (ScanResult_adv *pApList, mico_Context_t * const inContext)
{
  (void)inContext;
  if(wifi_scan_succeed == LUA_NOREF)
    return 0;
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
  return 0;
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

static const LUA_REG_TYPE wifi_station_map[] =
{
  { LSTRKEY( "getip" ), LFUNCVAL ( lwifi_station_getip ) },
  { LSTRKEY( "getipadv" ), LFUNCVAL ( lwifi_station_getipadv ) },
  { LSTRKEY( "getlink" ), LFUNCVAL ( lwifi_station_getlink ) },
  { LSTRKEY( "stop" ), LFUNCVAL ( lwifi_station_stop ) },
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE wifi_ap_map[] =
{
  { LSTRKEY( "getip" ), LFUNCVAL ( lwifi_ap_getip ) },
  { LSTRKEY( "getipadv" ), LFUNCVAL ( lwifi_ap_getipadv ) },
  { LSTRKEY( "stop" ), LFUNCVAL ( lwifi_ap_stop ) },
  { LNILKEY, LNILVAL }
};
const LUA_REG_TYPE wifi_map[] =
{
  { LSTRKEY( "startap" ), LFUNCVAL( lwifi_startap )},
  { LSTRKEY( "startsta" ), LFUNCVAL( lwifi_startsta )},
  { LSTRKEY( "scan" ), LFUNCVAL( lwifi_scan ) },
  { LSTRKEY( "stop" ), LFUNCVAL( lwifi_stop ) },
  { LSTRKEY( "powersave" ), LFUNCVAL( lwifi_powersave ) },
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_wifi(lua_State *L)
{
  luaL_register( L, EXLIB_WIFI, wifi_map );
  // Setup the new tables (station and ap) inside wifi
  lua_newtable( L );
  luaL_register( L, NULL, wifi_station_map );
  lua_setfield( L, -2, "sta" );

  lua_newtable( L );
  luaL_register( L, NULL, wifi_ap_map );
  lua_setfield( L, -2, "ap" );
  return 1;
}
