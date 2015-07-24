#ifndef __PRESEARCH_H__
#define __PRESEARCH_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus



bool FSInit(uint8_t DeviceID);

bool FSDeInit(uint8_t DeviceID);

extern bool IsFsInited(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
