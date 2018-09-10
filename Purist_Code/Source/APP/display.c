
#include "display.h"

#include "Relay.h"

#include "Dica.h"

#include <string.h>

#include <stdio.h>

#include <math.h>

#include <stdlib.h>

#include "RTC_Driver.h"

#include "LCD.h"

#include "keyboard.h"
	
#include "task.h"

#include "ad7799_Driver.h"

#include "ad7799.h"

#include "IWDG_Driver.h"

#include "font.h"

#include "storage.h"

#include "lib_def.h"

#include "UartCmd.h"

#include "Sapp.h"

#include "cminterface.h"

#include "hal_defs.h"

//#define TOC

#define SENSOR_COORD_X (90)  //50
#define SENSOR_COORD_Y (80)

#define SHOW_COORD_X   (20)
#define SHOW_COORD_Y   (LCD_H - 45)


#define DISP_INFO_OFFSET (0)

#define DISP_QTW_VOLUME_OFFSET (DISP_INFO_OFFSET + (sizeof(DISP_INFO_STRU) +1)/2*2)

#define TEMP_RANGE_FORMAT "MAX %02d MIN %02d ℃      "
#define TIME_FORMAT       "%04d-%02d-%02d %02d:%02d:%02d"
#define TIME_FORMAT_SHORT "%04d-%02d-%02d %02d:%02d"
#define PACK_FORMAT_ENG       "%03dDAYS %04dL"
#define UV_FORMAT_ENG         "%03dDAYS %04dHr"
#define PACK_FORMAT_DUT       "%03dTAGE %04dL"
#define UV_FORMAT_DUT         "%03dTAGE %04dHr"
#define PACK_FORMAT_CHN       "%03d天%04d升"
#define UV_FORMAT_CHN         "%03d天%04d小时"

#define TIME_SET_NUMS (14)
#define TIME_SET_POS {0,1,2,3,5,6,8,9,11,12,14,15,17,18}
#define TEMP_RANGE_SET_NUMS (4)
#define TEMP_RANGE_SET_POS {4,5,11,12}
#define PACK_SET_NUMS (7)
#define PACK_SET_POS {0,1,2,8,9,10,11}
#define UV_SET_NUMS (7)
#define UV_SET_POS {0,1,2,8,9,10,11}
#define CELLCONST_SET_NUMS (4)
#define CELLCONST_SET_POS {0,2,3,4}
#define TEMPCONST_SET_NUMS (4)
#define TEMPCONST_SET_POS {0,2,3,4}
#define TIME_RECT_MARGIN (3)

#define DISP_UF_TOP (SHOW_COORD_Y)

#define TOC_MEAS_PERIOD (100)
//导通3采样周期设定
//#define TOC_MEAS_PERIOD ((Display.cfg.cfg3.usTocWashTime+Display.cfg.cfg3.usTocOxiTime)*1000)
#define TOC_DELAY_SAMPLE ((Display.cfg.cfg3.usTocWashTime+Display.cfg.cfg3.usTocOxiTime-2)*1000)

#define DISP_MAX_DISP_COL_OFFSET (10)

//#define DISP_UF_TEST
#define DISP_ADC_METHOD_CONTINEOUS   0
#define DISP_ADC_METHOD_SIMUTANEOUS  1


#define DISP_ADC_METHOD_TYPE DISP_ADC_METHOD_SIMUTANEOUS

#ifndef DISP_UF_TEST
#define DISP_UF_COUNT_DOWN_SENARIO1 (113*60 + 42)

#define DISP_UF_SUB_COUNT_DOWN_3MIN (3*60)

#define DISP_UF_SUB_COUNT_DOWN_5MIN (5*60)

#define DISP_UF_SUB_COUNT_DOWN_4MIN (4*60)

#define DISP_UF_SUB_COUNT_DOWN_2SEC (2)

#define DISP_UF_SUB_COUNT_DOWN_1MIN (1*60)

#define DISP_UF_SUB_COUNT_DOWN_30SEC (30)

#define DISP_UF_SUB_COUNT_DOWN_15MIN (15*60)

#else
#define DISP_UF_COUNT_DOWN_SENARIO1 (5*60)

#define DISP_UF_SUB_COUNT_DOWN_3MIN (5)

#define DISP_UF_SUB_COUNT_DOWN_5MIN (5)

#define DISP_UF_SUB_COUNT_DOWN_4MIN (4)

#define DISP_UF_SUB_COUNT_DOWN_1MIN (4)

#define DISP_UF_SUB_COUNT_DOWN_30SEC (4)

#define DISP_UF_SUB_COUNT_DOWN_2SEC (2)

#define DISP_UF_SUB_COUNT_DOWN_15MIN (5)

#endif

#define DISP_LCD_DORMANT_TIME (3*60)

#define DISP_ALARM_DURATION_TIME (3*60)

#define DISP_TOC_ALARM_DURATION_TIME (10)

#ifndef DISP_UF_TEST
#define DISP_IDLE_WASH_PERIOD (30*60)
#else
#define DISP_IDLE_WASH_PERIOD (1*60)
#endif

#define UV_PFM_PEROID ((10*60)) // 10 MINUTE

#define DEALTA_OFFSET (curFont->sizeY/2)

#define UNIT_TYPE_0 "MΩ.cm"
#define UNIT_TYPE_1 "μS/cm"

#define FEED_ENG_ON "ON"
#define FEED_ENG_OFF "OFF"
#define FEED_CH_ON "开"
#define FEED_CH_OFF "关"

#define TANK_VALID (0)

#define LOW_RES (1000)
#define HIGH_RES (18000)

#define AT_CONFIG_DEVNAME  "DEVNAME"

/*
状态    进水阀E7|循环阀E5|取水阀E6|    冲洗阀E8   |紫外灯N  |泵C3   |传感器I2|传感器I3|传感器I4
就绪           OFF        |            OFF  |      OFF          |           OFF          |      OFF        |     OFF    |      OFF        |  OFF           |OFF
循环           OFF        |            ON   |      OFF          |           OFF          |      ON         |     ON |      ON         |  OFF           |ON
取水           ON         |            OFF  |      ON           |           OFF          |      ON         |     ON |      ON         |  ON            |ON
定量取水       ON         |            OFF  |      ON           |           OFF          |      ON         |     ON |      ON         |  ON            |ON
泄压           OFF        |            OFF  |      ON           |           OFF          |      OFF        |     OFF    |      OFF        |  OFF           |OFF
TOC冲洗        OFF        |            ON   |      OFF          |           OFF          |      OFF        |     ON |      ON         |  OFF           |ON
TOC氧化        OFF        |            ON   |      OFF          |           ON           |      ON         |     ON |      ON         |  OFF           |ON
排气           ON         |            oFF  |      ON           |           ON           |      OFF        |     OFF    |      OFF        |  OFF           |ON

*/

static tm stimer;  

static tm stimer4Us;

static tm sLastUpdateTime;

DISPLAY_STRU Display;

int key_toc; //toc过程时，跳出取水用20160930
int key_em;
static int TOC_Exception = 0;

const int SensorMask4State[DISPLAY_STATE_BUTT] = {0,5,7,7,0,5,5,0};

typedef struct
{
    MsgHead msgHead;
    void *para;
}DISP_MSG;

typedef struct _display_adc_chl
{
    int iValue;
    uint8_t  ucChl;
    uint8_t ucPoolIdx;
    struct _display_adc_chl *next;
}DISPLAY_ADC_CHL_STRU;

typedef struct  {
  DISPLAY_ADC_CHL_STRU *next;
}DISPLAY_ADC_CHLS_STRU;

typedef struct
{
    uint8_t NextIndex;
}DISPLAY_ADC_POOL_STRU;

#define DISP_MSG_LENGHT (sizeof(DISP_MSG)-sizeof(MsgHead))

RECT rectTime = {0,0,0,0};

FONT_STRU font24;

FONT_STRU font8;

#ifdef CODEGB_32
FONT_STRU font32;
#endif

#ifdef CODEGB_48
FONT_STRU font48;
#endif

#define MAX_ADC_ACTIONS 4

FONT_STRU *curFont;

static FONT_STRU *oldfont;

DISPLAY_ADC_CHLS_STRU adcChlHeader;

DISPLAY_ADC_CHL_STRU aAdcChl[MAX_ADC_ACTIONS];

DISPLAY_ADC_POOL_STRU aAdcPool[MAX_ADC_ACTIONS];


#define DISPLAY_ADC_CHECK_UNIT (100) // 100MS

#define DECREASE_VALUE(value) do {(value) = ((value) >=DISPLAY_ADC_CHECK_UNIT) ? (value)-DISPLAY_ADC_CHECK_UNIT : 0;}while(0)

void Disp_Move2TOCFlush(void);
void Disp_Prepare4Degass2(void);
void Disp_UserSetPrepareUFClean(uint8_t ucSub);
void Disp_PrepareMove2TOC(void);
void Disp_AddAdcChl(DISPLAY_ADC_CHL_STRU *pAdcChl);
uint8_t Disp_AllocAdcAction(void);

void Disp_UserUpdate4FeedQuality(void);

void Disp_Prepare4Qtw(void);

void Disp_Set_Cirtime(void); //20170911添加

extern void Att_value_split(char *parg,char sep);

void Disp_SelectFont(FONT_STRU *ft)
{
    if (curFont != ft)
    {
       oldfont = curFont;
       curFont = ft;
    }
}

void Disp_Invalidate(void);

#define is_digit(ch)  ((ch>='0' && ch<='9'))
  
#define is_space(ch)  ((ch==' ') || (ch=='\t')) 
 
void Disp_atof(char *s,UINT32 *value,UINT32 delimit)  
{  
    int i,sign;  
    UINT32 power;
    for(i=0;is_space(s[i]);i++);
    sign = (s[i]=='-') ? -1 : 1;  
    if(s[i] == '-' || s[i] == '+')
        i++;  
    for(*value = 0; is_digit(s[i]); i++)
        *value = *value*10+(s[i]-'0');  
    if(s[i] == '.')  
        i++;  
    for(power = 1; is_digit(s[i]); i++) 
    {  
        *value = *value*10+(s[i]-'0');  
        power *= 10;  
    }  
        
      *value *= sign;

     if (power < 1000)
     {
         *value *= (1000/power);
     }
     else
     {
         *value /= (power/1000);
     }
    
}  


/**
  * @brief  Configures valves according to state.
  * @param  state: working state.
  *   This parameter can be one of following parameters:    
  *     @arg DISPLAY_STATE_ENUM
  * @retval None
  */
void Disp_ValveCtrl(STATE_STRU *state)
{
   switch(state->ucMain)
   {
   default:
   case DISPLAY_STATE_IDLE:
       switch(state->ucSub)
        {
        case DISPLAY_SUB_STATE_IDLE_FLUSH:
            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
            // ADD FOR V2
            RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);
            break;
        default:
            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,FALSE);
            // ADD FOR V2
            RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
            break;
        }
       break;
   case DISPLAY_STATE_CIRCULATION:
       RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
       RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,TRUE);
       RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
       RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,TRUE);
       RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
       // ADD FOR V2
      // RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
       RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE); //20161114
       break;
   case DISPLAY_STATE_NORMAL_TAKING_WATER:
       RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
       RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
       RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,TRUE);
       RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,TRUE);
       RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
       // ADD FOR V2
     //  RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
       RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE); //20161114
       break;
   case DISPLAY_STATE_DECOMPRESSION:
       RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
       RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
       RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,TRUE);
       RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
       RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,FALSE);
       // ADD FOR V2
       RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
       break;
   case DISPLAY_STATE_QUANTITY_TAKING_WATER:
       RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
       RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
       RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,TRUE);
       RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,TRUE);
       RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
       // ADD FOR V2
       //RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
       RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);//20161114
       break;
    // ADD FOR V2
    case DISPLAY_STATE_TOC:
        switch(state->ucSub)
        {
        case DISPLAY_SUB_STATE_TOC_FLUSH:
            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,TRUE);
            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
           // RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
            RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE); //20161114
            break;
        case DISPLAY_SUB_STATE_TOC_OXIDATION:
            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,TRUE);
            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,TRUE);
            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
           // RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);
			RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);//20161117
            break;
        default:
            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,FALSE);
           // RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
			RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE); //20161114
            break;
        }
        break;
    case DISPLAY_STATE_DEGASS:
        switch(state->ucSub)
         {
         case DISPLAY_SUB_STATE_DEGASS1:
             RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
             RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
             RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
             RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
             RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
             RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);
             break;
         case DISPLAY_SUB_STATE_DEGASS2:
             RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
             RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
             RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,TRUE);
             RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
             RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
             RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
            break;
         default:
             RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
             RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
             RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
             RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
             RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,FALSE);
             // ADD FOR V2
             RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
             break;
         }
         break;
     case DISPLAY_STATE_UF_CLEAN:
        switch(state->ucSub)
        {
        case DISPALY_SUB_STATE_UF_PRECLEAN:
            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
            // ADD FOR V2
            RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);
            break;
        case DISPALY_SUB_STATE_UF_CLEAN:
            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
            // ADD FOR V2
            RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);
            break;
        default:
            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,FALSE);
            // ADD FOR V2
            RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
            break;
        }
        break;
    // END FOR V2
    
   }
}

void Disp_Select_Sensor(int Mask)
{
    int iLoop;
    for (iLoop = 0; iLoop < DISPLAY_MAX_SENSOR; iLoop++)
    {
        if ((1 << iLoop) & Mask)
        {
            RelayLogicCtrl(RELAY_SENSOR_SWITCH1+iLoop,TRUE);  
        }
        else
        {
			RelayLogicCtrl(RELAY_SENSOR_SWITCH1+iLoop,FALSE);
		  
        }
    }
}

void Disp_ReadInfo(void)
{
   
    RTC_ReadBackup((uint8_t *)&Display.info,DISP_INFO_OFFSET,sizeof(DISP_INFO_STRU));

    //VOS_LOG(VOS_LOG_DEBUG,"PACKLIFEDAY = %d",Display.info.PACKLIFEDAY);
    //VOS_LOG(VOS_LOG_DEBUG,"PACKLIFEL = %d",Display.info.PACKLIFEL);
    //VOS_LOG(VOS_LOG_DEBUG,"FILTERLIFE = %d",Display.info.FILTERLIFE);
    //VOS_LOG(VOS_LOG_DEBUG,"UVLIFEDAY = %d",Display.info.UVLIFEDAY);
    //VOS_LOG(VOS_LOG_DEBUG,"UVLIFEHOUR = %d",Display.info.UVLIFEHOUR);

    // add for V2
    Storage_ReadRunTimeInfo(&Display);
    // end for v2

    if (INVALID_CONFIG_VALUE_LONG == Display.info.PACKLIFEDAY)
    {
        Display.info.PACKLIFEDAY = RTC_Get_Second(); // NEW pack
    }
    
    if (INVALID_CONFIG_VALUE == Display.info.PACKLIFEL)
    {
        Display.info.PACKLIFEL = 0; // NEW pack
    }

    if (INVALID_CONFIG_VALUE_LONG == Display.info.FILTERLIFE)
    {
        Display.info.FILTERLIFE = RTC_Get_Second(); // NEW filter
    }
    
    if (INVALID_CONFIG_VALUE_LONG == Display.info.UVLIFEDAY)
    {
        Display.info.UVLIFEDAY = RTC_Get_Second(); // NEW uv
    }

    if (INVALID_CONFIG_VALUE == Display.info.UVLIFEHOUR)
    {
        Display.info.UVLIFEHOUR = 0; // NEW uv
    }

    // more to add here
    // add for v2
#ifdef UF_FUNCTION    
    if (INVALID_CONFIG_VALUE_LONG == Display.ulUFCleanTime)
    {
        Display.ulUFCleanTime = RTC_Get_Second(); // NEW uv

        Display.bit1InitPending4UF = TRUE;
    }    
#endif    
    // end for v2

    UartCmdPrintf(VOS_LOG_DEBUG,"ulUFCleanTime = %d",Display.ulUFCleanTime);
   
}

void Disp_WriteInfo(void)
{
    RTC_WriteBackup((uint8_t *)&Display.info,DISP_INFO_OFFSET,sizeof(DISP_INFO_STRU));
}

void Disp_ReadQtwVolume(void)
{
    int ret = RTC_ReadBackup((uint8_t *)&Display.usQtwWq,DISP_QTW_VOLUME_OFFSET,sizeof(uint16_t));

    if (ret == 0)
    {
        UartCmdPrintf(VOS_LOG_ERROR,"read qtw fail");
    }
    
    if (INVALID_CONFIG_VALUE == Display.usQtwWq)
    {
        Display.usQtwWq = 0;
    }

}

void Disp_WriteQtwVolume(void)
{
    int ret = RTC_WriteBackup((uint8_t *)&Display.usQtwWq,DISP_QTW_VOLUME_OFFSET,sizeof(uint16_t));

    if (ret == 0)
    {
		UartCmdPrintf(VOS_LOG_ERROR,"write qtw fail");
    }
}

void Disp_SaveQtwVolume(void)
{
    uint16_t ustemp = Display.usQtwWq;
    Disp_ReadQtwVolume();
	if (ustemp != Display.usQtwWq)
    {
        Display.usQtwWq = ustemp;
        Disp_WriteQtwVolume();
    }
}


void Disp_SetAlarm(uint8_t alarmId,uint8_t event)
{
    if (alarmId >= DISP_ALARM_NUM)
    {
        return;
    }

    if (!Display.aAlarm[alarmId].bit1Fired
        && !event)
    {
        return ; // already canceled
    }

    if (Display.aAlarm[alarmId].bit1Fired
        && event)
    {
        return ; // already fired
    }
 
    Display.aAlarm[alarmId].bit1Fired = event ? TRUE : FALSE;

    if (event)
    {
        Display.aAlarm[alarmId].ulFireSec = RTC_Get_Second();
    }
    else
    {
        Display.aAlarm[alarmId].bit1Triggered = FALSE;
    }
        
}

void Disp_Int2floatFormat(int value,int integer,int decimal,char *buf)
{
    int divid = 1;
    int len = 0;
    int value1,value2;
    static uint8_t tmpBuf[32];

    for (value1 = 0; value1 < decimal; value1++)
    {
        divid *= 10;
    }

    value1 = value/divid; // get integer part

    value2 = value%divid;  // get decimal part

    
    do{
        tmpBuf[len++] = value1%10 + '0';
        value1 /= 10;
    }while(value1 > 0);
    
    for (value1 = len; value1 < integer; value1++)
    {
        tmpBuf[len++] = ' ';
    }

    // now reverse
    for (value1 = 0; value1 < len; value1++)
    {
        buf[value1] = tmpBuf[len - 1 - value1];
    }

    buf[len++] = '.';

    for (value = 0; value < decimal; value++)
    {
        divid /= 10;
        buf[len++] = '0' + (value2/divid);
        value2 %= divid;
    }
    buf[len] = 0;   

}

void Disp_Int2floatFormat_toc(int value,int integer,int decimal,char *buf)
{
    int divid = 1;
    int len = 0;
    int value1,value2;
    static uint8_t tmpBuf[32];

    for (value1 = 0; value1 < decimal; value1++)
    {
        divid *= 10;
    }

    value1 = value/divid; // get integer part

    value2 = value%divid;  // get decimal part

    
    do{
        tmpBuf[len++] = value1%10 + '0';
        value1 /= 10;
    }while(value1 > 0);
    
    for (value1 = len; value1 < integer; value1++)
    {
        tmpBuf[len++] = ' ';
    }

    // now reverse
    for (value1 = 0; value1 < len; value1++)
    {
        buf[value1] = tmpBuf[len - 1 - value1];
    }

   // buf[len++] = '.';

    for (value = 0; value < decimal; value++)
    {
        divid /= 10;
        buf[len++] = '0' + (value2/divid);
        value2 %= divid;
    }
    buf[len] = 0;   

}

void Disp_Int2float(int value,char *buf)
{
    Disp_Int2floatFormat(value,2,1,buf);
}



void Disp_Int2float1(int value,char *buf)
{
	Disp_Int2floatFormat(value,1,3,buf);
}

void Disp_report(void *para)
{
	Message *Msg;
    Msg = MessageAlloc(PID_SELF,DISP_MSG_LENGHT);

    if (Msg)
    {
		DISP_MSG *dmsg = (DISP_MSG *)Msg;
        dmsg->msgHead.nMsgType = SELF_MSG_CODE_USER_DISP;
        dmsg->para = para;
        MessageSend(Msg);
    }
}

void Disp_CheckConsumptiveMaterialState(void)
{
    uint32_t ulCurTime = RTC_Get_Second();
    uint32_t ulTemp;

    Display.bit1FilterCheck = FALSE;
    Display.bit1PackCheck = FALSE;
    Display.bit1UVCheck = FALSE;

    if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
    {
        // check pack
        if (ulCurTime > Display.info.PACKLIFEDAY)
        {
            ulTemp = (ulCurTime - Display.info.PACKLIFEDAY)/DISP_DAYININSECOND;

            if (ulTemp >= Display.cfg.cfg2.PACKLIFEDAY)
            {
                Display.bit1PackCheck = TRUE;
            }
            if (Display.info.PACKLIFEL >= Display.cfg.cfg2.PACKLIFEL)
            {
                Display.bit1PackCheck = TRUE;
            }

        }
        // check uv
        if (ulCurTime > Display.info.UVLIFEDAY)
        {
            ulTemp = (ulCurTime - Display.info.UVLIFEDAY)/DISP_DAYININSECOND;

            if (ulTemp >= Display.cfg.cfg2.UVLIFEDAY)
            {
                Display.bit1UVCheck = TRUE;
            }
            
            if (Display.info.UVLIFEHOUR >= Display.cfg.cfg2.UVLIFEHOUR*(3600/UV_PFM_PEROID))
            {
                Display.bit1UVCheck = TRUE;
            }

        }        
    }

    // check filter
    if (ulCurTime > Display.info.FILTERLIFE)
    {
        ulTemp = (ulCurTime - Display.info.FILTERLIFE)/DISP_DAYININSECOND;

        if (ulTemp >= Display.cfg.cfg2.FILTERLIFE)
        {
            Display.bit1FilterCheck = TRUE;
        }
    }           
}

void Disp_CumulateUvTime(uint32_t ulStartTime,uint32_t ulEndTime)
{

    if (ulEndTime > ulStartTime)
    {
        Display.ulCumulatedVuTime += (ulEndTime - ulStartTime);
    }

    //VOS_LOG(VOS_LOG_DEBUG,"Uv = %d",Display.ulCumulatedVuTime);

}

void Disp_CleanTail(int xoff,int yoff,char *text,uint16_t usBkColor)
{
    // clean tail
	xoff += curFont->sizeX*strlen((char *)text);

    LCD_Fill(xoff,yoff,LCD_W - 1,yoff+curFont->sizeY,usBkColor);

}

void Disp_WorkState(int xoff, int yoff,char *text)
{
    // display text info
    if (DISP_LAN_CHINESE == Display.cfg.cfg1.language)
    {
#ifdef CODEGB_48
       //Disp_SelectFont(&font48);
#endif
    }

    if (0xFF == xoff 
        && 0XFF == yoff)
    {
        xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text));
        
        yoff = LCD_H/2 - curFont->sizeY/2 - DEALTA_OFFSET;
    }
    
    curFont->DrawText(xoff,yoff,(u8 *)text,Display.usForColor,Display.usBackColor);

    if (DISP_LAN_CHINESE == Display.cfg.cfg1.language)
    {
#ifdef CODEGB_48
       //Disp_SelectFont(oldfont);
#endif
    }

}

void Disp_Move2DstState(void)
{
    uint32_t ulOldTime;
    uint8_t  ucPfm4Uv = FALSE;
    uint8_t  ucMeas = FALSE;

    STATE_STRU OldState = Display.CurState;

    Disp_ValveCtrl(&Display.TgtState);

    switch(Display.CurState.ucMain) //hook before state transfer
    {
    case DISPLAY_STATE_QUANTITY_TAKING_WATER:
        RelayLogicCtrl(RELAY_LED_WATER_ANYIN,FALSE);
        ucPfm4Uv = TRUE;
        ucMeas = TRUE;
        break;
    case DISPLAY_STATE_NORMAL_TAKING_WATER:
        ucPfm4Uv = TRUE;
        ucMeas = TRUE;
        break;
    case DISPLAY_STATE_CIRCULATION:
        ucPfm4Uv = TRUE;
        break;
        // BEGIN ADD FOR V2
    case DISPLAY_STATE_TOC:
        // END ADD FOR V2
        if (DISPLAY_SUB_STATE_TOC_OXIDATION == Display.CurState.ucSub)
        {
            ucPfm4Uv = TRUE;
        }
        break;
    case DISPLAY_STATE_UF_CLEAN:
        break;
    default:
        break;
    }

    ulOldTime = Display.ulStartSecond;

    Display.CurState        = Display.TgtState;
    Display.TgtState.ucMain = DISPLAY_STATE_BUTT;
    Display.TgtState.ucSub  = DISPLAY_SUB_STATE_BUTT;
    Display.ulStartSecond   = RTC_Get_Second();

    if (ucPfm4Uv)
    {
        Disp_CumulateUvTime(ulOldTime,Display.ulStartSecond);    
    }

    if (ucMeas && !Display.bit1Measured)
    {
        if (Display.iConductivity[0]) // VALID measurement
        {
            Display.bit1Measured = 1;
        }
    }

    switch(Display.CurState.ucMain) //hook after state transfer
    {
    case DISPLAY_STATE_IDLE:
        if (OldState.ucMain != DISPLAY_STATE_IDLE)
        {
            Display.usIdleTimes = 0;
        }
        // fall through
    case DISPLAY_STATE_DECOMPRESSION:
        //Disp_SetAlarm(DISP_ALARM_PRODUCT_RS,FALSE);
        //Disp_SetAlarm(DISP_ALARM_TEMPERATURE,FALSE);
        break;
    case DISPLAY_STATE_CIRCULATION:
        break;
    case DISPLAY_STATE_NORMAL_TAKING_WATER:
        RelayLogicCtrl(RELAY_LED_WATER_ANYIN,TRUE);
        break;
    case DISPLAY_STATE_QUANTITY_TAKING_WATER:
        break;
    case DISPLAY_STATE_UF_CLEAN:
        break;
    }

    // notify
    if (Display.bit1AuthState)
    {
        uint8 dataBuf[4];
        dataBuf[0] = 0;
        dataBuf[1] = 1;
        dataBuf[2] = RF_CMD_STATUS;
        dataBuf[3] = Display.CurState.ucMain;
        UartCmdSerialWrite(SAPP_CMD_DATA | 0x80,dataBuf,4);        
    }
    
}

void Disp_Move2DstPage(void)
{
#if (IWDG_SUPPORT > 0)
    IWDG_Feed();  
#endif

    Display.curPage = Display.tgtPage;

    Display.tgtPage.ucMain = DISPLAY_PAGE_BUTT;
    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_BUTT;

    Display.ucLastShow = 0XFF;

}

int Disp_get_conduct(void)
{
    uint8_t ret;
    uint8_t idx = 0;
    uint32_t result[4] = {0,0,0,0};
    int iLoop;
    
    AD7799_Start(AD7799_CH0,AD7799_GAIN0,AD7799_UR1,(AD7799_MOD0<<5));

    for (iLoop = 0 ;iLoop < 4; iLoop++)
    {
        ret = sAD7799_WaitBusy();
    
        if (!ret)
        {
            result[idx++] = AD7799_Read();
        }
        else
        {
			UartCmdPrintf(VOS_LOG_WARNING,"AD FAIL");
        }
        
    }

    for (iLoop = 1; iLoop < idx; iLoop++)
    {
        result[0] += result[iLoop];
    }

    if (idx > 0)
    {
        result[0] /= idx;
    }

    // move to idle
    AD7799_Start(AD7799_CH0,AD7799_GAIN0,AD7799_UR1,(AD7799_MOD2<<5));  
    return result[0];
}

int Disp_get_temperature(void)
{
    uint8_t ret;
    uint8_t idx = 0;
    uint32_t result[4] = {0,0,0,0};
    int iLoop;
    
    AD7799_Start(AD7799_CH1,AD7799_GAIN0,AD7799_UR1,(AD7799_MOD0<<5));

    for (iLoop = 0 ;iLoop < 4; iLoop++)
    {
        ret = sAD7799_WaitBusy();
    
        if (!ret)
        {
            result[idx++] = AD7799_Read();
        }
        else
        {
			UartCmdPrintf(VOS_LOG_WARNING,"AD FAIL");
        }
            
    }

    for (iLoop = 1; iLoop < idx; iLoop++)
    {
        result[0] += result[iLoop];
    }

    if (idx > 0)
    {
        result[0] /= idx;
    }

    // move to idle
    AD7799_Start(AD7799_CH0,AD7799_GAIN0,AD7799_UR1,(AD7799_MOD2<<5));
    
    return result[0] ;
}

#define e (2.718281828)

//各个批次公式选择
#define P180412
//#define P171207
//#define P170316
//#define P161128 
//#define P160524
//#define P160331

#ifdef P180412
void Disp_Calc_Conductivity(int iChl,int iconduct, int itemp)
{
	double a = 0;
    double valueTMP = itemp*1.0/1;
    double valueRES = iconduct*1.0/1;
    double tempResistance,tempResistivity,tempTemperature;
  
    valueTMP = valueTMP*(Display.cfg.cfg1.TEMPCONSTANT[0]/1000.00);
    tempResistance = (valueTMP * 4900) / (2480 - valueTMP);//Rt Calculation 
    tempTemperature = 3950 / ((3950/298.15) + log(tempResistance) - log(10000)) - 273.15;//T Calculation	
  
	if (iChl == 0)  //产水电阻，通道1
    {
		Display.iTemperature[iChl] = tempTemperature*1000; // float 2 integer conversion
        // calculate resistence
		tempResistivity = 31500*(pow(valueRES,-1.43));
		if(tempTemperature <= 15)
		{
			a = 0.001 * tempTemperature + 0.0257;
		}
		if(tempTemperature > 15)
		{
			a = 0.0017 * tempTemperature + 0.014;
		}
		tempResistivity = tempResistivity*(1.0 + a*(tempTemperature-25)); // Display.cfg.TEMPCONSTANT
		Display.iConductivity[iChl] = tempResistivity*Display.cfg.cfg1.CELLCONSTANT[iChl]; // float 2 integer conversions
		if (Display.iConductivity[iChl] >= HIGH_RES)
		{
			Display.iConductivity[iChl] = 18200;
		}
		else if (Display.iConductivity[iChl] < LOW_RES)
		{
			//Display.iConductivity = 1000;
		}

		if (Display.aulBestCdt[iChl] < Display.iConductivity[iChl])
		{
			Display.aulBestCdt[iChl] = Display.iConductivity[iChl];
		}
	}
	if (iChl == 1)   //进水电导率，通道2
    {
		tempResistivity = 0.000000003326*(pow(valueRES,3))+ 0.000003586*(pow(valueRES,2)) + 0.0297*valueRES;
		tempResistivity = tempResistivity*(1.0 + 0.02*(25 - (Display.iTemperature[0]/1000)));
		Display.iConductivity[iChl] = (tempResistivity * Display.cfg.cfg1.CELLCONSTANT[iChl]);
    } 
} 
#endif

#ifdef P171207
void Disp_Calc_Conductivity(int iChl,int iconduct, int itemp)
{
	double a = 0;
    double valueTMP = itemp*1.0/1;
    double valueRES = iconduct*1.0/1;
    double tempResistance,tempResistivity,tempTemperature;
  
    valueTMP = valueTMP*(Display.cfg.cfg1.TEMPCONSTANT[0]/1000.00);
    tempResistance = (valueTMP * 4850) / (2480 - valueTMP);//Rt Calculation 
    tempTemperature = 3950 / ((3950/298.15) + log(tempResistance) - log(10000)) - 273.15;//T Calculation	
  
	if (iChl == 0)  //产水电阻，通道1
    {
		Display.iTemperature[iChl] = tempTemperature*1000; // float 2 integer conversion
        // calculate resistence
		tempResistivity = 31500*(pow(valueRES,-1.43));
		if(tempTemperature < 20)
		{
			a = 0.00097 * tempTemperature + 0.0262;
		}
		if(tempTemperature >= 20)
		{
			a = 0.00175 * tempTemperature + 0.011;
		}
		tempResistivity = tempResistivity*(1.0 + a*(tempTemperature-25)); // Display.cfg.TEMPCONSTANT
		Display.iConductivity[iChl] = tempResistivity*Display.cfg.cfg1.CELLCONSTANT[iChl]; // float 2 integer conversions
		if (Display.iConductivity[iChl] >= HIGH_RES)
		{
			Display.iConductivity[iChl] = 18200;
		}
		else if (Display.iConductivity[iChl] < LOW_RES)
		{
			//Display.iConductivity = 1000;
		}

		if (Display.aulBestCdt[iChl] < Display.iConductivity[iChl])
		{
			Display.aulBestCdt[iChl] = Display.iConductivity[iChl];
		}
	}
	if (iChl == 1)   //进水电导率，通道2
    {
		tempResistivity = 0.000000003326*(pow(valueRES,3))+ 0.000003586*(pow(valueRES,2)) + 0.0297*valueRES;
		tempResistivity = tempResistivity*(1.0 + 0.02*(25 - (Display.iTemperature[0]/1000)));
		Display.iConductivity[iChl] = (tempResistivity * Display.cfg.cfg1.CELLCONSTANT[iChl]);
    } 
} 
#endif

#ifdef P170316
void Disp_Calc_Conductivity(int iChl,int iconduct, int itemp)
{
	double a = 0;
    double valueTMP = itemp*1.0/1;
    double valueRES = iconduct*1.0/1;
    double tempResistance,tempResistivity,tempTemperature;
  
    valueTMP = valueTMP*(Display.cfg.cfg1.TEMPCONSTANT[0]/1000.00);
    tempResistance = (valueTMP * 4900) / (2480 - valueTMP);//Rt Calculation 
    tempTemperature = 3950 / ((3950/298.15) + log(tempResistance) - log(10000)) - 273.15;//T Calculation	
  
	if (iChl == 0)  //产水电阻，通道1
    {
		Display.iTemperature[iChl] = tempTemperature*1000; // float 2 integer conversion
        // calculate resistence
		tempResistivity = 26100/(pow(valueRES,1.4));
		if(tempTemperature < 20)
		{
			a = 0.00102 * tempTemperature + 0.027;
		}
		if(tempTemperature >= 20)
		{
			a = 0.00157 * tempTemperature + 0.018;
		}
		tempResistivity = tempResistivity*(1.0 + a*(tempTemperature-25)); // Display.cfg.TEMPCONSTANT
		Display.iConductivity[iChl] = tempResistivity*Display.cfg.cfg1.CELLCONSTANT[iChl]; // float 2 integer conversions
		if (Display.iConductivity[iChl] >= HIGH_RES)
		{
			Display.iConductivity[iChl] = 18200;
		}
		else if (Display.iConductivity[iChl] < LOW_RES)
		{
			//Display.iConductivity = 1000;
		}

		if (Display.aulBestCdt[iChl] < Display.iConductivity[iChl])
		{
			Display.aulBestCdt[iChl] = Display.iConductivity[iChl];
		}
	}
	if (iChl == 1)   //进水电导率，通道2
    {
		tempResistivity = 0.000000003326*(pow(valueRES,3))+ 0.000003586*(pow(valueRES,2)) + 0.0297*valueRES;
		tempResistivity = tempResistivity*(1.0 + 0.02*(25 - (Display.iTemperature[0]/1000)));
		Display.iConductivity[iChl] = (tempResistivity * Display.cfg.cfg1.CELLCONSTANT[iChl]);
    } 
} 
#endif

#ifdef P161128 
void Disp_Calc_Conductivity(int iChl,int iconduct, int itemp)
{
	double a = 0;
    double valueTMP = itemp*1.0/1;
    double valueRES = iconduct*1.0/1;
    double tempResistance,tempResistivity,tempTemperature;
  
    valueTMP = valueTMP*(Display.cfg.cfg1.TEMPCONSTANT[0]/1000.00);
    tempResistance = (valueTMP * 4900) / (2480 - valueTMP);//Rt Calculation 
    tempTemperature = 3950 / ((3950/298.15) + log(tempResistance) - log(10000)) - 273.15;//T Calculation	
  
    if (iChl == 0)  //产水电阻，通道1
    {
		Display.iTemperature[iChl] = tempTemperature*1000; // float 2 integer conversion
		// calculate resistence
		tempResistivity = 26100/(pow(valueRES,1.4));
		if(tempTemperature < 15)
		{
			a = 0.00091 * tempTemperature + 0.027;
		}
		if(tempTemperature >= 15)
		{
			a = 0.0015 * tempTemperature + 0.019;
		}
		tempResistivity = tempResistivity*(1.0 + a*(tempTemperature-25)); // Display.cfg.TEMPCONSTANT
		Display.iConductivity[iChl] = tempResistivity*Display.cfg.cfg1.CELLCONSTANT[iChl]; // float 2 integer conversions
		if (Display.iConductivity[iChl] >= HIGH_RES)
		{
			Display.iConductivity[iChl] = 18200;
		}
		else if (Display.iConductivity[iChl] < LOW_RES)
		{
			//Display.iConductivity = 1000;
		}

		if (Display.aulBestCdt[iChl] < Display.iConductivity[iChl])
		{
			Display.aulBestCdt[iChl] = Display.iConductivity[iChl];
		}
	}
	if (iChl == 1)   //进水电导率，通道2
    {
		tempResistivity = 0.000000003326*(pow(valueRES,3))+ 0.000003586*(pow(valueRES,2)) + 0.0297*valueRES;
		tempResistivity = tempResistivity*(1.0 + 0.02*(25 - (Display.iTemperature[0]/1000)));
		Display.iConductivity[iChl] = (tempResistivity * Display.cfg.cfg1.CELLCONSTANT[iChl]);
    }
} 

#endif

#ifdef P160524
void Disp_Calc_Conductivity(int iChl,int iconduct, int itemp)
{
	double a = 0;
	double valueTMP = itemp*1.0/1;
	double valueRES = iconduct*1.0/1;
	double tempResistance,tempResistivity,tempTemperature;

	valueTMP = valueTMP*(Display.cfg.cfg1.TEMPCONSTANT[0]/1000.00);//1.0114
	tempResistance = (valueTMP * 4800) / (2475 - valueTMP);//Rt Calculation 
	tempTemperature = 3950 / ((3950/298.15) + log(tempResistance) - log(10000)) - 273.15;//T Calculation	
	if (iChl == 0)
	{
		Display.iTemperature[iChl] = tempTemperature*1000; // float 2 integer conversion
		tempResistivity = 25700/(pow(valueRES,1.4));
		if(tempTemperature < 15)
		{
			a = 0.00091 * tempTemperature + 0.0277;
		}
		if(tempTemperature >= 15)
		{
			a = 0.0015 * tempTemperature + 0.02;
		}
		tempResistivity = tempResistivity*(1.0 + a*(tempTemperature-25)); // Display.cfg.TEMPCONSTANT
		Display.iConductivity[iChl] = tempResistivity*Display.cfg.cfg1.CELLCONSTANT[iChl]; // float 2 integer conversions
		if (Display.iConductivity[iChl] >= HIGH_RES)
		{
			Display.iConductivity[iChl] = 18200;
		}
		else if (Display.iConductivity[iChl] < LOW_RES)
		{
        //Display.iConductivity = 1000;
		}

		if (Display.aulBestCdt[iChl] < Display.iConductivity[iChl])
		{
			Display.aulBestCdt[iChl] = Display.iConductivity[iChl];
		}
	}
	if (iChl == 1)   //进水电导率，通道2
    {
		tempResistivity = 0.000000003326*(pow(valueRES,3))+ 0.000003586*(pow(valueRES,2)) + 0.0297*valueRES;
		tempResistivity = tempResistivity*(1.0 + 0.02*(25 - (Display.iTemperature[0]/1000)));
		Display.iConductivity[iChl] = (tempResistivity * Display.cfg.cfg1.CELLCONSTANT[iChl]);
    }
} 
#endif

#ifdef P160331
void Disp_Calc_Conductivity(int iChl,int iconduct, int itemp)
{
	double a = 0;
	double valueTMP = itemp*1.0/1;
	double valueRES = iconduct*1.0/1;
	double tempResistance,tempResistivity,tempTemperature;

	valueTMP = valueTMP*(Display.cfg.cfg1.TEMPCONSTANT[0]/1000.00);//1.0114;//
	tempResistance = (valueTMP * 5050) / (2500 - valueTMP);//Rt Calculation 
	tempTemperature = 3290 / ((3290/298.15) + log(tempResistance) - log(10000)) - 273.15;//T Calculation	
	// T=3290/((3290/298.15)+LN(R)-LN(10000))-273.15
	if (iChl == 0)
	{
//    tempTemperature = (tempTemperature*1000)/1000;
		Display.iTemperature[iChl] = tempTemperature*1000; // float 2 integer conversion
    // calculate resistence
		tempResistivity = 28900/(pow(valueRES,1.4));
		if(tempTemperature < 15)
		{
			a = 0.00102 * tempTemperature + 0.0263;
		}
			if(tempTemperature >= 15)
		{
			a = 0.0014 * tempTemperature + 0.0195;
		}
		tempResistivity = tempResistivity*(1.0 + a*(tempTemperature-25)); // Display.cfg.TEMPCONSTANT
		Display.iConductivity[iChl] = tempResistivity*Display.cfg.cfg1.CELLCONSTANT[iChl]; // float 2 integer conversions
		if (Display.iConductivity[iChl] >= HIGH_RES)
		{
			Display.iConductivity[iChl] = 18200;
		}
		else if (Display.iConductivity[iChl] < LOW_RES)
		{
		//Display.iConductivity = 1000;
		}
		if (Display.aulBestCdt[iChl] < Display.iConductivity[iChl])
		{
			Display.aulBestCdt[iChl] = Display.iConductivity[iChl];
		}
	}
	if (iChl == 1)   //进水电导率，通道2
    {
		tempResistivity = 0.000000003326*(pow(valueRES,3))+ 0.000003586*(pow(valueRES,2)) + 0.0297*valueRES;
		tempResistivity = tempResistivity*(1.0 + 0.02*(25 - (Display.iTemperature[0]/1000)));
		Display.iConductivity[iChl] = (tempResistivity * Display.cfg.cfg1.CELLCONSTANT[iChl]);
    } 
}
#endif

double Disp_Calc_TOC_BASE(int iChl,int iconduct, int itemp)
{
	double valueTMP = itemp*1.0/1;
    double valueRES = iconduct*1.0/1;
	static double tempB25;

	valueTMP = valueTMP*(Display.cfg.cfg1.TEMPCONSTANT[0]/1000.00);//1.0114  
    if (iChl == 2)
    { 
		tempB25 = (1+(25-(Display.iTemperature[0]/1000))*0.03)*valueRES; //用1通道温度补偿
    }
	return tempB25;
} 

void Disp_Calc_TOC_PEAK(int iChl,int iconduct, int itemp, int temp_B)
{
	double valueTMP = itemp*1.0/1;
    double valueRES = iconduct*1.0/1;
    double valueB = temp_B*1.0/1;
    double tempResistivity,tempP25;

    valueTMP = valueTMP*(Display.cfg.cfg1.TEMPCONSTANT[0]/1000.00);//1.0114
    if (iChl == 2)
    {
		tempP25 = (1+(25-(Display.iTemperature[0]/1000))*0.03)*(valueRES-valueB);
		//tempResistivity = 97.5*(log(tempP25)) - 623;
		tempResistivity = 97.5*(log(tempP25));
		if(tempResistivity > 647)
		{
			tempResistivity -= 647;
		}
		else
		{
			if((647-tempResistivity)<=6)
			    tempResistivity = 5.0;
			else if((647-tempResistivity)<=12 && (647-tempResistivity)>6)
				tempResistivity = 4.0;
			else if((647-tempResistivity)<=18 && (647-tempResistivity)>12)
			    tempResistivity = 3.0;
			else 
	            tempResistivity = 2.0;
		}     
		Display.iConductivity[iChl] =  tempResistivity*Display.cfg.cfg1.CELLCONSTANT[iChl];
		if(Display.iConductivity[iChl]<=2000&&Display.iConductivity[iChl]>0)
		{
			Display.iConductivity[iChl] = 2000;
		}
	}
} 

int Disp_GetTankState(void)
{
    int result = GetInputLogicStatus(INPUT_TANK_STATE); // late implement
    if (TANK_VALID != result)
    {
        Disp_SetAlarm(DISP_ALARM_TANK_EMPTY,TRUE);
    }
    else // low valid
    {
        Disp_SetAlarm(DISP_ALARM_TANK_EMPTY,FALSE);
    } 
    return (TANK_VALID == result); 
}


void Disp_ShowTime(void)
{
#ifdef SHOW_TIME    
    LCD_DrawRectangle(rectTime.left,rectTime.top,rectTime.right,rectTime.bottom,MAGENTA);

    sprintf((char *)Config_buff,TIME_FORMAT_SHORT,timer.w_year,timer.w_month,timer.w_date,timer.hour,timer.min);

    curFont->DrawText(rectTime.left + TIME_RECT_MARGIN,rectTime.top + TIME_RECT_MARGIN,(char *)Config_buff,BLACK,WHITE);
#endif    
}

void Disp_ShowEmptyTank(int xoff, int yoff)
{
	char *text;
    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
		||DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        text = "Empty Tank";
    }
    else
    {
        text = "水箱空";
    }
   
    Disp_WorkState(xoff,yoff,text);
}

typedef enum
{
    CHECK_SHOW_PACK = 0,
    CHECK_SHOW_UV,
    CHECK_SHOW_FILTER,
    CHECK_SHOW_TANK,
    CHECK_SHOW_PRODUCT,
    CHECK_SHOW_PRODUCT1,
    CHECK_SHOW_TEMPERATURE,
    CHECK_SHOW_TEMPERATURE1,
    
#ifdef TOC
    CHECK_SHOW_TOC_PB,    // P/B<1.3
    CHECK_SHOW_TOC_B25,   //B25>300
    CHECK_SHOW_TOC_VALUE, // VALUE
#endif

#ifdef UF_FUNCTION        
    CHECK_SHOW_UF_CLEAN,
#endif    
    CHECK_SHOW_NUM,
}CHECK_SHOW_EUNM;

void Disp_ShowCheckState(int xoff,int yoff)
{
    uint8_t aucShow[CHECK_SHOW_NUM];
    uint8_t iItems ;
    uint8_t showType = 0; // 0 for alarm, 1 for cm state
    uint8_t update_bg = Display.bit3ShowAlarm;

    iItems = Display.bit1PackCheck 
             + Display.bit1FilterCheck 
             + Display.bit1UVCheck 
             + Display.aAlarm[DISP_ALARM_PRODUCT_RS].bit1Triggered
             + Display.aAlarm[DISP_ALARM_PRODUCT_RS1].bit1Triggered
             + Display.aAlarm[DISP_ALARM_TEMPERATURE].bit1Triggered
             + Display.aAlarm[DISP_ALARM_TEMPERATURE1].bit1Triggered
             + Display.aAlarm[DISP_ALARM_TANK_EMPTY].bit1Fired
             + Display.aAlarm[DISP_ALARM_TOC_PB].bit1Triggered
             + Display.aAlarm[DISP_ALARM_TOC_B25].bit1Triggered
             + Display.aAlarm[DISP_ALARM_TOC_VALUE].bit1Triggered
             + Display.bit1PendingUFClean;
    if (0 == iItems)
    {
        if (0XFF != Display.ucLastShow)
        {
            Display.ucLastShow = 0XFF;
        }

        if (Display.bit1ShowCmState)
        {
            // LCD_Fill(xoff,yoff,LCD_W - 1 ,yoff + curFont->sizeY,Display.usBackColor);
            //报警消失出现红条问题
            if(DISPLAY_PAGE_USER_SET != Display.curPage.ucMain
               && DISPLAY_PAGE_ENG_SET != Display.curPage.ucMain)
            {
           	    LCD_Fill(xoff,yoff,LCD_W - 1 ,yoff + curFont->sizeY,Display.usBackColor);
            }
            Display.bit1ShowCmState = FALSE;
        }

        Display.bit3ShowAlarm = DISP_SHOW_ALARM_NONE;

        if (Display.bit3ShowAlarm != update_bg)
        {
            // update bg
            Disp_Invalidate();
        }    
        return;
    }

    // avoid  to display states in setting stage
    if (DISPLAY_PAGE_QUANTITY_TAKING_WATER == Display.curPage.ucMain
        && DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_SETTING == Display.curPage.ucSub)
    {
        return;
    }

    if (DISPLAY_PAGE_USER_SET == Display.curPage.ucMain
        || DISPLAY_PAGE_ENG_SET == Display.curPage.ucMain
        || DISPLAY_PAGE_UF_CLEAN == Display.curPage.ucMain
        || DISPLAY_PAGE_DEGASS == Display.curPage.ucMain)
    {
        return;
    }  
    
    iItems = 0;
    Display.bit3ShowAlarm = DISP_SHOW_ALARM_NONE;

    if (Display.bit1PackCheck)
    {
		aucShow[iItems++]  = CHECK_SHOW_PACK;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_CM;
    }
    if (Display.bit1UVCheck)
    {
		aucShow[iItems++]  = CHECK_SHOW_UV;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_CM;
    }

    if (Display.bit1FilterCheck)
    {
        aucShow[iItems++]  = CHECK_SHOW_FILTER;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_CM;
    }

    if (Display.aAlarm[DISP_ALARM_TANK_EMPTY].bit1Fired)
    {
        aucShow[iItems++]  = CHECK_SHOW_TANK;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_SENSOR; // Supersede consume material check state
    }

    if (Display.aAlarm[DISP_ALARM_PRODUCT_RS].bit1Triggered)
    {
        aucShow[iItems++]  = CHECK_SHOW_PRODUCT;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_SENSOR;
    }
    
    if (Display.aAlarm[DISP_ALARM_PRODUCT_RS1].bit1Triggered)
    {
        aucShow[iItems++]  = CHECK_SHOW_PRODUCT1;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_SENSOR;
    }

    if (Display.aAlarm[DISP_ALARM_TEMPERATURE].bit1Triggered)
    {
        aucShow[iItems++]  = CHECK_SHOW_TEMPERATURE;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_SENSOR;
    }   

    if (Display.aAlarm[DISP_ALARM_TEMPERATURE1].bit1Triggered)
    {
        aucShow[iItems++]     = CHECK_SHOW_TEMPERATURE1;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_SENSOR;
    } 
#ifdef TOC
    if (Display.aAlarm[DISP_ALARM_TOC_PB].bit1Triggered)
    {
        aucShow[iItems++]  = CHECK_SHOW_TOC_PB;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_SENSOR;
    }
	if (Display.aAlarm[DISP_ALARM_TOC_B25].bit1Triggered)
    {
        aucShow[iItems++]  = CHECK_SHOW_TOC_B25;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_SENSOR;
    }
	if (Display.aAlarm[DISP_ALARM_TOC_VALUE].bit1Triggered)
    {
        aucShow[iItems++]  =  CHECK_SHOW_TOC_VALUE;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_SENSOR;
    }
#endif

#ifdef UF_FUNCTION        
    if (Display.bit1PendingUFClean)
    {
        aucShow[iItems++]  = CHECK_SHOW_UF_CLEAN;   
        Display.bit3ShowAlarm = DISP_SHOW_ALARM_UF_CLEAN;
    }
#endif
    Display.ucRollIdx = Display.ucRollIdx % iItems;

    if (Display.bit3ShowAlarm != update_bg)
    {
        // update ...
        Disp_Invalidate();
    }
    
    switch(aucShow[Display.ucRollIdx])
    {
    case CHECK_SHOW_PACK: // show pack
        if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
            || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
        {
            strcpy((char *)Config_buff,"EXCH.U PACK");
        }
        else
        {
            strcpy((char *)Config_buff,"请更换纯化柱");
        }
        showType = 1;
        break;
    case CHECK_SHOW_UV: // show uv
        if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
            || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
        {
            strcpy((char *)Config_buff,"EXCH.UV LAMP");
        }
        else
        {
            strcpy((char *)Config_buff,"请更换紫外灯");
        }
        showType = 1;
        break;
    case CHECK_SHOW_FILTER: // show filter
        if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
            || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
        {
            strcpy((char *)Config_buff,"EXCH.FINAL FILTER");
        }
        else
        {
            strcpy((char *)Config_buff,"请更换过滤柱");
        }
        showType = 1;
        break;
    case CHECK_SHOW_TANK:
        if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
            || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
        {
            strcpy((char *)Config_buff,"EMPTY TANK");
        }
        else
        {
            strcpy((char *)Config_buff,"水箱空");
        }        
        break;
    case CHECK_SHOW_PRODUCT:
        switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
            strcpy((char *)Config_buff,"PRODUCT < S.P.");
            break;
        case DISP_LAN_GERMANY:
            strcpy((char *)Config_buff,"AUSGABE < S.P.");
            break;
        default:
            strcpy((char *)Config_buff,"产水<设定值");
            break;
        }        
        break;
    case CHECK_SHOW_PRODUCT1:
        switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
            strcpy((char *)Config_buff,"FEED PRODUCT > S.P.");
            break;
        case DISP_LAN_GERMANY:
            strcpy((char *)Config_buff,"Eingang AUSGABE > S.P.");
            break;
        default:
            strcpy((char *)Config_buff,"进水 > 设定值");
            break;
        }        
        break;
    case CHECK_SHOW_TEMPERATURE:
        if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
            ||DISP_LAN_GERMANY == Display.cfg.cfg1.language)
        {
            strcpy((char *)Config_buff,"TEMP > S.P.");
        }
        else
        {
            strcpy((char *)Config_buff,"温度>设定值");
        }        
        break;
    case CHECK_SHOW_TEMPERATURE1:
        switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
            strcpy((char *)Config_buff,"FEED TEMP > S.P.");
            break;
        case DISP_LAN_GERMANY:
            strcpy((char *)Config_buff,"Eingang TEMP > S.P.");
            break;
        default:
            strcpy((char *)Config_buff,"进水 温度>设定值");
            break;
        }        
        break;
/*------------TOC报警显示------------*/
/*----------日期：20161109-----------*/
#ifdef TOC
    case CHECK_SHOW_TOC_PB:
		switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
		case DISP_LAN_GERMANY:
            strcpy((char *)Config_buff,"CHECK UV LAMP");
            break;
        default:
            strcpy((char *)Config_buff,"请检查紫外灯");
            break;
        }        
        break;
	case CHECK_SHOW_TOC_B25:
		switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
		case DISP_LAN_GERMANY:
            strcpy((char *)Config_buff,"CHECK U PACK");
            break;
        default:
            strcpy((char *)Config_buff,"请检查纯化柱");
            break;
        }        
        break;
	case CHECK_SHOW_TOC_VALUE:
		switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
		case DISP_LAN_GERMANY:
            strcpy((char *)Config_buff,"CHECK TOC SENSOR");
            break;
        default:
            strcpy((char *)Config_buff,"请检查TOC电极");
            break;
        }        
        break;
#endif 
#ifdef UF_FUNCTION        
    case CHECK_SHOW_UF_CLEAN:
        switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
            strcpy((char *)Config_buff,"UF Clean");
            break;
        case DISP_LAN_GERMANY:
            strcpy((char *)Config_buff,"UF Spuelung");
            break;
        default:
            strcpy((char *)Config_buff,"UF 清洗");
            break;
        }
        break;
#endif        
     default:
        return;
    }

    if (Display.ucLastShow == aucShow[Display.ucRollIdx]) 
    {
        // flash
        if (Display.bit1Flash)
        {
            Disp_CleanTail(xoff,yoff,"",Display.usBackColor);            
            Display.bit1Flash = FALSE;
        }
        else
        {
            Display.bit1Flash = TRUE; // content displayed 
            curFont->DrawText(xoff,yoff,Config_buff,Display.usForColor,(showType) ? YELLOW : Display.usBackColor);           
            // clean tail
            Disp_CleanTail(xoff,yoff,(char *)Config_buff,Display.usBackColor);
        }
    }
    else
    {
        Display.bit1Flash = TRUE; // content displayed
        curFont->DrawText(xoff,yoff,Config_buff,Display.usForColor,(showType) ? YELLOW : Display.usBackColor);
        // clean tail
        Disp_CleanTail(xoff,yoff,(char *)Config_buff,Display.usBackColor);
        Display.ucLastShow = aucShow[Display.ucRollIdx];       
        Display.ucRollIdx++;   
        Display.bit1ShowCmState = TRUE;
    }
}

void Disp_UpdateTime()
{
    if (stimer.min != timer.min)
    {
        //LCD_ClearRect(rectTime.left,rectTime.top,rectTime.right,rectTime.bottom,WHITE);
        stimer = timer;   
        Disp_ShowTime();
    }
}

void Disp_DisplayIdlePage(void)
{
    char *text;
    
    //Display.usBackColor = WHITE;
    //Display.usForColor = BLACK;

    LCD_Clear(Display.usBackColor);

    // display text info
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        text = "Ready";
        break;
    case DISP_LAN_GERMANY:
        text = "Bereit";
        break;
    default:
        text = "就 绪";
        break;
    }
    Disp_WorkState(20,20,text);
    Disp_ShowTime();   
}

int Disp_Sensors_Item(int chl,int big,int mask,int xoff,int yoff)
{
    char buf[16];
    u16 usBackColor = Display.usBackColor;
    int iOrgYoff = yoff;

    //Disp_Int2float1(Display.iConductivity,buf);
	if (mask & 0x1)
    {
        if (DISPLAY_SENSOR_2 == chl)
        {
            int iXSize ;
			//FEED_KEY == 0,不显示进水电导率
			if(0 == Display.cfg.cfg2.FEED_KEY)
			{
			    switch(Display.cfg.cfg1.language)
                {
                case DISP_LAN_ENGLISH:
					strcpy(buf,"  ");
					break;
                case DISP_LAN_GERMANY:
                    strcpy(buf,"  ");
                    break;
                default : 
                    strcpy(buf,"  ");
                    break;
                 }
			}
			else
			{
				switch(Display.cfg.cfg1.language)
                {
                case DISP_LAN_ENGLISH:
                     strcpy(buf,"Feed    ");
                     break;
                case DISP_LAN_GERMANY:
                     strcpy(buf,"Eingang ");
                     break;
                default : 
                     strcpy(buf,"进水    ");
                     break;
                 }
			}
            iXSize = (strlen((char *)buf) + (11))*curFont-> sizeX;
            if (xoff + iXSize >= LCD_W)
            {
                xoff -= ((xoff + iXSize - LCD_W) + 5);
            }
			//不显示TOC,进水电导上移一行
			if((DISPLAY_SENSOR_2 == chl) && (1 != Display.cfg.cfg2.TOC_SHOW) )
			{
			    curFont->DrawText(xoff-40,yoff-30,(u8 *)buf,BLACK,usBackColor);
			}
			else
			{
                curFont->DrawText(xoff-40,yoff,(u8 *)buf,BLACK,usBackColor);//
			}
            iXSize = strlen((char *)buf)*curFont->sizeX;            
            xoff += iXSize - 5;
        }
	           
        if (0 == Display.cfg.cfg2.UNIT && DISPLAY_SENSOR_1 == chl) //DISPLAY_SENSOR_1 == chl
        {
            if (Display.iConductivity[chl] >= LOW_RES)
            {
                Disp_Int2floatFormat((Display.iConductivity[chl] + 50)/100,2,1,buf);
            }
            else
            {
                strcpy(buf," < 1");
            }       
            sprintf((char *)Config_buff,"%s",buf);
        }
		else if(1 == Display.cfg.cfg2.UNIT && DISPLAY_SENSOR_1 == chl)
		{
		    uint32_t fus = (1000*1000)/Display.iConductivity[chl];
            Disp_Int2floatFormat(fus,1,3,buf);
            sprintf((char *)Config_buff,"%s",buf);
		}
		//进水电导率值
		else
        {
            uint32_t fus = (Display.iConductivity[chl]+5)/100; //20160922
            Disp_Int2floatFormat(fus,1,1,buf);
            sprintf((char *)Config_buff,"%s",buf); 
        }               
    #ifdef CODEGB_48  
        if (big) Disp_SelectFont(&font48);  
    #endif
		if (DISPLAY_SENSOR_1 == chl)
	    {
			curFont->DrawText(xoff,yoff,Config_buff,BLACK,usBackColor);
	    }
		 //进水电导不显示
		else if((DISPLAY_SENSOR_2 == chl) && (0 == Display.cfg.cfg2.FEED_KEY) ) 
        {
             //FEED_KEY == 0 不显示进水电导率值
        }
		 //TOC不显示
		else if((DISPLAY_SENSOR_2 == chl) && (1 != Display.cfg.cfg2.TOC_SHOW) )
        {
            curFont->DrawText(xoff-35,yoff - 30,Config_buff,BLACK,usBackColor);
        }
		 //显示TOC,进水电导
        else 
        {
            curFont->DrawText(xoff-35,yoff,Config_buff,BLACK,usBackColor);
        }
        xoff += strlen((char *)Config_buff)*curFont->sizeX;
        yoff += curFont->sizeY;
    
    #ifdef CODEGB_48    
        if (big) Disp_SelectFont(oldfont);  
    #endif    
        yoff -= curFont->sizeY;
		if (0 == Display.cfg.cfg2.UNIT && DISPLAY_SENSOR_1 == chl) //DISPLAY_SENSOR_1 == chl
        {
            sprintf((char *)Config_buff," %s",UNIT_TYPE_0);
        }
        else
        {
            sprintf((char *)Config_buff,"%s",UNIT_TYPE_1);
        }
        if (DISPLAY_SENSOR_1 == chl)
	    {
			curFont->DrawText(xoff,yoff,Config_buff,BLACK,usBackColor);
	    }
		else if((DISPLAY_SENSOR_2 == chl) && (0 == Display.cfg.cfg2.FEED_KEY))
        {
            //FEED_KEY == 0 不显示进水电导率单位
        }
		 //不显示TOC，进水电导上移一行
		else if((DISPLAY_SENSOR_2 == chl) && (1 != Display.cfg.cfg2.TOC_SHOW) )
		{
		    curFont->DrawText(xoff-35,yoff-30,Config_buff,BLACK,usBackColor);
		}
        else
        {
            curFont->DrawText(xoff-35,yoff,Config_buff,BLACK,usBackColor);
        }
        yoff += 30;
    }
  
    if (mask & 0x2)
    {
        // Disp_Int2float1(Display.iTemperature,buf);
		Disp_Int2floatFormat((Display.iTemperature[chl] + 50)/100,1,1,buf);
		switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
			sprintf((char *)Config_buff,"Temp.   %s℃",buf);
            break;
        case DISP_LAN_GERMANY:
            sprintf((char *)Config_buff,"Temp.   %s℃",buf);
            break;
        default : // DISP_LAN_GERMANY
            sprintf((char *)Config_buff,"水温    %s℃",buf);
            break;
        }
#ifdef CODEGB_48 
	    if(DISPLAY_STATE_TOC != Display.CurState.ucMain && big == TRUE)
            curFont->DrawText(xoff-140,yoff,Config_buff,BLACK,usBackColor);
		else
			curFont->DrawText(xoff-90,yoff,Config_buff,BLACK,usBackColor);
#else
	    curFont->DrawText(xoff-90,yoff,Config_buff,BLACK,usBackColor);
#endif  
        //curFont->DrawText(xoff-90,yoff,Config_buff,BLACK,usBackColor);
        yoff += 30;
    }
	if ((mask & 0x8) && (1 == Display.cfg.cfg2.TOC_SHOW)) //toc
    {    
       if(Display.iConductivity[chl] == 0)
       {
		   sprintf((char *)Config_buff,"TOC     0 ppb");
           curFont->DrawText(xoff-40,yoff-30,Config_buff,BLACK,usBackColor);
           yoff += 30;
       }
	   else if(Display.iConductivity[chl]>99000)
	   {
           sprintf((char *)Config_buff,"TOC     >99 ppb");
           curFont->DrawText(xoff-40,yoff-30,Config_buff,BLACK,usBackColor);
           yoff += 30;
	   }
	   else if(TOC_Exception == 1)
	   {
	       //sprintf((char *)Config_buff,"TOC     2 ppb"); //0
	       sprintf((char *)Config_buff,"TOC     --"); //0
           curFont->DrawText(xoff-40,yoff-30,Config_buff,BLACK,usBackColor);
           yoff += 30;
	   }
	   else
	   {
	      Disp_Int2floatFormat_toc((Display.iConductivity[chl]+500)/1000,3,0,buf);//20161031  1000
          sprintf((char *)Config_buff,"TOC     %s ppb",buf);
          curFont->DrawText(xoff-40,yoff-30,Config_buff,BLACK,usBackColor);
          yoff += 30;
	   }
    }
    return yoff - iOrgYoff;
    //usBackColor = Display.usBackColor;
}

int Disp_Check_IsAlarm()
{
	if(Display.bit1PackCheck != TRUE
	   &&Display.bit1UVCheck   != TRUE
       &&Display.bit1FilterCheck != TRUE
	   &&Display.aAlarm[DISP_ALARM_PRODUCT_RS].bit1Triggered != TRUE
       &&Display.aAlarm[DISP_ALARM_PRODUCT_RS1].bit1Triggered != TRUE
       &&Display.aAlarm[DISP_ALARM_TEMPERATURE].bit1Triggered != TRUE
       &&Display.aAlarm[DISP_ALARM_TEMPERATURE1].bit1Triggered != TRUE
       &&Display.aAlarm[DISP_ALARM_TANK_EMPTY].bit1Fired != TRUE
       &&Display.aAlarm[DISP_ALARM_TOC_PB].bit1Triggered != TRUE
       &&Display.aAlarm[DISP_ALARM_TOC_B25].bit1Triggered != TRUE
       &&Display.aAlarm[DISP_ALARM_TOC_VALUE].bit1Triggered != TRUE
       &&DISPLAY_PAGE_QUANTITY_TAKING_WATER != Display.curPage.ucMain
       &&DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_SETTING != Display.curPage.ucSub)
	{
		return 0;
	}
	else 
	{
		return 1;
	}
}

void Disp_Sensors(int xoff,int yoff,int mask4s)
{
    u8 ucC3State = FALSE;
	int isAlarm; //判断是否有报警、耗材到期或定量取水
	
    if (DEV_TYPE_V1 != Display.cfg.cfg3.devtype)
    {
        return;
    }

    if (DISPLAY_PAGE_USER_SET == Display.curPage.ucMain
        || DISPLAY_PAGE_ENG_SET == Display.curPage.ucMain)
    {
        return;
    }

    //usBackColor = Display.usBackColor;
    if ((DISPLAY_STATE_CIRCULATION == Display.CurState.ucMain)
        ||(DISPLAY_STATE_NORMAL_TAKING_WATER == Display.CurState.ucMain)
        ||(DISPLAY_STATE_QUANTITY_TAKING_WATER == Display.CurState.ucMain))
    {
        ucC3State =  TRUE;
    }

    if (ucC3State 
        && Display.aAlarm[DISP_ALARM_PRODUCT_RS].bit1Fired)
    {   
        if ((RTC_Get_Second() - Display.aAlarm[DISP_ALARM_PRODUCT_RS].ulFireSec) >= DISP_ALARM_DURATION_TIME)
        {
            //usBackColor = RED;
            Display.aAlarm[DISP_ALARM_PRODUCT_RS].bit1Triggered = TRUE;
        }
    }

    if (ucC3State 
        && Display.aAlarm[DISP_ALARM_PRODUCT_RS1].bit1Fired)
    {    
        if ((RTC_Get_Second() - Display.aAlarm[DISP_ALARM_PRODUCT_RS1].ulFireSec) >= DISP_ALARM_DURATION_TIME)
        {
            //usBackColor = RED;
            Display.aAlarm[DISP_ALARM_PRODUCT_RS1].bit1Triggered = TRUE;
        }
    }
    
    if (ucC3State 
        && Display.aAlarm[DISP_ALARM_TEMPERATURE].bit1Fired)
    {
        if ((RTC_Get_Second() - Display.aAlarm[DISP_ALARM_TEMPERATURE].ulFireSec) >= DISP_ALARM_DURATION_TIME)
        {
            //usBackColor = RED;
            Display.aAlarm[DISP_ALARM_TEMPERATURE].bit1Triggered = TRUE;
        }
    }

    if (ucC3State 
        && Display.aAlarm[DISP_ALARM_TEMPERATURE1].bit1Fired)
    {
        if ((RTC_Get_Second() - Display.aAlarm[DISP_ALARM_TEMPERATURE1].ulFireSec) >= DISP_ALARM_DURATION_TIME)
        {
            //usBackColor = RED;
            Display.aAlarm[DISP_ALARM_TEMPERATURE1].bit1Triggered = TRUE;
        }
    }
#ifdef TOC
    if (ucC3State 
        && Display.aAlarm[DISP_ALARM_TOC_PB].bit1Fired)
    {
        if ((RTC_Get_Second() - Display.aAlarm[DISP_ALARM_TOC_PB].ulFireSec) >= DISP_TOC_ALARM_DURATION_TIME)
        {
            //usBackColor = RED;
            Display.aAlarm[DISP_ALARM_TOC_PB].bit1Triggered = TRUE;	
        }
    }
	if (ucC3State 
        && Display.aAlarm[DISP_ALARM_TOC_B25].bit1Fired)
    {
        if ((RTC_Get_Second() - Display.aAlarm[DISP_ALARM_TOC_B25].ulFireSec) >= DISP_TOC_ALARM_DURATION_TIME)
        {
            //usBackColor = RED;
            Display.aAlarm[DISP_ALARM_TOC_B25].bit1Triggered = TRUE;
        }
    }
	if (ucC3State 
        && Display.aAlarm[DISP_ALARM_TOC_VALUE].bit1Fired)
    {
        if ((RTC_Get_Second() - Display.aAlarm[DISP_ALARM_TOC_VALUE].ulFireSec) >= DISP_TOC_ALARM_DURATION_TIME)
        {
            //usBackColor = RED;
            Display.aAlarm[DISP_ALARM_TOC_VALUE].bit1Triggered = TRUE;
        }
    }
#endif
    isAlarm = Disp_Check_IsAlarm();

    if(DISPLAY_STATE_TOC != Display.CurState.ucMain 
		&& (isAlarm == 0 || 0 == Display.cfg.cfg2.TOC_SHOW))
    {
        //int ret;
        int offset;
        if (mask4s & (1 << DISPLAY_SENSOR_1))
        {
			offset = Disp_Sensors_Item(DISPLAY_SENSOR_1,TRUE, 3,xoff,yoff);
            // yoff += offset;
            yoff += (offset + 30);       
        }

#ifdef UF_FUNCTION
        if (mask4s & (1 << DISPLAY_SENSOR_2))
        {
            offset = Disp_Sensors_Item(DISPLAY_SENSOR_2,FALSE, 1,xoff,yoff);
            yoff += offset;          
        }
#endif
#ifdef PRO
        if (mask4s & (1 << DISPLAY_SENSOR_2))
        {
			offset = Disp_Sensors_Item(DISPLAY_SENSOR_2,FALSE, 1,xoff,yoff);
			//offset = Disp_Sensors_Item(DISPLAY_SENSOR_2,FALSE, 3,xoff,yoff);
            yoff += offset;      
        }
#endif      
#ifdef TOC
        if (mask4s & (1 << DISPLAY_SENSOR_TOC))
        {		
			//offset = Disp_Sensors_Item(DISPLAY_SENSOR_TOC,FALSE, 3,xoff,yoff); //20160926
            offset = Disp_Sensors_Item(DISPLAY_SENSOR_TOC,FALSE, 8,xoff,yoff); 
            yoff += offset;
        }
#endif       
	}

    else if(DISPLAY_STATE_TOC != Display.CurState.ucMain && isAlarm == 1)
    {
        //int ret;
        int offset;
        if (mask4s & (1 << DISPLAY_SENSOR_1))
        {
            offset = Disp_Sensors_Item(DISPLAY_SENSOR_1,FALSE, 3,xoff,yoff);
            // yoff += offset;
            yoff += (offset + 30);   
        }

#ifdef UF_FUNCTION
        if (mask4s & (1 << DISPLAY_SENSOR_2))
        {
            offset = Disp_Sensors_Item(DISPLAY_SENSOR_2,FALSE, 1,xoff,yoff);
            yoff += offset;           
        }
#endif
#ifdef PRO
        if (mask4s & (1 << DISPLAY_SENSOR_2))
        {
            offset = Disp_Sensors_Item(DISPLAY_SENSOR_2,FALSE, 1,xoff,yoff);
			//offset = Disp_Sensors_Item(DISPLAY_SENSOR_2,FALSE, 3,xoff,yoff);
            yoff += offset;           
        }
#endif      
#ifdef TOC
        if (mask4s & (1 << DISPLAY_SENSOR_TOC))
        {
            //offset = Disp_Sensors_Item(DISPLAY_SENSOR_TOC,FALSE, 3,xoff,yoff); //20160926
			offset = Disp_Sensors_Item(DISPLAY_SENSOR_TOC,FALSE, 8,xoff,yoff); 
            yoff += offset;
        }
#endif       
    }
    else
    {
        int offset;
        if (mask4s & (1 << DISPLAY_SENSOR_1))
        {
			offset = Disp_Sensors_Item(DISPLAY_SENSOR_1,FALSE, 3,xoff,yoff);
            // yoff += offset;
		    yoff += (offset + 30);
        }

#ifdef UF_FUNCTION
        if (mask4s & (1 << DISPLAY_SENSOR_2))
        {
            offset = Disp_Sensors_Item(DISPLAY_SENSOR_2,FALSE, 3,xoff,yoff);
            yoff += offset;           
        }
#endif        
#ifdef PRO
        if (mask4s & (1 << DISPLAY_SENSOR_2))
        {
            //offset = Disp_Sensors_Item(DISPLAY_SENSOR_2,FALSE, 3,xoff,yoff);
            offset = Disp_Sensors_Item(DISPLAY_SENSOR_2,FALSE, 1,xoff,yoff);
            yoff += offset;           
        }
#endif
#ifdef TOC
        if (mask4s & (1 << DISPLAY_SENSOR_TOC))
        {
            //offset = Disp_Sensors_Item(DISPLAY_SENSOR_TOC,FALSE, 3,xoff,yoff);
            offset = Disp_Sensors_Item(DISPLAY_SENSOR_TOC,FALSE, 8,xoff,yoff);
            yoff += offset;
        }   
#endif    
    }
}

void Disp_UpdateIdlePage(int sensor)
{
    Disp_UpdateTime();
    if (sensor)
    {
		Disp_Sensors(SENSOR_COORD_X,SENSOR_COORD_Y,5);
    }
}

void Disp_Move2Idle(void)
{
    Disp_Move2DstPage();
    
    Disp_Move2DstState();
    
    Disp_DisplayIdlePage();
}

void Disp_PrepareMove2Idle(void)
{
    // transer to idle
    Display.tgtPage.ucMain = DISPLAY_PAGE_IDLE;
    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_IDLE;
    
    Display.TgtState.ucMain = DISPLAY_STATE_IDLE;
    Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;
    Display.ulIdleFlushTime  = 0;    
    Display.ulIdleCirTime = 0;    

    key_toc = 0;
	
    Disp_Move2Idle();

	Disp_Set_Cirtime(); //设置循环时间20170911
}

void Disp_DisplayIdleFlushPage(void)
{
    char text[16];
    int  xoff;
    int  yoff;

    LCD_Clear(Display.usBackColor);
    // display text info
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        strcpy(text, "Flush UF");
        break;
    case DISP_LAN_GERMANY:
        {
#if 0            
            uint8_t idx = 0;
            text[idx++] = 'U';
            text[idx++] = 'F';
            text[idx++] = ' ';
            text[idx++] = 'W';
            text[idx++] = 'a';
            text[idx++] = 'c';
            text[idx++] = 'h';
            text[idx++] = 'e';
#endif   
        strcpy(text, "Spuelung UF");
        }
        break;
    default:
        strcpy(text, "UF冲洗");
        break;
    }    
    xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);    
    yoff = LCD_H/2  - 6 - curFont->sizeY;
 
    Disp_WorkState(xoff,yoff,text);
    Disp_ShowTime();  
}

void Disp_DisplayFlushProgressInfo(void)
{
    char *text = (char *)Config_buff;
    int xoff;
    int yoff;
    int sec = Display.cfg.cfg3.usUFFlushTime - Display.ulIdleFlushCnt;
    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf(text,"  %d Sec  ", sec) ;
    }
    else
    {
        sprintf(text,"  %d 秒  ", sec) ;
    }    
    xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);    
    yoff = LCD_H/2  + (6 + curFont->sizeY) ;

    curFont->DrawText(xoff - curFont->sizeX*3,yoff,"   ",Display.usForColor,Display.usBackColor);
    curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
}

void Disp_Move2IdleFlush(void)
{
    Disp_Move2DstPage();
    Disp_Move2DstState();
    // displaying same information  as idle state
    Disp_DisplayIdleFlushPage();
}

void Disp_PrepareMove2IdleFlush(void)
{
    // transer to idle
    Display.tgtPage.ucMain = DISPLAY_PAGE_IDLE;
    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_IDLE_FLUSH;
    Display.TgtState.ucMain = DISPLAY_STATE_IDLE;
    Display.TgtState.ucSub = DISPLAY_SUB_STATE_IDLE_FLUSH;
    Display.ulIdleFlushCnt  = 0;  

    Display.ulIdleFlushTime = 0;
    key_em = 1;
    Disp_Move2IdleFlush();
}

void Disp_CirculationInitPage(void)
{
    char *text;

    //Display.usBackColor = WHITE;
    //Display.usForColor  = BLACK;
    
    LCD_Clear(Display.usBackColor);

    // display text info
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        text = "Recirculation";
        break;
    case DISP_LAN_GERMANY:
        text = "Rezirkulation";
        break;
    default:
        text = "循 环";
        break;
    }  
    Disp_WorkState(20,20,text);
    Disp_Sensors(SENSOR_COORD_X,SENSOR_COORD_Y,5);
}

void Disp_QtwCumulateVolume(void)
{
    if (DISPLAY_STATE_QUANTITY_TAKING_WATER != Display.CurState.ucMain)
    {
        return;
    }
    if (Display.spec.qtw.timeElaps == Display.spec.qtw.time4tw)
    {
        Display.ulCumulatedVolume += (Display.usQtwWq * 100); // convert to ml
    }
    else
    {
        uint32_t time = RTC_Get_Second();
        if (time > Display.ulStartSecond)
        {
            time -= Display.ulStartSecond;
            time = (time*Display.cfg.cfg2.FLOW*10)/60; // to ml/s
            Display.ulCumulatedVolume += time;
        }
    }
}

void Disp_NtwCumulateVolume(void)
{
    if (DISPLAY_STATE_NORMAL_TAKING_WATER != Display.CurState.ucMain)
    {
        return;
    } 
	else
    {
        uint32_t time = RTC_Get_Second();
        if (time > Display.ulStartSecond)
        {
            time -= Display.ulStartSecond;
            time = (time*Display.cfg.cfg2.FLOW*10)/60; // to ml/s           
            Display.ulCumulatedVolume += time;
            // update time milestone
            Display.ulStartSecond = RTC_Get_Second();
        }
    }
}

void Disp_StopQtwTakingWater()
{
    sys_untimeout(&Display.to4qtw);
    // update cumulated volume
    Disp_QtwCumulateVolume();   
}

void Disp_StopNtwTakingWater()
{
    sys_untimeout(&Display.to4qtw);
    Disp_NtwCumulateVolume();   
}

//第一次循环后跳转到TOC过程
void Disp_head_cir_msg_cb(void)
{
     Disp_PrepareMove2TOC();
}

void Disp_head_cir_cb(void *para)
{
     Disp_report(Disp_head_cir_msg_cb);
}

//第一次循环0626
void Disp_Move2HeadCirculation(void)
{
   Disp_Move2DstPage();

   Disp_Move2DstState();

   Disp_CirculationInitPage();

   // add supervisor time
   sys_timeout(Display.ulCalcirtime,SYS_TIMER_ONE_SHOT,Display.ulCalcirtime,Disp_head_cir_cb,&Display,&Display.to4cir);
 
}

void Disp_cir_msg_cb(void)
{
    // move2ilde
    Disp_PrepareMove2Idle();
}

void Disp_cir_cb(void *para)
{
     Disp_report(Disp_cir_msg_cb);
}

void Disp_Move2Circulation(void)
{
   Disp_Move2DstPage();

   Disp_Move2DstState();

   Disp_CirculationInitPage();

   // add supervisor time
  // sys_timeout(Display.ulCalcirtime,SYS_TIMER_ONE_SHOT,Display.ulCalcirtime,Disp_cir_cb,&Display,&Display.to4cir);
   sys_timeout(Display.ulCalcirtime,SYS_TIMER_ONE_SHOT,Display.ulCalcirtime,Disp_cir_cb,&Display,&Display.to4cir);
}

//第一次循环0626
void Disp_PrepareMove2HeadCirculation(void)
{
    Display.tgtPage.ucMain  = DISPLAY_PAGE_CIRCULATION;
    Display.tgtPage.ucSub   = DISPLAY_SUB_PAGE_IDLE;
    Display.TgtState.ucMain = DISPLAY_STATE_CIRCULATION;
    Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;

    Display.ulIdleCirTime = 0;   
    
    Disp_Move2HeadCirculation();
}

void Disp_PrepareMove2Circulation(void)
{
    Display.tgtPage.ucMain  = DISPLAY_PAGE_CIRCULATION;
    Display.tgtPage.ucSub   = DISPLAY_SUB_PAGE_IDLE;
    Display.TgtState.ucMain = DISPLAY_STATE_CIRCULATION;
    Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;

    Display.ulIdleCirTime = 0;   
    
    Disp_Move2Circulation();
}

void Disp_Prepare4Circulation(void)
{
#ifdef TOC
    // Disp_PrepareMove2TOC();
	if(1 == Display.cfg.cfg2.TOC_FUN)
	{
		//先循环后自动跳转到TOC过程0626
		Disp_PrepareMove2HeadCirculation();
	}
    else
    {
    	//检测不到TOC则不再进入toc过程
    	Disp_PrepareMove2Circulation();
    }   
#else
    Disp_PrepareMove2Circulation();
#endif
}

void Disp_QtwProductVolumn(void)
{

    int xoff = 0;
    int yoff = 0;
    int xend;
    char buf[16];

    uint8_t  *szUnit[DISP_LAN_NUM] = {{" L"},{" 升"},{" L"}}; 
    
    Disp_Int2float(Display.usQtwWq,buf);

#ifdef CODEGB_48    
    //Disp_SelectFont(&font48);  
#endif
    
    xoff = LCD_W/2 - curFont->sizeX/2*(strlen(buf)+1);
    
    yoff = SENSOR_COORD_Y - 6 - curFont->sizeY;

    xend = xoff + curFont->sizeX *(strlen(buf));

    curFont->DrawText(xoff,yoff,(u8 *)buf,Display.usForColor,Display.usBackColor);

    yoff += curFont->sizeY;   

#ifdef CODEGB_48    
     //Disp_SelectFont(oldfont);  
#endif

    yoff -= curFont->sizeY;   

    curFont->DrawText(xend+10,yoff,szUnit[Display.cfg.cfg1.language],Display.usForColor,Display.usBackColor);	
}

void Disp_CheckTankState(void)
{
    Disp_GetTankState();
    if (Display.aAlarm[DISP_ALARM_TANK_EMPTY].bit1Fired)
    {
        uint8_t uc2cir = FALSE;
        switch(Display.CurState.ucMain)
        {
        default: 
            break;
        case DISPLAY_STATE_NORMAL_TAKING_WATER:
            Disp_StopNtwTakingWater();
            uc2cir = TRUE;
            break;
        case DISPLAY_STATE_QUANTITY_TAKING_WATER:
            Disp_StopQtwTakingWater();
            uc2cir = TRUE;
            break;
        }
        if (uc2cir)
        {
            Disp_Prepare4Circulation();
        }
    }
}

void Disp_UpdateUFWashTime(void)
{
    Display.bit1PendingUFClean = FALSE;
    Display.bit1InitPending4UF = FALSE;

    Display.ulUFCleanTime = RTC_Get_Second();
#ifdef STORAGE_SD
    Storage_WriteRunTimeInfo(&Display);
#else
    Storage_WriteRunTimeInfo2Flash(&Display);
#endif
}

void Disp_DisplayUFCleanProgressInfo(void)
{
    char *text = (char *)Config_buff;
    int xoff;
    int yoff;
    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf(text,"  %d Min %d Sec  ", Display.spec.uf.usCountDownTimer/60,Display.spec.uf.usCountDownTimer%60) ;
    }
    else
    {
        sprintf(text,"  %d 分 %d 秒   ", Display.spec.uf.usCountDownTimer/60,Display.spec.uf.usCountDownTimer%60) ;
    }
    
    xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);    
    yoff = LCD_H/2  + (6 + curFont->sizeY) ;

    curFont->DrawText(xoff - curFont->sizeX*3,yoff,"   ",Display.usForColor,Display.usBackColor);
    curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
}


void Disp_uf_clean_msg_cb(void)
{
    if (DISPLAY_STATE_UF_CLEAN != Display.CurState.ucMain)
    {
        sys_untimeout(&Display.to4UfClean);
        return;
    }

    if (Display.spec.uf.usCountDownTimer > 0)
    {
        Display.spec.uf.usCountDownTimer--;
    }

    if (Display.spec.uf.usSubCountDownTimer)
    {
        Display.spec.uf.usSubCountDownTimer--;
    }

    //if (!(Display.spec.uf.usCountDownTimer % 10))
    {
        Disp_DisplayUFCleanProgressInfo();
    }
    
    {
        switch(Display.spec.uf.ucStage)
        {
        case DISPLAY_UF_SUB_STATE_1: // 
            if (0 == Display.spec.uf.usSubCountDownTimer)
            {
                RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
                RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,TRUE);
                RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
                RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
                RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
                RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);

                Display.spec.uf.ucStage             = DISPLAY_UF_SUB_STATE_2;
                Display.spec.uf.ucSubStage          = 0;
                Display.spec.uf.usSubCountDownTimer = DISP_UF_SUB_COUNT_DOWN_5MIN;
                
#ifdef DISP_UF_TEST
                UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_2:0");
#endif
            }
            break;
        case DISPLAY_UF_SUB_STATE_2:
            {
                if (0 == Display.spec.uf.usSubCountDownTimer)
                {
                    switch(Display.spec.uf.ucSubStage)
                    {
                    default:
                    case 0:
                        RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
                        RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
                        RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,TRUE);
                        RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
                        RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
                        RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
                        Display.spec.uf.ucSubStage  = 1;
                        Display.spec.uf.usSubCountDownTimer = DISP_UF_SUB_COUNT_DOWN_2SEC;
#ifdef DISP_UF_TEST
                        UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_2:1");
#endif
                        break;
                    case 1:
                        RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
                        RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
                        RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
                        RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
                        RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,FALSE);
                        RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
                        Display.spec.uf.ucSubStage  = 2;
                        Display.spec.uf.usSubCountDownTimer = DISP_UF_SUB_COUNT_DOWN_15MIN;
#ifdef DISP_UF_TEST
                        UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_2:2");
#endif
                        break;
                    case 2:
                        Display.spec.uf.ucCnt4Cir++;
                        if (Display.spec.uf.ucCnt4Cir < 5)
                        {
                            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
                            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,TRUE);
                            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
                            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
                            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
                            RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
                            Display.spec.uf.usSubCountDownTimer = DISP_UF_SUB_COUNT_DOWN_5MIN;
                            Display.spec.uf.ucSubStage          = 0; // ylf: recursive
                            UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_2:0");
                        }
                        else
                        {
                            RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
                            RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
                            RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
                            RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
                            RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
                            RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);
                        
                            Display.spec.uf.ucStage             = DISPLAY_UF_SUB_STATE_3;
                            Display.spec.uf.ucSubStage          = 0;
                            Display.spec.uf.usSubCountDownTimer = DISP_UF_SUB_COUNT_DOWN_5MIN;
#ifdef DISP_UF_TEST
                            UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_3:0");
#endif
                            
                        }
                        break;
                    }
                    
                }
            }
            break;
        case DISPLAY_UF_SUB_STATE_3:
            {
                if (0 == Display.spec.uf.usSubCountDownTimer)
                {
                    switch(Display.spec.uf.ucSubStage )
                    {
                    case 0:
                        RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
                        RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,TRUE);
                        RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
                        RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
                        RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
                        RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);
                        Display.spec.uf.ucSubStage          = 1;
                        Display.spec.uf.usSubCountDownTimer = DISP_UF_SUB_COUNT_DOWN_4MIN;
#ifdef DISP_UF_TEST
                        UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_3:1");
#endif
                        break;
                    case 1:
                        RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,TRUE);
                        RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,TRUE);
                        RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,TRUE);
                        RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
                        RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
                        RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);
                        
                        Display.spec.uf.ucSubStage          = 2;
                        Display.spec.uf.usSubCountDownTimer = DISP_UF_SUB_COUNT_DOWN_1MIN;
#ifdef DISP_UF_TEST
                        UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_3:2");
#endif
                        break;
                    case 2:
                        RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
                        RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
                        RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
                        RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
                        RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,TRUE);
                        RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,TRUE);
                        
                        Display.spec.uf.ucSubStage          = 3;
                        Display.spec.uf.usSubCountDownTimer = DISP_UF_SUB_COUNT_DOWN_30SEC;
#ifdef DISP_UF_TEST
                        UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_3:3");
#endif
                        break;
                    case 3:
                        RelayLogicCtrl(RELAY_VALVE_WATER_INFLOWING_E7,FALSE);
                        RelayLogicCtrl(RELAY_VALVE_CIRCULATE_E5,FALSE);
                        RelayLogicCtrl(RELAY_VALVE_WATER_INTAKING_E6,FALSE);
                        RelayLogicCtrl(RELAY_ULTRA_VIOLET_LAMP,FALSE);
                        RelayLogicCtrl(RELAY_PUMP_CIRCULATE_C3,FALSE);
                        RelayLogicCtrl(RELAY_VALVE_FLUSH_E8,FALSE);
                        
                        Display.spec.uf.ucSubStage          = 4;
                        Display.spec.uf.usSubCountDownTimer = 0xFFFF;

#ifdef DISP_UF_TEST
                        UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_3:4");
#endif
                        break;
                    }
                }
            }
            break;
        }
    }

    if (0 == Display.spec.uf.usCountDownTimer)
    {
        sys_untimeout(&Display.to4UfClean);
        
        Disp_UpdateUFWashTime(); 

        Disp_UserSetPrepareUFClean(DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP2);
    }
}


void Disp_uf_clean_cb(void *para)
{
     Disp_report(Disp_uf_clean_msg_cb);
}

void Disp_DisplayUFCleanInitPage(void)
{
    char *text;
    int xoff;
    int yoff;
    
    LCD_Clear(Display.usBackColor);
#ifdef CODEGB_48    
    //Disp_SelectFont(&font48);  
#endif
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        text = "Flush";
        break;
    case DISP_LAN_GERMANY:
        text = "Spuelung";
        break;
    default:
        text = "UF 清洗";
        break;
    }        

    xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);    
    yoff = LCD_H/2  - 6 - curFont->sizeY;

    curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);

#ifdef CODEGB_48    
     //Disp_SelectFont(oldfont);  
#endif
}

void Disp_uf_pre_clean_msg_cb(void)
{
    if (DISPLAY_STATE_UF_CLEAN != Display.CurState.ucMain)
    {
        sys_untimeout(&Display.to4UfPrePare);
        return;
    }
	else
    {
        sys_untimeout(&Display.to4UfPrePare);      
        Disp_UserSetPrepareUFClean(DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP1);
    }
}

void Disp_uf_pre_clean_cb(void *para)
{
     Disp_report(Disp_uf_pre_clean_msg_cb);
}

void Disp_DisplayPreUFCleanInitPage(void)
{
    char *text;
    int xoff;
    int yoff;
    
    LCD_Clear(Display.usBackColor);

#ifdef CODEGB_48    
    //Disp_SelectFont(&font48);  
#endif    
    // display info
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        text = "UF Prepare...";
        break;
    case DISP_LAN_GERMANY:
        text = "UF Bereiten...";
        break;
    default:
        text = "UF 准备中";
        break;
    }    

    xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1); 
    yoff = LCD_H/2  - 6 - curFont->sizeY;

    curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);

#ifdef CODEGB_48    
     //Disp_SelectFont(oldfont);  
#endif
}



void Disp_Move2UFPreClean(void)
{
   Disp_Move2DstPage();

   Disp_Move2DstState();

   Disp_DisplayPreUFCleanInitPage();

   // add supervisor time
   sys_timeout(30000,SYS_TIMER_ONE_SHOT,30000,Disp_uf_pre_clean_cb,&Display,&Display.to4UfPrePare);
   
}

void Disp_PrepareMove2UFPreClean(void)
{
    Display.tgtPage.ucMain  = DISPLAY_PAGE_UF_CLEAN;
    Display.tgtPage.ucSub   = DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_PREPARE;
    
    Display.TgtState.ucMain = DISPLAY_STATE_UF_CLEAN;
    Display.TgtState.ucSub  = DISPALY_SUB_STATE_UF_PRECLEAN;

    Display.spec.uf.ucStage    = 0;
    Display.spec.uf.ucSubStage = 0; 
    Display.spec.uf.ucCnt4Cir  = 0;
    Display.spec.uf.usCountDownTimer    = 0;
    Display.spec.uf.usSubCountDownTimer = 0;
    
    Disp_Move2UFPreClean();
}

void Disp_Move2UFClean(void)
{
   Disp_Move2DstPage();

   Disp_Move2DstState();

   Disp_DisplayUFCleanInitPage();

   Disp_DisplayUFCleanProgressInfo();

   // add supervisor time
   sys_timeout(1000,SYS_TIMER_PERIOD,1000,Disp_uf_clean_cb,&Display,&Display.to4UfClean);
   
}

void Disp_PrepareMove2UFClean(void)
{
    Display.tgtPage.ucMain  = DISPLAY_PAGE_UF_CLEAN;
    Display.tgtPage.ucSub   = DISPLAY_SUB_PAGE_IDLE;

    // for valve control
    Display.TgtState.ucMain = DISPLAY_STATE_UF_CLEAN;
    Display.TgtState.ucSub  = DISPALY_SUB_STATE_UF_CLEAN;

    Display.spec.uf.ucStage    = DISPLAY_UF_SUB_STATE_1;
    Display.spec.uf.ucSubStage = 0; 
    Display.spec.uf.ucCnt4Cir  = 0;
    Display.spec.uf.usCountDownTimer    = DISP_UF_COUNT_DOWN_SENARIO1;
    Display.spec.uf.usSubCountDownTimer = DISP_UF_SUB_COUNT_DOWN_3MIN;

#ifdef DISP_UF_TEST
    UartCmdPrintf(VOS_LOG_DEBUG,"DISPLAY_UF_SUB_STATE_1");
#endif  
    Disp_Move2UFClean();
}

void Disp_CheckUFClean(void)
{
    uint32_t ulCurTime = RTC_Get_Second();
    uint32_t ulTemp;

    // check pack
    if (ulCurTime < Display.ulUFCleanTime)
    {
        return;
    }

    if (Display.bit1PendingUFClean)
    {
        return;
    }
    
    ulTemp = (ulCurTime - Display.ulUFCleanTime)/DISP_DAYININSECOND;
    if (ulTemp >= Display.cfg.cfg3.usUFCleanTime)
    {
        Display.bit1PendingUFClean = TRUE;
    }
}

void Disp_TOCFlushPage(void)
{
    char *text;
    LCD_Clear(Display.usBackColor);
    // display text info
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        //text = "TOC Flush";
        text = "Recirculation";
        break;
    case DISP_LAN_GERMANY:
        //text = "TOC Spuelung";
         text = "Rezirkulation";
        break;
    default:
       // text = "TOC 冲洗";
        text = "循 环";
        break;
    }  
    Disp_WorkState(10,10,text);
	Disp_Sensors(SENSOR_COORD_X,SENSOR_COORD_Y,5);
}

void Disp_TOCOxidatePage(void)
{
    char *text;
    //Display.usBackColor = WHITE;
    //Display.usForColor  = BLACK;
    
    LCD_Clear(Display.usBackColor);

    // display text info
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        //text = "TOC Oxidate";
        text = "Recirculation";
        break;
    case DISP_LAN_GERMANY:
       // text = "TOC Oxidate";
       text = "Rezirkulation";
        break;
    default:
        //text = "TOC 氧化";
        text = "循 环";
        break;
    }
    
    Disp_WorkState(10,10,text);
	Disp_Sensors(SENSOR_COORD_X,SENSOR_COORD_Y,5);
}

void Disp_toc_msg_cb(void)
{
    // move2ilde
    Disp_Move2TOCFlush();
}

void Disp_toc_cb(void *para)
{
     Disp_report(Disp_toc_msg_cb);
}

void Disp_toc_fast_meas_msg_cb(void)
{
    // get sensor value
    if (DISPLAY_STATE_TOC != Display.CurState.ucMain)
    {
        Disp_Select_Sensor(0);            
        sys_untimeout(&Display.to4TOCMeas);       
        return;
    }
	else
    {
        uint8_t ucPoolIdx = Disp_AllocAdcAction();           
        if (ucPoolIdx < MAX_ADC_ACTIONS)
        {
            aAdcChl[ucPoolIdx].iValue = TOC_MEAS_PERIOD;
            aAdcChl[ucPoolIdx].ucChl = DISPLAY_SENSOR_TOC;
            aAdcChl[ucPoolIdx].ucPoolIdx = ucPoolIdx;
            Disp_AddAdcChl(&aAdcChl[ucPoolIdx]);
        }
    }
}


void Disp_toc_fast_meas_cb(void *para)
{
     Disp_report(Disp_toc_fast_meas_msg_cb);
}


void Disp_Move2TOCFlush(void)
{
	if(key_toc)
	{
		if (Display.spec.toc.ucIdx < Display.spec.toc.ucActionCnt )
		{
			uint8_t ucIdx = Display.spec.toc.ucIdx;
			Display.spec.toc.ucIdx++;
			switch(Display.spec.toc.aActionBuf[ucIdx])
			{
			case DISPALY_SUB_PAGE_TOC_FLUSH:
				Display.tgtPage.ucMain  = DISPLAY_PAGE_TOC;
				Display.tgtPage.ucSub   = DISPALY_SUB_PAGE_TOC_FLUSH;
				Display.TgtState.ucMain = DISPLAY_STATE_TOC;
				Display.TgtState.ucSub  = DISPLAY_SUB_STATE_TOC_FLUSH;
				Disp_Move2DstPage();
				Disp_Move2DstState();
				Disp_TOCFlushPage();
				// add supervisor time
				if(Display.spec.toc.ucIdx == 3)
				{
				//第二次TOC冲洗20s
					sys_timeout(20*1000,SYS_TIMER_ONE_SHOT,0,Disp_toc_cb,&Display,&Display.to4TOC);
				}
				else
				{
					sys_timeout(Display.cfg.cfg3.usTocWashTime*1000,SYS_TIMER_ONE_SHOT,0,Disp_toc_cb,&Display,&Display.to4TOC);
				}
				break;
			default: // DISPALY_SUB_PAGE_TOC_OXIDATION
				Display.tgtPage.ucMain  = DISPLAY_PAGE_TOC;
				Display.tgtPage.ucSub   = DISPALY_SUB_PAGE_TOC_OXIDATION;
				Display.TgtState.ucMain = DISPLAY_STATE_TOC;
				Display.TgtState.ucSub  = DISPLAY_SUB_STATE_TOC_OXIDATION;
				Disp_Move2DstPage();   
				Disp_Move2DstState(); 
				Disp_TOCOxidatePage();
				sys_timeout(Display.cfg.cfg3.usTocOxiTime*1000,SYS_TIMER_ONE_SHOT,0,Disp_toc_cb,&Display,&Display.to4TOC);
				break;
			}
		}
		else
		{
			sys_untimeout(&Display.to4TOC);
			sys_untimeout(&Display.to4TOCMeas);
			Disp_Select_Sensor(0);
			//Disp_PrepareMove2Idle();
			Disp_PrepareMove2Circulation();       
		}
	}
}

void Disp_PrepareMove2TOC(void)
{
	Display.spec.toc.ulMaxRes4Wash = 0;
    Display.spec.toc.ulMinRes4Oxi  = 0xffffffff;
	key_toc = 1;

    Display.spec.toc.ucIdx       = 0;
    Display.spec.toc.ucActionCnt = 0;
    Display.spec.toc.aActionBuf[Display.spec.toc.ucActionCnt] = DISPALY_SUB_PAGE_TOC_FLUSH;
    Display.spec.toc.ucActionCnt++;

    Display.spec.toc.aActionBuf[Display.spec.toc.ucActionCnt] = DISPALY_SUB_PAGE_TOC_OXIDATION;
    Display.spec.toc.ucActionCnt++;
    
    Display.spec.toc.aActionBuf[Display.spec.toc.ucActionCnt] = DISPALY_SUB_PAGE_TOC_FLUSH;
    Display.spec.toc.ucActionCnt++;
    
    Disp_Move2TOCFlush();

    // start fast measurement timer
    //sys_timeout(100,SYS_TIMER_PERIOD,TOC_MEAS_PERIOD,Disp_toc_fast_meas_cb,&Display,&Display.to4TOCMeas);
    //20161028 100->>TOC_DELAY_SAMPLE
    sys_timeout(TOC_DELAY_SAMPLE,SYS_TIMER_PERIOD,TOC_MEAS_PERIOD,Disp_toc_fast_meas_cb,&Display,&Display.to4TOCMeas);    
}

void Disp_QtwSettingInitPage(void)
{
    
    //Display.usBackColor = WHITE;
    //Display.usForColor = BLACK;
    LCD_Clear(Display.usBackColor);
    Disp_QtwProductVolumn();	
}

void Disp_QtwSettingUpdatePage(void)
{
     Disp_QtwProductVolumn();	 
}

void Disp_Move2QtwSetting(void)
{
	Disp_Move2DstPage();
    Disp_QtwSettingInitPage();
	Disp_Set_Cirtime(); //设置循环时间20170911
}

void Disp_PrepareMove2QtwSetting()
{
    Display.tgtPage.ucMain = DISPLAY_PAGE_QUANTITY_TAKING_WATER;
    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_SETTING;

    key_toc = 0;
	
    Disp_Move2QtwSetting();
}

void Disp_TankEmptyInitPage(void)
{
    uint16_t usTmp = Display.usBackColor;

    Display.usBackColor = RED;
    LCD_Clear(Display.usBackColor);
    Disp_ShowEmptyTank(0xff,0xff);
    Display.usBackColor = usTmp;
}

void Disp_Move2TankEmpty(void)
{
   Disp_Move2DstPage();

   Disp_Move2DstState();

   Disp_TankEmptyInitPage();
}

void Disp_QtwTakingWaterInitPage(void)
{   
    int xoff = 20;
    int yoff = 20;
    char *text;

    //Display.usForColor = BLACK;
    //Display.usBackColor = WHITE;
    LCD_Clear(Display.usBackColor);

    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language)
    {
        text = "Volumetric Dispensing";
    }
    else if (DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        text = "Volumen Ausgabe";
    }
    else
    {
        text = "定量取水";
    
    }
    Disp_WorkState(xoff,yoff,text);

    Disp_QtwProductVolumn();

	Disp_Sensors(SENSOR_COORD_X,SENSOR_COORD_Y,5);
}

typedef void (*disp_msg_cb)(void);

void Disp_msg_Handler(Message *Msg)
{
    DISP_MSG *dmsg = (DISP_MSG *)Msg;

    if (dmsg->para)
    {
        ((disp_msg_cb)dmsg->para)();
    }

}


void Disp_qtw_msg_cb(void)
{
	sys_untimeout(&Display.to4qtw);
    // calc 
	if (DISPLAY_STATE_QUANTITY_TAKING_WATER == Display.CurState.ucMain)
	{
		Display.spec.qtw.timeElaps = Display.spec.qtw.time4tw;
		Disp_QtwCumulateVolume();
		// late do someting
		Disp_Prepare4Circulation();
	}
	else
	{
		UartCmdPrintf(VOS_LOG_DEBUG,"incorrect state: %d",Display.CurState);
	}
}

void Disp_qtw_cb(void *para)
{
	Disp_report(Disp_qtw_msg_cb);
}

void Disp_Move2QtwTakingWater(void)
{
	Disp_Move2DstPage();
   // move to qtw taking state
	Disp_Move2DstState();
	Disp_QtwTakingWaterInitPage();
   // calculating taking water time
	Display.spec.qtw.time4tw = (Display.usQtwWq *10 * DISP_MINUTE2MS)/Display.cfg.cfg2.FLOW;
	Display.spec.qtw.timeElaps = 0;
	sys_timeout(Display.spec.qtw.time4tw,SYS_TIMER_ONE_SHOT,Display.spec.qtw.time4tw,Disp_qtw_cb,&Display,&Display.to4qtw);  
}

void Disp_NtwInitPage(void)
{
	char *text;
    LCD_Clear(Display.usBackColor);

    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language)
    {
        text = "Product";
    }
    else if (DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        text = "Ausgabe";
    }
    else
    {
        text = "产 水";
    }
    Disp_WorkState(20,20,text);
    Disp_Sensors(SENSOR_COORD_X,SENSOR_COORD_Y,5);
}

void Disp_ntw_msg_cb(void)
{
   // maybe do someting here!
   if (DISPLAY_STATE_NORMAL_TAKING_WATER == Display.CurState.ucMain)
   {
       Disp_NtwCumulateVolume();
   }
   else
   {
       sys_untimeout(&Display.to4qtw);
       UartCmdPrintf(VOS_LOG_DEBUG,"incorrect state: %d",Display.CurState);
   }
}

void Disp_ntw_cb(void *para)
{
     Disp_report(Disp_ntw_msg_cb);
}

void Disp_Move2Ntw(void)
{
	Disp_Move2DstPage();
	Disp_Move2DstState();
	Disp_NtwInitPage();
	sys_timeout(DISP_MINUTE2MS,SYS_TIMER_PERIOD,DISP_MINUTE2MS,Disp_ntw_cb,&Display,&Display.to4qtw);
}

void Disp_Prepare4Ntw(void)
{
    if (!Disp_GetTankState())
    {
        Display.tgtPage.ucMain = DISPLAY_PAGE_IDLE;
        Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_TANK_EMPTY;
        Display.TgtState.ucMain = DISPLAY_STATE_IDLE;
        Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;

        key_toc = 0;
        key_em = 1;
        Disp_Move2TankEmpty();
    }
    else
    {
        Display.tgtPage.ucMain = DISPLAY_PAGE_NORMAL_TAKING_WATER;
        Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_IDLE;
        Display.TgtState.ucMain = DISPLAY_STATE_NORMAL_TAKING_WATER;
        Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;

        key_toc = 0;
        
        Disp_Move2Ntw();
    }   
    Disp_Set_Cirtime(); //设置循环时间20170911
}

void Disp_DecInitPage(void)
{

    char *text;    
    LCD_Clear(Display.usBackColor);

    // display text info
    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language)
    {
        text = "Pressure Relief";
    }
    else if (DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        text = "Druck ablassen";
    }
    else
    {
        text = "泄 压";
    }
    Disp_WorkState(0xff,0xff,text);   
}

void Disp_Move2Decompression(void)
{
   Disp_Move2DstPage();

   // move to target state
   Disp_Move2DstState();

   Disp_DecInitPage();

}

void Disp_DisplayDegassingProgressInfo(void)
{
    char *text = (char *)Config_buff;

    int xoff;
    int yoff;

    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf(text,"  %d Sec  ", Display.spec.dg.usTimes) ;
    }
    else
    {
        sprintf(text,"  %d 秒  ", Display.spec.dg.usTimes) ;
    }
    

    xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);    
    yoff = LCD_H/2  + (6 + curFont->sizeY) ;

    curFont->DrawText(xoff - curFont->sizeX*3,yoff,"   ",Display.usForColor,Display.usBackColor);
    curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
}

void Disp_DegassInitPage(void)
{

    char *text;
    int xoff,yoff;
    
    LCD_Clear(Display.usBackColor);

    switch(Display.curPage.ucSub)
    {
    case DISPALY_SUB_PAGE_DEGASS1:
        // display text info
        if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
            || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
        {   
            text = "Degassing Stage 1";
        }
        else
        {
            text = "排气阶段 1";
        }
        break;
    default:
        // display text info
        if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
            || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
        {    
            text = "Degassing Stage 2";
        }
        else
        {
            text = "排气阶段 2";
        }     
        break;
    }

    xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);  
    yoff = LCD_H/2  - (curFont->sizeY/2) ;
    
    curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);  
}

void Disp_degass_msg_cb(void)
{
    if (DISPLAY_STATE_DEGASS != Display.CurState.ucMain)
    {
        sys_untimeout(&Display.to4Degass);
        return;
    }

    if (Display.spec.dg.usTimes)
    {
        Display.spec.dg.usTimes--;
    }

    //if (Display.spec.dg.usTimes % 10 == 0)
    {
        Disp_DisplayDegassingProgressInfo();
    }

    if (0 == Display.spec.dg.usTimes)
    {
        switch(Display.CurState.ucSub)
        {
        default:
        case DISPLAY_SUB_STATE_DEGASS1:
            {
                Disp_Prepare4Degass2();
            }
            break;
        case DISPLAY_SUB_STATE_DEGASS2:
            {
                sys_untimeout(&Display.to4Degass);         
                Disp_PrepareMove2Idle();
            }
            break;
        }
    }   
}

void Disp_degass_cb(void *para)
{
	Disp_report(Disp_degass_msg_cb);
}

void Disp_Move2Degass(void)
{
	Disp_Move2DstPage();
	// move to qtw taking state
	Disp_Move2DstState();
	Disp_DegassInitPage();
	sys_timeout(1000,SYS_TIMER_PERIOD,1000,Disp_degass_cb,&Display,&Display.to4Degass); 
}

void Disp_Prepare4Degass1(void)
{
    Display.tgtPage.ucMain = DISPLAY_PAGE_DEGASS;
    Display.tgtPage.ucSub  = DISPALY_SUB_PAGE_DEGASS1;
    Display.TgtState.ucMain = DISPLAY_STATE_DEGASS;
    Display.TgtState.ucSub  = DISPLAY_SUB_STATE_DEGASS1;

    Display.spec.dg.usTimes = 180;

    Disp_Move2Degass();

    Disp_DisplayDegassingProgressInfo();
}

void Disp_Prepare4Degass2(void)
{
    Display.tgtPage.ucMain = DISPLAY_PAGE_DEGASS;
    Display.tgtPage.ucSub  = DISPALY_SUB_PAGE_DEGASS2;
    Display.TgtState.ucMain = DISPLAY_STATE_DEGASS;
    Display.TgtState.ucSub  = DISPLAY_SUB_STATE_DEGASS2;

    Display.spec.dg.usTimes = 10;

    Disp_Move2Degass();
    
    Disp_DisplayDegassingProgressInfo();
}

void Disp_userSetDrawPack(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    int tmp;
    // total eight lines
    tmp = Display.cfg.cfg2.PACKLIFEDAY - (RTC_Get_Second() - Display.info.PACKLIFEDAY)/DISP_DAYININSECOND;
    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"PACK:%d DAYS %dL    ",tmp,Display.info.PACKLIFEL);
    }
    else
    {
        sprintf((char *)Config_buff,"纯化柱:%d 天 %d升  ",tmp,Display.info.PACKLIFEL);
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_userSetDrawFilter(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    int tmp;
    tmp = Display.cfg.cfg2.FILTERLIFE - (RTC_Get_Second() - Display.info.FILTERLIFE)/DISP_DAYININSECOND;
    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"FILTER:%d DAYS   ",tmp);
    }
    else
    {
        sprintf((char *)Config_buff,"过滤器:%d 天   ",tmp);
    }

    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_userSetDrawUV(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    int tmp;
    tmp = Display.cfg.cfg2.UVLIFEDAY - (RTC_Get_Second() - Display.info.UVLIFEDAY)/DISP_DAYININSECOND;;
    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"UV:%d DAYS %dHr   ",tmp,Display.info.UVLIFEHOUR/(3600/UV_PFM_PEROID));
    }
    else
    {
        sprintf((char *)Config_buff,"紫外灯:%d 天 %d 小时  ",tmp,Display.info.UVLIFEHOUR/(3600/UV_PFM_PEROID));
    }

    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_userSetDrawFlow(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    char buf[16];
    Disp_Int2floatFormat(Display.cfg.cfg2.FLOW,1,2,buf);
    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language )
    {
        sprintf((char *)Config_buff,"FLOW:%s L/min  ",buf);
    }
    else
    {
        sprintf((char *)Config_buff,"流速:%s 升/分钟  ",buf);
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_userSetDrawUnit(int xoff,int yoff,u16 charColor,u16 bkColor)
{

    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"UNIT:%s    ",Display.cfg.cfg2.UNIT ? UNIT_TYPE_1 : UNIT_TYPE_0);
    }
    else
    {
        sprintf((char *)Config_buff,"单位:%s    ",Display.cfg.cfg2.UNIT ? UNIT_TYPE_1 : UNIT_TYPE_0);
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_userSetDrawFeed_Key(int xoff,int yoff,u16 charColor,u16 bkColor)
{

    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"FEED SENSOR:%s    ",Display.cfg.cfg2.FEED_KEY ? FEED_ENG_ON : FEED_ENG_OFF);
    }
    else
    {
        sprintf((char *)Config_buff,"进水电极:%s    ",Display.cfg.cfg2.FEED_KEY ? FEED_CH_ON : FEED_CH_OFF);
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_DrawSelChar(int xoff,int yoff,u8 *string,u16 pos ,u16 charColor,u16 bkColor)
{
    uint8_t tmp;   
    tmp = string[pos]; 
    string[pos] = 0; 
    curFont->DrawText(xoff,yoff,string,charColor,bkColor);
    xoff += strlen((char *)string)*curFont->sizeX;
    curFont->DrawChar(xoff,yoff,tmp,bkColor,charColor);
    xoff += curFont->sizeX;    
    curFont->DrawText(xoff,yoff,&string[pos+1],charColor,bkColor);
    Disp_CleanTail(xoff,yoff,(char *)&string[pos+1],Display.usBackColor4Set);
}

void Disp_DrawSelString(int xoff,int yoff,u8 *string,u16 pos,u16 len ,u16 charColor,u16 bkColor)
{
    uint8_t tmp;   
    tmp = string[pos];   
    string[pos] = 0;   
    curFont->DrawText(xoff,yoff,string,charColor,bkColor);    
    xoff += strlen((char *)string)*curFont->sizeX;
    string[pos] = tmp;
    tmp = string[pos+len]; 
    string[pos+len] = 0;
    curFont->DrawText(xoff,yoff,&string[pos],bkColor,charColor); // reverse disp   
    xoff += strlen((char *)&string[pos])*curFont->sizeX;
    string[pos+len] = tmp;  
    curFont->DrawText(xoff,yoff,&string[pos+len],charColor,bkColor);
}


void Disp_userSetDrawTime(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    uint8_t pos[TIME_SET_NUMS] = TIME_SET_POS;
    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
       || DISP_LAN_GERMANY == Display.cfg.cfg1.language )
    {
        strcpy((char *)Config_buff,"TIME:");
    }
    else
    {
        strcpy((char *)Config_buff,"时间:");
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);

    xoff += strlen((char *)Config_buff)*curFont->sizeX;

    sprintf((char *)Config_buff,TIME_FORMAT,stimer4Us.w_year,stimer4Us.w_month,stimer4Us.w_date,stimer4Us.hour,stimer4Us.min,stimer4Us.sec);

    if (Display.ucCurCol < TIME_SET_NUMS)
    {
        Disp_DrawSelChar(xoff,yoff,Config_buff,pos[Display.ucCurCol],charColor,bkColor);
    }
    else
    {
        curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
    }
}

void Disp_userSetDrawLanguage(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        strcpy((char *)Config_buff,"LANGUAGE:ENGLISH");
        break;
    case DISP_LAN_GERMANY:
        strcpy((char *)Config_buff,"LANGUAGE:DEUTSCH");
        break;
    default:
        strcpy((char *)Config_buff,"语言:中文");
        break;
    }   
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}


void Disp_userSetDrawUFWash(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        strcpy((char *)Config_buff,"CLEAN UF/DISINFECTION");
        break;
    case DISP_LAN_GERMANY:
        strcpy((char *)Config_buff,"REINIGUNG UF/DESINFEKTION");
        break;
    default:
        strcpy((char *)Config_buff,"清洗 UF");
        break;
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_userSetDrawFlush(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        strcpy((char *)Config_buff,"FLUSH UF");
        break;
    case DISP_LAN_GERMANY:
        strcpy((char *)Config_buff,"SPUELUNG UF");
        break;
    default:
        strcpy((char *)Config_buff,"冲洗 UF");
        break;
    }  
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_userSetDrawFeedQuality(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    char buf[16];
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        strcpy((char *)Config_buff,"FEED QUALITY:");
        break;
    case DISP_LAN_GERMANY:
        strcpy((char *)Config_buff,"EINGANGSWERT:");
        break;
    default:
        strcpy((char *)Config_buff,"进水水质:");
        break;
    }
    curFont->DrawText(xoff,yoff,&Config_buff[Display.ucDispCol],charColor,bkColor);
    xoff += curFont->sizeX*strlen((char *)&Config_buff[Display.ucDispCol]);
    if (0 == Display.cfg.cfg2.UNIT)
    {
        Disp_Int2floatFormat((Display.iConductivity[1] + 50)/100,2,1,buf);
        sprintf((char *)Config_buff,"%s %s ",buf,UNIT_TYPE_1);    
    }
    else
    {
        uint32_t fus = (1000*1000)/Display.iConductivity[1];   
        Disp_Int2floatFormat(fus,1,3,buf);        
        sprintf((char *)Config_buff,"%s %s ",buf,UNIT_TYPE_1);        
    }

    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);  
    xoff += curFont->sizeX*strlen((char *)Config_buff);
    Disp_Int2floatFormat((Display.iTemperature[1] + 50)/100,1,1,buf);
    
    sprintf((char *)Config_buff,"%s ℃  ",buf);
    
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);

    // recordinfo
    Display.usX4FeedQuality             = xoff;
    Display.usY4FeedQuality             = yoff;
    Display.usCharColor4FeedQuality     = charColor;
    Display.usBackColor4FeedQuality     = bkColor;    
}

void Disp_userSetDrawSerialNo(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        sprintf((char *)Config_buff,"SERIAL NO:%s",Display.cfg.cfg3.SERIALNO);
        break;
    case DISP_LAN_GERMANY:
        sprintf((char *)Config_buff,"SERIENNUMMER:%s",Display.cfg.cfg3.SERIALNO);
        break;
    default:
        sprintf((char *)Config_buff,"序列号:%s",Display.cfg.cfg3.SERIALNO);
        break;
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

#define HIGH_LIGHTING(Idx,line)((Idx == line) ? LIGHTGREEN : Display.usForColor)

void Disp_UserUpdate4FeedQuality(void)
{
    if (DISPLAY_PAGE_USER_SET != Display.curPage.ucMain)
    {
        return;
    }

    if (0XFFFF == Display.usX4FeedQuality)
    {
        return;
    }
    Disp_userSetDrawFeedQuality(Display.usX4FeedQuality,Display.usY4FeedQuality,Display.usCharColor4FeedQuality,Display.usBackColor4FeedQuality);    
}

void Disp_UserSetInitPage(uint8_t ucCurLine,uint8_t ucCurCol,uint8_t ucLineOffset)
{
//20170516
    int xoff = 10;   
    int yoff = 8;   // int yoff = 15;
    int iLines = 0;

    uint8_t ucLineIdx = 0;
    uint8_t ucItems = 0;

    RTC_Get();

    stimer4Us = timer;

    LCD_Clear(WHITE);

    Display.ucCurLine    = ucCurLine;
    Display.ucCurCol     = ucCurCol;
    Display.ucLineOffset = ucLineOffset;
    Display.ucDispCol    = 0X0;

    Display.usX4FeedQuality             = 0XFFFF;
    Display.usY4FeedQuality             = 0XFFFF;
    Display.usCharColor4FeedQuality     = 0XFFFF;
    Display.usBackColor4FeedQuality     = 0XFFFF;    

    iLines = DISP_US_ITEM_NUM - ucLineOffset;

    for (ucLineIdx = 0; ucLineIdx < DEF_MIN(DISPLAY_MAX_LINE_PER_PAGE,iLines); ucLineIdx++)
    {
        switch(ucLineIdx+ucLineOffset)
        {
        case DISP_US_ITEM_PACK:
             if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
             {
                 Disp_userSetDrawPack(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                 Display.aucSetLine[ucItems++] = DISP_US_ITEM_PACK;
                 yoff += 24;
             }           
             break;
         case DISP_US_ITEM_FILTER:
             Disp_userSetDrawFilter(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
             Display.aucSetLine[ucItems++] = DISP_US_ITEM_FILTER;
             yoff += 24;
             break;
         case DISP_US_ITEM_UV:
             if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
             {
                Disp_userSetDrawUV(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = DISP_US_ITEM_UV;
                yoff += 24;
             }
             break;
         case DISP_US_ITEM_FLOW:
             Disp_userSetDrawFlow(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
             Display.aucSetLine[ucItems++] = DISP_US_ITEM_FLOW;
             yoff += 24;
             break;
         case DISP_US_ITEM_UNIT:
             if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
             {
                 Disp_userSetDrawUnit(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                 Display.aucSetLine[ucItems++] = DISP_US_ITEM_UNIT;
                 yoff += 24;
             }
             break;
#ifdef TIME_FUNCTION    
         case DISP_US_ITEM_TIME:
             Disp_userSetDrawTime(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
             Display.aucSetLine[ucItems++] = DISP_US_ITEM_TIME;
             yoff += 24;
             break;
#endif             
         case DISP_US_ITEM_LANGUAGE:
             Disp_userSetDrawLanguage(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
             Display.aucSetLine[ucItems++] = DISP_US_ITEM_LANGUAGE;
             yoff += 24;
             break;
#ifdef UF_FUNCTION             
         case DISP_US_ITEM_UF_CLEAN:
             Disp_userSetDrawUFWash(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
             Display.aucSetLine[ucItems++] = DISP_US_ITEM_UF_CLEAN;
             yoff += 24;
             break;
         case DISP_US_ITEM_UF_FLUSH:
             Disp_userSetDrawFlush(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
             Display.aucSetLine[ucItems++] = DISP_US_ITEM_UF_FLUSH;
             yoff += 24;
             break;
       case DISP_US_ITEM_FEED_QUALITY:
             Disp_userSetDrawFeedQuality(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
             Display.aucSetLine[ucItems++] = DISP_US_ITEM_FEED_QUALITY;
             yoff += 24;
             break;
#endif 
         case DISP_US_ITEM_FEED_KEY:
		 	 if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
		 	 {
                Disp_userSetDrawFeed_Key(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = DISP_US_ITEM_FEED_KEY;
                yoff += 24;
                break;
		 	 }
         case DISP_US_ITEM_SERIAL_NO:
             Disp_userSetDrawSerialNo(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
             Display.aucSetLine[ucItems++] = DISP_US_ITEM_SERIAL_NO;
             yoff += 24;
             break;
        }
    }
    Display.ucTotalLine = ucItems;
}

void Disp_Move2UserSet(void)
{
    Disp_Move2DstPage();
    Disp_UserSetInitPage(0,0XFF,0);
}

void Disp_Prepare4UserSet(void)
{
    Display.tgtPage.ucMain = DISPLAY_PAGE_USER_SET;
    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_IDLE;
    Disp_Move2UserSet();
}

void Disp_engSetDrawCellConstant(int iChl,int xoff,int yoff,u16 charColor,u16 bkColor)
{
    char buf[16];
    uint8_t pos[CELLCONST_SET_NUMS] = CELLCONST_SET_POS;   
    Disp_Int2floatFormat(Display.cfg.cfg1.CELLCONSTANT[iChl],1,3,buf);
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
#ifdef UF_FUNCTION        
        if (0 == iChl )strcpy((char *)Config_buff,"CELL CONST.UP:");
        if (1 == iChl )strcpy((char *)Config_buff,"CELL CONST.DI:");
	
#elif defined PRO
        if (0 == iChl )strcpy((char *)Config_buff,"CELL CONST.UP:");
        if (1 == iChl )strcpy((char *)Config_buff,"CELL CONST.DI:"); 
#else
        if (0 == iChl )strcpy((char *)Config_buff,"CELL CONST :");
#endif
#ifdef TOC            
        if (2 == iChl) strcpy((char *)Config_buff,"CELL CONST.TOC:");
#endif       
        break;
    case DISP_LAN_GERMANY:
#ifdef UF_FUNCTION        
        if (0 == iChl )strcpy((char *)Config_buff,"ZELL KONST.UP:");
        if (1 == iChl )strcpy((char *)Config_buff,"ZELL KONST.DI:");

#elif defined PRO
        if (0 == iChl )strcpy((char *)Config_buff,"ZELL KONST.UP:");
        if (1 == iChl )strcpy((char *)Config_buff,"ZELL KONST.DI:");
#else
        if (0 == iChl )strcpy((char *)Config_buff,"ZELL KONST :");
#endif
#ifdef TOC            
        if (2 == iChl) strcpy((char *)Config_buff,"ZELL CONST.TOC:");
#endif  

        break;
    default:
#ifdef UF_FUNCTION        
        if (0 == iChl )strcpy((char *)Config_buff,"电极常数.上  :");
        if (1 == iChl )strcpy((char *)Config_buff,"电极常数.下  :");

#elif defined PRO
        if (0 == iChl )strcpy((char *)Config_buff,"电极常数.上  :");
        if (1 == iChl )strcpy((char *)Config_buff,"电极常数.下  :"); 
#else
        if (0 == iChl )strcpy((char *)Config_buff,"电极常数   :");
#endif
#ifdef TOC            
        if (2 == iChl) strcpy((char *)Config_buff,"电极常数.TOC :");
#endif     
        break;
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);

    xoff += strlen((char *)Config_buff)*curFont->sizeX;
    sprintf((char *)Config_buff,"%s",buf);
    if (Display.ucCurCol < CELLCONST_SET_NUMS)
    {
        Disp_DrawSelChar(xoff,yoff,Config_buff,pos[Display.ucCurCol],charColor,bkColor);
    }
    else
    {
        curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
    }    
}

void Disp_engSetDrawTempConstant(int iChl,int xoff,int yoff,u16 charColor,u16 bkColor)
{
    char buf[16];
    uint8_t pos[TEMPCONST_SET_NUMS] = TEMPCONST_SET_POS;
    
    Disp_Int2floatFormat(Display.cfg.cfg1.TEMPCONSTANT[iChl],1,3,buf);

    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
#ifdef UF_FUNCTION        
        //if (0 == iChl) strcpy((char *)Config_buff,"TEMP CONST.UP:");
        if (0 == iChl) strcpy((char *)Config_buff,"TEMP CONST.UP:");
        if (1 == iChl) strcpy((char *)Config_buff,"TEMP CONST.DI:");
	
#elif defined PRO
       if (0 == iChl) strcpy((char *)Config_buff,"TEMP CONST.UP:");
       if (1 == iChl) strcpy((char *)Config_buff,"TEMP CONST.DI:");
#else
        if (0 == iChl) strcpy((char *)Config_buff,"TEMP CONST :");
#endif
#ifdef TOC            
        if (2 == iChl) strcpy((char *)Config_buff,"TEMP CONST.TOC:");
#endif 
        break;
    case DISP_LAN_GERMANY:
#ifdef UF_FUNCTION        
        if (0 == iChl) strcpy((char *)Config_buff,"TEMP KONST.UP:");
        if (1 == iChl) strcpy((char *)Config_buff,"TEMP KONST.DI:");

#elif defined PRO
        if (0 == iChl) strcpy((char *)Config_buff,"TEMP KONST.UP:");
        if (1 == iChl) strcpy((char *)Config_buff,"TEMP KONST.DI:");
#else
        if (0 == iChl) strcpy((char *)Config_buff,"TEMP KONST :");
#endif
#ifdef TOC            
        if (2 == iChl) strcpy((char *)Config_buff,"TEMP CONST.TOC:");
#endif         
        break;
    default:
#ifdef UF_FUNCTION        
        if (0 == iChl) strcpy((char *)Config_buff,"温补系数.上  :");
        if (1 == iChl) strcpy((char *)Config_buff,"温补系数.下  :");
#elif defined PRO
        if (0 == iChl) strcpy((char *)Config_buff,"温补系数.上  :");
        if (1 == iChl) strcpy((char *)Config_buff,"温补系数.下  :");
#else
        if (0 == iChl) strcpy((char *)Config_buff,"温补系数   :");
#endif
#ifdef TOC            
        if (2 == iChl) strcpy((char *)Config_buff,"温补系数.TOC :");
#endif         
        break;
    }

    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);

    xoff += strlen((char *)Config_buff)*curFont->sizeX;
    strcpy((char *)Config_buff,buf);

    if (Display.ucCurCol < TEMPCONST_SET_NUMS)
    {
        Disp_DrawSelChar(xoff,yoff,Config_buff,pos[Display.ucCurCol],charColor,bkColor);
    }
    else
    {
        curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
    }      
}

void Disp_engSetDrawTempRange(int chl,int xoff,int yoff,u16 charColor,u16 bkColor)
{
    uint8_t pos[TEMP_RANGE_SET_NUMS] = TEMP_RANGE_SET_POS;

    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
    case DISP_LAN_GERMANY:
        if (charColor == LIGHTGREEN)
        {
#ifdef UF_FUNCTION        
            if (0 == chl) strcpy((char *)Config_buff,"TP.UP:");
            if (1 == chl) strcpy((char *)Config_buff,"TP.DI:");
#elif defined PRO
            if (0 == chl) strcpy((char *)Config_buff,"TP.UP:");
            if (1 == chl) strcpy((char *)Config_buff,"TP.DI:");
#else
            if (0 == chl) strcpy((char *)Config_buff,"TEMP:");
#endif
        }
        else
        {
#ifdef UF_FUNCTION        
            if (0 == chl) strcpy((char *)Config_buff,"TEMP.S.P.UP:");
            if (1 == chl) strcpy((char *)Config_buff,"TEMP.S.P.DI:");
#elif defined PRO
            if (0 == chl) strcpy((char *)Config_buff,"TEMP.S.P.UP:");
            if (1 == chl) strcpy((char *)Config_buff,"TEMP.S.P.DI:");
#else
            if (0 == chl) strcpy((char *)Config_buff,"TEMP.S.P:");
#endif
        }
        break;
    default:
        if (charColor == LIGHTGREEN)
        {
#ifdef UF_FUNCTION        
            if (0 == chl) strcpy((char *)Config_buff,"温设.上:");
            if (1 == chl) strcpy((char *)Config_buff,"温设.下:");
#elif defined PRO
            if (0 == chl) strcpy((char *)Config_buff,"温设.上:");
            if (1 == chl) strcpy((char *)Config_buff,"温设.下:");
#else
            if (0 == chl) strcpy((char *)Config_buff,"温度设定:");
#endif
        }
        else
        {
#ifdef UF_FUNCTION        
            if (0 == chl) strcpy((char *)Config_buff,"温度设定.上:");
            if (1 == chl) strcpy((char *)Config_buff,"温度设定.下:");
#elif defined PRO
            if (0 == chl) strcpy((char *)Config_buff,"温度设定.上:");
            if (1 == chl) strcpy((char *)Config_buff,"温度设定.下:");
#else
            if (0 == chl) strcpy((char *)Config_buff,"温度设定:");
#endif
        }
        break;
    }

    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);

    xoff += strlen((char *)Config_buff)*curFont->sizeX;

    sprintf((char *)Config_buff,TEMP_RANGE_FORMAT,Display.cfg.cfg2.TEMP_MAX[chl],Display.cfg.cfg2.TEMP_MIN[chl]);

    if (Display.ucCurCol < TEMP_RANGE_SET_NUMS)
    {
        Disp_DrawSelChar(xoff,yoff,Config_buff,pos[Display.ucCurCol],charColor,bkColor);
    }
    else
    {
        curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
    }
}

void Disp_engSetDrawProductRs(int chl,int xoff,int yoff,u16 charColor,u16 bkColor)
{
    char buf[16];
    Disp_Int2floatFormat(Display.cfg.cfg2.PROD_RES[chl],1,1,buf);

    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
    case DISP_LAN_GERMANY:
#ifdef UF_FUNCTION        
        if (0 == chl) sprintf((char *)Config_buff,"PROD RES.SP.UP:%s"UNIT_TYPE_0,buf);
        if (1 == chl) sprintf((char *)Config_buff,"FEED COND.SP.:%s"UNIT_TYPE_1,buf);
#elif defined PRO
        if (0 == chl) sprintf((char *)Config_buff,"PROD RES.SP.UP:%s"UNIT_TYPE_0,buf);
        if (1 == chl) sprintf((char *)Config_buff,"FEED COND.SP.:%s"UNIT_TYPE_1,buf);
#else
        if (0 == chl) sprintf((char *)Config_buff,"PROD RES.SP.:%s"UNIT_TYPE_0,buf);
#endif
        break;
    default:
#ifdef UF_FUNCTION        
        if (0 == chl) sprintf((char *)Config_buff,"水质设定.上:%s "UNIT_TYPE_0,buf);
        if (1 == chl) sprintf((char *)Config_buff,"水质设定.下:%s "UNIT_TYPE_1,buf);
#elif defined PRO
        if (0 == chl) sprintf((char *)Config_buff,"水质设定.上:%s "UNIT_TYPE_0,buf);
        if (1 == chl) sprintf((char *)Config_buff,"水质设定.下:%s"UNIT_TYPE_1,buf);
#else
        if (0 == chl) sprintf((char *)Config_buff,"水质设定   :%s "UNIT_TYPE_0,buf);
#endif
        break;
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_engSetDrawPack(int xoff,int yoff,u16 charColor,u16 bkColor)
{

    uint8_t pos[PACK_SET_NUMS] = PACK_SET_POS;

    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"PACK  LIFE :");
    }
    else
    {
        sprintf((char *)Config_buff,"纯化柱寿命 :");
    }

    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);

    xoff += strlen((char *)Config_buff)*curFont->sizeX;

    if (Display.ucCurCol < PACK_SET_NUMS)
    {
        switch(Display.cfg.cfg1.language)
        {
        default:
            sprintf((char *)Config_buff,PACK_FORMAT_CHN,Display.cfg.cfg2.PACKLIFEDAY,Display.cfg.cfg2.PACKLIFEL);
            break;
        case DISP_LAN_ENGLISH:
            sprintf((char *)Config_buff,PACK_FORMAT_ENG,Display.cfg.cfg2.PACKLIFEDAY,Display.cfg.cfg2.PACKLIFEL);
            break;
        case DISP_LAN_GERMANY:
            sprintf((char *)Config_buff,PACK_FORMAT_DUT,Display.cfg.cfg2.PACKLIFEDAY,Display.cfg.cfg2.PACKLIFEL);
            break;
        }    
    
        Disp_DrawSelChar(xoff,yoff,Config_buff,pos[Display.ucCurCol],charColor,bkColor);
    }
    else
    {
        switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
            sprintf((char *)Config_buff,"%dDAYS%dL    ",Display.cfg.cfg2.PACKLIFEDAY,Display.cfg.cfg2.PACKLIFEL);
            break;
        case DISP_LAN_GERMANY:
            sprintf((char *)Config_buff,"%dTAGE%dL    ",Display.cfg.cfg2.PACKLIFEDAY,Display.cfg.cfg2.PACKLIFEL);
            break;
        default:
            sprintf((char *)Config_buff,"%d天%d升    ",Display.cfg.cfg2.PACKLIFEDAY,Display.cfg.cfg2.PACKLIFEL);
            break;
        }   
        curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
    }    
}

void Disp_engSetDrawFilter(int xoff,int yoff,u16 charColor,u16 bkColor)
{

    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        sprintf((char *)Config_buff,"FILTER LIFE:%dDAYS    ",Display.cfg.cfg2.FILTERLIFE);
        break;
    case DISP_LAN_GERMANY:
        sprintf((char *)Config_buff,"FILTER LIFE:%dTAGE    ",Display.cfg.cfg2.FILTERLIFE);
        break;
    default:
        sprintf((char *)Config_buff,"过滤器寿命 :%d天    ",Display.cfg.cfg2.FILTERLIFE);
        break;
    }

    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_engSetDrawUV(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    uint8_t pos[UV_SET_NUMS] = UV_SET_POS;
   
    if (DISP_LAN_CHINESE == Display.cfg.cfg1.language)
    {
        uint8_t ucPos;
        for (ucPos = 3; ucPos < UV_SET_NUMS; ucPos++)
        {
            pos[ucPos] -= 2;
        }
    }

    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
        || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"UV  LIFE   :");
    }
    else
    {
        sprintf((char *)Config_buff,"紫外灯寿命 :");
    }

    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);

    xoff += strlen((char *)Config_buff)*curFont->sizeX;

    if (Display.ucCurCol < UV_SET_NUMS)
    {
        switch(Display.cfg.cfg1.language)
        {
        default:
            sprintf((char *)Config_buff,UV_FORMAT_CHN,Display.cfg.cfg2.UVLIFEDAY,Display.cfg.cfg2.UVLIFEHOUR);
            break;
        case DISP_LAN_ENGLISH:
            sprintf((char *)Config_buff,UV_FORMAT_ENG,Display.cfg.cfg2.UVLIFEDAY,Display.cfg.cfg2.UVLIFEHOUR);
            break;
        case DISP_LAN_GERMANY:
            sprintf((char *)Config_buff,UV_FORMAT_DUT,Display.cfg.cfg2.UVLIFEDAY,Display.cfg.cfg2.UVLIFEHOUR);
            break;
        }
    
        Disp_DrawSelChar(xoff,yoff,Config_buff,pos[Display.ucCurCol],charColor,bkColor);
    }
    else
    {
        switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
            sprintf((char *)Config_buff,"%dDAYS%dHr    ",Display.cfg.cfg2.UVLIFEDAY,Display.cfg.cfg2.UVLIFEHOUR);
            break;
        case DISP_LAN_GERMANY:
            sprintf((char *)Config_buff,"%dTAGE%dHr    ",Display.cfg.cfg2.UVLIFEDAY,Display.cfg.cfg2.UVLIFEHOUR);
            break;
        default:
            sprintf((char *)Config_buff,"%d天%d小时  ",Display.cfg.cfg2.UVLIFEDAY,Display.cfg.cfg2.UVLIFEHOUR);
            break;
        }
    
        curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
    }      
}

void Disp_engSetDrawRecirculation(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        sprintf((char *)Config_buff,"RECIRC.    :%d min      ",Display.cfg.cfg1.cirtime);
        break;
    case DISP_LAN_GERMANY:
        sprintf((char *)Config_buff,"REZIRK.    :%d min      ",Display.cfg.cfg1.cirtime);
        break;
    default:
        sprintf((char *)Config_buff,"循环时间   :%d 分钟     ",Display.cfg.cfg1.cirtime);
        break;
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_engSetDrawUFDisinfection(int xoff,int yoff,u16 charColor,u16 bkColor)
{
    switch(Display.cfg.cfg1.language)
    {
    case DISP_LAN_ENGLISH:
        sprintf((char *)Config_buff,"UF DISINFECTION:%d DAYS    ",Display.cfg.cfg3.usUFCleanTime);
        break;
    case DISP_LAN_GERMANY:
        sprintf((char *)Config_buff,"UF REINIGUNG.:%d TAGE      ",Display.cfg.cfg3.usUFCleanTime);
        break;
    default:
        sprintf((char *)Config_buff,"UF 清洗周期:%d  天         ",Display.cfg.cfg3.usUFCleanTime);
        break;
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_engSetDrawUFFlush(int xoff,int yoff,u16 charColor,u16 bkColor)
{

    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"UF FLUSH :%d sec      ",Display.cfg.cfg3.usUFFlushTime);
    }
    else if (DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"SPELUNG :%d sec      ",Display.cfg.cfg3.usUFFlushTime);
    }
    else
    {
        sprintf((char *)Config_buff,"冲洗时间:%d 秒       ",Display.cfg.cfg3.usUFFlushTime);
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_engSetDrawUFAutoFlush(int xoff,int yoff,u16 charColor,u16 bkColor)
{

    if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"AUTO. FLUSH :%d Min      ",Display.cfg.cfg3.usAutoUFFlushTime);
    }
    else if (DISP_LAN_GERMANY == Display.cfg.cfg1.language)
    {
        sprintf((char *)Config_buff,"AUTO. SPUELUNG:%d Min      ",Display.cfg.cfg3.usAutoUFFlushTime);
    }
    else
    {
        sprintf((char *)Config_buff,"自动冲洗时间:%d 分钟       ",Display.cfg.cfg3.usAutoUFFlushTime);
    }
    curFont->DrawText(xoff,yoff,Config_buff,charColor,bkColor);
}

void Disp_EngSetInitPage(uint8_t ucCurLine,uint8_t ucCurCol,uint8_t ucLineOffSet)
{
//20170516
    int xoff = 10;  
    int yoff = 8;//int yoff = 15;
    int iLines = 0;
    uint8_t ucLineIdx = 0;
    uint8_t ucItems = 0;

    LCD_Clear(WHITE);

    Display.ucCurLine    = ucCurLine;
    Display.ucCurCol     = ucCurCol;
    Display.ucLineOffset = ucLineOffSet;
    Display.ucDispCol    = 0X0;
 
    iLines = DISP_ES_ITEM_NUM - ucLineOffSet;

    for (ucLineIdx = 0; ucLineIdx < DEF_MIN(DISPLAY_MAX_LINE_PER_PAGE,iLines);ucLineIdx++)
    {
        switch(ucLineIdx+ucLineOffSet)
        {
        case DISP_ES_ITEM_CELL_CONSTANT1:
#ifdef UF_FUNCTION            
        case DISP_ES_ITEM_CELL_CONSTANT2:
#endif         
#ifdef PRO
        case DISP_ES_ITEM_CELL_CONSTANT2:
#endif
#ifdef TOC            
        case DISP_ES_ITEM_CELL_CONSTANT3:
#endif            
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawCellConstant((ucLineIdx+ucLineOffSet - DISP_ES_ITEM_CELL_CONSTANT1)/2,xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = ucLineIdx+ucLineOffSet;
                yoff += 24;
            }
            break;
        case DISP_ES_ITEM_TEMP_CONSTANT1:
#ifdef UF_FUNCTION            
        case DISP_ES_ITEM_TEMP_CONSTANT2:
#endif  
#ifdef PRO
        case DISP_ES_ITEM_TEMP_CONSTANT2:
#endif
#ifdef TOC            
        case DISP_ES_ITEM_TEMP_CONSTANT3:
#endif            
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawTempConstant((ucLineIdx+ucLineOffSet - DISP_ES_ITEM_TEMP_CONSTANT1)/2,xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = ucLineIdx+ucLineOffSet;
                yoff += 24;
            }
            break;
        case DISP_ES_ITEM_TEMP_RANGE1:
#ifdef UF_FUNCTION            
        case DISP_ES_ITEM_TEMP_RANGE2:    
#endif  
#ifdef PRO           
        case DISP_ES_ITEM_TEMP_RANGE2:    
#endif
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawTempRange(ucLineIdx+ucLineOffSet - DISP_ES_ITEM_TEMP_RANGE1,xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = ucLineIdx+ucLineOffSet;//DISP_ES_ITEM_TEMP_RANGE1;
                yoff += 24;
            }
            break;
        case DISP_ES_ITEM_PRODUCT_RS1:
#ifdef UF_FUNCTION            
        case DISP_ES_ITEM_PRODUCT_RS2:
#endif 
#ifdef PRO            
        case DISP_ES_ITEM_PRODUCT_RS2:
#endif 
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawProductRs(ucLineIdx+ucLineOffSet-DISP_ES_ITEM_PRODUCT_RS1,xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = ucLineIdx+ucLineOffSet;//DISP_ES_ITEM_PRODUCT_RS1;
                yoff += 24;
            }
            break;
        case DISP_ES_ITEM_PACK:
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawPack(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = DISP_ES_ITEM_PACK;
                yoff += 24;
            }
            break;
        case DISP_ES_ITEM_FILTER:
            Disp_engSetDrawFilter(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
            Display.aucSetLine[ucItems++] = DISP_ES_ITEM_FILTER;
            yoff += 24;
            break;        
        case DISP_ES_ITEM_UV:
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawUV(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = DISP_ES_ITEM_UV;
                yoff += 24;
            }
            break;     
        case DISP_ES_ITEM_RECIRCULATION:
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawRecirculation(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = DISP_ES_ITEM_RECIRCULATION;
                yoff += 24;
            }
            break;
#ifdef UF_FUNCTION        
        case DISP_ES_ITEM_UF_DISINFECTION:
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawUFDisinfection(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = DISP_ES_ITEM_UF_DISINFECTION;
                yoff += 24;
            }
            break;
        case DISP_ES_ITEM_UF_FLUSH:
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawUFFlush(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = DISP_ES_ITEM_UF_FLUSH;
                yoff += 24;
            }
            break;
        case DISP_ES_ITEM_UF_AUTO_FLUSH:
            if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
            {
                Disp_engSetDrawUFAutoFlush(xoff,yoff,HIGH_LIGHTING(ucLineIdx,ucCurLine),Display.usBackColor4Set);
                Display.aucSetLine[ucItems++] = DISP_ES_ITEM_UF_AUTO_FLUSH;
                yoff += 24;
            }
            break;
#endif           
        }
    }  
    Display.ucTotalLine = ucItems;
}

void Disp_Move2EngSet(void)
{
    Disp_Move2DstPage();
    Disp_EngSetInitPage(0,0XFF,0);    
}

void Disp_Prepare4EngSet(void)
{
    Display.tgtPage.ucMain = DISPLAY_PAGE_ENG_SET;
    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_IDLE;
    Disp_Move2EngSet();
}

void Disp_KeyHandler_idle_idle(int key,int state)
{
    // set keystate    
    switch(key)
    {
    case KEY_CODE_NORMAL_K1: 
        if (KEYSTATE_PRESSED == state)
        {
            Disp_Prepare4Ntw();
        }
        break;
    case KEY_CODE_RECURSIVE_K2:
        {
            uint8_t ls = keyboard_get_linestate();
            if (KEYSTATE_PRESSED == state)
            {
                if (((1<<KEY_CODE_RECURSIVE_K2)|(1<<KEY_CODE_QUANTITY_K3)) == ls)
                {
                    Display.KeyState |=  (1 << KEY_CODE_QUANTITY_K3) ;
                    Disp_Prepare4UserSet();
                }
                else
                {
                    Disp_Prepare4Circulation();
                }                    
            }
        }
        break;
    case KEY_CODE_QUANTITY_K3:
        {
            uint8_t ls = keyboard_get_linestate();
            
            if (KEYSTATE_PRESSED == state)
            {
                if (((1<<KEY_CODE_RECURSIVE_K2)|(1<<KEY_CODE_QUANTITY_K3)) == ls)
                {
                    Display.KeyState |=  (1 << KEY_CODE_RECURSIVE_K2) ;
                    Disp_Prepare4UserSet();
                }
                else if (((1<<KEY_CODE_SET_K4)|(1<<KEY_CODE_QUANTITY_K3)) == ls)
                {
                    Display.KeyState |=  (1 << KEY_CODE_SET_K4) ;
                    Disp_Prepare4EngSet();
                }
                else
                {
                    Display.tgtPage.ucMain = DISPLAY_PAGE_QUANTITY_TAKING_WATER;
                    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_SETTING;
                    Disp_Move2QtwSetting();
                }
            }
        }
        break;
    case KEY_CODE_SET_K4:
        {
            uint8_t ls = keyboard_get_linestate();
            if (KEYSTATE_PRESSED == state)
            {
                if (((1<<KEY_CODE_SET_K4)|(1<<KEY_CODE_QUANTITY_K3)) == ls)
                {
                    Display.KeyState |=  (1 << KEY_CODE_QUANTITY_K3) ;
                    Disp_Prepare4EngSet();
                }
                else
                {
                    // transfer state to idle@dec
                    Display.tgtPage.ucMain = DISPLAY_PAGE_DECOMPRESSION;
                    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_IDLE;
                    Display.TgtState.ucMain = DISPLAY_STATE_DECOMPRESSION;
                    Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;
                    Disp_Move2Decompression();
                }
                    
            }
        }
        break;
    }
}

void Disp_KeyHandler_idle_empty_tank(int key,int state)
{
    switch(key)
    {
    case KEY_CODE_NORMAL_K1: 
    case KEY_CODE_QUANTITY_K3:
        if (KEYSTATE_PRESSED == state)
        {
            // transer to idle
            Disp_PrepareMove2Idle();
            key_em = 0;
        }
        break;
    }
}

void Disp_KeyHandler_idle_UF_flush(int key,int state)
{
   
    switch(key)
    {
    case KEY_CODE_NORMAL_K1: 
        if (KEYSTATE_PRESSED == state)
        {
       
            Disp_Prepare4Ntw();
        }
        break;
    case KEY_CODE_RECURSIVE_K2:
        {
            uint8_t ls = keyboard_get_linestate();
		     
            if (KEYSTATE_PRESSED == state)
            {    
                if (((1<<KEY_CODE_RECURSIVE_K2)|(1<<KEY_CODE_QUANTITY_K3)) == ls)
                {
                    Display.KeyState |=  (1 << KEY_CODE_QUANTITY_K3) ;
                    key_em = 0;
                    Disp_Prepare4UserSet();
                }
                else
                {
                    key_em = 0;
                    Disp_Prepare4Circulation();
                }                    
            }
        }
        break;
    case KEY_CODE_QUANTITY_K3:
        {
            uint8_t ls = keyboard_get_linestate();
            if (KEYSTATE_PRESSED == state)
            {

            
                if (((1<<KEY_CODE_RECURSIVE_K2)|(1<<KEY_CODE_QUANTITY_K3)) == ls)
                {
                    Display.KeyState |=  (1 << KEY_CODE_RECURSIVE_K2) ;
					 key_em = 0;
                    Disp_Prepare4UserSet();
                }
                else if (((1<<KEY_CODE_SET_K4)|(1<<KEY_CODE_QUANTITY_K3)) == ls)
                {
                    Display.KeyState |=  (1 << KEY_CODE_SET_K4) ;
					 key_em = 0;
                    Disp_Prepare4EngSet();
                }
                else
                {
                    Display.tgtPage.ucMain = DISPLAY_PAGE_QUANTITY_TAKING_WATER;
                    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_SETTING;
                     key_em = 0;
                    Disp_Move2QtwSetting();
                }
            }
        }
        break;
    case KEY_CODE_SET_K4:
        {
            uint8_t ls = keyboard_get_linestate();
		    
            if (KEYSTATE_PRESSED == state)
            {
                if (((1<<KEY_CODE_SET_K4)|(1<<KEY_CODE_QUANTITY_K3)) == ls)
                {
                    Display.KeyState |=  (1 << KEY_CODE_QUANTITY_K3) ;
                     key_em = 0;
                    Disp_Prepare4EngSet();
                }
                else
                {
                    // transfer state to idle@dec
                    Display.tgtPage.ucMain = DISPLAY_PAGE_DECOMPRESSION;
                    Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_IDLE;
                    Display.TgtState.ucMain = DISPLAY_STATE_DECOMPRESSION;
                    Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;
                    Disp_Move2Decompression();
					key_em = 0;
                }
                    
            }
        }
        break;
    }

}

void Disp_KeyHandler_qtw_Setting(int key,int state)
{
    switch(key)
    {
    case KEY_CODE_RECURSIVE_K2: 
        if (Display.usQtwWq < (DISP_MAX_WATER_VOLUME-1)
            && (KEYSTATE_PRESSED == state))      
		{	
            Display.usQtwWq += 1;
            Disp_QtwSettingUpdatePage();
        }
        break;
    case KEY_CODE_SET_K4:

        if (Display.usQtwWq > 0
            && (KEYSTATE_PRESSED == state))
        {
            Display.usQtwWq -= 1;
            Disp_QtwSettingUpdatePage();
        }
        break;
    case KEY_CODE_QUANTITY_K3:
        if (KEYSTATE_PRESSED == state)
        {
            // transfer state to Setting@QTW
            Disp_Prepare4Qtw();
        }
        break;
    case KEY_CODE_NORMAL_K1:
        if (KEYSTATE_PRESSED == state)
        {
            // transfer state to idle@NTW
            Disp_Prepare4Ntw();
        }        
        break;
    }

}

void Disp_KeyHandler_qtw_empty_tank(int key,int state)
{
    switch(key)
    {
    case KEY_CODE_NORMAL_K1:
    case KEY_CODE_QUANTITY_K3:
        if (KEYSTATE_PRESSED == state)
        {
        }
        break;
    }

}

void Disp_KeyHandler_qtw_tw(int key,int state)
{
    // currently empty
    switch(key)
    {
    case KEY_CODE_NORMAL_K1:
        if (KEYSTATE_PRESSED == state)
        {
            Disp_StopQtwTakingWater();

            // transfer state to idle@NTW
            Disp_Prepare4Ntw();
        }
        break;
    case KEY_CODE_QUANTITY_K3:
        if (KEYSTATE_PRESSED == state)
        {
            Disp_StopQtwTakingWater();

            // transfer state to idle@cir
            Disp_Prepare4Circulation();
        }
        break;
    }
    
}



void Disp_KeyHandler_qtw(int key,int state)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_SETTING:
        Disp_KeyHandler_qtw_Setting(key,state);
        break;
    case DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_VOID_TANK:
        Disp_KeyHandler_qtw_empty_tank(key,state);
        break;
    case DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_TAKING:
        Disp_KeyHandler_qtw_tw(key,state);
        break;
    }
}



void Disp_KeyHandler_ntw_idle(int key,int state)
{
    switch(key)
    {
    case KEY_CODE_NORMAL_K1:
        if (KEYSTATE_PRESSED == state)
        {
            Disp_StopNtwTakingWater();
            
            Disp_Prepare4Circulation();
        }
        break;
    default:
        break;
    }

}


void Disp_KeyHandler_ntw(int key,int state)
{

    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
        Disp_KeyHandler_ntw_idle(key,state);
        break;
    }

}

void Disp_KeyHandler_cir_idle(int key,int state)
{
    switch(key)
    {
    case KEY_CODE_NORMAL_K1:
        if (KEYSTATE_PRESSED == state)
        {
            sys_untimeout(&Display.to4cir);
            Disp_Prepare4Ntw();
        }
        break;
    // 2014/10/30 add begin
    case KEY_CODE_QUANTITY_K3:
        if (KEYSTATE_PRESSED == state)
        {
            sys_untimeout(&Display.to4cir);
			key_toc = 0;
            
            Disp_PrepareMove2QtwSetting();
        }
        break;
    // 2014/10/30 add end
    case KEY_CODE_RECURSIVE_K2:
        {
            if (KEYSTATE_PRESSED == state)
            {
               sys_untimeout(&Display.to4cir);
               Disp_PrepareMove2Idle();
            }
        }
        break;
    default:
        break;
    }

}

void Disp_KeyHandler_circular(int key,int state)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
        Disp_KeyHandler_cir_idle(key,state);
        break;
    case DISPALY_SUB_PAGE_TOC_FLUSH:
        break;
    case DISPALY_SUB_PAGE_TOC_OXIDATION:
        break;
    }

}
/*void Disp_KeyHandler_toc_idle(int key,int state) //20160926添加
{
    switch(key)
    {
    case KEY_CODE_NORMAL_K1:
        if (KEYSTATE_PRESSED == state)
        {
            sys_untimeout(&Display.to4cir);
            Disp_Prepare4Ntw();
        }
        break;
	default:
        break;
    }

} */

void Disp_KeyHandler_toc(int key,int state)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
		Disp_KeyHandler_cir_idle(key,state);
        break;
    case DISPALY_SUB_PAGE_TOC_FLUSH:
		Disp_KeyHandler_cir_idle(key,state);
        break;
    case DISPALY_SUB_PAGE_TOC_OXIDATION:
		Disp_KeyHandler_cir_idle(key,state);
        break;
    }

}


void Disp_KeyHandler_dec_idle(int key,int state)
{
    switch(key)
    {
    case KEY_CODE_SET_K4:
        if (KEYSTATE_PRESSED == state)
        {
            Disp_PrepareMove2Idle();
        }
        break;
    default:
        break;
    }

}

void Disp_KeyHandler_dec(int key,int state)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
        Disp_KeyHandler_dec_idle(key,state);
        break;
    }

}

void Disp_KeyHandler_UF_Disinfection_subs(int key,int state)
{
    switch(key)
    {
    case KEY_CODE_SET_K4:
        if (KEYSTATE_PRESSED == state)
        {
            switch(Display.curPage.ucSub)
            {
            case DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP1:
                Disp_PrepareMove2UFClean();
                break;
            case DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP2:
                Disp_Prepare4Degass1();
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

}


void Disp_KeyHandler_uf_disinfection(int key,int state)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP1:
    case DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP2:
        Disp_KeyHandler_UF_Disinfection_subs(key,state);
        break;
    default:
        break;
    }

}


void Disp_Us_ChangeSel(void)
{
   //20170516
    int xoff = 10;
   // int yoff = 15;
   int yoff = 8;

    // calc Move Display Page
    if (Display.ucCurLine + 1 + Display.ucLineOffset >= DISP_US_ITEM_NUM)
    {
        Disp_UserSetInitPage(0,0XFF,0);
        return;
    }

    if (Display.ucCurLine + 1 >= DISPLAY_MAX_LINE_PER_PAGE)
    {
        Display.ucLineOffset += DISPLAY_MAX_LINE_PER_PAGE;
        Disp_UserSetInitPage(0,0XFF,Display.ucLineOffset);
        return;
    }
    
  //20170516
  //  yoff = 15 + 24*Display.ucCurLine;
	  yoff = 8 + 24*Display.ucCurLine;
    Display.ucCurCol = 0XFF;
    Display.ucDispCol = 0X0;

    switch(Display.aucSetLine[Display.ucCurLine])
    {
    case DISP_US_ITEM_PACK:
        Disp_userSetDrawPack(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_FILTER:
        Disp_userSetDrawFilter(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_UV:
        Disp_userSetDrawUV(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_FLOW:
        Disp_userSetDrawFlow(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_UNIT:
        Disp_userSetDrawUnit(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
	
#ifdef TIME_FUNCTION    
    case DISP_US_ITEM_TIME:
        Disp_userSetDrawTime(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
#endif        
    case DISP_US_ITEM_LANGUAGE:
        Disp_userSetDrawLanguage(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
#ifdef UF_FUNCTION        
    case DISP_US_ITEM_UF_CLEAN:
        Disp_userSetDrawUFWash(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_UF_FLUSH:
        Disp_userSetDrawFlush(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_FEED_QUALITY:
        Disp_userSetDrawFeedQuality(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
#endif  
    case DISP_US_ITEM_FEED_KEY://20170127
		Disp_userSetDrawFeed_Key(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
		break;
    case DISP_US_ITEM_SERIAL_NO:
        Disp_userSetDrawSerialNo(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    }

    Display.ucCurLine = (Display.ucCurLine + 1) % Display.ucTotalLine;

    //yoff = 15 + 24*Display.ucCurLine;
    yoff = 8 + 24*Display.ucCurLine;
    switch(Display.aucSetLine[Display.ucCurLine])
    {
    case DISP_US_ITEM_PACK:
        Disp_userSetDrawPack(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_FILTER:
        Disp_userSetDrawFilter(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_UV:
        Disp_userSetDrawUV(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_FLOW:
        Disp_userSetDrawFlow(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_UNIT:
        Disp_userSetDrawUnit(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#ifdef TIME_FUNCTION    
    case DISP_US_ITEM_TIME:
        Disp_userSetDrawTime(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#endif        
    case DISP_US_ITEM_LANGUAGE:
        Disp_userSetDrawLanguage(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#ifdef UF_FUNCTION        
    case DISP_US_ITEM_UF_CLEAN:
        Disp_userSetDrawUFWash(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_UF_FLUSH:
        Disp_userSetDrawFlush(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_FEED_QUALITY:
        Disp_userSetDrawFeedQuality(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#endif      

    case DISP_US_ITEM_FEED_KEY: //20170117
		Disp_userSetDrawFeed_Key(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
		break;
    case DISP_US_ITEM_SERIAL_NO:
        Disp_userSetDrawSerialNo(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    }

}


void Disp_UserSetSetPack(void)
{
    Display.info.PACKLIFEDAY = RTC_Get_Second();
    Display.info.PACKLIFEL   = 0;

}

void Disp_UserSetSetFilter(void)
{
    Display.info.FILTERLIFE  = RTC_Get_Second();;
}

void Disp_UserSetSetUV(void)
{
    Display.info.UVLIFEDAY = RTC_Get_Second();
    Display.info.UVLIFEHOUR = 0;

}

void Disp_UserSetSetFlow(int key)
{

    if (KEY_CODE_SET_K4 == key)
    {
        if (Display.cfg.cfg2.FLOW > 0)
        {
            Display.cfg.cfg2.FLOW--;
        }
    }
    else
    {
        if (Display.cfg.cfg2.FLOW < 999)
        {
            Display.cfg.cfg2.FLOW++;
        }
    }

}

void Disp_UserSetSetLanguage(int key)
{
    uint8_t lan[] = {DISP_LAN_CHINESE,DISP_LAN_GERMANY,DISP_LAN_ENGLISH}; 
    Display.cfg.cfg1.language = lan[Display.cfg.cfg1.language];

}

void Disp_UserSetSetUnit(int key)
{

    Display.cfg.cfg2.UNIT = !Display.cfg.cfg2.UNIT;

}
void Disp_UserSetSetFeed_Key(int key)
{

    Display.cfg.cfg2.FEED_KEY = !Display.cfg.cfg2.FEED_KEY;

}


void Disp_UserSetSetTime(int key)
{

   // us key2 to navigate
   if (KEY_CODE_RECURSIVE_K2 == key)
   {
       Display.ucCurCol = ((uint8_t)(Display.ucCurCol + 1)) % TIME_SET_NUMS;
       return ;
   }

   // change value
   if (Display.ucCurCol < TIME_SET_NUMS)
   {
   
      uint8_t pos[14] = TIME_SET_POS;
      
      sprintf((char *)Config_buff,TIME_FORMAT,stimer4Us.w_year,stimer4Us.w_month,stimer4Us.w_date,stimer4Us.hour,stimer4Us.min,stimer4Us.sec);

      Display.bit1SetTime = TRUE;

      switch(Display.ucCurCol)
      {
      case 4:
         if (Config_buff[pos[4]] >= '1' )
         {
             Config_buff[pos[4]] = '0';
         }
         else
         {
           Config_buff[pos[4]] += 1;
         }         
         break;
      case 5:
         if (Config_buff[pos[4]] == '0' )
         {
             if (Config_buff[pos[Display.ucCurCol]] == '9' )
             {
                 Config_buff[pos[Display.ucCurCol]] = '1';
             }
             else
             {
               Config_buff[pos[Display.ucCurCol]] += 1;
             }        
         }
         else
         {
             if (Config_buff[pos[Display.ucCurCol]] >= '2' )
             {
                 Config_buff[pos[Display.ucCurCol]] = '0';
             }
             else
             {
               Config_buff[pos[Display.ucCurCol]] += 1;
             }        
         }         
        break;
      case 6:
          if (Config_buff[pos[Display.ucCurCol]] >= '3' )
          {
              Config_buff[pos[Display.ucCurCol]] = '0';
          }
          else
          {
              Config_buff[pos[Display.ucCurCol]] += 1;
          }         
        break;
      case 7:
          if (Config_buff[pos[6]] == '0' )
          {
              if (Config_buff[pos[Display.ucCurCol]] >= '9' )
              {
                  Config_buff[pos[Display.ucCurCol]] = '1';
              }
              else
              {
                Config_buff[pos[Display.ucCurCol]] += 1;
              }        
          }
          else
          {
              if (Config_buff[pos[Display.ucCurCol]] >= '9' )
              {
                  Config_buff[pos[Display.ucCurCol]] = '0';
              }
              else
              {
                  Config_buff[pos[Display.ucCurCol]] += 1;
              }        
          }         
          break;
      case 8:
      case 10:
      case 12:
          if (Config_buff[pos[Display.ucCurCol]] >= '5' )
          {
              Config_buff[pos[Display.ucCurCol]] = '0';
          }
          else
          {
            Config_buff[pos[Display.ucCurCol]] += 1;
          }        
        
        break;
      default:
          if (Config_buff[pos[Display.ucCurCol]] >= '9' )
          {
              Config_buff[pos[Display.ucCurCol]] = '0';
          }
          else
          {
            Config_buff[pos[Display.ucCurCol]] += 1;
          }        
          break;
      }
      {
          int year,mon,day,hour,min,sec;
          sscanf((char *)Config_buff,TIME_FORMAT,&year,&mon,&day,&hour,&min,&sec);
          stimer4Us.w_year  = year;
          stimer4Us.w_month = mon;
          stimer4Us.w_date  = day;
          stimer4Us.hour    = hour;
          stimer4Us.min     = min;
          stimer4Us.sec     = sec;
      }
   }
}

void Disp_DisplayUFCleanSettingPage(void)
{

    char *text;

    int xoff = 0;
    int yoff = 0;
    
    LCD_Clear(Display.usBackColor);

#ifdef CODEGB_48    
    //Disp_SelectFont(&font48);  
#endif
        
    // display info
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP1:
        switch(Display.cfg.cfg1.language)
        {
        case DISP_LAN_ENGLISH:
        case DISP_LAN_GERMANY:
            {
                text = "Insert            ";
    
                xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);
                
                yoff = LCD_H/2  - 6 - curFont->sizeY;
                
                curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
    
                //xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);
                yoff += curFont->sizeY + 5;
            
                text = "cartridge and pill";
                curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
    
                //xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);
                yoff += curFont->sizeY + 5;            
    
                text = "then press SET key";
                curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
            }            
            break;
        default:
            {
                text = "加入清洁剂   ";
                
                xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);
                
                yoff = LCD_H/2  - 6 - curFont->sizeY;
                
                curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);

                //xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);
                yoff += curFont->sizeY + 5;
                
                text = "然后按 SET 键";
                curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
    
            }            
            break;
        }
        break;
    case DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP2:
        if (DISP_LAN_ENGLISH == Display.cfg.cfg1.language
            || DISP_LAN_GERMANY == Display.cfg.cfg1.language)
        {
            text = "Insert DuoPack  ,";

            xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);
            
            yoff = LCD_H/2  - 6 - curFont->sizeY;
            
            curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
        
            text = "then press SET key";
            
            //xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);
            
            yoff += curFont->sizeY + 5;
            
            
            curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
        }
        else
        {
            text = "Insert DuoPack  ";
            
            xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);
            
            yoff = LCD_H/2  - 6 - curFont->sizeY;
            
            curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);

            text = "然后按 SET 键";
            
            //xoff = LCD_W/2 - curFont->sizeX/2*(strlen(text)+1);
            
            yoff += curFont->sizeY + 5;
            
            curFont->DrawText(xoff,yoff,(uint8_t *)text,Display.usForColor,Display.usBackColor);
        }
        break;
    default:
        return;
    }


#ifdef CODEGB_48    
     //Disp_SelectFont(oldfont);  
#endif

}

void Disp_UserSetPrepareUFClean(uint8_t ucSub)
{

     Display.tgtPage.ucMain = Display.curPage.ucMain;

     Display.tgtPage.ucSub =  ucSub;

     // close all valves
     Display.TgtState.ucMain = Display.CurState.ucMain;

     Display.TgtState.ucSub =  DISPLAY_SUB_STATE_IDLE;

     Disp_Move2DstState();
     
     Disp_Move2DstPage();

     Disp_DisplayUFCleanSettingPage();
}


void Disp_UserSetUFClean(int key)
{
     if (KEY_CODE_SET_K4 != key)
     {
         return;
     }

     // Do some work

     Disp_PrepareMove2UFPreClean();
}

void Disp_UserSetUFFlush(int key)
{
     if (KEY_CODE_SET_K4 != key)
     {
         key_em = 0;
         return;
     }

     //Disp_Prepare4Degass1();
     Disp_PrepareMove2IdleFlush();
}

void Disp_UserSetFeedQuality(int key)
{
     if (KEY_CODE_SET_K4 != key)
     {
         return;
     }

     Display.ucDispCol = (Display.ucDispCol + 1) % 8;

}


void Disp_Us_Set(int key)
{
//20170516
    int xoff = 10;
  //  int yoff = 15;
    int yoff = 8;

   // yoff = 15 + 24*Display.ucCurLine;
	yoff = 8 + 24*Display.ucCurLine;

    switch(Display.aucSetLine[Display.ucCurLine])
    {
    case DISP_US_ITEM_PACK:
        if (KEY_CODE_SET_K4 == key)
        {
            Disp_UserSetSetPack();
            Disp_userSetDrawPack(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        }
        break;
    case DISP_US_ITEM_FILTER:
        if (KEY_CODE_SET_K4 == key)
        {
            Disp_UserSetSetFilter();
            Disp_userSetDrawFilter(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        }
        break;
    case DISP_US_ITEM_UV:
        {
            Disp_UserSetSetUV();
            Disp_userSetDrawUV(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        }
        break;
    case DISP_US_ITEM_FLOW:
        Disp_UserSetSetFlow(key);
        Disp_userSetDrawFlow(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_US_ITEM_LANGUAGE:
        {
            Disp_UserSetSetLanguage(key);
            Disp_UserSetInitPage(Display.ucCurLine,Display.ucCurCol,Display.ucLineOffset);        
        }
        break;
    case DISP_US_ITEM_UNIT:
        Disp_UserSetSetUnit(key);
        Disp_userSetDrawUnit(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
	case DISP_US_ITEM_FEED_KEY: //20170127
		Disp_UserSetSetFeed_Key(key);
        Disp_userSetDrawFeed_Key(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#ifdef TIME_FUNCTION    
    case DISP_US_ITEM_TIME:
        Disp_UserSetSetTime(key);
        Disp_userSetDrawTime(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#endif        
        // add for v2
#ifdef UF_FUNCTION        
    case DISP_US_ITEM_UF_CLEAN:
        Disp_UserSetUFClean(key);
        break;
    case DISP_US_ITEM_UF_FLUSH:
        Disp_UserSetUFFlush(key);
        break;
    case DISP_US_ITEM_FEED_QUALITY:
        Disp_UserSetFeedQuality(key);  
        Disp_userSetDrawFeedQuality(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#endif   
    }

}


void Disp_CommSetSaveConfig(void)
{
   if (memcmp(&Display.cfg,&pParameter[PARAMETER_LOCAL_BEGIN_POS],offsetof(LOCAL_CONFIG_STRU,cfg3.devtype))) 
   {
       memcpy(&Config_buff[0],pParameter,PARAMETER_SIZE);

       memcpy(&Config_buff[PARAMETER_LOCAL_BEGIN_POS],&Display.cfg,offsetof(LOCAL_CONFIG_STRU,cfg3.devtype));

       Config_SaveData(CONFIG_PARAMETER_PAGE,&Config_buff[0],PARAMETER_SIZE);
   }

}

void Disp_Us_SetSaveConfig(void)
{
   Disp_CommSetSaveConfig();

   // for US specific settings
   if (Display.bit1SetTime)
   {
       Display.bit1SetTime = FALSE;

       RTC_Set(stimer4Us.w_year,stimer4Us.w_month,stimer4Us.w_date,stimer4Us.hour,stimer4Us.min,stimer4Us.sec);
   }

   {
       Disp_WriteInfo();

       Disp_CheckConsumptiveMaterialState();
   }   
}


void Disp_SaveInfo(void)
{
   int chflag = FALSE;

   if (DEV_TYPE_V1 != Display.cfg.cfg3.devtype)
   {
       return;
   }
   
   if (0 == Display.ulCumulatedVolume
      && 0 == Display.ulCumulatedVuTime)
   {
       return; // nothing to be saved
   }

   if (Display.ulCumulatedVolume > 0)
   {
       uint32_t ulTmp = Display.ulCumulatedVolume/1000; // convert to litre

       if (ulTmp > 0)
       {
           Display.ulCumulatedVolume = Display.ulCumulatedVolume%1000; // the remains

           if (0XFFFF - Display.info.PACKLIFEL >= ulTmp)
           {
               Display.info.PACKLIFEL += ulTmp;
                
               chflag = TRUE;
           }

       }

       // personal volume
       if (Display.bit1AuthState)
       {
           Display.UsrInfo.data += ulTmp;//Display.ulCumulatedVolume;
       }
       
   }

   if (Display.ulCumulatedVuTime > 0)
   {
       uint32_t ulTmp = Display.ulCumulatedVuTime/UV_PFM_PEROID; // convert to 10minute
   
       if (ulTmp > 0)
       {
           Display.ulCumulatedVuTime = Display.ulCumulatedVuTime%UV_PFM_PEROID; // the remains

           if (0XFFFF - Display.info.UVLIFEHOUR >= ulTmp)
           {
               Display.info.UVLIFEHOUR += ulTmp;
                
               chflag = TRUE;
           }
       }
   }

   if (chflag)
   {
       Disp_WriteInfo();
   }
   
}

void Disp_ResetInfo(void) //用于清除用水量20161018
{


   if (DEV_TYPE_V1 != Display.cfg.cfg3.devtype)
   {
       return;
   }

      
   if (Display.bit1AuthState)
   {
       //Display.UsrInfo.data += ulTmp;//Display.ulCumulatedVolume;
       Display.UsrInfo.data = 0; //用水量清零
   }
 
}

void Disp_KeyHandler_us_idle(int key,int state)
{
    switch(key)
    {
    case KEY_CODE_SET_K4:
        if (KEYSTATE_PRESSED == state)
        {
            Disp_Us_Set(key);
        }
        break;
    case KEY_CODE_RECURSIVE_K2:
        {
            if (KEYSTATE_PRESSED == state)
            {
                uint8_t ls = keyboard_get_linestate();
        
                if (ls == ((1 << KEY_CODE_RECURSIVE_K2)|((1 << KEY_CODE_QUANTITY_K3))))
                {
                    // mark key as used
                    Display.KeyState |=  (1 << KEY_CODE_QUANTITY_K3) ;

                    Disp_Us_SetSaveConfig();
                    Disp_PrepareMove2Idle();
                }
                else
                {
                    Disp_Us_Set(key);
                }
            }
        }
        break;
    case KEY_CODE_QUANTITY_K3:
        {
            if (KEYSTATE_PRESSED == state)
            {
                uint8_t ls = keyboard_get_linestate();
    
                if (ls == ((1 << KEY_CODE_RECURSIVE_K2)|((1 << KEY_CODE_QUANTITY_K3))))
                {
                    // mark key as used
                    Display.KeyState |=  (1 << KEY_CODE_RECURSIVE_K2) ;
                    
                    Disp_Us_SetSaveConfig();
                    
                    Disp_PrepareMove2Idle();
                }
                else
                {
                   // move to select next line
                   Disp_Us_ChangeSel();
                }   
            }
        }
        break;        
    default:
        break;
    }

}


void Disp_KeyHandler_us(int key,int state)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
        Disp_KeyHandler_us_idle(key,state);
        break;
    default:
        break;
    }

}

void Disp_Es_ChangeSel(void)
{
//20170516
    int xoff = 10;
   // int yoff = 15;
    int yoff = 8;

    int item;

    // calc Move Display Page
    if (Display.ucCurLine + 1 + Display.ucLineOffset >= DISP_ES_ITEM_NUM)
    {
        Disp_EngSetInitPage(0,0XFF,0);
        return;
    }

    if (Display.ucCurLine + 1 >= DISPLAY_MAX_LINE_PER_PAGE)
    {
        Display.ucLineOffset += DISPLAY_MAX_LINE_PER_PAGE;
        Disp_EngSetInitPage(0,0XFF,Display.ucLineOffset);
        return;
    }

    //yoff = 15 + 24*Display.ucCurLine;
	yoff = 8 + 24*Display.ucCurLine;
    Display.ucCurCol = 0XFF;

    item = Display.aucSetLine[Display.ucCurLine];

    switch(item)
    {
    case DISP_ES_ITEM_CELL_CONSTANT1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_CELL_CONSTANT2:
#endif  
#ifdef PRO     
    case DISP_ES_ITEM_CELL_CONSTANT2:
#endif 

#ifdef TOC        
    case DISP_ES_ITEM_CELL_CONSTANT3:
#endif        
        Disp_engSetDrawCellConstant((item-DISP_ES_ITEM_CELL_CONSTANT1)/2,xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_TEMP_CONSTANT1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_TEMP_CONSTANT2:
#endif
#ifdef PRO        
    case DISP_ES_ITEM_TEMP_CONSTANT2:
#endif 
#ifdef TOC        
    case DISP_ES_ITEM_TEMP_CONSTANT3:
#endif        
        Disp_engSetDrawTempConstant((item-DISP_ES_ITEM_TEMP_CONSTANT1)/2,xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_TEMP_RANGE1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_TEMP_RANGE2:
#endif
#ifdef PRO       
    case DISP_ES_ITEM_TEMP_RANGE2:
#endif
        Disp_engSetDrawTempRange(item-DISP_ES_ITEM_TEMP_RANGE1,xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_PRODUCT_RS1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_PRODUCT_RS2:
#endif   
#ifdef PRO        
    case DISP_ES_ITEM_PRODUCT_RS2:
#endif 
        Disp_engSetDrawProductRs(item-DISP_ES_ITEM_PRODUCT_RS1,xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_PACK:
        Disp_engSetDrawPack(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_FILTER:
        Disp_engSetDrawFilter(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_UV:
        Disp_engSetDrawUV(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_RECIRCULATION:
        Disp_engSetDrawRecirculation(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_UF_DISINFECTION:
        Disp_engSetDrawUFDisinfection(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_UF_FLUSH:
        Disp_engSetDrawUFFlush(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_UF_AUTO_FLUSH:
        Disp_engSetDrawUFAutoFlush(xoff,yoff,Display.usForColor,Display.usBackColor4Set);
        break;
#endif        
    }

    Display.ucCurLine = (Display.ucCurLine + 1) % Display.ucTotalLine;

   // yoff = 15 + 24*Display.ucCurLine;
    yoff = 8 + 24*Display.ucCurLine;
    item = Display.aucSetLine[Display.ucCurLine];

    switch(item)
    {
    case DISP_ES_ITEM_CELL_CONSTANT1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_CELL_CONSTANT2:
#endif 
#ifdef PRO        
    case DISP_ES_ITEM_CELL_CONSTANT2:
#endif
#ifdef TOC        
    case DISP_ES_ITEM_CELL_CONSTANT3:
#endif        
        Disp_engSetDrawCellConstant((item-DISP_ES_ITEM_CELL_CONSTANT1)/2,xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_TEMP_CONSTANT1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_TEMP_CONSTANT2:
#endif  
#ifdef PRO     
    case DISP_ES_ITEM_TEMP_CONSTANT2:
#endif
#ifdef TOC        
    case DISP_ES_ITEM_TEMP_CONSTANT3:
#endif        
        Disp_engSetDrawTempConstant((item-DISP_ES_ITEM_TEMP_CONSTANT1)/2,xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_TEMP_RANGE1:
        
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_TEMP_RANGE2:
#endif 
#ifdef PRO        
    case DISP_ES_ITEM_TEMP_RANGE2:
#endif
        Disp_engSetDrawTempRange(item-DISP_ES_ITEM_TEMP_RANGE1,xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_PRODUCT_RS1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_PRODUCT_RS2:
#endif 
#ifdef PRO      
    case DISP_ES_ITEM_PRODUCT_RS2:
#endif
        Disp_engSetDrawProductRs(item-DISP_ES_ITEM_PRODUCT_RS1,xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_PACK:
        Disp_engSetDrawPack(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_FILTER:
        Disp_engSetDrawFilter(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_UV:
        Disp_engSetDrawUV(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_RECIRCULATION:
        Disp_engSetDrawRecirculation(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_UF_DISINFECTION:
        Disp_engSetDrawUFDisinfection(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_UF_FLUSH:
        Disp_engSetDrawUFFlush(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_UF_AUTO_FLUSH:
        Disp_engSetDrawUFAutoFlush(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#endif        
    }

}

void Disp_EngSetSetCellConstant(int ichl,int key)
{
    char buf[16];

    // us key2 to navigage
    if (KEY_CODE_RECURSIVE_K2 == key)
    {
        Display.ucCurCol = ((uint8_t)(Display.ucCurCol + 1)) % CELLCONST_SET_NUMS;
        return ;
    }
    
    // change value
    if (Display.ucCurCol < CELLCONST_SET_NUMS)
    {
        uint8_t pos[CELLCONST_SET_NUMS] = CELLCONST_SET_POS;
    
        Disp_Int2floatFormat(Display.cfg.cfg1.CELLCONSTANT[ichl],1,3,(char *)buf);
        
        if (buf[pos[Display.ucCurCol]] >= '9' )
        {
            buf[pos[Display.ucCurCol]] = '0';
        }
        else
        {
            buf[pos[Display.ucCurCol]] += 1;
        }   

        {
            UINT32 fvalue;
            
            Disp_atof((char *)buf,&fvalue,1000);

            Display.cfg.cfg1.CELLCONSTANT[ichl]  = fvalue;
            
        }
    }

}

void Disp_EngSetSetTempConstant(int iChl,int key)
{
    char buf[16];

    // us key2 to navigage
    if (KEY_CODE_RECURSIVE_K2 == key)
    {
        Display.ucCurCol = ((uint8_t)(Display.ucCurCol + 1)) % TEMPCONST_SET_NUMS;
        return ;
    }
    
    // change value
    if (Display.ucCurCol < TEMPCONST_SET_NUMS)
    {
        uint8_t pos[TEMPCONST_SET_NUMS] = TEMPCONST_SET_POS;
    
        Disp_Int2floatFormat(Display.cfg.cfg1.TEMPCONSTANT[iChl],1,3,(char *)buf);
        
        if (buf[pos[Display.ucCurCol]] >= '9' )
        {
            buf[pos[Display.ucCurCol]] = '0';
        }
        else
        {
            buf[pos[Display.ucCurCol]] += 1;
        }   

        {
            UINT32 fvalue;
            
            Disp_atof((char *)buf,&fvalue,1000);
                    
            Display.cfg.cfg1.TEMPCONSTANT[iChl]  = fvalue;
        }
    }

}

void Disp_EngSetSetTempRange(int chl,int key)
{

    // us key2 to navigage
    if (KEY_CODE_RECURSIVE_K2 == key)
    {
        Display.ucCurCol = ((uint8_t)(Display.ucCurCol + 1)) % TEMP_RANGE_SET_NUMS;
        return ;
    }
    
    // change value
    if (Display.ucCurCol < TEMP_RANGE_SET_NUMS)
    {
        uint8_t pos[TEMP_RANGE_SET_NUMS] = TEMP_RANGE_SET_POS;
    
        sprintf((char *)Config_buff,TEMP_RANGE_FORMAT,Display.cfg.cfg2.TEMP_MAX[chl],Display.cfg.cfg2.TEMP_MIN[chl]);
        
        if (Config_buff[pos[Display.ucCurCol]] >= '9' )
        {
            Config_buff[pos[Display.ucCurCol]] = '0';
        }
        else
        {
          Config_buff[pos[Display.ucCurCol]] += 1;
        }   

        {
          int max,min;
          sscanf((char *)Config_buff,TEMP_RANGE_FORMAT,&max,&min);
          Display.cfg.cfg2.TEMP_MAX[chl]  = max;
          Display.cfg.cfg2.TEMP_MIN[chl]  = min;
        }
    }

}

void Disp_EngSetSetProductRs(int chl,int key)
{
    if (KEY_CODE_SET_K4 == key)
    {

        if (Display.cfg.cfg2.PROD_RES[chl] > 0)
        {
            Display.cfg.cfg2.PROD_RES[chl]--;
        }
       
    }
    else
    {
       if(chl == 0)
       {
        if (Display.cfg.cfg2.PROD_RES[chl] < 182)
        {
            Display.cfg.cfg2.PROD_RES[chl]++;
        }
       }
	   else if(chl == 1)
	   	{
	   	    Display.cfg.cfg2.PROD_RES[chl]++;
	   	}
    }

}

void Disp_EngSetSetPack(int key)
{
    // us key2 to navigage
    if (KEY_CODE_RECURSIVE_K2 == key)
    {
        Display.ucCurCol = ((uint8_t)(Display.ucCurCol + 1)) % PACK_SET_NUMS;
        return ;
    }
    
    // change value
    if (Display.ucCurCol < PACK_SET_NUMS)
    {
        uint8_t pos[PACK_SET_NUMS] = PACK_SET_POS;
        
        switch(Display.cfg.cfg1.language)
        {
        default:
            sprintf((char *)Config_buff,PACK_FORMAT_CHN,Display.cfg.cfg2.PACKLIFEDAY,Display.cfg.cfg2.PACKLIFEL);
            break;
        case DISP_LAN_ENGLISH:
            sprintf((char *)Config_buff,PACK_FORMAT_ENG,Display.cfg.cfg2.PACKLIFEDAY,Display.cfg.cfg2.PACKLIFEL);
            break;
        case DISP_LAN_GERMANY:
            sprintf((char *)Config_buff,PACK_FORMAT_DUT,Display.cfg.cfg2.PACKLIFEDAY,Display.cfg.cfg2.PACKLIFEL);
            break;
        }    
    
        
        if (Config_buff[pos[Display.ucCurCol]] >= '9' )
        {
            Config_buff[pos[Display.ucCurCol]] = '0';
        }
        else
        {
            Config_buff[pos[Display.ucCurCol]] += 1;
        }   

        {
          int max,min;
          switch(Display.cfg.cfg1.language)
          {
          default:
              sscanf((char *)Config_buff,PACK_FORMAT_CHN,&max,&min);
              break;
          case DISP_LAN_ENGLISH:
              sscanf((char *)Config_buff,PACK_FORMAT_ENG,&max,&min);
              break;
          case DISP_LAN_GERMANY:
              sscanf((char *)Config_buff,PACK_FORMAT_DUT,&max,&min);
              break;
          }    
          
          Display.cfg.cfg2.PACKLIFEDAY  = max;
          Display.cfg.cfg2.PACKLIFEL  = min;
        }
    }

}

void Disp_EngSetSetFilter(int key)
{
    if (KEY_CODE_SET_K4 == key)
    {
        if (Display.cfg.cfg2.FILTERLIFE > 0)
        {
            Display.cfg.cfg2.FILTERLIFE--;
        }
    }
    else
    {
        if (Display.cfg.cfg2.FILTERLIFE < 9999)
        {
            Display.cfg.cfg2.FILTERLIFE++;
        }
    }

}

void Disp_EngSetSetUV(int key)
{
    // us key2 to navigage
    if (KEY_CODE_RECURSIVE_K2 == key)
    {
        Display.ucCurCol = ((uint8_t)(Display.ucCurCol + 1)) % UV_SET_NUMS;
        return ;
    }
    
    // change value
    if (Display.ucCurCol < UV_SET_NUMS)
    {
        uint8_t pos[UV_SET_NUMS] = UV_SET_POS;
        
        if (DISP_LAN_CHINESE == Display.cfg.cfg1.language)
        {
            uint8_t ucPos;
            for (ucPos = 3; ucPos < UV_SET_NUMS; ucPos++)
            {
                pos[ucPos] -= 2;
            }
        }
    
        switch(Display.cfg.cfg1.language)
        {
        default:
            sprintf((char *)Config_buff,UV_FORMAT_CHN,Display.cfg.cfg2.UVLIFEDAY,Display.cfg.cfg2.UVLIFEHOUR);
            break;
        case DISP_LAN_ENGLISH:
            sprintf((char *)Config_buff,UV_FORMAT_ENG,Display.cfg.cfg2.UVLIFEDAY,Display.cfg.cfg2.UVLIFEHOUR);
            break;
        case DISP_LAN_GERMANY:
            sprintf((char *)Config_buff,UV_FORMAT_DUT,Display.cfg.cfg2.UVLIFEDAY,Display.cfg.cfg2.UVLIFEHOUR);
            break;
        }
        
        if (Config_buff[pos[Display.ucCurCol]] >= '9' )
        {
            Config_buff[pos[Display.ucCurCol]] = '0';
        }
        else
        {
          Config_buff[pos[Display.ucCurCol]] += 1;
        }   

        {
          int max,min;
          switch(Display.cfg.cfg1.language)
          {
          default:
              sscanf((char *)Config_buff,UV_FORMAT_CHN,&max,&min);
              break;
          case DISP_LAN_ENGLISH:
              sscanf((char *)Config_buff,UV_FORMAT_ENG,&max,&min);
              break;
          case DISP_LAN_GERMANY:
              sscanf((char *)Config_buff,UV_FORMAT_DUT,&max,&min);
              break;
          }
          
          Display.cfg.cfg2.UVLIFEDAY  = max;
          Display.cfg.cfg2.UVLIFEHOUR  = min;
        }
    }

}


void Disp_EngSetSetRecirculationTime(int key)
{
    if (KEY_CODE_SET_K4 == key)
    {
        #ifdef TOC
        if (Display.cfg.cfg1.cirtime > 7)
        {
            Display.cfg.cfg1.cirtime--;
        }
		#else
		if (Display.cfg.cfg1.cirtime > 0)
        {
            Display.cfg.cfg1.cirtime--;
        }
		#endif
    }
    else
    {
        if (Display.cfg.cfg1.cirtime < 60)
        {
            Display.cfg.cfg1.cirtime++;
        }
    }

}


void Disp_EngSetUFInfection(int key)
{
    if (KEY_CODE_SET_K4 == key)
    {
        if (Display.cfg.cfg3.usUFCleanTime > 0)
        {
            Display.cfg.cfg3.usUFCleanTime--;
        }
    }
    else
    {
        if (Display.cfg.cfg3.usUFCleanTime < 360)
        {
            Display.cfg.cfg3.usUFCleanTime++;
        }
    }

}



void Disp_EngSetUFFlush(int key)
{
    if (KEY_CODE_SET_K4 == key)
    {
        if (Display.cfg.cfg3.usUFFlushTime > 0)
        {
            Display.cfg.cfg3.usUFFlushTime--;
        }
    }
    else
    {
        if (Display.cfg.cfg3.usUFFlushTime < 999)
        {
            Display.cfg.cfg3.usUFFlushTime++;
        }
    }

}

void Disp_EngSetUFAutoFlush(int key)
{
    if (KEY_CODE_SET_K4 == key)
    {
        if (Display.cfg.cfg3.usAutoUFFlushTime > 0)
        {
            Display.cfg.cfg3.usAutoUFFlushTime--;
        }
    }
    else
    {
        if (Display.cfg.cfg3.usAutoUFFlushTime < 999)
        {
            Display.cfg.cfg3.usAutoUFFlushTime++;
        }
    }

}


void Disp_Es_Set(int key)
{
//20170516
    int xoff = 10;
   // int yoff = 15;
    int yoff = 8;
	int item;

   // yoff = 15 + 24*Display.ucCurLine;
    yoff = 8 + 24*Display.ucCurLine;

      item = Display.aucSetLine[Display.ucCurLine];
    switch(item)
    {
    case DISP_ES_ITEM_CELL_CONSTANT1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_CELL_CONSTANT2:
#endif 
#ifdef PRO        
    case DISP_ES_ITEM_CELL_CONSTANT2:
#endif 
#ifdef TOC        
    case DISP_ES_ITEM_CELL_CONSTANT3:
#endif        
        Disp_EngSetSetCellConstant((item-DISP_ES_ITEM_CELL_CONSTANT1)/2,key);
        Disp_engSetDrawCellConstant((item-DISP_ES_ITEM_CELL_CONSTANT1)/2,xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_TEMP_CONSTANT1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_TEMP_CONSTANT2:
#endif
#ifdef PRO        
    case DISP_ES_ITEM_TEMP_CONSTANT2:
#endif
#ifdef TOC        
    case DISP_ES_ITEM_TEMP_CONSTANT3:
#endif        
        Disp_EngSetSetTempConstant((item-DISP_ES_ITEM_TEMP_CONSTANT1)/2,key);
        Disp_engSetDrawTempConstant((item-DISP_ES_ITEM_TEMP_CONSTANT1)/2,xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_TEMP_RANGE1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_TEMP_RANGE2:
#endif  
#ifdef PRO      
    case DISP_ES_ITEM_TEMP_RANGE2:
#endif 
        Disp_EngSetSetTempRange(item-DISP_ES_ITEM_TEMP_RANGE1,key);
        Disp_engSetDrawTempRange(item-DISP_ES_ITEM_TEMP_RANGE1,xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_PRODUCT_RS1:
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_PRODUCT_RS2:
#endif     
#ifdef PRO       
    case DISP_ES_ITEM_PRODUCT_RS2:
#endif
        Disp_EngSetSetProductRs(item-DISP_ES_ITEM_PRODUCT_RS1,key);
        Disp_engSetDrawProductRs(item-DISP_ES_ITEM_PRODUCT_RS1,xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_PACK:
        Disp_EngSetSetPack(key);
        Disp_engSetDrawPack(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_FILTER:
        Disp_EngSetSetFilter(key);
        Disp_engSetDrawFilter(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_UV:
        Disp_EngSetSetUV(key);
        Disp_engSetDrawUV(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_RECIRCULATION:
        Disp_EngSetSetRecirculationTime(key);
        Disp_engSetDrawRecirculation(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#ifdef UF_FUNCTION        
    case DISP_ES_ITEM_UF_DISINFECTION:
        Disp_EngSetUFInfection(key);
        Disp_engSetDrawUFDisinfection(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_UF_FLUSH:
        Disp_EngSetUFFlush(key);
        Disp_engSetDrawUFFlush(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
    case DISP_ES_ITEM_UF_AUTO_FLUSH:
        Disp_EngSetUFAutoFlush(key);
        Disp_engSetDrawUFAutoFlush(xoff,yoff,LIGHTGREEN,Display.usBackColor4Set);
        break;
#endif        
    }
}
void Disp_KeyHandler_es_idle(int key,int state)
{
    switch(key)
    {
    case KEY_CODE_SET_K4:
        {
            if (KEYSTATE_PRESSED == state)
            {
                uint8_t ls = keyboard_get_linestate();
        
                if (ls == ((1 << KEY_CODE_SET_K4)|((1 << KEY_CODE_QUANTITY_K3))))
                {
                    // mark key as used
                    Display.KeyState |=  (1 << KEY_CODE_QUANTITY_K3) ;

                    Disp_CommSetSaveConfig();
                    Disp_PrepareMove2Idle();
                }
                else
                {
                    Disp_Es_Set(key);
                }
            }
            
        }        
        break;
    case KEY_CODE_RECURSIVE_K2:
        {
            if (KEYSTATE_PRESSED == state)
            {
               Disp_Es_Set(key);
            }
        }
        break;
    case KEY_CODE_QUANTITY_K3:
        {
            if (KEYSTATE_PRESSED == state)
            {
                uint8_t ls = keyboard_get_linestate();
    
                if (ls == ((1 << KEY_CODE_SET_K4)|((1 << KEY_CODE_QUANTITY_K3))))
                {
                    // mark key as used
                    Display.KeyState |=  (1 << KEY_CODE_SET_K4) ;
                    Disp_CommSetSaveConfig();
                    Disp_PrepareMove2Idle();
                }
                else
                {
                    Disp_Es_ChangeSel();
                }   
            }
        }
        break;
    default:
        break;
    }

}

void Disp_KeyHandler_es(int key,int state)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
        Disp_KeyHandler_es_idle(key,state);
        break;

    default:
        break;        
    }

}
/*
void Disp_KeyHandler_idle(int key,int state)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
    case DISPLAY_SUB_PAGE_IDLE_FLUSH:
        Disp_KeyHandler_idle_idle(key,state);
        break;
    default:
        Disp_KeyHandler_idle_empty_tank(key,state);
        break;
    }

}*/
void Disp_KeyHandler_idle(int key,int state)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
		Disp_KeyHandler_idle_idle(key,state);
        break;
    case DISPLAY_SUB_PAGE_IDLE_FLUSH:
        Disp_KeyHandler_idle_UF_flush(key,state);
        break;
    default:
        Disp_KeyHandler_idle_empty_tank(key,state);
        break;
    }

}


void Disp_KeyHandler(int key,int state)
{

    if (Display.ucDspWelcomeTime > 0)
    {
        return; // for welcome display
    }

    Display.usIdleLcdTimes = 0;

    if (state == KEYSTATE_UNPRESS)
    {
        Display.KeyState &=  ~(1 << key) ;
    }
    else
    {
        // filter out used keys 
        if (Display.KeyState & (1 << key))
        {
            return;
        }

//20170516修改定量取水长按键增减
/*
        if (DISPLAY_PAGE_USER_SET != Display.curPage.ucMain
            && DISPLAY_PAGE_ENG_SET != Display.curPage.ucMain)
*/
        if (DISPLAY_PAGE_USER_SET != Display.curPage.ucMain
            && DISPLAY_PAGE_ENG_SET != Display.curPage.ucMain
            && DISPLAY_PAGE_QUANTITY_TAKING_WATER != Display.curPage.ucMain)
        {
            Display.KeyState |=  (1 << key) ;
        }
    }

    if (!LCD_GetBackLightState()
        && (state == KEYSTATE_PRESSED))
    {
        LCD_EnableBackLight(TRUE);
        return;
    }
	
	if(gusOperMode != 0)
	{
	    return;//keyboard disable
	}
#if (IWDG_SUPPORT > 0)
    IWDG_Feed();  
#endif

    switch(Display.curPage.ucMain)
    {
    case DISPLAY_PAGE_IDLE:
        Disp_KeyHandler_idle(key,state);
        break;
    case DISPLAY_PAGE_USER_SET:
        Disp_KeyHandler_us(key,state);
        break;
    case DISPLAY_PAGE_ENG_SET:
        Disp_KeyHandler_es(key,state);
        break;
    case DISPLAY_PAGE_CIRCULATION:
        Disp_KeyHandler_circular(key,state);
        break;
    case DISPLAY_PAGE_NORMAL_TAKING_WATER:
        Disp_KeyHandler_ntw(key,state);
        break;
    case DISPLAY_PAGE_QUANTITY_TAKING_WATER:
        Disp_KeyHandler_qtw(key,state);
        break;
    case DISPLAY_PAGE_DECOMPRESSION:
        Disp_KeyHandler_dec(key,state);
        break;
    case DISPLAY_PAGE_UF_CLEAN:
        Disp_KeyHandler_uf_disinfection(key,state);
        break;
    case DISPLAY_PAGE_TOC:
        Disp_KeyHandler_toc(key,state);        
        break;
    }
}

void Disp_Prepare4Qtw(void)
{   
    
    if (!Disp_GetTankState())
    {
        Display.tgtPage.ucMain = DISPLAY_PAGE_IDLE;
        Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_TANK_EMPTY;
        Display.TgtState.ucMain = DISPLAY_STATE_IDLE;
        Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;
        key_em = 1;
        Disp_Move2TankEmpty();
    }
    else
    {
        if (Display.usQtwWq > 0)
        {
            Display.tgtPage.ucMain = DISPLAY_PAGE_QUANTITY_TAKING_WATER;
            Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_TAKING;

            Display.TgtState.ucMain = DISPLAY_STATE_QUANTITY_TAKING_WATER;
            Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;
            
            Disp_Move2QtwTakingWater();

            // check & save qtw volume
            Disp_SaveQtwVolume();
        }
        else
        {
            Disp_Prepare4Circulation();
        }
    }
}

void Disp_RemoteHandler_idle_idle(int cmd,int data)
{
    // set keystate
    
    switch(cmd)
    {
    case RF_CMD_NTW: 
        {
            Disp_Prepare4Ntw();
        }
        break;
    case RF_CMD_CIRCULAION:
        {
            Disp_Prepare4Circulation();
        }
        break;
    case RF_CMD_QTW:
        {
            if (data <= DISP_MAX_WATER_VOLUME
                && data > 0 )
            {
                Display.usQtwWq = data;
                Disp_Prepare4Qtw();
            }
        }
        break;
    case RF_CMD_DEPRESS:
        {
            // transfer state to idle@dec
            Display.tgtPage.ucMain = DISPLAY_PAGE_DECOMPRESSION;
            Display.tgtPage.ucSub  = DISPLAY_SUB_PAGE_IDLE;
            Display.TgtState.ucMain = DISPLAY_STATE_DECOMPRESSION;
            Display.TgtState.ucSub  = DISPLAY_SUB_STATE_IDLE;
            Disp_Move2Decompression();
                    
        }
        break;
    }
}


void Disp_RemoteHandler_idle(int key,int data)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
    case DISPLAY_SUB_PAGE_IDLE_FLUSH:
        Disp_RemoteHandler_idle_idle(key,data);
        break;
    default:
        break;
    }

}


void Disp_RemoteHandler_cir_idle(int key,int data)
{
    switch(key)
    {
    case RF_CMD_NTW:
        {
            sys_untimeout(&Display.to4cir);
            Disp_Prepare4Ntw();
        }
        break;
    // 2014/10/30 add begin
    case RF_CMD_QTW:
        {
            sys_untimeout(&Display.to4cir);
            
            //Disp_PrepareMove2QtwSetting();
            if (data <= DISP_MAX_WATER_VOLUME
                && data > 0 )
            {
                Display.usQtwWq = data;
                Disp_Prepare4Qtw();
            }
            
        }
        break;
    // 2014/10/30 add end
    case RF_CMD_CIRCULAION:
        {
           sys_untimeout(&Display.to4cir);
           Disp_PrepareMove2Idle();
        }
        break;
    default:
        break;
    }

}


void Disp_RemoteHandler_circular(int key,int data)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
        Disp_RemoteHandler_cir_idle(key,data);
        break;
    case DISPALY_SUB_PAGE_TOC_FLUSH:
		Disp_RemoteHandler_cir_idle(key,data); //20160929
        break;
    case DISPALY_SUB_PAGE_TOC_OXIDATION:
		Disp_RemoteHandler_cir_idle(key,data);//20160929
        break;
    }

}

void Disp_RemoteHandler_toc(int key,int data)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
        Disp_RemoteHandler_cir_idle(key,data);
        break;
    case DISPALY_SUB_PAGE_TOC_FLUSH:
		Disp_RemoteHandler_cir_idle(key,data); //20160929
        break;
    case DISPALY_SUB_PAGE_TOC_OXIDATION:
		Disp_RemoteHandler_cir_idle(key,data);//20160929
        break;
    }

}

void Disp_RemoteHandler_ntw_idle(int key,int state)
{
    switch(key)
    {
    case RF_CMD_NTW:
        {
            Disp_StopNtwTakingWater();
            
            Disp_Prepare4Circulation();
        }
        break;
    default:
        break;
    }

}

void Disp_RemoteHandler_ntw(int key,int data)
{

    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
        Disp_RemoteHandler_ntw_idle(key,data);
        break;
    }

}


void Disp_RemoteHandler_qtw_tw(int key,int data)
{
    // currently empty
    switch(key)
    {
    case RF_CMD_NTW:
        {
            Disp_StopQtwTakingWater();

            // transfer state to idle@NTW
            Disp_Prepare4Ntw();
        }
        break;
    case RF_CMD_QTW:
        {
            Disp_StopQtwTakingWater();

            // transfer state to idle@cir
            Disp_Prepare4Circulation();
        }
        break;
    }
    
}


void Disp_RemoteHandler_qtw(int key,int data)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_TAKING:
        Disp_RemoteHandler_qtw_tw(key,data);
        break;
    default:
        break;
    }
}

void Disp_RemoteHandler_dec_idle(int key,int state)
{
    switch(key)
    {
    case RF_CMD_DEPRESS:
        {
            Disp_PrepareMove2Idle();
        }
        break;
    default:
        break;
    }

}


void Disp_RemoteHandler_dec(int key,int data)
{
    switch(Display.curPage.ucSub)
    {
    case DISPLAY_SUB_PAGE_IDLE:
        Disp_RemoteHandler_dec_idle(key,data);
        break;
    }

}


void Disp_RemoteHandler(int key,int data)
{

    if (Display.ucDspWelcomeTime > 0)
    {
        return; // for welcome display
    }

    Display.usIdleLcdTimes = 0;

    // activating LCD
    if (!LCD_GetBackLightState())
    {
        LCD_EnableBackLight(TRUE);
    }

#if (IWDG_SUPPORT > 0)
    IWDG_Feed();  
#endif

    switch(Display.curPage.ucMain)
    {
    case DISPLAY_PAGE_IDLE:
    case DISPLAY_PAGE_USER_SET:
    case DISPLAY_PAGE_ENG_SET:  
        Disp_RemoteHandler_idle(key,data);
        break;
    case DISPLAY_PAGE_CIRCULATION:
        Disp_RemoteHandler_circular(key,data);
        break;
    case DISPLAY_PAGE_NORMAL_TAKING_WATER:
        Disp_RemoteHandler_ntw(key,data);
        break;
    case DISPLAY_PAGE_QUANTITY_TAKING_WATER:
        Disp_RemoteHandler_qtw(key,data);
        break;
    case DISPLAY_PAGE_DECOMPRESSION:
        Disp_RemoteHandler_dec(key,data);
        break;
	case DISPLAY_PAGE_TOC:
		    Disp_RemoteHandler_toc(key,data);
    }
}

void Disp_ReadConfig(void)
{

    memcpy(&Display.cfg,&pParameter[PARAMETER_LOCAL_BEGIN_POS],PARAMETER_LOCAL_SIZE);

    if (INVALID_CONFIG_VALUE_LONG == Display.cfg.cfg1.cirtime)
    {
        //Display.cfg.cfg1.cirtime = 3; // default 3minute
       #ifdef TOC
        Display.cfg.cfg1.cirtime = 7; //20161102
       #else
	    Display.cfg.cfg1.cirtime = 3;
	   #endif
       
    }

    if (INVALID_CONFIG_VALUE_LONG == Display.cfg.cfg1.language)
    {
        Display.cfg.cfg1.language = DISP_LAN_ENGLISH;
    }


    {
        int iChl;
        for (iChl = 0; iChl < 3; iChl++)
        {
            if (INVALID_CONFIG_VALUE_LONG == Display.cfg.cfg1.CELLCONSTANT[iChl])
            {
                Display.cfg.cfg1.CELLCONSTANT[iChl] = 1000; // DECIMAL 1.000
                //20160818 Display.cfg.cfg1.CELLCONSTANT[iChl] = 1000
            }
            
            if (INVALID_CONFIG_VALUE_LONG == Display.cfg.cfg1.TEMPCONSTANT[iChl])
            {
                Display.cfg.cfg1.TEMPCONSTANT[iChl] = 1000; // DECIMAL 1.000
            }
        }
        
        for (iChl = 0; iChl < 2; iChl++)
        {
            if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.TEMP_MAX[iChl])
            {
                Display.cfg.cfg2.TEMP_MAX[iChl] = 55; // celsius
            }
        
            if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.TEMP_MIN[iChl])
            {
                Display.cfg.cfg2.TEMP_MIN[iChl] = 0; // celsius
            }
        
            if (INVALID_CONFIG_VALUE_LONG == Display.cfg.cfg2.PROD_RES[iChl])
            {
                Display.cfg.cfg2.PROD_RES[0] = 150; // DECIMAL 15.0
                Display.cfg.cfg2.PROD_RES[1] = 500; // DECIMAL 50.0
            }
        }
    }
    if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.PACKLIFEDAY)
    {
        Display.cfg.cfg2.PACKLIFEDAY = 360; 
    }
    if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.PACKLIFEL)
    {
        Display.cfg.cfg2.PACKLIFEL = 3500; // 
        //20160818   Display.cfg.cfg2.PACKLIFEL = 3000;
    }

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.FILTERLIFE)
    {
        Display.cfg.cfg2.FILTERLIFE = 360; // 
        // 20160818 Display.cfg.cfg2.FILTERLIFE = 180;
    }
    if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.UVLIFEDAY)
    {
        Display.cfg.cfg2.UVLIFEDAY = 720; // 
        //20160818 Display.cfg.cfg2.UVLIFEDAY = 360;
    }
    if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.UVLIFEHOUR)
    {
        Display.cfg.cfg2.UVLIFEHOUR = 8000; // 
    }

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.UNIT)
    {
        Display.cfg.cfg2.UNIT = 0; // M
    }
	if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.FEED_KEY)//进水电导显示开关
    {
        Display.cfg.cfg2.FEED_KEY = 1; // on
    }
	if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.TOC_SHOW)//TOC显示判断
    {
        Display.cfg.cfg2.TOC_SHOW = 0; // OFF
    }
	//每次读取配置将TOC设为不显示20170524
	Display.cfg.cfg2.TOC_SHOW = 0; // OFF
	Display.cfg.cfg2.TOC_FUN = 1;  //检测TOC

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg2.FLOW)
    {
        Display.cfg.cfg2.FLOW = 150; // 1.50 DECIMAL
    }

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg3.devtype)
    {
        Display.cfg.cfg3.devtype = DEV_TYPE_V1;
    }


    if (Display.cfg.cfg3.SERIALNO[0] == 0XFF)
    {
        sprintf((char *)Display.cfg.cfg3.SERIALNO,"NO SERIAL");
    }

    Display.cfg.cfg3.SERIALNO[15] = 0; // MUST BE ZERO TERMINATED
//	sprintf((char *)Display.cfg.cfg3.SERIALNO,"S7PW050912");
    // begin add for v2

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg3.usTocWashTime)
    {
        Display.cfg.cfg3.usTocWashTime = 160; // second
    }  

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg3.usTocOxiTime)
    {
        Display.cfg.cfg3.usTocOxiTime = 180; // second
    }     

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg3.usUFCleanTime)
    {
        Display.cfg.cfg3.usUFCleanTime = 30; // DAY
    }

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg3.usUFFlushTime)
    {
        Display.cfg.cfg3.usUFFlushTime = 30; // second
    }

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg3.usAutoUFFlushTime)
    {
        Display.cfg.cfg3.usAutoUFFlushTime = 30; // min
    }

    if (INVALID_CONFIG_VALUE == Display.cfg.cfg3.usIdleCirPeriod)
    {
        Display.cfg.cfg3.usIdleCirPeriod = 60; // minute
    }  
	
  
    Display.ulCalcirtime = Display.cfg.cfg1.cirtime*DISP_MINUTE2MS;
   
//2017-1-17 TOC:6min
#ifdef TOC
     Display.ulCalcirtime -= 1000*360; //355
     Display.ulCalcirtime /= 2;
#endif


  /*
#ifdef TOC
    if (Display.ulCalcirtime  > 1000*(Display.cfg.cfg3.usTocOxiTime + Display.cfg.cfg3.usUFCleanTime + 30))
    {
        Display.ulCalcirtime -= 1000*(Display.cfg.cfg3.usTocOxiTime + Display.cfg.cfg3.usUFCleanTime + 30);
    }

   else
    {
      // Display.ulCalcirtime = 1000*(Display.cfg.cfg3.usTocOxiTime + Display.cfg.cfg3.usUFCleanTime*2) + 3*DISP_MINUTE2MS;
	   Display.ulCalcirtime = 1000*(Display.cfg.cfg3.usTocOxiTime + Display.cfg.cfg3.usUFCleanTime+30);
    } 
#endif
*/

    
#ifdef DISP_UF_TEST
    Display.cfg.usUFCleanTime = 1; // DAY
#endif

    // end add for v2
    // more to add here
   
}

void Disp_InitConfig(void)
{
    char serial[16];

    memcpy(serial,Display.cfg.cfg3.SERIALNO,16);

    memset(&Display.cfg,0XFF,PARAMETER_LOCAL_SIZE);

    memcpy(Display.cfg.cfg3.SERIALNO,serial,16);

    IWDG_Feed();
    
    Config_SaveData(CONFIG_PARAMETER_PAGE,(UINT8 *)&Display.cfg,PARAMETER_SIZE);

    Disp_ReadConfig();
}


uint8_t Disp_GetNextAdcChl(void)
{
    static int iChl = 0;

    int iCurChl = iChl;

    int iLoop;

    for (iLoop = 0; iLoop < DISPLAY_MAX_SENSOR; iLoop++)
    {
        iChl = (iChl + 1) % (DISPLAY_MAX_SENSOR ); 

        if ((1 << iChl) & Display.ucChlMask)
        {
            break;
        }
    
    }

    return iCurChl;
}

#define SBL_REQ_ADDR_LSB             RPC_POS_DAT0
#define SBL_REQ_ADDR_MSB            (SBL_REQ_ADDR_LSB+1)
#define SBL_REQ_DAT0                (SBL_REQ_ADDR_MSB+1)
#define SBL_RSP_STATUS               RPC_POS_DAT0
#define SBL_RSP_ADDR_LSB            (SBL_RSP_STATUS+1)
#define SBL_RSP_ADDR_MSB            (SBL_RSP_ADDR_LSB+1)
#define SBL_RSP_DAT0                (SBL_RSP_ADDR_MSB+1)
#define SBL_READ_HDR_LEN            (SBL_RSP_DAT0 - SBL_RSP_STATUS)


void DispMakeQueryCmd(void)
{
    char cmd[64];

    sprintf(cmd,"AT+CONFIG=%s=?",AT_CONFIG_DEVNAME);

    sappTgtType = RPC_UART_SOF;

    Config_Sapp_Cmd(SAPP_CMD_AT,(uint8_t *)cmd,strlen(cmd));
}

void DispMakeConfigCmd(void)
{
    char cmd[64];

    sprintf(cmd,"AT+CONFIG=%s=%s",AT_CONFIG_DEVNAME,Display.cfg.cfg3.SERIALNO);

    sappTgtType = RPC_UART_SOF;

    Config_Sapp_Cmd(SAPP_CMD_AT,(uint8_t *)cmd,strlen(cmd));
}

void Disp_SecondTask(void)
{
    int sensor = FALSE;

    if (!Display.bit1CheckBleName)
    {
        /* check name */
        DispMakeQueryCmd();
    }
        
    // late implement, for periodical routine
    if (Display.ucDspWelcomeTime > 0)
    {
        Display.ucDspWelcomeTime--;
        if (0 == Display.ucDspWelcomeTime)
        {

            Disp_CheckConsumptiveMaterialState();
        
            Disp_DisplayIdlePage();
        }
#ifdef UF_FUNCTION
        if (Display.bit1InitPending4UF)
        {
            Disp_UpdateUFWashTime();
        }
#endif        
    }
    else
    {
        // read sensors periodcally
        Display.usSeconds++;

        if (DEV_TYPE_V1 == Display.cfg.cfg3.devtype)
        {
#define OFFSET_TIME 100                        
        
            //aDisplay.ucPeriodSensorMeas++;
    
            //if ((Display.ucPeriodSensorMeas % 10) == 0)
            //if (DISPLAY_STATE_TOC != Display.CurState.ucMain)
#if (DISP_ADC_METHOD_TYPE ==  DISP_ADC_METHOD_CONTINEOUS)       
            {
                int iChl = Disp_GetNextAdcChl();

                if (Display.ausSampleMask[iChl] & (1 << Display.CurState.ucMain))
                {
                    uint8_t ucPoolIdx = Disp_AllocAdcAction();    

                    if (ucPoolIdx < MAX_ADC_ACTIONS)
                    {
                        aAdcChl[ucPoolIdx].iValue = OFFSET_TIME + 100;
						//aAdcChl[ucPoolIdx].iValue = OFFSET_TIME + 1000;
                        aAdcChl[ucPoolIdx].ucChl = iChl;
                        aAdcChl[ucPoolIdx].ucPoolIdx = ucPoolIdx;
                        Disp_AddAdcChl(&aAdcChl[ucPoolIdx]);
                    }

                }

                sensor = TRUE;
            }
#elif (DISP_ADC_METHOD_TYPE ==  DISP_ADC_METHOD_SIMUTANEOUS)
           /* {
                int iChl = 0;
                for (iChl = 0; iChl < DISPLAY_SENSOR_NUM; iChl++)
                {
                    if (Display.ausSampleMask[iChl] & (1 << Display.CurState.ucMain))
                    {
                        uint8_t ucPoolIdx = Disp_AllocAdcAction();    
                    
                        if (ucPoolIdx < MAX_ADC_ACTIONS)
                        {
                            //aAdcChl[ucPoolIdx].iValue = OFFSET_TIME + (iChl+1)*100;
							aAdcChl[ucPoolIdx].iValue = OFFSET_TIME + (iChl+1)*400;
                            aAdcChl[ucPoolIdx].ucChl  = iChl;
                            aAdcChl[ucPoolIdx].ucPoolIdx = ucPoolIdx;
                            Disp_AddAdcChl(&aAdcChl[ucPoolIdx]);
                        }
                    
                    }
                
                }
                sensor = TRUE;
            }*/
          {
                static int iChl = 0;
                if (iChl < 2)
                {
                    if (Display.ausSampleMask[iChl] & (1 << Display.CurState.ucMain))
                    {
                        uint8_t ucPoolIdx = Disp_AllocAdcAction();    
                    
                        if (ucPoolIdx < MAX_ADC_ACTIONS)
                        {
                            //aAdcChl[ucPoolIdx].iValue = OFFSET_TIME + (iChl+1)*100;
							aAdcChl[ucPoolIdx].iValue = OFFSET_TIME + 100;
                            aAdcChl[ucPoolIdx].ucChl  = iChl;
                            aAdcChl[ucPoolIdx].ucPoolIdx = ucPoolIdx;
                            Disp_AddAdcChl(&aAdcChl[ucPoolIdx]);
                        }
                    
                    }
                   iChl++;
                }
				else if (iChl == 2)
                {
                    if (Display.ausSampleMask[iChl] & (1 << Display.CurState.ucMain))
                    {
                        uint8_t ucPoolIdx = Disp_AllocAdcAction();    
                    
                        if (ucPoolIdx < MAX_ADC_ACTIONS)
                        {
                            //aAdcChl[ucPoolIdx].iValue = OFFSET_TIME + (iChl+1)*100;
							aAdcChl[ucPoolIdx].iValue = OFFSET_TIME + 100;
                            aAdcChl[ucPoolIdx].ucChl  = iChl;
                            aAdcChl[ucPoolIdx].ucPoolIdx = ucPoolIdx;
                            Disp_AddAdcChl(&aAdcChl[ucPoolIdx]);
                        }
                    
                    }
                   iChl = 0;
                }
                sensor = TRUE;
				
            }
#endif
        }

        if (DISPLAY_STATE_IDLE == Display.CurState.ucMain)
        {
            Display.usIdleTimes++;

            if (DISPLAY_PAGE_IDLE == Display.curPage.ucMain
                && DISPLAY_SUB_PAGE_IDLE == Display.curPage.ucSub)
            {
                Disp_UpdateIdlePage(sensor);
            }

#ifdef UF_FUNCTION        
            if (DISPLAY_SUB_STATE_IDLE_FLUSH == Display.CurState.ucSub)
            {
                Display.ulIdleFlushCnt++;

                if (Display.ulIdleFlushCnt >= Display.cfg.cfg3.usUFFlushTime)
                {
                    Disp_PrepareMove2Idle();
					key_em = 0;

                }
                else
                {
                    Disp_DisplayFlushProgressInfo();
                }
            }
#endif
            Display.ulIdleCirTime++;
            if ((Display.ulIdleCirTime >= (Display.cfg.cfg3.usIdleCirPeriod * 60)))
            {
                if (DISPLAY_PAGE_IDLE == Display.curPage.ucMain
                    && DISPLAY_SUB_PAGE_IDLE == Display.curPage.ucSub)
                {
                    Disp_Prepare4Circulation();
                }
            }

#ifdef UF_FUNCTION        
            Display.ulIdleFlushTime++;

            if (Display.ulIdleFlushTime >= (Display.cfg.cfg3.usAutoUFFlushTime*60))
            {
                if (DISPLAY_PAGE_IDLE == Display.curPage.ucMain
                    && DISPLAY_SUB_PAGE_IDLE == Display.curPage.ucSub)
                {
                    Disp_PrepareMove2IdleFlush();
                }
            }
#endif            

            Display.usIdleLcdTimes++;

            if (Display.usIdleLcdTimes >= DISP_LCD_DORMANT_TIME)
            {
                //LCD_EnableBackLight(FALSE);
								LCD_EnableBackLight(TRUE);
                Display.usIdleLcdTimes = 0;
            }
        }     

        if ((Display.usSeconds % 60) == 0)
        {
            // begin modify for V2: remark precompiler: SHOW_TIME 
//#ifdef SHOW_TIME        
            RTC_Get(); // update wall time
//#endif
            // end modify for V2
            //if ((Display.usSeconds % 600) == 0)
            {
                Disp_SaveInfo();
            }

            Disp_CheckConsumptiveMaterialState();

            // add for v2
            if (Display.bit1Today)
            {
                if (Display.bit1Measured)
                {
                    Display.bit1Today = 0;
                    // save data to storage,late implement
                    Storage_WriteMeasInfo(&Display,&sLastUpdateTime);
                }
                
            }
            if (timer.w_date != sLastUpdateTime.w_date)
            {

                Storage_WriteRunTimeInfo(&Display);

                // for next measurement
                if (Display.bit1Measured)
                {
                    Storage_WriteMeasInfo(&Display,&sLastUpdateTime);

                    // clear for next day
                    Display.bit1Measured = 0;
                }

                sLastUpdateTime = timer;
            }

            Disp_CheckUFClean();            
            // end for V2
            
        }

        
        Disp_CheckTankState();

        Disp_ShowCheckState(SHOW_COORD_X,SHOW_COORD_Y);
        
    }
}

void Disp_DisplayWelcomePage(void)
{

    LCD_Clear(Display.usBackColor);

    // display text info
    curFont->DrawText(10,40,Display.cfg.cfg3.SERIALNO,Display.usForColor,Display.usBackColor);
    
    Disp_ShowTime();
}


void Disp_Invalidate(void)
{
    uint16_t color[] = {WHITE,YELLOW,RED,YELLOW};
#if (IWDG_SUPPORT > 0)
     IWDG_Feed();  
#endif
    
    Display.usBackColor = color[Display.bit3ShowAlarm];

    switch(Display.curPage.ucMain)
    {
    case DISPLAY_PAGE_IDLE:
        switch(Display.curPage.ucSub)
        {
        case DISPLAY_SUB_PAGE_IDLE:
            Disp_DisplayIdlePage();
            break;
        }
        break;
    case DISPLAY_PAGE_CIRCULATION:
        Disp_CirculationInitPage();
        break;
#ifdef TOC
	case DISPLAY_PAGE_TOC:
		{
			if(Display.curPage.ucSub == DISPALY_SUB_PAGE_TOC_FLUSH)
			{
				Disp_TOCFlushPage();
				break;
			}
			else if(Display.curPage.ucSub == DISPALY_SUB_PAGE_TOC_OXIDATION)
			{
				Disp_TOCOxidatePage();
				break;
			}
			else
			{
				break;
			}
				
		}
#endif
/*
	case DISPLAY_PAGE_USER_SET:
		Disp_UserSetInitPage(0,0XFF,0);
		break;
	case DISPLAY_PAGE_ENG_SET:
		Disp_EngSetInitPage(0,0XFF,0);
		break;	*/
    case DISPLAY_PAGE_NORMAL_TAKING_WATER:
        Disp_NtwInitPage();
        break;
    case DISPLAY_PAGE_QUANTITY_TAKING_WATER:
        switch(Display.curPage.ucSub)
        {
        case DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_TAKING:
            Disp_QtwTakingWaterInitPage();
            break;
        case DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_SETTING:
            Disp_QtwSettingInitPage();
            break;
        }
        break;
    }
}


uint8_t Disp_AllocAdcAction(void)
{
    
    uint8_t CurrAdcIndex = MAX_ADC_ACTIONS;

    if (MAX_ADC_ACTIONS > Display.FreePoolIdx)
    {
        /* save the current obj index */
        CurrAdcIndex = Display.FreePoolIdx;
        
        /* set the new free index */
        if (MAX_ADC_ACTIONS > aAdcPool[CurrAdcIndex].NextIndex)
        {
            Display.FreePoolIdx = aAdcPool[CurrAdcIndex].NextIndex; // ylf: pointer to next slot 
        }
        else
        {
            /* No further free actions available */
            Display.FreePoolIdx = MAX_ADC_ACTIONS;
        }

    }
    return CurrAdcIndex;
}

void Disp_ReleaseAdcAction(uint8_t ObjIdx)
{
    aAdcPool[ObjIdx].NextIndex = Display.FreePoolIdx;
    Display.FreePoolIdx = ObjIdx;
}


void Disp_RmvAdcChl(DISPLAY_ADC_CHL_STRU *pAdcChl)
{
    if (adcChlHeader.next == NULL)
    {
        return ;
    }
    
    {
        DISPLAY_ADC_CHL_STRU *prev_t, *t;
        
        for (t = adcChlHeader.next, prev_t = NULL; t != NULL; prev_t = t, t = t->next) {
          if ((t == pAdcChl)) {
            /* We have a match */
            /* Unlink from previous in list */
            if (prev_t == NULL) {
              adcChlHeader.next = t->next;
            } else {
              prev_t->next = t->next;
            }
            /* If not the last one, add value of this one back to next */
            if (t->next != NULL) {
              t->next->iValue += t->iValue;
            }
            return;
          }
        }
    }
    
}

uint8_t Disp_CheckAdcChl(DISPLAY_ADC_CHL_STRU *pAdcChl)
{
    DISPLAY_ADC_CHL_STRU *t;
    
    for (t = adcChlHeader.next; t != NULL; t = t->next) {
      if ((t == pAdcChl)) {
        return TRUE;
      }
    }
    return FALSE;
    
}


void Disp_AddAdcChl(DISPLAY_ADC_CHL_STRU *pAdcChl)
{
    if (adcChlHeader.next == NULL)
    {
        adcChlHeader.next = pAdcChl;
        goto end ;
    }

    // validation
    if (Disp_CheckAdcChl(pAdcChl))
    {
        return;
    }

    if (adcChlHeader.next->iValue > pAdcChl->iValue) 
    {
      // put at head position!
      adcChlHeader.next->iValue -= pAdcChl->iValue;
      pAdcChl->next = adcChlHeader.next;
      adcChlHeader.next = pAdcChl;
    }
    else
    {
    
      DISPLAY_ADC_CHL_STRU *t;
      for(t = adcChlHeader.next; t != NULL; t = t->next) {
        pAdcChl->iValue -= t->iValue;
        if (t->next == NULL || t->next->iValue > pAdcChl->iValue) {
          if (t->next != NULL) {
            t->next->iValue -= pAdcChl->iValue;
          }
          pAdcChl->next = t->next;
          t->next = pAdcChl;
          break;
        }
      }
    }
end:
    Disp_Select_Sensor(1 << adcChlHeader.next->ucChl);
        
}

//20170911添加此函数动态设置循环时间
void Disp_Set_Cirtime(void)
{
    Display.ulCalcirtime = Display.cfg.cfg1.cirtime*DISP_MINUTE2MS;
   
//2017-1-17 TOC:6min
#ifdef TOC
	if(1 == Display.cfg.cfg2.TOC_FUN)
	{
		Display.ulCalcirtime -= 1000*360; //355
    	Display.ulCalcirtime /= 2;
	}
    else
    {
    	Display.ulCalcirtime = 1000 * 180; //
    }
#endif
}

void Disp_Adc_proc(void)
{
  //unsigned int time_needed;
  DISPLAY_ADC_CHLS_STRU *adcs;
  DISPLAY_ADC_CHL_STRU *adc;

  adcs = &adcChlHeader;
  adc = adcs->next;

  if (adc)
  {
      DECREASE_VALUE(adc->iValue); 
      if (adc->iValue > 0)
      {
          return;
      }
	  else
      {  
          int value1;
          int value2;
          int iChl  ;
#ifdef TOC
		  static int count_b, count_p; //用于计数
          static int temp_sum_b, temp_sum_p;
          static int temp_average_b, temp_average_p;
		  static double value_B25;
		  static int value_error;
#endif 
          // remove adc & release pool index
          adcs->next = adc->next;

          adc->next = NULL;

          iChl = adc->ucChl;

          Disp_ReleaseAdcAction(adc->ucPoolIdx);

          // perform adc convert

          Disp_Select_Sensor(1<<adc->ucChl); 
		 
          value1 = Disp_get_conduct();
  
          value1 = value1*100/4096*25/4096;
		 
          value2 = Disp_get_temperature();
  
          value2 = value2*100/4096*25/4096;
		  #ifdef TOC    
		  if(DISPLAY_SENSOR_TOC==iChl)
		  {
		      /*--------数据采集毫伏值异常报警------*/  
			  if(value1>2450)
			  {
			      value_error = 1;
			  } 
			  else 
			  {
			      value_error = 0;
			  }
			  
			  {	 
		          if(Display.spec.toc.ucIdx == 2)
		          {
		              temp_sum_b += value1;
				      count_b++;  
		          }
			      if(Display.spec.toc.ucIdx == 3 && temp_sum_b>0)
			      {
			          temp_average_b = temp_sum_b/count_b;
					  if(temp_average_b != 0&&count_b!=0)
					  {
					      value_B25 = Disp_Calc_TOC_BASE(adc->ucChl,temp_average_b,value2);	  
						  //基线采样小于50mv时，不显示TOC --20170118
					      if(temp_average_b <= 50)
					      {
					           Display.cfg.cfg2.TOC_SHOW = 0;
							   Display.cfg.cfg2.TOC_FUN= 0;
					      }
						  else
						  {
						       Display.cfg.cfg2.TOC_SHOW = 1;
							   Display.cfg.cfg2.TOC_FUN= 1;
						  } 
					  }	  
				      /*--------基线异常报警--------*/ 
					  if((value_B25>300 || value_error == 1) 
					  	  && (1 == Display.cfg.cfg2.TOC_SHOW))
				      {
				          if(value_B25>300)
				          {
		                      //"CHECK U PACK"
				              Disp_SetAlarm(DISP_ALARM_TOC_B25,TRUE);
							  TOC_Exception = 1;
				          }
				          else
				          {
				              //CHECK TOC SERNSOR
				              Disp_SetAlarm(DISP_ALARM_TOC_VALUE,TRUE);
							  TOC_Exception = 1;
				          }
				      }
				      else 
				      {  
				          Disp_SetAlarm(DISP_ALARM_TOC_VALUE,FALSE);
				          Disp_SetAlarm(DISP_ALARM_TOC_B25,FALSE);
						  TOC_Exception = 0;
				      }
					  
				      {  
				          count_p++;
				         //采10个样，取二次冲洗第15到第24个数据 
				          if(count_p>14&&count_p<=24)  //30~40
				          {
				 	          temp_sum_p += value1;
				          }
				          if(count_p==25)
				          {
				 	          temp_average_p = (temp_sum_p+5)/10;			          
					          //if(temp_average_b != 0 && temp_average_p != 0)
					          {
			                      Disp_Calc_TOC_PEAK(adc->ucChl,temp_average_p,value2,temp_average_b);		  
								  //基线采样小于50mv时，不显示TOC --20170118
					              if(temp_average_p <= 50)
					              {
					                  Display.cfg.cfg2.TOC_SHOW = 0;
									  Display.cfg.cfg2.TOC_FUN= 0;
					              }
						          else
						          {
						              Display.cfg.cfg2.TOC_SHOW = 1;
									  Display.cfg.cfg2.TOC_FUN= 1;
									  /*--------氧化P值与基线B值得比小于1.3报警--------*/
					                  if((((temp_average_p*1.0)/(temp_average_b*1.0))<1.3))
					                  {
					                      //CHECK UV LAMP
					                      Disp_SetAlarm(DISP_ALARM_TOC_PB,TRUE); 
					             	      TOC_Exception = 1;
					                  }
								      else
								      {
								          Disp_SetAlarm(DISP_ALARM_TOC_PB,FALSE); 
								          TOC_Exception = 0;
								      }
						          }
				          
					          }				
					           count_b=0; temp_sum_b=0; temp_average_b=0;
			                   count_p=0;temp_sum_p=0;temp_average_p=0;
                          }
				      }
			      }
		      }
		  }
		  // chl=0 | chl=1
		  else
		  {
		      Disp_Calc_Conductivity(adc->ucChl,value1,value2);
		  }
		  
		  #else 
		      Disp_Calc_Conductivity(adc->ucChl,value1,value2);	  
		  #endif
	
          switch(iChl)
          {
          case DISPLAY_SENSOR_1:
          case DISPLAY_SENSOR_2:
              {
                    
                int flags = 0;

                flags |= (1 << Display.CurState.ucMain);
                
                if (flags & ((1 << DISPLAY_STATE_NORMAL_TAKING_WATER) 
                           | (1 << DISPLAY_STATE_QUANTITY_TAKING_WATER)))
                {
                    if (!Display.bit1Measured)
                    {
                         if (Display.iConductivity[iChl] >= Display.aulMaxRes4Tw[iChl])
                         {
                             Display.aulMaxRes4Tw[iChl] = Display.iConductivity[iChl];
                             Display.aulMaxTemp4Tw[iChl] = Display.iTemperature[iChl];
                         }
                    }
                }  
                
                
                if (flags & ((1 << DISPLAY_STATE_CIRCULATION) 
                             | (1 << DISPLAY_STATE_NORMAL_TAKING_WATER)
                             | (1 << DISPLAY_STATE_QUANTITY_TAKING_WATER)
                             | (1 << DISPLAY_STATE_TOC)
                             | (1 << DISPLAY_STATE_IDLE)))//0927
                {
                    {
                        if (flags &((1 << DISPLAY_STATE_CIRCULATION)
							 |(1 << DISPLAY_STATE_TOC)))  //20170626修改进水电导显示()
						  // |(1 << DISPLAY_STATE_QUANTITY_TAKING_WATER)
                        {
                           Disp_Sensors(SENSOR_COORD_X,SENSOR_COORD_Y,1);
               
                        }
						else if( key_em == 1)
						{
						   Disp_Sensors(SENSOR_COORD_X,SENSOR_COORD_Y,0);
						}
                        else
                        {
                           Disp_Sensors(SENSOR_COORD_X,SENSOR_COORD_Y,3);
                        }
                    }

                    // check alarm
                    if (DISPLAY_SENSOR_1 == iChl)
                    {
                        if ((Display.iConductivity[DISPLAY_SENSOR_1]/100) <= Display.cfg.cfg2.PROD_RES[0])
                        {
                            Disp_SetAlarm(DISP_ALARM_PRODUCT_RS,TRUE);
                        }
                        else
                        {
                            Disp_SetAlarm(DISP_ALARM_PRODUCT_RS,FALSE);
		
                        }
    
                        if ((Display.iTemperature[DISPLAY_SENSOR_1]/1000) > Display.cfg.cfg2.TEMP_MAX[0]
                            || (Display.iTemperature[DISPLAY_SENSOR_1]/1000) < Display.cfg.cfg2.TEMP_MIN[0])
                        {
                            Disp_SetAlarm(DISP_ALARM_TEMPERATURE,TRUE);
                        }
                        else
                        {
                            Disp_SetAlarm(DISP_ALARM_TEMPERATURE,FALSE);
                        }   
                    }

                    if (!(flags & (1 << DISPLAY_STATE_TOC)))
                    {
                        if (DISPLAY_SENSOR_2 == iChl)
                        {
                            //if ((Display.iConductivity[DISPLAY_SENSOR_2]/100) <= Display.cfg.cfg2.PROD_RES[0])
                            if ((Display.iConductivity[DISPLAY_SENSOR_2]/100) > Display.cfg.cfg2.PROD_RES[1]
								&&Display.cfg.cfg2.FEED_KEY == 1) //进水水质报警条件
                            {
                                Disp_SetAlarm(DISP_ALARM_PRODUCT_RS1,TRUE);
                            }
                            else
                            {
                                Disp_SetAlarm(DISP_ALARM_PRODUCT_RS1,FALSE);
                            }
                            /*进水温度报警*/
							Disp_SetAlarm(DISP_ALARM_TEMPERATURE1,FALSE);
							/*
                            if ((Display.iTemperature[DISPLAY_SENSOR_2]/1000) > Display.cfg.cfg2.TEMP_MAX[0]
                                || (Display.iTemperature[DISPLAY_SENSOR_2]/1000) < Display.cfg.cfg2.TEMP_MIN[0])
                            {
                                Disp_SetAlarm(DISP_ALARM_TEMPERATURE1,TRUE);
                            }
                            
                            else
                            {
                                Disp_SetAlarm(DISP_ALARM_TEMPERATURE1,FALSE);
                            }   
                             */
                            Disp_UserUpdate4FeedQuality();
                        }
                    }
                }
              }                                
              break;
          case DISPLAY_SENSOR_TOC:
              if (DISPLAY_STATE_TOC == Display.CurState.ucMain)
              {
                  if (Display.spec.toc.ulMaxRes4Wash < Display.iConductivity[iChl])
                  {
                      Display.spec.toc.ulMaxRes4Wash  = Display.iConductivity[iChl];
                  }
              }
              else 
              {
                  if (Display.spec.toc.ulMinRes4Oxi > Display.iConductivity[iChl])
                  {
                      Display.spec.toc.ulMinRes4Oxi  = Display.iConductivity[iChl];
                  }
              }          
            
            break;
          }
          // log information
			UartCmdPrintf(VOS_LOG_DEBUG,"CHL%d,C:%dmv,T:%dmv \n",iChl+1,value1,value2);
		}
       // prepare for next conversion
       if (adcs->next)
       {
		Disp_Select_Sensor(1 << adcs->next->ucChl);
       }
   }  
}

void Disp_adc_msg_cb(void)
{
    Disp_Adc_proc();    
}

void Disp_adc_cb(void *para)
{
     Disp_report(Disp_adc_msg_cb);
}


/* 2017/06 ADD for changing BLE Name */
void DispBleProc(uint8 *pRsp,uint8 ucRspLen)
{

    switch (pRsp[RPC_POS_CMD1] & 0X7F)
    {
    case SAPP_CMD_AT:
        if (0 == pRsp[SBL_RSP_STATUS])
        {
            int len = pRsp[RPC_POS_LEN] - 1; 
            char *atrsp = (char *)&pRsp[SBL_RSP_STATUS+1];
        
            if (len > 0 ) // if have additional data
            {
                char *parse;
                
                atrsp[len] = 0;
        
                // split 
                parse = strchr(atrsp,':');
                if (parse) 
                {
                    *parse++ = 0;
        
                    if (0 == strcmp(atrsp,AT_CONFIG_DEVNAME))
                    {
                        if (0 == strncmp(parse,(char *)Display.cfg.cfg3.SERIALNO,16))
                        {
                            Display.bit1CheckBleName = TRUE;
                        }
                        else
                        {
                            DispMakeConfigCmd();
													
                        }
                    }
                }
                
            }
            else
            {
                Display.bit1CheckBleName = TRUE;
            }
        
        }   
        else
        {
        }
        break;
    }

}


// Begin BLE_VERSION

uint8 Disp_DataHandler(uint8 *pCmd,uint8 *pRsp,uint8 CmdLen,uint8 *pucRspLen)
{
    uint8 ret = 0; 

    uint8 *pData    = &pCmd[2];
    uint8 *pRspData = &pRsp[2];

    uint8 ucCmd = pCmd[1]; // pcmd[0] ->sublen
    
    // access info
    switch(ucCmd)
    {
    case RF_CMD_ACCESS: // access info
        {
            char *szName = "";
            char *szPass = "";
            int  len = pCmd[0];
            char *tmp = (char *)pData;

            if (len <= CmdLen - 2)
            {
                tmp[len] = 0;
                
                Att_value_split(tmp,'&');

                if (strlen(tmp) > 0)
                {
                    szName = tmp;
                    tmp += strlen(tmp) + 1;
                }

                if (strlen(tmp) > 0)
                {
                    szPass = tmp;
                    tmp += strlen(tmp) + 1;
                } 

                // update info
                if (Display.bit1AuthState)
                {
                    Disp_SaveInfo();
                    Storage_UpdateUser(&Display.UsrInfo);
                }
                
                strcpy(Display.UsrInfo.Name,szName);
                strcpy(Display.UsrInfo.Pwd,szPass);
                Display.UsrInfo.data = 0;

                szName = Display.UsrInfo.Name;
                szPass = Display.UsrInfo.Pwd;
                
                Display.bit1Admin = 0;
                Display.bit1AuthState = 0;
                
                pRsp[0] = 2;
                pRsp[1] = ucCmd;
                
                if ( 0 == strcmp(gAdmin.NAME, szName)
                    && 0 == strcmp(gAdmin.PSD, szPass))
                {
                    Display.bit1AuthState = 1;
                    Display.bit1Admin     = 1;
                    ret = Storage_GetUser(szName,&Display.UsrInfo);  
                    if (ret)
                    {
                        strcpy(Display.UsrInfo.Name, gAdmin.NAME) ;
                        strcpy(Display.UsrInfo.Pwd, gAdmin.PSD) ;
                        Display.UsrInfo.attr = USER_TYPE_ADMIN ;
                        Display.UsrInfo.data = 0 ;
                    }
                    pRspData[0] = 0;
                    pRspData[1] = USER_TYPE_ADMIN ;
                    *pucRspLen += 4;
                    return 0;
                }

                ret = Storage_CheckUser(szName,szPass);

                if (ret == 0)
                {
                    Display.bit1AuthState = 1;
                    Storage_GetUser(szName,&Display.UsrInfo);

					//if(Display.UsrInfo.attr == USER_TYPE_MANAGER)
					//{
					    //Display.bit1Admin = 2;
					//}
					pRspData[0] = 0;
                    pRspData[1] = Display.UsrInfo.attr;// == USER_TYPE_MANAGER) ? USER_TYPE_USER: Display.UsrInfo.attr;
                    *pucRspLen += 4;
                    return 0;
                }

                pRspData[0] = 1;
                pRspData[1] = 0XFF ;
                *pucRspLen += 4;
                return 0;               
            }
            break;
        }
    case RF_CMD_CONFIG: //  user config
        {
            char *ops = "";
            char *szName = "";
            char *szPass = "";
            char *szAtt = "";
            int  len = pCmd[0];
            char *tmp = (char *)pData;

            if (len <= CmdLen - 2)
            {
                tmp[len] = 0;

                if (Display.bit1AuthState)
                {
                    Att_value_split(tmp,'&');
    
                    if (strlen(tmp) > 0)
                    {
                        ops = tmp;
                        tmp += strlen(tmp) + 1;
                    }
    
                    if (strlen(tmp) > 0)
                    {
                        szName = tmp;
                        tmp += strlen(tmp) + 1;
                    }
    
                    if (strlen(tmp) > 0)
                    {
                        szPass = tmp;
                        tmp += strlen(tmp) + 1;
                    }   
                    if (strlen(tmp) > 0)
                    {
                        szAtt = tmp;
                        tmp += strlen(tmp) + 1;
                    }   

                    if (strlen(szAtt) != 0)
                    {
                        if (szAtt[0] == '1')//ADD USER_TYPE_MANAGER
                        {
                            if (strlen(szName))
                            {
                                if (0 == strcmp(ops,"ADD"))
                                {
                                    if (strlen(szName) && strlen(szPass))
                                    {
										if(Display.bit1Admin || 
                                            (0 == strcmp(szName,Display.UsrInfo.Name)
                                             && USER_TYPE_MANAGER == Display.UsrInfo.attr))//only for ADMIN
										{
										    ret = Storage_AddUser(szName,szPass,szAtt[0] - '0');
										}
										else
										{
										    ret = 0xa5;//Permission denied
										}
                                    }
                                    else
                                    {
                                        ret = 0xa3;//usr or pwd invalid
                                    }
                                }
                                else if (strcmp(ops,"RMV") == 0)
                                {
                                    if(Display.bit1Admin)
									{
									    ret = Storage_RmvUser(szName);
                                    }
                                }
								else
                                {
                                    ret = 0xa2;//config invalid
                                }
                        	}
                        }
						else if(szAtt[0] == '2')//ADD USER_TYPE_USER
						{
						    if (strlen(szName))
                            {
                                if (strcmp(ops,"ADD") == 0)
                                {
                                    if (strlen(szName) && strlen(szPass))
                                    {
                                        if((USER_TYPE_MANAGER == Display.UsrInfo.attr) || 
                                            (Display.bit1Admin)  ||
                                            ( 0 == (strcmp(szName,Display.UsrInfo.Name))
                                                 && USER_TYPE_USER == Display.UsrInfo.attr))
										{
										    ret = Storage_AddUser(szName,szPass,szAtt[0] - '0');
                                        }
										else
										{
										    ret = 0xa5;//Permission denied
										}
                                    }
                                    else
                                    {
                                        ret = 0xa3;//usr or pwd invalid
                                    }
                                }
                                else if (strcmp(ops,"RMV") == 0)
                                {
                                    if(USER_TYPE_MANAGER == Display.UsrInfo.attr)
                                    {
                                        ret = Storage_RmvUser(szName);
                                    }
									else
									{
									    ret = 0xa5;//Permission denied
									}
                                }
								else
                                {
                                    ret = 0xa2;//config invalid
                                }
                        	}
						}
						else
						ret = 0xa4;//Permission denied: ADMIN is readonly
                    }
                    else
                    {
                        ret = 0Xa1;//attr invalid
                    }
                    
                    pRsp[0] = 1;
                    pRsp[1] = ucCmd;
                    pRspData[0] = ret;
                    *pucRspLen += 3;
                    return 0;                   
                }
            }
            return 0XF3; // FAILED
        }
    case RF_CMD_QTW: // QUANTITY TAKE WATER
        {
            if (Display.bit1AuthState)
            {
                int value = BUILD_UINT32(pData[0],pData[1],pData[2],pData[3]);
                Disp_RemoteHandler(RF_CMD_QTW,value); 

                pRsp[0] = 1;
                pRsp[1] = ucCmd;
                pRspData[0] = Display.CurState.ucMain;
                *pucRspLen += 3;
                
                return 0;
            }
            return 0XF3; // FAILED
        }
    case RF_CMD_NTW: //  NORMAL TAKE WATER
        {
            if (Display.bit1AuthState)
            {
                Disp_RemoteHandler(RF_CMD_NTW,0);   

                pRsp[0] = 1;
                pRsp[1] = ucCmd;
                pRspData[0] = Display.CurState.ucMain;
                *pucRspLen += 3;
                
                return 0;
            }
            return 0XF3; // FAILED
        }
    case RF_CMD_CIRCULAION: // circulation
        {
            if (Display.bit1AuthState)
            {
                Disp_RemoteHandler(RF_CMD_CIRCULAION,0);   

                pRsp[0] = 1;
                pRsp[1] = ucCmd;
                pRspData[0] = Display.CurState.ucMain;
                *pucRspLen += 3;
                
                return 0;
            }
            return 0XF3; // FAILED
        }
    case RF_CMD_DEPRESS: // depress
        {
            if (Display.bit1AuthState)
            {
                Disp_RemoteHandler(RF_CMD_DEPRESS,0);   

                pRsp[0] = 1;
                pRsp[1] = ucCmd;
                pRspData[0] = Display.CurState.ucMain;
                *pucRspLen += 3;
                
                return 0;
            }
            return 0XF3; // FAILED
        }
    case RF_CMD_RESET_CM:  //复位耗材
        {
            if (Display.bit1AuthState)
            {
                uint32_t ulMask = BUILD_UINT32(pData[0],pData[1],pData[2],pData[3]);

				//Disp_ResetInfo();//用水量设为0
                //Storage_UpdateUser(&Display.UsrInfo);//将用户用水量更新为0

                if (ulMask & (1 << CM_TYPE_PACK))
                {
                    Disp_UserSetSetPack();
                }
                if (ulMask & (1 << CM_TYPE_FILTER))
                {
                    Disp_UserSetSetFilter();
                
                }
                if (ulMask & (1 << CM_TYPE_UV))
                {
                    Disp_UserSetSetUV();
                }

                if (ulMask)
                {
                   Disp_WriteInfo();

                   Disp_CheckConsumptiveMaterialState();
                }
                
                pRsp[0]     = 1;
                pRsp[1]     = ucCmd;
                pRspData[0] = 0;
                *pucRspLen += 3;
                
                return 0;
            }
            return 0XF3; // FAILED
        }
    case RF_CMD_CM:
        {
            if (Display.bit1AuthState)
            {
                int tmp;
                pRsp[0] = 10;//LEN
                pRsp[1] = ucCmd;
                        
                tmp = Display.cfg.cfg2.PACKLIFEDAY - (RTC_Get_Second() - Display.info.PACKLIFEDAY)/DISP_DAYININSECOND;
                pRspData[0] = (tmp & 0XFF);
                pRspData[1] = ((tmp >>  8) & 0XFF);

                tmp = Display.info.PACKLIFEL;
                pRspData[2] = (tmp & 0XFF);
                pRspData[3] = ((tmp >>  8) & 0XFF);

                tmp = Display.cfg.cfg2.UVLIFEDAY - (RTC_Get_Second() - Display.info.UVLIFEDAY)/DISP_DAYININSECOND;
                pRspData[4] = (tmp & 0XFF);
                pRspData[5] = ((tmp >>  8) & 0XFF);

                tmp = Display.info.UVLIFEHOUR/(3600/UV_PFM_PEROID);
                pRspData[6] = (tmp & 0XFF);
                pRspData[7] = ((tmp >>  8) & 0XFF);

                tmp = Display.cfg.cfg2.FILTERLIFE - (RTC_Get_Second() - Display.info.FILTERLIFE)/DISP_DAYININSECOND;
                pRspData[8] = (tmp & 0XFF);
                pRspData[9] = ((tmp >>  8) & 0XFF);
                
                *pucRspLen += 12;
                return 0;
             }
            return 0XF3;
        }
    case RF_CMD_CONDUCTIVITY:
         {
		 	if(Display.bit1AuthState)
            {
                int tmp;
				// pRsp[0] = 5;//LEN
				#ifdef TOC_APP
                   pRsp[0] = 9;
				#else
				   pRsp[0] = 5;
				#endif
                pRsp[1] = ucCmd;
                        
                tmp = Display.iConductivity[0];
                pRspData[0] = (tmp & 0XFF);
                pRspData[1] = ((tmp >>  8) & 0XFF);

                tmp = Display.cfg.cfg2.UNIT;
                pRspData[2] = (tmp & 0XFF);

                tmp = Display.iTemperature[0];
                pRspData[3] = (tmp & 0XFF);
                pRspData[4] = ((tmp >>  8) & 0XFF);
				
                /*  电导率，TOC发送 20161018 */
				#ifdef TOC_APP
				  tmp = Display.iConductivity[1];
                  pRspData[5] = (tmp & 0XFF);
                  pRspData[6] = ((tmp >>  8) & 0XFF);

				  tmp = (int)(Display.iConductivity[2]/10); //20161229
                  pRspData[7] = (tmp & 0XFF);
                  pRspData[8] = ((tmp >>  8) & 0XFF);
				#endif
				
             // *pucRspLen += 7; //20161018
				#ifdef TOC_APP
                  *pucRspLen += 11;
				#else
                  *pucRspLen += 7; 
				#endif
                return 0;
            }
            return 0xF3;
    	}
    case RF_CMD_HISTORY:
        {
			if(Display.bit1AuthState)
            {
                //Storage_WriteMeasInfo(&Display,&timer);
                uint32 tmp;
                tm tmpDate;
                tmpDate.w_year = BUILD_UINT16(pData[0],pData[1]);
                tmpDate.w_month = ((pData[2]) & 0xFF);
                tmpDate.w_date = ((pData[3]) & 0xFF);

                pRsp[0] = 4;//LEN
                pRsp[1] = ucCmd;

				ret = Storage_GetMeasInfo((uint8 *)&tmp,&tmpDate);
                if (!ret)
                {
                    pRspData[0] = tmp & 0XFF;
                    pRspData[1] = (tmp >> 8) & 0XFF;
                    pRspData[2] = (tmp >> 16) & 0XFF;
                    pRspData[3] = (tmp >> 24) & 0XFF;
                    
                    *pucRspLen += 6;
                    return 0;
                }
				else
				{
                    return ret;
				}
            }
            return 0xF3;
    	}
    case RF_CMD_USER_HIS:
        {
			if(Display.bit1AuthState)
            {
                //Storage_WriteMeasInfo(&Display,&timer);
                //uint32 tmp;
                STORAGE_USER_INFO tmpUserInfo;
                int  len = pCmd[0];
				
                pData[len] = 0;
                if (strlen((char *)&pData[1]) == 0)
                {
                    return 0xa0;//username invalid
                }
                                
                strcpy(tmpUserInfo.Name,(char *)&pData[1]);

                pRspData[0] = pCmd[2];

                pRsp[0] = 5; //LEN
                pRsp[1] = ucCmd;

                if (0 != Storage_GetUser(tmpUserInfo.Name,&tmpUserInfo))
                {
                    pRspData[1] = 0;
                    pRspData[2] = 0;
                    pRspData[3] = 0;
                    pRspData[4] = 0;
                }
                else
                {
                    pRspData[1] = tmpUserInfo.data & 0XFF;
                    pRspData[2] = (tmpUserInfo.data >> 8) & 0XFF;
                    pRspData[3] = (tmpUserInfo.data >> 16) & 0XFF;
                    pRspData[4] = (tmpUserInfo.data >> 24) & 0XFF;
                }
                
                *pucRspLen += 7;
                return 0;
            }
            return 0xF3;
    	}
    case RF_CMD_STATUS:
        {
            if (Display.bit1AuthState)
            {
				pRsp[0] = 1;
                pRsp[1] = ucCmd;
                pRspData[0] = Display.CurState.ucMain;
                *pucRspLen += 3;
                
                return 0;
            }
            return 0XF3; // FAILED
        }
	case RF_CMD_KEY:
		{
            if (Display.bit1AuthState)
            {
                if((Display.bit1Admin) ||
                    (Display.UsrInfo.attr == USER_TYPE_MANAGER))
                {
                    gusOperMode = pData[0];
					
					pRsp[0] = 1;
                    pRsp[1] = ucCmd;
					pRspData[0] = 0;//success
                    *pucRspLen += 3;
                    
                    return 0;
                }
				else
				{
				    pRsp[0] = 1;
                    pRsp[1] = ucCmd;
					pRspData[0] = 0xa5;//Permission denied
                    *pucRspLen += 3;
                    
                    return 0;
				}
            }
            return 0XF3; // FAILED
	    }
#if 0			
    case RF_CMD_RESTORE_FACTORY_CFG:
        {
            Disp_InitConfig();

            pRsp[0] = 1;
            pRsp[1] = ucCmd;
            pRspData[0] = 0;//success
            *pucRspLen += 3;
            
            return 0;
        }
#endif				
    default:
        break;
    }    
    return SAPP_IGNORED;
}

uint8 Disp_InnerHandler(uint8 *pCmd,uint8 *pRsp,uint8 CmdLen,uint8 *pucRspLen)
{
    switch(pCmd[0])
    {
    case 0: // Connection state
        {
            uint8 oldState = Display.bit1ConnState;
            Display.bit1ConnState = pCmd[1];
            if (oldState != Display.bit1ConnState)
            {
                // state changed ,clear old info
                if (oldState && Display.bit1AuthState)
                {
                    Disp_SaveInfo();
                
                    // update user info
                    Storage_UpdateUser(&Display.UsrInfo);
                }
                
                Display.UsrInfo.Name[0] = 0;
                Display.UsrInfo.Pwd[0] = 0;
                Display.bit1AuthState = 0;
                Display.bit1Admin = 0;
            }
        }
        break;
    default:
        break;
    }
    return SAPP_IGNORED;
}

// End BLE_VERSION

void Disp_Init(void)
{
    // begin add for v2
    Storage_Init();
    // end add for v2
    
    memset(&Display,0,sizeof(DISPLAY_STRU));

    // global initialization
    Display.usForColor       = BLACK;
    Display.usBackColor      = WHITE;
    Display.usBackColor4Set  = WHITE;
    Display.cfg.cfg3.devtype      = DEV_TYPE_V1; // late update from cfg
    Display.ucDspWelcomeTime = 3; // fixed to three second
    Display.ulStartSecond    = RTC_Get_Second();
    Display.ucLastShow       = 0XFF;
    Display.bit1Today        = TRUE;

    Display.ausSampleMask[DISPLAY_SENSOR_1] =  (1<<DISPLAY_STATE_IDLE) 
                                               |(1<<DISPLAY_STATE_CIRCULATION) 
                                               |(1<<DISPLAY_STATE_NORMAL_TAKING_WATER)
                                               |(1<<DISPLAY_STATE_QUANTITY_TAKING_WATER)
                                               |(0<<DISPLAY_STATE_TOC)   // 1 TOC过程不检测通道1
                                               |(1<<DISPLAY_STATE_DEGASS); 

    Display.ucChlMask |= 1<<DISPLAY_SENSOR_1; 

#ifdef UF_FUNCTION    
    
    Display.ausSampleMask[DISPLAY_SENSOR_2] =  (1<<DISPLAY_STATE_IDLE)
                                               |(0<<DISPLAY_STATE_CIRCULATION)
                                               |(1<<DISPLAY_STATE_NORMAL_TAKING_WATER)
                                               |(1<<DISPLAY_STATE_QUANTITY_TAKING_WATER)
                                               |(0<<DISPLAY_STATE_TOC)
                                               |(0<<DISPLAY_STATE_DEGASS);
   Display.ucChlMask |= 1<<DISPLAY_SENSOR_2; 
#endif

#ifdef PRO     //20160824
    
    Display.ausSampleMask[DISPLAY_SENSOR_2] =  (1<<DISPLAY_STATE_IDLE) //1
                                               |(0<<DISPLAY_STATE_CIRCULATION) //0
                                               |(1<<DISPLAY_STATE_NORMAL_TAKING_WATER)  //1
                                               |(1<<DISPLAY_STATE_QUANTITY_TAKING_WATER)  //1
                                               |(0<<DISPLAY_STATE_TOC)
                                               |(0<<DISPLAY_STATE_DEGASS);
   Display.ucChlMask |= 1<<DISPLAY_SENSOR_2; 
#endif

#ifdef TOC
    Display.ausSampleMask[DISPLAY_SENSOR_TOC] = (0<<DISPLAY_STATE_IDLE)   //0
                                               |(0<<DISPLAY_STATE_CIRCULATION)
                                               |(0<<DISPLAY_STATE_NORMAL_TAKING_WATER)// 产水时不检测通道3
                                               |(0<<DISPLAY_STATE_QUANTITY_TAKING_WATER) // 不检测通道3
                                               |(0<<DISPLAY_STATE_TOC) // leave to TOC proc
                                               |(0<<DISPLAY_STATE_DEGASS);
    Display.ucChlMask |= 1<<DISPLAY_SENSOR_TOC; 
#endif

    // add for v2
    sLastUpdateTime = timer;
    // end for v2
    
    stimer = timer;

    oldfont = NULL;

    adcChlHeader.next = NULL;

    // init default fount
    font24.DrawChar = LCD_PutChar12x24;
    font24.DrawText = LCD_PutStringGB24x24;
    font24.GetTextSize = LCD_GetStringGB24x24TextSize;
    font24.sizeX = 12;
    font24.sizeY = 24;

    font8.DrawChar = LCD_PutChar8x16;
    font8.DrawText = LCD_PutString8x16;
    font8.GetTextSize = LCD_GetStringGB8x16TextSize;
    font8.sizeX = 8;
    font8.sizeY = 16;

#ifdef CODEGB_32
    font32.DrawChar = NULL;
    font32.DrawText = PutStringGB3232;
    font32.GetTextSize = LCD_GetStringGB32x32TextSize;
    font32.sizeX = 16;
    font32.sizeY = 32;
#endif

#ifdef CODEGB_48
    font48.DrawChar = NULL;
    font48.DrawText = PutStringGB4848;
    font48.GetTextSize = LCD_GetStringGB48x48TextSize;
    font48.sizeX = 24;
    font48.sizeY = 48;
#endif

    Disp_SelectFont(&font24); // font24 as default font   
    {
        int xoff;

        sprintf((char *)Config_buff,TIME_FORMAT_SHORT,0,0,0,0,0);
        xoff = curFont->sizeX*strlen((char *)Config_buff);

        rectTime.left     = LCD_W - TIME_RECT_MARGIN*2 - xoff;
        rectTime.right    = rectTime.left + xoff + TIME_RECT_MARGIN*2 ;
        rectTime.top      = TIME_RECT_MARGIN ;
        rectTime.bottom   = rectTime.top + curFont->sizeY + TIME_RECT_MARGIN*2;
    }

    Disp_ReadConfig();

    Disp_ReadInfo();

    Disp_ReadQtwVolume();

    Disp_ValveCtrl(&Display.CurState);

    // Disp_DisplayIdlePage();
    Disp_DisplayWelcomePage();

    // begin add for v2
    Disp_CheckUFClean();    
    // end add for v2

    {
        uint8_t Idx;
        Display.FreePoolIdx = 0;
    
        for (Idx = 0 ; Idx < MAX_ADC_ACTIONS ; Idx++)
        {
            aAdcPool[Idx].NextIndex = Idx + 1;
        }    
    }
    sys_timeout(1000,SYS_TIMER_PERIOD,100,Disp_adc_cb,&Display,&Display.to4Adc);    
}

