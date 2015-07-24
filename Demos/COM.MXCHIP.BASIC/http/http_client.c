/**
******************************************************************************
* @file    http_client.c 
* @author  William Xu
* @version V1.0.0
* @date    21-May-2015
* @brief   First MiCO application to say hello world!
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/

#include "MICO.h"
#include "MICODefine.h"
#include "MICONotificationCenter.h"

#include "HTTPUtils.h"
#include "SocketUtils.h"
#include "StringUtils.h"

#define http_client_log(M, ...) custom_log("HTTP", M, ##__VA_ARGS__)

#define BUF_LEN (3*1024)

void http_client_thread( void *inContext );
static OSStatus LocalConfigRespondInComingMessage(int fd, HTTPHeader_t* inHeader, void * inUserContext);
static OSStatus onReceivedData(struct _HTTPHeader_t * httpHeader, uint32_t pos, uint8_t * data, size_t len, void * userContext);
static void onClearHTTPHeader(struct _HTTPHeader_t * httpHeader, void * userContext );

static char *ap_ssid = "sqdmz";
static char *ap_key  = "0987654321";
static char *ota_url = "COM.MXCHIP.BASIC.bin";
static char *http_server = "192.168.1.100";
static int http_port = 80;

static network_InitTypeDef_adv_st wNetConfigAdv;
static mico_semaphore_t http_sem;
static mico_semaphore_t button_sem;
static mico_mutex_t http_mutex;
static mico_Context_t *inContext;

typedef struct _configContext_t{
  uint32_t flashStorageAddress;
  bool     isFlashLocked;
} configContext_t;

const char http_requir[]=
{
"GET /%s HTTP/1.1\r\n\
Host: %s\r\n\
Cache-Control: no-cache\r\n\r\n"
};

USED void PlatformEasyLinkButtonClickedCallback(void)
{
  mico_rtos_set_semaphore(&button_sem);
  return;
}

void micoNotify_WifiStatusHandler(WiFiEvent event,  const int inContext)
{
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    http_client_log("Station up");
    mico_rtos_set_semaphore(&http_sem);
    MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    http_client_log("Station down");
    MicoRfLed(false);
    break;
  default:
    break;
  }
  return;
}

void micoNotify_ConnectFailedHandler(OSStatus err, const int inContext)
{
  (void)inContext;
  http_client_log("Wlan Connection Err %d", err);
}

static void connect_ap( void )
{  
  memset(&wNetConfigAdv, 0x0, sizeof(network_InitTypeDef_adv_st));
  
  strcpy((char*)wNetConfigAdv.ap_info.ssid, ap_ssid);
  strcpy((char*)wNetConfigAdv.key, ap_key);
  wNetConfigAdv.key_len = strlen(ap_key);
  wNetConfigAdv.ap_info.security = SECURITY_TYPE_AUTO;
  wNetConfigAdv.ap_info.channel = 0; //Auto
  wNetConfigAdv.dhcpMode = DHCP_Client;
  wNetConfigAdv.wifi_retry_interval = 100;
  micoWlanStartAdv(&wNetConfigAdv);
  
  http_client_log("connect to %s...", wNetConfigAdv.ap_info.ssid);
}

int application_start( void )
{
  OSStatus err = kNoErr;
  IPStatusTypedef para;
  
  memset(inContext, 0x0, sizeof(mico_Context_t));
  inContext = ( mico_Context_t *)malloc(sizeof(mico_Context_t) );
  require_action( inContext, exit, err = kNoMemoryErr );  
  
  MicoInit( );
  
  /*The notification message for the registered WiFi status change*/
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)micoNotify_WifiStatusHandler );
  require_noerr( err, exit ); 
  
  err = MICOAddNotification( mico_notify_WIFI_CONNECT_FAILED, (void *)micoNotify_ConnectFailedHandler );
  require_noerr( err, exit );
  
  err = mico_rtos_init_semaphore(&http_sem, 1);
  require_noerr( err, exit );
  
  err = mico_rtos_init_semaphore(&button_sem, 1);
  require_noerr( err, exit );
  
  err = mico_rtos_init_mutex(&http_mutex);
  require_noerr( err, exit ); 
  
  connect_ap( );
  
  mico_rtos_get_semaphore(&http_sem, MICO_WAIT_FOREVER);
  
  micoWlanGetIPStatus(&para, Station);
  http_client_log("local ip: %s", para.ip);
  
  http_client_log("please chick easylink button, and start ota demo");
  while ( (mico_rtos_get_semaphore(&button_sem, MICO_WAIT_FOREVER)) == kNoErr )
  {
    err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "http client", http_client_thread, 0x800, NULL );
    require_noerr_action( err, exit, http_client_log("ERROR: Unable to start the http client thread.") );
  }
  return err;

exit:
  http_client_log("ERROR, err: %d", err);
  return err;
}


void http_client_thread( void *inContext )
{
  OSStatus err;
  int client_fd = -1;
  int clientFdIsSet;
  fd_set readfds;
  char ipstr[16];
  struct sockaddr_t addr;
  struct timeval_t t;
  char *httpRequest = NULL;
  HTTPHeader_t *httpHeader = NULL;
  configContext_t httpContext = {0, false};
  
  httpHeader = HTTPHeaderCreateWithCallback(onReceivedData, onClearHTTPHeader, &httpContext);
  require_action( httpHeader, exit, err = kNoMemoryErr );
  HTTPHeaderClear( httpHeader );
  
  http_client_log("Free memory %d bytes", MicoGetMemoryInfo()->free_memory) ; 
  
  while(1) {
   err = gethostbyname(http_server, (uint8_t *)ipstr, 16);
   require_noerr(err, ReConnWithDelay);
   http_client_log("server address: host:%s, ip: %s", http_server, ipstr);
   break;

  ReConnWithDelay:
   mico_thread_sleep(1);
  }
   
  client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  addr.s_ip = inet_addr(ipstr);
  addr.s_port = http_port;
  
  err = connect(client_fd, &addr, sizeof(addr)); 
  require_noerr_action(err, exit, http_client_log("connect http server failed"));

  http_client_log("connect http server success, fd %d", client_fd);
  
  httpRequest = malloc(256);
  sprintf(httpRequest, http_requir, ota_url, http_server);
  err = SocketSend( client_fd, (uint8_t *)httpRequest, strlen(httpRequest) );
  require_noerr( err, exit );
  free(httpRequest);
  
  t.tv_sec = 60;
  t.tv_usec = 0;

  while(1){
    FD_ZERO(&readfds);
    FD_SET(client_fd, &readfds);
    clientFdIsSet = 0;

    if(httpHeader->len == 0){
      require(select(1, &readfds, NULL, NULL, &t) >= 0, exit);
      clientFdIsSet = FD_ISSET(client_fd, &readfds);
    }
  
    if(clientFdIsSet||httpHeader->len){
      err = SocketReadHTTPHeader( client_fd, httpHeader );

      switch ( err )
      {
        case kNoErr:

          err = SocketReadHTTPBody( client_fd, httpHeader );
          
          if(httpHeader->dataEndedbyClose == true){
            err = LocalConfigRespondInComingMessage( client_fd, httpHeader, &httpContext);
            require_noerr(err, exit);
            err = kConnectionErr;
            goto exit;
          }else{
            require_noerr(err, exit);
            err = LocalConfigRespondInComingMessage( client_fd, httpHeader, &httpContext);
            require_noerr(err, exit);
          }
      
          // Reuse HTTPHeader
          HTTPHeaderClear( httpHeader );
        break;

        case EWOULDBLOCK:
            // NO-OP, keep reading
        break;

        case kNoSpaceErr:
          http_client_log("ERROR: Cannot fit HTTPHeader.");
          goto exit;
        
        case kConnectionErr:
          // NOTE: kConnectionErr from SocketReadHTTPHeader means it's closed
          http_client_log("ERROR: Connection closed.");
          goto exit;

        default:
          http_client_log("ERROR: HTTP Header parse internal error: %d", err);
          goto exit;
      }
    }
  }

exit:
  http_client_log("Exit: Client exit with err = %d, fd:%d", err, client_fd);
  SocketClose(&client_fd);
  if(httpHeader) {
    HTTPHeaderClear( httpHeader );
    free(httpHeader);
  }
  mico_rtos_delete_thread(NULL);
  return;
}

static void onClearHTTPHeader(struct _HTTPHeader_t * inHeader, void * inUserContext )
{
  UNUSED_PARAMETER(inHeader);
  configContext_t *context = (configContext_t *)inUserContext;

  if(context->isFlashLocked == true){
    mico_rtos_unlock_mutex(&http_mutex);
    context->isFlashLocked = false;
  }
}

static OSStatus onReceivedData(struct _HTTPHeader_t * inHeader, uint32_t inPos, uint8_t * inData, size_t inLen, void * inUserContext )
{
  OSStatus err = kNoErr;
  const char *value;
  size_t valueSize;
  configContext_t *context = (configContext_t *)inUserContext;
  
  if(inLen == 0)
    return err;
  
  err = HTTPGetHeaderField( inHeader->buf, inHeader->len, "Content-Type", NULL, NULL, &value, &valueSize, NULL );
  
  if(err == kNoErr && strnicmpx( value, valueSize, kMIMEType_Stream ) == 0){
    http_client_log("OTA data %d, %d to %x", inPos, inLen, context->flashStorageAddress); 
    if(inPos == 0)
    {
      context->flashStorageAddress = UPDATE_START_ADDRESS;
      mico_rtos_lock_mutex(&http_mutex); //We are write the Flash content, no other write is possiable
      context->isFlashLocked = true;
      err = MicoFlashInitialize( MICO_FLASH_FOR_UPDATE );
      require_noerr(err, flashErrExit);
      err = MicoFlashErase(MICO_FLASH_FOR_UPDATE, UPDATE_START_ADDRESS, UPDATE_END_ADDRESS);
      require_noerr(err, flashErrExit);
      err = MicoFlashWrite(MICO_FLASH_FOR_UPDATE, &context->flashStorageAddress, (uint8_t *)inData, inLen);
      require_noerr(err, flashErrExit);
    }
    else
    {
      err = MicoFlashWrite(MICO_FLASH_FOR_UPDATE, &context->flashStorageAddress, (uint8_t *)inData, inLen);
      require_noerr(err, flashErrExit);
    }
  }
  else{
    return kUnsupportedErr;
  }

  if(err!=kNoErr)  http_client_log("onReceivedData");
  return err;
  
flashErrExit:
  MicoFlashFinalize(MICO_FLASH_FOR_UPDATE);
  mico_rtos_unlock_mutex(&http_mutex);
  return err;
}

OSStatus LocalConfigRespondInComingMessage(int fd, HTTPHeader_t* inHeader, void * inUserContext)
{
  OSStatus err = kUnknownErr;
  const char *value;
  size_t valueSize;
  configContext_t *context = (configContext_t *)inUserContext;
  
  err = HTTPGetHeaderField( inHeader->buf, inHeader->len, "Content-Type", NULL, NULL, &value, &valueSize, NULL );
  
  if(err == kNoErr && strnicmpx( value, valueSize, kMIMEType_Stream ) == 0){
    if(inHeader->contentLength > 0){
      
      http_client_log("Receive OTA data!");
      MicoFlashFinalize(MICO_FLASH_FOR_UPDATE);
      mico_rtos_unlock_mutex(&http_mutex);
    
      memset(&inContext->flashContentInRam.bootTable, 0, sizeof(boot_table_t));
      inContext->flashContentInRam.bootTable.length = context->flashStorageAddress - UPDATE_START_ADDRESS;;
      inContext->flashContentInRam.bootTable.start_address = UPDATE_START_ADDRESS;
      inContext->flashContentInRam.bootTable.type = 'A';
      inContext->flashContentInRam.bootTable.upgrade_type = 'U';
      
      MICOUpdateConfiguration(inContext);  
      mico_thread_msleep(500);
      MicoSystemReboot();
    }
  
    goto exit;
  }
  else{
    return kNotFoundErr;
  };

 exit:
  if(inHeader->persistent == false)  //Return an err to close socket and exit the current thread
    err = kConnectionErr;

  return err;

}



