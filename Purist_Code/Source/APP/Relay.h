#ifndef _RELAY_H_
#define _RELAY_H_

#include "stm32_eval.h"

#include "DtypeStm32.h"

typedef enum
{
   RELAY_ULTRA_VIOLET_LAMP       = 0,  // ����ƿ����źţ�����Ч��Ĭ�Ϲر�
   RELAY_PUMP_CIRCULATE_C3  ,           // ѭ���ÿ����źţ�����Ч��Ĭ�Ϲر�
   RELAY_VALVE_CIRCULATE_E5  ,           // ѭ���ÿ����źţ�����Ч��Ĭ�Ϲر�
   RELAY_VALVE_WATER_INTAKING_E6  ,     // ȡˮ��ŷ�����������Ч��Ĭ�Ϲر�
   RELAY_VALVE_WATER_INFLOWING_E7 ,     // ��ˮ��ŷ�����������Ч��Ĭ�Ϲر�
   RELAY_LED_WATER_ANYIN        ,     // ����ˮ��ȡˮ״ָ̬ʾ�ƣ�����ˮ��ȡˮʱ���������𣬵���Ч
   RELAY_VALVE_FLUSH_E8        ,        // ��ϴ��E8������Ч��Ĭ�Ϲر�

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
