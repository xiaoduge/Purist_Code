#include    <ucos_ii.h>

#include    <string.h>

#include "stm32f10x.h"


#include "stm32_eval.h"

#include "app_cfg.h"

#include "ad7799.h"

#include "ad7799_Driver.h"


int AD7799_sh(int event,int chl,void *para)
{

   if (DICA_SENSOR_EVENT_FALLING == event)
   {
       // data ready , read data
   }

   return TRUE;
}

uint8_t AD7799_Init(uint8_t Gain)  
{  
    uint8_t ID;
	//uint8_t Cmd[2];  

    // sAD7799_Install_WaitBusy_Callback(AD7799_sh,0);
    sAD7799_Reset();  
    OSTimeDlyHMSM(0,0,0,20);  
    sAD7799_ReadReg(AD7799_MAKE_REG(AD7799_RADDR_ID),&ID,1);               //读取器件ID  
    if((ID==0xFF)||(ID==0x00))
    {
        return 1;  
    }

    //VOS_LOG(VOS_LOG_DEBUG,"AD7799 ID =%02x",ID);
    
    sAD7799_Calibrate(AD7799_CH0,Gain);             //通道1校准  for conductive rate
    sAD7799_Calibrate(AD7799_CH1,Gain);             //通道2校准  for temperature

    // ylf: example code for setting CH2 to general IO
    // Cmd[0] = IO_REG_ENABLE|IO_REG_5|IO_REG_4;  
    // sAD7799_WriteReg(AD7799_WADDR_IO,Cmd,1);  
    return 0;  
}  

void AD7799_Start(uint8_t CovChx,uint8_t CovGain,uint8_t CovRate,uint8_t CovMode)  
{  
    uint8_t Cmd[2];  
    
    Cmd[0] = CON_REG_UNIPOL|CovGain;  
    Cmd[1] = CON_REG_BUFFER|CovChx;  
    sAD7799_WriteReg(AD7799_MAKE_REG(AD7799_WADDR_CONFIG),Cmd,2); 
    
    Cmd[0] = CovMode;  
    Cmd[1] = CovRate;  
    sAD7799_WriteReg(AD7799_MAKE_REG(AD7799_WADDR_MODE),Cmd,2);  
}  

uint32_t AD7799_Read(void)  
{  
    uint8_t  Cmd[4];  
    uint32_t D;  
    Cmd[0]=0;  
    sAD7799_ReadReg(AD7799_MAKE_REG(AD7799_RADDR_DATA),&Cmd[1],3);  
    D = (Cmd[1]<<16)|(Cmd[2]<<8)|Cmd[3];  
    return D;  
} 


