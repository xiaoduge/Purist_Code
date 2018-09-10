#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Relay.h"

#include "sapp.h"
#include "at.h"

#include "Config.h"

#include "Beep.h"

#include "UartCmd.h"

#include "sys_time.h"

uint16_t msgSeq;
uint32_t msgTick;

extern UINT32 ulAppTicks ;

#define TICK2MS(value) ((value)*10)


typedef enum
{
    APP_PARK_ACTION_NULL = 0,
    APP_PARK_ACTION_OPEN,
    APP_PARK_ACTION_CLOSE
}APP_PARK_ACTION_ENUM;

typedef enum
{
    APP_PARK_STATE_OPENED = 0,
    APP_PARK_STATE_CLOSED,
}APP_PARK_STATE;

typedef enum
{
    APP_PARK_SUB_STATE_NULL = 0, // for stable state
    APP_PARK_SUB4OPENED_CLOSING,
    APP_PARK_SUB4OPENED_PAUSE,
    APP_PARK_SUB4OPENED_OPENING,

    APP_PARK_SUB4CLOSED_OPENING,
    APP_PARK_SUB4CLOSED_PAUSE,
    APP_PARK_SUB4CLOSED_CLOSING,
    
}APP_PARK_SUB_STATE;


typedef enum
{
    APP_PARK_MODE_0 = 0, // for stable state
    APP_PARK_MODE_1,
    APP_PARK_MODE_NUM,
}APP_PARK_MODE_ENUM;

typedef struct
{
    uint8_t  ucBitOngoing : 1;
    uint8_t  ucSwitch     : 1;
    
    uint8_t  ucCurMainState; // APP_PARK_STATE
    uint8_t  ucCurSubState;  // APP_PARK_SUB_STATE
    uint8_t  ucTgtState;     // APP_PARK_STATE
    uint32_t ulStartTick;    // 

    uint8_t  ucPendingAction; // refer APP_PARK_ACTION_ENUM

    sys_timeo to;
    
}APP_PARK_STRU;

APP_PARK_STRU appPark;

void AppPark_HandleBuffWork(void);
void AppPark_PulseTimeCb(void *);

void AppPark_PulseCb(void)
{
    sys_timeout(gusPulseInterval,SYS_TIMER_ONE_SHOT,gusPulseInterval,AppPark_PulseTimeCb,&appPark,&appPark.to);
}

void AppPark_Pulse(uint8_t ucSwitchOn)
{
    appPark.ucBitOngoing = TRUE;

    if (ucSwitchOn)
    {
        switch(gusMode)
        {
        default:
            RelayPulse(RELAY_DOOR_OPEN_EN,gusPulseWidth,AppPark_PulseCb);
            break;
        case APP_PARK_MODE_1:
            RelayPulse(RELAY_DOOR_OPEN_EN,gusPulseWidth,AppPark_PulseCb);
            break;
        }
    }
    else
    {
        switch(gusMode)
        {
        default:
            RelayPulse(RELAY_DOOR_CLOSE_EN,gusPulseWidth,AppPark_PulseCb);
            break;   
        case APP_PARK_MODE_1:
            RelayPulse(RELAY_DOOR_OPEN_EN,gusPulseWidth,AppPark_PulseCb);
            break;
        }
    }

    appPark.ulStartTick = ulAppTicks;
}

void AppPark_OpenDoor(void)
{
    switch(gusMode)
    {
    default:
        AppPark_Pulse(TRUE);
        break;
    case APP_PARK_MODE_1:
        appPark.ucTgtState = APP_PARK_STATE_OPENED;
    
        switch(appPark.ucCurMainState)
        {
        case APP_PARK_STATE_OPENED:
            switch(appPark.ucCurSubState)
            {
            case APP_PARK_SUB_STATE_NULL:
                // The desired state, just return
                AppPark_HandleBuffWork();
                break;
            case APP_PARK_SUB4OPENED_CLOSING:
                AppPark_Pulse(TRUE);
                appPark.ucCurSubState = APP_PARK_SUB4OPENED_PAUSE;
                DEBUG_LOG("OPENED:PAUSE");
                break;
            case APP_PARK_SUB4OPENED_PAUSE:
                AppPark_Pulse(TRUE);
                appPark.ucCurSubState = APP_PARK_SUB4OPENED_OPENING;
                DEBUG_LOG("OPENED:OPENING");
                break;
            case APP_PARK_SUB4OPENED_OPENING:
                break;
            default:
                break;
            }
            break;
        default: // APP_PARK_STATE_CLOSED
            switch(appPark.ucCurSubState)
            {
            case APP_PARK_SUB_STATE_NULL:
                AppPark_Pulse(TRUE);
                appPark.ucCurSubState = APP_PARK_SUB4CLOSED_OPENING;
                DEBUG_LOG("CLOSED:OPENING");
                break;
            case APP_PARK_SUB4CLOSED_OPENING:
                break;
            case APP_PARK_SUB4CLOSED_PAUSE:
                AppPark_Pulse(TRUE);
                appPark.ucCurSubState = APP_PARK_SUB4CLOSED_CLOSING;
                DEBUG_LOG("CLOSED:CLOSING");
                break;
            case APP_PARK_SUB4CLOSED_CLOSING:
                break;
            default:
                break;
            }        
            break;
        }
        break;
    }
}

void AppPark_CloseDoor(void)
{
    switch(gusMode)
    {
    default:
        AppPark_Pulse(FALSE);
        break;
    case APP_PARK_MODE_1:
        appPark.ucTgtState = APP_PARK_STATE_CLOSED;
        switch(appPark.ucCurMainState)
        {
        case APP_PARK_STATE_OPENED:
            switch(appPark.ucCurSubState)
            {
            case APP_PARK_SUB_STATE_NULL:
                AppPark_Pulse(TRUE);
                appPark.ucCurSubState = APP_PARK_SUB4OPENED_CLOSING;
    
                DEBUG_LOG("OPENED:CLOSING");
                break;
            case APP_PARK_SUB4OPENED_CLOSING:
                break;
            case APP_PARK_SUB4OPENED_PAUSE:
                AppPark_Pulse(TRUE);
                appPark.ucCurSubState = APP_PARK_SUB4OPENED_OPENING;
                
                DEBUG_LOG("OPENED:OPENING");
                break;
            case APP_PARK_SUB4OPENED_OPENING:
                break;
            default:
                break;
            }
            break;
        default: // APP_PARK_STATE_CLOSED
            switch(appPark.ucCurSubState)
            {
            case APP_PARK_SUB_STATE_NULL:
                // The desired state, just return
                AppPark_HandleBuffWork();
                break;
            case APP_PARK_SUB4CLOSED_OPENING:
                AppPark_Pulse(TRUE);
                appPark.ucCurSubState = APP_PARK_SUB4CLOSED_PAUSE;
                DEBUG_LOG("CLOSED:PAUSE");
                break;
            case APP_PARK_SUB4CLOSED_PAUSE:
                AppPark_Pulse(TRUE);
                appPark.ucCurSubState = APP_PARK_SUB4CLOSED_CLOSING;
                DEBUG_LOG("CLOSED:CLOSING");
                break;
            case APP_PARK_SUB4CLOSED_CLOSING:
                break;
            default:
                break;
            }        
            break;
        }
        break;
    }
}

void AppPark_PulseTimeCb(void *para)
{
    uint8_t ucPendingAction = appPark.ucPendingAction;
    
    appPark.ucBitOngoing = FALSE;

    appPark.ucPendingAction = APP_PARK_ACTION_NULL;

    switch (ucPendingAction)
    {
    case APP_PARK_ACTION_OPEN:
        AppPark_OpenDoor();
        break;
    case APP_PARK_ACTION_CLOSE:
        AppPark_CloseDoor();
        break;
    default:
        if (0XFF == appPark.ucTgtState)
        {
            return ;
        }
        switch(appPark.ucTgtState)
        {
        case APP_PARK_STATE_OPENED:
            AppPark_OpenDoor();
            break;
        case APP_PARK_STATE_CLOSED:
            AppPark_CloseDoor();
            break;
        default:
            break;
        }
        
        break;
    }

}


void AppPark_Entry(uint8_t ucSwitchOn)
{
    DEBUG_LOG("ENTRY:%d,%d,%d",ucSwitchOn,appPark.ucCurMainState,appPark.ucCurSubState);

    if (appPark.ucBitOngoing)
    {
        appPark.ucPendingAction = ucSwitchOn ? APP_PARK_ACTION_OPEN : APP_PARK_ACTION_CLOSE;
        return ;
    }

    switch(gusMode)
    {
    default :
        break;
    case APP_PARK_MODE_1:
        if (APP_PARK_STATE_OPENED == appPark.ucCurMainState
            && APP_PARK_SUB4OPENED_OPENING == appPark.ucCurSubState)
        {
            appPark.ucPendingAction = ucSwitchOn ? APP_PARK_ACTION_OPEN :  APP_PARK_ACTION_CLOSE;
            return ;
        }
    
        if (APP_PARK_STATE_CLOSED == appPark.ucCurMainState
            && APP_PARK_SUB4CLOSED_CLOSING == appPark.ucCurSubState)
        {
            appPark.ucPendingAction = ucSwitchOn ? APP_PARK_ACTION_OPEN : APP_PARK_ACTION_CLOSE;
            return ;
        }
        break;
    }
    

    if (ucSwitchOn)
    {
        AppPark_OpenDoor();
    }
    else
    {
        AppPark_CloseDoor();
    }
}

void AppPark_HandleBuffWork(void)
{
    if ((APP_PARK_SUB_STATE_NULL == appPark.ucCurSubState)
        && (APP_PARK_ACTION_NULL != appPark.ucPendingAction))
    {
        uint8 ucSwitch = (APP_PARK_ACTION_OPEN == appPark.ucPendingAction);

        appPark.ucPendingAction = APP_PARK_ACTION_NULL;
        
        AppPark_Entry(ucSwitch);  

        return;
    }

}

void AppPark_RadioCmd(uint8_t *cmd,uint8_t ucLen)
{
    cmd[ucLen] = 0;

    DEBUG_LOG((char *)cmd,ucLen);
    {
        
        uint16_t devid = 0XFFFF;
        uint16_t cmdId = 0XFF;
        uint8_t  msgseq = 0XFF;
        
        char *pCont  = (char *)cmd;
        char *pTmp   = NULL;
        
        pTmp = strtok(pCont, ",");
        while(pTmp)  
        {   
            char *cfgAtt;
            char *cfgVal;
        
            if (!Att_value_parse(pTmp,'=',&cfgAtt,&cfgVal))
            {
                break;
            }

            if (!strcmp(cfgAtt,"M"))
            {
                msgseq = simple_strtoul(cfgVal);
            }
            else if (!strcmp(cfgAtt,"C"))
            {
                devid = simple_strtoul(cfgVal);
            }
            else if (!strcmp(cfgAtt,"S"))
            {
                cmdId = simple_strtoul(cfgVal);
            } 
            
            pTmp = strtok(NULL, ",");     
        }

        //VOS_LOG(VOS_LOG_DEBUG,"c = %d,m=%d,s=%d",devid,msgseq,cmdId);
        /* Packet RX */
        MainAlarmWithDuration(1);               

        if (devid != gusDeviceId)
        {
            return;
        }

        if (msgSeq == msgseq
            && (TICK2MS(ulAppTicks - msgTick)) <= 2000)
        {
            return;
        }

        msgSeq = msgseq;

        msgTick = ulAppTicks;

        switch(cmdId)
        {
        case 0:
            //RelayPulse(RELAY_DOOR_CLOSE_EN,gusPulseWidth,AppPark_PulseCb);
            AppPark_Entry(FALSE);
            break;
        case 1:
            //RelayPulse(RELAY_DOOR_OPEN_EN,gusPulseWidth,AppPark_PulseCb);
            AppPark_Entry(TRUE);
            break;
        case 2:
            // late implement
            // AppPark_Entry((APP_PARK_STATE_OPENED != appPark.ucCurMainState));
            // appPark.ucSwitch = TRUE;
            AppPark_Pulse(TRUE);
            break;
        default:
            break;
        }
    }
}


void AppPark_AdvanceState(void)
{
    switch(gusMode)
    {
    default:
        break;
    case APP_PARK_MODE_1:
        switch(appPark.ucCurMainState)
        {
        case APP_PARK_STATE_OPENED:
            switch(appPark.ucCurSubState)
            {
            case APP_PARK_SUB_STATE_NULL:
                break;
            case APP_PARK_SUB4OPENED_CLOSING:
                if (TICK2MS(ulAppTicks - appPark.ulStartTick) >= gusTime4Close)
                {
                    appPark.ucCurMainState = APP_PARK_STATE_CLOSED;
                    appPark.ucCurSubState  = APP_PARK_SUB_STATE_NULL;
                    if (APP_PARK_STATE_CLOSED == appPark.ucTgtState)
                    {
                        appPark.ucTgtState = 0XFF;
                    }
                    DEBUG_LOG("OPENED->CLOSED");
                }
                DEBUG_LOG("S:%d",TICK2MS(ulAppTicks - appPark.ulStartTick));
                
                break;
            case APP_PARK_SUB4OPENED_PAUSE:
                break;
            case APP_PARK_SUB4OPENED_OPENING:
                if (TICK2MS(ulAppTicks - appPark.ulStartTick) >= gusTime4Open)
                {
                    appPark.ucCurMainState = APP_PARK_STATE_OPENED;
                    appPark.ucCurSubState  = APP_PARK_SUB_STATE_NULL;
                    if (APP_PARK_STATE_OPENED == appPark.ucTgtState)
                    {
                        appPark.ucTgtState = 0XFF;
                    }
                    DEBUG_LOG("OPENED->OPENED");
                    
                }
                DEBUG_LOG("S:%d",TICK2MS(ulAppTicks - appPark.ulStartTick));
                
                break;
            default:
                break;
            }
            break;
        default: // APP_PARK_STATE_CLOSED
            switch(appPark.ucCurSubState)
            {
            case APP_PARK_SUB_STATE_NULL:
                break;
            case APP_PARK_SUB4CLOSED_OPENING:
                if (TICK2MS(ulAppTicks - appPark.ulStartTick) >= gusTime4Open)
                {
                    appPark.ucCurMainState = APP_PARK_STATE_OPENED;
                    appPark.ucCurSubState  = APP_PARK_SUB_STATE_NULL;
                    if (APP_PARK_STATE_OPENED == appPark.ucTgtState)
                    {
                        appPark.ucTgtState = 0XFF;
                    }
                    DEBUG_LOG("CLOSED->OPENED");
                    
                }
                DEBUG_LOG("S:%d",TICK2MS(ulAppTicks - appPark.ulStartTick));
                
                break;
            case APP_PARK_SUB4CLOSED_PAUSE:
                break;
            case APP_PARK_SUB4CLOSED_CLOSING:
                if (TICK2MS(ulAppTicks - appPark.ulStartTick) >= gusTime4Close)
                {
                    appPark.ucCurMainState = APP_PARK_STATE_CLOSED;
                    appPark.ucCurSubState  = APP_PARK_SUB_STATE_NULL;
                    if (APP_PARK_STATE_CLOSED == appPark.ucTgtState)
                    {
                        appPark.ucTgtState = 0XFF;
                    }
                    DEBUG_LOG("CLOSED->CLOSED");
                }
                DEBUG_LOG("S:%d",TICK2MS(ulAppTicks - appPark.ulStartTick));
                break;
            default:
                break;
            }        
            break;
        }    
        break;
    }
}

void AppPark_SecondTask(void)
{
    AppPark_AdvanceState();

    AppPark_HandleBuffWork();

}
void AppPark_Init(void)
{
    msgSeq = 0XFFFF;

    memset(&appPark,0,sizeof(appPark));

    appPark.ucTgtState = 0XFF;
    appPark.ucPendingAction = APP_PARK_ACTION_NULL;

}
