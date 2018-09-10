#ifndef _RELAY_H_
#define _RELAY_H_

#include "stm32_eval.h"

#include "DtypeStm32.h"

typedef enum
{
   RELAY_ULTRA_VIOLET_LAMP       = 0,  // 紫外灯开启信号，高有效，默认关闭
   RELAY_PUMP_CIRCULATE_C3  ,           // 循环泵开启信号，高有效，默认关闭
   RELAY_VALVE_CIRCULATE_E5  ,           // 循环泵开启信号，高有效，默认关闭
   RELAY_VALVE_WATER_INTAKING_E6  ,     // 取水电磁阀开启，高有效，默认关闭
   RELAY_VALVE_WATER_INFLOWING_E7 ,     // 进水电磁阀开启，高有效，默认关闭
   RELAY_LED_WATER_ANYIN        ,     // 任意水量取水状态指示灯，任意水量取水时亮，否则灭，低有效
   RELAY_VALVE_FLUSH_E8        ,        // 冲洗阀E8，高有效，默认关闭

   RELAY_SENSOR_SWITCH1,
   RELAY_SENSOR_SWITCH2,
   RELAY_SENSOR_SWITCH3,
   
   RELAY_NUMBER,
}RELAY_ENUM;

typedef void (*RelayPulse_Cb)(void);


void InitRelays(void);
void RelayLogicCtrl(UINT8 ucChannel,UINT8 ucEnable);
UINT8 GetRelayLogicStatus(UINT8 ucChannel);
void RelayToggle(UINT8 ucChannel);
void RelayPulse(UINT8 ucChannel,uint32_t duration,RelayPulse_Cb cb);

#endif
