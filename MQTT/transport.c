
#include "MQTTPacket.h"

/**
  * @brief  通过TCP方式发送数据到TCP服务器
  * @param  buf 数据首地址
  * @param  buflen 数据长度
  * @retval 小于0表示发送失败
  */
int transport_sendPacketBuffer(unsigned char* buf, int buflen)
{
  //return send(SOCK_TCPS,buf,buflen);
  return 0;
}
/**
  * @brief  阻塞方式接收TCP服务器发送的数据
  * @param  buf 数据存储首地址
  * @param  count 数据缓冲区长度
  * @retval 小于0表示接收数据失败
  */
int transport_getdata(unsigned char* buf, int count)
{
  //return recv(SOCK_TCPS,buf,count);
  return 0;
}


/**
  * @brief  打开一个socket并连接到服务器
  * @param  无
  * @retval 小于0表示打开失败
  */
int transport_open(void)
{
  /*
  int32_t ret;
  //新建一个Socket并绑定本地端口5000
  ret = socket(SOCK_TCPS,Sn_MR_TCP,5000,0x00);
  if(ret != SOCK_TCPS){
    printf("%d:Socket Error\r\n",SOCK_TCPS);
    while(1);
  }else{
    printf("%d:Opened\r\n",SOCK_TCPS);
  }

  //连接TCP服务器
  ret = connect(SOCK_TCPS,domain_ip,1883);//端口必须为1883
  if(ret != SOCK_OK){
    printf("%d:Socket Connect Error\r\n",SOCK_TCPS);
    while(1);
  }else{
    printf("%d:Connected\r\n",SOCK_TCPS);
  }
	return 0;
  */
  return 0;
}

int transport_close(void)
{
  //close(SOCK_TCPS);
  return 0;
}
