/**
  ******************************************************************************
  * @file    app_cfg.h 
  * @author  STM32 Team
  * @version V1.0.0
  * @date    10/10/2014
  * @brief   Interface for app entry.
  ******************************************************************************
  *          DO NOT MODIFY ANY OF THE FOLLOWING CODE EXCEPT UNDER PERMISSION OF ITS PROPRIETOR
  * @copy
  *
  *
  * <h2><center>&copy; COPYRIGHT 2012 Shanghai ZhenNeng Corporation</center></h2>
  */ 

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/


#define  APP_TASK_MAIN_PRIO                   3  // DO NOT CHANGE!!

#define  APP_TASK_NUMBER                     (3) // DO NOT CHANGE!! AT MOST 3 APP task

#define  APP_TASK_ROOT_PRIO                  (APP_TASK_MAIN_PRIO + APP_TASK_NUMBER)


#define  OS_TASK_TMR_PRIO              (OS_LOWEST_PRIO - 2)

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*********************************************************************************************************
*/


#define  APP_TASK_MAIN_STK_SIZE             512
#define  APP_TASK_ROOT_STK_SIZE             128

#define  APP_TASK_PROBE_STR_STK_SIZE         64

#define  OS_PROBE_TASK_STK_SIZE              64

#define  APP_TIMER_TICK                        (10)             // in ms

#define CHECK_KEY_DURATION                    (10/APP_TIMER_TICK) //10ms ->  in check tick
#define SECOND_DRUATION                       (1000/APP_TIMER_TICK) // 1000MS -> in check tick
#define LIGHT_ADJUST_DURATION                 (100/APP_TIMER_TICK) // 100MS -> in check tick


#define	TIMER_CHECK_SECOND                     0 

//#define	TIMER_CHECK_KEY                        1 

#define	TIMER_CHECK_BEEP                       2 

#define	TIMER_CHECK_ALARM                      3 

#define TIMER_USER_BASE   (TIMER_CHECK_ALARM + 1) // id = 3


typedef enum
{
   // user add here
   SELF_MSG_CODE_USER_BEGIN = 0X10,
   SELF_MSG_CODE_USER_DISP,
   
   SELF_MSG_CODE_NUM,
}SELF_MSG_CODE_ENUM;

enum KEYS_ENUM
{
    KEY_CODE_NORMAL_K1 = 0,
    KEY_CODE_RECURSIVE_K2 , // ѭ������
    KEY_CODE_QUANTITY_K3 ,     // ����ȡˮ����
    KEY_CODE_SET_K4,           // ���ð���S
    KEY_CODE_NUM,   
};

//#define DEBUG

#ifdef DEBUG_INFO
#define INFO_Printf(fmt,arg...) printf(fmt,## arg)
#else
#define INFO_Printf(fmt,arg...)
#endif

#ifdef DEBUG_ERROR
#define ERROR_Printf(fmt,arg...) printf(fmt,## arg)
#else
#define ERROR_Printf(fmt,arg...)
#endif


#define IWDG_SUPPORT 1 

#define RTC_SUPPORT  1

#define ALARM_SUPPORT 1

#define BEEP_SUPPORT 1

#define RUN_SUPPORT 1

#define SERIAL_SUPPORT 0

#define CAN_SUPPORT 0   // USB & CAN cann't exist at the same time

#define USB_SUPPORT 0   // USB & CAN cann't exist at the same time

#define SD_SUPPORT 1

#endif
