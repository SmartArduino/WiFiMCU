
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TRANSPORT_H
#define __TRANSPORT_H
/* Includes ------------------------------------------------------------------*/

/* Exported Functions --------------------------------------------------------*/

int transport_sendPacketBuffer(unsigned char* buf, int buflen);
int transport_getdata(unsigned char* buf, int count);
int transport_open(void);
int transport_close(void);



#endif /* __TRANSPORT_H */

/*********************************END OF FILE**********************************/
