/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MQTT_H
#define __MQTT_H
/* Includes ------------------------------------------------------------------*/

/* Exported Functions --------------------------------------------------------*/

int mqtt_publish(char *pTopic,char *pMessage);
int mqtt_subscrib(char *pTopic,char *pMessage);



#endif /* __MQTT_H */

/*********************************END OF FILE**********************************/
