#include <string.h>
#include "Display.h"

#include "RTC_Driver.h"

#include "SDDriver_hal.h"

#include "storage.h"

#include "Config.h"

#include "UartCmd.h"

#include "ff.h"

#include <string.h>

#include <stdio.h>

#define STORAGE_SECTOR_SIZE (512)


typedef struct
{
    DISP_INFO_STRU DispInfo;
    UINT32         ulUFWashTime; // in second
}STORAGE_RUN_TIME_INFO;


STORAGE_RUN_TIME_INFO sStorageRunTimeInfo;

FATFS fs;

STORAGE_USER_INFO  sUsrInfo;


uint8_t Storage_ReadRunTimeInfo(DISPLAY_STRU *pDisp)
{
    memcpy(&sStorageRunTimeInfo,pRuntimeInfo,sizeof(STORAGE_RUN_TIME_INFO));
    // validating data
    if (INVALID_CONFIG_VALUE_LONG == pDisp->info.PACKLIFEDAY
        && (INVALID_CONFIG_VALUE_LONG != sStorageRunTimeInfo.DispInfo.PACKLIFEDAY))
    {
        Display.info.PACKLIFEDAY =  sStorageRunTimeInfo.DispInfo.PACKLIFEDAY;
    }

    
    if (INVALID_CONFIG_VALUE == pDisp->info.PACKLIFEL
        && (INVALID_CONFIG_VALUE != sStorageRunTimeInfo.DispInfo.PACKLIFEL))
    {
        Display.info.PACKLIFEL = sStorageRunTimeInfo.DispInfo.PACKLIFEL; 
    }
    
    if (INVALID_CONFIG_VALUE_LONG == pDisp->info.FILTERLIFE
        && (INVALID_CONFIG_VALUE_LONG != sStorageRunTimeInfo.DispInfo.FILTERLIFE))
    {
        Display.info.FILTERLIFE = sStorageRunTimeInfo.DispInfo.FILTERLIFE; 
    }
    
    if (INVALID_CONFIG_VALUE_LONG == pDisp->info.UVLIFEDAY
        && INVALID_CONFIG_VALUE_LONG != sStorageRunTimeInfo.DispInfo.UVLIFEDAY)
    {
        Display.info.UVLIFEDAY = sStorageRunTimeInfo.DispInfo.UVLIFEDAY; 
    }
    
    if (INVALID_CONFIG_VALUE == pDisp->info.UVLIFEHOUR
        && INVALID_CONFIG_VALUE != sStorageRunTimeInfo.DispInfo.UVLIFEHOUR)
    {
        Display.info.UVLIFEHOUR = sStorageRunTimeInfo.DispInfo.UVLIFEHOUR; 
    }

    pDisp->ulUFCleanTime = sStorageRunTimeInfo.ulUFWashTime;
    
    return 0;
}

uint8_t Storage_WriteRunTimeInfo(DISPLAY_STRU *pDisp)
{
    int tmp = sizeof(STORAGE_RUN_TIME_INFO);

	return tmp;
}


uint8_t Storage_WriteRunTimeInfo2Flash(DISPLAY_STRU *pDisp)
{
    int tmp = sizeof(STORAGE_RUN_TIME_INFO);

    memcpy(&sStorageRunTimeInfo.DispInfo,&pDisp->info,sizeof(DISP_INFO_STRU));

    sStorageRunTimeInfo.ulUFWashTime = pDisp->ulUFCleanTime;

    // move to buffer
    memcpy(Config_buff,&sStorageRunTimeInfo,tmp);

    Config_SaveData(CONFIG_RUN_TIME_INFO_PAGE,Config_buff,tmp);

    return tmp;
}


uint8_t Storage_WriteMeasInfo(DISPLAY_STRU *pDisp,tm *ptm)
{
#if SD_SUPPORT > 0
    FRESULT res;
    FIL file;
    UINT size;
    char fileName[24];

    uint8 tmp[4];

    sprintf(fileName,"0:/%02d-%02d.dat",ptm->w_month,ptm->w_date);
    
    res = f_open(&file, fileName, FA_OPEN_ALWAYS | FA_WRITE);
    
    if(res != FR_OK)
    {
        return 0XF0;
    }

	  {
        tmp[0] = pDisp->aulBestCdt[0] & 0XFF;
        tmp[1] = pDisp->aulBestCdt[0] >>  8 & 0XFF;
        tmp[2] = pDisp->aulBestCdt[0] >>  16 & 0XFF;
        tmp[3] = pDisp->aulBestCdt[0] >>  24 & 0XFF;
		
        res = f_write(&file,&tmp, 4, &size);
	  }

	  pDisp->aulBestCdt[0] = 0;

    if(res != FR_OK)
    {
        f_close(&file); 

        return res;
    }

    f_close(&file); 
    return 0;
#else
  return 0XF0;
#endif
}

int Storage_GetMeasInfo(uint8 *iData,tm *ptm)
{
#if SD_SUPPORT > 0

    FRESULT res;
    FIL file;

    UINT size;

    char fileName[24];


    sprintf(fileName,"0:/%02d-%02d.dat",ptm->w_month,ptm->w_date);
    
    res = f_open(&file, fileName, FA_READ);
    
    if(res != FR_OK)
    {
        return 0XF0;
    }
    
    if(f_read( &file, iData , 4,&size) != FR_OK)
    {
        f_close(&file);

        return 0xF1;
    }
    
    f_close(&file); 

    return 0;
#else
    return 0XF0;    
#endif  
}

int Storage_AddUser(char *szName,char *szPass,char attr)
{
#if	SD_SUPPORT > 0
	FRESULT res;
	FIL file;
	UINT size;
	char fileName[24];

	sprintf(fileName,"0:/%s",szName);

	memset(&sUsrInfo,0,sizeof(sUsrInfo));

	strcpy(sUsrInfo.Name,szName);
	sUsrInfo.attr = attr;

	if(Storage_GetUser(szName,&sUsrInfo) == FR_OK)
	{
	    res = f_open(&file, fileName, FA_OPEN_ALWAYS | FA_WRITE);
	}
	else
	{
    	res = f_open(&file, fileName, FA_CREATE_ALWAYS | FA_WRITE);
	}
	
	if(res != FR_OK)
	{
		return 0XF0;
	}

	strcpy(sUsrInfo.Pwd,szPass);

	if(f_write(&file,&sUsrInfo, sizeof(sUsrInfo), &size) != FR_OK)
	{
	    f_close(&file); 

	    return 0xF1;
	}

	f_close(&file); 
	return 0;
#else
  return 0XF0;  	
#endif
}



int Storage_RmvUser(char *szName)
{
#if SD_SUPPORT > 0
    FRESULT res ;

    char fileName[24];

    sprintf(fileName,"0:/%s",szName);
    
    res = f_unlink(fileName);
    return res;
#else
  return 0XFF;  
#endif  
}


int Storage_UpdateUser(STORAGE_USER_INFO *pUsrInfo)
{
#if SD_SUPPORT > 0
    FRESULT res;
    FIL file;

    UINT size;

    char fileName[24];

    sprintf(fileName,"0:/%s",pUsrInfo->Name);
    
    res = f_open(&file, fileName, FA_OPEN_ALWAYS | FA_WRITE);
    
    if(res != FR_OK)
    {
        return 0XF0;
    }

    
    if(f_write(&file,pUsrInfo, sizeof(STORAGE_USER_INFO), &size) != FR_OK)
    {
        f_close(&file); 
        return 0xF1;
    }

    f_close(&file); 
    return 0;
#else
  return 0XF0;  
#endif
}

int Storage_CheckUser(char *szName,char *szPass)
{
#if SD_SUPPORT > 0

    FRESULT res;
    FIL file;

    UINT size;

    char fileName[24];


    sprintf(fileName,"0:/%s",szName);
    
    res = f_open(&file, fileName, FA_READ);
    
    if(res != FR_OK)
    {
        return 0XF0;
    }
    
    memset(&sUsrInfo,0,sizeof(sUsrInfo));
    
    if(f_read( &file,&sUsrInfo, sizeof(sUsrInfo),&size) != FR_OK)
    {
        f_close(&file);
        return 0xF1;
    }
    
    f_close(&file); 

    if (strcmp(sUsrInfo.Name, szName) != 0)
    {
        return 0XF2;
    }

    if (strcmp(sUsrInfo.Pwd, szPass) != 0)
    {
        return 0XF2;
    }
    return 0;
#else
    return 0XF0;    
#endif
    
}

int Storage_GetUser(char *szName,STORAGE_USER_INFO *ui)
{
#if SD_SUPPORT > 0

    FRESULT res;
    FIL file;

    UINT size;

    char fileName[24];


    sprintf(fileName,"0:/%s",szName);
    
    res = f_open(&file, fileName, FA_READ);
    
    if(res != FR_OK)
    {
        return 0XF0;
    }
    
    if(f_read( &file,ui, sizeof(STORAGE_USER_INFO),&size) != FR_OK)
    {
        f_close(&file);

        return 0xF1;
    }
    
    f_close(&file); 

    return 0;
#else
    return 0XF0;    
#endif
    
}


void Storage_Init(void)
{
#if SD_SUPPORT > 0

    FRESULT res;

    res = f_mount(0, &fs);
    
    if(res != FR_OK)
    {
      UartCmdPrintf(VOS_LOG_ERROR,"f_mount fail %d\n",res);
    }
#endif  
    memset(&sStorageRunTimeInfo,0xff,sizeof(sStorageRunTimeInfo));

}
