/**
******************************************************************************
* @file    http_server.c 
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

#define http_server_log(M, ...) custom_log("HTTP", M, ##__VA_ARGS__)

#define BUF_LEN (3*1024)

void http_server_thread( void *inContext );
void http_server_client_thread( void *inFd );
static OSStatus LocalConfigRespondInComingMessage(int fd, HTTPHeader_t* inHeader, void * inUserContext);
static OSStatus onReceivedData(struct _HTTPHeader_t * httpHeader, uint32_t pos, uint8_t * data, size_t len, void * userContext);
static void onClearHTTPHeader(struct _HTTPHeader_t * httpHeader, void * userContext );

static char *ap_ssid = "sqdmz";
static char *ap_key  = "0987654321";

static network_InitTypeDef_adv_st wNetConfigAdv;
static mico_semaphore_t http_sem;
static mico_mutex_t http_mutex;
static mico_Context_t *inContext;

typedef struct _configContext_t{
  uint32_t flashStorageAddress;
  bool     isFlashLocked;
} configContext_t;

const char systemPage[] = {
"<html><head><title>System</title>\r\n\
</head>\r\n\
<body>\
<br /><font size=\"6\" color=\"red\">MiCO HTTP server and OTA Demo</font><br />\r\n\
<br />mxchipWNet library version:&nbsp;%s&nbsp;<br />\r\n\
<br />ssid:&nbsp;%s&nbsp;\r\n\
<br />pass:&nbsp;%s&nbsp;<br /><br />\r\n\
</FORM>\r\n\
<FORM ENCTYPE=\"multipart/form-data\" action=\"update.htm\" METHOD=POST>\r\n\
<label>Update firmware: <input type=\"file\" name=\"imagefile\" accept=\"bin\"></label>\
<input type=\"submit\" name=\"upload\" value=\"Upload\">\
</FORM></body></html>\r\n"
};

const char systemResponseSucc[]={
"<html>\r\n\
<head>\r\n\
<title>MXCHIP Wi-Fi module</title>\r\n\
</head>\r\n\
<body>\r\n\
<p>Firmware update success, system reboot...please wait 5 seconds and refresh</p>\r\n\
</body>\r\n\
</html>"
};

void micoNotify_WifiStatusHandler(WiFiEvent event,  const int inContext)
{
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    http_server_log("Station up");
    mico_rtos_set_semaphore(&http_sem);
    MicoRfLed(true);
    break;
  case NOTIFY_STATION_DOWN:
    http_server_log("Station down");
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
  http_server_log("Wlan Connection Err %d", err);
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
  
  http_server_log("connect to %s...", wNetConfigAdv.ap_info.ssid);
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
  
  err = mico_rtos_init_mutex(&http_mutex);
  require_noerr( err, exit ); 
  
  connect_ap( );
  
  mico_rtos_get_semaphore(&http_sem, MICO_WAIT_FOREVER);
  
  micoWlanGetIPStatus(&para, Station);
  http_server_log("local ip: %s", para.ip);
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "http server", http_server_thread, 0x800, NULL );
  require_noerr_action( err, exit, http_server_log("ERROR: Unable to start the http server thread.") );
  
  return err;

exit:
  http_server_log("ERROR, err: %d", err);
  return err;
}

void http_server_thread( void *inContext )
{
  OSStatus err;
  int j;
  struct sockaddr_t addr;
  int sockaddr_t_size;
  fd_set readfds;
  char ip_address[16];
  
  int http_listener_fd = -1;

  /*Establish a TCP server fd that accept the http clients connections*/ 
  http_listener_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  require_action(IsValidSocket( http_listener_fd ), exit, err = kNoResourcesErr );
  addr.s_ip = INADDR_ANY;
  addr.s_port = 80;
  err = bind(http_listener_fd, &addr, sizeof(addr));
  require_noerr( err, exit );

  err = listen(http_listener_fd, 0);
  require_noerr( err, exit );

  http_server_log("Server established at port: %d, fd: %d", addr.s_port, http_listener_fd);
  
  while(1){
    FD_ZERO(&readfds);
    FD_SET(http_listener_fd, &readfds);  
    select(1, &readfds, NULL, NULL, NULL);

    /*Check http connection requests */
    if(FD_ISSET(http_listener_fd, &readfds)){
      sockaddr_t_size = sizeof(struct sockaddr_t);
      j = accept(http_listener_fd, &addr, &sockaddr_t_size);
      if (j > 0) {
        inet_ntoa(ip_address, addr.s_ip );
        http_server_log("Client %s:%d connected, fd: %d", ip_address, addr.s_port, j);
        if(kNoErr != mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Local Clients", http_server_client_thread, 0x1500, &j) ) 
          SocketClose(&j);
      }
    }
   }

exit:
    http_server_log("Exit: Local controller exit with err = %d", err);
    mico_rtos_delete_thread(NULL);
    return;
}

void http_server_client_thread( void *inFd )
{
  OSStatus err;
  int clientFd = *(int *)inFd;
  int clientFdIsSet;
  fd_set readfds;
  struct timeval_t t;
  HTTPHeader_t *httpHeader = NULL;
  configContext_t httpContext = {0, false};
  
  httpHeader = HTTPHeaderCreateWithCallback(onReceivedData, onClearHTTPHeader, &httpContext);
  require_action( httpHeader, exit, err = kNoMemoryErr );
  HTTPHeaderClear( httpHeader );

  t.tv_sec = 60;
  t.tv_usec = 0;
  http_server_log("Free memory %d bytes", MicoGetMemoryInfo()->free_memory) ; 

  while(1){
    FD_ZERO(&readfds);
    FD_SET(clientFd, &readfds);
    clientFdIsSet = 0;

    if(httpHeader->len == 0){
      require(select(1, &readfds, NULL, NULL, &t) >= 0, exit);
      clientFdIsSet = FD_ISSET(clientFd, &readfds);
    }
  
    if(clientFdIsSet||httpHeader->len){
      err = SocketReadHTTPHeader( clientFd, httpHeader );

      switch ( err )
      {
        case kNoErr:

          err = SocketReadHTTPBody( clientFd, httpHeader );
          
          if(httpHeader->dataEndedbyClose == true){
            err = LocalConfigRespondInComingMessage( clientFd, httpHeader, &httpContext);
            require_noerr(err, exit);
            err = kConnectionErr;
            goto exit;
          }else{
            require_noerr(err, exit);
            err = LocalConfigRespondInComingMessage( clientFd, httpHeader, &httpContext);
            require_noerr(err, exit);
          }
      
          // Reuse HTTPHeader
          HTTPHeaderClear( httpHeader );
        break;

        case EWOULDBLOCK:
            // NO-OP, keep reading
        break;

        case kNoSpaceErr:
          http_server_log("ERROR: Cannot fit HTTPHeader.");
          goto exit;
        
        case kConnectionErr:
          // NOTE: kConnectionErr from SocketReadHTTPHeader means it's closed
          http_server_log("ERROR: Connection closed.");
          goto exit;

        default:
          http_server_log("ERROR: HTTP Header parse internal error: %d", err);
          goto exit;
      }
    }
  }

exit:
  http_server_log("Exit: Client exit with err = %d, fd:%d", err, clientFd);
  SocketClose(&clientFd);
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
  char *data = NULL;
  int len_data;
  configContext_t *context = (configContext_t *)inUserContext;
  
  if(inLen == 0)
    return err;

  if( HTTPHeaderMatchURL( inHeader, "update.htm" ) == kNoErr ){
    http_server_log("OTA data %d, %d to %x", inPos, inLen, context->flashStorageAddress); 
    if(inPos == 0)
    {
      data=strstr((char *)inData,"\r\n\r\n")+4;
      len_data = inLen-(data-(char *)inData);
      http_server_log("OTA data len %d to %x", len_data, context->flashStorageAddress); 
      context->flashStorageAddress = UPDATE_START_ADDRESS;
      mico_rtos_lock_mutex(&http_mutex); //We are write the Flash content, no other write is possiable
      context->isFlashLocked = true;
      err = MicoFlashInitialize( MICO_FLASH_FOR_UPDATE );
      require_noerr(err, flashErrExit);
      err = MicoFlashErase(MICO_FLASH_FOR_UPDATE, UPDATE_START_ADDRESS, UPDATE_END_ADDRESS);
      require_noerr(err, flashErrExit);
      err = MicoFlashWrite(MICO_FLASH_FOR_UPDATE, &context->flashStorageAddress, (uint8_t *)data, len_data);
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

  if(err!=kNoErr)  http_server_log("onReceivedData");
  return err;
  
flashErrExit:
  MicoFlashFinalize(MICO_FLASH_FOR_UPDATE);
  mico_rtos_unlock_mutex(&http_mutex);
  return err;
}

OSStatus LocalConfigRespondInComingMessage(int fd, HTTPHeader_t* inHeader, void * inUserContext)
{
  OSStatus err = kUnknownErr;
  uint8_t *httpResponse = NULL;
  size_t httpResponseLen = 0;
  char httpbody[1500];
  configContext_t *context = (configContext_t *)inUserContext;
  
  if(HTTPHeaderMatchURL( inHeader, "/" ) == kNoErr){    
    sprintf(httpbody, systemPage, MicoGetVer(), ap_ssid, ap_key);
    err =  CreateSimpleHTTPMessageNoCopy( kMIMEType_TextHTML, strlen(httpbody), &httpResponse, &httpResponseLen );
    require_noerr( err, exit );
    require( httpResponse, exit );
    err = SocketSend( fd, httpResponse, httpResponseLen );
    require_noerr( err, exit );
    err = SocketSend( fd, (uint8_t *)httpbody, strlen(httpbody) );
    require_noerr( err, exit );
    http_server_log("system page sent");
    goto exit;
  }
  else if(HTTPHeaderMatchURL( inHeader, "update.htm" ) == kNoErr){
    if(inHeader->contentLength > 0){
      http_server_log("Receive OTA data!");
      
      MicoFlashFinalize(MICO_FLASH_FOR_UPDATE);
      mico_rtos_unlock_mutex(&http_mutex);
      http_server_log("size %d", context->flashStorageAddress - UPDATE_START_ADDRESS);
      err = CreateSimpleHTTPMessageNoCopy( kMIMEType_TextHTML, strlen(systemResponseSucc), &httpResponse, &httpResponseLen );
      require_noerr( err, exit );
      require( httpResponse, exit );
      err = SocketSend( fd, httpResponse, httpResponseLen );
      err = SocketSend( fd, (uint8_t *)systemResponseSucc, strlen(systemResponseSucc) );
      require_noerr( err, exit );
    
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
  if(httpResponse)  free(httpResponse);

  return err;

}



