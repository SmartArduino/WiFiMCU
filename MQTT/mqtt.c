
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "MQTTPacket.h"
#include "transport.h"
//#include "socket.h"	// Just include one header for WIZCHIP
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
extern uint8_t domain_ip[4];

/**
  * @brief  向代理（服务器）发送一个消息
  * @param  pTopic 消息主题
  * @param  pMessage 消息内容
  * @retval 小于0表示发送失败
  */
int mqtt_publish(char *pTopic,char *pMessage)
{
  int32_t len,rc;
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  unsigned char buf[200];
  MQTTString topicString = MQTTString_initializer;
  int msglen = strlen(pMessage);
  int buflen = sizeof(buf);

  data.clientID.cstring = "me";
  data.keepAliveInterval = 5;
  data.cleansession = 1;
  len = MQTTSerialize_connect(buf, buflen, &data); /* 1 */

  topicString.cstring = pTopic;
  len += MQTTSerialize_publish(buf + len, buflen - len, 0, 0, 0, 0, topicString, (unsigned char*)pMessage, msglen); /* 2 */

  len += MQTTSerialize_disconnect(buf + len, buflen - len); /* 3 */
  transport_open();	
  rc = transport_sendPacketBuffer(buf,len);
  transport_close();
	if (rc == len)
		printf("Successfully published\n\r");
	else
		printf("Publish failed\n\r");
  return 0;
}
/**
  * @brief  向服务器订阅一个消息，该函数会因为TCP接收数据函数而阻塞
  * @param  pTopic 消息主题，传入
  * @param  pMessage 消息内容，传出
  * @retval 小于0表示订阅消息失败
  */
int mqtt_subscrib(char *pTopic,char *pMessage)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int rc = 0;
	unsigned char buf[200];
	int buflen = sizeof(buf);
	int msgid = 1;
	MQTTString topicString = MQTTString_initializer;
	int req_qos = 0;

	int len = 0;
	rc = transport_open();
	if(rc < 0){
    printf("transport_open error\n\r");
		return rc;
  }

	data.clientID.cstring = "";
	data.keepAliveInterval = 5;//服务器保持连接时间，超过该时间后，服务器会主动断开连接，单位为秒
	data.cleansession = 1;
	data.username.cstring = "";
	data.password.cstring = "";

	len = MQTTSerialize_connect(buf, buflen, &data);
	rc = transport_sendPacketBuffer(buf, len);
  if(rc != len){
    printf("connect transport_sendPacketBuffer error\n\r");
    goto exit;
  }
	/* wait for connack */
	if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
	{
		unsigned char sessionPresent, connack_rc;

		if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
		{
			printf("Unable to connect, return code %d\n\r", connack_rc);
			goto exit;
		}
	}else{
    printf("MQTTPacket_read error\n\r");
		goto exit;
  }

	/* subscribe */
	topicString.cstring = pTopic;
	len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);

	rc = transport_sendPacketBuffer(buf, len);
  if(rc != len){
    printf("connect transport_sendPacketBuffer error\n\r");
    goto exit;
  }
	if (MQTTPacket_read(buf, buflen, transport_getdata) == SUBACK) 	/* wait for suback */
	{
		unsigned short submsgid;
		int subcount;
		int granted_qos;

		rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, buflen);
		if (granted_qos != 0)
		{
			printf("granted qos != 0, %d\n\r", granted_qos);
			goto exit;
		}
	}
	else
		goto exit;

	/* loop getting msgs on subscribed topic */
	topicString.cstring = pTopic;
  memset(buf,0,buflen);
  //transport_getdata接收数据会阻塞，除非服务器断开连接后才返回
  if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBLISH){
    unsigned char dup;
    int qos;
    unsigned char retained;
    unsigned short msgid;
    int payloadlen_in;
    unsigned char* payload_in;
    MQTTString receivedTopic;

    rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
        &payload_in, &payloadlen_in, buf, buflen);
    printf("message arrived %d: %s\n\r", payloadlen_in, payload_in);
    strcpy(pMessage,(const char *)payload_in);
  }
  printf("disconnecting\n\r");
  len = MQTTSerialize_disconnect(buf, buflen);
  rc = transport_sendPacketBuffer(buf, len);
exit:
	transport_close();
  return rc;
}


