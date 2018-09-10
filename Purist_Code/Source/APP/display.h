#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#define TOC
//#define UF_FUNCTION
#define PRO   //20160822
#define TOC_APP //用与手机APP显示TOC


#include "stm32_eval.h"
#include "DtypeStm32.h"
#include "config.h"
#include "sys_time.h "

#define TIME_FUNCTION

typedef enum
{
   DISPLAY_PAGE_IDLE,
   DISPLAY_PAGE_USER_SET,
   DISPLAY_PAGE_ENG_SET,
   DISPLAY_PAGE_CIRCULATION,
   DISPLAY_PAGE_NORMAL_TAKING_WATER,
   DISPLAY_PAGE_QUANTITY_TAKING_WATER,
   DISPLAY_PAGE_DECOMPRESSION,
   // begin add for V2
   // for UF
   DISPLAY_PAGE_UF_CLEAN,
   DISPLAY_PAGE_DEGASS,
   // end for UF
   DISPLAY_PAGE_TOC,
   // end add for V2
   DISPLAY_PAGE_BUTT,
}DISPLAY_PAGE_ENUM;


typedef enum
{
   DISPLAY_SUB_PAGE_IDLE,

   // SUB PAGE FOR IDLE
   DISPLAY_SUB_PAGE_TANK_EMPTY,
   DISPLAY_SUB_PAGE_IDLE_FLUSH,
   

   // SUB PAGE FOR USER_SET = 0X10
   // SUB PAGE FOR ENG_SET = 0X20
   // SUB PAGE FOR CIRCULATION = 0X30
   // SUB PAGE FOR NORMAL_TAKING_WATER = 0X40
   // SUB PAGE FOR QUANTITY_TAKING_WATER
   DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_SETTING = 0X50,
   DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_VOID_TANK ,
   DISPLAY_SUB_PAGE_QUANTITY_TAKING_WATER_TAKING ,
   // SUB PAGE FOR DECOMPRESSION
   // SUB PAGE FOR US SETTING
   DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_PREPARE = 0X60,
   DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP1,
   DISPLAY_SUB_PAGE_US_SET_UF_CLEAN_STEP2,
   
   // SUB PAGE FOR UF WASH
   // DISPLAY_SUB_PAGE_US_UF_WASH = 0X70,

   // SUB PAGE FOR CIR
   DISPALY_SUB_PAGE_TOC_FLUSH            = 0X80,
   DISPALY_SUB_PAGE_TOC_OXIDATION,

   DISPALY_SUB_PAGE_DEGASS1            = 0X90,
   DISPALY_SUB_PAGE_DEGASS2,
   
   
   DISPLAY_SUB_PAGE_BUTT,
}DISPLAY_SUB_PAGE_ENUM;

typedef struct 
{
    uint16_t left,top,right,bottom;
}RECT;

typedef enum
{
    DISPLAY_STATE_IDLE,
    DISPLAY_STATE_CIRCULATION,
    DISPLAY_STATE_NORMAL_TAKING_WATER,
    DISPLAY_STATE_QUANTITY_TAKING_WATER,
    DISPLAY_STATE_DECOMPRESSION,
    // BEGIN ADDITON FOR V2
    DISPLAY_STATE_UF_CLEAN,
    DISPLAY_STATE_TOC,
    DISPLAY_STATE_DEGASS,
    // END ADDITON FOR V2

    DISPLAY_STATE_BUTT,
        
}DISPLAY_STATE_ENUM;

typedef enum
{
    DISPLAY_SUB_STATE_IDLE = 0,

    DISPLAY_SUB_STATE_IDLE_FLUSH,

    DISPLAY_SUB_STATE_DEGASS1,

    DISPLAY_SUB_STATE_DEGASS2,

    DISPLAY_SUB_STATE_TOC_FLUSH,
    
    DISPLAY_SUB_STATE_TOC_OXIDATION,

    DISPALY_SUB_STATE_UF_PRECLEAN,
    
    DISPALY_SUB_STATE_UF_CLEAN,

    DISPLAY_SUB_STATE_BUTT,       
}DISPLAY_SUB_STATE_ENUM;


#define DISP_DECIMAL (1) // 10^1

#define DISP_MAX_WATER_VOLUME (1000)

#define DISP_30SECOND2MS   (30*1000)

#define DISP_MINUTE2MS     (60*1000)

#define DISP_DAYININSECOND (24*60*60)

enum
{
	USER_TYPE_ADMIN = 0,
	USER_TYPE_MANAGER ,
	USER_TYPE_USER ,
};

typedef struct
{
    char          Name[16];
	char          Pwd[16];
	char          attr;      //0,1,2
	unsigned int  data;
}STORAGE_USER_INFO;


typedef struct 
{
    uint16_t usDummy;
}DISP_SPECIAL_IDLE;

typedef struct 
{
    uint16_t usDummy;
}DISP_SPECIAL_CIRCULATION;

typedef struct 
{
    uint16_t usDummy;
}DISP_SPECIAL_NORMAL_TAKING_WATER;

#define DISP_CHECK_PERIOD (100) // 100ms

typedef struct 
{
    uint16_t usDummy;
    uint32_t time4tw;
    uint32_t timeElaps;
    
}DISP_SPECIAL_QUANTITY_TAKING_WATER;

typedef struct 
{
    uint16_t usDummy;
}DISP_SPECIAL_DECOMPRESSION;

// add for V2
typedef enum
{
    DISPLAY_UF_SUB_STATE_1 = 0,
    DISPLAY_UF_SUB_STATE_2,
    DISPLAY_UF_SUB_STATE_3,
    DISPLAY_UF_SUB_STATE_4,
    DISPLAY_UF_SUB_STATE_5,

    DISPLAY_UF_SUB_STATE_BUTT,
        
}DISPLAY_UF_SUB_STATE_ENUM;
// end for V2

typedef struct 
{
    uint16_t usCountDownTimer;
    uint16_t usSubCountDownTimer;
    uint8_t ucStage; // refer DISPLAY_UF_SUB_STATE_ENUM
    uint8_t ucSubStage;
    uint8_t ucCnt4Cir;
}DISP_SPECIAL_UF_WASH;

typedef struct 
{
    uint8_t aActionBuf[4]; // refer DISPLAY_UF_SUB_STATE_ENUM
    uint8_t ucActionCnt;
    uint8_t ucIdx;
    uint32_t  ulMaxRes4Wash;
    uint32_t  ulMinRes4Oxi;
}DISP_SPECIAL_TOC;

typedef struct 
{
    uint16_t  usTimes;
}DISP_SPECIAL_DEGASS;


typedef struct
{
    uint8_t ucMain;// refer DISPLAY_PAGE_ENUM
    uint8_t ucSub; // DISPLAY_SUB_PAGE_ENUM
}PAGE_STRU;

typedef struct
{
    uint8_t ucMain;// refer DISPLAY_STATE_ENUM
    uint8_t ucSub; // DISPLAY_SUB_STATE_ENUM
}STATE_STRU;


typedef enum
{
    DISP_LAN_ENGLISH = 0,
    DISP_LAN_CHINESE,
    DISP_LAN_GERMANY,
    DISP_LAN_NUM,
}DISP_LANGUAGE_ENUM;

#define DISP_K4_US_SET_TIMES (8)

#define DISP_K4_ENG_SET_TIMES (16)

typedef struct
{
    UINT32 PACKLIFEDAY;   //纯化柱:  0~999 DAYS    0~9999L 
    UINT32 FILTERLIFE;    //过滤器寿命 :0~999DAYS
    UINT32 UVLIFEDAY;     // 0~999 DAYS
    UINT16 UVLIFEHOUR;    // 0~9999Hr (unit 10min)
    UINT16 PACKLIFEL;     //纯化柱:   0~9999L 
}DISP_INFO_STRU;

typedef enum
{
    DEV_TYPE_V1 = 0,
    DEV_TYPE_V2,
    DEV_TYPE_NUM,
}DISP_DEV_TYPE_ENUM;


typedef enum
{
    DISP_ALARM_PRODUCT_RS,
    DISP_ALARM_PRODUCT_RS1,
    DISP_ALARM_TEMPERATURE,
    DISP_ALARM_TEMPERATURE1,
#ifdef TOC
    DISP_ALARM_TOC_PB,    // P/B<1.3
    DISP_ALARM_TOC_B25,   //B25>300
    DISP_ALARM_TOC_VALUE, // VALUE
#endif
    DISP_ALARM_TANK_EMPTY,
    DISP_ALARM_NUM,
}DISP_ALARM_ENUM;

typedef struct
{
    uint8_t bit1Fired     : 1;
    uint8_t bit1Triggered : 1;
    uint32_t ulFireSec;
}DISP_ALARM_STRU;

#define DISPLAY_MAX_LINE_PER_PAGE (9)


typedef enum
{
    DISPLAY_SENSOR_1 = 0,
    DISPLAY_SENSOR_2,
    DISPLAY_SENSOR_TOC,
    DISPLAY_SENSOR_NUM,

}DISPLAY_SENSOR_ENUM;

#define DISPLAY_MAX_SENSOR (DISPLAY_SENSOR_NUM)


typedef enum
{
    DISP_US_ITEM_PACK = 0,
    DISP_US_ITEM_FILTER,
    DISP_US_ITEM_UV,
    DISP_US_ITEM_FLOW,
    DISP_US_ITEM_UNIT,
#ifdef TIME_FUNCTION    
    DISP_US_ITEM_TIME,
#endif    
    DISP_US_ITEM_LANGUAGE,
#ifdef UF_FUNCTION    
    // begin add for v2
    DISP_US_ITEM_UF_CLEAN,
    DISP_US_ITEM_UF_FLUSH,
    DISP_US_ITEM_FEED_QUALITY,
    // end add for v2
#endif   
    DISP_US_ITEM_FEED_KEY, //20170117
    DISP_US_ITEM_SERIAL_NO,
    DISP_US_ITEM_NUM,    
}DISP_US_ITEM_ENUM;

typedef enum
{
    DISP_ES_ITEM_CELL_CONSTANT1 = 0,
    DISP_ES_ITEM_TEMP_CONSTANT1,
#ifdef UF_FUNCTION        
    DISP_ES_ITEM_CELL_CONSTANT2,
#endif    
#ifdef UF_FUNCTION    
    DISP_ES_ITEM_TEMP_CONSTANT2,
#endif    
#ifdef PRO        
    DISP_ES_ITEM_CELL_CONSTANT2,
#endif    
#ifdef PRO   
    DISP_ES_ITEM_TEMP_CONSTANT2,
#endif  
#ifdef TOC    
    DISP_ES_ITEM_CELL_CONSTANT3,
#endif
#ifdef TOC    
    DISP_ES_ITEM_TEMP_CONSTANT3,
#endif    
    DISP_ES_ITEM_TEMP_RANGE1,
#ifdef UF_FUNCTION        
    // begin add for v2
    DISP_ES_ITEM_TEMP_RANGE2,
#endif   
#ifdef PRO       
    // begin add for v2
    DISP_ES_ITEM_TEMP_RANGE2,
#endif  
    // end add for v2
    DISP_ES_ITEM_PRODUCT_RS1,
    // begin add for v2
#ifdef UF_FUNCTION        
    DISP_ES_ITEM_PRODUCT_RS2,
#endif 
#ifdef PRO        
    DISP_ES_ITEM_PRODUCT_RS2,
#endif 
    // end add for v2
    DISP_ES_ITEM_PACK,
    DISP_ES_ITEM_FILTER,
    DISP_ES_ITEM_UV,
    DISP_ES_ITEM_RECIRCULATION,
#ifdef UF_FUNCTION        
    DISP_ES_ITEM_UF_DISINFECTION,
    DISP_ES_ITEM_UF_FLUSH,
    DISP_ES_ITEM_UF_AUTO_FLUSH,
#endif    
    DISP_ES_ITEM_NUM,    
}DISP_ES_ITEM_ENUM;

typedef enum
{
    DISP_SHOW_ALARM_NONE = 0,
    DISP_SHOW_ALARM_CM ,
    DISP_SHOW_ALARM_SENSOR ,
    DISP_SHOW_ALARM_UF_CLEAN,
}DISP_SHOW_ALARM_ENUM;

#define DISP_USER_NAME_LENGTH (24)
#define DISP_USER_PASS_LENGTH (6)

typedef struct
{
    uint32_t bit1PackCheck   : 1;
    uint32_t bit1UVCheck     : 1;
    uint32_t bit1FilterCheck : 1;
    uint32_t bit1SetTime     : 1;
    uint32_t bit1ShowCmState : 1;
    uint32_t bit1Flash       : 1;
    uint32_t bit3ShowAlarm   : 3; // REFER DISP_SHOW_ALARM_ENUM

    // begin add for v2
    uint32_t bit1PendingUFClean : 1;
    // end add for v2
    uint32_t bit1Measured      : 1;
    uint32_t bit1Today         : 1;
    uint32_t bit1InitPending4UF : 1;

	// begin for BLE
    uint32_t bit1ConnState    : 1;
    uint32_t bit1AuthState    : 1;
    uint32_t bit1Admin        : 1;
    uint32_t bit1CheckBleName : 1;
	// end for BLE
    
    
    //uint8_t ucDevType; // refer DISP_DEV_TYPE_ENUM
    uint8_t ucDspWelcomeTime;

    //uint8_t ucPeriodSensorMeas;
    
    STATE_STRU CurState; // refer DISPLAY_STATE_ENUM
    STATE_STRU TgtState; // refer DISPLAY_STATE_ENUM

    //uint8_t fontSizeX,fontSizeY;

    uint8_t ucCurLine;
    uint8_t ucTotalLine;
    uint8_t ucCurCol;
    uint8_t ucDispCol;
    uint8_t ucRollIdx; 
    uint8_t ucLastShow;

    PAGE_STRU curPage;
    PAGE_STRU tgtPage;

    uint16_t usIdleTimes;
    uint16_t usIdleLcdTimes;

    uint16_t usSeconds;

    uint16_t usForColor; // for general purpose, i.e. setting
    uint16_t usBackColor; // for general purpose, i.e. setting

    uint16_t usBackColor4Set;

	DISP_INFO_STRU info;
    LOCAL_CONFIG_STRU cfg;

    int iConductivity[DISPLAY_MAX_SENSOR]; // decimal 3
    int iTemperature [DISPLAY_MAX_SENSOR] ; // decimal 3

    int  aulBestCdt[DISPLAY_MAX_SENSOR];

    //int iOrgConductivity; // decimal 3
    //int iOrgTemperature ; // decimal 3

    uint32_t ulStartSecond;

    uint32_t ulCumulatedVolume; // in ml
    uint32_t ulCumulatedVuTime; // in second


    DISP_ALARM_STRU aAlarm[DISP_ALARM_NUM];

    // FOR SETTING
    uint8_t aucSetLine[DISP_ES_ITEM_NUM]; // us&es have equal lines, refer DISP_US_ITEM_ENUM&DISP_ES_ITEM_ENUM
    uint8_t ucLineOffset;

    uint16_t usQtwWq; // water quantity 

    union {
        DISP_SPECIAL_IDLE idle;
        DISP_SPECIAL_CIRCULATION cir;
        DISP_SPECIAL_NORMAL_TAKING_WATER ntw;
        DISP_SPECIAL_QUANTITY_TAKING_WATER qtw;
        DISP_SPECIAL_DECOMPRESSION dec;
        DISP_SPECIAL_UF_WASH       uf;
        DISP_SPECIAL_TOC           toc;
        DISP_SPECIAL_DEGASS        dg;
    }spec;

    sys_timeo to4qtw; // timer for quantity&normal taking water

    sys_timeo to4cir; // timer for circulation duration

    
    sys_timeo to4UfPrePare; // timer for idle state supervisor 

    sys_timeo to4UfClean; // timer for idle state supervisor 

    sys_timeo to4TOC; // timer for idle state supervisor 

    sys_timeo to4TOCMeas; // timer for TOC fast measurement 

    sys_timeo to4Degass; // timer for DE gass
    
    sys_timeo to4Adc; // timer for sensor

    uint8_t   KeyState;

    // add for v2
    //uint8_t   ucLastUpdateDay;
    uint32_t  ulUFCleanTime;
    uint32_t  ulCalcirtime;   // in second 
    uint32_t  aulMaxRes4Tw[DISPLAY_MAX_SENSOR];
    uint32_t  aulMaxTemp4Tw[DISPLAY_MAX_SENSOR];
    uint32_t  ulIdleFlushTime; // count for idle wash period
    uint32_t  ulIdleFlushCnt;  // count for idle washing duration
    uint32_t  ulIdleCirTime; // count for idle wash period

    uint8_t FreePoolIdx;
    // end for v2
    uint16_t ausSampleMask[DISPLAY_SENSOR_NUM];

    uint8_t ucChlMask;

    // add for realtime feed quality update
    uint16_t      usX4FeedQuality;
    uint16_t      usY4FeedQuality;
    uint16_t      usCharColor4FeedQuality;
    uint16_t      usBackColor4FeedQuality;

	// add for BLE
	STORAGE_USER_INFO  UsrInfo;
	// end for BLE

}DISPLAY_STRU;

extern DISPLAY_STRU Display;

void Disp_Init(void);
void Disp_KeyHandler(int key,int state);
void Disp_msg_Handler(Message *Msg);
void Disp_SecondTask(void);
void DispBleProc(uint8 *pRsp,uint8 ucRspLen);

#endif
